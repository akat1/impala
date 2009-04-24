/*
 * ImpalaOS
 *  http://trzask.codepainters.com/impala/trac/
 *
 * $Id$
 */

#include <sys/types.h>
#include <sys/thread.h>
#include <sys/libkutil.h>
#include <sys/sched.h>
#include <sys/utils.h>
#include <sys/kprintf.h>

thread_t *curthread;
thread_t *thread_idle;

list_t threads_list;

static thread_t thread_table[MAX_THREAD];
static uint last_pid;

static list_t __free_threads;

/*=============================================================================
 * Obsluga watkow.
 */

/// Inicjalizuje obs³ugê w±tków.
void
thread_init()
{
    last_pid = 0;
    mem_zero(&thread_table, sizeof(thread_table));
    list_create(&threads_list, offsetof(thread_t, L_threads), FALSE);
    list_create(&__free_threads, offsetof(thread_t, L_threads), FALSE);
    for (int i = 0; i < MAX_THREAD; i++) {
        list_insert_tail(&__free_threads, &thread_table[i]);
    }
    curthread = thread_create(0, 0, 0);
    curthread->thr_flags = THREAD_RUN;

}

/**
 * Przydziela deskryptor w±tku.
 * @param priv Poziom uprzywilejowania.
 * @param entry Adres procedury wej¶ciowej.
 * @param arg Adres argumentu przekazywany do procedury wej¶ciowej.
 *
 * Procedura przydziela ogólny deskryptor w±tku. W±tek przydzielony
 * w ten sposób znajduje siê w stanie surowym.
 */
thread_t *
thread_create(int priv, addr_t entry, addr_t arg)
{
    thread_t *free_thr = list_extract_first(&__free_threads);
    if (free_thr) {
        thread_t *t = free_thr;
        t->thr_flags = THREAD_NEW;
        t->thr_tid = last_pid++;
        t->thr_priv = priv;
        t->thr_entry_point = entry;
        t->thr_entry_arg = arg;
        t->thr_wakeup_time = 0;
        thread_context_init(&t->thr_context, priv, t->thr_stack);
        list_insert_tail(&threads_list, t);
        return t;
    } else {
        kprintf("ERROR: no free threads!\n");
        return 0;
    }
}

/*=============================================================================
 * Obsluga mutexow. (FIFO).
 */

static void _mutex_wakeup(mutex_t *m);

/// Inicjalizuje deskryptor zamka mutex.
void
mutex_init(mutex_t *m, int flags)
{
    mem_zero(m, sizeof(mutex_t));
    m->mtx_flags = flags;
    m->mtx_locked = MUTEX_UNLOCKED;
    spinlock_init(&m->mtx_slock);
    list_create(&m->mtx_locking, offsetof(thread_t, L_wait), FALSE);
    if (flags & MUTEX_CONDVAR) {
        list_create(&m->mtx_waiting, offsetof(thread_t, L_wait), FALSE);
    }
}

/// Niszczy zamek.
void
mutex_destroy(mutex_t *m)
{
}

/**
 * Zamyka zamek.
 * @param m zamek do zamkniêcia.
 *
 * W przypadku gdy zamek jest zajêty aktualny w±tek
 * zostaje u¶piony.
 * W celu zapewnienia nieg³odzenia implementowana jest strategia
 * wykorzystuj±ca kolejkê FIFO.
 */

void
mutex_lock(mutex_t *m)
{
    if ( atomic_change_int(&m->mtx_locked, MUTEX_LOCKED) == MUTEX_UNLOCKED) {
        m->mtx_owner = curthread;
    } else {
        spinlock_lock(&m->mtx_slock);
        list_insert_tail(&m->mtx_locking, curthread);
        spinlock_unlock(&m->mtx_slock);
        sched_wait();
    }
}

/**
 * Odblokowuje zamek.
 * @param m zamek.
 *
 */

void
mutex_unlock(mutex_t *m)
{
    thread_t *n;

    spinlock_lock(&m->mtx_slock);
    _mutex_wakeup(m);
    m->mtx_owner = NULL;
    n = list_extract_first(&m->mtx_locking);
    if (n) {
        m->mtx_owner = n;
        sched_wakeup(n);
    } else {
        m->mtx_locked = MUTEX_UNLOCKED;
    }
    spinlock_unlock(&m->mtx_slock);
}

/**
 * Budzenie w±tków oczekuj±cych na sygna³.
 */

void
_mutex_wakeup(mutex_t *m)
{
    int l = 0;
    if (m->mtx_flags & MUTEX_WAKEUP_ALL) {
        l = list_length(&m->mtx_waiting);
    } else
    if (m->mtx_flags & MUTEX_WAKEUP_ONE) {
        l = (list_length(&m->mtx_waiting)==0)? 0 : 1;
    }
    if (l)
    for (int i = 0; i < l; i++) {
        thread_t *n = list_extract_first(&m->mtx_waiting);
           list_insert_tail(&m->mtx_locking, n);
    }
    m->mtx_flags &= ~(MUTEX_WAKEUP_ONE|MUTEX_WAKEUP_ALL);
}

/**
 * Próbuje zamkn±æ zamek.
 * @param m zamek.
 * @return Zwraca prawdê wtedy i tylko wtedy, gdy uda³o siê zamkn±æ zamek.
 *         w przeciwnym wypadku zwracany jest fa³sz.
 */

bool
mutex_trylock(mutex_t *m)
{
    if (atomic_change_int(&m->mtx_locked, MUTEX_LOCKED) == MUTEX_UNLOCKED) {
        m->mtx_owner = curthread;
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * Oczekuje na danym zamku na sygna³.
 * @param m zamkniêty zamek.
 *
 * Je¿eli w±tek jest w³a¶cicielem zamku to mo¿e oczekiwaæ na nim sygna³.
 * Procedura wychodzi z sekcji krytycznej, a nastêpnie usypia w±tek.
 * U¶piony w±tek jest dodawany do listy w±tków oczekuj±cych na sygna³.
 * 
 * Gdy w±tek zostanie obudzony to zamek zostanie automatycznie mu
 * przydzielony (powróci do swojej sekcji krytycznej).
 */

void
mutex_wait(mutex_t *m)
{
    spinlock_lock(&m->mtx_slock);
    list_insert_tail(&m->mtx_waiting, curthread);
    spinlock_unlock(&m->mtx_slock);
    sched_unlock_and_wait(m);
}

/**
 * Budzi jeden w±tek oczekuj±cy na sygna³.
 * @param m zamek.
 * 
 * Je¿eli w±tek jest w³a¶cicielem zamka, to mo¿e obudziæ oczekuj±cy sygna³u
 * na tym zamku w±tek. Procedura zaznacza informacjê, ¿e przy odblokowaniu
 * zamku nale¿y przenie¶æ jeden w±tek oczekuj±cy na sygna³ do listy w±tków
 * chc±cych wej¶æ do sekcji krytycznej.
 */

void
mutex_wakeup(mutex_t *m)
{
    spinlock_lock(&m->mtx_slock);
//     kprintf("mutex_wakeup:%p\n", m->mtx_owner);
    m->mtx_flags |= MUTEX_WAKEUP_ONE;
    spinlock_unlock(&m->mtx_slock);
}

/// Budzi wszystkie w±tki oczekuj±ce na sygna³.
void
mutex_wakeup_all(mutex_t *m)
{
    spinlock_lock(&m->mtx_slock);
    m->mtx_flags |= MUTEX_WAKEUP_ALL;
    spinlock_unlock(&m->mtx_slock);
}

/*=============================================================================
 * Obsluga kolejek
 */


/**
 * Inicjalizuje wspó³bie¿n± kolejkê.
 * @param q wska¼nik do deskryptora kolejki.
 * @param off przesuniêcie uchwytu dla listy.
 */

void
cqueue_init(cqueue_t *q, int off)
{
    mutex_init(&q->q_mtx, MUTEX_CONDVAR);
    list_create(&q->q_data, off, FALSE);
}

/**
 * Wy³±cza wspo³biezn± kolejkê.
 * @param q kolejka.
 *
 * Niepozwala w±tkom spaæ w oczekiwaniu na kolejne dane. U¿yteczne
 * przy koñczeniu pracy z dan± kolejk±. NIEZAIMPLEMENTOWANE.
 */
void
cqueue_shutdown(cqueue_t *q)
{
}

/**
 * Wrzuca wska¼nik w kolejkê.
 * @param q kolejka
 * @param d wska¼nik do zakolejkowania.
 *
 * Procedura po zakolejkowaniu wska¼nika budzi jeden
 * z w±tków oczekuj±cych na dane.
 */
void
cqueue_insert(cqueue_t *q, void *d)
{
    mutex_lock(&q->q_mtx);
    list_insert_tail(&q->q_data, d);
//     kprintf("cqI: wakeup\n");
    mutex_wakeup(&q->q_mtx);
    mutex_unlock(&q->q_mtx);
}

/**
 * Pobranie z kolejki wska¼nika.
 * @param q kolejka
 *
 * Procedura usypia w±tek, gdy kolejka jest pusta a kolejka
 * nie zota³a w³±czona. W przeciwnym wypadku zwraca NULL.
 */
 
void*
cqueue_extract(cqueue_t *q)
{
    void *p;
    mutex_lock(&q->q_mtx);
    while ( (p = list_extract_first(&q->q_data)) == NULL ) {
//         kprintf("cqE: wait\n");
        mutex_wait(&q->q_mtx);
    }
    mutex_unlock(&q->q_mtx);
    return p;
}

/*============================================================================
 * Semafory
 */

void
semaph_init(semaph_t *sem)
{
    mutex_init(&sem->mtx, MUTEX_CONDVAR);
    sem->count = 0;
}

void
semaph_post(semaph_t *sem)
{
    mutex_lock(&sem->mtx);
    sem->count++;
    mutex_wakeup(&sem->mtx);
    mutex_unlock(&sem->mtx);
}

void
semaph_wait(semaph_t *sem)
{
    mutex_lock(&sem->mtx);
    if (sem->count == 0) {
        mutex_wait(&sem->mtx);
    }
    sem->count--;
    mutex_unlock(&sem->mtx);
}

