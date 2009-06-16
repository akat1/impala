/*
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
 * $Id: TEMPLATE.c 418 2009-06-14 00:48:52Z takeshi $
 */

#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/syscall.h>
#include <sys/vm.h>

typedef struct getcwd_args getcwd_args_t;
struct getcwd_args {
    char *buf;
    size_t size;
};

int sc_getcwd(thread_t *p, syscall_result_t *r, getcwd_args_t *args);

int
sc_getcwd(thread_t *t, syscall_result_t *r, getcwd_args_t *args)
{
    int err = 0;
    if((err = vm_is_avail((vm_addr_t)args->buf, args->size)))
        return err;
    vnode_t *cd = t->thr_proc->p_curdir;    //nie boimy siê trzymaæ ten wska¼nik
    vnode_t *tmp = NULL;
    str_ncpy(args->buf, "/", args->size);
    return -EOK;
    lkp_state_t pc;
    pc.flags = LKP_GET_PARENT;
    pc.max_link_cnt = 10;
    pc.path = "..";
    pc.now = pc.path;
    ///@todo Zaimplementowaæ getcwd
    //Chwilowo wstrzymujê prace nad getcwd. Aby go zrealizowaæ, potrzebne jest
    // jakie¶ rozszerzenie VFS... np. VOP_NAME() :)...
    // z tym, ¿e vnode nie musi mieæ unikatowej nazwy.. a mo¿e nawet nie mieæ
    // ¿adnej ;) (VOP_ANYNAME?) Mo¿na by u¿ywaæ ostatnio VOP_LOOKUPowanej nazwy
    // wskazuj±cej na ten vnode.
    //
    // A tak na prawdê, to póki co u nas chyba wszystko ma dok³adnie jedn± nazwê
    // Wiêc na razie nie by³o by wielkiego problemu z tak± funkcj±...
    panic("Use of unimplemented syscall getcwd");
    while(cd) {
        VOP_LOOKUP(cd, &tmp, &pc);
        pc.now = pc.path;
    }
    return -EOK;
}


