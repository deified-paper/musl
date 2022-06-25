#include "stdio_impl.h"
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "libc.h"

FILE *__fdopen(int fd, const char *mode)
{
	FILE *f;
	struct winsize wsz;

	/* Check for valid initial mode character */
	if (!strchr("rwa", *mode)) {
		errno = EINVAL;
		return 0;
	}

	/* Allocate FILE+buffer or fail */
	if (!(f=malloc(sizeof *f + UNGET + BUFSIZ))) return 0;

	/* Zero-fill only the struct, not the buffer */
	memset(f, 0, sizeof *f);

	/* Impose mode restrictions */
	if (!strchr(mode, '+')) f->flags = (*mode == 'r') ? F_NOWR : F_NORD;

	/* Apply close-on-exec flag */
	if (strchr(mode, 'e'))
		if (__syscall(SYS_fcntl, fd, F_SETFD, FD_CLOEXEC) < 0)
			goto out_err;

	/* Set append mode on fd if opened for append */
	if (*mode == 'a') {
		int flags = __syscall(SYS_fcntl, fd, F_GETFL);
		if (flags < 0)
			goto out_err;
		if (!(flags & O_APPEND))
			if (__syscall(SYS_fcntl, fd, F_SETFL, flags | O_APPEND) < 0)
				goto out_err;
		f->flags |= F_APP;
	}

	f->fd = fd;
	f->buf = (unsigned char *)f + sizeof *f + UNGET;
	f->buf_size = BUFSIZ;

	/* Activate line buffered mode for terminals */
	f->lbf = EOF;
	if (!(f->flags & F_NOWR)) {
		int ret = __syscall(SYS_ioctl, fd, TIOCGWINSZ, &wsz);
		if (ret < 0 && ret != -ENOTTY && ret != -ENOSYS)
			goto out_err;
		if (!ret)
			f->lbf = '\n';
	}

	/* Initialize op ptrs. No problem if some are unneeded. */
	f->read = __stdio_read;
	f->write = __stdio_write;
	f->seek = __stdio_seek;
	f->close = __stdio_close;

	if (!libc.threaded) f->lock = -1;

	/* Add new FILE to open file list */
	return __ofl_add(f);

out_err:
	free(f);
	return 0;
}

weak_alias(__fdopen, fdopen);
