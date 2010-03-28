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

/*
 * Implementacja programu ls:
 * http://www.opengroup.org/onlinepubs/000095399/utilities/ls.html
 *
 * Obsługiwane opcje: -a, -i, -l, -1, -F
 * Rozszerzenia: -G (wyłącza kolorowanie)
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


typedef void print_f(const char *name, const struct stat *st);

struct {
    char         many;
    char         all;
    char         serial;
    char         graph;
    char         output_is_tty;
    char         ender;
    print_f     *print;
} progopt;

static const char * mode2str(mode_t m);
static const char *print_name(const char *name, const struct stat *st);
static void print_dir(const char *file);
static void print_file(const char *file);
static void print_entry(const char *file, const struct stat *st);
static void short_entry(const char *file, const struct stat *st);
static void long_entry(const char *file, const struct stat *st);
static void print_path(const char *file);
static const char *print_serial(const struct stat *st);
static const char *print_gid(const struct stat *st);
static const char *print_uid(const struct stat *st);

int main(int argc, char **argv);

#define error(fmt, a...) fprintf(stderr, "%s: " fmt "\n", PROG, ## a)

const char *PROG = "ls";

const char *
mode2str(mode_t m)
{
#define iss(bit, ch) ((m&bit)? ch : '-')
    static char out[11];
    out[0] = S_ISBLK(m)? 'b' :
                S_ISCHR(m)? 'c':
                S_ISDIR(m)? 'd':
                S_ISFIFO(m)? 'f':
                S_ISLNK(m)? 'l':
                S_ISSOCK(m)? 's':
                S_ISREG(m)? '-':
                '?';
    out[1] = iss(S_IRUSR,  'r');
    out[2] = iss(S_IWUSR,  'w');
    out[3] = iss(S_IXUSR,  'x');
    out[4] = iss(S_IRGRP,  'r');
    out[5] = iss(S_IWGRP,  'w');
    out[6] = iss(S_IXGRP,  'x');
    out[7] = iss(S_IROTH,  'r');
    out[8] = iss(S_IWOTH,  'w');
    out[9] = iss(S_IXOTH,  'x');
    out[10] = 0;
#undef iss
    return out;
}

void
print_path(const char *file)
{
    struct stat st;
    lstat(file, &st);
    if (S_ISDIR(st.st_mode))
        print_dir(file);
        else print_file(file);
}

void
print_file(const char *file)
{
    struct stat st;
    lstat(file, &st);
    if (!S_ISDIR(st.st_mode)) {
        if (progopt.many == 2) {
            printf("\n");
        } else {
            progopt.many = 2;
        }
        print_entry(file, &st);
    }
}

void
print_dir(const char *file)
{
    struct stat st;
    lstat(file, &st);
    if (S_ISDIR(st.st_mode)) {
        if (progopt.many) {
            if (progopt.many == 2) printf("\n");
            printf("%s:\n", file);
            progopt.many = 2;
        }
        DIR *dir = opendir(file);
        if (!dir) {
            error("cannot open directory %s", file);
            exit(-1);
        }
        struct dirent *dent;
        while ( (dent = readdir(dir)) ) {
            char path[256];
            struct stat st2;
            snprintf(path, sizeof(path), "%s/%s", file, dent->d_name);
            if (stat(path, &st2)) {
                error("cannot stat(2) path %s", path);
                exit(-1);
            }
            print_entry(dent->d_name, &st2);
        }
        closedir(dir);
    }
}

const char *
print_name(const char *name, const struct stat *st)
{
    const char *ender = "";
    const char *xender = ender;
    static char buf[50];
    const char *graph_enter = "";
    const char *xgraph_enter = graph_enter;
    const char *graph_leave = (progopt.graph)? "\033[0m" : "";

    if (S_ISDIR(st->st_mode)) {
        graph_enter = "\033[34;40;1m";
        ender = "/";
    } else
    if (S_ISLNK(st->st_mode)) {
        graph_enter = "\033[33;40;1m";
        ender = "@";
    } else
    if (S_ISFIFO(st->st_mode)) {
        graph_enter = "\033[35;40;1m";
        ender = "|";
    } else
    if (S_ISSOCK(st->st_mode)) {
        graph_enter = "\033[36;40;1m";
        ender = "=";
    } else
    if (S_ISCHR(st->st_mode)) {
        graph_enter = "\033[31;40;1m";
    } else
    if (S_ISBLK(st->st_mode)) {
        graph_enter = "\033[31;40;1m";
    } else
    if (S_ISREG(st->st_mode)) {
        if (st->st_mode & (S_IXUSR|S_IXGRP|S_IXOTH)) {
            graph_enter = "\033[32;40;1m";
            ender = "*";
        } else {
            graph_enter = "";
            ender = "";
        }
    }
    if (progopt.ender) xender = ender;
    if (progopt.graph) xgraph_enter = graph_enter;
    snprintf(buf, sizeof(buf), "%s%s%s%s", xgraph_enter, name,
        graph_leave, xender);
    return buf;
}

const char *
print_serial(const struct stat *st)
{
    static char buf[20];
    if (!progopt.serial) return "";
    snprintf(buf, sizeof(buf), "%6u ", st->st_ino);
    return buf;
}

const char *
print_uid(const struct stat *st)
{
    static char buf[20];
    snprintf(buf, sizeof(buf), "%u", st->st_uid);
    return buf;
}

const char *
print_gid(const struct stat *st)
{
    static char buf[20];
    snprintf(buf, sizeof(buf), "%u", st->st_gid);
    return buf;
}

void
short_entry(const char *file, const struct stat *st)
{
    printf("%s%s\n", print_serial(st), print_name(file, st));
}

void
long_entry(const char *file, const struct stat *st)
{
    printf("%s%s %2u %7s %7s %10u %s\n",
        print_serial(st),
        mode2str(st->st_mode),
        st->st_nlink,
        print_uid(st),
        print_gid(st),
        st->st_size,
        print_name(file, st)
     );
}

void
print_entry(const char *file, const struct stat *st)
{
    if ( file[0] == '.' && !progopt.all) return;
    progopt.print(file, st);
}

int
main(int argc, char **argv)
{
    char ch;
    progopt.all = 0;
    progopt.serial = 0;
    progopt.output_is_tty = isatty(1);
    progopt.graph = (progopt.output_is_tty)? 1 : 0;
    progopt.print = short_entry;
    progopt.ender = 0;
    while ( (ch = getopt(argc, argv, "1ailFG")) != -1 )
    switch (ch) {
        case '1':
            break;
        case 'a':
            progopt.all = 1;
            break;
        case 'i':
            progopt.serial = 1;
            break;
        case 'l':
            progopt.print = long_entry;
            break;
        case 'F':
            progopt.ender = 1;
            break;
        case 'G':
            progopt.graph = 0;
            break;
    }
    if (!progopt.output_is_tty) progopt.graph = 0;
    argv += optind;
    argc -= optind;
    if (argc < 2) {
        progopt.many = 0;
        const char *name = (argc==0) ? "." : argv[0];
        print_path(name);
    } else {
        progopt.many = 1;
        // zgodnie z standardem, najpierw drukowane są niekatalogi
        for (int i = 0; i < argc; i++) {
            print_file(argv[i]);
        }
        for (int i = 0; i < argc; i++) {
            print_dir(argv[i]);
        }
    }
    return 0;
}

