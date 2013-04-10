/*
 * Crystalfontz CFA-10049 FIQ handler
 *
 * Copyright (C) 2012 Free Electrons
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/cdev.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/string.h>

#include <asm/fiq.h>
#include <asm/pgtable.h>
#include <asm/uaccess.h>

#include <mach/mxs.h>

#include "cfa10049_fiq.h"

#define TIMROT_TIMCTRL_REG(n)		(0x20 + (n) * 0x40)
#define TIMROT_TIMCTRL_SELECT_32K		(0xb)
#define TIMROT_TIMCTRL_ALWAYS_TICK		(0xf)
#define TIMROT_TIMCTRL_RELOAD			(1 << 6)
#define TIMROT_TIMCTRL_UPDATE			(1 << 7)
#define TIMROT_TIMCTRL_IRQ_EN			(1 << 14)
#define TIMROT_TIMCTRL_IRQ			(1 << 15)
#define TIMROT_FIXED_COUNT_REG(n)	(0x40 + (n) * 0x40)

#define HW_ICOLL_INTERRUPTn_SET(n)	(0x0124 + (n) * 0x10)

#define GPIO	(3 * 32 + 4)

#define BIT_SET 0x4
#define BIT_CLR	0x8
#define BIT_TOG	0xc

#define HW_PINCTRL_DOE3			0xb30
#define HW_PINCTRL_DOUT3		0x730
#define HW_TIMROT_TIMCTRL2		0xa0

struct cfafiq_data {
	struct cdev	chrdev;
	unsigned int	irq;
	void __iomem	*pinctrl_base;
	struct clk	*timer_clk;
	void __iomem	*timrot_base;
};

static void __iomem *mxs_icoll_base = MXS_IO_ADDRESS(MXS_ICOLL_BASE_ADDR);
static unsigned long *fiq_base;
static struct cfafiq_data *cfa10049_fiq_data;

extern unsigned char cfa10049_fiq_handler, cfa10049_fiq_handler_end;

static irqreturn_t cfafiq_handler(int irq, void *private)
{
	struct fiq_buffer *fiq_buf = (struct fiq_buffer*)fiq_base;
	struct fiq_cell *cell;

	cell = fiq_buf->data + fiq_buf->rd_idx++;
	if (fiq_buf->rd_idx >= fiq_buf->size)
		fiq_buf->rd_idx = 0;

	writel(cell->timer, cfa10049_fiq_data->timrot_base + TIMROT_FIXED_COUNT_REG(2));
	writel(cell->clear, cfa10049_fiq_data->pinctrl_base + HW_PINCTRL_DOUT3 + BIT_CLR);
	writel(cell->set, cfa10049_fiq_data->pinctrl_base + HW_PINCTRL_DOUT3 + BIT_SET);

	asm volatile (
		"ldr r8, =0xf5018000\n\t"
		"ldr r9, =0xf5068000\n\t"
	/* 	/\* Enable data lines for this gpio *\/ */
	/* 	"mov	r10, #1\n\t" */
	/* 	"lsl	r10, r10, #4\n\t" */
	/* 	"str	r10, [r8, #0xb34]\n\t" */
	/* 	/\* Invert the values of the gpio *\/ */
	/* 	"str	r10, [r8, #0x73c]\n\t" */
		/* Acknowledge the interrupt */
		"mov	r10, #1\n\t"
		"lsl	r10, r10, #15\n\t"
		"str	r10, [r9, #0xa8]\n\t"
		::: "memory", "cc", "r8", "r9", "r10");

	return IRQ_HANDLED;
}

#ifdef FIQ
static struct fiq_handler cfa10049_fh = {
	.name	= "cfa10049_fiq_handler"
};
#endif

static int cfa10049_fiq_mmap(struct file *file, struct vm_area_struct *vma)
{
	size_t size = vma->vm_end - vma->vm_start;
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;

	printk("Louloulou\n");

	printk("vm_start: 0x%lx,\tvm_end: 0x%lx,\tsize: %lu\n", vma->vm_start, vma->vm_end, size);

	if (offset + size > FIQ_BUFFER_SIZE)
		return -EINVAL;

	printk("Ohay\n");

	offset += __pa(fiq_base);

	printk("Test\n");

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	printk("Pwet\n");

	/* Remap-pfn-range will mark the range VM_IO */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    offset >> PAGE_SHIFT,
			    size,
			    vma->vm_page_prot)) {
		return -EAGAIN;
	}

	printk("%s: mmap buffer P(%lx)->V(%lx)\n", __FILE__,
	       offset, vma->vm_start);
	

	printk("Kwain\n");

	return 0;
}

static long cfa10049_fiq_ioctl(struct file *file,
			       unsigned int cmd, unsigned long arg)
{
	printk("Calling IOCTL\n");

	switch (cmd) {
	case FIQ_START:
#ifdef FIQ
		enable_fiq(cfa10049_fiq_data->irq);
#else
		enable_irq(cfa10049_fiq_data->irq);
#endif
		break;
	case FIQ_STOP:
#ifdef FIQ
		disable_fiq(cfa10049_fiq_data->irq);
#else
		disable_irq(cfa10049_fiq_data->irq);
#endif
		break;
	default:
		return -ENOTTY;
	};

	return 0;
}

static struct file_operations cfa10049_fiq_fops = {
	.mmap		= &cfa10049_fiq_mmap,
	.unlocked_ioctl	= &cfa10049_fiq_ioctl,
};

static struct miscdevice cfa10049_fiq_dev = {
	.name	= "cfa10049_fiq",
	.fops	= &cfa10049_fiq_fops,
	.minor	= MISC_DYNAMIC_MINOR,
};

static int cfafiq_probe(struct platform_device *pdev)
{
	struct fiq_buffer *fiq_buf;
	struct device_node *np;
	int ret;

#ifdef FIQ
	struct pt_regs regs;
#endif

	np = pdev->dev.of_node;
	if (!np) {
		dev_err(&pdev->dev, "No device tree data available\n");
		return -EINVAL;
	}

	cfa10049_fiq_data = devm_kzalloc(&pdev->dev, sizeof(*cfa10049_fiq_data), GFP_KERNEL);
	if (!cfa10049_fiq_data)
		return -ENOMEM;

	cfa10049_fiq_data->irq = irq_of_parse_and_map(np, 0);
	if (cfa10049_fiq_data->irq < 0) {
		dev_err(&pdev->dev, "Couldn't register given IRQ\n");
		return -EINVAL;
	}

	np = of_find_compatible_node(NULL, NULL, "fsl,imx28-timrot");
	cfa10049_fiq_data->timrot_base = of_iomap(np, 0);
	if (!cfa10049_fiq_data->timrot_base)
		return -ENOMEM;

	np = of_find_compatible_node(NULL, NULL, "fsl,imx28-pinctrl");
	cfa10049_fiq_data->pinctrl_base = of_iomap(np, 0);
	if (!cfa10049_fiq_data->pinctrl_base)
		return -ENOMEM;

	cfa10049_fiq_data->timer_clk = clk_get_sys("timrot", NULL);
	if (IS_ERR(cfa10049_fiq_data->timer_clk))
		return -EINVAL;

	clk_prepare_enable(cfa10049_fiq_data->timer_clk);

	printk("Rate: %lu\n", clk_get_rate(cfa10049_fiq_data->timer_clk));
	printk("cfa10049_fiq_data: %p\n", cfa10049_fiq_data);

	printk("IRQ: %d\n", cfa10049_fiq_data->irq);
	printk("Timrot Base: 0x%x\n", (u32)cfa10049_fiq_data->timrot_base);
	printk("Pinctrl base: 0x%x\n", (u32)cfa10049_fiq_data->pinctrl_base);
	printk("Icoll base: 0x%x\n", (u32)mxs_icoll_base);

	fiq_base = (unsigned long*)__get_free_pages(GFP_KERNEL, get_order(FIQ_BUFFER_SIZE));
	memset(fiq_base, 0, FIQ_BUFFER_SIZE);
	if (!fiq_base) {
		printk("Couldn't allocate memory\n");
		return -ENOMEM;
	}

	fiq_buf = (struct fiq_buffer*)fiq_base;

	printk("Allocated pages at address 0x%p, with size %dMB\n", fiq_base, FIQ_BUFFER_SIZE >> 20);

	printk("Size of fiq_buf %d\n", sizeof(*fiq_buf));

	/* FIXME: Underestimated size by 4 bytes */
	fiq_buf->size = FIQ_BUFFER_SIZE - sizeof(*fiq_buf);

	printk("Size of the the buffer %lu\n", fiq_buf->size);

	/* 
	 * Setup timer 2 for our FIQ (the two first are already used
	 * for the clocksource events). Since we are targeting an
	 * imx28, we only use the timrotv2.
	 */
	writel(TIMROT_TIMCTRL_RELOAD | TIMROT_TIMCTRL_UPDATE | TIMROT_TIMCTRL_ALWAYS_TICK | TIMROT_TIMCTRL_IRQ_EN,
	/* writel(TIMROT_TIMCTRL_RELOAD | TIMROT_TIMCTRL_SELECT_32K | TIMROT_TIMCTRL_IRQ_EN, */
		cfa10049_fiq_data->timrot_base + TIMROT_TIMCTRL_REG(2));
	writel(1000000, cfa10049_fiq_data->timrot_base + TIMROT_FIXED_COUNT_REG(2));


#ifdef FIQ
	ret = claim_fiq(&cfa10049_fh);
	if (ret)
		return ret;

	set_fiq_handler(&cfa10049_fiq_handler,
			&cfa10049_fiq_handler_end - &cfa10049_fiq_handler);

	regs.ARM_r8 = (long)cfa10049_fiq_data->timrot_base;
	regs.ARM_r9 = (long)cfa10049_fiq_data->pinctrl_base;
	regs.ARM_r10 = (long)fiq_base;
	set_fiq_regs(&regs);

	/* Enable the FIQ */
	writel(1 << 4, mxs_icoll_base + HW_ICOLL_INTERRUPTn_SET(50));
#else
	ret = request_irq(cfa10049_fiq_data->irq,
			  cfafiq_handler,
			  IRQ_NOAUTOEN,
			  pdev->dev.driver->name,
			  cfa10049_fiq_data);
	disable_irq(cfa10049_fiq_data->irq);
#endif

	ret = misc_register(&cfa10049_fiq_dev);
	if (ret)
		return ret;

	return 0;
}

static int cfafiq_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id cfafiq_of_match[] = {
	{ .compatible = "crystalfontz,cfa10049-fiq" },
	{}
};

static struct platform_driver cfafiq_driver = {
	.probe	= cfafiq_probe,
	.remove	= cfafiq_remove,
	.driver = {
		.name = "cfafiq",
		.of_match_table = of_match_ptr(cfafiq_of_match),
		.owner = THIS_MODULE,
	},
};

module_platform_driver(cfafiq_driver);

MODULE_DESCRIPTION("FIQ handler to driver the CFA-10049 stepper drivers");
MODULE_AUTHOR("Maxime Ripard <maxime.ripard@free-electrons.com>");
MODULE_LICENSE("GPL");
