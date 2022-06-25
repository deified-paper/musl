/* Copyright 2011-2012 Nicholas J. Kain, licensed under standard MIT license */
.text
.extern __hq_syscall
.global __unmapself
.type   __unmapself,@function
__unmapself:
	# Save arguments into callee-preserved registers, for __hq_syscall()
	mov %rdi,%r15
	mov %rsi,%r14

	mov $11, %rdi
	call __hq_syscall

	# Restore saved arguments, for munmap()
	mov %r14,%rsi
	mov %r15,%rdi
	movl $11,%eax   /* SYS_munmap */
	syscall         /* munmap(arg2,arg3) */
	xor %rdi,%rdi   /* exit() args: always return success */
	movl $60,%eax   /* SYS_exit */
	syscall         /* exit(0) */
