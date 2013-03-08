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

#define HW_PINCTRL_DOE3			0xb30
#define HW_PINCTRL_DOUT3		0x730
#define HW_TIMROT_TIMCTRL2		0xa0
	
	.text
	.global cfa10049_fiq_handler_end
	.global cfa10049_fiq_timer

ENTRY(cfa10049_fiq_handler)
	/*
	 * The interrupt ACK needs to be put first in
	 * the handler to have time for the write to the
	 * interrupt controller to propagates on the bus.
	 * See http://forums.arm.com/index.php?/topic/13692-pipeline-issue-on-return-from-fiq/
	 */
	/* Acknowledge the interrupt */
	mov	r13, #1
	lsl	r13, r13, #15
	str	r13, [r8, #HW_TIMROT_TIMCTRL2 + BIT_CLR]
	/* Enable data lines for this gpio */
	mov	r13, #1
	lsl	r13, r13, #4
	str	r13, [r9, #HW_PINCTRL_DOE3 + BIT_SET]
	/* Invert the values of the gpio */
	str	r13, [r9, #HW_PINCTRL_DOUT3 + BIT_TOG]
	/* return from FIQ */
	subs	pc, lr, #4
cfa10049_fiq_timer:
	.word 0x0
cfa10049_fiq_handler_end: