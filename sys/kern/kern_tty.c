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
 * $Id: kern_console.c 281 2009-05-28 16:04:04Z takeshi $
 */

#include <fs/devfs/devfs.h>
#include <sys/types.h>
//#include <sys/console.h>
#include <sys/termios.h>
#include <sys/kmem.h>
#include <sys/device.h>
#include <sys/string.h>
#include <sys/signum.h>
#include <sys/thread.h>
#include <sys/errno.h>
#include <sys/utils.h>
#include <sys/ioctl.h>
#include <sys/proc.h>
#include <sys/uio.h>
#include <sys/vm.h>
#include <machine/video.h>
#include <machine/interrupt.h>
#include <machine/pckbd.h>


static d_open_t tty_open;
static d_close_t tty_close;
static d_write_t tty_write;
static d_read_t tty_read;
static d_close_t tty_close;
static d_ioctl_t tty_ioctl;


static devsw_t ttysw = {
    tty_open,
    tty_close,
    tty_ioctl,
    tty_read,
    tty_write,
    nostrategy,
    DEV_TTY,
    "ttyv"
};

static void tty_conf_set_default(termios_t *tconf);
static void tty_erase(tty_t *tty);
static void tty_kill(tty_t *tty);

static cc_t default_cc[NCCS] = { CTRL('d'), '\n', '\b', CTRL('c'), CTRL('u'), 1,
                        CTRL('\\'), CTRL('q'), CTRL('s'), CTRL('z'), 0 };

void
tty_conf_set_default(termios_t *tconf)
{
    tconf->c_lflag = ICANON | ECHO | ISIG | ECHOE | ECHOKE;
    tconf->c_iflag = IXON | ICRNL | BRKINT;
    tconf->c_cflag = CREAD | CS7 | PARENB;
    tconf->c_oflag = OPOST | ONLCR;
    for(int i = 0; i<NCCS; i++)
        tconf->c_cc[i] = default_cc[i];
}

tty_t *
tty_create(const char *name, int unit, void *priv, tty_lowops_t *lops)
{
    tty_t *tty = kmem_alloc(sizeof(tty_t), KM_SLEEP);
    if(!tty)
        return NULL;
    char buf[128];
    if(unit==-1)
        snprintf(buf, 128, name);
    else
        snprintf(buf, 128, name, unit);
    tty->t_private = priv;
    tty->t_session = 0; ///@todo zrobi� to porz�dnie
    tty->t_group = 0;
    tty->t_lowops = lops;
    tty->t_inq = clist_create(MAX_INPUT);
    tty->t_clq = clist_create(MAX_INPUT);
    tty_conf_set_default(&(tty->t_conf));
    devd_t *dev = devd_create(&ttysw, unit, tty);///@todo naprawi�: unit mo�e si� powtarza�
    tty->t_dev = dev;
    devfs_register(buf, dev, 0, 0, 0777);
    return tty;
}


/*========================================================================
 * Plik urz�dzenia: /dev/ttyXX
 */

int
tty_open(devd_t *d, int flags)
{
    proc_t *p = curthread->thr_proc;
    tty_t *tty = d->priv;
    if(!(flags & O_NOCTTY) && (p->p_ctty == NULL)) {
        //ustawiamy terminal steruj�cy procesu
        p->p_ctty = tty;
    }

    return 0;
}

int
tty_read(devd_t *d, uio_t *u, int flags)
{
    tty_t *tty = d->priv;
    size_t need = u->size;
    cc_t *cc = tty->t_conf.c_cc;
    
    if(tty->t_conf.c_lflag & ICANON) {
        if(clist_size(tty->t_inq) == 0) {
            if(flags & O_NONBLOCK)
                return -EAGAIN;
            //sleepq();
        }
        size_t to_go = MIN(need, clist_size(tty->t_inq));
        int i=0;
        char BUF[MAX_INPUT];
        while(clist_size(tty->t_inq) > 0 && to_go-->0) {
            int c = clist_pop(tty->t_inq);
            BUF[i++] = c;
            if(c == NL || c == cc[VEOF] || c == cc[VEOL]) //max 1 linia
                break;
        }
        uio_move(BUF, i, u);
        return i;
    } else {
        int TIME = cc[VTIME], MIN = cc[VMIN];
        if (TIME == 0 && MIN == 0) {
        } else if(TIME > 0 && MIN == 0) {
        } else if(TIME > 0 && MIN > 0) {
        } else if(TIME == 0 && MIN > 0) {
            
        }
    }
    return 0;
}

int
tty_write(devd_t *d, uio_t *u, int flags)
{
    tty_t *tty = d->priv;
    if(u->size == 0)
        return 0;
    char *buf = kmem_alloc(u->size, KM_SLEEP);
    if(!buf)
        return -ENOMEM;
    uio_move(buf, u->size, u);
    for(int i=0; i<u->size; i++)
        tty_output(tty, buf[i]);

    kmem_free(buf);
    return u->size;
}


/////////////////////////////////////////////////////


int
tty_ioctl(devd_t *d, int cmd, uintptr_t param)
{
    int err = 0;
    tty_t *tty = d->priv;
    switch(cmd) {
        case TCGETS: {
            termios_t *tconf = (termios_t*)param;
            if((err = copyout(tconf, &tty->t_conf, sizeof(termios_t))))
                return err;
            break;
        }
        case TCSETS: {
            termios_t *tconf = (termios_t*)param;
            if((err=copyin(&tty->t_conf, tconf, sizeof(termios_t))))
                return err;
            break;
        }
        case TIOCSPGRP:
            ///@todo testy uprawnie�
            if(curthread->thr_proc->p_ctty != tty)
                return -ENOTTY;
            
            tty->t_group = (pid_t) param;
            break;
        case TIOCGPGRP:
            return tty->t_group;
            break;
    }
    return 0;
}

int
tty_close(devd_t *d)
{
    return 0;
}


//=======================================================

void
tty_input(tty_t *tty, int ch)
{
    cc_t *cc = tty->t_conf.c_cc;
    tcflag_t lflag = tty->t_conf.c_lflag;
    if(lflag & ISIG) {
        if(ch == cc[VINTR]) {
            //pgsignal(tty->t_group, SIGINT);
            return;
        } else if(ch == cc[VQUIT]) {
            //pgsignal(tty->t_group, SIGQUIT);
            return;
        } else if(ch == cc[VSUSP]) {
            //pgsignal(tty->t_group, SIGTSTP);
            return;
        }
    }
    if(lflag & ICANON) { //przetwarzanie KILL, ERASE
        if(ch == cc[VERASE]) {
            tty_erase(tty);
            return;
        } else if(ch == cc[VKILL]) {
            tty_kill(tty);
            return;
        } else if(ch == cc[VEOF]) {
            clist_move(tty->t_inq, tty->t_clq);
            return;
        }
        tcflag_t iflag = tty->t_conf.c_iflag;
        if(ch == CR) {
            if(iflag & IGNCR)
                return;
            if(iflag & ICRNL)
                ch = NL;
        } else if(ch == NL) {
            if(iflag & INLCR)
                ch = CR;
        }
        clist_push(tty->t_clq, ch);
        if(lflag & ECHO) {
            tty_output(tty, ch);
        }
        if(ch == NL || ch == cc[VEOL]) {
            clist_move(tty->t_inq, tty->t_clq);
        }
        
    } else { //bez ICANON
        clist_push(tty->t_clq, ch);
    }
}

void
tty_output(tty_t *tty, char c)
{
    //na razie olewamy buforowanie... "may provide a buffering mechanism;"
    tty->t_lowops->tty_write(tty->t_private, &c, 1);
}

/// Canonical processing of ERASE character
void
tty_erase(tty_t *tty)
{
    clist_unpush(tty->t_clq);
    if(tty->t_conf.c_lflag & ECHOE) {
        tty_output(tty, '\b');
        tty_output(tty, ' ');
        tty_output(tty, '\b');
    }
}


/// Canonical processing of KILL character
void
tty_kill(tty_t *tty)
{
    if(tty->t_conf.c_lflag & ECHOK) {
        while(clist_size(tty->t_clq) > 0)
            tty_erase(tty);
    } else
        clist_flush(tty->t_clq);
}

//=========== temp place for clist

clist_t *
clist_create(size_t size)
{
    KASSERT(size>0);
    clist_t *l = kmem_alloc(sizeof(clist_t), KM_SLEEP);
    if(!l)
        return NULL;
    l->buf = kmem_alloc(size, KM_SLEEP);
    if(!l->buf) {
        kmem_free(l);
        return NULL;
    }        
    l->buf_size = size;
    l->beg = l->size = 0;
    l->end = size - 1;
    l->mtx = kmem_alloc(sizeof(mutex_t), KM_SLEEP);
    if(!l->mtx) panic("Jeszcze nie zaimpl..");
    mutex_init(l->mtx, MUTEX_NORMAL);
    return l;
}

void
clist_flush(clist_t *l)
{
    int x = spltty();
    mutex_lock(l->mtx);
    l->beg = l->size = 0;
    l->end = l->buf_size - 1;
    mutex_unlock(l->mtx);
    splx(x);
}

int
clist_size(clist_t *l)
{
    return l->size;
}

void
clist_push(clist_t *l, int ch)
{
    int x = spltty();
    mutex_lock(l->mtx);
    if(l->size == l->buf_size) {
        mutex_unlock(l->mtx);
        return; ///@todo porz�dna obs�uga
    }
    if(l->beg == 0)
        l->beg = l->buf_size;
    l->beg--;
    l->size++;
    l->buf[l->beg] = ch;
    mutex_unlock(l->mtx);
    splx(x);
}

void
clist_unpush(clist_t *l)
{
    int x = spltty();
    mutex_lock(l->mtx);
    if(l->size == 0) {
        mutex_unlock(l->mtx);
        return;
    }
    l->beg++;
    if(l->beg == l->buf_size)
        l->beg = 0;
    l->size--;
    mutex_unlock(l->mtx);
    splx(x);
}

int clist_pop(clist_t *l)
{
    int x = spltty();
    mutex_lock(l->mtx);
    if(l->size == 0) {
        mutex_unlock(l->mtx);
        return 0;
    }
    l->size--;
    int res = l->buf[l->end];
    if(l->end == 0)
        l->end = l->buf_size;
    l->end--;
    mutex_unlock(l->mtx);
    splx(x);
    return res;
}

void
clist_move(clist_t *dst, clist_t *src)
{
    int x = spltty();
    mutex_lock(dst->mtx);
    mutex_lock(src->mtx);
    while(src->size>0) {
        if(dst->size == dst->buf_size)
            break;
        src->size--;
        dst->size++;
        if(dst->beg == 0)
            dst->beg = dst->buf_size;
        dst->beg--;
        dst->buf[dst->beg] = src->buf[src->end];
        if(src->end == 0)
            src->end = src->buf_size;
        src->end--;
    }

    mutex_unlock(src->mtx);
    mutex_unlock(dst->mtx);
    splx(x);
}

