/* Copyright 2011-2012 Nicholas J. Kain, licensed under standard MIT license */
.text
.extern __hq_syscall
.global __set_thread_area
.hidden __set_thread_area
.type __set_thread_area,@function
__set_thread_area:
	push %rdi
	mov $158, %rdi
	call __hq_syscall

	pop %rsi                /* shift for syscall */
	movl $0x1002,%edi       /* SET_FS register */
	movl $158,%eax          /* set fs segment to */
	syscall                 /* arch_prctl(SET_FS, arg)*/
	ret
