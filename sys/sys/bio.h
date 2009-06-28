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

/** @file buforowane wej¶cie-wyj¶cie.
 *
 * Plik zawiera deklaracje dla modu³u BIO.
 */

#ifndef __SYS_BIO_H
#define __SYS_BIO_H
#ifdef __KERNEL

/// Nag³ówek buforu wej¶cia-wyj¶cia.
struct iobuf {
    void        *addr;      ///< adres buforu.
    int          oper;      ///< operacja BIO_READ/WRITE
    int          errno;     ///< numer b³êdu
    blkno_t      blkno;     ///< numer bloku
    size_t       bcount;    ///< wielko¶æ buforu w blokach(sektorach)
    size_t       size;      ///< wielko¶c buforu w bajtach
    size_t       resid;     ///< ilo¶æ pozosta³ych bajtów do wys³ania
    int          flags;     ///< znaczniki
    devd_t      *dev;       ///< buforowane urz±dzenie
    sleepq_t     sleepq;    ///< ¶pi±ca kolejka do czekania na wynik
    list_node_t  L_bioq; 
    list_node_t  L_bufs;
    list_node_t  L_hash;
    list_node_t  L_free;
};

/// znaczniki nag³ówka buforu
enum BUF_FLAGS {
    BIO_BUSY      = 0x00001,    ///< bufor jest u¿ywany
    BIO_DONE      = 0x00002,    ///< operacja I/O zakoñczona pomy¶lnie
    BIO_WRITE     = 0x00004,    ///< bufor zaw. dane do zapisu
    BIO_READ      = 0x00008,    ///< bufor do odczytu
    BIO_ERROR     = 0x00020,    ///< operacja I/O zakoñczona z b³êdem
    BIO_CACHE     = 0x00080,    ///< bufor jest w pamiêci podrêcznej
    BIO_DELWRI    = 0x00100,    ///< opó¼niony zapis
    BIO_WANTED    = 0x00200,    ///< kto¶ oczekuje na bufor
    BIO_VALID     = 0x00400,    ///< blkno i dev s± poprawne
};

/// kolejka oparacji BIO (dla sterowników)
typedef struct bio_queue bio_queue_t;
struct bio_queue {
    mutex_t     bq_mtx;
    list_t      bq_queue;
};

void bio_init(void);
iobuf_t *bio_getblk(devd_t *d, blkno_t n, size_t bsize);
iobuf_t *bio_read(devd_t *d, blkno_t n, size_t bsize);
void bio_write(iobuf_t *bp);
void bio_delwrite(iobuf_t *bp);
void bio_release(iobuf_t *bp);
void bio_wait(iobuf_t *bp);
void bio_wakeup(iobuf_t *bp);
void bio_done(iobuf_t *bp);
void bio_error(iobuf_t *bp, int errno);

void bioq_init(bio_queue_t *q);
void bioq_enqueue(bio_queue_t *q, iobuf_t *bp);
iobuf_t *bioq_dequeue(bio_queue_t *q);
void bioq_lock(bio_queue_t *q);
void bioq_unlock(bio_queue_t *q);


ssize_t physio(devd_t *dev, uio_t *uio, int bioflags);


#endif
#endif

