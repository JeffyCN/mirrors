/*
 * Support for Medifield PNW Camera Imaging ISP subsystem.
 *
 * Copyright (c) 2010 Intel Corporation. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#ifndef _ATOM_ISP_H
#define _ATOM_ISP_H

#include <linux/types.h>
#include <linux/version.h>
#include "atomisp_parameters.h"

#define ATOMISP_CSS_VERSION_15	KERNEL_VERSION(1, 5, 0)
#define ATOMISP_CSS_VERSION_17	KERNEL_VERSION(1, 7, 0)
#define ATOMISP_CSS_VERSION_20	KERNEL_VERSION(2, 0, 0)

/*ISP binary running mode*/
#define CI_MODE_PREVIEW		0x8000
#define CI_MODE_VIDEO		0x4000
#define CI_MODE_STILL_CAPTURE	0x2000
#define CI_MODE_CONTINUOUS	0x1000
#define CI_MODE_NONE		0x0000

#define OUTPUT_MODE_FILE 0x0100
#define OUTPUT_MODE_TEXT 0x0200

//uncomment the below line to see timestamps for the important functions
//#define TIMING

#ifdef TIMING
        #define TIMING_ENTER printk("TIMING: %s: ENTRY \n", __func__);
        #define TIMING_END printk("TIMING: %s: EXIT \n", __func__);
#else
        #define TIMING_ENTER
        #define TIMING_END
#endif



enum atomisp_calibration_type {
	calibration_type1,
	calibration_type2,
	calibration_type3
};

struct atomisp_calibration_group {
	unsigned int size;
	unsigned int type;
	unsigned short *calb_grp_values;
};

struct atomisp_3a_output {
	int ae_y;
	int awb_cnt;
	int awb_gr;
	int awb_r;
	int awb_b;
	int awb_gb;
	int af_hpf1;
	int af_hpf2;
};

struct atomisp_3a_config {
	unsigned int ae_y_coef_r;	/* [gain] Weight of R for Y */
	unsigned int ae_y_coef_g;	/* [gain] Weight of G for Y */
	unsigned int ae_y_coef_b;	/* [gain] Weight of B for Y */
	unsigned int awb_lg_high_raw;	/* [intensity]
					   AWB level gate high for raw */
	unsigned int awb_lg_low;	/* [intensity] AWB level gate low */
	unsigned int awb_lg_high;	/* [intensity] AWB level gate high */
	int af_fir1_coef[7];	/* [factor] AF FIR coefficients of fir1 */
	int af_fir2_coef[7];	/* [factor] AF FIR coefficients of fir2 */
};

struct atomisp_metadata {
        void __user *data;
        uint32_t width;
        uint32_t height;
        uint32_t stride; /* in bytes */
        uint32_t exp_id; /* exposure ID */
        uint32_t *effective_width; /* mipi packets valid data size */
};

/* structure that describes the 3A and DIS grids shared with 3A lib*/
/*struct atomisp_grid_info {
	 ISP input size that is visible for user
	unsigned int isp_in_width;
	unsigned int isp_in_height;
	 3A statistics grid:
	unsigned int s3a_width;
	unsigned int s3a_height;
	unsigned int s3a_bqs_per_grid_cell;
	 DIS grid:
	unsigned int dis_width;   also used for vertical projections
	unsigned int dis_aligned_width;
	unsigned int dis_height;  also used for horizontal projections
	unsigned int dis_aligned_height;
	unsigned int dis_bqs_per_grid_cell;
	unsigned int dis_hor_coef_num;
	unsigned int dis_ver_coef_num;
};*/

struct atomisp_grid_info {
	unsigned int enable;            /**< 3A statistics enabled.*/
	unsigned int use_dmem;          /**< DMEM or VMEM determines layout. */
	unsigned int has_histogram;     /**< Statistics include histogram. */
	unsigned int width;	    	    /**< Width of 3A grid table. */
	unsigned int height;	    	/**< Height of 3A grid table.*/
	unsigned int aligned_width;     /**< Horizontal stride (for alloc). */
	unsigned int aligned_height;    /**< Vertical stride (for alloc).*/
	unsigned int bqs_per_grid_cell; /**< Grid cell size in BQ(Bayer Quad) unit.*/
	unsigned int deci_factor_log2;  /**< log2 of bqs_per_grid_cell. */
	unsigned int elem_bit_depth;
};

struct atomisp_dis_vector {
	int x;
	int y;
};

struct atomisp_3a_rgby_output {
        uint32_t r;
        uint32_t g;
        uint32_t b;
        uint32_t y;
};


struct atomisp_dis_coefficients {
	struct atomisp_grid_info grid_info;
	short __user *vertical_coefficients;
	short __user *horizontal_coefficients;
};

struct atomisp_dis_statistics {
	struct atomisp_grid_info grid_info;
	int __user *vertical_projections;
	int __user *horizontal_projections;
};

struct atomisp_3a_statistics {
	struct atomisp_grid_info  grid_info;
	struct atomisp_3a_output __user *data;
	struct atomisp_3a_rgby_output __user *rgby_data;
	unsigned int exp_id;
};

/**
 * struct atomisp_cont_capture_conf - continuous capture parameters
 * @num_captures: number of still images to capture
 * @skip_frames: number of frames to skip between 2 captures
 * @offset: offset in ring buffer to start capture
 *
 * For example, to capture 1 frame from past, current, and 1 from future
 * and skip one frame between each capture, parameters would be:
 * num_captures:3
 * skip_frames:1
 * offset:-2
 */

struct atomisp_cont_capture_conf {
	int num_captures;
	unsigned int skip_frames;
	int offset;
	__u32 reserved[5];
};


struct atomisp_metadata_config {
        uint32_t metadata_height;
        uint32_t metadata_stride;
};

struct atomisp_parm {
	struct atomisp_grid_info info;
	struct atomisp_wb_config wb_config;
	struct atomisp_cc_config cc_config;
	struct atomisp_ob_config ob_config;
	struct atomisp_de_config de_config;
	struct atomisp_ce_config ce_config;
	struct atomisp_dp_config dp_config;
	struct atomisp_nr_config nr_config;
	struct atomisp_ee_config ee_config;
	struct atomisp_tnr_config tnr_config;
	struct atomisp_metadata_config metadata_config;
};

struct atomisp_parameters {
	struct atomisp_wb_config *wb_config;
	struct atomisp_cc_config *cc_config;
	struct atomisp_tnr_config *tnr_config;
	struct atomisp_ecd_config  *ecd_config; /* Eigen Color Demosaicing */
	struct atomisp_ynr_config  *ynr_config; /* Y(Luma) Noise Reduction */
	struct atomisp_fc_config   *fc_config;  /* Fringe Control */
	struct atomisp_formats_config *formats_config; /* Format Control*/
	struct atomisp_cnr_config  *cnr_config; /* Chroma Noise Reduction */
	struct atomisp_macc_config *macc_config;
	struct atomisp_ctc_config  *ctc_config; /* Chroma Tone Control */
	struct atomisp_aa_config   *aa_config;  /* Anti-Aliasing */
	struct atomisp_aa_config   *baa_config;  /* Anti-Aliasing */
	struct atomisp_ce_config *ce_config;
	struct atomisp_dvs_6axis_config *dvs_6axis_config;
	struct atomisp_ob_config *ob_config;
	struct atomisp_dp_config *dp_config;
	struct atomisp_nr_config *nr_config;
	struct atomisp_ee_config *ee_config;
	struct atomisp_de_config *de_config;
	struct atomisp_gc_config *gc_config;
	struct atomisp_anr_config  *anr_config; /* Advanced Noise Reduction */
	struct atomisp_3a_config *a3a_config;
	struct atomisp_xnr_config *xnr_config;
	struct atomisp_dz_config   *dz_config;  /* Digital Zoom */
	struct atomisp_cc_config *yuv2rgb_cc_config; /* Color Correction config */
	struct atomisp_cc_config *rgb2yuv_cc_config; /* Color Correction config */
	struct atomisp_macc_table *macc_table;
	struct atomisp_gamma_table *gamma_table;
	struct atomisp_ctc_table *ctc_table;
	struct atomisp_xnr_table *xnr_table;
	struct atomisp_rgb_gamma_table *r_gamma_table;
	struct atomisp_rgb_gamma_table *g_gamma_table;
	struct atomisp_rgb_gamma_table *b_gamma_table;
	struct atomisp_vector      *motion_vector; /* For 2-axis DVS */
	struct atomisp_shading_table *shading_table;
	struct atomisp_morph_table *morph_table;
	struct atomisp_anr_thres   *anr_thres;

};

struct atomisp_makernote_info {
	/* bits 31-16: numerator, bits 15-0: denominator */
	unsigned int focal_length;
	/* bits 31-16: numerator, bits 15-0: denominator*/
	unsigned int f_number_curr;
	/*
	* bits 31-24: max f-number numerator
	* bits 23-16: max f-number denominator
	* bits 15-8: min f-number numerator
	* bits 7-0: min f-number denominator
	*/
	unsigned int f_number_range;
};

/* Parameter for overlay image loading */
struct atomisp_overlay {
	/* the frame containing the overlay data The overlay frame width should
	 * be the multiples of 2*ISP_VEC_NELEMS. The overlay frame height
	 * should be the multiples of 2.
	 */
	struct v4l2_framebuffer *frame;
	/* Y value of overlay background */
	unsigned char bg_y;
	/* U value of overlay background */
	char bg_u;
	/* V value of overlay background */
	char bg_v;
	/* the blending percent of input data for Y subpixels */
	unsigned char blend_input_perc_y;
	/* the blending percent of input data for U subpixels */
	unsigned char blend_input_perc_u;
	/* the blending percent of input data for V subpixels */
	unsigned char blend_input_perc_v;
	/* the blending percent of overlay data for Y subpixels */
	unsigned char blend_overlay_perc_y;
	/* the blending percent of overlay data for U subpixels */
	unsigned char blend_overlay_perc_u;
	/* the blending percent of overlay data for V subpixels */
	unsigned char blend_overlay_perc_v;
	/* the overlay start x pixel position on output frame It should be the
	   multiples of 2*ISP_VEC_NELEMS. */
	unsigned int overlay_start_x;
	/* the overlay start y pixel position on output frame It should be the
	   multiples of 2. */
	unsigned int overlay_start_y;
};

/* Sensor resolution specific data for AE calculation.*/
struct atomisp_sensor_mode_data {
	unsigned int coarse_integration_time_min;
	unsigned int coarse_integration_time_max_margin;
	unsigned int fine_integration_time_min;
	unsigned int fine_integration_time_max_margin;
	unsigned int fine_integration_time_def;
	unsigned int frame_length_lines;
	unsigned int line_length_pck;
	unsigned int read_mode;
	unsigned int vt_pix_clk_freq_mhz;
	unsigned int crop_horizontal_start; /* Sensor crop start cord. (x0,y0)*/
	unsigned int crop_vertical_start;
	unsigned int crop_horizontal_end; /* Sensor crop end cord. (x1,y1)*/
	unsigned int crop_vertical_end;
	unsigned int output_width; /* input size to ISP after binning/scaling */
	unsigned int output_height;
	uint8_t binning_factor_x; /* horizontal binning factor used */
	uint8_t binning_factor_y; /* vertical binning factor used */
	uint8_t reserved[2];
};

struct atomisp_exposure {
	unsigned int integration_time[8];
	unsigned int shutter_speed[8];
	unsigned int gain[4];
	unsigned int aperture;
};

/* For texture streaming. */
struct atomisp_bc_video_package {
	int ioctl_cmd;
	int device_id;
	int inputparam;
	int outputparam;
};

enum atomisp_focus_hp {
	ATOMISP_FOCUS_HP_IN_PROGRESS = (1U << 2),
	ATOMISP_FOCUS_HP_COMPLETE    = (2U << 2),
	ATOMISP_FOCUS_HP_FAILED      = (3U << 2)
};

/* Masks */
#define ATOMISP_FOCUS_STATUS_MOVING           (1U << 0)
#define ATOMISP_FOCUS_STATUS_ACCEPTS_NEW_MOVE (1U << 1)
#define ATOMISP_FOCUS_STATUS_HOME_POSITION    (3U << 2)

enum atomisp_camera_port {
	ATOMISP_CAMERA_PORT_PRIMARY,
	ATOMISP_CAMERA_PORT_SECONDARY,
	ATOMISP_CAMERA_PORT_THIRD,
	ATOMISP_CAMERA_NR_PORTS
};

/* Flash modes. Default is off.
 * Setting a flash to TORCH or INDICATOR mode will automatically
 * turn it on. Setting it to FLASH mode will not turn on the flash
 * until the FLASH_STROBE command is sent. */
enum atomisp_flash_mode {
	ATOMISP_FLASH_MODE_OFF,
	ATOMISP_FLASH_MODE_FLASH,
	ATOMISP_FLASH_MODE_TORCH,
	ATOMISP_FLASH_MODE_INDICATOR,
};

/* Flash statuses, used by atomisp driver to check before starting
 * flash and after having started flash. */
enum atomisp_flash_status {
	ATOMISP_FLASH_STATUS_OK,
	ATOMISP_FLASH_STATUS_HW_ERROR,
	ATOMISP_FLASH_STATUS_INTERRUPTED,
	ATOMISP_FLASH_STATUS_TIMEOUT,
};

/* Frame status. This is used to detect corrupted frames and flash
 * exposed frames. Usually, the first 2 frames coming out of the sensor
 * are corrupted. When using flash, the frame before and the frame after
 * the flash exposed frame may be partially exposed by flash. The ISP
 * statistics for these frames should not be used by the 3A library.
 * The frame status value can be found in the "reserved" field in the
 * v4l2_buffer struct. */
enum atomisp_frame_status {
	ATOMISP_FRAME_STATUS_OK,
	ATOMISP_FRAME_STATUS_CORRUPTED,
	ATOMISP_FRAME_STATUS_FLASH_EXPOSED,
	ATOMISP_FRAME_STATUS_FLASH_PARTIAL,
	ATOMISP_FRAME_STATUS_FLASH_FAILED,
};

enum atomisp_acc_type {
	ATOMISP_ACC_STANDALONE,	/* Stand-alone acceleration */
	ATOMISP_ACC_OUTPUT,	/* Accelerator stage on output frame */
	ATOMISP_ACC_VIEWFINDER	/* Accelerator stage on viewfinder frame */
};

enum atomisp_acc_arg_type {
	ATOMISP_ACC_ARG_SCALAR_IN,    /* Scalar input argument */
	ATOMISP_ACC_ARG_SCALAR_OUT,   /* Scalar output argument */
	ATOMISP_ACC_ARG_SCALAR_IO,    /* Scalar in/output argument */
	ATOMISP_ACC_ARG_PTR_IN,	     /* Pointer input argument */
	ATOMISP_ACC_ARG_PTR_OUT,	     /* Pointer output argument */
	ATOMISP_ACC_ARG_PTR_IO,	     /* Pointer in/output argument */
	ATOMISP_ARG_PTR_NOFLUSH,  /* Pointer argument will not be flushed */
	ATOMISP_ARG_PTR_STABLE,   /* Pointer input argument that is stable */
	ATOMISP_ACC_ARG_FRAME	     /* Frame argument */
};

/** ISP memories */
enum atomisp_acc_memory {
	ATOMISP_ACC_MEMORY_PMEM = 0,
	ATOMISP_ACC_MEMORY_DMEM,
	ATOMISP_ACC_MEMORY_VMEM,
	ATOMISP_ACC_MEMORY_VAMEM0,
	ATOMISP_ACC_MEMORY_VAMEM1,
	ATOMISP_ACC_MEMORY_VAMEM2,
	ATOMISP_ACC_MEMORY_HMEM0,
	ATOMISP_ACC_MEMORY_HMEM1,
	ATOMISP_ACC_MEMORY_HMEM2,
	ATOMISP_ACC_MEMORY_HMEM3,
	ATOMISP_ACC_NR_MEMORY		/* Must be last */
};

struct atomisp_sp_arg {
	enum atomisp_acc_arg_type type;	/* Type  of SP argument */
	void                    *value;	/* Value of SP argument */
	unsigned int             size;	/* Size  of SP argument */
};

/* Acceleration API */

/* For CSS 1.0 only */
struct atomisp_acc_fw_arg {
	unsigned int fw_handle;
	unsigned int index;
	void __user *value;
	size_t size;
};

/*
 * Set arguments after first mapping with ATOMISP_IOC_ACC_S_MAPPED_ARG.
 * For CSS 1.5 only.
 */
struct atomisp_acc_s_mapped_arg {
	unsigned int fw_handle;
	__u32 offset;
	__u32 memory;			/* one of enum atomisp_acc_memory */
	size_t length;
	unsigned long css_ptr;
};

struct atomisp_acc_fw_abort {
	unsigned int fw_handle;
	/* Timeout in us */
	unsigned int timeout;
};

struct atomisp_acc_fw_load {
	unsigned int size;
	unsigned int fw_handle;
	void __user *data;
};

/*
 * Load firmware to specified pipeline.
 * For CSS 1.5 only.
 */
struct atomisp_acc_fw_load_to_pipe {
	__u32 flags;			/* Flags, see below for valid values */
	unsigned int fw_handle;		/* Handle, filled by kernel. */
	__u32 size;			/* Firmware binary size */
	void __user *data;		/* Pointer to firmware */
	__u32 type;			/* Binary type */
	__u32 reserved[3];		/* Set to zero */
};

#define ATOMISP_ACC_FW_LOAD_FL_PREVIEW		(1 << 0)
#define ATOMISP_ACC_FW_LOAD_FL_COPY		(1 << 1)
#define ATOMISP_ACC_FW_LOAD_FL_VIDEO		(1 << 2)
#define ATOMISP_ACC_FW_LOAD_FL_CAPTURE		(1 << 3)
#define ATOMISP_ACC_FW_LOAD_FL_ACC		(1 << 4)
#define ATOMISP_ACC_FW_LOAD_FL_ENABLE           (1 << 16)

#define ATOMISP_ACC_FW_LOAD_TYPE_NONE		0 /* Normal binary: don't use */
#define ATOMISP_ACC_FW_LOAD_TYPE_OUTPUT		1 /* Stage on output */
#define ATOMISP_ACC_FW_LOAD_TYPE_VIEWFINDER	2 /* Stage on viewfinder */
#define ATOMISP_ACC_FW_LOAD_TYPE_STANDALONE	3 /* Stand-alone acceleration */

struct atomisp_acc_map {
	__u32 flags;			/* Flags, see list below */
	__u32 length;			/* Length of data in bytes */
	void __user *user_ptr;		/* Pointer into user space */
	unsigned long css_ptr;		/* Pointer into CSS address space */
	__u32 reserved[4];		/* Set to zero */
};

struct atomisp_acc_state {
        __u32 flags;                    /* Flags, see list below */
#define ATOMISP_STATE_FLAG_ENABLE       ATOMISP_ACC_FW_LOAD_FL_ENABLE
        unsigned int fw_handle;
};

#define ATOMISP_MAP_FLAG_NOFLUSH	0x0001	/* Do not flush cache */
#define ATOMISP_MAP_FLAG_CACHED         0x0002  /* Enable cache */

/*
 * V4L2 private internal data interface.
 * -----------------------------------------------------------------------------
 * struct v4l2_private_int_data - request private data stored in video device
 * internal memory.
 * @size: sanity check to ensure userspace's buffer fits whole private data.
 *	  If not, kernel will make partial copy (or nothing if @size == 0).
 *	  @size is always corrected for the minimum necessary if IOCTL returns
 *	  no error.
 * @data: pointer to userspace buffer.
 */
struct v4l2_private_int_data {
	__u32 size;
	void __user *data;
	__u32 reserved[2];
};

/*Private IOCTLs for ISP */
#define ATOMISP_IOC_G_XNR \
	_IOR('v', BASE_VIDIOC_PRIVATE + 0, int)
#define ATOMISP_IOC_S_XNR \
	_IOW('v', BASE_VIDIOC_PRIVATE + 1, int)
#define ATOMISP_IOC_G_NR \
	_IOR('v', BASE_VIDIOC_PRIVATE + 2, struct atomisp_nr_config)
#define ATOMISP_IOC_S_NR \
	_IOW('v', BASE_VIDIOC_PRIVATE + 3, struct atomisp_nr_config)
#define ATOMISP_IOC_G_TNR \
	_IOR('v', BASE_VIDIOC_PRIVATE + 4, struct atomisp_tnr_config)
#define ATOMISP_IOC_S_TNR \
	_IOW('v', BASE_VIDIOC_PRIVATE + 5, struct atomisp_tnr_config)
#define ATOMISP_IOC_G_HISTOGRAM \
	_IOWR('v', BASE_VIDIOC_PRIVATE + 6, struct atomisp_histogram)
#define ATOMISP_IOC_S_HISTOGRAM \
	_IOW('v', BASE_VIDIOC_PRIVATE + 7, struct atomisp_histogram)
#define ATOMISP_IOC_G_BLACK_LEVEL_COMP \
	_IOR('v', BASE_VIDIOC_PRIVATE + 8, struct atomisp_ob_config)
#define ATOMISP_IOC_S_BLACK_LEVEL_COMP \
	_IOW('v', BASE_VIDIOC_PRIVATE + 9, struct atomisp_ob_config)
#define ATOMISP_IOC_G_EE \
	_IOR('v', BASE_VIDIOC_PRIVATE + 12, struct atomisp_ee_config)
#define ATOMISP_IOC_S_EE \
	_IOW('v', BASE_VIDIOC_PRIVATE + 13, struct atomisp_ee_config)
/* Digital Image Stabilization:
 * 1. get dis statistics: reads DIS statistics from ISP (every frame)
 * 2. set dis coefficients: set DIS filter coefficients (one time)
 * 3. set dis motion vecotr: set motion vector (result of DIS, every frame)
 */
#define ATOMISP_IOC_G_DIS_STAT \
	_IOWR('v', BASE_VIDIOC_PRIVATE + 14, struct atomisp_dis_statistics)
#define ATOMISP_IOC_S_DIS_COEFS \
	_IOW('v', BASE_VIDIOC_PRIVATE + 15, struct atomisp_dis_coefficients)
#define ATOMISP_IOC_S_DIS_VECTOR \
	_IOW('v', BASE_VIDIOC_PRIVATE + 16, struct atomisp_dis_vector)

#define ATOMISP_IOC_G_3A_STAT \
	_IOW('v', BASE_VIDIOC_PRIVATE + 17, struct atomisp_3a_statistics)
#define ATOMISP_IOC_G_ISP_PARM \
	_IOR('v', BASE_VIDIOC_PRIVATE + 18, struct atomisp_parm)
#define ATOMISP_IOC_S_ISP_PARM \
	_IOW('v', BASE_VIDIOC_PRIVATE + 19, struct atomisp_parm)
#define ATOMISP_IOC_G_ISP_GAMMA \
	_IOR('v', BASE_VIDIOC_PRIVATE + 20, struct atomisp_gamma_table)
#define ATOMISP_IOC_S_ISP_GAMMA \
	_IOW('v', BASE_VIDIOC_PRIVATE + 21, struct atomisp_gamma_table)
#define ATOMISP_IOC_G_ISP_GDC_TAB \
	_IOR('v', BASE_VIDIOC_PRIVATE + 22, struct atomisp_morph_table)
#define ATOMISP_IOC_S_ISP_GDC_TAB \
	_IOW('v', BASE_VIDIOC_PRIVATE + 23, struct atomisp_morph_table)
#define ATOMISP_IOC_ISP_MAKERNOTE \
	_IOWR('v', BASE_VIDIOC_PRIVATE + 24, struct atomisp_makernote_info)

/* macc parameter control*/
#define ATOMISP_IOC_G_ISP_MACC \
	_IOR('v', BASE_VIDIOC_PRIVATE + 25, struct atomisp_macc_config)
#define ATOMISP_IOC_S_ISP_MACC \
	_IOW('v', BASE_VIDIOC_PRIVATE + 26, struct atomisp_macc_config)

/* Defect pixel detection & Correction */
#define ATOMISP_IOC_G_ISP_BAD_PIXEL_DETECTION \
	_IOR('v', BASE_VIDIOC_PRIVATE + 27, struct atomisp_dp_config)
#define ATOMISP_IOC_S_ISP_BAD_PIXEL_DETECTION \
	_IOW('v', BASE_VIDIOC_PRIVATE + 28, struct atomisp_dp_config)

/* False Color Correction */
#define ATOMISP_IOC_G_ISP_FALSE_COLOR_CORRECTION \
	_IOR('v', BASE_VIDIOC_PRIVATE + 29, struct atomisp_de_config)
#define ATOMISP_IOC_S_ISP_FALSE_COLOR_CORRECTION \
	_IOW('v', BASE_VIDIOC_PRIVATE + 30, struct atomisp_de_config)

/* ctc parameter control */
#define ATOMISP_IOC_G_ISP_CTC \
	_IOR('v', BASE_VIDIOC_PRIVATE + 31, struct atomisp_ctc_table)
#define ATOMISP_IOC_S_ISP_CTC \
	_IOW('v', BASE_VIDIOC_PRIVATE + 32, struct atomisp_ctc_table)

/* white balance Correction */
#define ATOMISP_IOC_G_ISP_WHITE_BALANCE \
	_IOR('v', BASE_VIDIOC_PRIVATE + 33, struct atomisp_wb_config)
#define ATOMISP_IOC_S_ISP_WHITE_BALANCE \
	_IOW('v', BASE_VIDIOC_PRIVATE + 34, struct atomisp_wb_config)

/* fpn table loading */
#define ATOMISP_IOC_S_ISP_FPN_TABLE \
	_IOW('v', BASE_VIDIOC_PRIVATE + 35, struct v4l2_framebuffer)

/* overlay image loading */
#define ATOMISP_IOC_G_ISP_OVERLAY \
	_IOWR('v', BASE_VIDIOC_PRIVATE + 36, struct atomisp_overlay)
#define ATOMISP_IOC_S_ISP_OVERLAY \
	_IOW('v', BASE_VIDIOC_PRIVATE + 37, struct atomisp_overlay)

/* bcd driver bridge */
#define ATOMISP_IOC_CAMERA_BRIDGE \
	_IOWR('v', BASE_VIDIOC_PRIVATE + 38, struct atomisp_bc_video_package)

/* Sensor resolution specific info for AE */
#define ATOMISP_IOC_G_SENSOR_MODE_DATA \
	_IOR('v', BASE_VIDIOC_PRIVATE + 39, struct atomisp_sensor_mode_data)

#define ATOMISP_IOC_S_EXPOSURE \
	_IOW('v', BASE_VIDIOC_PRIVATE + 40, struct atomisp_exposure)

/* sensor calibration registers group */
#define ATOMISP_IOC_G_SENSOR_CALIBRATION_GROUP \
	_IOWR('v', BASE_VIDIOC_PRIVATE + 41, struct atomisp_calibration_group)

/* white balance Correction */
#define ATOMISP_IOC_G_3A_CONFIG \
	_IOR('v', BASE_VIDIOC_PRIVATE + 42, struct atomisp_3a_config)
#define ATOMISP_IOC_S_3A_CONFIG \
	_IOW('v', BASE_VIDIOC_PRIVATE + 43, struct atomisp_3a_config)

/* Accelerate ioctls */
#define ATOMISP_IOC_ACC_LOAD \
	_IOWR('v', BASE_VIDIOC_PRIVATE + 44, struct atomisp_acc_fw_load)

#define ATOMISP_IOC_ACC_UNLOAD \
	_IOWR('v', BASE_VIDIOC_PRIVATE + 45, unsigned int)

/* For CSS 1.0 only */
#define ATOMISP_IOC_ACC_S_ARG \
	_IOW('v', BASE_VIDIOC_PRIVATE + 46, struct atomisp_acc_fw_arg)

#define ATOMISP_IOC_ACC_START \
	_IOW('v', BASE_VIDIOC_PRIVATE + 47, unsigned int)

#define ATOMISP_IOC_ACC_WAIT \
	_IOW('v', BASE_VIDIOC_PRIVATE + 48, unsigned int)

#define ATOMISP_IOC_ACC_ABORT \
	_IOW('v', BASE_VIDIOC_PRIVATE + 49, struct atomisp_acc_fw_abort)

/* sensor OTP memory read */
#define ATOMISP_IOC_G_SENSOR_PRIV_INT_DATA \
	_IOWR('v', BASE_VIDIOC_PRIVATE + 50, struct v4l2_private_int_data)

/* LCS (shading) table write */
#define ATOMISP_IOC_S_ISP_SHD_TAB \
	_IOWR('v', BASE_VIDIOC_PRIVATE + 51, struct atomisp_shading_table)

/* Gamma Correction */
#define ATOMISP_IOC_G_ISP_GAMMA_CORRECTION \
	_IOR('v', BASE_VIDIOC_PRIVATE + 52, struct atomisp_gc_config)

#define ATOMISP_IOC_S_ISP_GAMMA_CORRECTION \
	_IOW('v', BASE_VIDIOC_PRIVATE + 53, struct atomisp_gc_config)

#define ATOMISP_IOC_ACC_DESTAB \
	_IOW('v', BASE_VIDIOC_PRIVATE + 54, struct atomisp_acc_fw_arg)

/*
 * Reserved ioctls. We have customer implementing it internally.
 * We can't use both numbers to not cause ABI conflict.
 * Anyway, those ioctls are hacks and not implemented by us:
 *
 * #define ATOMISP_IOC_G_SENSOR_REG \
 *	_IOW('v', BASE_VIDIOC_PRIVATE + 55, struct atomisp_sensor_regs)
 * #define ATOMISP_IOC_S_SENSOR_REG \
 *	_IOW('v', BASE_VIDIOC_PRIVATE + 56, struct atomisp_sensor_regs)
 */

/* motor internal memory read */
#define ATOMISP_IOC_G_MOTOR_PRIV_INT_DATA \
	_IOWR('v', BASE_VIDIOC_PRIVATE + 57, struct v4l2_private_int_data)

/*
 * Ioctls to map and unmap user buffers to CSS address space for acceleration.
 * User fills fields length and user_ptr and sets other fields to zero,
 * kernel may modify the flags and sets css_ptr.
 */
#define ATOMISP_IOC_ACC_MAP \
	_IOWR('v', BASE_VIDIOC_PRIVATE + 58, struct atomisp_acc_map)

/* User fills fields length, user_ptr, and css_ptr and zeroes other fields. */
#define ATOMISP_IOC_ACC_UNMAP \
	_IOW('v', BASE_VIDIOC_PRIVATE + 59, struct atomisp_acc_map)

#define ATOMISP_IOC_ACC_S_MAPPED_ARG \
	_IOW('v', BASE_VIDIOC_PRIVATE + 60, struct atomisp_acc_s_mapped_arg)

#define ATOMISP_IOC_S_PARAMETERS \
	_IOW('v', BASE_VIDIOC_PRIVATE + 61, struct atomisp_parameters)

#define ATOMISP_IOC_S_CONT_CAPTURE_CONFIG \
	_IOWR('v', BASE_VIDIOC_PRIVATE + 62, struct atomisp_cont_capture_conf)

#define ATOMISP_IOC_ACC_LOAD_TO_PIPE \
	_IOWR('v', BASE_VIDIOC_PRIVATE + 63, struct atomisp_acc_fw_load_to_pipe)

#define ATOMISP_IOC_G_METADATA \
        _IOWR('v', BASE_VIDIOC_PRIVATE + 64, struct atomisp_metadata)

#define ATOMISP_IOC_S_ACC_STATE\
	_IOWR('v', BASE_VIDIOC_PRIVATE + 65, struct atomisp_acc_state)

#define ATOMISP_IOC_G_ACC_STATE\
	_IOWR('v', BASE_VIDIOC_PRIVATE + 66, struct atomisp_acc_state)

#define ATOMISP_FW_LOAD \
       _IOWR('v', BASE_VIDIOC_PRIVATE + 102, int)


#define V4L2_CID_ATOMISP_BAD_PIXEL_DETECTION \
	(V4L2_CID_PRIVATE_BASE + 0)
#define V4L2_CID_ATOMISP_POSTPROCESS_GDC_CAC \
	(V4L2_CID_PRIVATE_BASE + 1)
#define V4L2_CID_ATOMISP_VIDEO_STABLIZATION \
	(V4L2_CID_PRIVATE_BASE + 2)
#define V4L2_CID_ATOMISP_FIXED_PATTERN_NR \
	(V4L2_CID_PRIVATE_BASE + 3)
#define V4L2_CID_ATOMISP_FALSE_COLOR_CORRECTION \
	(V4L2_CID_PRIVATE_BASE + 4)
#define V4L2_CID_ATOMISP_LOW_LIGHT \
	(V4L2_CID_PRIVATE_BASE + 5)

/* Camera class:
 * Exposure, Flash and privacy (indicator) light controls, to be upstreamed */
#define V4L2_CID_CAMERA_LASTP1             (V4L2_CID_CAMERA_CLASS_BASE + 1024)

#define V4L2_CID_FOCAL_ABSOLUTE            (V4L2_CID_CAMERA_LASTP1 + 0)
#define V4L2_CID_FNUMBER_ABSOLUTE          (V4L2_CID_CAMERA_LASTP1 + 1)
#define V4L2_CID_FNUMBER_RANGE             (V4L2_CID_CAMERA_LASTP1 + 2)

/* Flash related CIDs, see also:
 * http://linuxtv.org/downloads/v4l-dvb-apis/extended-controls.html\
 * #flash-controls */

/* Request a number of flash-exposed frames. The frame status can be
 * found in the reserved field in the v4l2_buffer struct. */
#define V4L2_CID_REQUEST_FLASH             (V4L2_CID_FLASH_CLASS_BASE + 13)
/* Query flash driver status. See enum atomisp_flash_status above. */
#define V4L2_CID_FLASH_STATUS              (V4L2_CID_FLASH_CLASS_BASE + 14)
/* Set the flash mode (see enum atomisp_flash_mode) */
#define V4L2_CID_FLASH_MODE                (V4L2_CID_FLASH_CLASS_BASE + 15)

/* VCM slew control */
#define V4L2_CID_VCM_SLEW                  (V4L2_CID_CAMERA_LASTP1 + 11)
/* VCM step time */
#define V4L2_CID_VCM_TIMEING               (V4L2_CID_CAMERA_LASTP1 + 12)
/* sensor test pattern */
//Redefiniton of v4l2-core function in kernel 3.8
//#define V4L2_CID_TEST_PATTERN              (V4L2_CID_CAMERA_LASTP1 + 13)

/* Query Focus Status */
#define V4L2_CID_FOCUS_STATUS              (V4L2_CID_CAMERA_LASTP1 + 14)

/* Query sensor's binning factor */
#define V4L2_CID_BIN_FACTOR_HORZ	   (V4L2_CID_CAMERA_LASTP1 + 15)
#define V4L2_CID_BIN_FACTOR_VERT	   (V4L2_CID_CAMERA_LASTP1 + 16)

/* number of frames to skip at stream start */
#define V4L2_CID_G_SKIP_FRAMES		   (V4L2_CID_CAMERA_LASTP1 + 17)

/* Query sensor's 2A status */
#define V4L2_CID_2A_STATUS                 (V4L2_CID_CAMERA_LASTP1 + 18)
#define V4L2_2A_STATUS_AE_READY            (1 << 0)
#define V4L2_2A_STATUS_AWB_READY           (1 << 1)

#define V4L2_CID_FMT_AUTO			(V4L2_CID_CAMERA_LASTP1 + 19)
#define V4L2_CID_RUN_MODE			(V4L2_CID_CAMERA_LASTP1 + 20)
#define V4L2_CID_ENABLE_VFPP			(V4L2_CID_CAMERA_LASTP1 + 21)

#define ATOMISP_RUN_MODE_VIDEO			1
#define ATOMISP_RUN_MODE_STILL_CAPTURE		2
#define ATOMISP_RUN_MODE_CONTINUOUS_CAPTURE	3
#define ATOMISP_RUN_MODE_PREVIEW		4

#define V4L2_BUF_FLAG_BUFFER_INVALID       0x0400
#define V4L2_BUF_FLAG_BUFFER_VALID         0x0800

#define V4L2_BUF_TYPE_VIDEO_CAPTURE_ION  (V4L2_BUF_TYPE_PRIVATE + 1024)

#define V4L2_EVENT_ATOMISP_3A_STATS_READY  (V4L2_EVENT_PRIVATE_START + 1)
#define V4L2_EVENT_ATOMISP_METADATA_READY  (V4L2_EVENT_PRIVATE_START + 2)
#define V4L2_EVENT_ATOMISP_ACC_COMPLETE  (V4L2_EVENT_PRIVATE_START + 3)

/* Nonstandard color effects for V4L2_CID_COLORFX */
enum {
	V4L2_COLORFX_SKIN_WHITEN_LOW = 1001,
	V4L2_COLORFX_SKIN_WHITEN_HIGH = 1002,
};

#endif /* _ATOM_ISP_H */
