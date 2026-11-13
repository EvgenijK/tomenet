#include "settings/startup-ins.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <errno.h>

/* ai_fgets semantics: comments begin with exactly the ### prefix; blank lines
 * are records, and each physical read is bounded to the legacy 1024 bytes. */
static bool line(FILE *file, char *out, size_t capacity)
{
    char bytes[1024];
    do { if (!fgets(bytes, sizeof(bytes), file)) return false; }
    while (!strncmp(bytes, "###", 3));
    size_t n = strlen(bytes);
    if (n && bytes[n-1] == '\n') bytes[--n] = 0;
    if (n && bytes[n-1] == '\r') bytes[--n] = 0;
    if (n >= capacity) n = capacity - 1;
    memcpy(out, bytes, n); out[n] = 0;
    return true;
}

static unsigned format(const char *header)
{
    int major=0, minor=0, patch=0, extra=0, branch=0, build=0;
    char tag[80] = {0};
    int fields = sscanf(header, "Auto-Inscriptions file for TomeNET v%d.%d.%d.%d.%d.%d%79s",
        &major,&minor,&patch,&extra,&branch,&build,tag);
    if (fields < 6) {
        tag[0] = 0;
        if (sscanf(header,"Auto-Inscriptions file for TomeNET v%d.%d.%d%79s",
            &major,&minor,&patch,tag) < 3) return 0;
    }
    if (major<4 || (major==4 && (minor<7 || (minor==7 &&
        (patch<1 || (patch==1 && (!tag[0] || tag[0]=='a'))))))) return 1;
    if (major==4 && minor==7 && patch<4) return 2;
    if (major==4 && (minor<9 || (minor==9 &&
        (patch<1 || (patch==1 && (!tag[0] || tag[0]=='-')))))) return 3;
    /* Legacy compat branch ultimately selects v5 even for bare 4.9.3;
     * numeric flag lines below trigger its 4.9.3-Test repair. */
    return 5;
}

static bool old_flags(FILE *file, SvStartupInsRule *rule, unsigned version,
                      const char *first)
{
    char bytes[5];
    if (version >= 3) {
        if (first) snprintf(bytes,sizeof(bytes),"%s",first);
        else if (!line(file,bytes,sizeof(bytes))) return false;
        int pickup=atoi(bytes);
        rule->ignore=pickup>=2;
        if (pickup>=2) pickup-=2;
        rule->autopickup=pickup!=0;
        if (!line(file,bytes,sizeof(bytes))) return false;
        rule->autodestroy=atoi(bytes)!=0;
    }
    if (version>=4) {
        if (!line(file,bytes,sizeof(bytes))) return false;
        rule->bags_only=atoi(bytes)!=0;
        if (!line(file,bytes,sizeof(bytes))) return false;
        rule->disabled=atoi(bytes)!=0;
    }
    return true;
}

static void convert(SvStartupIns *ins)
{
    SvFilePublication publication;
    SvFileResult opened=sv_file_publish_begin(&publication,&ins->source,false);
    if (opened!=SV_FILE_OK && opened!=SV_FILE_REPLACE_REQUIRED) {
        ins->conversion_failed=true; return;
    }
    FILE *out=publication.stream;
    bool ok=fprintf(out,"Auto-Inscriptions file for TomeNET v4.9.3.0.0.3\n")>0;
    for (size_t i=0;i<SV_INS_RULES && ok;i++) {
        const SvStartupInsRule *r=&ins->rules[i];
        ok=fprintf(out,"%s\n%s\n  %c,%c,%c,%c\n",r->match,r->tag,
            r->force?'F':'-',r->ignore?'i':r->autopickup?'a':r->autodestroy?'A':'-',
            r->bags_only?'b':'-',r->disabled?'X':'-')>0;
    }
    if (ok && sv_file_publish_commit(&publication,true)==SV_FILE_OK) ins->converted=true;
    else { sv_file_publish_cancel(&publication); ins->conversion_failed=true; }
}

static bool load(SvStartupIns *ins, const SvFilePath *path)
{
    FILE *file=sv_file_open(path,"r");
    if (!file) {
        if(errno!=ENOENT) { ins->status=SV_INS_IO; ins->warnings++; ins->source=*path; }
        return false;
    }
    char bytes[1024];
    if (!line(file,bytes,80)) {
        bool failed=ferror(file)!=0;
        if(fclose(file)) failed=true;
        if(failed) { ins->status=SV_INS_IO; ins->warnings++; ins->source=*path; }
        return false;
    }
    unsigned version=format(bytes);
    if (!version) { ins->warnings++; fclose(file); return false; }
    ins->format=version;
    bool partial=false, repair=false;
    for (unsigned i=0;i<SV_INS_RULES;i++) {
        if (!line(file,bytes,version<5?SV_INS_MATCH+1:SV_INS_MATCH)) break;
        SvStartupInsRule next={0};
        char *match=bytes;
        if (version<5 && *match=='!') { next.force=true; match++; }
        else if (version<5) match[SV_INS_MATCH-1]=0;
        if (version==1) for(char *c=match;*c;c++) if(*c=='?') *c='#';
        size_t match_size=strlen(match);
        if(match_size>=sizeof(next.match)) match_size=sizeof(next.match)-1;
        memcpy(next.match,match,match_size); next.match[match_size]=0;
        size_t slot=ins->count;
        bool duplicate=false;
        if (next.match[0]) {
            for(size_t j=0;j<ins->count;j++)
                if (!strcmp(ins->rules[j].match,next.match)) { slot=j; duplicate=true; break; }
            if(slot==ins->count) ins->count++;
            if(!duplicate) ins->rules[slot]=next; /* new partial match is retained */
        }
        char tag[SV_INS_TAG];
        if (!line(file,tag,sizeof(tag))) { partial=true; break; }
        if(duplicate) {
            bool force=next.force;
            next=ins->rules[slot]; next.force=force;
        }
        memcpy(next.tag,tag,sizeof(tag));
        if(next.match[0]) ins->rules[slot]=next;
        bool complete=true;
        if(version<5) {
            next.ignore=false;
            complete=old_flags(file,&next,version,NULL);
        }
        else {
            if (!line(file,bytes,sizeof(bytes))) { partial=true; break; }
            if(bytes[0]!=' ') {
                repair=true; version=4;
                if(next.match[0]=='!') {
                    memmove(next.match,next.match+1,strlen(next.match)); next.force=true;
                }
                complete=old_flags(file,&next,4,bytes);
            } else {
                char force,pickup,bags,disabled;
                complete=sscanf(bytes,"  %c,%c,%c,%c",&force,&pickup,&bags,&disabled)==4;
                if(complete) {
                    next.force=force=='F'; next.ignore=pickup=='i';
                    next.autopickup=pickup=='a'; next.autodestroy=pickup=='A';
                    next.bags_only=bags=='b'; next.disabled=disabled=='X';
                }
            }
        }
        if(next.match[0]) {
            if(next.match[0]=='$') {
                regex_t compiled;
                next.invalid_regex=regcomp(&compiled,next.match+1,REG_EXTENDED|REG_ICASE)!=0;
                if(!next.invalid_regex) regfree(&compiled);
            }
            ins->rules[slot]=next;
        }
        if(!complete) { partial=true; break; }
    }
    bool io=ferror(file)!=0;
    if(fclose(file)) io=true;
    if(io) { partial=true; ins->warnings++; ins->status=SV_INS_IO; }
    if(!ins->count) {
        if(io || version<5 || repair) ins->source=*path;
        if(version<5 || repair) convert(ins);
        return false;
    }
    ins->source=*path;
    ins->status=partial?SV_INS_PARTIAL:SV_INS_READY;
    ins->format=version;
    if(version<5 || repair) convert(ins);
    return true;
}

SvStartupIns *sv_startup_ins_load(const char *root, const char *character,
    const char *class_name, const char *trait_name, const char *race_name)
{
    if(!root) return NULL;
    SvStartupIns *ins=calloc(1,sizeof(*ins));
    if(!ins) return NULL;
    const char *names[]={character,class_name,trait_name,race_name,"global"};
    for(size_t i=0;i<sizeof(names)/sizeof(*names);i++) {
        if(!names[i] || !names[i][0]) continue;
        char name[256];
        int count=snprintf(name,sizeof(name),"%s.ins",names[i]);
        SvFilePath path;
        if(count<0 || (size_t)count>=sizeof(name) ||
           !sv_file_shared_user(&path,root,name)) { ins->warnings++; continue; }
        if(load(ins,&path)) return ins;
        /* No entry loaded: next most-specific file starts with an empty list. */
        memset(ins->rules,0,sizeof(ins->rules)); ins->count=0;
    }
    if(ins->status!=SV_INS_IO) ins->status=SV_INS_MISSING;
    return ins;
}
void sv_startup_ins_destroy(SvStartupIns *ins) { free(ins); }
