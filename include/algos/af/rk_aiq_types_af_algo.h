#ifndef __RK_AIQ_TYPES_AF_ALGO_H__
#define __RK_AIQ_TYPES_AF_ALGO_H__

#define RKAIQ_RAWAF_WIN_NUM     2
#define RKAIQ_RAWAF_LINE_NUM        5
#define RKAIQ_RAWAF_GAMMA_NUM       17
#define RKAIQ_RAWAF_SUMDATA_ROW     15
#define RKAIQ_RAWAF_SUMDATA_COLUMN  15
#define RKAIQ_RAWAF_SUMDATA_NUM     225

typedef struct {
    unsigned long long roia_sharpness;
    unsigned int roia_luminance;
    unsigned int roib_sharpness;
    unsigned int roib_luminance;
    unsigned int global_sharpness[RKAIQ_RAWAF_SUMDATA_NUM];
    struct timeval focus_starttim;
    struct timeval focus_endtim;
    struct timeval zoom_starttim;
    struct timeval zoom_endtim;
    int64_t sof_tim;
} rk_aiq_af_algo_stat_t;

typedef struct {
    unsigned char contrast_af_en;
    unsigned char rawaf_sel;

    unsigned char window_num;
    unsigned short wina_h_offs;
    unsigned short wina_v_offs;
    unsigned short wina_h_size;
    unsigned short wina_v_size;

    unsigned short winb_h_offs;
    unsigned short winb_v_offs;
    unsigned short winb_h_size;
    unsigned short winb_v_size;

    unsigned char gamma_flt_en;
    unsigned short gamma_y[RKAIQ_RAWAF_GAMMA_NUM];

    unsigned char gaus_flt_en;
    unsigned char gaus_h0;
    unsigned char gaus_h1;
    unsigned char gaus_h2;

    unsigned char line_en[RKAIQ_RAWAF_LINE_NUM];
    unsigned char line_num[RKAIQ_RAWAF_LINE_NUM];

    unsigned short afm_thres;

    unsigned char lum_var_shift[RKAIQ_RAWAF_WIN_NUM];
    unsigned char afm_var_shift[RKAIQ_RAWAF_WIN_NUM];
} rk_aiq_af_algo_meas_t;

typedef struct {
    bool lens_pos_valid;
    bool zoom_pos_valid;
    unsigned int next_lens_pos;
    unsigned int next_zoom_pos;
} rk_aiq_af_algo_focus_pos_t;

#endif
