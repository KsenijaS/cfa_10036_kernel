/*
 * Crystalfontz CFA-10049 FIQ test app
 *
 * Copyright (C) 2012 Free Electrons
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "cfa10049_fiq.h"

#define FIQ_PATH	"/dev/cfa10049_fiq"

int main(int argc, char *argv[])
{
	struct fiq_buffer *fiq_buf;
	void *addr;
	unsigned i;
	int fd;
	int ret = 0;

	fd = open(FIQ_PATH, O_RDWR);
	if (!fd) {
		printf("Couldn't open fiq file\n");
		ret = -EINVAL;
		goto out;
	}

	addr = mmap(NULL, FIQ_BUFFER_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED) {
		printf("Couldn't map the file, error %d, %s\n", errno, strerror(errno));
		ret = -ENOMEM;
		goto out_close;
	}

	fiq_buf = (struct fiq_buffer*)addr;

	for (i = 0;i < 256; i++) {
		fiq_buf->data[fiq_buf->wr_idx++] = i;
		if (fiq_buf->wr_idx == fiq_buf->size)
			fiq_buf->wr_idx = 0;
	}

	ret = ioctl(fd, FIQ_START);
	if (ret) {
		printf("Couldn't start the FIQ\n");
		goto out_munmap;
	}

	for (;; i++) {
		fiq_buf->data[fiq_buf->wr_idx++] = i;
		if (fiq_buf->wr_idx == fiq_buf->size)
			fiq_buf->wr_idx = 0;
	}

out_munmap:
	munmap(NULL, FIQ_BUFFER_SIZE);
out_close:
	close(fd);
out:
	return ret;
}
