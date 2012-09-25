/*
 * Driver for the Nuvoton NAU7802 ADC
 *
 * Copyright 2013 Free Electrons
 *
 * Licensed under the GPLv2 or later.
 */

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/of_irq.h>
#include <linux/log2.h>

#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>

#define NAU7802_REG_PUCTRL	0x00
#define NAU7802_PUCTRL_RR(x)		(x << 0)
#define NAU7802_PUCTRL_RR_BIT		NAU7802_PUCTRL_RR(1)
#define NAU7802_PUCTRL_PUD(x)		(x << 1)
#define NAU7802_PUCTRL_PUD_BIT		NAU7802_PUCTRL_PUD(1)
#define NAU7802_PUCTRL_PUA(x)		(x << 2)
#define NAU7802_PUCTRL_PUA_BIT		NAU7802_PUCTRL_PUA(1)
#define NAU7802_PUCTRL_PUR(x)		(x << 3)
#define NAU7802_PUCTRL_PUR_BIT		NAU7802_PUCTRL_PUR(1)
#define NAU7802_PUCTRL_CS(x)		(x << 4)
#define NAU7802_PUCTRL_CS_BIT		NAU7802_PUCTRL_CS(1)
#define NAU7802_PUCTRL_CR(x)		(x << 5)
#define NAU7802_PUCTRL_CR_BIT		NAU7802_PUCTRL_CR(1)
#define NAU7802_PUCTRL_AVDDS(x)		(x << 7)
#define NAU7802_PUCTRL_AVDDS_BIT	NAU7802_PUCTRL_AVDDS(1)
#define NAU7802_REG_CTRL1	0x01
#define NAU7802_CTRL1_VLDO(x)		(x << 3)
#define NAU7802_CTRL1_GAINS(x)		(x)
#define NAU7802_CTRL1_GAINS_BITS	0x07
#define NAU7802_REG_CTRL2	0x02
#define NAU7802_CTRL2_CHS(x)		(x << 7)
#define NAU7802_CTRL2_CRS(x)		(x << 4)
#define NAU7802_CTRL2_CHS_BIT		NAU7802_CTRL2_CHS(1)
#define NAU7802_REG_ADC_B2	0x12
#define NAU7802_REG_ADC_B1	0x13
#define NAU7802_REG_ADC_B0	0x14
#define NAU7802_REG_ADC_CTRL	0x15

struct nau7802_state {
	struct i2c_client	*client;
	s32			last_value;
	struct mutex		lock;
	struct mutex		data_lock;
	u32			vref_mv;
	u32			conversion_count;
	u32			min_conversions;
	u8			sample_rate;
	struct completion	value_ok;
};

static int nau7802_i2c_read(struct nau7802_state *st, u8 reg, u8 *data)
{
	int ret = 0;

	ret = i2c_smbus_read_byte_data(st->client, reg);
	if (ret < 0) {
		dev_err(&st->client->dev, "failed to read from I2C\n");
		return ret;
	}

	*data = ret;

	return 0;
}

static int nau7802_i2c_write(struct nau7802_state *st, u8 reg, u8 data)
{
	return i2c_smbus_write_byte_data(st->client, reg, data);
}

static const u16 nau7802_sample_freq_avail[] = {10, 20, 40, 80,
						10, 10, 10, 320};

static ssize_t nau7802_sysfs_set_sampling_frequency(struct device *dev,
		struct device_attribute *attr,
		const char *buf,
		size_t len)
{
	struct iio_dev *idev = dev_to_iio_dev(dev);
	struct nau7802_state *st = iio_priv(idev);
	u32 val;
	int ret, i;

	ret = kstrtouint(buf, 10, &val);
	if (ret)
		return ret;

	ret = -EINVAL;
	for (i = 0; i < 8; i++)
		if (val == nau7802_sample_freq_avail[i]) {
			mutex_lock(&st->lock);
			st->sample_rate = i;
			st->conversion_count = 0;
			nau7802_i2c_write(st, NAU7802_REG_CTRL2,
					NAU7802_CTRL2_CRS(st->sample_rate));
			mutex_unlock(&st->lock);
			ret = len;
			break;
		}
	return ret;
}

static ssize_t nau7802_sysfs_get_sampling_frequency(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct iio_dev *idev = dev_to_iio_dev(dev);
	struct nau7802_state *st = iio_priv(idev);

	return sprintf(buf, "%d\n",
	       nau7802_sample_freq_avail[st->sample_rate]);
}

static IIO_DEV_ATTR_SAMP_FREQ(S_IWUSR | S_IRUGO,
		nau7802_sysfs_get_sampling_frequency,
		nau7802_sysfs_set_sampling_frequency);

static IIO_CONST_ATTR_SAMP_FREQ_AVAIL("10 40 80 320");

static IIO_CONST_ATTR(gain_available, "1 2 4 8 16 32 64 128");

static ssize_t nau7802_sysfs_set_gain(struct device *dev,
		struct device_attribute *attr,
		const char *buf,
		size_t len)
{
	struct iio_dev *idev = dev_to_iio_dev(dev);
	struct nau7802_state *st = iio_priv(idev);
	u32 val;
	u8 data;
	int ret;

	ret = kstrtouint(buf, 10, &val);
	if (ret)
		return ret;

	if (val < 1 || val > 128 || !is_power_of_2(val))
		return -EINVAL;

	mutex_lock(&st->lock);
	st->conversion_count = 0;

	ret = nau7802_i2c_read(st, NAU7802_REG_CTRL1, &data);
	if (ret < 0)
		goto nau7802_sysfs_set_gain_out;
	ret = nau7802_i2c_write(st, NAU7802_REG_CTRL1,
			(data & (~NAU7802_CTRL1_GAINS_BITS)) | ilog2(val));
nau7802_sysfs_set_gain_out:
	mutex_unlock(&st->lock);
	return ret ? ret : len;
}

static ssize_t nau7802_sysfs_get_gain(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct iio_dev *idev = dev_to_iio_dev(dev);
	struct nau7802_state *st = iio_priv(idev);
	u8 data;
	int ret;

	ret = nau7802_i2c_read(st, NAU7802_REG_CTRL1, &data);
	if (ret < 0)
		return ret;

	return sprintf(buf, "%d\n", 1 << (data & NAU7802_CTRL1_GAINS_BITS));
}

static IIO_DEVICE_ATTR(gain, S_IWUSR | S_IRUGO,
		nau7802_sysfs_get_gain,
		nau7802_sysfs_set_gain, 0);

static ssize_t nau7802_sysfs_set_min_conversions(struct device *dev,
		struct device_attribute *attr,
		const char *buf,
		size_t len)
{
	struct iio_dev *idev = dev_to_iio_dev(dev);
	struct nau7802_state *st = iio_priv(idev);
	u32 val;
	int ret;

	ret = kstrtouint(buf, 10, &val);
	if (ret)
		return ret;

	mutex_lock(&st->lock);
	st->min_conversions = val;
	st->conversion_count = 0;
	mutex_unlock(&st->lock);
	return len;
}

static ssize_t nau7802_sysfs_get_min_conversions(struct device *dev,
		struct device_attribute *attr,
		char *buf)
{
	struct iio_dev *idev = dev_to_iio_dev(dev);
	struct nau7802_state *st = iio_priv(idev);

	return sprintf(buf, "%d\n", st->min_conversions);
}

static IIO_DEVICE_ATTR(min_conversions, S_IWUSR | S_IRUGO,
		nau7802_sysfs_get_min_conversions,
		nau7802_sysfs_set_min_conversions, 0);

static struct attribute *nau7802_attributes[] = {
	&iio_dev_attr_sampling_frequency.dev_attr.attr,
	&iio_const_attr_sampling_frequency_available.dev_attr.attr,
	&iio_dev_attr_gain.dev_attr.attr,
	&iio_const_attr_gain_available.dev_attr.attr,
	&iio_dev_attr_min_conversions.dev_attr.attr,
	NULL
};

static const struct attribute_group nau7802_attribute_group = {
	.attrs = nau7802_attributes,
};

static int nau7802_read_conversion(struct nau7802_state *st)
{
	int ret;
	u8 data;

	mutex_lock(&st->data_lock);
	ret = nau7802_i2c_read(st, NAU7802_REG_ADC_B2, &data);
	if (ret)
		goto nau7802_read_conversion_out;
	st->last_value = data << 24;

	ret = nau7802_i2c_read(st, NAU7802_REG_ADC_B1, &data);
	if (ret)
		goto nau7802_read_conversion_out;
	st->last_value |= data << 16;

	ret = nau7802_i2c_read(st, NAU7802_REG_ADC_B0, &data);
	if (ret)
		goto nau7802_read_conversion_out;
	st->last_value |= data << 8;

	st->last_value >>= 8;

nau7802_read_conversion_out:
	mutex_unlock(&st->data_lock);
	return ret;
}

static int nau7802_sync(struct nau7802_state *st)
{
	int ret;
	u8 data;

	ret = nau7802_i2c_read(st, NAU7802_REG_PUCTRL, &data);
	if (ret)
		return ret;
	ret = nau7802_i2c_write(st, NAU7802_REG_PUCTRL,
				data | NAU7802_PUCTRL_CS_BIT);
	return ret;
}

static irqreturn_t nau7802_eoc_trigger(int irq, void *private)
{
	struct iio_dev *idev = private;
	struct nau7802_state *st = iio_priv(idev);
	u8 status;
	int ret;

	ret = nau7802_i2c_read(st, NAU7802_REG_PUCTRL, &status);
	if (ret)
		return IRQ_HANDLED;

	if (!(status & NAU7802_PUCTRL_CR_BIT))
		return IRQ_NONE;

	if (nau7802_read_conversion(st))
		return IRQ_HANDLED;

	/* wait for enough conversions before getting a stable value when
	 * changing channels */
	if (st->conversion_count < st->min_conversions)
		st->conversion_count++;
	if (st->conversion_count >= st->min_conversions)
		complete_all(&st->value_ok);
	return IRQ_HANDLED;
}

static int nau7802_read_irq(struct iio_dev *idev,
			struct iio_chan_spec const *chan,
			int *val)
{
	struct nau7802_state *st = iio_priv(idev);
	int ret;

	INIT_COMPLETION(st->value_ok);
	enable_irq(st->client->irq);

	nau7802_sync(st);

	/* read registers to ensure we flush everything */
	ret = nau7802_read_conversion(st);
	if (ret)
		goto read_chan_info_failure;

	/* Wait for a conversion to finish */
	ret = wait_for_completion_interruptible_timeout(&st->value_ok,
			msecs_to_jiffies(1000));
	if (ret == 0)
		ret = -ETIMEDOUT;

	if (ret < 0)
		goto read_chan_info_failure;

	disable_irq(st->client->irq);

	*val = st->last_value;
	return IIO_VAL_INT;

read_chan_info_failure:
	disable_irq(st->client->irq);
	return ret;
}

static int nau7802_read_poll(struct iio_dev *idev,
			struct iio_chan_spec const *chan,
			int *val)
{
	struct nau7802_state *st = iio_priv(idev);
	int ret;
	u8 data;

	nau7802_sync(st);

	/* read registers to ensure we flush everything */
	ret = nau7802_read_conversion(st);
	if (ret)
		return ret;

	do {
		nau7802_i2c_read(st, NAU7802_REG_PUCTRL, &data);
		if (ret)
			return ret;
		while (!(data & NAU7802_PUCTRL_CR_BIT)) {
			if (st->sample_rate != 0x07)
				msleep(20);
			else
				mdelay(4);
			nau7802_i2c_read(st, NAU7802_REG_PUCTRL, &data);
			if (ret)
				return ret;
		}

		nau7802_read_conversion(st);
		if (ret)
			return ret;
		if (st->conversion_count < st->min_conversions)
			st->conversion_count++;
	} while (st->conversion_count < st->min_conversions);

	*val = st->last_value;
	return IIO_VAL_INT;
}

static int nau7802_read_raw(struct iio_dev *idev,
			    struct iio_chan_spec const *chan,
			    int *val, int *val2, long mask)
{
	struct nau7802_state *st = iio_priv(idev);
	u8 data;
	int ret;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		mutex_lock(&st->lock);
		/*
		 * Select the channel to use
		 *   - Channel 1 is value 0 in the CHS register
		 *   - Channel 2 is value 1 in the CHS register
		 */
		ret = nau7802_i2c_read(st, NAU7802_REG_CTRL2, &data);
		if (ret < 0)
			return ret;
		if (((data & NAU7802_CTRL2_CHS_BIT) && !chan->channel) ||
				(!(data & NAU7802_CTRL2_CHS_BIT) &&
				 chan->channel)) {
			st->conversion_count = 0;
			ret = nau7802_i2c_write(st, NAU7802_REG_CTRL2,
					NAU7802_CTRL2_CHS(chan->channel) |
					NAU7802_CTRL2_CRS(st->sample_rate));
			if (ret < 0)
				return ret;
		}

		if (st->client->irq)
			ret = nau7802_read_irq(idev, chan, val);
		else
			ret = nau7802_read_poll(idev, chan, val);

		mutex_unlock(&st->lock);
		return ret;
	case IIO_CHAN_INFO_SCALE:
		ret = nau7802_i2c_read(st, NAU7802_REG_CTRL1, &data);
		if (ret < 0)
			return ret;

		*val = 0;
		*val2 = (((u64)st->vref_mv) * 1000000000ULL) >>
			(chan->scan_type.realbits +
			 (data & NAU7802_CTRL1_GAINS_BITS));
		return IIO_VAL_INT_PLUS_MICRO;
	default:
		break;
	}
	return -EINVAL;
}

static const struct iio_info nau7802_info = {
	.driver_module = THIS_MODULE,
	.read_raw = &nau7802_read_raw,
	.attrs = &nau7802_attribute_group,
};

static int nau7802_channel_init(struct iio_dev *idev)
{
	struct iio_chan_spec *chan_array;
	int i;

	idev->num_channels = 2;

	chan_array = devm_kzalloc(&idev->dev,
				  sizeof(*chan_array) * idev->num_channels,
				  GFP_KERNEL);

	for (i = 0; i < idev->num_channels; i++) {
		struct iio_chan_spec *chan = chan_array + i;

		chan->type = IIO_VOLTAGE;
		chan->indexed = 1;
		chan->channel = i;
		chan->scan_index = i;
		chan->scan_type.sign = 's';
		chan->scan_type.realbits = 23;
		chan->scan_type.storagebits = 24;
		chan->info_mask = IIO_CHAN_INFO_SCALE_SHARED_BIT |
			IIO_CHAN_INFO_RAW_SEPARATE_BIT;
	}
	idev->channels = chan_array;

	return idev->num_channels;
}

static int nau7802_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct iio_dev *idev;
	struct nau7802_state *st;
	struct device_node *np = client->dev.of_node;
	int ret;
	u8 data;
	u32 tmp;

	if (!client->dev.of_node) {
		dev_err(&client->dev, "No device tree node available.\n");
		return -EINVAL;
	}

	/*
	 * If we have some interrupts declared, use them, if not, fall back
	 * on polling.
	 */
	if (of_find_property(np, "interrupts", NULL)) {
		if (client->irq <= 0) {
			client->irq = irq_of_parse_and_map(np, 0);
			if (client->irq <= 0)
				return -EPROBE_DEFER;
		}
	} else
		client->irq = 0;

	idev = iio_device_alloc(sizeof(struct nau7802_state));
	if (idev == NULL)
		return -ENOMEM;

	st = iio_priv(idev);

	i2c_set_clientdata(client, idev);

	idev->dev.parent = &client->dev;
	idev->name = dev_name(&client->dev);
	idev->modes = INDIO_DIRECT_MODE;
	idev->info = &nau7802_info;

	st->client = client;

	/* Reset the device */
	nau7802_i2c_write(st, NAU7802_REG_PUCTRL, NAU7802_PUCTRL_RR_BIT);

	/* Enter normal operation mode */
	nau7802_i2c_write(st, NAU7802_REG_PUCTRL, NAU7802_PUCTRL_PUD_BIT);

	/*
	 * After about 200 usecs, the device should be ready and then
	 * the Power Up bit will be set to 1. If not, wait for it.
	 */
	do {
		udelay(200);
		ret = nau7802_i2c_read(st, NAU7802_REG_PUCTRL, &data);
		if (ret)
			return -ENODEV;
	} while (!(data & NAU7802_PUCTRL_PUR_BIT));

	of_property_read_u32(np, "nuvoton,vldo", &tmp);
	st->vref_mv = tmp;

	data = NAU7802_PUCTRL_PUD_BIT | NAU7802_PUCTRL_PUA_BIT |
		NAU7802_PUCTRL_CS_BIT;
	if (tmp >= 2400)
		data |= NAU7802_PUCTRL_AVDDS_BIT;

	nau7802_i2c_write(st, NAU7802_REG_PUCTRL, data);
	nau7802_i2c_write(st, NAU7802_REG_ADC_CTRL, 0x30);

	if (tmp >= 2400) {
		data = NAU7802_CTRL1_VLDO((4500 - tmp) / 300);
		nau7802_i2c_write(st, NAU7802_REG_CTRL1, data);
	}

	st->min_conversions = 6;

	/*
	 * The ADC fires continuously and we can't do anything about
	 * it. So we need to have the IRQ disabled by default, and we
	 * will enable them back when we will need them..
	 */
	if (client->irq) {
		irq_set_status_flags(client->irq, IRQ_NOAUTOEN);
		ret = request_threaded_irq(client->irq,
				NULL,
				nau7802_eoc_trigger,
				IRQF_TRIGGER_HIGH | IRQF_ONESHOT,
				client->dev.driver->name,
				idev);
		if (ret) {
			/*
			 * What may happen here is that our IRQ controller is
			 * not able to get level interrupt but this is required
			 * by this ADC as when going over 40 sample per second,
			 * the interrupt line may stay high between conversions.
			 * So, we continue no matter what but we switch to
			 * polling mode.
			 */
			dev_info(&client->dev,
				"Failed to allocate IRQ, using polling mode\n");
			client->irq = 0;
			/*
			 * We are polling, use the fastest sample rate by
			 * default
			 */
			st->sample_rate = 0x7;
			nau7802_i2c_write(st, NAU7802_REG_CTRL2,
					NAU7802_CTRL2_CRS(st->sample_rate));
		}
	}

	/* Setup the ADC channels available on the board */
	ret = nau7802_channel_init(idev);
	if (ret < 0) {
		dev_err(&client->dev, "Couldn't initialize the channels.\n");
		goto error_channel_init;
	}

	init_completion(&st->value_ok);
	mutex_init(&st->lock);
	mutex_init(&st->data_lock);

	ret = iio_device_register(idev);
	if (ret < 0) {
		dev_err(&client->dev, "Couldn't register the device.\n");
		goto error_device_register;
	}

	return 0;

error_device_register:
	mutex_destroy(&st->lock);
error_channel_init:
	if (client->irq)
		free_irq(client->irq, idev);
	iio_device_free(idev);
	return ret;
}

static int nau7802_remove(struct i2c_client *client)
{
	struct iio_dev *idev = i2c_get_clientdata(client);
	struct nau7802_state *st = iio_priv(idev);

	iio_device_unregister(idev);
	mutex_destroy(&st->lock);
	mutex_destroy(&st->data_lock);
	if (client->irq)
		free_irq(client->irq, idev);
	iio_device_free(idev);

	return 0;
}

static const struct i2c_device_id nau7802_i2c_id[] = {
	{ "nau7802", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, nau7802_i2c_id);

static const struct of_device_id nau7802_dt_ids[] = {
	{ .compatible = "nuvoton,nau7802" },
	{},
};
MODULE_DEVICE_TABLE(of, nau7802_dt_ids);

static struct i2c_driver nau7802_driver = {
	.probe = nau7802_probe,
	.remove = nau7802_remove,
	.id_table = nau7802_i2c_id,
	.driver = {
		   .name = "nau7802",
		   .of_match_table = of_match_ptr(nau7802_dt_ids),
	},
};

module_i2c_driver(nau7802_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Nuvoton NAU7802 ADC Driver");
MODULE_AUTHOR("Maxime Ripard <maxime.ripard@free-electrons.com>");
MODULE_AUTHOR("Alexandre Belloni <alexandre.belloni@free-electrons.com>");
