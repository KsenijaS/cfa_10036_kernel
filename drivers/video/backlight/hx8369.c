/*
 * Driver for the Himax HX-8369 LCD Controller
 *
 * Copyright 2012 Crystalfontz America, Inc.
 *
 * Licensed under the GPLv2 or later.
 */

#include <linux/delay.h>
#include <linux/lcd.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/spi/spi.h>

#define HX8369_SWRESET					0x01
#define HX8369_GET_RED_CHANNEL			0x06
#define HX8369_GET_GREEN_CHANNEL		0x07
#define HX8369_GET_BLUE_CHANNEL			0x08
#define HX8369_GET_POWER_MODE			0x0a
#define HX8369_GET_MADCTL				0x0b
#define HX8369_GET_PIXEL_FORMAT			0x0c
#define HX8369_GET_DISPLAY_MODE			0x0d
#define HX8369_GET_SIGNAL_MODE			0x0e
#define HX8369_GET_DIAGNOSTIC_RESULT	0x0f
#define HX8369_ENTER_SLEEP_MODE			0x10
#define HX8369_EXIT_SLEEP_MODE			0x11
#define HX8369_ENTER_PARTIAL_MODE		0x12
#define HX8369_ENTER_NORMAL_MODE		0x13
#define HX8369_EXIT_INVERSION_MODE		0x20
#define HX8369_ENTER_INVERSION_MODE		0x21
#define HX8369_SET_GAMMA_CURVE			0x26
#define HX8369_SET_DISPLAY_OFF			0x28
#define HX8369_SET_DISPLAY_ON			0x29
#define HX8369_SET_COLUMN_ADDRESS		0x2a
#define HX8369_SET_PAGE_ADDRESS			0x2b
#define HX8369_WRITE_MEMORY_START		0x2c
#define HX8369_SET_COLOR_DEPTH			0x2d
#define HX8369_READ_MEMORY_START		0x2e
#define HX8369_SET_PARTIAL_AREA			0x30
#define HX8369_SET_SCROLL_AREA			0x33
#define HX8369_SET_TEAR_OFF				0x34
#define HX8369_SET_TEAR_ON				0x35
#define HX8369_SET_ADDRESS_MODE			0x36
#define HX8369_SET_SCROLL_START			0x37
#define HX8369_EXIT_IDLE_MODE			0x38
#define HX8369_ENTER_IDLE_MODE			0x39
#define HX8369_SET_PIXEL_FORMAT			0x3a
#define HX8369_SET_PIXEL_FORMAT_DBI_3BIT	(0x1)
#define HX8369_SET_PIXEL_FORMAT_DBI_16BIT	(0x5)
#define HX8369_SET_PIXEL_FORMAT_DBI_18BIT	(0x6)
#define HX8369_SET_PIXEL_FORMAT_DBI_24BIT	(0x7)
#define HX8369_SET_PIXEL_FORMAT_DPI_3BIT	(0x1 << 4)
#define HX8369_SET_PIXEL_FORMAT_DPI_16BIT	(0x5 << 4)
#define HX8369_SET_PIXEL_FORMAT_DPI_18BIT	(0x6 << 4)
#define HX8369_SET_PIXEL_FORMAT_DPI_24BIT	(0x7 << 4)
#define HX8369_WRITE_MEMORY_CONTINUE	0x3c
#define HX8369_READ_MEMORY_CONTINUE		0x3e
#define HX8369_SET_TEAR_SCAN_LINES		0x44
#define HX8369_GET_SCAN_LINES			0x45
#define HX8369_SET_DISPLAY_BRIGHTNESS	0x51
#define HX8369_GET_DISPLAY_BRIGHTNESS	0x52
#define HX8369_WRITE_CABC_DISPLAY_VALUE	0x53
#define HX8369_READ_CABC_DISPLAY_VALUE	0x54
#define HX8369_WRITE_CABC_BRIGHT_CTRL	0x55
#define HX8369_READ_CABC_BRIGHT_CTRL	0x56
#define HX8369_WRITE_CABC_MIN_BRIGHTNESS	0x5e
#define HX8369_READ_CABC_MIN_BRIGHTNESS	0x5f
#define HX8369_READ_AUTO_CABC_DIAG_RSLT	0x68
#define HX8369_READ_DDB_START			0xa1
#define HX8369_READ_DDB_CONTINUE		0xa8
#define HX8369_SET_INTERNAL_OSCILLATOR	0xb0
#define HX8369_SET_POWER				0xb1
#define HX8369_SET_DISPLAY_MODE			0xb2
#define HX8369_SET_DISPLAY_MODE_RGB_THROUGH	(0x3)
#define HX8369_SET_DISPLAY_MODE_RGB_INTERFACE	(1 << 4)
#define HX8369_SET_RGB					0xb3
#define HX8369_SET_RGB_ENABLE_HIGH		0x1
#define HX8369_SET_DISPLAY_WAVEFORM_CYC 0xb4
#define HX8369_SET_VCOM					0xb6
#define HX8369_SET_EXTENSION_COMMAND	0xb9
#define HX8369_SET_ID					0xc3
#define HX8369_SET_PANEL				0xcc
#define HX8369_SET_GIP					0xd5
#define HX8369_SET_TEMP_SENSOR_CTRL		0xd8
#define HX8369_READ_ID1					0xda
#define HX8369_READ_ID2					0xdb
#define HX8369_READ_ID3					0xdc
#define HX8369_SET_GAMMA_CURVE_RELATED	0xe0
#define HX8369_GET_LCD_CTRLR_ID_VERSION	0xf4

struct hx8369_data {
	unsigned		reset;
	struct spi_device	*spi;
	int			state;
};

static u8 hx8369_seq_write_CABC_min_brightness[] = {
	HX8369_WRITE_CABC_MIN_BRIGHTNESS, 0x00,
};

static u8 hx8369_seq_write_CABC_control[] = {
	HX8369_WRITE_CABC_DISPLAY_VALUE, 0x24,
};

static u8 hx8369_seq_set_display_brightness[] = {
	HX8369_SET_DISPLAY_BRIGHTNESS, 0xFF,
};

static u8 hx8369_seq_write_CABC_control_setting[] = {
	HX8369_WRITE_CABC_BRIGHT_CTRL, 0x02,
};

static u8 hx8369_seq_extension_command[] = {
	HX8369_SET_EXTENSION_COMMAND, 0xff, 0x83, 0x69,
};

static u8 hx8369_seq_display_related[] = {
	HX8369_SET_DISPLAY_MODE, 0x00, 0x2b, 0x03, 0x03, 0x70, 0x00, 
	0xff, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00,	0x01,
};

static u8 hx8369_seq_panel_waveform_cycle[] = {
	HX8369_SET_DISPLAY_WAVEFORM_CYC, 0x0a, 0x1d, 0x80, 0x06, 0x02,
};

static u8 hx8369_seq_set_address_mode[] = {
	HX8369_SET_ADDRESS_MODE, 0x00,
};

static u8 hx8369_seq_vcom[] = {
	HX8369_SET_VCOM, 0x3e, 0x3e,
};

static u8 hx8369_seq_gip[] = {
	HX8369_SET_GIP, 0x00, 0x01, 0x03, 0x25, 0x01, 0x02, 0x28, 0x70,
	0x11, 0x13, 0x00, 0x00, 0x40, 0x26, 0x51, 0x37, 0x00, 0x00, 0x71,
	0x35, 0x60, 0x24, 0x07, 0x0f, 0x04, 0x04,
};

static u8 hx8369_seq_power[] = {
	HX8369_SET_POWER, 0x01, 0x00, 0x34, 0x03, 0x00, 0x11, 0x11, 0x32,
	0x2f, 0x3f, 0x3f, 0x01, 0x3a, 0x01, 0xe6, 0xe6, 0xe6, 0xe6, 0xe6,
};

static u8 hx8369_seq_gamma_curve_related[] = {
	HX8369_SET_GAMMA_CURVE_RELATED, 0x00, 0x0d, 0x19, 0x2f, 0x3b, 0x3d,
	0x2e, 0x4a, 0x08, 0x0e, 0x0f, 0x14, 0x16, 0x14, 0x14, 0x14, 0x1e,
	0x00, 0x0d, 0x19, 0x2f, 0x3b, 0x3d, 0x2e, 0x4a, 0x08, 0x0e, 0x0f,
	0x14, 0x16, 0x14, 0x14, 0x14, 0x1e,
};

static int hx8369_spi_write_then_read(struct lcd_device *lcdev,
				u8 *txbuf, u16 txlen,
				u8 *rxbuf, u16 rxlen)
{
	struct hx8369_data *lcd = lcd_get_data(lcdev);
	struct spi_message msg;
	struct spi_transfer xfer[2];
	u16 *local_txbuf = NULL;
	int ret = 0;

	memset(xfer, 0, sizeof(xfer));
	spi_message_init(&msg);

	if (txlen) {
		int i;

		local_txbuf = kcalloc(txlen, sizeof(*local_txbuf), GFP_KERNEL);

		if (!local_txbuf)
			return -ENOMEM;

		for (i = 0; i < txlen; i++) {
			local_txbuf[i] = txbuf[i];
			if (i > 0)
				local_txbuf[i] |= 1 << 8;
		}

		xfer[0].len = 2 * txlen;
		xfer[0].bits_per_word = 9;
		xfer[0].tx_buf = local_txbuf;
		spi_message_add_tail(&xfer[0], &msg);
	}

	if (rxlen) {
		xfer[1].len = rxlen;
		xfer[1].bits_per_word = 8;
		xfer[1].rx_buf = rxbuf;
		spi_message_add_tail(&xfer[1], &msg);
	}

	ret = spi_sync(lcd->spi, &msg);
	if (ret < 0)
		dev_err(&lcdev->dev, "Couldn't send SPI data\n");

	if (txlen)
		kfree(local_txbuf);

	return ret;
}

static inline int hx8369_spi_write_array(struct lcd_device *lcdev,
					u8 *value, u8 len)
{
	return hx8369_spi_write_then_read(lcdev, value, len, NULL, 0);
}

static inline int hx8369_spi_write_byte(struct lcd_device *lcdev,
					u8 value)
{
	return hx8369_spi_write_then_read(lcdev, &value, 1, NULL, 0);
}

static int hx8369_enter_standby(struct lcd_device *lcdev)
{
	int ret;

	ret = hx8369_spi_write_byte(lcdev, HX8369_SET_DISPLAY_OFF);
	if (ret < 0)
		return ret;

	usleep_range(10000, 12000);

	ret = hx8369_spi_write_byte(lcdev, HX8369_ENTER_SLEEP_MODE);
	if (ret < 0)
		return ret;

	msleep(120);

	return 0;
}

static int hx8369_exit_standby(struct lcd_device *lcdev)
{
	int ret;

	ret = hx8369_spi_write_byte(lcdev, HX8369_EXIT_SLEEP_MODE);
	if (ret < 0)
		return ret;

	msleep(120);

	ret = hx8369_spi_write_byte(lcdev, HX8369_SET_DISPLAY_ON);
	if (ret < 0)
		return ret;

	return 0;
}

static int hx8369_lcd_init(struct lcd_device *lcdev)
{
	struct hx8369_data *lcd = lcd_get_data(lcdev);
	int ret;

	/* Reset the screen */
	gpio_set_value(lcd->reset, 1);
	usleep_range(10000, 12000);
	gpio_set_value(lcd->reset, 0);
	usleep_range(10000, 12000);
	gpio_set_value(lcd->reset, 1);
	msleep(120);
	
	ret = hx8369_spi_write_array(lcdev, hx8369_seq_extension_command,
				ARRAY_SIZE(hx8369_seq_extension_command));
	if (ret < 0)
		return ret;
	msleep(10);

	ret = hx8369_spi_write_array(lcdev, hx8369_seq_display_related,
				ARRAY_SIZE(hx8369_seq_display_related));
	if (ret < 0)
		return ret;
		
	ret = hx8369_spi_write_array(lcdev, hx8369_seq_panel_waveform_cycle,
				ARRAY_SIZE(hx8369_seq_panel_waveform_cycle));
	if (ret < 0)
		return ret;

	ret = hx8369_spi_write_array(lcdev, hx8369_seq_set_address_mode,
				ARRAY_SIZE(hx8369_seq_set_address_mode));
	if (ret < 0)
		return ret;

	ret = hx8369_spi_write_array(lcdev, hx8369_seq_vcom,
				ARRAY_SIZE(hx8369_seq_vcom));
	if (ret < 0)
		return ret;

	ret = hx8369_spi_write_array(lcdev, hx8369_seq_gip,
				ARRAY_SIZE(hx8369_seq_gip));
	if (ret < 0)
		return ret;

	ret = hx8369_spi_write_array(lcdev, hx8369_seq_power,
				ARRAY_SIZE(hx8369_seq_power));
	if (ret < 0)
		return ret;

	ret = hx8369_spi_write_byte(lcdev, HX8369_EXIT_SLEEP_MODE);
	if (ret < 0)
		return ret;

	msleep(120);

	ret = hx8369_spi_write_array(lcdev, hx8369_seq_gamma_curve_related,
				ARRAY_SIZE(hx8369_seq_gamma_curve_related));
	if (ret < 0)
		return ret;

	ret = hx8369_spi_write_byte(lcdev, HX8369_EXIT_SLEEP_MODE);
	if (ret < 0)
		return ret;
	msleep(1);

	ret = hx8369_spi_write_array(lcdev, hx8369_seq_write_CABC_control,
				ARRAY_SIZE(hx8369_seq_write_CABC_control));
	if (ret < 0)
		return ret;
	msleep(10);

	ret = hx8369_spi_write_array(lcdev, hx8369_seq_write_CABC_control_setting,
				ARRAY_SIZE(hx8369_seq_write_CABC_control_setting));
	if (ret < 0)
		return ret;

	ret = hx8369_spi_write_array(lcdev, hx8369_seq_write_CABC_min_brightness,
				ARRAY_SIZE(hx8369_seq_write_CABC_min_brightness));
	if (ret < 0)
		return ret;
	msleep(10);

	ret = hx8369_spi_write_array(lcdev, hx8369_seq_set_display_brightness,
				ARRAY_SIZE(hx8369_seq_set_display_brightness));
	if (ret < 0)
		return ret;
	
	ret = hx8369_spi_write_byte(lcdev, HX8369_SET_DISPLAY_ON);
	if (ret < 0)
		return ret;
	msleep(100);

	return 0;
}

#define POWER_IS_ON(pwr)	((pwr) <= FB_BLANK_NORMAL)

static int hx8369_set_power(struct lcd_device *lcdev, int power)
{
	struct hx8369_data *lcd = lcd_get_data(lcdev);
	int ret = 0;

	if (POWER_IS_ON(power) && !POWER_IS_ON(lcd->state))
		ret = hx8369_exit_standby(lcdev);
	else if (!POWER_IS_ON(power) && POWER_IS_ON(lcd->state))
		ret = hx8369_enter_standby(lcdev);

	if (ret == 0)
		lcd->state = power;
	else
		dev_warn(&lcdev->dev, "failed to set power mode %d\n", power);

	return ret;
}

static int hx8369_get_power(struct lcd_device *lcdev)
{
	struct hx8369_data *lcd = lcd_get_data(lcdev);

	return lcd->state;
}

static struct lcd_ops hx8369_ops = {
	.set_power	= hx8369_set_power,
	.get_power	= hx8369_get_power,
};

static int hx8369_probe(struct spi_device *spi)
{
	struct lcd_device *lcdev;
	struct hx8369_data *lcd;
	int ret;

	lcd = devm_kzalloc(&spi->dev, sizeof(*lcd), GFP_KERNEL);
	if (!lcd) {
		dev_err(&spi->dev, "Couldn't allocate lcd internal structure!\n");
		return -ENOMEM;
	}

	ret = spi_setup(spi);
	if (ret < 0) {
		dev_err(&spi->dev, "SPI setup failed.\n");
		return ret;
	}

	lcd->spi = spi;

	lcd->reset = of_get_named_gpio(spi->dev.of_node, "gpios-reset", 0);
	if (!gpio_is_valid(lcd->reset)) {
		dev_err(&spi->dev, "Missing dt property: gpios-reset\n");
		return -EINVAL;
	}

	ret = devm_gpio_request_one(&spi->dev, lcd->reset,
				    GPIOF_OUT_INIT_HIGH,
				    "hx8369-reset");
	if (ret) {
		dev_err(&spi->dev,
			"failed to request gpio %d: %d\n",
			lcd->reset, ret);
		return -EINVAL;
	}

	lcdev = lcd_device_register("mxsfb", &spi->dev, lcd, &hx8369_ops);
	if (IS_ERR(lcdev)) {
		ret = PTR_ERR(lcdev);
		return ret;
	}
	spi_set_drvdata(spi, lcdev);

	ret = hx8369_lcd_init(lcdev);
	if (ret) {
		dev_err(&spi->dev, "Couldn't initialize panel\n");
		goto init_error;
	}

	dev_info(&spi->dev, "Panel probed\n");

	return 0;

init_error:
	lcd_device_unregister(lcdev);
	return ret;
}

static int hx8369_remove(struct spi_device *spi)
{
	struct lcd_device *lcdev = spi_get_drvdata(spi);

	lcd_device_unregister(lcdev);
	return 0;
}

static const struct of_device_id hx8369_dt_ids[] = {
	{ .compatible = "himax,hx8369" },
	{},
};
MODULE_DEVICE_TABLE(of, hx8369_dt_ids);

static struct spi_driver hx8369_driver = {
	.probe  = hx8369_probe,
	.remove = hx8369_remove,
	.driver = {
		.name = "hx8369",
		.of_match_table = of_match_ptr(hx8369_dt_ids),
	},
};

module_spi_driver(hx8369_driver);

MODULE_AUTHOR("Brian Lilly <brian@crystalfontz.com>");
MODULE_DESCRIPTION("Himax HX-8369 LCD Driver");
MODULE_LICENSE("GPL");
