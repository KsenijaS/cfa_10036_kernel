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
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>

#include <asm/fiq.h>
#include <asm/uaccess.h>

#include <mach/mxs.h>

#include <linux/delay.h>

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

static void __iomem *mxs_icoll_base = MXS_IO_ADDRESS(MXS_ICOLL_BASE_ADDR);

struct cfafiq_data {
	unsigned int	irq;
	void __iomem	*pinctrl_base;
	struct clk	*timer_clk;
	void __iomem	*timrot_base;
};

static irqreturn_t cfafiq_handler(int irq, void *private)
{

	printk("Plop\n");

	asm volatile (
		"ldr r8, =0xf5018000\n\t"
		"ldr r9, =0xf5068000\n\t"
		/* Enable data lines for this gpio */
		"mov	r10, #1\n\t"
		"lsl	r10, r10, #4\n\t"
		"str	r10, [r8, #0xb34]\n\t"
		/* Invert the values of the gpio */
		"str	r10, [r8, #0x73c]\n\t"
		/* Acknowledge the interrupt */
		"mov	r10, #1\n\t"
		"lsl	r10, r10, #15\n\t"
		"str	r10, [r9, #0xa8]\n\t"
		::: "memory", "cc", "r8", "r9", "r10");

	return IRQ_HANDLED;
}


static struct fiq_handler cfa10049_fh = {
	.name	= "cfa10049_fiq_handler"
};

extern unsigned char cfa10049_fiq_handler, cfa10049_fiq_handler_end;
static u32 cfa10049_fiq_timer;

static ssize_t cfa10049_fiq_read(struct file *file, char __user *userbuf,
				size_t count, loff_t *ppos)
{
	unsigned long rate = 24000000;
	u32 rate_us = rate / 1000000;
	u32 period = cfa10049_fiq_timer / rate_us;
	char buf[64];
	ssize_t len;

	len = snprintf(buf, sizeof(buf), "%ul us\n", period);

	return simple_read_from_buffer(userbuf, count, ppos, buf, len);
}

static ssize_t cfa10049_fiq_write(struct file *file,
				const char __user *userbuf,
				size_t count, loff_t *ppos)
{
	unsigned long rate = 24000000;
	u32 period, val;
	char buf[64];
	int ret;

	count = min_t(size_t, count, (sizeof(buf)-1));
	if (copy_from_user(buf, userbuf, count))
		return -EFAULT;

	buf[count] = 0;

	ret = sscanf(buf, "%ul", &val);
	if (ret < 1)
		return -EINVAL;

	if (!val)
		return -EINVAL;

	cfa10049_fiq_timer = val * (rate / 1000000);

	return count;
}

static struct file_operations cfa10049_fiq_fops = {
	.read	= &cfa10049_fiq_read,
	.write	= &cfa10049_fiq_write,
};

static struct miscdevice cfa10049_fiq_dev = {
	.name	= "cfa10049_fiq",
	.fops	= &cfa10049_fiq_fops,
	.minor	= MISC_DYNAMIC_MINOR,
};

static int cfafiq_probe(struct platform_device *pdev)
{
	struct cfafiq_data *fiqdata;
	struct device_node *np;
	struct pt_regs regs;
	int ret;

	np = pdev->dev.of_node;
	if (!np) {
		dev_err(&pdev->dev, "No device tree data available\n");
		return -EINVAL;
	}

	fiqdata = devm_kzalloc(&pdev->dev, sizeof(*fiqdata), GFP_KERNEL);
	if (!fiqdata)
		return -ENOMEM;

	fiqdata->irq = irq_of_parse_and_map(np, 0);
	if (fiqdata->irq < 0) {
		dev_err(&pdev->dev, "Couldn't register given IRQ\n");
		return -EINVAL;
	}

	np = of_find_compatible_node(NULL, NULL, "fsl,imx28-timrot");
	fiqdata->timrot_base = of_iomap(np, 0);
	if (!fiqdata->timrot_base)
		return -ENOMEM;

	np = of_find_compatible_node(NULL, NULL, "fsl,imx28-pinctrl");
	fiqdata->pinctrl_base = of_iomap(np, 0);
	if (!fiqdata->pinctrl_base)
		return -ENOMEM;

	fiqdata->timer_clk = clk_get_sys("timrot", NULL);
	if (IS_ERR(fiqdata->timer_clk))
		return -EINVAL;

	clk_prepare_enable(fiqdata->timer_clk);

	printk("IRQ: %d\n", fiqdata->irq);
	printk("Timrot Base: 0x%x\n", (u32)fiqdata->timrot_base);
	printk("Pinctrl base: 0x%x\n", (u32)fiqdata->pinctrl_base);
	printk("Icoll base: 0x%x\n", (u32)mxs_icoll_base);

	/* 
	 * Setup timer 2 for our FIQ (the two first are already used
	 * for the clocksource events). Since we are targeting an
	 * imx28, we only use the timrotv2.
	 */
	writel(TIMROT_TIMCTRL_RELOAD | TIMROT_TIMCTRL_ALWAYS_TICK | TIMROT_TIMCTRL_IRQ_EN,
	/* writel(TIMROT_TIMCTRL_RELOAD | TIMROT_TIMCTRL_SELECT_32K | TIMROT_TIMCTRL_IRQ_EN, */
		fiqdata->timrot_base + TIMROT_TIMCTRL_REG(2));

	writel(0xf4240, fiqdata->timrot_base + TIMROT_FIXED_COUNT_REG(2));

	ret = claim_fiq(&cfa10049_fh);
	if (ret)
		return ret;

	set_fiq_handler(&cfa10049_fiq_handler,
			&cfa10049_fiq_handler_end - &cfa10049_fiq_handler);

	regs.ARM_r8 = (long)fiqdata->timrot_base;
	regs.ARM_r9 = (long)fiqdata->pinctrl_base;
	set_fiq_regs(&regs);

	/* Enable the FIQ */
	writel(1 << 4, mxs_icoll_base + HW_ICOLL_INTERRUPTn_SET(50));

	ret = misc_register(&cfa10049_fiq_dev);
	if (ret)
		return ret;

	printk("Plop");

	enable_fiq(fiqdata->irq);

	/* ret = request_irq(fiqdata->irq, */
	/* 		cfafiq_handler, */
	/* 		0, */
	/* 		pdev->dev.driver->name, */
	/* 		fiqdata); */

	/* ret = register_chrdev(0, "cfafiq", &cfafiq_fops); */
	/* if (ret) */
	/* 	return ret; */

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
