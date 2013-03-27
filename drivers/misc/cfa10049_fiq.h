/*
 * Crystalfontz CFA-10049 FIQ handler API
 *
 * Copyright (C) 2012 Free Electrons
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/ioctl.h>

#define FIQ_BUFFER_SIZE		(2 * 1024 * 1024)

#define FIQ_IOC_MAGIC            'p'
#define FIQ_START		_IO(FIQ_IOC_MAGIC, 0xb0)
#define FIQ_STOP		_IO(FIQ_IOC_MAGIC, 0xb1)

struct fiq_buffer {
	unsigned long	rd_idx;
	unsigned long	wr_idx;
	unsigned long	size;
	unsigned long	*data;
};
