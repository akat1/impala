/* Impala C Library
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
#ifndef __UNISTD_H
#define __UNISTD_H

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

//TODO: odkomentowa³em rzeczy wymagane przez ash
//int          access(const char *, int);
//unsigned     alarm(unsigned);
//int          chown(const char *, uid_t, gid_t);
//size_t       confstr(int, char *, size_t);
//int          execl(const char *, const char *, ...);
//int          execle(const char *, const char *, ...);
//int          execlp(const char *, const char *, ...);
//int          execv(const char *, char *const []);
int          execve(const char *, char *const [], char *const []);
//int          execvp(const char *, char *const []);
void         _exit(int);
void         exit(int);
//int          fchown(int, uid_t, gid_t);
//long         fpathconf(int, int);
//int          ftruncate(int, off_t);
char        *getcwd(char *, size_t);
gid_t        getegid(void);
uid_t        geteuid(void);
pid_t        getpgid(pid_t pid);
//int          getgroups(int, gid_t []);
int          gethostname(char *, size_t);
//char        *getlogin(void);
//int          getlogin_r(char *, size_t);
int          getopt(int, char * const [], const char *);
pid_t        getpgrp(void);
int          isatty(int);
//long         pathconf(const char *, int);
//int          pause(void);
int          pipe(int [2]);
//ssize_t      readlink(const char *restrict, char *restrict, size_t);
//int          setegid(gid_t);
//int          seteuid(uid_t);
int          setpgid(pid_t, pid_t);
//pid_t        setsid(void);
unsigned int   sleep(unsigned int);
//long         sysconf(int);
pid_t        tcgetpgrp(int);
int          tcsetpgrp(int, pid_t);
//char        *ttyname(int);
//int          ttyname_r(int, char *, size_t);
int eaccess(const char *pathname, int mode);    //chc±, to dajemy

#define R_OK (1<<0)
#define W_OK (1<<1)
#define X_OK (1<<2)
#define F_OK (1<<3)

#define STDIN_FILENO   0
#define STDOUT_FILENO  1
#define STDERR_FILENO  2


ssize_t read(int fd, void *buf, size_t l);
ssize_t write(int fd, const void *buf, size_t l);
gid_t getgid(void);
pid_t getpid(void);
pid_t getppid(void);
uid_t getuid(void);
int close(int fd);
int setuid(uid_t uid);
int setgid(gid_t gid);
off_t lseek(int fd, off_t offset, int whence);
pid_t fork(void);
int link(const char *oldpath, const char *newpath);
int symlink(const char *oldpath, const char *newpath);
int unlink(const char *pathname);
int rmdir(const char *pathname);
int dup(int fd);
int chdir(const char *path);
int dup2(int oldfd, int newfd);
int getdents(int fd, dirent_t *data, size_t count);

//getopt

extern char *optarg;
extern int optind, opterr, optopt, optreset;

#include <sys/select.h>


#endif
