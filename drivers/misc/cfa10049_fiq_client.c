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

	fd = open(FIQ_PATH, O_RDWR);
	if (!fd) {
		printf("Couldn't open fiq file\n");
		return -EINVAL;
	}

	addr = mmap(NULL, FIQ_BUFFER_SIZE, PROT_WRITE, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED) {
		printf("Couldn't map the file\n");
		return -ENOMEM;
	}

	fiq_buf = (struct fiq_buffer*)addr;

	for (i = 0;; i++) {
		usleep(10);
		fiq_buf->data[fiq_buf->wr_idx++] = ((i % 2) + 1) * 1000000;
		if (fiq_buf->wr_idx = fiq_buf->size)
			fiq_buf->wr_idx = 0;
	}
}
