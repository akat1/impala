#ifndef __CLIST_H__
#define __CLIST_H__

#ifdef __KERNEL
///< bufor znaków w postaci kolejki
struct clist {
    int      *buf;       ///< bufor na dane
    int       beg;       ///< miejsce gdzie zaczyna siê kolejka
    int       end;       ///< koniec kolejki (najstarsze dane)
    int       size;      ///< aktualnie wykorzystana przestrzeñ
    int       buf_size;  ///< wielko¶æ ca³ego bufora
    sleepq_t *slpq;      
};
typedef struct clist clist_t;

clist_t *clist_create(size_t size);
void clist_destroy(clist_t *l);
void clist_wakeup(clist_t *l);
void clist_wait(clist_t *l);
void clist_push(clist_t *l, int ch);
char clist_unpush(clist_t *l);//mo¿e siê jako¶ zdecydowaæ? char / int
int  clist_pop(clist_t *l);
void clist_unpop(clist_t *l, int ch);
void clist_move(clist_t *dst, clist_t *src);
void clist_flush(clist_t *dst);
int  clist_size(clist_t *l);
int  clist_do_uio(clist_t *l, uio_t *u, int flags);

#endif //__KERNEL

#endif
