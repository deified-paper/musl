#define _GNU_SOURCE
#include <stdarg.h>
#include <unistd.h>
#include <sched.h>
#include "lock.h"
#include "pthread_impl.h"
#include "aio_impl.h"
#include "syscall.h"
#include "dynlink.h"

extern int _Fork(int (*)(void *), void *);

struct clone_args {
	int flags;
	pid_t *ptid, *ctid;
};

static int do_clone(void *p)
{
	struct clone_args *args = p;
	int mask = CLONE_PARENT_SETTID | CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID;
	int r = __syscall(SYS_clone, args->flags & ~mask, 0);
	if (r>0 && (args->flags & CLONE_PARENT_SETTID)) *args->ptid = r;
	if (!r && (args->flags & CLONE_CHILD_SETTID)) *args->ctid = __syscall(SYS_gettid);
	if (!r && (args->flags & CLONE_CHILD_CLEARTID)) __syscall(SYS_set_tid_address, args->ctid);
	return r;
}

int clone(int (*func)(void *), void *stack, int flags, void *arg, ...)
{
	va_list ap;
	pid_t *ptid, *ctid;
	void  *tls;

	va_start(ap, arg);
	ptid = va_arg(ap, pid_t *);
	tls  = va_arg(ap, void *);
	ctid = va_arg(ap, pid_t *);
	va_end(ap);

	if (!(flags & CLONE_VM)) {
		struct clone_args args = { .flags = flags, .ptid = ptid, .ctid = ctid };
		if (flags & CLONE_THREAD) return __syscall_ret(-EINVAL);
		int r = _Fork(do_clone, &args);
		if (r) return r;
		__asm__ volatile ("mov %0, %%rdi" : : "rm" (arg));
		CRTJMP(func, stack);
	}

	return __syscall_ret(__clone(func, stack, flags, arg, ptid, tls, ctid));
}
