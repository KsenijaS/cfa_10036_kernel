/*
 * Crystalfontz CFA-10049 FIQ handler
 *
 * Copyright (C) 2012 Free Electrons
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/linkage.h>
#include <asm/assembler.h>

#define BIT_SET 0x4
#define BIT_CLR	0x8
#define BIT_TOG	0xc

#define MXS_PINCTRL_BASE	0xf5018000
#define HW_PINCTRL_DOE3			0xb30
#define HW_PINCTRL_DOUT3		0x730
#define MXS_TIMROT_BASE		0xf5068000
#define HW_TIMROT_TIMCTRL2		0xa0
	
	.text
	.global cfa10049_fiq_handler_end

ENTRY(cfa10049_fiq_handler)
	ldr	r8, =MXS_PINCTRL_BASE
	ldr	r9, =MXS_TIMROT_BASE
	/* Enable data lines for this gpio */
	mov	r10, #1
	lsl	r10, r10, #4
	str	r10, [r8, #HW_PINCTRL_DOE3 + BIT_SET]
	/* Invert the values of the gpio */
	str	r10, [r8, #HW_PINCTRL_DOUT3 + BIT_TOG]
	/* Acknowledge the interrupt */
	mov	r10, #1
	lsl	r10, r10, #15
	str	r10, [r9, #HW_TIMROT_TIMCTRL2 + BIT_CLR]
	/* return from FIQ */
	subs	pc, lr, #4
cfa10049_fiq_handler_end: