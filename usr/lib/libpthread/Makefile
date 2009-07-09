LIBRARY= libpthread
PREFIX=${AOUT_PREFIX}
CFLAGS=${_U_FLAGS}
INCDIR=-Iinclude -Iprivate
SRCS=\
    _pthread_rt.c\
	pthread_create.c\
	pthread_cancel.c\
	pthread_detach.c\
	pthread_equal.c\
	pthread_exit.c\
	pthread_join.c\
	pthread_kill.c\
	pthread_once.c\
	pthread_self.c\
	pthread_setcancelstate.c\
	pthread_setcanceltype.c\
	pthread_testcancel.c\
	pthread_yield.c\
	pthread_attr_getstackaddr.c\
	pthread_attr_getstacksize.c\
	pthread_attr_destroy.c\
	pthread_attr_init.c\
	pthread_attr_setstackaddr.c\
	pthread_attr_setstacksize.c\
	pthread_mutex_destroy.c\
	pthread_mutex_init.c\
	pthread_mutex_lock.c\
	pthread_mutex_unlock.c\
	pthread_mutex_trylock.c\
	pthread_cond_broadcast.c\
	pthread_cond_destroy.c\
	pthread_cond_init.c\
	pthread_cond_signal.c\
	pthread_cond_wait.c


include ${IMPALA_MK}/lib.mk
