#ifndef __CLIST_H__
#define __CLIST_H__

#define PIPE_BUF 4096

#ifdef __KERNEL
///< bufor znaków w postaci kolejki
struct clist {
    char     *buf;       ///< bufor na dane
    int       beg;       ///< miejsce gdzie zaczyna się kolejka
    int       end;       ///< koniec kolejki (najstarsze dane)
    int       size;      ///< aktualnie wykorzystana przestrzeń
    int       buf_size;  ///< wielkość całego bufora
    sleepq_t *slpq;      
};
typedef struct clist clist_t;

clist_t *clist_create(size_t size);
void clist_destroy(clist_t *l);
void clist_wakeup(clist_t *l);
void clist_wait(clist_t *l);
void clist_push(clist_t *l, char ch);
char clist_unpush(clist_t *l);
int  clist_pop(clist_t *l);   ///< ujemne, jeżeli pusto
void clist_unpop(clist_t *l, char ch);
void clist_move(clist_t *dst, clist_t *src);
void clist_flush(clist_t *dst);
int  clist_size(clist_t *l);

#endif //__KERNEL

#endif
