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
#include <math.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "cfa10049_fiq.h"

#define FIQ_PATH	"/dev/cfa10049_fiq"

#define XP_FREQUENCY	(2000)
#define XP_PERIOD	(1000000 / XP_FREQUENCY)
#define XP_HALF_PERIOD	(XP_PERIOD / 2)
#define YP_FREQUENCY	(2100)
#define YP_PERIOD	(1000000 / YP_FREQUENCY)
#define YP_HALF_PERIOD	(YP_PERIOD / 2)

int main(int argc, char *argv[])
{
	struct fiq_buffer *fiq_buf;
	struct fiq_cell *cell;
	void *addr;
	unsigned tick, prev_tick;
	unsigned long prev_clr, prev_set;
	int fd;
	int ret = 0;

	fd = open(FIQ_PATH, O_RDWR);
	if (!fd) {
		printf("Couldn't open fiq file\n");
		ret = -EINVAL;
		goto out;
	}

	addr = mmap(NULL, FIQ_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (addr == MAP_FAILED) {
		printf("Couldn't map the file, error %d, %s\n", errno, strerror(errno));
		ret = -ENOMEM;
		goto out_close;
	}

	fiq_buf = (struct fiq_buffer*)addr;

	for (tick = 0;; tick++) {
		unsigned long cur_set = 0;
		unsigned long cur_clr = 0;

		if (fiq_buf->status == FIQ_STATUS_ERR_URUN) {
			printf("Underrun, stopping\n");
			ioctl(fd, FIQ_STOP);
			ret = -ENOMEM;
			goto out_munmap;
		}

		if (fmod(tick, XP_PERIOD) < XP_HALF_PERIOD)
			cur_clr |= 1 << 21;
		else
			cur_set |= 1 << 21;

		if (fmod(tick, YP_PERIOD) < YP_HALF_PERIOD)
			cur_clr |= 1 << 22;
		else
			cur_set |= 1 << 22;

		if ((prev_set != cur_set) || (prev_clr != cur_clr)) {
			cell = fiq_buf->data + fiq_buf->wr_idx++;

			cell->timer = tick - prev_tick;
			cell->clear = cur_clr;
			cell->set = cur_set;

			prev_clr = cur_clr;
			prev_set = cur_set;
			prev_tick = tick;
		}

		if (fiq_buf->wr_idx >= fiq_buf->size)
			fiq_buf->wr_idx = 0;

		/* Once we have filled the buffer enough, we can start
		   the FIQ */
		if (tick == 102400) {
			ret = ioctl(fd, FIQ_START);
			if (ret) {
				printf("Couldn't start the FIQ\n");
				goto out_munmap;
			}
		}
	}

out_munmap:
	munmap(addr, FIQ_BUFFER_SIZE);
out_close:
	close(fd);
out:
	return ret;
}
