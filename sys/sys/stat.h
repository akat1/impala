/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://bitbucket.org/wieczyk/impala/
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
#ifndef __SYS_STAT_H
#define __SYS_STAT_H

#include <sys/types.h>
#include <sys/time.h>

struct stat {
    dev_t       st_dev;
    ino_t       st_ino;
    mode_t      st_mode;
    nlink_t     st_nlink;
    uid_t       st_uid;
    gid_t       st_gid;
    dev_t       st_rdev;
    off_t       st_size;
    blksize_t   st_blksize;
    blkcnt_t    st_blocks;
    timespec_t  st_atimespec;
    timespec_t  st_mtimespec;
    timespec_t  st_ctimespec;
};

#define st_atime st_atimespec.tv_sec
#define st_mtime st_mtimespec.tv_sec
#define st_ctime st_ctimespec.tv_sec

#define S_TMASK    0770000

#define S_IFMT     0170000
#define S_IFSOCK   0140000      ///< gniazdko
#define S_IFLNK    0120000      ///< odnośnik
#define S_IFREG    0100000      ///< plik regularny
#define S_IFBLK    0060000      ///< urz. blokowe
#define S_IFDIR    0040000      ///< katalog
#define S_IFCHR    0020000      ///< urz. znakowe
#define S_IFIFO    0010000      ///< potok
#define S_ISUID    0004000      ///< 
#define S_ISGID    0002000      ///<
#define S_ISVTX    0001000      ///<
#define S_IRWXU    00700        ///< u=rwx
#define S_IRUSR    00400        ///< u+r
#define S_IWUSR    00200        ///< u+w
#define S_IXUSR    00100        ///< u+x
#define S_IRWXG    00070        ///< g=rwx
#define S_IRGRP    00040        ///< g+r
#define S_IWGRP    00020        ///< g+w
#define S_IXGRP    00010        ///< g+x
#define S_IRWXO    00007        ///< o=rwx
#define S_IROTH    00004        ///< o+r
#define S_IWOTH    00002        ///< o+w
#define S_IXOTH    00001        ///< o+x

#define S_ISREG(m) ((m&S_TMASK)==S_IFREG)
#define S_ISDIR(m) ((m&S_TMASK)==S_IFDIR)
#define S_ISCHR(m) ((m&S_TMASK)==S_IFCHR)
#define S_ISBLK(m) ((m&S_TMASK)==S_IFBLK)
#define S_ISFIFO(m) ((m&S_TMASK)==S_IFIFO)
#define S_ISSOCK(m) ((m&S_TMASK)==S_IFSOCK)
#define S_ISLNK(m) ((m&S_TMASK)==S_IFLNK)

#define STAT_NORMAL 0
#define STAT_LINK   1

#ifdef __KERNEL

#define VATYPE_TO_SMODE(t) (((t)==VNODE_TYPE_DIR)?S_IFDIR:\
                           (((t)==VNODE_TYPE_REG)?S_IFREG:\
                           (((t)==VNODE_TYPE_DEV)?S_IFBLK:\
                           (((t)==VNODE_TYPE_LNK)?S_IFLNK:0))))

#else

int chmod(const char *path, mode_t mode);
int fchmod(int fd, mode_t mode);
int stat(const char *path, struct stat *buf);
int fstat(int filedes, struct stat *buf);
int lstat(const char *path, struct stat *buf);
mode_t umask(mode_t mask);
int mkdir(const char *name, mode_t m);

#endif /* __KERNEL */

#include <sys/file.h>

#endif
