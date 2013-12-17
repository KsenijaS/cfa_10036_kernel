/*
 * Driver for the Raydium RM68120 LCD Controller
 *
 * Copyright 2013 Crystalfontz America, Inc.
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
/*
#define RM68120_NOOP							0x0000
#define RM68120_SWRESET							0x0100
#define RM68120_MANUFACTURE_ID					0x0400
#define RM68120_DRIVER_VERSION_ID				0x0401
#define RM68120_DRIVER_ID						0x0402
#define RM68120_RD_NUM_ERRORS_DSI				0x0500
#define RM68120_GET_POWER_MODE					0x0a00
#define RM68120_GET_MADCTR						0x0b00
#define RM68120_GET_PIXEL_FORMAT				0x0c00
#define RM68120_GET_DISPLAY_MODE				0x0d00
#define RM68120_GET_SIGNAL_MODE					0x0e00
#define RM68120_GET_DIAGNOSTIC_RESULT			0x0f00
#define RM68120_ENTER_SLEEP_MODE				0x1000
#define RM68120_EXIT_SLEEP_MODE					0x1100
#define RM68120_ENTER_PARTIAL_MODE				0x1200
#define RM68120_ENTER_NORMAL_MODE				0x1300
#define RM68120_EXIT_INVERSION_MODE				0x2000
#define RM68120_ENTER_INVERSION_MODE			0x2100
#define RM68120_ALL_PIXELS_OFF					0x2200
#define RM68120_ALL_PIXELS_ON					0x2300
#define RM68120_SET_GAMMA_CURVE					0x2600
#define RM68120_SET_DISPLAY_OFF					0x2800
#define RM68120_SET_DISPLAY_ON					0x2900
#define RM68120_SET_COLUMN_ADDRESS				0x2a00
#define RM68120_SET_PAGE_ADDRESS				0x2b00
#define RM68120_WRITE_MEMORY_START				0x2c00
#define RM68120_READ_MEMORY_START				0x2e00
#define RM68120_SET_PARTIAL_AREA				0x3000
#define RM68120_SET_TEAR_OFF					0x3400
#define RM68120_SET_TEAR_ON						0x3500
#define RM68120_SET_ADDRESS_MODE				0x3600
#define RM68120_EXIT_IDLE_MODE					0x3800
#define RM68120_ENTER_IDLE_MODE					0x3900
#define RM68120_SET_PIXEL_FORMAT				0x3a00
#define RM68120_SET_PIXEL_FORMAT_DBI_16BIT	   (0x5)
#define RM68120_SET_PIXEL_FORMAT_DBI_18BIT	   (0x6)
#define RM68120_SET_PIXEL_FORMAT_DBI_24BIT	   (0x7)
#define RM68120_SET_PIXEL_FORMAT_DPI_16BIT	   (0x5 << 4)
#define RM68120_SET_PIXEL_FORMAT_DPI_18BIT	   (0x6 << 4)
#define RM68120_SET_PIXEL_FORMAT_DPI_24BIT	   (0x7 << 4)
#define RM68120_WRITE_MEMORY_CONTINUE			0x3c00
#define RM68120_READ_MEMORY_CONTINUE			0x3e00
#define RM68120_SET_TEAR_SCAN_LINES				0x4400
#define RM68120_GET_SCAN_LINES					0x4500
#define RM68120_DEEP_STANDBY_MODE				0x4f00
#define RM68120_DEEP_STANDBY_MODE_ON		   (0x1)
#define RM68120_DEEP_STANDBY_MODE_OFF		   (0x0)
#define RM68120_WRITE_DISPLAY_PROFILE			0x5000
#define RM68120_SET_DISPLAY_BRIGHTNESS_CTL_BLK  0x5100
#define RM68120_GET_DISPLAY_BRIGHTNESS_CTL_BLK	0x5200
#define RM68120_WRITE_CABC_DISPLAY_VALUE		0x5300
#define RM68120_READ_CABC_DISPLAY_VALUE			0x5400
#define RM68120_WRITE_CABC_BRIGHT_CTRL			0x5500
#define RM68120_READ_CABC_BRIGHT_CTRL			0x5600
#define RM68120_WRITE_HYSTERESIS_FILTER			0x5700
#define RM68120_WRITE_GAMMA_SETTING				0x5800
#define RM68120_READ_FRT_LGT_SNSR_MSB			0x5a00
#define RM68120_READ_FRT_LGT_SNSR_LSB			0x5b00
#define RM68120_READ_FRT_SNSR_MEDIAN_MSB		0x5c00
#define RM68120_READ_FRT_SNSR_MEDIAN_LSB		0x5d00
#define RM68120_WRITE_CABC_MIN_BRIGHTNESS		0x5e00
#define RM68120_READ_CABC_MIN_BRIGHTNESS		0x5f00
#define RM68120_WRITE_LGT_SNSR_COMP_COEF_VAL	0x6500
#define RM68120_READ_LGT_SNSR_COMP_COEF_VAL_MSB	0x6600
#define RM68120_READ_LGT_SNSR_COMP_COEF_VAL_LSB	0x6700
#define RM68120_READ_BLK_WHT_LOW_BITS			0x7000
#define RM68120_READ_BLK_BKX					0x7100
#define RM68120_READ_BLK_BKY					0x7200
#define RM68120_READ_WHT_WX						0x7300
#define RM68120_READ_WHT_WY						0x7400
#define RM68120_READ_RED_GRN_LOW_BITS			0x7500
#define RM68120_READ_RED_RX						0x7600
#define RM68120_READ_RED_RY						0x7700
#define RM68120_READ_GRN_GX						0x7800
#define RM68120_READ_GRN_GY						0x7900
#define RM68120_READ_BLUE_ALPHA_LOW_BITS		0x7a00
#define RM68120_READ_BLUE_BX					0x7b00
#define RM68120_READ_BLUE_BY					0x7c00
#define RM68120_READ_ALPHA_AX					0x7d00
#define RM68120_READ_ALPHA_AY					0x7e00
#define RM68120_READ_DDB_START					0xa100
#define RM68120_READ_DDB_CONTINUE				0xa800
#define RM68120_READ_FIRST_CHECKSUM				0xaa00
#define RM68120_READ_CONT_CHECKSUM				0xaf00
#define RM68120_READ_ID1						0xda00
#define RM68120_READ_ID2						0xdb00
#define RM68120_READ_ID3						0xdc00
*/

#define WRITE_FIRST_TRANS 0x20
#define WRITE_SECOND_TRANS 0x00
#define WRITE_THIRD_TRANS 0x40
#define READ_FIRST_TRANS 0x20
#define READ_SECOND_TRANS 0x00
#define READ_THIRD_TRANS 0xC0


struct rm68120_data {
	unsigned			reset;
	struct spi_device	*spi;
	int					state;
};

static int RM68120_spi_write_2bytes(struct lcd_device *lcdev, 
	unsigned char reg_high_addr, unsigned char reg_low_addr)
{
	char tx_buf[4];
	int ret = 0;
	struct rm68120_data *lcd = lcd_get_data(lcdev);
	struct spi_message msg;
	struct spi_transfer xfer;
	
	memset(&xfer, 0, sizeof xfer);
	xfer.tx_buf = tx_buf;
	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);
	
	tx_buf[0] = WRITE_FIRST_TRANS;
	tx_buf[1] = reg_high_addr;
	tx_buf[2] = WRITE_SECOND_TRANS;
	tx_buf[3] = reg_low_addr;
	xfer.rx_buf = NULL;
	xfer.len = 4;
	xfer.bits_per_word = 16;

	ret = spi_sync(lcd->spi, &msg);
	if (ret < 0)
		dev_err(&lcdev->dev, "Couldn't send SPI data\n");

	return ret;
}

static int RM68120_spi_write_3bytes(struct lcd_device *lcdev, unsigned char reg_high_addr,
	unsigned char reg_low_addr, unsigned char write_data)
{
	char tx_buf[6];
	int ret = 0;
	struct rm68120_data *lcd = lcd_get_data(lcdev);
	struct spi_message msg;
	struct spi_transfer xfer;
	
	memset(&xfer, 0, sizeof xfer);
	xfer.tx_buf = tx_buf;
	
	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);
	
	tx_buf[0] = WRITE_FIRST_TRANS;
	tx_buf[1] = reg_high_addr;
	tx_buf[2] = WRITE_SECOND_TRANS;
	tx_buf[3] = reg_low_addr;
	tx_buf[4] = WRITE_THIRD_TRANS;
	tx_buf[5] = write_data;
	xfer.rx_buf = NULL;
	xfer.len = 6;
	xfer.bits_per_word = 16;

	ret = spi_sync(lcd->spi, &msg);
	if (ret < 0)
		dev_err(&lcdev->dev, "Couldn't send SPI data\n");

	return ret;
}

static int rm68120_enter_standby(struct lcd_device *lcdev)
{
	int ret;

	/* set display off */
	ret=RM68120_spi_write_2bytes(lcdev,0x28,0x00);
	if (ret < 0)
		return ret;
								  
	usleep_range(10000, 12000);

	/* enter sleep mode */
	ret=RM68120_spi_write_2bytes(lcdev,0x10,0x00);
	if (ret < 0)
		return ret;
								  
	msleep(120);

	return 0;
}

static int rm68120_exit_standby(struct lcd_device *lcdev)
{
	int ret;

	/* set display on */
	ret=RM68120_spi_write_2bytes(lcdev,0x29,0x00);
	if (ret < 0)
		return ret;
								  
	usleep_range(10000, 12000);

	/* exit sleep mode */
	ret=RM68120_spi_write_2bytes(lcdev,0x11,0x00);
	if (ret < 0)
		return ret;
								  
	msleep(120);

	return 0;
}

static void rm68120_lcd_reset(struct lcd_device *lcdev)
{
	struct rm68120_data *lcd = lcd_get_data(lcdev);

	/* Reset the screen */
	gpio_set_value(lcd->reset, 1);
	usleep_range(10000, 12000);
	gpio_set_value(lcd->reset, 0);
	usleep_range(10000, 12000);
	gpio_set_value(lcd->reset, 1);

	/* The controller needs 120ms to recover from reset */
	msleep(120);
}

static int rm68120_lcd_init(struct lcd_device *lcdev)
{
	struct rm68120_data *lcd = lcd_get_data(lcdev);
	int ret;

	/* Reset the screen */
	gpio_set_value(lcd->reset, 1);
	usleep_range(10000, 12000);
	gpio_set_value(lcd->reset, 0);
	usleep_range(10000, 12000);
	gpio_set_value(lcd->reset, 1);
	msleep(120);
	
	//#Enable Page1 
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x00,0X55);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x01,0XAA);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x02,0X52);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x03,0X08);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x04,0x01);
	if (ret < 0)
		return ret;

	ret=RM68120_spi_write_3bytes(lcdev,0xBC,0x01,0xA8);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xBC,0x02,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xBD,0x01,0xA8);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xBD,0x02,0x00);
	if (ret < 0)
		return ret;

	ret=RM68120_spi_write_3bytes(lcdev,0xBE,0x01,0x60);
	if (ret < 0)
		return ret;

	//#R+ 
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x00,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x01,0x5D);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x02,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x03,0x69);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x04,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x05,0x7C);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x06,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x07,0x8D);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x08,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x09,0x9C);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x0A,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x0B,0xB3);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x0C,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x0D,0xCA);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x0E,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x0F,0xEC);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x10,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x11,0x08);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x12,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x13,0x32);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x14,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x15,0x53);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x16,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x17,0x85);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x18,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x19,0xAE);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x1A,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x1B,0xAF);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x1C,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x1D,0xD3);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x1E,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x1F,0xF5);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x20,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x21,0x09);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x22,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x23,0x23);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x24,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x25,0x3B);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x26,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x27,0x66);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x28,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x29,0x90);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x2A,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x2B,0xE0);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x2C,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x2D,0x1C);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x2E,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x2F,0xCC);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x30,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x31,0xCC);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x32,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD1,0x33,0xCC);
	if (ret < 0)
		return ret;

	//#G+ 
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x00,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x01,0x5D);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x02,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x03,0x69);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x04,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x05,0x7C);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x06,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x07,0x8D);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x08,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x09,0x9C);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x0A,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x0B,0xB3);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x0C,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x0D,0xCA);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x0E,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x0F,0xEC);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x10,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x11,0x08);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x12,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x13,0x32);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x14,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x15,0x53);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x16,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x17,0x85);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x18,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x19,0xAE);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x1A,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x1B,0xAF);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x1C,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x1D,0xD3);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x1E,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x1F,0xF5);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x20,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x21,0x09);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x22,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x23,0x23);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x24,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x25,0x3B);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x26,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x27,0x66);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x28,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x29,0x90);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x2A,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x2B,0xE0);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x2C,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x2D,0x1C);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x2E,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x2F,0xCC);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x30,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x31,0xCC);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x32,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD2,0x33,0xCC);
	if (ret < 0)
		return ret;

	//#B+
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x00,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x01,0x5D);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x02,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x03,0x69);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x04,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x05,0x7C);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x06,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x07,0x8D);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x08,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x09,0x9C);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x0A,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x0B,0xB3);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x0C,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x0D,0xCA);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x0E,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x0F,0xEC);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x10,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x11,0x08);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x12,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x13,0x32);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x14,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x15,0x53);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x16,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x17,0x85);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x18,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x19,0xAE);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x1A,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x1B,0xAF);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x1C,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x1D,0xD3);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x1E,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x1F,0xF5);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x20,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x21,0x09);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x22,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x23,0x23);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x24,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x25,0x3B);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x26,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x27,0x66);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x28,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x29,0x90);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x2A,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x2B,0xE0);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x2C,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x2D,0x1C);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x2E,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x2F,0xCC);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x30,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x31,0xCC);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x32,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD3,0x33,0xCC);
	if (ret < 0)
		return ret;

	 //#R- 
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x00,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x01,0x5D);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x02,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x03,0x69);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x04,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x05,0x7C);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x06,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x07,0x8D);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x08,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x09,0x9C);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x0A,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x0B,0xB3);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x0C,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x0D,0xCA);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x0E,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x0F,0xEC);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x10,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x11,0x08);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x12,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x13,0x32);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x14,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x15,0x53);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x16,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x17,0x85);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x18,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x19,0xAE);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x1A,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x1B,0xAF);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x1C,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x1D,0xD3);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x1E,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x1F,0xF5);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x20,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x21,0x09);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x22,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x23,0x23);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x24,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x25,0x3B);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x26,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x27,0x66);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x28,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x29,0x90);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x2A,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x2B,0xE0);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x2C,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x2D,0x1C);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x2E,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x2F,0xCC);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x30,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x31,0xCC);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x32,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD4,0x33,0xCC);
	if (ret < 0)
		return ret;

	 //#G- 
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x00,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x01,0x5D);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x02,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x03,0x69);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x04,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x05,0x7C);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x06,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x07,0x8D);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x08,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x09,0x9C);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x0A,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x0B,0xB3);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x0C,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x0D,0xCA);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x0E,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x0F,0xEC);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x10,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x11,0x08);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x12,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x13,0x32);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x14,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x15,0x53);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x16,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x17,0x85);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x18,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x19,0xAE);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x1A,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x1B,0xAF);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x1C,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x1D,0xD3);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x1E,0x01);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x1F,0xF5);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x20,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x21,0x09);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x22,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x23,0x23);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x24,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x25,0x3B);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x26,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x27,0x66);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x28,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x29,0x90);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x2A,0x02);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x2B,0xE0);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x2C,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x2D,0x1C);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x2E,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x2F,0xCC);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x30,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x31,0xCC);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x32,0x03);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD5,0x33,0xCC);
	if (ret < 0)
		return ret;

	//#B-
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x00,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x01,0x5D);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x02,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x03,0x69);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x04,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x05,0x7C);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x06,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x07,0x8D);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x08,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x09,0x9C);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x0A,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x0B,0xB3);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x0C,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x0D,0xCA);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x0E,0x00);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x0F,0xEC);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x10,0x01);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x11,0x08);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x12,0x01);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x13,0x32);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x14,0x01);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x15,0x53);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x16,0x01);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x17,0x85);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x18,0x01);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x19,0xAE);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x1A,0x01);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x1B,0xAF);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x1C,0x01);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x1D,0xD3);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x1E,0x01);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x1F,0xF5);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x20,0x02);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x21,0x09);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x22,0x02);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x23,0x23);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x24,0x02);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x25,0x3B);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x26,0x02);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x27,0x66);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x28,0x02);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x29,0x90);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x2A,0x02);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x2B,0xE0);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x2C,0x03);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x2D,0x1C);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x2E,0x03);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x2F,0xCC);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x30,0x03);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x31,0xCC);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x32,0x03);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xD6,0x33,0xCC); 
	if (ret < 0)
		return ret;


	ret=RM68120_spi_write_3bytes(lcdev,0xB0,0x00,0x12);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xB0,0x01,0x12);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xB0,0x02,0x12);               
	if (ret < 0)
		return ret;
								   
	ret=RM68120_spi_write_3bytes(lcdev,0xB1,0x00,0x0A);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xB1,0x01,0x0A);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xB1,0x02,0x0A);               
	if (ret < 0)
		return ret;
								   
	ret=RM68120_spi_write_3bytes(lcdev,0xBA,0x00,0x24);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xBA,0x01,0x24);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xBA,0x02,0x24);               
	if (ret < 0)
		return ret;
								   
	ret=RM68120_spi_write_3bytes(lcdev,0xB9,0x00,0x34);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xB9,0x01,0x34);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xB9,0x02,0x34);                                             
	if (ret < 0)
		return ret;

	//#Enable Page0                  
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x00,0x55);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x01,0xAA);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x02,0x52);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x03,0x08);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x04,0x00);                                             
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xB1,0x00,0xCC);   
	if (ret < 0)
		return ret;

	ret=RM68120_spi_write_3bytes(lcdev,0xB7,0x00,0x70);                   
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xB7,0x01,0x70);  
	if (ret < 0)
		return ret;

	ret=RM68120_spi_write_3bytes(lcdev,0xB4,0x00,0x10);                             
	if (ret < 0)
		return ret;
								   
	ret=RM68120_spi_write_3bytes(lcdev,0xBC,0x00,0x05);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xBC,0x01,0x05);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xBC,0x02,0x05);               
	if (ret < 0)
		return ret;
								   
	ret=RM68120_spi_write_3bytes(lcdev,0xB8,0x00,0x01);               
	if (ret < 0)
		return ret;
								   
	ret=RM68120_spi_write_3bytes(lcdev,0xB7,0x00,0x55);              
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xB7,0x01,0x55);               
	if (ret < 0)
		return ret;
								   
	ret=RM68120_spi_write_3bytes(lcdev,0xBD,0x02,0x07);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xBD,0x03,0x31);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xBE,0x02,0x07);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xBE,0x03,0x31);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xBF,0x02,0x07);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xBF,0x03,0x31);              
	if (ret < 0)
		return ret;
								   
	//#Enable Page2 
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x00,0X55);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x01,0XAA);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x02,0X52);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x03,0X08);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x04,0x02);
	if (ret < 0)
		return ret;

	ret=RM68120_spi_write_3bytes(lcdev,0xFF,0x00,0xAA);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xFF,0x01,0x55);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xFF,0x02,0x25);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xFF,0x03,0x01);               
	if (ret < 0)
		return ret;
								   
	ret=RM68120_spi_write_3bytes(lcdev,0x35,0x00,0x00);               
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0x3a,0x00,0x66);  //18bit 66,   24bit 77          
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0x36,0x00,0x00);                  
	if (ret < 0)
		return ret;
								  
	//#Enable Page1 
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x00,0X55);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x01,0XAA);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x02,0X52);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x03,0X08);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_3bytes(lcdev,0xF0,0x04,0x01);
	if (ret < 0)
		return ret;

	ret=RM68120_spi_write_2bytes(lcdev,0x11,0x00);	               
	if (ret < 0)
		return ret;
	msleep(150);                    
								  
	ret=RM68120_spi_write_2bytes(lcdev,0x29,0x00);
	if (ret < 0)
		return ret;
	ret=RM68120_spi_write_2bytes(lcdev,0X2c,0x00);
	if (ret < 0)
		return ret;

	return 0;
}

#define POWER_IS_ON(pwr)	((pwr) <= FB_BLANK_NORMAL)

static int rm68120_set_power(struct lcd_device *lcdev, int power)
{
	struct rm68120_data *lcd = lcd_get_data(lcdev);
	int ret = 0;

	if (POWER_IS_ON(power) && !POWER_IS_ON(lcd->state))
		ret = rm68120_exit_standby(lcdev);
	else if (!POWER_IS_ON(power) && POWER_IS_ON(lcd->state))
		ret = rm68120_enter_standby(lcdev);

	if (ret == 0)
		lcd->state = power;
	else
		dev_warn(&lcdev->dev, "failed to set power mode %d\n", power);

	return ret;
}

static int rm68120_get_power(struct lcd_device *lcdev)
{
	struct rm68120_data *lcd = lcd_get_data(lcdev);

	return lcd->state;
}

static struct lcd_ops rm68120_ops = {
	.set_power	= rm68120_set_power,
	.get_power	= rm68120_get_power,
};

static const struct of_device_id rm68120_dt_ids[] = {
	{ .compatible = "raydium,rm68120" },
	{},
};
MODULE_DEVICE_TABLE(of, rm68120_dt_ids);

static int rm68120_probe(struct spi_device *spi)
{
	struct lcd_device *lcdev;
	struct rm68120_data *lcd;
	int ret;

	lcd = devm_kzalloc(&spi->dev, sizeof(*lcd), GFP_KERNEL);
	if (!lcd) {
		dev_err(&spi->dev, "Couldn't allocate lcd internal structure!\n");
		return -ENOMEM;
	}


	spi->chip_select=0;
	spi->mode = SPI_MODE_0;
	spi->max_speed_hz=1100000;

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
				    "rm68120-reset");
	if (ret) {
		dev_err(&spi->dev,
			"failed to request gpio %d: %d\n",
			lcd->reset, ret);
		return -EINVAL;
	}

	lcdev = lcd_device_register("mxsfb", &spi->dev, lcd, &rm68120_ops);
	if (IS_ERR(lcdev)) {
		ret = PTR_ERR(lcdev);
		return ret;
	}
	spi_set_drvdata(spi, lcdev);

	rm68120_lcd_reset(lcdev);

	ret = rm68120_lcd_init(lcdev);
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

static int rm68120_remove(struct spi_device *spi)
{
	struct lcd_device *lcdev = spi_get_drvdata(spi);

	lcd_device_unregister(lcdev);
	return 0;
}

static struct spi_driver rm68120_driver = {
	.probe  = rm68120_probe,
	.remove = rm68120_remove,
	.driver = {
		.name = "rm68120",
		.of_match_table = of_match_ptr(rm68120_dt_ids),
	},
};

module_spi_driver(rm68120_driver);

MODULE_AUTHOR("Brian Lilly <brian@crystalfontz.com>");
MODULE_DESCRIPTION("Raydium RM68120 LCD Driver");
MODULE_LICENSE("GPL");
