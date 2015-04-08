/*
 * Driver for the Himax HX-8363 LCD Controller
 *
 * Copyright 2015 Crystalfontz America, Inc.
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

#define ENTER_SLEEP_MODE	0x10
#define EXIT_SLEEP_MODE		0x11
#define SET_DISPLAY_OFF		0x28
#define SET_DISPLAY_ON		0x29
#define SET_MEMORY_ADDRESSING_MODE		0x36
#define SET_PIXEL_FORMAT		0x3a
#define SET_PIXEL_FORMAT_DBI_18BIT	(0x6)
#define SET_PIXEL_FORMAT_DPI_18BIT	(0x6 << 4)
#define SET_POWER			0xb1
#define SET_RGB_INTERFACE	0xb3
#define SET_DISPLAY_WAVEFORM_CYCLE		0xb4
#define SET_VCOM_VOLTAGE				0xb6
#define SET_EXTENSION_COMMAND		0xb9
#define SET_POWER_OPTION	0xbf
//TODO Fix command name
#define SET_PANEL_DRIVING	0xc0
#define SET_PANEL_RELATED	0xcc
#define SET_GAMMA_CURVE_RELATED		0xe0

/*
#define HX8357_SWRESET			0x01
#define HX8357_GET_RED_CHANNEL		0x06
#define HX8357_GET_GREEN_CHANNEL	0x07
#define HX8357_GET_BLUE_CHANNEL		0x08
#define HX8357_GET_POWER_MODE		0x0a
#define HX8357_GET_MADCTL		0x0b
#define HX8357_GET_PIXEL_FORMAT		0x0c
#define HX8357_GET_DISPLAY_MODE		0x0d
#define HX8357_GET_SIGNAL_MODE		0x0e
#define HX8357_GET_DIAGNOSTIC_RESULT	0x0f
#define HX8357_ENTER_PARTIAL_MODE	0x12
#define HX8357_ENTER_NORMAL_MODE	0x13
#define HX8357_EXIT_INVERSION_MODE	0x20
#define HX8357_ENTER_INVERSION_MODE	0x21
#define HX8357_SET_COLUMN_ADDRESS	0x2a
#define HX8357_SET_PAGE_ADDRESS		0x2b
#define HX8357_WRITE_MEMORY_START	0x2c
#define HX8357_READ_MEMORY_START	0x2e
#define HX8357_SET_PARTIAL_AREA		0x30
#define HX8357_SET_SCROLL_AREA		0x33
#define HX8357_SET_TEAR_OFF		0x34
#define HX8357_SET_TEAR_ON		0x35
#define HX8357_SET_SCROLL_START		0x37
#define HX8357_EXIT_IDLE_MODE		0x38
#define HX8357_ENTER_IDLE_MODE		0x39
#define HX8357_SET_PIXEL_FORMAT_DBI_3BIT	(0x1)
#define HX8357_SET_PIXEL_FORMAT_DBI_16BIT	(0x5)
#define HX8357_SET_PIXEL_FORMAT_DPI_3BIT	(0x1 << 4)
#define HX8357_SET_PIXEL_FORMAT_DPI_16BIT	(0x5 << 4)
#define HX8357_WRITE_MEMORY_CONTINUE	0x3c
#define HX8357_READ_MEMORY_CONTINUE	0x3e
#define HX8357_SET_TEAR_SCAN_LINES	0x44
#define HX8357_GET_SCAN_LINES		0x45
#define HX8369_SET_DISPLAY_BRIGHTNESS		0x51
#define HX8357_READ_DDB_START		0xa1
#define HX8369_WRITE_CABC_DISPLAY_VALUE		0x53
#define HX8369_WRITE_CABC_BRIGHT_CTRL		0x55
#define HX8369_WRITE_CABC_MIN_BRIGHTNESS	0x5e
#define HX8357_SET_DISPLAY_FRAME	0xc5
#define HX8357_SET_RGB			0xc6
#define HX8357_SET_RGB_ENABLE_HIGH		(1 << 1)
#define HX8357_SET_VCOM			0xd1
#define HX8357_SET_POWER_NORMAL		0xd2
#define HX8369_SET_GIP				0xd5
#define HX8357_SET_PANEL_RELATED	0xe9


*/
struct display_data {
	unsigned			reset;
	struct spi_device	*spi;
	int					state;
};

static u8 seq_set_panel_driving[] = {
	SET_PANEL_DRIVING, 0x41, 0x19,
};

static u8 seq_set_gamma_curve_related[] = {
	SET_GAMMA_CURVE_RELATED, 0x01, 0x07, 0x4c, 0xb0, 0x36, 0x3f, 
	0x06, 0x49, 0x51, 0x96, 0x18, 0xd8, 0x18, 0x50, 0x13, 0x01, 
	0x07, 0x4c, 0xb0, 0x36, 0x3f, 0x06, 0x49, 0x51, 0x96, 0x18, 
	0xd8, 0x18, 0x50, 0x13,
};

static u8 seq_set_panel_related[] = {
	SET_PANEL_RELATED, 0x02,
};

static u8 seq_set_memory_addressing_mode[] = {
	SET_MEMORY_ADDRESSING_MODE, 0x0a,
};

static u8 seq_set_pixel_format[] = {
	SET_PIXEL_FORMAT,
	SET_PIXEL_FORMAT_DPI_18BIT |
	SET_PIXEL_FORMAT_DBI_18BIT,
};

static u8 seq_set_extension_command[] = {
	SET_EXTENSION_COMMAND, 0xff, 0x83, 0x63,
};

static u8 seq_set_power_option[] = {
	SET_POWER_OPTION, 0x00, 0x10,
};

static u8 seq_set_rbg_interface[] = {
	SET_RGB_INTERFACE, 0x01,
};

static u8 seq_set_display_waveform_cycle[] = {
	SET_DISPLAY_WAVEFORM_CYCLE, 0x01, 0x12, 0x72, 0x12, 0x06, 
	0x03, 0x54, 0x03, 0x4e, 0x00, 0x00,
};

static u8 seq_set_vcom_voltage[] = {
	SET_VCOM_VOLTAGE, 0x33,
};

static u8 seq_set_power[] = {
	SET_POWER, 0x81, 0x30, 0x08, 0x33, 0x01, 0x13, 0x10, 0x10,
	0x35, 0x3D, 0x1A, 0x1A,
};

/*
static u8 hx8357_seq_column_address[] = {
	HX8357_SET_COLUMN_ADDRESS, 0x00, 0x00, 0x01, 0x3f,
};

static u8 hx8357_seq_page_address[] = {
	HX8357_SET_PAGE_ADDRESS, 0x00, 0x00, 0x01, 0xdf,
};

static u8 hx8357_seq_rgb[] = {
	HX8357_SET_RGB, 0x02,
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

static u8 hx8357_seq_display_frame[] = {
	HX8357_SET_DISPLAY_FRAME, 0x0c,
};

static u8 hx8357_seq_panel_related[] = {
	HX8357_SET_PANEL_RELATED, 0x01,
};

static u8 hx8357_seq_undefined1[] = {
	0xea, 0x03, 0x00, 0x00,
};

static u8 hx8357_seq_undefined2[] = {
	0xeb, 0x40, 0x54, 0x26, 0xdb,
};

static u8 hx8357_seq_power_normal[] = {
	HX8357_SET_POWER_NORMAL, 0x05, 0x12,
};

static u8 hx8357_seq_vcom[] = {
	HX8357_SET_VCOM, 0x40, 0x10,
};

static u8 hx8369_seq_display_related[] = {
	HX8369_SET_DISPLAY_MODE, 0x00, 0x2b, 0x03, 0x03, 0x70, 0x00,
	0xff, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00,	0x01,
};

static u8 hx8369_seq_gip[] = {
	HX8369_SET_GIP, 0x00, 0x01, 0x03, 0x25, 0x01, 0x02, 0x28, 0x70,
	0x11, 0x13, 0x00, 0x00, 0x40, 0x26, 0x51, 0x37, 0x00, 0x00, 0x71,
	0x35, 0x60, 0x24, 0x07, 0x0f, 0x04, 0x04,
};

static u8 hx8369_seq_gamma_curve_related[] = {
	HX8369_SET_GAMMA_CURVE_RELATED, 0x00, 0x0d, 0x19, 0x2f, 0x3b, 0x3d,
	0x2e, 0x4a, 0x08, 0x0e, 0x0f, 0x14, 0x16, 0x14, 0x14, 0x14, 0x1e,
	0x00, 0x0d, 0x19, 0x2f, 0x3b, 0x3d, 0x2e, 0x4a, 0x08, 0x0e, 0x0f,
	0x14, 0x16, 0x14, 0x14, 0x14, 0x1e,
};

*/
static int hx8363_spi_write_then_read(struct lcd_device *lcdev,
				u8 *txbuf, u16 txlen,
				u8 *rxbuf, u16 rxlen)
{
	struct display_data *lcd = lcd_get_data(lcdev);
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

static inline int spi_write_array(struct lcd_device *lcdev,
					u8 *value, u8 len)
{
	return hx8363_spi_write_then_read(lcdev, value, len, NULL, 0);
}

static inline int spi_write_byte(struct lcd_device *lcdev,
					u8 value)
{
	return hx8363_spi_write_then_read(lcdev, &value, 1, NULL, 0);
}

static int enter_standby(struct lcd_device *lcdev)
{
	int ret;

	ret = spi_write_byte(lcdev, SET_DISPLAY_OFF);
	if (ret < 0)
		return ret;

	usleep_range(10000, 12000);

	ret = spi_write_byte(lcdev, ENTER_SLEEP_MODE);
	if (ret < 0)
		return ret;

	/*
	 * The controller needs 120ms when entering in sleep mode before we can
	 * send the command to go off sleep mode
	 */
	msleep(120);

	return 0;
}

static int exit_standby(struct lcd_device *lcdev)
{
	int ret;

	ret = spi_write_byte(lcdev, EXIT_SLEEP_MODE);
	if (ret < 0)
		return ret;

	/*
	 * The controller needs 120ms when exiting from sleep mode before we
	 * can send the command to enter in sleep mode
	 */
	msleep(120);

	ret = spi_write_byte(lcdev, SET_DISPLAY_ON);
	if (ret < 0)
		return ret;

	return 0;
}

static void lcd_reset(struct lcd_device *lcdev)
{
	struct display_data *lcd = lcd_get_data(lcdev);

	/* Reset the screen */
	gpio_set_value(lcd->reset, 1);
	usleep_range(10000, 12000);
	gpio_set_value(lcd->reset, 0);
	usleep_range(10000, 12000);
	gpio_set_value(lcd->reset, 1);

	/* The controller needs 120ms to recover from reset */
	msleep(120);
}

static int lcd_init(struct lcd_device *lcdev)
{
	int ret;

	ret = spi_write_array(lcdev, seq_set_extension_command,
				ARRAY_SIZE(seq_set_extension_command));
	if (ret < 0)
		return ret;

	ret = spi_write_array(lcdev, seq_set_power,
				ARRAY_SIZE(seq_set_power));
	if (ret < 0)
		return ret;

	ret = spi_write_array(lcdev, seq_set_pixel_format,
			ARRAY_SIZE(seq_set_pixel_format));
	if (ret < 0)
		return ret;

	ret = spi_write_array(lcdev, seq_set_memory_addressing_mode,
				ARRAY_SIZE(seq_set_memory_addressing_mode));
	if (ret < 0)
		return ret;

	ret = spi_write_array(lcdev, seq_set_panel_driving,
				ARRAY_SIZE(seq_set_panel_driving));
	if (ret < 0)
		return ret;

	ret = spi_write_array(lcdev, seq_set_power_option,
				ARRAY_SIZE(seq_set_power_option));
	if (ret < 0)
		return ret;

	ret = spi_write_array(lcdev, seq_set_rbg_interface,
				ARRAY_SIZE(seq_set_rbg_interface));
	if (ret < 0)
		return ret;
		
	ret = spi_write_array(lcdev, seq_set_display_waveform_cycle,
				ARRAY_SIZE(seq_set_display_waveform_cycle));
	if (ret < 0)
		return ret;

	ret = spi_write_array(lcdev, seq_set_vcom_voltage,
				ARRAY_SIZE(seq_set_vcom_voltage));
	if (ret < 0)
		return ret;
	
	ret = spi_write_array(lcdev, seq_set_panel_related,
				ARRAY_SIZE(seq_set_panel_related));
	if (ret < 0)
		return ret;
	
	msleep(120);

	ret = spi_write_array(lcdev, seq_set_gamma_curve_related,
				ARRAY_SIZE(seq_set_gamma_curve_related));
	if (ret < 0)
		return ret;

	msleep(150);

	//	usleep_range(10000, 12000);
//
// ALL BELOW IS SHIT
//

	/*
	 * The controller needs 120ms to fully recover from exiting sleep mode
	 */
	ret = spi_write_byte(lcdev, EXIT_SLEEP_MODE);
	if (ret < 0)
		return ret;
	msleep(200);

/*
	ret = hx8357_spi_write_array(lcdev, hx8369_seq_set_display_brightness,
				ARRAY_SIZE(hx8369_seq_set_display_brightness));
	if (ret < 0)
		return ret;
*/
	ret = spi_write_byte(lcdev, SET_DISPLAY_ON);
	if (ret < 0)
		return ret;

	return 0;
}

#define POWER_IS_ON(pwr)	((pwr) <= FB_BLANK_NORMAL)

static int set_display_power_state(struct lcd_device *lcdev, int power)
{
	struct display_data *lcd = lcd_get_data(lcdev);
	int ret = 0;

	if (POWER_IS_ON(power) && !POWER_IS_ON(lcd->state))
		ret = exit_standby(lcdev);
	else if (!POWER_IS_ON(power) && POWER_IS_ON(lcd->state))
		ret = enter_standby(lcdev);

	if (ret == 0)
		lcd->state = power;
	else
		dev_warn(&lcdev->dev, "failed to set power mode %d\n", power);

	return ret;
}

static int get_display_power_state(struct lcd_device *lcdev)
{
	struct display_data *lcd = lcd_get_data(lcdev);

	return lcd->state;
}

static struct lcd_ops hx8363_ops = {
	.set_power	= set_display_power_state,
	.get_power	= get_display_power_state,
};

static const struct of_device_id hx8363_dt_ids[] = {
	{
		.compatible = "himax,hx8363",
		.data = lcd_init,
	},
	{},
};
MODULE_DEVICE_TABLE(of, hx8363_dt_ids);

static int hx8363_probe(struct spi_device *spi)
{
	struct lcd_device *lcdev;
	struct display_data *lcd;
	const struct of_device_id *match;
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

	match = of_match_device(hx8363_dt_ids, &spi->dev);
	if (!match || !match->data)
		return -EINVAL;

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

	lcdev = lcd_device_register("mxsfb", &spi->dev, lcd, &hx8363_ops);
	if (IS_ERR(lcdev)) {
		ret = PTR_ERR(lcdev);
		return ret;
	}
	spi_set_drvdata(spi, lcdev);

	lcd_reset(lcdev);

	ret = ((int (*)(struct lcd_device *))match->data)(lcdev);
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

static int hx8363_remove(struct spi_device *spi)
{
	struct lcd_device *lcdev = spi_get_drvdata(spi);

	lcd_device_unregister(lcdev);
	return 0;
}

static struct spi_driver hx8363_driver = {
	.probe  = hx8363_probe,
	.remove = hx8363_remove,
	.driver = {
		.name = "hx8363",
		.of_match_table = of_match_ptr(hx8363_dt_ids),
	},
};

module_spi_driver(hx8363_driver);

MODULE_AUTHOR("Max Roberg <max@crystalfontz.com>");
MODULE_DESCRIPTION("Himax HX-8363 LCD Driver");
MODULE_LICENSE("GPL");
