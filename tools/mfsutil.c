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

#define _POSIX_C_SOURCE 200112L

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sysexits.h>
#include <dirent.h>
#include "mfs.h"


typedef struct node node_t;
struct node {
    int         id;
    char       *filename;
    size_t      size;
    int         type;
    int         attr;
    node_t     *parent;
    node_t     *next;
    node_t     *childs;
    char       *fullpath;
    int         childcnt;
};

#define NEW_NODE() ((node_t*)xmalloc(sizeof(node_t)))
#define STREQ(s,e) (strcmp(s,e) == 0)
const char *image_filename = NULL;
const char *symbol_name = "mfs_image";
const char *c_filename = NULL;
node_t *root_node = NULL;
int     node_id = 0;

void *
xmalloc(size_t s)
{
    void *x = malloc(s);
    if (x == NULL) {
        perror("cannot allocate memory");
        abort();
    }
    return x;
}

node_t *
node_newdir(node_t *p, const char *fname)
{
    node_t *d = NEW_NODE();
    d->id = node_id++;
    d->filename = strdup(fname);
    d->type = MFS_TYPE_DIR;
    d->parent = p;
    d->childs = NULL;
    d->childcnt = 0;
    d->size = 0;
    if (p == NULL) {
        d->next = NULL;
    } else {
        p->childcnt++;
        d->next = p->childs;
        p->childs = d;
    }
    return d;
}

void
node_newfile(node_t *dir, const char *fname, size_t s, const char *path)
{
    node_t *f = NEW_NODE();
    f->id = node_id++;
    f->filename = strdup(fname);
    f->type = MFS_TYPE_REG;
    f->parent = dir;
    f->childs = NULL;
    f->childcnt = 0;
    f->size = s;
    dir->childcnt++;
    f->next = dir->childs;
    f->fullpath = (char*)xmalloc(strlen(fname)+2+strlen(path));
    sprintf(f->fullpath, "%s/%s", path, fname);
    dir->childs = f;
}

int min(int a, int b)
{
    if(a<b) return a;
    return b;
}

const char *badnames[] = {
    ".",
    "..",
    ".svn",
    "CVS",
    NULL
};

int
is_badname(const char *e)
{
    const char **xs = badnames;
    for (int i = 0; *xs != NULL; xs++) {
        if (STREQ(e,*xs)) return 1;
    }
    return 0;
}


static int
xscandir(node_t *dirn, const char *path)
{
    char newpath[MFS_MAX_PATH];
    DIR *dird = opendir(".");
    struct stat entrystat;
    struct dirent *entry;
    while ( (entry = readdir(dird)) != NULL) {
        if (is_badname(entry->d_name))
            continue;
        if (stat(entry->d_name, &entrystat) == -1) {
            perror("stat");
            return -1;
        }
        if (S_ISDIR(entrystat.st_mode)) {
            printf("+dir %s/%s\n", path, entry->d_name);
            node_t *dnext = node_newdir(dirn, entry->d_name);
            if (chdir(entry->d_name) == -1) {
                perror("cannot change directory");
                return -1;
            }
            snprintf(newpath, MFS_MAX_PATH, "%s/%s", path, entry->d_name);
            xscandir(dnext, newpath);
            chdir("..");
        } else
        if (S_ISREG(entrystat.st_mode)) {
            printf("+file %s/%s\n", path, entry->d_name);
            node_newfile(dirn, entry->d_name, entrystat.st_size, path);
        } else {
            fprintf(stderr, "-skip %s/%s\n", path, entry->d_name);
        }
    }
    return 0;
}


static int
scan(const char *path)
{
    printf("Scanning %s\n", path);
    if (chdir(path) == -1) {
        perror("cannot change directory");
        return -1;
    }
    root_node = node_newdir(NULL, "");
    return xscandir(root_node, "");
}

static void
fillptable(node_t **ptable, node_t *now)
{
    ptable[now->id] = now;
    now = now->childs;
    while(now) {
        fillptable(ptable, now);
        now = now->next;
    }
}


static int
build(const char *arg)
{
    mfs_header_t header;
    int ncount = node_id;
    mfs_data_entry_t ntable[ncount];
    node_t *npointers[ncount];
    #define ID_OFF(id) (id*sizeof(mfs_data_entry_t) + sizeof(header))
    node_t *n = root_node;
    fillptable(npointers, root_node);
    int data_off = sizeof(header) + ncount * sizeof(mfs_data_entry_t);
    for(int i=0; i<ncount; i++) {
        node_t *p = npointers[i];
        strncpy(ntable[i].name, p->filename, MFS_MAX_FNAME);
        ntable[i].size = p->size;
        ntable[i].type = p->type;
        ntable[i].attr = p->attr;
        ntable[i].data_off = p->size?data_off:0;
        data_off += p->size;
        ntable[i].parent_id = p->parent?p->parent->id+1:0;
        ntable[i].child_id = p->childs?p->childs->id+1:0;
        ntable[i].next_id = p->next?p->next->id+1:0;
    }
    
    header.magic0 = MFS_MAGIC0;
    header.magic1 = MFS_MAGIC1;
    header.items = ncount;
    chdir("..");
    FILE *f = fopen(image_filename, "w+");
    chdir(arg);
    fwrite(&header, sizeof(header), 1, f);
    fwrite(&ntable, sizeof(mfs_data_entry_t), ncount, f);
    for(int i=0; i<ncount; i++) {
        node_t *p = npointers[i];
        int rest = p->size;
        if(p->type != MFS_TYPE_REG) continue;
        FILE *f2 = fopen(p->fullpath+1, "r");
        while(rest > 0) {
            char buf[1024];
            int c=fread(buf, 1, min(1024, rest), f2);
            fwrite(buf, 1, c, f);
            rest-=c;
        }
        fclose(f2);
    }
    fclose(f);
    #undef ID_OFF
}

static int
image_init()
{
    return 0;
}

static int
c_init()
{
    return 0;
}


int
main(int argc, char **argv )
{
    char ch;
    while ( (ch = getopt(argc, argv, "hi:c:s:")) != -1 )
    switch (ch) {
        case 'h':
            break;
        case 'i':
            image_filename = optarg;
            break;
        case 's':
            symbol_name = optarg;
            break;
        case 'c':
            c_filename = optarg;
            break;
        default:
            fprintf(stderr, "bad usage [%c]\n", ch);
            return EX_USAGE;
    }
    argc -= optind;
    argv += optind;
    if (argc != 1) {
        fprintf(stderr, "type root directory\n");
        return EX_USAGE;
    }
    if (!c_filename && !image_filename) {
        fprintf(stderr, "bad usage, use -i or -c, or both.\n");
        return EX_USAGE;
    }
    if (image_init() == -1) return EX_OSERR;
    if (c_init() == -1) return EX_OSERR;
    if (scan(argv[0]) == -1) return EX_OSERR;
    if (build(argv[0]) == -1) return EX_SOFTWARE;
    return EX_OK;
}

