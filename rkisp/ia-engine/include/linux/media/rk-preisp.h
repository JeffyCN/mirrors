// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Rockchip preisp driver
 * Copyright (C) 2018 Rockchip Electronics Co., Ltd.
 */

#ifndef _UAPI_RKPREISP_H
#define _UAPI_RKPREISP_H

#include <linux/types.h>

#define CIFISP_CMD_SET_HDRAE_EXP	\
	_IOW('V', BASE_VIDIOC_PRIVATE + 0, struct preisp_hdrae_exp_s)

#define CIFISP_CMD_SAVE_HDRAE_PARAM	\
	_IOW('V', BASE_VIDIOC_PRIVATE + 1, struct preisp_hdrae_para_s)

/**
 * struct preisp_hdrae_para_s - awb and lsc para for preisp
 *
 * @r_gain: awb r gain
 * @b_gain: awb b gain
 * @gr_gain: awb gr gain
 * @gb_gain: awb gb gain
 * @lsc_table: lsc data of gr
 */
struct preisp_hdrae_para_s {
	unsigned short r_gain;
	unsigned short b_gain;
	unsigned short gr_gain;
	unsigned short gb_gain;
	int lsc_table[17 * 17];
}  __attribute__ ((packed));

/**
 * struct preisp_hdrae_exp_s - hdrae exposure
 *
 */
struct preisp_hdrae_exp_s {
	unsigned int long_exp_reg;
	unsigned int long_gain_reg;
	unsigned int middle_exp_reg;
	unsigned int middle_gain_reg;
	unsigned int short_exp_reg;
	unsigned int short_gain_reg;
	unsigned int long_exp_val;
	unsigned int long_gain_val;
	unsigned int middle_exp_val;
	unsigned int middle_gain_val;
	unsigned int short_exp_val;
	unsigned int short_gain_val;
} __attribute__ ((packed));

/**
 * from rk1608 system.h, these structs will be used to
 * translate embeded data from isp driver.
 */

#define CIFISP_PREISP_HDRAE_HIST_BIN_NUM	(1 << 8)
#define CIFISP_PREISP_HDRAE_MAXFRAMES		3
#define CIFISP_PREISP_HDRAE_MAXGRIDITEMS	(15 * 15)
#define CIFISP_PREISP_HDRAE_1_FRAME_OFFSET	2
#define CIFISP_PREISP_HDRAE_2_FRAME_OFFSET	(369 + CIFISP_PREISP_HDRAE_1_FRAME_OFFSET)
#define CIFISP_PREISP_HDRAE_3_FRAME_OFFSET	(369 + CIFISP_PREISP_HDRAE_2_FRAME_OFFSET)
#define CIFISP_PREISP_HDRAE_LOGHIST_BIN_NUM 100 //zlj

struct cifisp_preisp_hdrae_hist_meas_res {
	unsigned int hist_bin[CIFISP_PREISP_HDRAE_HIST_BIN_NUM];
};

struct cifisp_preisp_hdrae_mean_meas_res {
	unsigned short y_meas[CIFISP_PREISP_HDRAE_MAXGRIDITEMS];
};

struct cifisp_preisp_hdrae_oneframe_result {
	struct cifisp_preisp_hdrae_hist_meas_res hist_meas;
	struct cifisp_preisp_hdrae_mean_meas_res mean_meas;
};
struct cifisp_preisp_hdrae_DRIndex_res{
	unsigned int fNormalIndex;
	unsigned int fLongIndex;
};

struct cifisp_preisp_hdrae_OE_meas_res{
	unsigned int OE_Pixel;
	unsigned int SumHistPixel;
	unsigned int SframeMaxLuma; //zlj add
};

#define cifisp_preisp_HDRAE_MSGID	0x00000001

struct cifisp_preisp_hdrae_result {
	unsigned int mesg_id;
	unsigned int mesg_size;
	struct cifisp_preisp_hdrae_oneframe_result oneframe[CIFISP_PREISP_HDRAE_MAXFRAMES];
	/*zlj still no add*/
	struct cifisp_preisp_hdrae_OE_meas_res OEMeasRes; //zlj add
	struct cifisp_preisp_hdrae_DRIndex_res DRIndexRes;//zlj add
	unsigned int reg_exp_time[3];
	unsigned int reg_exp_gain[3];
	unsigned int lgmean;
};

struct cifisp_preisp_dspmsg_head {
	unsigned int mesg_total_size;
	unsigned int frame_id;
	unsigned int mesg_count;
};

#define CIFISP_PREISP_GOC_CURVE_SIZE 34

typedef struct cifisp_preisp_hdr_ae_embeded_type {
	struct cifisp_preisp_dspmsg_head head;
	struct cifisp_preisp_hdrae_result result;
    unsigned short cifisp_preisp_goc_curve[CIFISP_PREISP_GOC_CURVE_SIZE];
} cifisp_preisp_hdr_ae_embeded_type_t;
#endif /* _UAPI_RKPREISP_H */
