#include <unistd.h>
#include <signal.h>
#include "syscall.h"
#include "libc.h"
#include "lock.h"
#include "pthread_impl.h"
#include "aio_impl.h"

static void dummy(int x) { }
weak_alias(dummy, __aio_atfork);

void __hq_init(int);

pid_t _Fork(int (*func)(void *), void *arg)
{
	pid_t ret;
	sigset_t set;
	__block_all_sigs(&set);
	__aio_atfork(-1);
	LOCK(__abort_lock);
	ret = (*func)(arg);
	if (!ret) {
		struct pthread *self = __pthread_self();
		self->tid = __hq_raw_syscall0(SYS_gettid);

		// Reinitialize after fork
		__hq_init(1);

		self->robust_list.off = 0;
		self->robust_list.pending = 0;
		self->next = self->prev = self;
		__thread_list_lock = 0;
		libc.threads_minus_1 = 0;
		if (libc.need_locks) libc.need_locks = -1;
	}
	UNLOCK(__abort_lock);
	__aio_atfork(!ret);
	__restore_sigs(&set);
	return __syscall_ret(ret);
}
