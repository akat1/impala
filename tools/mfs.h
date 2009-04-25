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
#ifndef __SYS_FS_MFS_H
#define __SYS_FS_MFS_H

typedef struct mfs_entry mfs_entry;
typedef struct mfs_data_entry mfs_data_entry_t;

enum {
    MFS_MAX_PATH        = 256,
    MFS_MAX_FILENAME    = 100
};

enum {
    MFS_TYPE_REG    = 0,
    MFS_TYPE_DIR    = 1,
    MFS_TYPE_LNK    = 2,
    MFS_TYPE_XXX    = 3
};

enum {
    MFS_ATTR_OWNER_R  = 400,
    MFS_ATTR_OWNER_W  = 0200,
    MFS_ATTR_OWNER_X  = 0100,
    MFS_ATTR_GROUP_R  =  040,
    MFS_ATTR_GROUP_W  =  020,
    MFS_ATTR_GROUP_X  =  010,
    MFS_ATTR_OTHER_R  =   04,
    MFS_ATTR_OTHER_W  =   02,
    MFS_ATTR_OTHER_X  =   01
};

struct mfs_data_entry {
    char    name[MFS_MAX_PATH];
    size_t  size;
    int     type;
    int     attr;
};

#ifdef __KERNEL
struct mfs_entry {
    char        name[MFS_MAX_FILENAME];
    size_t      size;
    void       *data_addr;
    int         type;
    list_t      entries;
    list_node_t entry;
};
#endif


#endif
