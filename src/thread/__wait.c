#include "pthread_impl.h"

void __wait(volatile int *addr, volatile int *waiters, int val, int priv)
{
	int spins=100;
	if (priv) priv = FUTEX_PRIVATE;
	while (spins-- && (!waiters || !*waiters)) {
		if (*addr==val) a_spin();
		else return;
	}
	if (waiters) a_inc(waiters);
	while (*addr==val) {
		__hq_raw_syscall4(SYS_futex, (unsigned long)addr, FUTEX_WAIT|priv, val, 0) != -ENOSYS
		|| __hq_raw_syscall4(SYS_futex, (unsigned long)addr, FUTEX_WAIT, val, 0);
	}
	if (waiters) a_dec(waiters);
}
