#ifndef SV_MOTD_H
#define SV_MOTD_H
#include <stddef.h>
#include <string.h>
/* Presentation of the setup field, not a packet decoder. Each row owns its
 * formatting state just like baseline show_motd / Term_putstr. */
#define SV_MOTD_ROWS 23
#define SV_MOTD_STRIDE 120
typedef struct {
    unsigned char glyphs[SV_MOTD_ROWS][SV_MOTD_STRIDE];
    unsigned char colors[SV_MOTD_ROWS][SV_MOTD_STRIDE];
    size_t length[SV_MOTD_ROWS], columns;
} SvMotd;
static inline SvMotd sv_motd_prepare(const unsigned char *bytes,size_t size)
{
    SvMotd out={0};
    const char *palette="dwsorgbuDWvyRGBUpfaechmLCSHAMIPxnTqFVtENQYilOkKjJXzZ0123456789";
    for(size_t row=0;bytes && row<SV_MOTD_ROWS;++row) {
        size_t start=row*SV_MOTD_STRIDE;
        unsigned char color=1,first=1,previous=1;
        int initial=1;
        for(size_t i=0;i<SV_MOTD_STRIDE && start+i<size && bytes[start+i];++i) {
            unsigned char c=bytes[start+i];
            if(c==255) {
                if(++i>=SV_MOTD_STRIDE || start+i>=size || !(c=bytes[start+i]))break;
                if(c==255)c='{';
                else if(c=='-') {previous=color;color=first;continue;}
                else if(c=='.') {unsigned char swap=color;color=previous;previous=swap;continue;}
                else if(c=='%')continue;
                else {
                    const char *found=strchr(palette,c);
                    if(found) {
                        previous=color;color=(unsigned char)(found-palette);
                        if(initial)first=color;
                        continue;
                    }
                    /* An unrecognised code retains its symbol, as Term_putstr. */
                }
            }
            initial=0;
            size_t column=out.length[row]++;
            out.glyphs[row][column]=c;out.colors[row][column]=color;
        }
        if(out.length[row]>out.columns)out.columns=out.length[row];
    }
    return out;
}
#endif
