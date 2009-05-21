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

enum {
    DEV_BLOCK = 512,
    MAX_FILENAME = 100,
    MAX_PATH = 255,
};

enum {
    MFS_TYPE_REG,
    MFS_TYPE_SPEC,
    MFS_TYPE_DIR
};

typedef struct node node_t;
struct node {
    char       *filename;
    size_t      size;
    int         type;
    node_t     *parent;
    node_t     *next;
    node_t     *childs;
    int         childcnt;
};
#define NEW_NODE() ((node_t*)xmalloc(sizeof(node_t)))
#define STREQ(s,e) (strcmp(s,e) == 0)
const char *image_filename = NULL;
const char *symbol_name = "mfs_image";
const char *c_filename = NULL;
node_t *root_node = NULL;

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
}

void
node_newfile(node_t *dir, const char *fname, size_t s)
{
    node_t *f = NEW_NODE();
    f->filename = strdup(fname);
    f->type = MFS_TYPE_REG;
    f->parent = dir;
    f->childs = NULL;
    f->childcnt = 0;
    f->size = s;
    dir->childcnt++;
    f->next = dir->childs;
    dir->childs = f;
}

static int
xscandir(node_t *dirn, const char *path)
{
    char newpath[MAX_PATH];
    DIR *dird = opendir(".");
    struct stat entrystat;
    struct dirent *entry;
    while ( (entry = readdir(dird)) != NULL) {
        if (STREQ(entry->d_name,".") || STREQ(entry->d_name,".."))
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
            snprintf(newpath, MAX_PATH, "%s/%s", path, entry->d_name);
            xscandir(dnext, newpath);
            chdir("..");
        } else
        if (S_ISREG(entrystat.st_mode)) {
            printf("+file %s/%s\n", path, entry->d_name);
            node_newfile(dirn, entry->d_name, entrystat.st_size);
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


static int
build()
{
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

