#include <time.h>
#include "syscall.h"

int futex(unsigned int *addr, int op, unsigned int val,
          const struct timespec *timeout, unsigned int *addr2,
          unsigned int addr3) {
    return __syscall_ret(
        __hq_raw_syscall6(__NR_futex, (unsigned long)addr, op, val, (unsigned long)timeout, (unsigned long)addr2, addr3));
}
