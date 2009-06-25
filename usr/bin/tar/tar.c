/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */

#define __POSIX_C_SOURCE 200112L
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include "tar.h"


/*=============================================================================
 * Bardzo prosta implementacja USTAR (Uniform Standard TAR), obs³ugujemy
 * te¿ GNU sygnaturê (GNU tar produkuje tary oznaczone "prawie" jako USTAR).
 *      -- wieczyk
 */

enum OPER {
    TEST = 't',
    APPEND = 'r',
    EXTRACT = 'x',
    CREATE = 'c',
    USAGE = 'h'
};

/// nag³ówek wpisu w TAR
struct tentry {
    char    name[100];
    char    mode[8];
    char    uid[8];
    char    gid[8];
    char    size[12];
    char    mtime[12];
    char    chksum[8];
    char    typeflag;
    char    linkname[100];
    char    magic[6];
    char    version[2];
    char    uname[32];  
    char    gname[32];
    char    major[8];
    char    minor[8];
    char    prefix[155];
};



static const char *tar = "tar";
static const char *gzip = "/bin/gzip";

/*=============================================================================
 * pomocnicze procedury
 */

static void write_header(FILE *, struct tentry *, const char *, const char *,
        uid_t, gid_t, size_t, mode_t, time_t );
static void write_num(char *, int, int);
static unsigned int read_num(const char *, int );
static int is_in_list(char **, const char *);
static const char *mode2str(const char *, int);

#define WRITE_NUM(dst, num) write_num(dst, sizeof(dst), num)
#define READ_NUM(src) read_num(src, sizeof(src))


void
write_num(char *dst, int size, int num)
{
    char helper[50];
    char format[10];
    snprintf(format, sizeof(format), "%%0%uo", size-1);
    snprintf(helper, size, format, num);
    memcpy(dst, helper, size);
}

unsigned int
read_num(const char *src, int size)
{
    char helper[50];
    unsigned int num;
    memset(helper, 0, 50);
    memcpy(helper, src, size);
    sscanf(helper, "%o", &num);
    return num;
}

const char *
mode2str(const char *_m, int type)
{
#define iss(bit, ch) ((m&bit)? ch : '-')
    static char out[11];
    mode_t m = read_num(_m, 12);
    switch (type) {
        case AREGTYPE:
        case REGTYPE:
            out[0] = '-';
            break;
        case DIRTYPE:
            out[0] = 'd';
            break;
        default:
            out[0] = '?';
            break;
    }
    // tak, mo¿na to zrobiæ w 3 iteracjach jakiej¶
    // pêtli interpretuj±cej po 3 bity, ale po co?
    out[1] = iss(TUREAD,  'r');
    out[2] = iss(TUWRITE, 'w');
    out[3] = iss(TUEXEC,  'x');
    out[4] = iss(TGREAD,  'r');
    out[5] = iss(TGWRITE, 'w');
    out[6] = iss(TGEXEC,  'x');
    out[7] = iss(TOREAD,  'r');
    out[8] = iss(TOWRITE, 'w');
    out[9] = iss(TOEXEC,  'x');
    out[10] = 0;
#undef iss
    return out;
}

int
is_in_list(char **argv, const char *name)
{
    for (;*argv; argv++) {
        if (strcmp(name, *argv) == 0)
            return 1;
    }
    return 0;
}

void
write_header(FILE *archive, struct tentry *entry, const char *uname,
    const char *gname, uid_t uid, gid_t gid, size_t size, mode_t mode,
    time_t mtime)
{
    int i;
    unsigned int u = 0;
    unsigned char *b = (unsigned char*)entry;

    memcpy(entry->magic, TMAGIC, TMAGLEN);
    memcpy(entry->version, TVERSION, 2);
    strcpy(entry->uname, uname);
    strcpy(entry->gname, gname);
    WRITE_NUM(entry->mode, mode & 0xfff);
    WRITE_NUM(entry->uid, uid);
    WRITE_NUM(entry->gid, gid);
    WRITE_NUM(entry->size, size);
    WRITE_NUM(entry->mtime, mtime);
    memset(entry->chksum, ' ', sizeof(entry->chksum));
    for (i = 0; i < 512; i++)
        u += b[i];
    WRITE_NUM(entry->chksum, u);
    fwrite(entry, 512, 1, archive);
} 


/*=============================================================================
 * dopisywanie elementów do archiwum
 *
 * Do dorzucania s³u¿y append_to_arch, który w wykorzystuje
 * appennd_file_to_arch gdy dorzucany element to regularny plik
 * oraz append_dir_to_arch gdy to katalog - wtedy jest wzajemna rekursja
 * miêdzy tymi dwoma procedurami.
 */

static void append_dir_to_arch(FILE *, const char *, int, const char *);
static void append_file_to_arch(FILE *, const char *, int, const char *);
static void append_to_arch(FILE *, const char *, int, const char *);

void
append_dir_to_arch(FILE *archive, const char *dirname, int verb, const char *PREFIX)
{
    char path[256];
    DIR *dir = opendir(dirname);
    struct dirent *dent;
    if (dir == NULL) {
        fprintf(stderr, "%s: cannot open directory %s (skipped)\n", tar, dirname);
        return;
    }
    snprintf(path, 256, "%s/", PREFIX);
    while ( (dent = readdir(dir)) ) {
        if (strcmp(dent->d_name,".") == 0 || strcmp(dent->d_name,"..") == 0)
            continue;
        append_to_arch(archive, dent->d_name, verb, path);
    }
    closedir(dir);
    
}

void
append_file_to_arch(FILE *archive, const char *name, int verb, const char *PREFIX)
{
    FILE *file = fopen(name, "r");
    if (file == NULL) {
        fprintf(stderr, "%s: cannot open file %s\n", tar, name);
        exit(-1);
    }
    while (!feof(file)) {
        char buf[512];
        memset(buf, 0, 512);
        int n = fread(buf, 1, 512, file);
        if (n > 0) {
           fwrite(buf, 512, 1, archive);
        }
    }
    fclose(file);
}


///@todo obs³uga linków (czekamy na readlink(2))
void
append_to_arch(FILE *archive, const char *file, int verb, const char *PREFIX)
{
    struct tentry entry;
    struct stat st;
//    char *linkname;
    int namelen;
    char path[256];
    memset(&entry, 0, sizeof(entry));
    snprintf(path, 256, "%s%s", PREFIX, file);
    if ( lstat(path, &st) == -1 ) {
        fprintf(stderr, "%s: cannot stat file %s\n", tar, file);
        exit(-1);
    }
    if (verb) printf("a %s\n", path);
    namelen = strlen(path);
    if (namelen < sizeof(entry.name)) {
        strcpy(entry.name, path);
        entry.prefix[0] = 0;
    } else 
    if (namelen < sizeof(entry.name) + sizeof(entry.prefix)) {
        int i;
        int last = -1;
        for (i = 0; i < sizeof(entry.prefix)-1; i++) {
            if (path[i] == '/') last = i;
        }
        if (last == -1) {
            fprintf(stderr, "%s: bad file name %s (skipped)\n", tar, file);
            return;
        }
        memcpy(entry.prefix, path, last);
        entry.prefix[last] = 0;
        memcpy(entry.name, path + last, namelen - last);
        entry.name[namelen-last] = 0;
    }
    if (S_ISDIR(st.st_mode)) {
        strcpy(entry.size, "0");
        if (0 && entry.name[strlen(entry.name)-1] != '/') 
            strcat(entry.name,"/");
        entry.typeflag = DIRTYPE;
        write_header(archive, &entry, "root", "wheel", st.st_uid, st.st_gid,
            0, st.st_mode, st.st_mtime);
        append_dir_to_arch(archive, path, verb, path);
    } else
    if (S_ISREG(st.st_mode)) {
        entry.typeflag = REGTYPE;
        snprintf(entry.size, 12, "%011o", st.st_size); //XXX
        write_header(archive, &entry, "root", "wheel", st.st_uid, st.st_gid,
            st.st_size, st.st_mode, st.st_mtime);
        append_file_to_arch(archive, path, verb, path);
    }

}

/*=============================================================================
 * rozpakowywanie elementów z archiwum (i testowanie)
 */

static void extract_from_arch(FILE *, char **, int, int, int, int);
static int is_zero(const char *buf);
static void progressbar(int percent, const char *fmt, ...);

int
is_zero(const char *buf)
{
    int i;
    for (i = 0; i < 512; i++)
        if (buf[i]) return 0;
    return 1;
}

void
progressbar(int percent, const char *fmt, ...)
{   
    enum {
        FIX = 1000
    };
    va_list ap;
    int i;
    int win;
    char buf[256];
    char *msg = buf;
    win = 80;   // czekamy na jaki¶ TIOCGETWINSZ czy tam co¶ innego
    int size = win;
    int used = ((win*FIX)/100 * percent) / FIX;
    va_start(ap, fmt);
    vsnprintf(buf, win+1, fmt, ap);
    
    printf("\033[2K\r\033[7m");
    for (i = 0; i < used; i++) {
        if (*msg) {
            fputc(*msg, stdout);
            msg++;
        } else {
            fputc(' ', stdout);
        }
    }
    printf("\033[0m");
    for (; i < size; i++) {
        if (*msg) {
            fputc(*msg, stdout);
            msg++;
        } else {
            fputc(' ', stdout);
        }
    }
    fflush(stdout);
    printf("\r");
}

void
extract_from_arch(FILE *archive, char **names, int verb, int everb, int blocks,
    int t)
{
    int lastline = 0;
    int zeros = 2;
    char buf[512];
    struct tentry *entry = (struct tentry *) buf;
    char path[256];
    int fbs = 0;
    int size = 0;
    int tsize = 0;
    int i;
    FILE *file = NULL;
    for (i = 0; i < blocks-2 && zeros; i++) {
        if (everb) fflush(stdout);
        if (fread(buf, 512, 1, archive) != 1) {
            fprintf(stderr, "%s: unexpected end of archive\n", tar);
            exit(-1);
        }
        if (fbs == 0) {
            if (file) {
                fclose(file);
                file = NULL;
            }
            if (everb && lastline) {
                printf("\r\033[2Kx %s\n", path);
            }
            if (is_zero(buf)) {
                zeros--;
                continue;
            };
            if (entry->prefix[0]) 
                snprintf(path, 256, "%s/%s", entry->prefix, entry->name);
                else snprintf(path, 256, "%s", entry->name);
            if (strncmp(entry->magic, TMAGIC, 5) != 0 
                || (strncmp(entry->version, TVERSION, 2)
                && strncmp(entry->version, TGNUVERSION, 2))) {
                fprintf(stderr, "%s: invalid format, it is USTAR?\n", tar);
                exit(-1);
            }
            size = READ_NUM(entry->size);
            tsize = size;
            fbs = (size+511)/512;
            if (names != NULL && !is_in_list(names,path)) {
                continue;
            }
            if (t) {
                if (verb)
                    printf("%6s %7s %7s %10u !time! %s\n",
                        mode2str(entry->mode,entry->typeflag), entry->uname,
                         entry->gname, READ_NUM(entry->size), /*!time!*/
                         path);
                    else printf("%s\n", path);
            } else  {
                if (verb) {
                    printf("x %s %s", path, (everb)? "": "\n");
                }
                if (entry->typeflag == REGTYPE) {
                    lastline = 1;
                    file = fopen(path, "w");
                    if (file == NULL) {
                        fprintf(stderr, "%s: cannot create file %s\n",
                            tar, path);
                        exit(-1);
                    }
                } else
                if (entry->typeflag == DIRTYPE) {
                    lastline = 0;
                    if (mkdir(path, READ_NUM(entry->mode)) && errno != EEXIST){
                        fprintf(stderr, "%s: cannot create dir %s\n", tar,path);
                        exit(-1);
                    }
                    size = 1;
                }
                if (everb && !lastline) printf("\n");
            }
        } else {
            if (file) {
                if (everb && lastline) {
                    int total = ((tsize-size)*100)/tsize*100;
                    total/=100;
                    progressbar(total, "x %s [%u%%]", path, total);
                }
                fwrite(buf, (fbs==1)? size : 512, 1, file);
            }
            size -= 512;
            fbs--;
        }
    }
    if (everb && lastline) printf("\r\033[2Kx %s\n", path);
}

/*=============================================================================
 * program w³a¶ciwy
 */

int main(int argc, char **argv);

int operate(int, char **, int, const char *, const char *, int, int);

#ifdef __Impala__
static char *xoptarg = "/mnt/fd0/impala/dist.tar";
static int xoptind = 3;

static int
xgetopt(int argc, char * const argv[], const char *optstring)
{
    static char res[] = { 'x', 'v', 'V', 'f', -1 };
    static int i = 0;
    return res[i++];
}

#define getopt xgetopt
#define optind xoptind
#define optarg xoptarg
#endif

int
operate(int argc, char **argv, int oper, const char *file, const char *mode,
    int verb, int everb)
{
    FILE *archive;
    struct stat st;

    if (!file) {
        fprintf(stderr, "%s: forgot to specify archive file\n", tar);
        return -1;
    }
    if (oper == APPEND) {
        fprintf(stderr, "%s: operation -r not supported\n", tar);
        return -1;
    }
    if (oper == CREATE && argc == 0) {
        fprintf(stderr, "%s: no files\n", tar);
        return -1;
    }
    if (oper != CREATE) {
        if (stat(file, &st)) {
            fprintf(stderr, "%s: cannot stat(2) on file %s\n", tar, file);
            return -1;
        }
    }
    archive = fopen(file, mode);
    if (!archive) {
        ///@todo bledy
        fprintf(stderr, "%s: cannot open archive\n", tar);
        return -1;
    }
    if (oper == CREATE) {
        int i;
        for (i = 0; i < argc; i++) {
            append_to_arch(archive, argv[i], verb, "");
        }
        char block[512];
        memset(block, 0, 512);
        fwrite(block, 512, 1, archive);
        fwrite(block, 512, 1, archive);
    } else
    if (oper == EXTRACT || oper == TEST) {
        int bs = st.st_size / 512;
        if (st.st_size % 512) {
            fprintf(stderr, "%s: strange file size (ignored error)\n", tar);
        }
        extract_from_arch(archive, (argc==0)? NULL: argv, verb, everb, bs,
            oper==TEST);
    }
    fclose(archive);
    return 0;
}


int
main(int argc, char **argv)
{
    int oper = 0;
    char ch;
    int verb = 0;
    int everb = 0;
    const char *mode = NULL;
    const char *file = NULL;
    char historic[10];
    tar = argv[0];
    if ( getenv("GZIP") ) gzip = getenv("GZIP");
    // historic
    if (argc > 1 && argv[1][0] != '-') {
        historic[0] = '-';
        historic[1] = 0;
        strncat(historic, argv[1], sizeof(historic)-3);
        argv[1] = historic;
    }
    while ( (ch = getopt(argc, argv, "hrtxcf:Vv")) != -1 ) 
    switch (ch) {
        case 't':
            if (oper) {
                fprintf(stderr, "%s: bad options\n", tar);
                return -1;
            }
            oper = TEST;
            mode = "r";
            break;
        case 'V':
            everb = 1;
        case 'v':
            verb = 1;
            break;
        case 'r':
            if (oper) {
                fprintf(stderr, "%s: bad options\n", tar);
                return -1;
            }
            oper = APPEND;
            break;
        case 'x':
            if (oper) { 
                fprintf(stderr, "%s: bad options\n", tar);
                return -1;
            }
            oper = EXTRACT;
            mode = "r";
            break;
        case 'c':
            if (oper) {
                fprintf(stderr, "%s: bad options\n", tar);
                return -1;
            }
            oper = CREATE;
            mode = "w";
            break;
        case 'f':
            file = optarg;
            break;
        case 'h':
            if (oper) {
                fprintf(stderr, "%s: bad options\n", tar);
                return -1;
            }
            oper = USAGE;
            break;
        default:
            fprintf(stderr, "%s: bad usage\n", tar);
            return -1;
    }
    argv += optind;
    argc -= optind;

    if (oper == USAGE) {
        printf("tar keys files..\n");
        printf(" keys: c (create) r (add/replace) t (test) x (extract)\n");
        printf(" supported are only tar archives in POSIX format (with GNU 'POSIX' signature)\n");
        return 0;
    }

    if (!(oper == TEST || oper == EXTRACT || oper == APPEND || oper == CREATE)) {
        fprintf(stderr, "%s: operation not specified\n", tar);
        return -1;
    }
    return operate(argc, argv, oper, file, mode, verb, everb);
}
