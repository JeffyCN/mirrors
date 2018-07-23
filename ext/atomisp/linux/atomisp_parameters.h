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

#ifndef _ATOM_ISP_PARAMETERS_H
#define _ATOM_ISP_PARAMETERS_H

#include <linux/types.h>
#include <linux/version.h>

#define ATOMISP_NUM_SC_COLORS	4
#define ATOMISP_SC_FLAG_QUERY	(1 << 0)
#define ATOMISP_GAMMA_TABLE_SIZE        1024
#define ATOMISP_MORPH_TABLE_NUM_PLANES  6
#define ATOMISP_NUM_MACC_AXES           16
#define ATOMISP_VAMEM_2_CTC_TABLE_SIZE_LOG2		8
#define ATOMISP_CTC_TABLE_SIZE          ((1U<<ATOMISP_VAMEM_2_CTC_TABLE_SIZE_LOG2) + 1)

/** Number of elements in the sRGB gamma table. */
#define ATOMISP_VAMEM_1_RGB_GAMMA_TABLE_SIZE_LOG2 8
#define ATOMISP_VAMEM_1_RGB_GAMMA_TABLE_SIZE (1U << ATOMISP_VAMEM_1_RGB_GAMMA_TABLE_SIZE_LOG2)
#define ATOMISP_VAMEM_2_RGB_GAMMA_TABLE_SIZE ((1U << ATOMISP_VAMEM_1_RGB_GAMMA_TABLE_SIZE_LOG2) + 1)
#define ATOMISP_VAMEM_2_RGB_GAMMA_TABLE_SIZE_LOG2 8

/** Number of elements in the xnr table. */
#define ATOMISP_VAMEM_1_XNR_TABLE_SIZE_LOG2		6
#define ATOMISP_VAMEM_1_XNR_TABLE_SIZE		(1U<<ATOMISP_VAMEM_1_XNR_TABLE_SIZE_LOG2)
#define ATOMISP_VAMEM_2_XNR_TABLE_SIZE_LOG2		6
#define ATOMISP_VAMEM_2_XNR_TABLE_SIZE		(1U<<ATOMISP_VAMEM_2_XNR_TABLE_SIZE_LOG2)


/* Configuration used by Bayer noise reduction and YCC noise reduction */
struct atomisp_nr_config {
	/* [gain] Strength of noise reduction for Bayer NR (Used by Bayer NR) */
	unsigned int bnr_gain;
	/* [gain] Strength of noise reduction for YCC NR (Used by YCC NR) */
	unsigned int ynr_gain;
	/* [intensity] Sensitivity of Edge (Used by Bayer NR) */
	unsigned int direction;
	/* [intensity] coring threshold for Cb (Used by YCC NR) */
	unsigned int threshold_cb;
	/* [intensity] coring threshold for Cr (Used by YCC NR) */
	unsigned int threshold_cr;
};

/* Temporal noise reduction configuration */
struct atomisp_tnr_config {
	unsigned int gain;	 /* [gain] Strength of NR */
	unsigned int threshold_y;/* [intensity] Motion sensitivity for Y */
	unsigned int threshold_uv;/* [intensity] Motion sensitivity for U/V */
};

/* Histogram. This contains num_elements values of type unsigned int.
 * The data pointer is a DDR pointer (virtual address).
 */
struct atomisp_histogram {
	unsigned int num_elements;
	void __user *data;
};

enum atomisp_ob_mode {
	atomisp_ob_mode_none,
	atomisp_ob_mode_fixed,
	atomisp_ob_mode_raster
};

/* Optical black level configuration */
struct atomisp_ob_config {
	/* Obtical black level mode (Fixed / Raster) */
	enum atomisp_ob_mode mode;
	/* [intensity] optical black level for GR (relevant for fixed mode) */
	unsigned int level_gr;
	/* [intensity] optical black level for R (relevant for fixed mode) */
	unsigned int level_r;
	/* [intensity] optical black level for B (relevant for fixed mode) */
	unsigned int level_b;
	/* [intensity] optical black level for GB (relevant for fixed mode) */
	unsigned int level_gb;
	/* [BQ] 0..63 start position of OB area (relevant for raster mode) */
	unsigned short start_position;
	/* [BQ] start..63 end position of OB area (relevant for raster mode) */
	unsigned short end_position;
};

/* Edge enhancement (sharpen) configuration */
struct atomisp_ee_config {
	/* [gain] The strength of sharpness. u5_11 */
	unsigned int gain;
	/* [intensity] The threshold that divides noises from edge. u8_8 */
	unsigned int threshold;
	/* [gain] The strength of sharpness in pell-mell area. u5_11 */
	unsigned int detail_gain;
};


struct atomisp_gc_config {
	__u16 gain_k1;
	__u16 gain_k2;
};

/* White Balance (Gain Adjust) */
struct atomisp_wb_config {
	unsigned int integer_bits;
	unsigned int gr;	/* unsigned <integer_bits>.<16-integer_bits> */
	unsigned int r;		/* unsigned <integer_bits>.<16-integer_bits> */
	unsigned int b;		/* unsigned <integer_bits>.<16-integer_bits> */
	unsigned int gb;	/* unsigned <integer_bits>.<16-integer_bits> */
};

/* Color Space Conversion settings */
struct atomisp_cc_config {
	unsigned int fraction_bits;
	int matrix[3 * 3];	/* RGB2YUV Color matrix, signed
				   <13-fraction_bits>.<fraction_bits> */
};

/* De pixel noise configuration */
struct atomisp_de_config {
	unsigned int pixelnoise;
	unsigned int c1_coring_threshold;
	unsigned int c2_coring_threshold;
};

/* Chroma enhancement */
struct atomisp_ce_config {
	unsigned int uv_level_min;
	unsigned int uv_level_max;
};

/* Defect pixel correction configuration */
struct atomisp_dp_config {
	/* [intensity] The threshold of defect Pixel Correction, representing
	 * the permissible difference of intensity between one pixel and its
	 * surrounding pixels. Smaller values result in more frequent pixel
	 * corrections. u0_16
	 */
	unsigned int threshold;
	/* [gain] The sensitivity of mis-correction. ISP will miss a lot of
	 * defects if the value is set too large. u8_8
	 */
	unsigned int gain;
};

/* XNR threshold */
struct atomisp_xnr_config {
	unsigned int threshold;
};

struct atomisp_anr_thres {
	__u16 data[13*64];
};
struct atomisp_vector {
	unsigned int x; /**< horizontal motion (in pixels) */
	unsigned int y; /**< vertical motion (in pixels) */
};

struct atomisp_dz_config {
	unsigned int dx;
	unsigned int dy;
};
struct atomisp_anr_config {
	unsigned int threshold; /**< Threshold */
	unsigned int thresholds[4*4*4];
	unsigned int factors[3];
};
struct atomisp_aa_config {
	__u16 strength;	/**< Strength of the filter. */
};

struct atomisp_ecd_config {
	__u16 zip_strength;	/*< Strength of zipper reduction.**/
	__u16 fc_strength;	/*< Strength of false color reduction. */
	__u16 fc_debias;	/*< Prevent color change. */

};

struct atomisp_ynr_config {
	__u16 edge_sense_gain_0;   /*< Sensitivity of edge in dark area.*/
	__u16 edge_sense_gain_1;   /*< Sensitivity of edge in bright area.*/
	__u16 corner_sense_gain_0; /*< Sensitivity of corner in dark area.*/
	__u16 corner_sense_gain_1; /*< Sensitivity of corner in bright area.*/
};

struct atomisp_fc_config {
		__u8  gain_exp;   	/**< Common exponent of gains.*/
		__u16 coring_pos_0; /**< Coring threshold for positive edge in dark area. */
		__u16 coring_pos_1; /**< Coring threshold for positive edge in bright area. */
		__u16 coring_neg_0; /**< Coring threshold for negative edge in dark area. */
		__u16 coring_neg_1; /**< Coring threshold for negative edge in bright area.*/
		__u16 gain_pos_0; 	/**< Gain for positive edge in dark area.*/
		__u16 gain_pos_1; 	/**< Gain for positive edge in bright area.*/
		__u16 gain_neg_0; 	/**< Gain for negative edge in dark area.*/
		__u16 gain_neg_1; 	/**< Gain for negative edge in bright area. */
		__u16 crop_pos_0;	/**< Limit for positive edge in dark area.*/
		__u16 crop_pos_1; 	/**< Limit for positive edge in bright area. */
		__u16  crop_neg_0;	/**< Limit for negative edge in dark area.*/
		__u16  crop_neg_1;  /**< Limit for negative edge in bright area. */
};
struct atomisp_formats_config{
	unsigned int video_full_range_flag; /**< selects the range of YUV output. */
};
struct atomisp_cnr_config {
	__u16 coring_u;			/**< Coring level of U. */
	__u16 coring_v;			/**< Coring level of V. */
	__u16 sense_gain_vy;	/**< Sensitivity of horizontal edge of Y. */
	__u16 sense_gain_vu;	/**< Sensitivity of horizontal edge of U.*/
	__u16 sense_gain_vv;	/**< Sensitivity of horizontal edge of V. */
	__u16 sense_gain_hy;	/**< Sensitivity of vertical edge of Y. */
	__u16 sense_gain_hu;	/**< Sensitivity of vertical edge of U.*/
	__u16 sense_gain_hv;	/**< Sensitivity of vertical edge of V. */
};

struct atomisp_dvs_6axis_config {
        uint32_t exp_id;
        uint32_t width_y;
        uint32_t height_y;
        uint32_t width_uv;
        uint32_t height_uv;
        uint32_t *xcoords_y;
        uint32_t *ycoords_y;
        uint32_t *xcoords_uv;
        uint32_t *ycoords_uv;
};


struct atomisp_ctc_config {
	__u16 y0;		/**< 1st kneepoint gain. */
	__u16 y1;	/**< 2nd kneepoint gain.) */
	__u16 y2;	/**< 3rd kneepoint gain.*/
	__u16 y3;	/**< 4th kneepoint gain. */
	__u16 y4;	/**< 5th kneepoint gain. */
	__u16 y5;	/**< 6th kneepoint gain. */
	__u16 ce_gain_exp;	/**< Common exponent of y-axis gain. */
	__u16 x1;	/**< 2nd kneepoint luma. */
	__u16 x2;	/**< 3rd kneepoint luma. */
	__u16 x3;	/**< 4th kneepoint luma. */
	__u16 x4;	/**< 5tn kneepoint luma. */
};

struct atomisp_gamma_table {
	unsigned short data[ATOMISP_GAMMA_TABLE_SIZE];
};

/* Morphing table for advanced ISP.
 * Each line of width elements takes up COORD_TABLE_EXT_WIDTH elements
 * in memory.
 */

struct atomisp_morph_table {
	unsigned int height;
	unsigned int width;	/* number of valid elements per line */
	unsigned short __user *coordinates_x[ATOMISP_MORPH_TABLE_NUM_PLANES];
	unsigned short __user *coordinates_y[ATOMISP_MORPH_TABLE_NUM_PLANES];
};



struct atomisp_shading_table {
	__u32 enable;
	/* native sensor resolution */
	__u32 sensor_width;
	__u32 sensor_height;
	/* number of data points per line per color (bayer quads) */
	__u32 width;
	/* number of lines of data points per color (bayer quads) */
	__u32 height;
	/* bits of fraction part for shading table values */
	__u32 fraction_bits;
	/* one table for each color (use sh_css_sc_color to index) */
	__u16 __user *data[ATOMISP_NUM_SC_COLORS];
};

/* parameter for MACC */

struct atomisp_macc_table {
	short data[4 * ATOMISP_NUM_MACC_AXES];
};

struct atomisp_macc_config {
	int color_effect;
	struct atomisp_macc_table table;
};

/* Parameter for ctc parameter control */
struct atomisp_ctc_table {
	unsigned short data[ATOMISP_CTC_TABLE_SIZE];
};

union atomisp_rgb_gamma_data{
	__u16 vamem_1[ATOMISP_VAMEM_1_RGB_GAMMA_TABLE_SIZE];
	/**< RGB Gamma table on vamem type1. This table is not used,
		because sRGB Gamma Correction is not implemented for ISP2300. */
	__u16 vamem_2[ATOMISP_VAMEM_2_RGB_GAMMA_TABLE_SIZE];
		/**< RGB Gamma table on vamem type2. */
};

struct atomisp_rgb_gamma_table {
	int vamem_type; /** VAMEM1 or VAMEM2**/
	union atomisp_rgb_gamma_data data;
};

union atomisp_xnr_data {
	__u16 vamem_1[ATOMISP_VAMEM_1_XNR_TABLE_SIZE];
	/**< Coefficients table on vamem type1. */
	__u16 vamem_2[ATOMISP_VAMEM_2_XNR_TABLE_SIZE];
	/**< Coefficients table on vamem type2.*/
};

struct atomisp_xnr_table {
	int vamem_type; /** VAMEM1 or VAMEM2**/
	union atomisp_xnr_data data;
};


#endif /* _ATOM_ISP_PARAMETERS_H */
