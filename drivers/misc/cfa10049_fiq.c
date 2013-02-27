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
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>

#include <mach/mxs.h>

#define TIMROT_TIMCTRL_REG(n)		(0x20 + (n) * 0x40)
#define TIMROT_TIMCTRL_SELECT_32K		(0xb)
#define TIMROT_TIMCTRL_ALWAYS_TICK		(0xf)
#define TIMROT_TIMCTRL_RELOAD			(1 << 6)
#define TIMROT_TIMCTRL_IRQ_EN			(1 << 14)
#define TIMROT_TIMCTRL_IRQ			(1 << 15)
#define TIMROT_FIXED_COUNT_REG(n)	(0x40 + (n) * 0x40)

#define GPIO	(3 * 32 + 4)

/* struct file_operations cfafiq_fops = { */
/* }; */

static void __iomem *mxs_timrot_base = MXS_IO_ADDRESS(MXS_TIMROT_BASE_ADDR);

struct cfafiq_data {
	unsigned int	irq;
};

static irqreturn_t cfafiq_handler(int irq, void *private)
{

	printk("Plop\n");

	asm volatile (
		"ldr r0, =0xf5018b30\n\t"
		"ldr r1, =0xf5018730\n\t"
		"ldr r2, =0xf50680a0\n\t"
		/* Enable data lines for this gpio */
		"mov r3, #1\n\t"
		"lsl r3, r3, #4\n\t"
		"str r3, [r0, #4]\n\t"
		/* Invert the values of the gpio */
		"str r3, [r1, #0xc]\n\t"
		/* Acknowledge the interrupt */
		"mov r3, #1\n\t"
		"lsl r3, r3, #15\n\t"
		"str r3, [r2, #8]\n\t"
		::: "memory", "cc", "r0", "r1", "r2", "r3");

	return IRQ_HANDLED;
}

static int cfafiq_probe(struct platform_device *pdev)
{
	struct cfafiq_data *fiqdata;
	struct device_node *np;
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

	/* 
	 * Setup timer 2 for our FIQ (the two first are already used
	 * for the clocksource events). Since we are targeting an
	 * imx28, we only use the timrotv2.
	 */
	/* __raw_writel(TIMROT_TIMCTRL_RELOAD | TIMROT_TIMCTRL_ALWAYS_TICK | TIMROT_TIMCTRL_IRQ_EN, */
	__raw_writel(TIMROT_TIMCTRL_RELOAD | TIMROT_TIMCTRL_SELECT_32K | TIMROT_TIMCTRL_IRQ_EN,
		mxs_timrot_base + TIMROT_TIMCTRL_REG(2));

	__raw_writel(0xffff,
			mxs_timrot_base + TIMROT_FIXED_COUNT_REG(2));

	ret = request_irq(fiqdata->irq,
			cfafiq_handler,
			0,
			pdev->dev.driver->name,
			fiqdata);

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
