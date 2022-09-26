// SPDX-License-Identifier: GPL-2.0
/*
 * rn6854 driver
 * Copyright (C) 2022 Rockchip Electronics Co., Ltd.
 *
 * V0.0X01.0X00 first version.
 */

#include <linux/clk.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <linux/regulator/consumer.h>
#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/rk-camera-module.h>
#include <media/media-entity.h>
#include <media/v4l2-async.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-subdev.h>
#include <linux/pinctrl/consumer.h>
#include <linux/rk-preisp.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/platform_device.h>
#include <linux/input.h>

/* Video format support */
//Analog CVBS PAL
//Analog CVBS NTSC
//Analog HD 720P25
//Analog HD 720P30
//Analog FHD 1080P25
//Analog FHD 1080P30

/* Video combination support */
// ch0-ch1-ch2-ch3
// 1080P-1080P
// 1080P-720P-720P
// 1080P-720P-CVBS
// 720P-720P-720P-720P
// 720P-720P-720P-CVBS
// CVBS-CVBS-CVBS-CVBS

/* CVBS output mode selection, frame or field, default is frame */
// #define RN6854M_USE_CVBS_FIELD

/* MIPI clock mode selection, continuous clock or non-continuous clock, default is continuous */
#define RN6854M_USE_MIPI_NON_CONTINUOUS_CLOCK

/* How the screen displays when there is no input, black or blue, default is black */
#define RN6854M_USE_BLUE_SCREEN

/* test mode, output as color bar screen displays */
//#define RN6854_COLOR_BAR

#define DRIVER_VERSION				KERNEL_VERSION(0, 0x01, 0x0)

#ifndef V4L2_CID_DIGITAL_GAIN
#define V4L2_CID_DIGITAL_GAIN			V4L2_CID_GAIN
#endif

#define RN6854_XVCLK_FREQ			27000000

/************************
 * RN6854M MIPI Frequency
 * Data rate 648M
 * Clock	 324M
 ************************/
#define RN6854_LINK_FREQ			(324000000UL)
#define RN6854_LINK_FREQ_324M		(324000000UL)

#define RN6854_LANES				4
#define RN6854_BITS_PER_SAMPLE		8

#define OF_CAMERA_PINCTRL_STATE_DEFAULT		"rockchip,camera_default"
#define OF_CAMERA_PINCTRL_STATE_SLEEP		"rockchip,camera_sleep"

#define V4L2_CID_PRIVATE_BASE	0x08000000
#define V4L2_CID_CHECK_CAMERA   (V4L2_CID_PRIVATE_BASE + 1)
#define V4L2_CID_SWITCH_CAMERA  (V4L2_CID_PRIVATE_BASE + 2)

#define RN6854_NAME				"rn6854"
#define RN6854_ID_ADDR			(0xfe)
#define RN6854_ID				(0x05)

#define RN6854_MEDIA_BUS_FMT	MEDIA_BUS_FMT_UYVY8_2X8

static const char * const rn6854_supply_names[] = {
	"avdd",		/* Analog power */
	"dovdd",	/* Digital I/O power */
	"dvdd",		/* Digital core power */
};

#define RN6854_NUM_SUPPLIES ARRAY_SIZE(rn6854_supply_names)

// #define RN6854_FMT_1080P30x2 1
#define RN6854_FMT_1080P25x2 1
// #define RN6854_FMT_720P30x4  1
#define RN6854_FMT_720P25x4  1
// #define RN6854_FMT_720P25x1  1
// #define RN6854_FMT_CVBS25x4  1

#define RN_RESO_720P_NSTC_VALUE	 0x20
#define RN_RESO_720P_PAL_VALUE	 0x21
#define RN_RESO_1080P_NSTC_VALUE 0x30
#define RN_RESO_1080P_PAL_VALUE	 0x31

#define RN6854_STATUS_NO_VIDEO	(1 << 4)

enum rn6854_max_pad {
	PAD0,
	PAD1,
	PAD2,
	PAD3,
	PAD_MAX,
};

enum rn6854_support_reso {
	RN_RESO_UNKOWN = 0,
	RN_RESO_D1_PAL,
	RN_RESO_960H_PAL,
	RN_RESO_720P_PAL,
	RN_RESO_960P_PAL,
	RN_RESO_1080P_PAL,
	RN_RESO_D1_NTSC,
	RN_RESO_960H_NTSC,
	RN_RESO_720P_NTSC,
	RN_RESO_960P_NTSC,
	RN_RESO_1080P_NTSC,
};

struct regval {
	u8 addr;
	u8 val;
};

struct rn6854_mode {
	u32 bus_fmt;
	u32 width;
	u32 height;
	struct v4l2_fract max_fps;
	u32 mipi_freq_idx;
	u32 bpp;
	const struct regval *global_reg_list;
	const struct regval *reg_list;
	u32 hdr_mode;
	u32 vc[PAD_MAX];
	u32 channel_reso[PAD_MAX];
};

struct rn6854 {
	struct i2c_client	*client;
	struct clk		*xvclk;
	struct gpio_desc	*reset_gpio;
	struct gpio_desc	*power_gpio;
	struct gpio_desc	*vi_gpio;
	struct regulator_bulk_data supplies[RN6854_NUM_SUPPLIES];
	struct pinctrl		*pinctrl;
	struct pinctrl_state	*pins_default;
	struct pinctrl_state	*pins_sleep;
	struct v4l2_subdev	subdev;
	struct media_pad	pad;
	struct v4l2_ctrl_handler ctrl_handler;
	struct v4l2_ctrl	*pixel_rate;
	struct v4l2_ctrl	*link_freq;
	struct mutex		mutex;
	struct mutex		i2c_mutex;
	bool			power_on;
	struct rn6854_mode cur_mode;
	u32			module_index;
	u32			cfg_num;
	const char		*module_facing;
	const char		*module_name;
	const char		*len_name;
	int streaming;
	struct task_struct *detect_thread;
	struct input_dev *input_dev;
	u8 detect_status;
	struct v4l2_control *ctrl;
	u8 is_reset;
	unsigned char check_status[4];
};

#define to_rn6854(sd) container_of(sd, struct rn6854, subdev)

// detect_status: bit 0~3 means channels plugin status : 1 no exist 0: exist
static ssize_t show_hotplug_status(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct rn6854 *rn6854 = to_rn6854(sd);

	return sprintf(buf, "%d\n", rn6854->detect_status);
}

static DEVICE_ATTR(hotplug_status, 0400, show_hotplug_status, NULL);
static struct attribute *dev_attrs[] = {
	&dev_attr_hotplug_status.attr,
	NULL,
};
static struct attribute_group dev_attr_grp = {
	.attrs = dev_attrs,
};

static __maybe_unused const struct regval common_setting_720P30x4_regs[] = {
	{0x81, 0x0F},
	{0xA3, 0x04},
	{0xDF, 0xF0},
	{0x88, 0x40},
	{0xF6, 0x40},
	{0xD3, 0x50},
	{0xFF, 0x00},
	{0x00, 0x20},
	{0x06, 0x08},
	{0x07, 0x63},
	{0x2A, 0x01},
	{0x3A, 0x24},
	{0x3F, 0x10},
	{0x4C, 0x37},
	{0x4F, 0x03},
	{0x50, 0x02},
	{0x56, 0x01},
	{0x5F, 0x40},
	{0x63, 0xF5},
	{0x59, 0x00},
	{0x5A, 0x44},
	{0x58, 0x01},
	{0x59, 0x33},
	{0x5A, 0x23},
	{0x58, 0x01},
	{0x51, 0x4E},
	{0x52, 0x87},
	{0x53, 0x12},
	{0x5B, 0x07},
	{0x5E, 0x08},
	{0x6A, 0x82},
	{0x28, 0x92},
	{0x03, 0x80},
	{0x04, 0x80},
	{0x05, 0x00},
	{0x57, 0x23},
	{0x68, 0x32},
	{0x37, 0x33},
	{0x61, 0x6C},
#ifdef RN6854M_USE_BLUE_SCREEN
	{0x3A, 0x24},
#else
	{0x3A, 0x2C},
	{0x3B, 0x00},
	{0x3C, 0x80},
	{0x3D, 0x80},
#endif
	{0x33, 0x10},
	{0x4A, 0xA8},
	{0x2E, 0x30},
	{0x2E, 0x00},
	{0xFF, 0x01},
	{0x00, 0x20},
	{0x06, 0x08},
	{0x07, 0x63},
	{0x2A, 0x01},
	{0x3A, 0x24},
	{0x3F, 0x11},
	{0x4C, 0x37},
	{0x4F, 0x03},
	{0x50, 0x02},
	{0x56, 0x01},
	{0x5F, 0x40},
	{0x63, 0xF5},
	{0x59, 0x00},
	{0x5A, 0x44},
	{0x58, 0x01},
	{0x59, 0x33},
	{0x5A, 0x23},
	{0x58, 0x01},
	{0x51, 0x4E},
	{0x52, 0x87},
	{0x53, 0x12},
	{0x5B, 0x07},
	{0x5E, 0x08},
	{0x6A, 0x82},
	{0x28, 0x92},
	{0x03, 0x80},
	{0x04, 0x80},
	{0x05, 0x00},
	{0x57, 0x23},
	{0x68, 0x32},
	{0x37, 0x33},
	{0x61, 0x6C},
#ifdef RN6854M_USE_BLUE_SCREEN
	{0x3A, 0x24},
#else
	{0x3A, 0x2C},
	{0x3B, 0x00},
	{0x3C, 0x80},
	{0x3D, 0x80},
#endif
	{0x33, 0x10},
	{0x4A, 0xA8},
	{0x2E, 0x30},
	{0x2E, 0x00},
	{0xFF, 0x02},
	{0x00, 0x20},
	{0x06, 0x08},
	{0x07, 0x63},
	{0x2A, 0x01},
	{0x3A, 0x24},
	{0x3F, 0x12},
	{0x4C, 0x37},
	{0x4F, 0x03},
	{0x50, 0x02},
	{0x56, 0x01},
	{0x5F, 0x40},
	{0x63, 0xF5},
	{0x59, 0x00},
	{0x5A, 0x44},
	{0x58, 0x01},
	{0x59, 0x33},
	{0x5A, 0x23},
	{0x58, 0x01},
	{0x51, 0x4E},
	{0x52, 0x87},
	{0x53, 0x12},
	{0x5B, 0x07},
	{0x5E, 0x08},
	{0x6A, 0x82},
	{0x28, 0x92},
	{0x03, 0x80},
	{0x04, 0x80},
	{0x05, 0x00},
	{0x57, 0x23},
	{0x68, 0x32},
	{0x37, 0x33},
	{0x61, 0x6C},
#ifdef RN6854M_USE_BLUE_SCREEN
	{0x3A, 0x24},
#else
	{0x3A, 0x2C},
	{0x3B, 0x00},
	{0x3C, 0x80},
	{0x3D, 0x80},
#endif
	{0x33, 0x10},
	{0x4A, 0xA8},
	{0x2E, 0x30},
	{0x2E, 0x00},
	{0xFF, 0x03},
	{0x00, 0x20},
	{0x06, 0x08},
	{0x07, 0x63},
	{0x2A, 0x01},
	{0x3A, 0x24},
	{0x3F, 0x13},
	{0x4C, 0x37},
	{0x4F, 0x03},
	{0x50, 0x02},
	{0x56, 0x01},
	{0x5F, 0x40},
	{0x63, 0xF5},
	{0x59, 0x00},
	{0x5A, 0x44},
	{0x58, 0x01},
	{0x59, 0x33},
	{0x5A, 0x23},
	{0x58, 0x01},
	{0x51, 0x4E},
	{0x52, 0x87},
	{0x53, 0x12},
	{0x5B, 0x07},
	{0x5E, 0x08},
	{0x6A, 0x82},
	{0x28, 0x92},
	{0x03, 0x80},
	{0x04, 0x80},
	{0x05, 0x00},
	{0x57, 0x23},
	{0x68, 0x32},
	{0x37, 0x33},
	{0x61, 0x6C},
#ifdef RN6854M_USE_BLUE_SCREEN
	{0x3A, 0x24},
#else
	{0x3A, 0x2C},
	{0x3B, 0x00},
	{0x3C, 0x80},
	{0x3D, 0x80},
#endif
	{0x33, 0x10},
	{0x4A, 0xA8},
	{0x2E, 0x30},
	{0x2E, 0x00},
	{0xFF, 0x09},
	{0x00, 0x03},
	{0xFF, 0x08},
	{0x04, 0x03},
	{0x6C, 0x1F},
	{0x06, 0x7C},
	{0x21, 0x01},
	{0x34, 0x06},
	{0x35, 0x0B},
	{0x78, 0x80},
	{0x79, 0x02},
	{0x7A, 0x80},
	{0x7B, 0x02},
	{0x7C, 0x80},
	{0x7D, 0x02},
	{0x7E, 0x80},
	{0x7F, 0x02},
	{0x6C, 0x0F},
	{0x04, 0x00},
	//{0x20, 0xAA},
#ifdef RN6854M_USE_MIPI_NON_CONTINUOUS_CLOCK
	{0x07, 0x05},
#else
	{0x07, 0x04},
#endif
	{0xFF, 0x0A},
	{0x6C, 0x10},
#ifdef RN6854_COLOR_BAR
	{0xFF, 0x00},
	{0x00, 0x80},
#endif
};

static __maybe_unused const struct regval common_setting_720P25x4_regs[] = {
	{0x81, 0x0F},
	{0xA3, 0x04},
	{0xDF, 0xF0},
	{0x88, 0x40},
	{0xF6, 0x40},
	{0xD3, 0x50},
	{0xFF, 0x00},
	{0x00, 0x20},
	{0x06, 0x08},
	{0x07, 0x63},
	{0x2A, 0x01},
	{0x3A, 0x24},
	{0x3F, 0x10},
	{0x4C, 0x37},
	{0x4F, 0x03},
	{0x50, 0x02},
	{0x56, 0x01},
	{0x5F, 0x40},
	{0x63, 0xF5},
	{0x59, 0x00},
	{0x5A, 0x42},
	{0x58, 0x01},
	{0x59, 0x33},
	{0x5A, 0x23},
	{0x58, 0x01},
	{0x51, 0xE1},
	{0x52, 0x88},
	{0x53, 0x12},
	{0x5B, 0x07},
	{0x5E, 0x08},
	{0x6A, 0x82},
	{0x28, 0x92},
	{0x03, 0x80},
	{0x04, 0x80},
	{0x05, 0x00},
	{0x57, 0x23},
	{0x68, 0x32},
	{0x37, 0x33},
	{0x61, 0x6C},
#ifdef RN6854M_USE_BLUE_SCREEN
	{0x3A, 0x24},
#else
	{0x3A, 0x2C},
	{0x3B, 0x00},
	{0x3C, 0x80},
	{0x3D, 0x80},
#endif
	{0x33, 0x10},
	{0x4A, 0xA8},
	{0x2E, 0x30},
	{0x2E, 0x00},
	{0xFF, 0x01},
	{0x00, 0x20},
	{0x06, 0x08},
	{0x07, 0x63},
	{0x2A, 0x01},
	{0x3A, 0x24},
	{0x3F, 0x11},
	{0x4C, 0x37},
	{0x4F, 0x03},
	{0x50, 0x02},
	{0x56, 0x01},
	{0x5F, 0x40},
	{0x63, 0xF5},
	{0x59, 0x00},
	{0x5A, 0x42},
	{0x58, 0x01},
	{0x59, 0x33},
	{0x5A, 0x23},
	{0x58, 0x01},
	{0x51, 0xE1},
	{0x52, 0x88},
	{0x53, 0x12},
	{0x5B, 0x07},
	{0x5E, 0x08},
	{0x6A, 0x82},
	{0x28, 0x92},
	{0x03, 0x80},
	{0x04, 0x80},
	{0x05, 0x00},
	{0x57, 0x23},
	{0x68, 0x32},
	{0x37, 0x33},
	{0x61, 0x6C},
#ifdef RN6854M_USE_BLUE_SCREEN
	{0x3A, 0x24},
#else
	{0x3A, 0x2C},
	{0x3B, 0x00},
	{0x3C, 0x80},
	{0x3D, 0x80},
#endif
	{0x33, 0x10},
	{0x4A, 0xA8},
	{0x2E, 0x30},
	{0x2E, 0x00},
	{0xFF, 0x02},
	{0x00, 0x20},
	{0x06, 0x08},
	{0x07, 0x63},
	{0x2A, 0x01},
	{0x3A, 0x24},
	{0x3F, 0x12},
	{0x4C, 0x37},
	{0x4F, 0x03},
	{0x50, 0x02},
	{0x56, 0x01},
	{0x5F, 0x40},
	{0x63, 0xF5},
	{0x59, 0x00},
	{0x5A, 0x42},
	{0x58, 0x01},
	{0x59, 0x33},
	{0x5A, 0x23},
	{0x58, 0x01},
	{0x51, 0xE1},
	{0x52, 0x88},
	{0x53, 0x12},
	{0x5B, 0x07},
	{0x5E, 0x08},
	{0x6A, 0x82},
	{0x28, 0x92},
	{0x03, 0x80},
	{0x04, 0x80},
	{0x05, 0x00},
	{0x57, 0x23},
	{0x68, 0x32},
	{0x37, 0x33},
	{0x61, 0x6C},
#ifdef RN6854M_USE_BLUE_SCREEN
	{0x3A, 0x24},
#else
	{0x3A, 0x2C},
	{0x3B, 0x00},
	{0x3C, 0x80},
	{0x3D, 0x80},
#endif
	{0x33, 0x10},
	{0x4A, 0xA8},
	{0x2E, 0x30},
	{0x2E, 0x00},
	{0xFF, 0x03},
	{0x00, 0x20},
	{0x06, 0x08},
	{0x07, 0x63},
	{0x2A, 0x01},
	{0x3A, 0x24},
	{0x3F, 0x13},
	{0x4C, 0x37},
	{0x4F, 0x03},
	{0x50, 0x02},
	{0x56, 0x01},
	{0x5F, 0x40},
	{0x63, 0xF5},
	{0x59, 0x00},
	{0x5A, 0x42},
	{0x58, 0x01},
	{0x59, 0x33},
	{0x5A, 0x23},
	{0x58, 0x01},
	{0x51, 0xE1},
	{0x52, 0x88},
	{0x53, 0x12},
	{0x5B, 0x07},
	{0x5E, 0x08},
	{0x6A, 0x82},
	{0x28, 0x92},
	{0x03, 0x80},
	{0x04, 0x80},
	{0x05, 0x00},
	{0x57, 0x23},
	{0x68, 0x32},
	{0x37, 0x33},
	{0x61, 0x6C},
#ifdef RN6854M_USE_BLUE_SCREEN
	{0x3A, 0x24},
#else
	{0x3A, 0x2C},
	{0x3B, 0x00},
	{0x3C, 0x80},
	{0x3D, 0x80},
#endif
	{0x33, 0x10},
	{0x4A, 0xA8},
	{0x2E, 0x30},
	{0x2E, 0x00},
	{0xFF, 0x09},
	{0x00, 0x03},
	{0xFF, 0x08},
	{0x04, 0x03},
	{0x6C, 0x1F},
	{0x06, 0x7C},
	{0x21, 0x01},
	{0x34, 0x06},
	{0x35, 0x0B},
	{0x78, 0x80},
	{0x79, 0x02},
	{0x7A, 0x80},
	{0x7B, 0x02},
	{0x7C, 0x80},
	{0x7D, 0x02},
	{0x7E, 0x80},
	{0x7F, 0x02},
	{0x6C, 0x0F},
	{0x04, 0x00},
#ifdef RN6854M_USE_MIPI_NON_CONTINUOUS_CLOCK
	{0x07, 0x05},
	//{0x20, 0xAA},
#else
	{0x07, 0x04},
#endif
	{0xFF, 0x0A},
	{0x6C, 0x10},
#ifdef RN6854_COLOR_BAR
	{0xFF, 0x00},
	{0x00, 0x80},
#endif
	{0xFF, 0x00},
};

static __maybe_unused const struct regval common_setting_1080P25x2_regs[] = {
	{0x81, 0x03},
	{0xA3, 0x04},
	{0xDF, 0xFC},
	{0xF0, 0xC0},
	{0x88, 0x40},
	{0xF6, 0x40},
	{0xD3, 0x50},
	{0xFF, 0x00},
	{0x00, 0x20},
	{0x06, 0x08},
	{0x07, 0x63},
	{0x2A, 0x01},
	{0x3A, 0x24},
	{0x3F, 0x10},
	{0x4C, 0x37},
	{0x4F, 0x03},
	{0x50, 0x03},
	{0x56, 0x02},
	{0x5F, 0x44},
	{0x63, 0xF8},
	{0x59, 0x00},
	{0x5A, 0x48},
	{0x58, 0x01},
	{0x59, 0x33},
	{0x5A, 0x23},
	{0x58, 0x01},
	{0x51, 0xF4},
	{0x52, 0x29},
	{0x53, 0x15},
	{0x5B, 0x01},
	{0x5E, 0x08},
	{0x6A, 0x87},
	{0x28, 0x92},
	{0x03, 0x80},
	{0x04, 0x80},
	{0x05, 0x04},
	{0x57, 0x23},
	{0x68, 0x00},
	{0x37, 0x33},
	{0x61, 0x6C},
#ifdef RN6854M_USE_BLUE_SCREEN
	{0x3A, 0x24},
#else
	{0x3A, 0x2C},
	{0x3B, 0x00},
	{0x3C, 0x80},
	{0x3D, 0x80},
#endif
	{0x33, 0x10},
	{0x4A, 0xA8},
	{0x2E, 0x30},
	{0x2E, 0x00},
	{0xFF, 0x01},
	{0x00, 0x20},
	{0x06, 0x08},
	{0x07, 0x63},
	{0x2A, 0x01},
	{0x3A, 0x24},
	{0x3F, 0x11},
	{0x4C, 0x37},
	{0x4F, 0x03},
	{0x50, 0x03},
	{0x56, 0x02},
	{0x5F, 0x44},
	{0x63, 0xF8},
	{0x59, 0x00},
	{0x5A, 0x48},
	{0x58, 0x01},
	{0x59, 0x33},
	{0x5A, 0x23},
	{0x58, 0x01},
	{0x51, 0xF4},
	{0x52, 0x29},
	{0x53, 0x15},
	{0x5B, 0x01},
	{0x5E, 0x08},
	{0x6A, 0x87},
	{0x28, 0x92},
	{0x03, 0x80},
	{0x04, 0x80},
	{0x05, 0x04},
	{0x57, 0x23},
	{0x68, 0x00},
	{0x37, 0x33},
	{0x61, 0x6C},
#ifdef RN6854M_USE_BLUE_SCREEN
	{0x3A, 0x24},
#else
	{0x3A, 0x2C},
	{0x3B, 0x00},
	{0x3C, 0x80},
	{0x3D, 0x80},
#endif
	{0x33, 0x10},
	{0x4A, 0xA8},
	{0x2E, 0x30},
	{0x2E, 0x00},
	{0xFF, 0x09},
	{0x00, 0x03},
	{0xFF, 0x08},
	{0x04, 0x03},
	{0x6C, 0x13},
	{0x06, 0x7C},
	{0x21, 0x01},
	{0x34, 0x06},
	{0x35, 0x0B},
	{0x78, 0xC0},
	{0x79, 0x03},
	{0x7A, 0xC0},
	{0x7B, 0x03},
	{0x6C, 0x03},
	{0x04, 0x00},
	//{0x20, 0xAA},
#ifdef RN6854M_USE_MIPI_NON_CONTINUOUS_CLOCK
	{0x07, 0x05},
#else
	{0x07, 0x04},
#endif
	{0xFF, 0x0A},
	{0x6C, 0x10},
#ifdef RN6854_COLOR_BAR
	{0xFF, 0x00},
	{0x00, 0x80},
#endif
};

static __maybe_unused const struct regval common_setting_1080P30x2_regs[] = {
	{0x81, 0x03},
	{0xA3, 0x04},
	{0xDF, 0xFC},
	{0xF0, 0xC0},
	{0x88, 0x40},
	{0xF6, 0x40},
	{0xD3, 0x50},
	{0xFF, 0x00},
	{0x00, 0x20},
	{0x06, 0x08},
	{0x07, 0x63},
	{0x2A, 0x01},
	{0x3A, 0x24},
	{0x3F, 0x10},
	{0x4C, 0x37},
	{0x4F, 0x03},
	{0x50, 0x03},
	{0x56, 0x02},
	{0x5F, 0x44},
	{0x63, 0xF8},
	{0x59, 0x00},
	{0x5A, 0x49},
	{0x58, 0x01},
	{0x59, 0x33},
	{0x5A, 0x23},
	{0x58, 0x01},
	{0x51, 0xF4},
	{0x52, 0x29},
	{0x53, 0x15},
	{0x5B, 0x01},
	{0x5E, 0x08},
	{0x6A, 0x87},
	{0x28, 0x92},
	{0x03, 0x80},
	{0x04, 0x80},
	{0x05, 0x04},
	{0x57, 0x23},
	{0x68, 0x00},
	{0x37, 0x33},
	{0x61, 0x6C},
#ifdef RN6854M_USE_BLUE_SCREEN
	{0x3A, 0x24},
#else
	{0x3A, 0x2C},
	{0x3B, 0x00},
	{0x3C, 0x80},
	{0x3D, 0x80},
#endif
	{0x33, 0x10},
	{0x4A, 0xA8},
	{0x2E, 0x30},
	{0x2E, 0x00},
	{0xFF, 0x01},
	{0x00, 0x20},
	{0x06, 0x08},
	{0x07, 0x63},
	{0x2A, 0x01},
	{0x3A, 0x24},
	{0x3F, 0x11},
	{0x4C, 0x37},
	{0x4F, 0x03},
	{0x50, 0x03},
	{0x56, 0x02},
	{0x5F, 0x44},
	{0x63, 0xF8},
	{0x59, 0x00},
	{0x5A, 0x49},
	{0x58, 0x01},
	{0x59, 0x33},
	{0x5A, 0x23},
	{0x58, 0x01},
	{0x51, 0xF4},
	{0x52, 0x29},
	{0x53, 0x15},
	{0x5B, 0x01},
	{0x5E, 0x08},
	{0x6A, 0x87},
	{0x28, 0x92},
	{0x03, 0x80},
	{0x04, 0x80},
	{0x05, 0x04},
	{0x57, 0x23},
	{0x68, 0x00},
	{0x37, 0x33},
	{0x61, 0x6C},
#ifdef RN6854M_USE_BLUE_SCREEN
	{0x3A, 0x24},
#else
	{0x3A, 0x2C},
	{0x3B, 0x00},
	{0x3C, 0x80},
	{0x3D, 0x80},
#endif
	{0x33, 0x10},
	{0x4A, 0xA8},
	{0x2E, 0x30},
	{0x2E, 0x00},
	{0xFF, 0x09},
	{0x00, 0x03},
	{0xFF, 0x08},
	{0x04, 0x03},
	{0x6C, 0x13},
	{0x06, 0x7C},
	{0x21, 0x01},
	{0x34, 0x06},
	{0x35, 0x0B},
	{0x78, 0xC0},
	{0x79, 0x03},
	{0x7A, 0xC0},
	{0x7B, 0x03},
	{0x6C, 0x03},
	{0x04, 0x00},
	//{0x20, 0xAA},
#ifdef RN6854M_USE_MIPI_NON_CONTINUOUS_CLOCK
	{0x07, 0x05},
#else
	{0x07, 0x04},
#endif
	{0xFF, 0x0A},
	{0x6C, 0x10},
#ifdef RN6854_COLOR_BAR
	{0xFF, 0x00},
	{0x00, 0x80},
#endif
};

static __maybe_unused const struct regval common_setting_720P25x1_regs[] = {
	{0x81, 0x01},
	{0xDF, 0xFE},
	{0x88, 0x40},
	{0xF6, 0x40},
	{0xFF, 0x00},
	{0x00, 0x20},
	{0x06, 0x08},
	{0x07, 0x63},
	{0x2A, 0x01},
	{0x3A, 0x20},
	{0x3F, 0x10},
	{0x4C, 0x37},
	{0x4F, 0x03},
	{0x50, 0x02},
	{0x56, 0x01},
	{0x5F, 0x40},
	{0x63, 0xF5},
	{0x59, 0x00},
	{0x5A, 0x42},
	{0x58, 0x01},
	{0x59, 0x33},
	{0x5A, 0x02},
	{0x58, 0x01},
	{0x51, 0xE1},
	{0x52, 0x88},
	{0x53, 0x12},
	{0x5B, 0x07},
	{0x5E, 0x08},
	{0x6A, 0x82},
	{0x28, 0x92},
	{0x03, 0x80},
	{0x04, 0x80},
	{0x05, 0x00},
	{0x57, 0x23},
	{0x68, 0x32},
	{0x37, 0x33},
	{0x61, 0x6C},
	{0xFF, 0x09},
	{0x00, 0x03},
	{0xFF, 0x08},
	{0x04, 0x03},
	{0x6C, 0x11},
	{0x06, 0x4C},
	{0x21, 0x01},
	{0x34, 0x06},
	{0x35, 0x0B},
	{0x78, 0x80},
	{0x79, 0x02},
	{0x6C, 0x01},
	{0x04, 0x00},
	{0x07, 0x05},
	//{0x20, 0xAA},
	{0xFF, 0x0A},
	{0x6C, 0x10},
#ifdef RN6854_COLOR_BAR
	{0xFF, 0x00},
	{0x00, 0x80},
#endif
};

static __maybe_unused const struct regval common_setting_cvbsx4_pal_regs[] = {
	{0x81, 0x0F},
	{0xA3, 0x04},
	{0xDF, 0x0F},
	{0x88, 0x40},
	{0xF6, 0x40},
	{0xD3, 0x50},
	{0xFF, 0x00},
	{0x00, 0x00},
	{0x06, 0x08},
	{0x07, 0x62},
	{0x2A, 0x81},
	{0x3A, 0x24},
	{0x3F, 0x10},
	{0x4C, 0x37},
	{0x4F, 0x00},
	{0x50, 0x00},
	{0x56, 0x01},
	{0x5F, 0x00},
	{0x63, 0x75},
	{0x59, 0x00},
	{0x5A, 0x00},
	{0x58, 0x01},
	{0x59, 0x33},
	{0x5A, 0x02},
	{0x58, 0x01},
	{0x5B, 0x00},
	{0x5E, 0x01},
	{0x6A, 0x00},
	{0x28, 0xB2},
#ifdef RN6854M_USE_CVBS_FIELD
	{0x20, 0x24},
	{0x23, 0x17},
	{0x24, 0x37},
	{0x25, 0x17},
	{0x26, 0x00},
	{0x42, 0x00},
#else
	{0x20, 0xA4},
	{0x23, 0x17},
	{0x24, 0xFF},
	{0x25, 0x00},
	{0x26, 0xFF},
	{0x42, 0x00},
#endif
	{0x03, 0x80},
	{0x04, 0x80},
	{0x05, 0x03},
	{0x57, 0x20},
	{0x68, 0x32},
	{0x37, 0x33},
	{0x61, 0x6C},
#ifdef RN6854M_USE_BLUE_SCREEN
	{0x3A, 0x24},
#else
	{0x3A, 0x2C},
	{0x3B, 0x00},
	{0x3C, 0x80},
	{0x3D, 0x80},
#endif
	{0x33, 0x10},
	{0x4A, 0xA8},
	{0x2E, 0x30},
	{0x2E, 0x00},
	{0xFF, 0x01},
	{0x00, 0x00},
	{0x06, 0x08},
	{0x07, 0x62},
	{0x2A, 0x81},
	{0x3A, 0x24},
	{0x3F, 0x11},
	{0x4C, 0x37},
	{0x4F, 0x00},
	{0x50, 0x00},
	{0x56, 0x01},
	{0x5F, 0x00},
	{0x63, 0x75},
	{0x59, 0x00},
	{0x5A, 0x00},
	{0x58, 0x01},
	{0x59, 0x33},
	{0x5A, 0x02},
	{0x58, 0x01},
	{0x5B, 0x00},
	{0x5E, 0x01},
	{0x6A, 0x00},
	{0x28, 0xB2},
#ifdef RN6854M_USE_CVBS_FIELD
	{0x20, 0x24},
	{0x23, 0x17},
	{0x24, 0x37},
	{0x25, 0x17},
	{0x26, 0x00},
	{0x42, 0x00},
#else
	{0x20, 0xA4},
	{0x23, 0x17},
	{0x24, 0xFF},
	{0x25, 0x00},
	{0x26, 0xFF},
	{0x42, 0x00},
#endif
	{0x03, 0x80},
	{0x04, 0x80},
	{0x05, 0x03},
	{0x57, 0x20},
	{0x68, 0x32},
	{0x37, 0x33},
	{0x61, 0x6C},
#ifdef RN6854M_USE_BLUE_SCREEN
	{0x3A, 0x24},
#else
	{0x3A, 0x2C},
	{0x3B, 0x00},
	{0x3C, 0x80},
	{0x3D, 0x80},
#endif
	{0x33, 0x10},
	{0x4A, 0xA8},
	{0x2E, 0x30},
	{0x2E, 0x00},
	{0xFF, 0x02},
	{0x00, 0x00},
	{0x06, 0x08},
	{0x07, 0x62},
	{0x2A, 0x81},
	{0x3A, 0x24},
	{0x3F, 0x12},
	{0x4C, 0x37},
	{0x4F, 0x00},
	{0x50, 0x00},
	{0x56, 0x01},
	{0x5F, 0x00},
	{0x63, 0x75},
	{0x59, 0x00},
	{0x5A, 0x00},
	{0x58, 0x01},
	{0x59, 0x33},
	{0x5A, 0x02},
	{0x58, 0x01},
	{0x5B, 0x00},
	{0x5E, 0x01},
	{0x6A, 0x00},
	{0x28, 0xB2},
#ifdef RN6854M_USE_CVBS_FIELD
	{0x20, 0x24},
	{0x23, 0x17},
	{0x24, 0x37},
	{0x25, 0x17},
	{0x26, 0x00},
	{0x42, 0x00},
#else
	{0x20, 0xA4},
	{0x23, 0x17},
	{0x24, 0xFF},
	{0x25, 0x00},
	{0x26, 0xFF},
	{0x42, 0x00},
#endif
	{0x03, 0x80},
	{0x04, 0x80},
	{0x05, 0x03},
	{0x57, 0x20},
	{0x68, 0x32},
	{0x37, 0x33},
	{0x61, 0x6C},
#ifdef RN6854M_USE_BLUE_SCREEN
	{0x3A, 0x24},
#else
	{0x3A, 0x2C},
	{0x3B, 0x00},
	{0x3C, 0x80},
	{0x3D, 0x80},
#endif
	{0x33, 0x10},
	{0x4A, 0xA8},
	{0x2E, 0x30},
	{0x2E, 0x00},
	{0xFF, 0x03},
	{0x00, 0x00},
	{0x06, 0x08},
	{0x07, 0x62},
	{0x2A, 0x81},
	{0x3A, 0x24},
	{0x3F, 0x13},
	{0x4C, 0x37},
	{0x4F, 0x00},
	{0x50, 0x00},
	{0x56, 0x01},
	{0x5F, 0x00},
	{0x63, 0x75},
	{0x59, 0x00},
	{0x5A, 0x00},
	{0x58, 0x01},
	{0x59, 0x33},
	{0x5A, 0x02},
	{0x58, 0x01},
	{0x5B, 0x00},
	{0x5E, 0x01},
	{0x6A, 0x00},
	{0x28, 0xB2},
#ifdef RN6854M_USE_CVBS_FIELD
	{0x20, 0x24},
	{0x23, 0x17},
	{0x24, 0x37},
	{0x25, 0x17},
	{0x26, 0x00},
	{0x42, 0x00},
#else
	{0x20, 0xA4},
	{0x23, 0x17},
	{0x24, 0xFF},
	{0x25, 0x00},
	{0x26, 0xFF},
	{0x42, 0x00},
#endif
	{0x03, 0x80},
	{0x04, 0x80},
	{0x05, 0x03},
	{0x57, 0x20},
	{0x68, 0x32},
	{0x37, 0x33},
	{0x61, 0x6C},
#ifdef RN6854M_USE_BLUE_SCREEN
	{0x3A, 0x24},
#else
	{0x3A, 0x2C},
	{0x3B, 0x00},
	{0x3C, 0x80},
	{0x3D, 0x80},
#endif
	{0x33, 0x10},
	{0x4A, 0xA8},
	{0x2E, 0x30},
	{0x2E, 0x00},
	{0xFF, 0x09},
	{0x00, 0x03},
	{0xFF, 0x08},
	{0x04, 0x03},
	{0x6C, 0x1F},
	{0x06, 0x7C},
	{0x21, 0x01},
	{0x34, 0x06},
	{0x35, 0x0B},
	{0x78, 0x68},
	{0x79, 0x01},
	{0x7A, 0x68},
	{0x7B, 0x01},
	{0x7C, 0x68},
	{0x7D, 0x01},
	{0x7E, 0x68},
	{0x7F, 0x01},
	{0x6C, 0x0F},
	{0x04, 0x00},
	//{0x20, 0xAA},
#ifdef RN6854M_USE_MIPI_NON_CONTINUOUS_CLOCK
	{0x07, 0x05},
#else
	{0x07, 0x04},
#endif
#ifdef RN6854_COLOR_BAR
	{0xFF, 0x00},
	{0x00, 0x80},
#endif
	{0xFF, 0x0A},
	{0x6C, 0x10},
};

static struct rn6854_mode supported_modes[] = {
	{
		.bus_fmt = RN6854_MEDIA_BUS_FMT,
		.width = 1920,
		.height = 1080,
		.max_fps = {
			.numerator = 10000,
			.denominator = 300000,
		},
#ifdef RN6854_FMT_1080P30x2
		.global_reg_list = common_setting_1080P30x2_regs,
#endif
#ifdef RN6854_FMT_1080P25x2
		.global_reg_list = common_setting_1080P25x2_regs,
#endif
		.mipi_freq_idx = 0,
		.bpp = 8,
		.vc[PAD0] = V4L2_MBUS_CSI2_CHANNEL_0,
		.vc[PAD1] = V4L2_MBUS_CSI2_CHANNEL_1,
		.vc[PAD2] = V4L2_MBUS_CSI2_CHANNEL_2,
		.vc[PAD3] = V4L2_MBUS_CSI2_CHANNEL_3,
		.channel_reso[PAD0] = RN_RESO_1080P_PAL,
		.channel_reso[PAD1] = RN_RESO_1080P_PAL,
		.channel_reso[PAD2] = RN_RESO_1080P_PAL,
		.channel_reso[PAD3] = RN_RESO_1080P_PAL,
	},
	{
		.bus_fmt = RN6854_MEDIA_BUS_FMT,
		.width = 1280,
		.height = 720,
		.max_fps = {
			.numerator = 10000,
			.denominator = 300000,
		},
#ifdef RN6854_FMT_720P30x4
		.global_reg_list = common_setting_720P30x4_regs,
#endif
#ifdef RN6854_FMT_720P25x4
		.global_reg_list = common_setting_720P25x4_regs,
#endif
#ifdef RN6854_FMT_720P25x1
		.global_reg_list = common_setting_720P25x1_regs,
#endif
		.mipi_freq_idx = 0,
		.bpp = 8,
		.vc[PAD0] = V4L2_MBUS_CSI2_CHANNEL_0,
		.vc[PAD1] = V4L2_MBUS_CSI2_CHANNEL_1,
		.vc[PAD2] = V4L2_MBUS_CSI2_CHANNEL_2,
		.vc[PAD3] = V4L2_MBUS_CSI2_CHANNEL_3,
		.channel_reso[PAD0] = RN_RESO_720P_PAL,
		.channel_reso[PAD1] = RN_RESO_720P_PAL,
		.channel_reso[PAD2] = RN_RESO_720P_PAL,
		.channel_reso[PAD3] = RN_RESO_720P_PAL,
	},
#ifdef RN6854_FMT_CVBS25x4
	{
		.bus_fmt = RN6854_MEDIA_BUS_FMT,
		.width = 720,
		.height = 604,
		.max_fps = {
			.numerator = 10000,
			.denominator = 300000,
		},
		.global_reg_list = common_setting_cvbsx4_pal_regs,
		.mipi_freq_idx = 1,
		.bpp = 8,
		.vc[PAD0] = V4L2_MBUS_CSI2_CHANNEL_0,
		.vc[PAD1] = V4L2_MBUS_CSI2_CHANNEL_1,
		.vc[PAD2] = V4L2_MBUS_CSI2_CHANNEL_2,
		.vc[PAD3] = V4L2_MBUS_CSI2_CHANNEL_3,
	},
#endif
};

static const s64 link_freq_items[] = {
	RN6854_LINK_FREQ,
	RN6854_LINK_FREQ_324M,
};

/* sensor register write */
static int rn6854_write_reg(struct i2c_client *client, u8 reg, u8 val)
{
	struct i2c_msg msg;
	u8 buf[2];
	int ret;

	buf[0] = reg & 0xFF;
	buf[1] = val;

	msg.addr = client->addr;
	msg.flags = client->flags;
	msg.buf = buf;
	msg.len = sizeof(buf);

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret >= 0) {
		usleep_range(300, 400);
		return 0;
	}

	dev_err(&client->dev,
		"rn6854 write reg(0x%x val:0x%x) failed !\n", reg, val);

	return ret;
}

static int rn6854_write_array(struct i2c_client *client,
			      const struct regval *regs, int size)
{
	int i, ret = 0;

	i = 0;
	while (i < size) {
		ret = rn6854_write_reg(client, regs[i].addr, regs[i].val);
		if (ret) {
			dev_err(&client->dev, "%s failed !\n", __func__);
			break;
		}
		i++;
	}

	return ret;
}

/* sensor register read */
static int rn6854_read_reg(struct i2c_client *client, u8 reg, u8 *val)
{
	struct i2c_msg msg[2];
	u8 buf[1];
	int ret;

	buf[0] = reg & 0xFF;

	msg[0].addr = client->addr;
	msg[0].flags = client->flags;
	msg[0].buf = buf;
	msg[0].len = sizeof(buf);

	msg[1].addr = client->addr;
	msg[1].flags = client->flags | I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = 1;

	ret = i2c_transfer(client->adapter, msg, 2);
	if (ret >= 0) {
		*val = buf[0];
		return 0;
	}

	dev_err(&client->dev, "rn6854 read reg(0x%x) failed !\n", reg);

	return ret;
}

// pre_initial_start
static void rn6854_pre_initial(struct i2c_client *client)
{
	unsigned char rom_byte1, rom_byte2, rom_byte3;
	unsigned char rom_byte4, rom_byte5 = 0, rom_byte6 = 0;

	rn6854_write_reg(client, 0xE1, 0x80);
	rn6854_write_reg(client, 0xFA, 0x81);
	rn6854_read_reg(client, 0xFB, &rom_byte1);
	rn6854_read_reg(client, 0xFB, &rom_byte2);
	rn6854_read_reg(client, 0xFB, &rom_byte3);
	rn6854_read_reg(client, 0xFB, &rom_byte4);
	rn6854_read_reg(client, 0xFB, &rom_byte5);
	rn6854_read_reg(client, 0xFB, &rom_byte6);

// config. decoder according to rom_byte5 and rom_byte6
	if (rom_byte6 == 0x00 && rom_byte5 == 0x00) {
		rn6854_write_reg(client, 0xEF, 0xAA);
		rn6854_write_reg(client, 0xE7, 0xFF);
		rn6854_write_reg(client, 0xFF, 0x09);
		rn6854_write_reg(client, 0x03, 0x0C);
		rn6854_write_reg(client, 0xFF, 0x0B);
		rn6854_write_reg(client, 0x03, 0x0C);
	} else if ((rom_byte6 == 0x34 && rom_byte5 == 0xA9) ||
		   (rom_byte6 == 0x2C && rom_byte5 == 0xA8)) {
		rn6854_write_reg(client, 0xEF, 0xAA);
		rn6854_write_reg(client, 0xE7, 0xFF);
		rn6854_write_reg(client, 0xFC, 0x60);
		rn6854_write_reg(client, 0xFF, 0x09);
		rn6854_write_reg(client, 0x03, 0x18);
		rn6854_write_reg(client, 0xFF, 0x0B);
		rn6854_write_reg(client, 0x03, 0x18);
	} else {
		rn6854_write_reg(client, 0xEF, 0xAA);
		rn6854_write_reg(client, 0xFC, 0x60);
		rn6854_write_reg(client, 0xFF, 0x09);
		rn6854_write_reg(client, 0x03, 0x18);
		rn6854_write_reg(client, 0xFF, 0x0B);
		rn6854_write_reg(client, 0x03, 0x18);
	}
}

static __maybe_unused void rn6854_vo_cfg(struct rn6854 *rn6854)
{
	struct i2c_client *client = rn6854->client;
	u8 yc_cnt_h = (rn6854->cur_mode.width / 2) >> 8;
	u8 yc_cnt_l = (rn6854->cur_mode.width / 2) & 0xFF;

	rn6854_write_reg(client, 0xFF, 0x09);
	rn6854_write_reg(client, 0x00, 0x03);
	rn6854_write_reg(client, 0xFF, 0x08);
	rn6854_write_reg(client, 0x04, 0x03);
	rn6854_write_reg(client, 0x6C, 0x1F);
	rn6854_write_reg(client, 0x06, 0x7C);
	rn6854_write_reg(client, 0x21, 0x01);
	rn6854_write_reg(client, 0x34, 0x06);
	rn6854_write_reg(client, 0x35, 0x0B);
	rn6854_write_reg(client, 0x78, yc_cnt_l);
	rn6854_write_reg(client, 0x79, yc_cnt_h);
	rn6854_write_reg(client, 0x7A, yc_cnt_l);
	rn6854_write_reg(client, 0x7B, yc_cnt_h);
	rn6854_write_reg(client, 0x7C, yc_cnt_l);
	rn6854_write_reg(client, 0x7D, yc_cnt_h);
	rn6854_write_reg(client, 0x7E, yc_cnt_l);
	rn6854_write_reg(client, 0x7F, yc_cnt_h);
	rn6854_write_reg(client, 0x6C, 0x0F);
	rn6854_write_reg(client, 0x04, 0x00);
	//rn6854_write_reg(client, 0x20, 0xAA);
#ifdef RN6854M_USE_MIPI_NON_CONTINUOUS_CLOCK
	rn6854_write_reg(client, 0x07, 0x05);
#else
	rn6854_write_reg(client, 0x07, 0x04);
#endif

}


static __maybe_unused int rn6854_set_quick_stream(struct rn6854 *rn6854, int on)
{
	mutex_lock(&rn6854->i2c_mutex);
	if (!on) {
		//rn6854_write_reg(client, 0xff, 0x08); // Selects MIPI CSI1 register set
		//rn6854_write_reg(client, 0x6C, 0x10); // reset and disable all chan
		//rn6854_write_reg(client, 0x06, 0x40); // disable all mipi lane
		//rn6854_write_reg(client, 0x04, 0x03); // reset csi dphy
		usleep_range(100 * 1000, 200 * 1000);
	} else {
		//rn6854_vo_cfg(rn6854);
	}
	mutex_unlock(&rn6854->i2c_mutex);

	return 0;
}

static __maybe_unused int rn6854_auto_detect_hotplug(struct rn6854 *rn6854)
{
	u8 tmp, i;
	struct i2c_client *client = rn6854->client;

	if (!mutex_trylock(&rn6854->i2c_mutex))
		return -1;
	rn6854->detect_status = 0;
	for (i = 0; i < 4; i++) {
		if (rn6854_write_reg(client, 0xff, i) < 0)
			goto err_out;
		if (rn6854_read_reg(client, 0x00, &tmp) < 0)
			goto err_out;

		if (tmp & RN6854_STATUS_NO_VIDEO)
			tmp = RN6854_STATUS_NO_VIDEO;

		if (tmp != rn6854->check_status[i]) {
			dev_info(&client->dev, "channel %d status 0x%x\n", i, tmp);
			rn6854->check_status[i] = tmp;
			rn6854->detect_status |= 1 << i;
		}
	}
	mutex_unlock(&rn6854->i2c_mutex);

	return 0;
err_out:
	mutex_unlock(&rn6854->i2c_mutex);
	return  -1;
}

static int rn6854_get_reso_dist(const struct rn6854_mode *mode,
				struct v4l2_mbus_framefmt *framefmt)
{
	return abs(mode->width - framefmt->width) +
	       abs(mode->height - framefmt->height);
}

static struct rn6854_mode *
rn6854_find_best_fit(struct rn6854 *rn6854,
		     struct v4l2_subdev_format *fmt)
{
	struct v4l2_mbus_framefmt *framefmt = &fmt->format;
	int dist;
	int cur_best_fit = 0;
	int cur_best_fit_dist = -1;
	unsigned int i;

	for (i = 0; i < rn6854->cfg_num; i++) {
		dist = rn6854_get_reso_dist(&supported_modes[i], framefmt);
		if ((cur_best_fit_dist == -1 || dist <= cur_best_fit_dist) &&
		    supported_modes[i].bus_fmt == framefmt->code) {
			cur_best_fit_dist = dist;
			cur_best_fit = i;
		}
	}

	return &supported_modes[cur_best_fit];
}

static int rn6854_set_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *fmt)
{
	struct rn6854 *rn6854 = to_rn6854(sd);
	struct rn6854_mode *mode;
	u64 pixel_rate = 0;

	mutex_lock(&rn6854->mutex);

	mode = rn6854_find_best_fit(rn6854, fmt);
	memcpy(&rn6854->cur_mode, mode, sizeof(struct rn6854_mode));
	fmt->format.code = mode->bus_fmt;
	fmt->format.width = mode->width;
	fmt->format.height = mode->height;
	fmt->format.field = V4L2_FIELD_NONE;
	fmt->format.colorspace = V4L2_COLORSPACE_SRGB;

	if (fmt->which == V4L2_SUBDEV_FORMAT_TRY) {
#ifdef CONFIG_VIDEO_V4L2_SUBDEV_API
		*v4l2_subdev_get_try_format(sd, cfg, fmt->pad) = fmt->format;
#else
		mutex_unlock(&rn6854->mutex);
		return -ENOTTY;
#endif
	} else {
		__v4l2_ctrl_s_ctrl(rn6854->link_freq, mode->mipi_freq_idx);
		pixel_rate = (u32)link_freq_items[mode->mipi_freq_idx] /
			      mode->bpp * 2 * RN6854_LANES;
		__v4l2_ctrl_s_ctrl_int64(rn6854->pixel_rate, pixel_rate);
		dev_dbg(&rn6854->client->dev, "mipi_freq_idx %d\n", mode->mipi_freq_idx);
		dev_dbg(&rn6854->client->dev, "pixel_rate %lld\n", pixel_rate);
	}
	mutex_unlock(&rn6854->mutex);

	return 0;
}

static int rn6854_get_fmt(struct v4l2_subdev *sd,
			  struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *fmt)
{
	struct rn6854 *rn6854 = to_rn6854(sd);
	struct i2c_client *client = rn6854->client;

	const struct rn6854_mode *mode = &rn6854->cur_mode;

	mutex_lock(&rn6854->mutex);
	if (fmt->which == V4L2_SUBDEV_FORMAT_TRY) {
#ifdef CONFIG_VIDEO_V4L2_SUBDEV_API
		fmt->format = *v4l2_subdev_get_try_format(sd, cfg, fmt->pad);
#else
		mutex_unlock(&rn6854->mutex);
		return -ENOTTY;
#endif
	} else {
		fmt->format.width = mode->width;
		fmt->format.height = mode->height;
		fmt->format.code = mode->bus_fmt;
		fmt->format.field = V4L2_FIELD_NONE;
		fmt->reserved[0] = mode->vc[fmt->pad];
	}
	mutex_unlock(&rn6854->mutex);

	dev_dbg(&client->dev, "%s: %x %dx%d vc %x\n",
		__func__, fmt->format.code,
		fmt->format.width, fmt->format.height, fmt->pad);

	return 0;
}

static int rn6854_enum_mbus_code(struct v4l2_subdev *sd,
				 struct v4l2_subdev_pad_config *cfg,
				 struct v4l2_subdev_mbus_code_enum *code)
{
	struct rn6854 *rn6854 = to_rn6854(sd);

	if (code->index != 0)
		return -EINVAL;
	code->code = rn6854->cur_mode.bus_fmt;

	return 0;
}

static int rn6854_enum_frame_sizes(struct v4l2_subdev *sd,
				   struct v4l2_subdev_pad_config *cfg,
				   struct v4l2_subdev_frame_size_enum *fse)
{
	struct rn6854 *rn6854 = to_rn6854(sd);

	if (fse->index >= rn6854->cfg_num)
		return -EINVAL;

	if (fse->code != supported_modes[fse->index].bus_fmt)
		return -EINVAL;

	fse->min_width  = supported_modes[fse->index].width;
	fse->max_width  = supported_modes[fse->index].width;
	fse->max_height = supported_modes[fse->index].height;
	fse->min_height = supported_modes[fse->index].height;
	return 0;
}

static int rn6854_enum_frame_interval(struct v4l2_subdev *sd,
				      struct v4l2_subdev_pad_config *cfg,
				      struct v4l2_subdev_frame_interval_enum *fie)
{
	struct rn6854 *rn6854 = to_rn6854(sd);

	if (fie->index >= rn6854->cfg_num)
		return -EINVAL;

	fie->code = RN6854_MEDIA_BUS_FMT;
	fie->width = supported_modes[fie->index].width;
	fie->height = supported_modes[fie->index].height;
	fie->interval = supported_modes[fie->index].max_fps;
	fie->reserved[0] = supported_modes[fie->index].hdr_mode;
	return 0;
}

static int rn6854_g_mbus_config(struct v4l2_subdev *sd,
				struct v4l2_mbus_config *cfg)
{
	cfg->type = V4L2_MBUS_CSI2;
	cfg->flags = V4L2_MBUS_CSI2_4_LANE | V4L2_MBUS_CSI2_CHANNELS;

	return 0;
}

static void rn6854_get_module_inf(struct rn6854 *rn6854,
				  struct rkmodule_inf *inf)
{
	memset(inf, 0, sizeof(*inf));
	strscpy(inf->base.sensor, RN6854_NAME, sizeof(inf->base.sensor));
	strscpy(inf->base.module, rn6854->module_name,
		sizeof(inf->base.module));
	strscpy(inf->base.lens, rn6854->len_name, sizeof(inf->base.lens));
}

static void rn6854_read_all_vfc(struct rn6854 *rn6854, unsigned char *vfc)
{
	int ch;
	uint8_t fmt, val;
	struct i2c_client *client = rn6854->client;

	for (ch = 0; ch < 4; ch++) {
		val = rn6854->check_status[ch];
		if (val & BIT(4)) {
			/* No video */
			vfc[ch] = RN_RESO_UNKOWN;
		} else {
			fmt = (val & 0xE0) >> 5;
			fmt |= (val & 1) << 3;
			switch (fmt) {
			case 0:
				dev_info(&client->dev, "channel %d det D1 pal", ch);
				vfc[ch] = RN_RESO_D1_PAL;
				break;
			case 1:
				dev_info(&client->dev, "channel %d det 720p pal", ch);
				vfc[ch] = RN_RESO_720P_PAL;
				break;
			case 2:
				dev_info(&client->dev, "channel %d det 1080p pal", ch);
				vfc[ch] = RN_RESO_1080P_PAL;
				break;
			case 4:
				dev_info(&client->dev, "channel %d det D1 ntsc", ch);
				vfc[ch] = RN_RESO_D1_NTSC;
				break;
			case 5:
				dev_info(&client->dev, "channel %d det 720p ntsc", ch);
				vfc[ch] = RN_RESO_720P_NTSC;
				break;
			case 6:
				dev_info(&client->dev, "channel %d det 1080p ntsc", ch);
				vfc[ch] = RN_RESO_1080P_NTSC;
				break;
			default:
				dev_err(&client->dev, "channel %d det unknown video format %d",
					ch, fmt);
				vfc[ch] = RN_RESO_UNKOWN;
				break;
			}
		}
	}
}

static void rn6854_get_vc_fmt_inf(struct rn6854 *rn6854,
				  struct rkmodule_vc_fmt_info *inf)
{
	int ch = 0;
	unsigned char ch_vfc[4] = { 0xff, 0xff, 0xff, 0xff };
	struct i2c_client *client = rn6854->client;

	rn6854_read_all_vfc(rn6854, ch_vfc);

	dev_dbg(&client->dev, "%s: 0x%x 0x%x 0x%x 0x%x", __func__,
		ch_vfc[0], ch_vfc[1], ch_vfc[2], ch_vfc[3]);

	for (ch = 0; ch < 4; ch++) {
		switch (ch_vfc[ch]) {
		case RN_RESO_D1_PAL:
			inf->width[ch] = 720;
			inf->height[ch] = 576;
			inf->fps[ch] = 25;
			break;
		case RN_RESO_D1_NTSC:
			inf->width[ch] = 720;
			inf->height[ch] = 480;
			inf->fps[ch] = 30;
			break;
		case RN_RESO_720P_PAL:
			inf->width[ch] = 1280;
			inf->height[ch] = 720;
			inf->fps[ch] = 25;
			break;
		case RN_RESO_720P_NTSC:
			inf->width[ch] = 1280;
			inf->height[ch] = 720;
			inf->fps[ch] = 30;
			break;
		case RN_RESO_1080P_PAL:
			inf->width[ch] = 1920;
			inf->height[ch] = 1080;
			inf->fps[ch] = 25;
			break;
		case RN_RESO_1080P_NTSC:
			inf->width[ch] = 1920;
			inf->height[ch] = 1080;
			inf->fps[ch] = 30;
			break;
		default:
#ifdef RN6854_FMT_CVBS25x4
			inf->width[ch]  = 720;
			inf->height[ch] = 604;
			inf->fps[ch] = 25;
#else
			inf->width[ch]  = 0;
			inf->height[ch] = 0;
			inf->fps[ch] = 0;
#endif
			break;
		}
	}
}

static void rn6854_get_vc_hotplug_inf(struct rn6854 *rn6854,
				      struct rkmodule_vc_hotplug_info *inf)
{
	memset(inf, 0, sizeof(*inf));
	rn6854_auto_detect_hotplug(rn6854);
}

static void rn6854_get_vicap_rst_inf(struct rn6854 *rn6854,
				     struct rkmodule_vicap_reset_info *rst_info)
{
	rst_info->is_reset = rn6854->is_reset;
	rst_info->src = RKCIF_RESET_SRC_ERR_HOTPLUG;
}

static void rn6854_set_vicap_rst_inf(struct rn6854 *rn6854,
				     struct rkmodule_vicap_reset_info rst_info)
{
	rn6854->is_reset = rst_info.is_reset;
}

static long rn6854_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	struct rn6854 *rn6854 = to_rn6854(sd);
	long ret = 0;
	u32 stream = 0;

	switch (cmd) {
	case RKMODULE_GET_MODULE_INFO:
		rn6854_get_module_inf(rn6854, (struct rkmodule_inf *)arg);
		break;
	case RKMODULE_GET_VC_FMT_INFO:
		rn6854_get_vc_fmt_inf(rn6854, (struct rkmodule_vc_fmt_info *)arg);
		break;
	case RKMODULE_GET_VC_HOTPLUG_INFO:
		rn6854_get_vc_hotplug_inf(rn6854, (struct rkmodule_vc_hotplug_info *)arg);
		break;
	case RKMODULE_GET_VICAP_RST_INFO:
		rn6854_get_vicap_rst_inf(rn6854, (struct rkmodule_vicap_reset_info *)arg);
		break;
	case RKMODULE_SET_VICAP_RST_INFO:
		rn6854_set_vicap_rst_inf(rn6854, *(struct rkmodule_vicap_reset_info *)arg);
		break;
	case RKMODULE_GET_START_STREAM_SEQ:
		*(int *)arg = RKMODULE_START_STREAM_FRONT;
		break;
	case RKMODULE_SET_QUICK_STREAM:
		stream = *((u32 *)arg);
		ret = rn6854_set_quick_stream(rn6854, !!stream);
		break;
	default:
		ret = -ENOTTY;
		break;
	}

	return ret;
}

#ifdef CONFIG_COMPAT
static long rn6854_compat_ioctl32(struct v4l2_subdev *sd,
				  unsigned int cmd, unsigned long arg)
{
	void __user *up = compat_ptr(arg);
	struct rkmodule_inf *inf;
	struct rkmodule_vc_fmt_info *vc_fmt_inf;
	struct rkmodule_vc_hotplug_info *vc_hp_inf;
	struct rkmodule_vicap_reset_info *vicap_rst_inf;
	int *seq;
	long ret = 0;

	switch (cmd) {
	case RKMODULE_GET_MODULE_INFO:
		inf = kzalloc(sizeof(*inf), GFP_KERNEL);
		if (!inf) {
			ret = -ENOMEM;
			return ret;
		}

		ret = rn6854_ioctl(sd, cmd, inf);
		if (!ret) {
			if (copy_to_user(up, inf, sizeof(*inf)))
				return -EFAULT;
		}
		kfree(inf);
		break;
	case RKMODULE_GET_VC_FMT_INFO:
		vc_fmt_inf = kzalloc(sizeof(*vc_fmt_inf), GFP_KERNEL);
		if (!vc_fmt_inf) {
			ret = -ENOMEM;
			return ret;
		}

		ret = rn6854_ioctl(sd, cmd, vc_fmt_inf);
		if (!ret) {
			if (copy_to_user(up, vc_fmt_inf, sizeof(*vc_fmt_inf)))
				return -EFAULT;
		}
		kfree(vc_fmt_inf);
		break;
	case RKMODULE_GET_VC_HOTPLUG_INFO:
		vc_hp_inf = kzalloc(sizeof(*vc_hp_inf), GFP_KERNEL);
		if (!vc_hp_inf) {
			ret = -ENOMEM;
			return ret;
		}

		ret = rn6854_ioctl(sd, cmd, vc_hp_inf);
		if (!ret) {
			if (copy_to_user(up, vc_hp_inf, sizeof(*vc_hp_inf)))
				return -EFAULT;
		}
		kfree(vc_hp_inf);
		break;
	case RKMODULE_GET_VICAP_RST_INFO:
		vicap_rst_inf = kzalloc(sizeof(*vicap_rst_inf), GFP_KERNEL);
		if (!vicap_rst_inf) {
			ret = -ENOMEM;
			return ret;
		}

		ret = rn6854_ioctl(sd, cmd, vicap_rst_inf);
		if (!ret) {
			if (copy_to_user(up, vicap_rst_inf, sizeof(*vicap_rst_inf)))
				return -EFAULT;
		}
		kfree(vicap_rst_inf);
		break;
	case RKMODULE_SET_VICAP_RST_INFO:
		vicap_rst_inf = kzalloc(sizeof(*vicap_rst_inf), GFP_KERNEL);
		if (!vicap_rst_inf) {
			ret = -ENOMEM;
			return ret;
		}

		if (copy_from_user(vicap_rst_inf, up, sizeof(*vicap_rst_inf)))
			return -EFAULT;
		ret = rn6854_ioctl(sd, cmd, vicap_rst_inf);
		kfree(vicap_rst_inf);
		break;
	case RKMODULE_GET_START_STREAM_SEQ:
		seq = kzalloc(sizeof(*seq), GFP_KERNEL);
		if (!seq) {
			ret = -ENOMEM;
			return ret;
		}

		ret = rn6854_ioctl(sd, cmd, seq);
		if (!ret) {
			if (copy_to_user(up, seq, sizeof(*seq)))
				return -EFAULT;
		}
		kfree(seq);
		break;
	default:
		ret = -ENOIOCTLCMD;
		break;
	}

	return ret;
}
#endif

static __maybe_unused void rn6854_manual_mode(struct rn6854 *rn6854)
{
	int array_size = 0;

	if (rn6854->cur_mode.global_reg_list == common_setting_1080P30x2_regs)
		array_size = ARRAY_SIZE(common_setting_1080P30x2_regs);
	else if (rn6854->cur_mode.global_reg_list == common_setting_1080P25x2_regs)
		array_size = ARRAY_SIZE(common_setting_1080P25x2_regs);
	else if (rn6854->cur_mode.global_reg_list == common_setting_720P30x4_regs)
		array_size = ARRAY_SIZE(common_setting_720P30x4_regs);
	else if (rn6854->cur_mode.global_reg_list == common_setting_720P25x4_regs)
		array_size = ARRAY_SIZE(common_setting_720P25x4_regs);
	else if (rn6854->cur_mode.global_reg_list == common_setting_720P25x1_regs)
		array_size = ARRAY_SIZE(common_setting_720P25x1_regs);
	else if (rn6854->cur_mode.global_reg_list == common_setting_cvbsx4_pal_regs)
		array_size = ARRAY_SIZE(common_setting_cvbsx4_pal_regs);

	rn6854_write_array(rn6854->client, rn6854->cur_mode.global_reg_list, array_size);
}

static int detect_thread_function(void *data)
{
	struct rn6854 *rn6854 = (struct rn6854 *) data;
	struct i2c_client *client = rn6854->client;
	int need_reset_wait = -1;

	if (rn6854->power_on) {
		rn6854_auto_detect_hotplug(rn6854);
		rn6854->is_reset = 0;
	}
	while (!kthread_should_stop()) {
		if (rn6854->power_on) {
			rn6854_auto_detect_hotplug(rn6854);
			if (rn6854->detect_status != 0) {
				input_event(rn6854->input_dev, EV_MSC,
					    MSC_RAW, rn6854->detect_status);
				input_sync(rn6854->input_dev);
				need_reset_wait = 5;
			}
			if (need_reset_wait > 0) {
				need_reset_wait--;
			} else if (need_reset_wait == 0) {
				need_reset_wait = -1;
				rn6854->is_reset = 1;
				dev_info(&client->dev, "trigger reset time up\n");
			}
		}
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(msecs_to_jiffies(500));
	}

	return 0;
}

static int __maybe_unused detect_thread_start(struct rn6854 *rn6854)
{
	int ret = 0;
	struct i2c_client *client = rn6854->client;

	rn6854->detect_thread = kthread_create(detect_thread_function,
					       rn6854, "rn6854_kthread");

	if (IS_ERR(rn6854->detect_thread)) {
		dev_err(&client->dev, "kthread_create rn6854_kthread failed\n");
		ret = PTR_ERR(rn6854->detect_thread);
		rn6854->detect_thread = NULL;
		return ret;
	}
	wake_up_process(rn6854->detect_thread);

	return ret;
}

static int __maybe_unused detect_thread_stop(struct rn6854 *rn6854)
{
	if (rn6854->detect_thread)
		kthread_stop(rn6854->detect_thread);
	rn6854->detect_thread = NULL;

	return 0;
}

static int __rn6854_start_stream(struct rn6854 *rn6854)
{
	mutex_lock(&rn6854->i2c_mutex);
	rn6854_pre_initial(rn6854->client);
	rn6854_manual_mode(rn6854);
	mutex_unlock(&rn6854->i2c_mutex);

	return 0;
}

static int __rn6854_stop_stream(struct rn6854 *rn6854)
{
	struct i2c_client *client = rn6854->client;

	mutex_lock(&rn6854->i2c_mutex);
	rn6854_write_reg(client, 0xff, 0x08); // Selects MIPI CSI1 register set
	rn6854_write_reg(client, 0x6C, 0x10); // reset and disable all chan
	//rn6854_write_reg(client, 0x06, 0x40); // disable all mipi lane
	rn6854_write_reg(client, 0x04, 0x03); // reset csi dphy
	mutex_unlock(&rn6854->i2c_mutex);
	return 0;
}

static int rn6854_stream(struct v4l2_subdev *sd, int on)
{
	struct rn6854 *rn6854 = to_rn6854(sd);
	struct i2c_client *client = rn6854->client;

	dev_info(&client->dev, "s_stream: %d. %dx%d\n", on,
		 rn6854->cur_mode.width,
		 rn6854->cur_mode.height);

	mutex_lock(&rn6854->mutex);
	on = !!on;
	if (rn6854->streaming == on)
		goto unlock;

	if (on)
		__rn6854_start_stream(rn6854);
	else
		__rn6854_stop_stream(rn6854);

	rn6854->streaming = on;

unlock:
	mutex_unlock(&rn6854->mutex);

	return 0;
}

static int rn6854_power(struct v4l2_subdev *sd, int on)
{
	struct rn6854 *rn6854 = to_rn6854(sd);
	struct i2c_client *client = rn6854->client;
	int ret = 0;

	mutex_lock(&rn6854->mutex);

	/* If the power state is not modified - no work to do. */
	if (rn6854->power_on == !!on)
		goto exit;

	if (on) {
		ret = pm_runtime_get_sync(&client->dev);
		if (ret < 0) {
			pm_runtime_put_noidle(&client->dev);
			goto exit;
		}
		rn6854->power_on = true;
	} else {
		pm_runtime_put(&client->dev);
		rn6854->power_on = false;
	}

exit:
	mutex_unlock(&rn6854->mutex);

	return ret;
}

static int __rn6854_reset(struct rn6854 *rn6854)
{
	if (!IS_ERR(rn6854->reset_gpio)) {
		gpiod_set_value_cansleep(rn6854->reset_gpio, 1);
		usleep_range(30 * 1000, 50 * 1000);

		gpiod_set_value_cansleep(rn6854->reset_gpio, 0);
		usleep_range(50 * 1000, 70 * 1000);
	}
	return 0;
}

static int __rn6854_power_on(struct rn6854 *rn6854)
{
	int ret;
	struct device *dev = &rn6854->client->dev;

	if (!IS_ERR_OR_NULL(rn6854->pins_default)) {
		ret = pinctrl_select_state(rn6854->pinctrl,
					   rn6854->pins_default);
		if (ret < 0)
			dev_err(dev, "could not set pins. ret=%d\n", ret);
	}

	if (!IS_ERR(rn6854->power_gpio)) {
		gpiod_set_value_cansleep(rn6854->power_gpio, 1);
		usleep_range(20 * 1000, 40 * 1000);
	}

	usleep_range(1500, 2000);

	ret = clk_set_rate(rn6854->xvclk, RN6854_XVCLK_FREQ);
	if (ret < 0)
		dev_warn(dev, "Failed to set xvclk rate\n");
	if (clk_get_rate(rn6854->xvclk) != RN6854_XVCLK_FREQ)
		dev_warn(dev, "xvclk mismatched\n");
	ret = clk_prepare_enable(rn6854->xvclk);
	if (ret < 0) {
		dev_err(dev, "Failed to enable xvclk\n");
		goto err_clk;
	}

	ret = regulator_bulk_enable(RN6854_NUM_SUPPLIES, rn6854->supplies);
	if (ret < 0) {
		dev_err(dev, "Failed to enable regulators\n");
		goto err_clk;
	}
	__rn6854_reset(rn6854);

	return 0;

err_clk:
	if (!IS_ERR(rn6854->reset_gpio))
		gpiod_set_value_cansleep(rn6854->reset_gpio, 1);

	clk_disable_unprepare(rn6854->xvclk);
	if (!IS_ERR_OR_NULL(rn6854->pins_sleep))
		pinctrl_select_state(rn6854->pinctrl, rn6854->pins_sleep);

	return ret;
}

static void __rn6854_power_off(struct rn6854 *rn6854)
{
	int ret;
	struct device *dev = &rn6854->client->dev;

	if (!IS_ERR(rn6854->reset_gpio))
		gpiod_set_value_cansleep(rn6854->reset_gpio, 1);

	clk_disable_unprepare(rn6854->xvclk);

	if (!IS_ERR_OR_NULL(rn6854->pins_sleep)) {
		ret = pinctrl_select_state(rn6854->pinctrl,
					   rn6854->pins_sleep);
		if (ret < 0)
			dev_err(dev, "could not set pins\n");
	}

	regulator_bulk_disable(RN6854_NUM_SUPPLIES, rn6854->supplies);
	if (!IS_ERR(rn6854->power_gpio)) {
		gpiod_set_value_cansleep(rn6854->power_gpio, 0);
		// to ensure camera is power down
		usleep_range(50 * 1000, 80 * 1000);
	}
}

static int rn6854_initialize_controls(struct rn6854 *rn6854)
{
	const struct rn6854_mode *mode;
	struct v4l2_ctrl_handler *handler;
	u64 pixel_rate;
	int ret;

	handler = &rn6854->ctrl_handler;
	mode = &rn6854->cur_mode;
	ret = v4l2_ctrl_handler_init(handler, 2);
	if (ret)
		return ret;
	handler->lock = &rn6854->mutex;

	rn6854->link_freq = v4l2_ctrl_new_int_menu(handler, NULL,
						   V4L2_CID_LINK_FREQ,
						   ARRAY_SIZE(link_freq_items) - 1, 0,
						   link_freq_items);
	__v4l2_ctrl_s_ctrl(rn6854->link_freq, mode->mipi_freq_idx);

	/* pixel rate = link frequency * 2 * lanes / BITS_PER_SAMPLE */
	pixel_rate = (u32)link_freq_items[mode->mipi_freq_idx] / mode->bpp * 2 * RN6854_LANES;
	rn6854->pixel_rate = v4l2_ctrl_new_std(handler, NULL,
					       V4L2_CID_PIXEL_RATE,
					       0, pixel_rate, 1, pixel_rate);
	if (handler->error) {
		ret = handler->error;
		dev_err(&rn6854->client->dev,
			"Failed to init controls(%d)\n", ret);
		goto err_free_handler;
	}

	dev_info(&rn6854->client->dev, "mipi_freq_idx %d\n", mode->mipi_freq_idx);
	dev_info(&rn6854->client->dev, "pixel_rate %lld\n", pixel_rate);
	dev_info(&rn6854->client->dev, "link_freq %lld\n", link_freq_items[mode->mipi_freq_idx]);

	rn6854->subdev.ctrl_handler = handler;

	return 0;

err_free_handler:
	v4l2_ctrl_handler_free(handler);

	return ret;
}

static int rn6854_runtime_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct rn6854 *rn6854 = to_rn6854(sd);

	return __rn6854_power_on(rn6854);
}

static int rn6854_runtime_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct rn6854 *rn6854 = to_rn6854(sd);

	__rn6854_power_off(rn6854);

	return 0;
}

#ifdef CONFIG_VIDEO_V4L2_SUBDEV_API
static int rn6854_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct rn6854 *rn6854 = to_rn6854(sd);
	struct v4l2_mbus_framefmt *try_fmt =
		v4l2_subdev_get_try_format(sd, fh->pad, 0);
#ifdef RN6854_FMT_CVBS25x4
	const struct rn6854_mode *def_mode = &supported_modes[2];
#else
	const struct rn6854_mode *def_mode = &supported_modes[1];
#endif

	mutex_lock(&rn6854->mutex);
	/* Initialize try_fmt */
	try_fmt->width = def_mode->width;
	try_fmt->height = def_mode->height;
	try_fmt->code = def_mode->bus_fmt;
	try_fmt->field = V4L2_FIELD_NONE;

	mutex_unlock(&rn6854->mutex);
	/* No crop or compose */

	return 0;
}
#endif

#ifdef CONFIG_VIDEO_V4L2_SUBDEV_API
static const struct v4l2_subdev_internal_ops rn6854_internal_ops = {
	.open = rn6854_open,
};
#endif

static const struct v4l2_subdev_video_ops rn6854_video_ops = {
	.s_stream = rn6854_stream,
	.g_mbus_config = rn6854_g_mbus_config,
};

static const struct v4l2_subdev_pad_ops rn6854_subdev_pad_ops = {
	.enum_mbus_code = rn6854_enum_mbus_code,
	.enum_frame_size = rn6854_enum_frame_sizes,
	.enum_frame_interval = rn6854_enum_frame_interval,
	.get_fmt = rn6854_get_fmt,
	.set_fmt = rn6854_set_fmt,
};

static const struct v4l2_subdev_core_ops rn6854_core_ops = {
	.s_power = rn6854_power,
	.ioctl   = rn6854_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl32 = rn6854_compat_ioctl32,
#endif
};

static const struct v4l2_subdev_ops rn6854_subdev_ops = {
	.core = &rn6854_core_ops,
	.video = &rn6854_video_ops,
	.pad   = &rn6854_subdev_pad_ops,
};

static int check_chip_id(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	unsigned char chip_id = 0;

	rn6854_read_reg(client, RN6854_ID_ADDR, &chip_id);

	if (chip_id != RN6854_ID) {
		dev_err(dev, "wrong chip id 0x%x\n", chip_id);
		return -EINVAL;
	}
	dev_info(dev, "detect chip id: 0x%x\n", RN6854_ID);

	return 0;
}

static int rn6854_configure_regulators(struct rn6854 *rn6854)
{
	unsigned int i;

	for (i = 0; i < RN6854_NUM_SUPPLIES; i++)
		rn6854->supplies[i].supply = rn6854_supply_names[i];

	return devm_regulator_bulk_get(&rn6854->client->dev,
				       RN6854_NUM_SUPPLIES,
				       rn6854->supplies);
}

static int rn6854_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct device_node *node = dev->of_node;
	struct rn6854 *rn6854;
	struct v4l2_subdev *sd;
	__maybe_unused char facing[2];
	int ret;

	dev_info(dev, "driver version: %02x.%02x.%02x",
		 DRIVER_VERSION >> 16,
		 (DRIVER_VERSION & 0xff00) >> 8,
		 DRIVER_VERSION & 0x00ff);

	rn6854 = devm_kzalloc(dev, sizeof(*rn6854), GFP_KERNEL);
	if (!rn6854)
		return -ENOMEM;

	ret = of_property_read_u32(node, RKMODULE_CAMERA_MODULE_INDEX,
				   &rn6854->module_index);
	ret |= of_property_read_string(node, RKMODULE_CAMERA_MODULE_FACING,
				       &rn6854->module_facing);
	ret |= of_property_read_string(node, RKMODULE_CAMERA_MODULE_NAME,
				       &rn6854->module_name);
	ret |= of_property_read_string(node, RKMODULE_CAMERA_LENS_NAME,
				       &rn6854->len_name);
	if (ret) {
		dev_err(dev, "could not get %s!\n", RKMODULE_CAMERA_LENS_NAME);
		return -EINVAL;
	}

	rn6854->client = client;
	rn6854->cfg_num = ARRAY_SIZE(supported_modes);

#ifdef RN6854_FMT_CVBS25x4
	memcpy(&rn6854->cur_mode, &supported_modes[2], sizeof(struct rn6854_mode));
#else
	memcpy(&rn6854->cur_mode, &supported_modes[1], sizeof(struct rn6854_mode));
#endif
	memset(rn6854->check_status, 0xFF, sizeof(rn6854->check_status));

	rn6854->xvclk = devm_clk_get(dev, "xvclk");
	if (IS_ERR(rn6854->xvclk)) {
		dev_err(dev, "Failed to get xvclk\n");
		return -EINVAL;
	}

	rn6854->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(rn6854->reset_gpio))
		dev_warn(dev, "Failed to get reset-gpios\n");

	rn6854->power_gpio = devm_gpiod_get(dev, "power", GPIOD_OUT_HIGH);
	if (IS_ERR(rn6854->power_gpio))
		dev_warn(dev, "Failed to get power-gpios\n");

	rn6854->pinctrl = devm_pinctrl_get(dev);
	if (!IS_ERR(rn6854->pinctrl)) {
		rn6854->pins_default =
			pinctrl_lookup_state(rn6854->pinctrl,
					     OF_CAMERA_PINCTRL_STATE_DEFAULT);
		if (IS_ERR(rn6854->pins_default))
			dev_err(dev, "could not get default pinstate\n");

		rn6854->pins_sleep =
			pinctrl_lookup_state(rn6854->pinctrl,
					     OF_CAMERA_PINCTRL_STATE_SLEEP);
		if (IS_ERR(rn6854->pins_sleep))
			dev_err(dev, "could not get sleep pinstate\n");
	} else {
		dev_err(dev, "no pinctrl\n");
	}
	ret = rn6854_configure_regulators(rn6854);
	if (ret) {
		dev_err(dev, "Failed to config regulators\n");
		return -EINVAL;
	}

	mutex_init(&rn6854->mutex);
	mutex_init(&rn6854->i2c_mutex);

	sd = &rn6854->subdev;
	v4l2_i2c_subdev_init(sd, client, &rn6854_subdev_ops);
	ret = rn6854_initialize_controls(rn6854);
	if (ret) {
		dev_err(dev, "Failed to initialize controls rn6854\n");
		goto err_destroy_mutex;
	}

	ret = __rn6854_power_on(rn6854);
	if (ret) {
		dev_err(dev, "Failed to power on rn6854\n");
		goto err_free_handler;
	}

	ret = check_chip_id(client);
	if (ret) {
		dev_err(dev, "Failed to check senosr id\n");
		goto err_free_handler;
	}

#ifdef CONFIG_VIDEO_V4L2_SUBDEV_API
	sd->internal_ops = &rn6854_internal_ops;
	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
#endif

#if defined(CONFIG_MEDIA_CONTROLLER)
	rn6854->pad.flags = MEDIA_PAD_FL_SOURCE;
	sd->entity.function = MEDIA_ENT_F_CAM_SENSOR;
	ret = media_entity_pads_init(&sd->entity, 1, &rn6854->pad);
	if (ret < 0)
		goto err_power_off;
#endif

	memset(facing, 0, sizeof(facing));
	if (strcmp(rn6854->module_facing, "back") == 0)
		facing[0] = 'b';
	else
		facing[0] = 'f';

	snprintf(sd->name, sizeof(sd->name), "m%02d_%s_%s %s",
		 rn6854->module_index, facing,
		 RN6854_NAME, dev_name(sd->dev));

	ret = v4l2_async_register_subdev_sensor_common(sd);
	if (ret) {
		dev_err(dev, "v4l2 async register subdev failed\n");
		goto err_clean_entity;
	}

	if (sysfs_create_group(&dev->kobj, &dev_attr_grp))
		return -ENODEV;

	rn6854->input_dev = devm_input_allocate_device(dev);
	if (rn6854->input_dev == NULL) {
		dev_err(dev, "failed to allocate rn6854 input device\n");
		return -ENOMEM;
	}
	rn6854->input_dev->name = sd->name;
	set_bit(EV_MSC,  rn6854->input_dev->evbit);
	set_bit(MSC_RAW, rn6854->input_dev->mscbit);

	ret = input_register_device(rn6854->input_dev);
	if (ret) {
		pr_err("%s: failed to register rn6854 input device\n", __func__);
		return ret;
	}

	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
	pm_runtime_idle(dev);

	detect_thread_start(rn6854);

	dev_info(dev, "AHD rn6854 probe succeed!\n");

	return 0;

err_clean_entity:
#if defined(CONFIG_MEDIA_CONTROLLER)
	media_entity_cleanup(&sd->entity);
#endif
err_power_off:
	__rn6854_power_off(rn6854);
err_free_handler:
	v4l2_ctrl_handler_free(&rn6854->ctrl_handler);
err_destroy_mutex:
	mutex_destroy(&rn6854->mutex);
	mutex_destroy(&rn6854->i2c_mutex);

	return ret;
}

static int rn6854_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct rn6854 *rn6854 = to_rn6854(sd);

	detect_thread_stop(rn6854);

	v4l2_async_unregister_subdev(sd);
#if defined(CONFIG_MEDIA_CONTROLLER)
	media_entity_cleanup(&sd->entity);
#endif
	v4l2_ctrl_handler_free(&rn6854->ctrl_handler);
	mutex_destroy(&rn6854->mutex);
	mutex_destroy(&rn6854->i2c_mutex);

	pm_runtime_disable(&client->dev);
	if (!pm_runtime_status_suspended(&client->dev))
		__rn6854_power_off(rn6854);
	pm_runtime_set_suspended(&client->dev);

	return 0;
}

static const struct dev_pm_ops rn6854_pm_ops = {
	SET_RUNTIME_PM_OPS(rn6854_runtime_suspend,
			   rn6854_runtime_resume, NULL)
};

#if IS_ENABLED(CONFIG_OF)
static const struct of_device_id rn6854_of_match[] = {
	{ .compatible = "richnex,rn6854" },
	{},
};
MODULE_DEVICE_TABLE(of, rn6854_of_match);
#endif

static const struct i2c_device_id rn6854_match_id[] = {
	{ "richnex,rn6854", 0 },
	{ },
};

static struct i2c_driver rn6854_i2c_driver = {
	.driver = {
		.name = RN6854_NAME,
		.pm = &rn6854_pm_ops,
		.of_match_table = of_match_ptr(rn6854_of_match),
	},
	.probe		= &rn6854_probe,
	.remove		= &rn6854_remove,
	.id_table	= rn6854_match_id,
};

static int __init sensor_mod_init(void)
{
	return i2c_add_driver(&rn6854_i2c_driver);
}

static void __exit sensor_mod_exit(void)
{
	i2c_del_driver(&rn6854_i2c_driver);
}

device_initcall_sync(sensor_mod_init);
module_exit(sensor_mod_exit);

MODULE_AUTHOR("Vicent Chi <vicent.chi@rock-chips.com>");
MODULE_AUTHOR("lba <libinan@imagintech.com.cn>");
MODULE_DESCRIPTION("rn6854 sensor driver");
MODULE_LICENSE("GPL");
