#define _GNU_SOURCE
#include <unistd.h>
#include "pthread_impl.h"

void __hq_pointer_check(const void **, const void *);

pid_t gettid(void)
{
	const pid_t *ptid = &__pthread_self()->tid, tid = *ptid;
	__hq_pointer_check((const void **)ptid, (void *)(intptr_t)tid);
	return tid;
}
