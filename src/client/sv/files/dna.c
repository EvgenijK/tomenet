#define _POSIX_C_SOURCE 200809L
#include "files/dna.h"
#define CLIENT_SIDE
#include "../../../common/defines.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/* Exact baseline roman suffix parser, local to avoid linking common.c globals. */
static char *roman_suffix(char* cname, int *arabic_ptr) {
	char *p, *p0;
	int arabic = 0, rome_prev = 0, maybe_prefix_val;
	bool maybe_prefix = false;

	/* Not long enough to contain roman number? */
	if (strlen(cname) < 3) return(NULL);
	/* No separator between name and number? */
	if (!strchr(cname, ' ')) return(NULL);

	/* Locate begin of the roman number */
	p = cname + strlen(cname) - 1;
	while (*p != ' ') p--;
	p0 = p + 1;

	/* Extract and validate roman number */
	while (*(++p)) {
		switch (*p) {
		case 'I':
			if (arabic % 5 >= 3) return(NULL); //this digit may never occur more than 3 times in a row

			if (maybe_prefix) arabic += rome_prev; //prefix was not a prefix but a normal digit ('I' cannot get a prefix)
			if (arabic % 5 == 0) {
				maybe_prefix = true;
				maybe_prefix_val = 1;
			} else {
				arabic++;
				maybe_prefix = false;
			}
			rome_prev = 1;
			break;
		case 'V':
			if (rome_prev) {
				if (rome_prev == 5) return(NULL); //this specific digit is never allowed to repeat
				if (rome_prev < 5 && !maybe_prefix) return(NULL); //smaller digits before us are only allowed if they can be a subtractive prefix to us
			}

			if (maybe_prefix) { //was prefix a prefix or a normal digit?
				if (rome_prev == 1) arabic--;
				else arabic += rome_prev;
			}
			arabic += 5;
			maybe_prefix = false;
			rome_prev = 5;
			break;
		case 'X':
			if (arabic % 50 >= 30) return(NULL); //this digit may never occur more than 3 times in a row
			if (rome_prev) {
				if (rome_prev < 10 && !maybe_prefix) return(NULL); //smaller digits before us are only allowed if they can be a subtractive prefix to us
			}

			if (maybe_prefix) { //was prefix a prefix or a normal digit?
				if (rome_prev == 1) arabic--;
				else arabic += rome_prev;
			}
			if (arabic % 50 == 0) {
				maybe_prefix = true;
				maybe_prefix_val = 10;
			} else {
				arabic += 10;
				maybe_prefix = false;
			}
			rome_prev = 10;
			break;
		case 'L':
			if (rome_prev) {
				if (rome_prev == 50) return(NULL); //this specific digit is never allowed to repeat
				if (rome_prev < 50 && !maybe_prefix) return(NULL); //smaller digits before us are only allowed if they can be a subtractive prefix to us
				if (rome_prev < 10) return(NULL); //catch previous digits that can be prefix but are too small to work as prefix for the current digit
			}

			if (maybe_prefix) { //was prefix a prefix or a normal digit?
				if (rome_prev == 10) arabic -= 10;
				else arabic += rome_prev;
			}
			arabic += 50;
			maybe_prefix = false;
			rome_prev = 50;
			break;
		case 'C':
			if (arabic % 500 >= 300) return(NULL); //this digit may never occur more than 3 times in a row
			if (rome_prev) {
				if (rome_prev < 100 && !maybe_prefix) return(NULL); //smaller digits before us are only allowed if they can be a subtractive prefix to us
				if (rome_prev < 10) return(NULL); //catch previous digits that can be prefix but are too small to work as prefix for the current digit
			}

			if (maybe_prefix) { //was prefix a prefix or a normal digit?
				if (rome_prev == 10) arabic -= 10;
				else arabic += rome_prev;
			}
			if (arabic % 500 == 0) {
				maybe_prefix = true;
				maybe_prefix_val = 100;
			} else {
				arabic += 100;
				maybe_prefix = false;
			}
			rome_prev = 100;
			break;
		case 'D':
			if (rome_prev) {
				if (rome_prev == 500) return(NULL); //this specific digit is never allowed to repeat
				if (rome_prev < 500 && !maybe_prefix) return(NULL); //smaller digits before us are only allowed if they can be a subtractive prefix to us
				if (rome_prev < 100) return(NULL); //catch previous digits that can be prefix but are too small to work as prefix for the current digit
			}

			if (maybe_prefix) { //was prefix a prefix or a normal digit?
				if (rome_prev == 100) arabic -= 100;
				else arabic += rome_prev;
			}
			arabic += 500;
			maybe_prefix = false;
			rome_prev = 500;
			break;
		case 'M':
			// (no limit for amount of Ms occurring in a row..)
			if (rome_prev) {
				if (rome_prev < 1000 && !maybe_prefix) return(NULL); //smaller digits before us are only allowed if they can be a subtractive prefix to us
				if (rome_prev < 100) return(NULL); //catch previous digits that can be prefix but are too small to work as prefix for the current digit
			}

			if (maybe_prefix) { //was prefix a prefix or a normal digit?
				if (rome_prev == 100) arabic -= 100;
				//else arabic += rome_prev; //not needed because of 'irregular' below: maybe_prefix can never be set to true from an 'M', so we don't need to catch that case <-here.
			}
			arabic += 1000;
			maybe_prefix = false; //irregular ;) simply as it's the highest roman number..
			rome_prev = 1000;
			break;
		case '.':
			// allow one optional final trailing dot
			if (!*(p + 1)) continue;
		default:
			/* Other letters do not belong into roman numbers */
			return(NULL);
		}
	}

	/* Finish up the arabic number and return it too */
	if (arabic_ptr) {
		if (maybe_prefix) arabic += maybe_prefix_val;
		*arabic_ptr = arabic;
	}

	/* Success, return starting position of the roman number */
	return(p0);
}

bool sv_dna_filename(char out[256], const char *name)
{
    if (!name || !*name || strlen(name)>249 || strchr(name,'/') || strchr(name,'\\') ||
        strchr(name,'\n') || strchr(name,'\r')) return false;
    char normalized[256]; strcpy(normalized,name);
    char *suffix=roman_suffix(normalized,NULL);
    if (suffix) suffix[-1]=0;
    if (!*normalized || !strcmp(normalized,".") || !strcmp(normalized,"..")) return false;
    size_t n=strlen(normalized);
    snprintf(out,256,"%s%s",normalized,n>4 && !strcmp(normalized+n-4,".dna")?"":".dna");
    return true;
}
static SvFileResult save_dna(const SvDna *dna, const char *root, const char *name,
                             bool check_version, const struct stat *expected)
{
    char filename[256],text[512]; SvFilePath path;
    if (!dna || !sv_dna_filename(filename,name) || !sv_file_shared_user(&path,root,filename)) return SV_FILE_INVALID;
    int size=snprintf(text,sizeof(text),"Character birth file for TomeNET v%d.%d.%d%s\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n",
        VERSION_MAJOR,VERSION_MINOR,VERSION_PATCH,CLIENT_VERSION_TAG,
        dna->mode,dna->class_id,dna->race,dna->trait,dna->stats[0],dna->stats[1],dna->stats[2],dna->stats[3],dna->stats[4],dna->stats[5]);
    if (size<0 || (size_t)size>=sizeof(text)) return SV_FILE_INVALID;
    SvFilePublication op;
    SvFileResult result=sv_file_publish_begin(&op,&path,false);
    if (result!=SV_FILE_OK && result!=SV_FILE_REPLACE_REQUIRED) return result;
    if (check_version && ((expected!=NULL)!=op.existed || (expected &&
        (expected->st_dev!=op.observed.st_dev || expected->st_ino!=op.observed.st_ino ||
         expected->st_size!=op.observed.st_size ||
         expected->st_mtim.tv_sec!=op.observed.st_mtim.tv_sec || expected->st_mtim.tv_nsec!=op.observed.st_mtim.tv_nsec ||
         expected->st_ctim.tv_sec!=op.observed.st_ctim.tv_sec || expected->st_ctim.tv_nsec!=op.observed.st_ctim.tv_nsec)))) {
        sv_file_publish_cancel(&op); return SV_FILE_CONFLICT;
    }
    result=sv_file_publish_write(&op,text,(size_t)size);
    return result==SV_FILE_OK?sv_file_publish_commit(&op,true):result;
}
SvFileResult sv_dna_save(const SvDna *dna, const char *root, const char *name)
{ return save_dna(dna,root,name,false,NULL); }
SvFileResult sv_dna_load(SvDna *out, const char *root, const char *bundle,
                        const char *name, const char *previous)
{
    char filename[256],relative[272],header[256]; SvFilePath path;
    const char *source=previous && *previous?previous:name;
    if (!out || !sv_dna_filename(filename,source)) return SV_FILE_INVALID;
    snprintf(relative,sizeof(relative),"user/%s",filename);
    SvFileResult result=sv_file_resource(&path,root,bundle,relative);
    if (result!=SV_FILE_OK) return result;
    FILE *file=sv_file_open(&path,"rb");
    if (!file) return SV_FILE_IO;
    struct stat loaded;
    if (fstat(fileno(file),&loaded)) { fclose(file); return SV_FILE_IO; }
    SvDna next={0}; int values[10],major=0,minor=0,patch=0; char tag=0;
    bool valid=fgets(header,sizeof(header),file)!=NULL;
    const char *at=valid?strstr(header,"TomeNET v"):NULL;
    valid=at && sscanf(at,"TomeNET v%2d.%2d.%2d%c",&major,&minor,&patch,&tag)==4;
    for (unsigned i=0;valid && i<10;++i) {
        char token[64]; char *end;
        if (fscanf(file,"%63s",token)!=1) { valid=false; break; }
        errno=0; long value=strtol(token,&end,10);
        if (errno || *end || value<INT16_MIN || value>INT16_MAX) { valid=false; break; }
        values[i]=(int)value;
    }
    if (ferror(file)) { fclose(file); return SV_FILE_IO; }
    if (fclose(file)) return SV_FILE_IO;
    if (!valid || values[1]<0 || values[1]>=255 || values[2]<0 || values[2]>=255 ||
        values[3]<0 || values[3]>=255) return SV_FILE_INVALID;
    next.mode=(int16_t)values[0]; next.class_id=(int16_t)values[1];
    next.race=(int16_t)values[2]; next.trait=(int16_t)values[3];
    for (unsigned i=0;i<6;++i) next.stats[i]=(int16_t)values[4+i];
    bool old=major<4 || (major==4 && (minor<7 || (minor==7 && (patch<1 || (patch==1 && tag<'b')))));
    if (old) {
        if (next.mode&1) next.mode=(int16_t)((next.mode&~1)|0x100);
        if (next.mode&0x20) next.mode=(int16_t)((next.mode&~0x20)|0x200);
        result=save_dna(&next,root,source,true,path.owner==SV_FILE_SHARED?&loaded:NULL);
        if (result!=SV_FILE_OK) return result;
    }
    *out=next;
    return SV_FILE_OK;
}
