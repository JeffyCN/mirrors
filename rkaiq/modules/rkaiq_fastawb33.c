#ifdef ISP_HW_V33
#ifndef rk_kprintf
#include "awb_head2.h"
#define rk_kprintf LOGE_AWB
typedef struct isp_awbgain_cfg {
    int32_t r;
    int32_t gr;
    int32_t gb;
    int32_t b;
}isp_awbgain_cfg1;
#define rt_memset memset
#else
#include "rkaiq_parse_iq.h"
#define RK_AIQ_AWB_GRID_NUM_TOTAL   225
#define  RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT 8
#endif


#define UTL_FIX_MAX_U0012           1.0f //exactly this would be < 1-1/1024
#define UTL_FIX_MIN_U0012           0.0f
#define UTL_FIX_PRECISION_U0012     4096.0f
#define UTL_FIX_MASK_U0012          0x0FFF

#define UTL_FIX_MAX_S0310           4.0f //exactly this would be < 4-1/1024
#define UTL_FIX_MIN_S0310           -4.0f
#define UTL_FIX_PRECISION_S0310     1024.0f
#define UTL_FIX_MASK_S0310          0x1FFF
#define UTL_FIX_SIGN_S0310          0x1000

#define UTL_FIX_MAX_S0312           4.0f //exactly this would be < 4-1/4096
#define UTL_FIX_MIN_S0312           -4.0f
#define UTL_FIX_PRECISION_S0312     4096.0f
#define UTL_FIX_MASK_S0312          0x7FFF
#define UTL_FIX_SIGN_S0312          0x4000

#define AWB_UV_RANGE_SLOVE_FRACTION 10
#define AWB_UV_RANGE_SLOVE_SINTER   9
#define RK_AIQ_WP_INCLUDE_BIS       6


#define AWB_CHANNEL_R       0
#define AWB_CHANNEL_GR      1
#define AWB_CHANNEL_GB      2
#define AWB_CHANNEL_B       3
#define AWB_CHANNEL_MAX     4

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_U1200 \n
 *  \RETURNVALUE unsigned integer value in uint16_t container \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to unsigned integer values with \n
 *               12 bit integer and no fractional part to program the \n
 *               marvin registers. \n
 */
/*****************************************************************************/

// unsigned 12 bit integer, no fractional part
// 0xfff = 4095
// 0x001 = 1
// 0x000 = 0

uint16_t UtlFloatToFix_U0012(float fFloat)
{
    uint16_t ulFix = 0;

    if ((fFloat >  UTL_FIX_MAX_U0012) || (fFloat < UTL_FIX_MIN_U0012))
    {
        // TRACE(TR_ERROR,"%s : Out of range, input value is %f, while the range is [%f,%f)",__FUNCTION__,fFloat,UTL_FIX_MIN_U0012,UTL_FIX_MAX_U0012);
    }

    fFloat *= UTL_FIX_PRECISION_U0012;

    // round
    // no handling of negative values required
    ulFix = (uint16_t)(fFloat + 0.5f);


    // set upper (unused) bits to 0
    ulFix &= UTL_FIX_MASK_U0012;


    return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_S0210\n
 *  \RETURNVALUE signed fixed point value in short container, leading 1's \n
 *               in the upper bits, not used by the format, are suppressed \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to signed fixed point values (two's
 *               complement) with 2 bit integer and 10 bit fractional part to
 *               program the marvin registers. \n
 */
/*****************************************************************************/

// signed (two's complement) fixed point 2 bit integer / 10 bit fractional part

int16_t FloatToS16(float fFloat)
{
    int16_t ulFix;
    if ((fFloat > 0.0f))
    {
        ulFix = (int16_t)(fFloat + 0.5f);
    }
    else
    {
        ulFix  = (int16_t)(fFloat - 0.5f);
    }
    return (ulFix);
}

int16_t UtlFloatToFix_S0310(float fFloat)
{
    int16_t ulFix = 0;

    if ((fFloat > UTL_FIX_MAX_S0310) || (fFloat < UTL_FIX_MIN_S0310))
    {
        // TRACE(TR_ERROR,"%s : Out of range, input value is %f, while the range is [%f,%f)",__FUNCTION__,fFloat,UTL_FIX_MIN_S0310,UTL_FIX_MAX_S0310);
    }

    fFloat *= UTL_FIX_PRECISION_S0310;

    // round, two's complement if negative
#if 0
    if ((fFloat > 0.0f))
    {
        ulFix = (uint32_t)(fFloat + 0.5f);
    }
    else
    {
        fFloat = -fFloat;
        ulFix  = (uint32_t)(fFloat + 0.5f);
        ulFix  = ~ulFix;
        ulFix++;
    }
// set upper (unused) bits to 0
// ulFix &= UTL_FIX_MASK_S0312;
#else
    if ((fFloat > 0.0f))
    {
        ulFix = (int16_t)(fFloat + 0.5f);
    }
    else
    {
        ulFix  = (int16_t)(fFloat - 0.5f);
    }
#endif


    return ulFix;
}



int16_t UtlFloatToFix_S0312(float fFloat)
{
    int16_t ulFix = 0;

    if ((fFloat > UTL_FIX_MAX_S0312) || (fFloat < UTL_FIX_MIN_S0312))
    {
        // TRACE(TR_ERROR,"%s : Out of range, input value is %f, while the range is [%f,%f)",__FUNCTION__,fFloat,UTL_FIX_MIN_S0312,UTL_FIX_MAX_S0312);
    }

    fFloat *= UTL_FIX_PRECISION_S0312;

    // round, two's complement if negative
#if 0
    if ((fFloat > 0.0f))
    {
        ulFix = (uint32_t)(fFloat + 0.5f);
    }
    else
    {
        fFloat = -fFloat;
        ulFix  = (uint32_t)(fFloat + 0.5f);
        ulFix  = ~ulFix;
        ulFix++;
    }
// set upper (unused) bits to 0
// ulFix &= UTL_FIX_MASK_S0312;
#else
    if ((fFloat > 0.0f))
    {
        ulFix = (int16_t)(fFloat + 0.5f);
    }
    else
    {
        ulFix  = (int16_t)(fFloat - 0.5f);
    }
#endif


    return ulFix;
}

void WriteAwbReg(struct isp33_rawawb_meas_cfg *awb_cfg)
{
#if 0
    rk_kprintf("\n------------enter-----------\n\n");

    //rk_kprintf("sw_rawawb_en = 0x%0x (%d)\n",                                awb_cfg->en                            ,awb_cfg->en);
    rk_kprintf("sw_drc2awb_sel = 0x%0x (%d)\n",                              awb_cfg->drc2awb_sel, awb_cfg->drc2awb_sel);
    rk_kprintf("sw_bnr2awb_sel = 0x%0x (%d)\n",                              awb_cfg->bnr2awb_sel, awb_cfg->bnr2awb_sel);
    rk_kprintf("sw_rawawb_sel = 0x%0x (%d)\n",                               awb_cfg->rawawb_sel, awb_cfg->rawawb_sel);
    rk_kprintf("sw_rawawb_uv_en0 = 0x%0x (%d)\n",                            awb_cfg->uv_en0, awb_cfg->uv_en0);
    rk_kprintf("sw_rawawb_xy_en0 = 0x%0x (%d)\n",                            awb_cfg->xy_en0, awb_cfg->xy_en0);
    rk_kprintf("sw_rawawb_in_rshift_to_12bit_en = 0x%0x (%d)\n",             awb_cfg->in_rshift_to_12bit_en, awb_cfg->in_rshift_to_12bit_en);
    rk_kprintf("sw_rawawb_in_overexposure_check_en = 0x%0x (%d)\n",          awb_cfg->in_overexposure_check_en, awb_cfg->in_overexposure_check_en);
    rk_kprintf("sw_rawawb_wind_size = 0x%0x (%d)\n",                         awb_cfg->wind_size, awb_cfg->wind_size);
    rk_kprintf("sw_rawlsc_bypass_en = 0x%0x (%d)\n",                         awb_cfg->rawlsc_bypass_en, awb_cfg->rawlsc_bypass_en);
    rk_kprintf("sw_rawawb_light_num = 0x%0x (%d)\n",                         awb_cfg->light_num, awb_cfg->light_num);
    //rk_kprintf("sw_rawawb_2ddr_path_en = 0x%0x (%d)\n",                      awb_cfg->2ddr_path_en                  ,awb_cfg->2ddr_path_en);
    rk_kprintf("sw_rawawb_uv_en1 = 0x%0x (%d)\n",                            awb_cfg->uv_en1, awb_cfg->uv_en1);
    rk_kprintf("sw_rawawb_xy_en1 = 0x%0x (%d)\n",                            awb_cfg->xy_en1, awb_cfg->xy_en1);
    //rk_kprintf("sw_rawawb_2ddr_path_sel = 0x%0x (%d)\n",                     awb_cfg->2ddr_path_sel                 ,awb_cfg->2ddr_path_sel);
    rk_kprintf("sw_rawawbin_low12bit_val = 0x%0x (%d)\n",                    awb_cfg->low12bit_val, awb_cfg->low12bit_val);
    rk_kprintf("sw_rawawb_blk_measure_en = 0x%0x (%d)\n",                    awb_cfg->blk_measure_enable, awb_cfg->blk_measure_enable);
    rk_kprintf("sw_rawawb_blk_measure_mode = 0x%0x (%d)\n",                  awb_cfg->blk_measure_mode, awb_cfg->blk_measure_mode);
    rk_kprintf("sw_rawawb_blk_measure_xytype = 0x%0x (%d)\n",                awb_cfg->blk_measure_xytype, awb_cfg->blk_measure_xytype);
    rk_kprintf("sw_rawawb_blk_rtdw_measure_en = 0x%0x (%d)\n",               awb_cfg->blk_rtdw_measure_en, awb_cfg->blk_rtdw_measure_en);
    rk_kprintf("sw_rawawb_blk_measure_illu_idx = 0x%0x (%d)\n",              awb_cfg->blk_measure_illu_idx, awb_cfg->blk_measure_illu_idx);
    rk_kprintf("sw_rawawb_ds16x8_mode_en = 0x%0x (%d)\n",                    awb_cfg->ds16x8_mode_en, awb_cfg->ds16x8_mode_en);
    rk_kprintf("sw_rawawb_blk_with_luma_wei_en = 0x%0x (%d)\n",              awb_cfg->blk_with_luma_wei_en, awb_cfg->blk_with_luma_wei_en);
    rk_kprintf("sw_rawawb_in_overexposure_threshold = 0x%0x (%d)\n",         awb_cfg->in_overexposure_threshold, awb_cfg->in_overexposure_threshold);
    rk_kprintf("sw_rawawb_h_offs = 0x%0x (%d)\n",                            awb_cfg->h_offs, awb_cfg->h_offs);
    rk_kprintf("sw_rawawb_v_offs = 0x%0x (%d)\n",                            awb_cfg->v_offs, awb_cfg->v_offs);
    rk_kprintf("sw_rawawb_h_size = 0x%0x (%d)\n",                            awb_cfg->h_size, awb_cfg->h_size);
    rk_kprintf("sw_rawawb_v_size = 0x%0x (%d)\n",                            awb_cfg->v_size, awb_cfg->v_size);
    rk_kprintf("sw_rawawb_r_max = 0x%0x (%d)\n",                             awb_cfg->r_max, awb_cfg->r_max);
    rk_kprintf("sw_rawawb_g_max = 0x%0x (%d)\n",                             awb_cfg->g_max, awb_cfg->g_max);
    rk_kprintf("sw_rawawb_b_max = 0x%0x (%d)\n",                             awb_cfg->b_max, awb_cfg->b_max);
    rk_kprintf("sw_rawawb_y_max = 0x%0x (%d)\n",                             awb_cfg->y_max, awb_cfg->y_max);
    rk_kprintf("sw_rawawb_r_min = 0x%0x (%d)\n",                             awb_cfg->r_min, awb_cfg->r_min);
    rk_kprintf("sw_rawawb_g_min = 0x%0x (%d)\n",                             awb_cfg->g_min, awb_cfg->g_min);
    rk_kprintf("sw_rawawb_b_min = 0x%0x (%d)\n",                             awb_cfg->b_min, awb_cfg->b_min);
    rk_kprintf("sw_rawawb_y_min = 0x%0x (%d)\n",                             awb_cfg->y_min, awb_cfg->y_min);
    rk_kprintf("sw_rawawb_wp_luma_wei_en0 = 0x%0x (%d)\n",                   awb_cfg->wp_luma_wei_en0, awb_cfg->wp_luma_wei_en0);
    rk_kprintf("sw_rawawb_wp_luma_wei_en1 = 0x%0x (%d)\n",                   awb_cfg->wp_luma_wei_en1, awb_cfg->wp_luma_wei_en1);
    rk_kprintf("sw_rawawb_wp_blk_wei_en0 = 0x%0x (%d)\n",                    awb_cfg->wp_blk_wei_en0, awb_cfg->wp_blk_wei_en0);
    rk_kprintf("sw_rawawb_wp_blk_wei_en1 = 0x%0x (%d)\n",                    awb_cfg->wp_blk_wei_en1, awb_cfg->wp_blk_wei_en1);
    rk_kprintf("sw_rawawb_wp_hist_xytype = 0x%0x (%d)\n",                    awb_cfg->wp_hist_xytype, awb_cfg->wp_hist_xytype);
    rk_kprintf("sw_rawawb_wp_luma_weicurve_y0 = 0x%0x (%d)\n",               awb_cfg->wp_luma_weicurve_y0, awb_cfg->wp_luma_weicurve_y0);
    rk_kprintf("sw_rawawb_wp_luma_weicurve_y1 = 0x%0x (%d)\n",               awb_cfg->wp_luma_weicurve_y1, awb_cfg->wp_luma_weicurve_y1);
    rk_kprintf("sw_rawawb_wp_luma_weicurve_y2 = 0x%0x (%d)\n",               awb_cfg->wp_luma_weicurve_y2, awb_cfg->wp_luma_weicurve_y2);
    rk_kprintf("sw_rawawb_wp_luma_weicurve_y3 = 0x%0x (%d)\n",               awb_cfg->wp_luma_weicurve_y3, awb_cfg->wp_luma_weicurve_y3);
    rk_kprintf("sw_rawawb_wp_luma_weicurve_y4 = 0x%0x (%d)\n",               awb_cfg->wp_luma_weicurve_y4, awb_cfg->wp_luma_weicurve_y4);
    rk_kprintf("sw_rawawb_wp_luma_weicurve_y5 = 0x%0x (%d)\n",               awb_cfg->wp_luma_weicurve_y5, awb_cfg->wp_luma_weicurve_y5);
    rk_kprintf("sw_rawawb_wp_luma_weicurve_y6 = 0x%0x (%d)\n",               awb_cfg->wp_luma_weicurve_y6, awb_cfg->wp_luma_weicurve_y6);
    rk_kprintf("sw_rawawb_wp_luma_weicurve_y7 = 0x%0x (%d)\n",               awb_cfg->wp_luma_weicurve_y7, awb_cfg->wp_luma_weicurve_y7);
    rk_kprintf("sw_rawawb_wp_luma_weicurve_y8 = 0x%0x (%d)\n",               awb_cfg->wp_luma_weicurve_y8, awb_cfg->wp_luma_weicurve_y8);
    rk_kprintf("sw_rawawb_wp_luma_weicurve_w0 = 0x%0x (%d)\n",               awb_cfg->wp_luma_weicurve_w0, awb_cfg->wp_luma_weicurve_w0);
    rk_kprintf("sw_rawawb_wp_luma_weicurve_w1 = 0x%0x (%d)\n",               awb_cfg->wp_luma_weicurve_w1, awb_cfg->wp_luma_weicurve_w1);
    rk_kprintf("sw_rawawb_wp_luma_weicurve_w2 = 0x%0x (%d)\n",               awb_cfg->wp_luma_weicurve_w2, awb_cfg->wp_luma_weicurve_w2);
    rk_kprintf("sw_rawawb_wp_luma_weicurve_w3 = 0x%0x (%d)\n",               awb_cfg->wp_luma_weicurve_w3, awb_cfg->wp_luma_weicurve_w3);
    rk_kprintf("sw_rawawb_wp_luma_weicurve_w4 = 0x%0x (%d)\n",               awb_cfg->wp_luma_weicurve_w4, awb_cfg->wp_luma_weicurve_w4);
    rk_kprintf("sw_rawawb_wp_luma_weicurve_w5 = 0x%0x (%d)\n",               awb_cfg->wp_luma_weicurve_w5, awb_cfg->wp_luma_weicurve_w5);
    rk_kprintf("sw_rawawb_wp_luma_weicurve_w6 = 0x%0x (%d)\n",               awb_cfg->wp_luma_weicurve_w6, awb_cfg->wp_luma_weicurve_w6);
    rk_kprintf("sw_rawawb_wp_luma_weicurve_w7 = 0x%0x (%d)\n",               awb_cfg->wp_luma_weicurve_w7, awb_cfg->wp_luma_weicurve_w7);
    rk_kprintf("sw_rawawb_wp_luma_weicurve_w8 = 0x%0x (%d)\n",               awb_cfg->wp_luma_weicurve_w8, awb_cfg->wp_luma_weicurve_w8);
    rk_kprintf("sw_ccm_coeff0_r = 0x%0x (%d)\n",                             awb_cfg->ccm_coeff0_r, awb_cfg->ccm_coeff0_r);
    rk_kprintf("sw_ccm_coeff1_r = 0x%0x (%d)\n",                             awb_cfg->ccm_coeff1_r, awb_cfg->ccm_coeff1_r);
    rk_kprintf("sw_ccm_coeff2_r = 0x%0x (%d)\n",                             awb_cfg->ccm_coeff2_r, awb_cfg->ccm_coeff2_r);
    rk_kprintf("sw_ccm_coeff0_g = 0x%0x (%d)\n",                             awb_cfg->ccm_coeff0_g, awb_cfg->ccm_coeff0_g);
    rk_kprintf("sw_ccm_coeff1_g = 0x%0x (%d)\n",                             awb_cfg->ccm_coeff1_g, awb_cfg->ccm_coeff1_g);
    rk_kprintf("sw_ccm_coeff2_g = 0x%0x (%d)\n",                             awb_cfg->ccm_coeff2_g, awb_cfg->ccm_coeff2_g);
    rk_kprintf("sw_ccm_coeff0_b = 0x%0x (%d)\n",                             awb_cfg->ccm_coeff0_b, awb_cfg->ccm_coeff0_b);
    rk_kprintf("sw_ccm_coeff1_b = 0x%0x (%d)\n",                             awb_cfg->ccm_coeff1_b, awb_cfg->ccm_coeff1_b);
    rk_kprintf("sw_ccm_coeff2_b = 0x%0x (%d)\n",                             awb_cfg->ccm_coeff2_b, awb_cfg->ccm_coeff2_b);

    rk_kprintf("sw_rawawb_pre_wbgain_inv_r = 0x%0x (%d)\n",                  awb_cfg->pre_wbgain_inv_r, awb_cfg->pre_wbgain_inv_r);
    rk_kprintf("sw_rawawb_pre_wbgain_inv_g = 0x%0x (%d)\n",                  awb_cfg->pre_wbgain_inv_g, awb_cfg->pre_wbgain_inv_g);
    rk_kprintf("sw_rawawb_pre_wbgain_inv_b = 0x%0x (%d)\n",                  awb_cfg->pre_wbgain_inv_b, awb_cfg->pre_wbgain_inv_b);
    rk_kprintf("sw_rawawb_vertex0_u_0 = 0x%0x (%d)\n",                       awb_cfg->vertex0_u_0, awb_cfg->vertex0_u_0);
    rk_kprintf("sw_rawawb_vertex0_v_0 = 0x%0x (%d)\n",                       awb_cfg->vertex0_v_0, awb_cfg->vertex0_v_0);
    rk_kprintf("sw_rawawb_vertex1_u_0 = 0x%0x (%d)\n",                       awb_cfg->vertex1_u_0, awb_cfg->vertex1_u_0);
    rk_kprintf("sw_rawawb_vertex1_v_0 = 0x%0x (%d)\n",                       awb_cfg->vertex1_v_0, awb_cfg->vertex1_v_0);
    rk_kprintf("sw_rawawb_vertex2_u_0 = 0x%0x (%d)\n",                       awb_cfg->vertex2_u_0, awb_cfg->vertex2_u_0);
    rk_kprintf("sw_rawawb_vertex2_v_0 = 0x%0x (%d)\n",                       awb_cfg->vertex2_v_0, awb_cfg->vertex2_v_0);
    rk_kprintf("sw_rawawb_vertex3_u_0 = 0x%0x (%d)\n",                       awb_cfg->vertex3_u_0, awb_cfg->vertex3_u_0);
    rk_kprintf("sw_rawawb_vertex3_v_0 = 0x%0x (%d)\n",                       awb_cfg->vertex3_v_0, awb_cfg->vertex3_v_0);
    rk_kprintf("sw_rawawb_islope01_0 = 0x%0x (%d)\n",                        awb_cfg->islope01_0, awb_cfg->islope01_0);
    rk_kprintf("sw_rawawb_islope12_0 = 0x%0x (%d)\n",                        awb_cfg->islope12_0, awb_cfg->islope12_0);
    rk_kprintf("sw_rawawb_islope23_0 = 0x%0x (%d)\n",                        awb_cfg->islope23_0, awb_cfg->islope23_0);
    rk_kprintf("sw_rawawb_islope30_0 = 0x%0x (%d)\n",                        awb_cfg->islope30_0, awb_cfg->islope30_0);
    rk_kprintf("sw_rawawb_vertex0_u_1 = 0x%0x (%d)\n",                       awb_cfg->vertex0_u_1, awb_cfg->vertex0_u_1);
    rk_kprintf("sw_rawawb_vertex0_v_1 = 0x%0x (%d)\n",                       awb_cfg->vertex0_v_1, awb_cfg->vertex0_v_1);
    rk_kprintf("sw_rawawb_vertex1_u_1 = 0x%0x (%d)\n",                       awb_cfg->vertex1_u_1, awb_cfg->vertex1_u_1);
    rk_kprintf("sw_rawawb_vertex1_v_1 = 0x%0x (%d)\n",                       awb_cfg->vertex1_v_1, awb_cfg->vertex1_v_1);
    rk_kprintf("sw_rawawb_vertex2_u_1 = 0x%0x (%d)\n",                       awb_cfg->vertex2_u_1, awb_cfg->vertex2_u_1);
    rk_kprintf("sw_rawawb_vertex2_v_1 = 0x%0x (%d)\n",                       awb_cfg->vertex2_v_1, awb_cfg->vertex2_v_1);
    rk_kprintf("sw_rawawb_vertex3_u_1 = 0x%0x (%d)\n",                       awb_cfg->vertex3_u_1, awb_cfg->vertex3_u_1);
    rk_kprintf("sw_rawawb_vertex3_v_1 = 0x%0x (%d)\n",                       awb_cfg->vertex3_v_1, awb_cfg->vertex3_v_1);
    rk_kprintf("sw_rawawb_islope01_1 = 0x%0x (%d)\n",                        awb_cfg->islope01_1, awb_cfg->islope01_1);
    rk_kprintf("sw_rawawb_islope12_1 = 0x%0x (%d)\n",                        awb_cfg->islope12_1, awb_cfg->islope12_1);
    rk_kprintf("sw_rawawb_islope23_1 = 0x%0x (%d)\n",                        awb_cfg->islope23_1, awb_cfg->islope23_1);
    rk_kprintf("sw_rawawb_islope30_1 = 0x%0x (%d)\n",                        awb_cfg->islope30_1, awb_cfg->islope30_1);
    rk_kprintf("sw_rawawb_vertex0_u_2 = 0x%0x (%d)\n",                       awb_cfg->vertex0_u_2, awb_cfg->vertex0_u_2);
    rk_kprintf("sw_rawawb_vertex0_v_2 = 0x%0x (%d)\n",                       awb_cfg->vertex0_v_2, awb_cfg->vertex0_v_2);
    rk_kprintf("sw_rawawb_vertex1_u_2 = 0x%0x (%d)\n",                       awb_cfg->vertex1_u_2, awb_cfg->vertex1_u_2);
    rk_kprintf("sw_rawawb_vertex1_v_2 = 0x%0x (%d)\n",                       awb_cfg->vertex1_v_2, awb_cfg->vertex1_v_2);
    rk_kprintf("sw_rawawb_vertex2_u_2 = 0x%0x (%d)\n",                       awb_cfg->vertex2_u_2, awb_cfg->vertex2_u_2);
    rk_kprintf("sw_rawawb_vertex2_v_2 = 0x%0x (%d)\n",                       awb_cfg->vertex2_v_2, awb_cfg->vertex2_v_2);
    rk_kprintf("sw_rawawb_vertex3_u_2 = 0x%0x (%d)\n",                       awb_cfg->vertex3_u_2, awb_cfg->vertex3_u_2);
    rk_kprintf("sw_rawawb_vertex3_v_2 = 0x%0x (%d)\n",                       awb_cfg->vertex3_v_2, awb_cfg->vertex3_v_2);
    rk_kprintf("sw_rawawb_islope01_2 = 0x%0x (%d)\n",                        awb_cfg->islope01_2, awb_cfg->islope01_2);
    rk_kprintf("sw_rawawb_islope12_2 = 0x%0x (%d)\n",                        awb_cfg->islope12_2, awb_cfg->islope12_2);
    rk_kprintf("sw_rawawb_islope23_2 = 0x%0x (%d)\n",                        awb_cfg->islope23_2, awb_cfg->islope23_2);
    rk_kprintf("sw_rawawb_islope30_2 = 0x%0x (%d)\n",                        awb_cfg->islope30_2, awb_cfg->islope30_2);
    rk_kprintf("sw_rawawb_vertex0_u_3 = 0x%0x (%d)\n",                       awb_cfg->vertex0_u_3, awb_cfg->vertex0_u_3);
    rk_kprintf("sw_rawawb_vertex0_v_3 = 0x%0x (%d)\n",                       awb_cfg->vertex0_v_3, awb_cfg->vertex0_v_3);
    rk_kprintf("sw_rawawb_vertex1_u_3 = 0x%0x (%d)\n",                       awb_cfg->vertex1_u_3, awb_cfg->vertex1_u_3);
    rk_kprintf("sw_rawawb_vertex1_v_3 = 0x%0x (%d)\n",                       awb_cfg->vertex1_v_3, awb_cfg->vertex1_v_3);
    rk_kprintf("sw_rawawb_vertex2_u_3 = 0x%0x (%d)\n",                       awb_cfg->vertex2_u_3, awb_cfg->vertex2_u_3);
    rk_kprintf("sw_rawawb_vertex2_v_3 = 0x%0x (%d)\n",                       awb_cfg->vertex2_v_3, awb_cfg->vertex2_v_3);
    rk_kprintf("sw_rawawb_vertex3_u_3 = 0x%0x (%d)\n",                       awb_cfg->vertex3_u_3, awb_cfg->vertex3_u_3);
    rk_kprintf("sw_rawawb_vertex3_v_3 = 0x%0x (%d)\n",                       awb_cfg->vertex3_v_3, awb_cfg->vertex3_v_3);
    rk_kprintf("sw_rawawb_islope01_3 = 0x%0x (%d)\n",                        awb_cfg->islope01_3, awb_cfg->islope01_3);
    rk_kprintf("sw_rawawb_islope12_3 = 0x%0x (%d)\n",                        awb_cfg->islope12_3, awb_cfg->islope12_3);
    rk_kprintf("sw_rawawb_islope23_3 = 0x%0x (%d)\n",                        awb_cfg->islope23_3, awb_cfg->islope23_3);
    rk_kprintf("sw_rawawb_islope30_3 = 0x%0x (%d)\n",                        awb_cfg->islope30_3, awb_cfg->islope30_3);
    rk_kprintf("sw_rawawb_wt0 = 0x%0x (%d)\n",                               awb_cfg->wt0, awb_cfg->wt0);
    rk_kprintf("sw_rawawb_wt1 = 0x%0x (%d)\n",                               awb_cfg->wt1, awb_cfg->wt1);
    rk_kprintf("sw_rawawb_wt2 = 0x%0x (%d)\n",                               awb_cfg->wt2, awb_cfg->wt2);
    rk_kprintf("sw_rawawb_mat0_x = 0x%0x (%d)\n",                            awb_cfg->mat0_x, awb_cfg->mat0_x);
    rk_kprintf("sw_rawawb_mat0_y = 0x%0x (%d)\n",                            awb_cfg->mat0_y, awb_cfg->mat0_y);
    rk_kprintf("sw_rawawb_mat1_x = 0x%0x (%d)\n",                            awb_cfg->mat1_x, awb_cfg->mat1_x);
    rk_kprintf("sw_rawawb_mat1_y = 0x%0x (%d)\n",                            awb_cfg->mat1_y, awb_cfg->mat1_y);
    rk_kprintf("sw_rawawb_mat2_x = 0x%0x (%d)\n",                            awb_cfg->mat2_x, awb_cfg->mat2_x);
    rk_kprintf("sw_rawawb_mat2_y = 0x%0x (%d)\n",                            awb_cfg->mat2_y, awb_cfg->mat2_y);
    rk_kprintf("sw_rawawb_nor_x0_0 = 0x%0x (%d)\n",                          awb_cfg->nor_x0_0, awb_cfg->nor_x0_0);
    rk_kprintf("sw_rawawb_nor_x1_0 = 0x%0x (%d)\n",                          awb_cfg->nor_x1_0, awb_cfg->nor_x1_0);
    rk_kprintf("sw_rawawb_nor_y0_0 = 0x%0x (%d)\n",                          awb_cfg->nor_y0_0, awb_cfg->nor_y0_0);
    rk_kprintf("sw_rawawb_nor_y1_0 = 0x%0x (%d)\n",                          awb_cfg->nor_y1_0, awb_cfg->nor_y1_0);
    rk_kprintf("sw_rawawb_big_x0_0 = 0x%0x (%d)\n",                          awb_cfg->big_x0_0, awb_cfg->big_x0_0);
    rk_kprintf("sw_rawawb_big_x1_0 = 0x%0x (%d)\n",                          awb_cfg->big_x1_0, awb_cfg->big_x1_0);
    rk_kprintf("sw_rawawb_big_y0_0 = 0x%0x (%d)\n",                          awb_cfg->big_y0_0, awb_cfg->big_y0_0);
    rk_kprintf("sw_rawawb_big_y1_0 = 0x%0x (%d)\n",                          awb_cfg->big_y1_0, awb_cfg->big_y1_0);
    rk_kprintf("sw_rawawb_nor_x0_1 = 0x%0x (%d)\n",                          awb_cfg->nor_x0_1, awb_cfg->nor_x0_1);
    rk_kprintf("sw_rawawb_nor_x1_1 = 0x%0x (%d)\n",                          awb_cfg->nor_x1_1, awb_cfg->nor_x1_1);
    rk_kprintf("sw_rawawb_nor_y0_1 = 0x%0x (%d)\n",                          awb_cfg->nor_y0_1, awb_cfg->nor_y0_1);
    rk_kprintf("sw_rawawb_nor_y1_1 = 0x%0x (%d)\n",                          awb_cfg->nor_y1_1, awb_cfg->nor_y1_1);
    rk_kprintf("sw_rawawb_big_x0_1 = 0x%0x (%d)\n",                          awb_cfg->big_x0_1, awb_cfg->big_x0_1);
    rk_kprintf("sw_rawawb_big_x1_1 = 0x%0x (%d)\n",                          awb_cfg->big_x1_1, awb_cfg->big_x1_1);
    rk_kprintf("sw_rawawb_big_y0_1 = 0x%0x (%d)\n",                          awb_cfg->big_y0_1, awb_cfg->big_y0_1);
    rk_kprintf("sw_rawawb_big_y1_1 = 0x%0x (%d)\n",                          awb_cfg->big_y1_1, awb_cfg->big_y1_1);
    rk_kprintf("sw_rawawb_nor_x0_2 = 0x%0x (%d)\n",                          awb_cfg->nor_x0_2, awb_cfg->nor_x0_2);
    rk_kprintf("sw_rawawb_nor_x1_2 = 0x%0x (%d)\n",                          awb_cfg->nor_x1_2, awb_cfg->nor_x1_2);
    rk_kprintf("sw_rawawb_nor_y0_2 = 0x%0x (%d)\n",                          awb_cfg->nor_y0_2, awb_cfg->nor_y0_2);
    rk_kprintf("sw_rawawb_nor_y1_2 = 0x%0x (%d)\n",                          awb_cfg->nor_y1_2, awb_cfg->nor_y1_2);
    rk_kprintf("sw_rawawb_big_x0_2 = 0x%0x (%d)\n",                          awb_cfg->big_x0_2, awb_cfg->big_x0_2);
    rk_kprintf("sw_rawawb_big_x1_2 = 0x%0x (%d)\n",                          awb_cfg->big_x1_2, awb_cfg->big_x1_2);
    rk_kprintf("sw_rawawb_big_y0_2 = 0x%0x (%d)\n",                          awb_cfg->big_y0_2, awb_cfg->big_y0_2);
    rk_kprintf("sw_rawawb_big_y1_2 = 0x%0x (%d)\n",                          awb_cfg->big_y1_2, awb_cfg->big_y1_2);
    rk_kprintf("sw_rawawb_nor_x0_3 = 0x%0x (%d)\n",                          awb_cfg->nor_x0_3, awb_cfg->nor_x0_3);
    rk_kprintf("sw_rawawb_nor_x1_3 = 0x%0x (%d)\n",                          awb_cfg->nor_x1_3, awb_cfg->nor_x1_3);
    rk_kprintf("sw_rawawb_nor_y0_3 = 0x%0x (%d)\n",                          awb_cfg->nor_y0_3, awb_cfg->nor_y0_3);
    rk_kprintf("sw_rawawb_nor_y1_3 = 0x%0x (%d)\n",                          awb_cfg->nor_y1_3, awb_cfg->nor_y1_3);
    rk_kprintf("sw_rawawb_big_x0_3 = 0x%0x (%d)\n",                          awb_cfg->big_x0_3, awb_cfg->big_x0_3);
    rk_kprintf("sw_rawawb_big_x1_3 = 0x%0x (%d)\n",                          awb_cfg->big_x1_3, awb_cfg->big_x1_3);
    rk_kprintf("sw_rawawb_big_y0_3 = 0x%0x (%d)\n",                          awb_cfg->big_y0_3, awb_cfg->big_y0_3);
    rk_kprintf("sw_rawawb_big_y1_3 = 0x%0x (%d)\n",                          awb_cfg->big_y1_3, awb_cfg->big_y1_3);
    rk_kprintf("sw_rawawb_exc_wp_region0_excen = 0x%0x (%d)\n",              awb_cfg->exc_wp_region0_excen, awb_cfg->exc_wp_region0_excen);
    rk_kprintf("sw_rawawb_exc_wp_region0_measen = 0x%0x (%d)\n",             awb_cfg->exc_wp_region0_measen, awb_cfg->exc_wp_region0_measen);
    rk_kprintf("sw_rawawb_exc_wp_region0_domain = 0x%0x (%d)\n",             awb_cfg->exc_wp_region0_domain, awb_cfg->exc_wp_region0_domain);
    rk_kprintf("sw_rawawb_exc_wp_region0_weight = 0x%0x (%d)\n",             awb_cfg->exc_wp_region0_weight, awb_cfg->exc_wp_region0_weight);
    rk_kprintf("sw_rawawb_exc_wp_region1_excen = 0x%0x (%d)\n",              awb_cfg->exc_wp_region1_excen, awb_cfg->exc_wp_region1_excen);
    rk_kprintf("sw_rawawb_exc_wp_region1_measen = 0x%0x (%d)\n",             awb_cfg->exc_wp_region1_measen, awb_cfg->exc_wp_region1_measen);
    rk_kprintf("sw_rawawb_exc_wp_region1_domain = 0x%0x (%d)\n",             awb_cfg->exc_wp_region1_domain, awb_cfg->exc_wp_region1_domain);
    rk_kprintf("sw_rawawb_exc_wp_region1_weight = 0x%0x (%d)\n",             awb_cfg->exc_wp_region1_weight, awb_cfg->exc_wp_region1_weight);
    rk_kprintf("sw_rawawb_exc_wp_region2_excen = 0x%0x (%d)\n",              awb_cfg->exc_wp_region2_excen, awb_cfg->exc_wp_region2_excen);
    rk_kprintf("sw_rawawb_exc_wp_region2_measen = 0x%0x (%d)\n",             awb_cfg->exc_wp_region2_measen, awb_cfg->exc_wp_region2_measen);
    rk_kprintf("sw_rawawb_exc_wp_region2_domain = 0x%0x (%d)\n",             awb_cfg->exc_wp_region2_domain, awb_cfg->exc_wp_region2_domain);
    rk_kprintf("sw_rawawb_exc_wp_region2_weight = 0x%0x (%d)\n",             awb_cfg->exc_wp_region3_weight, awb_cfg->exc_wp_region2_weight);
    rk_kprintf("sw_rawawb_exc_wp_region3_excen = 0x%0x (%d)\n",              awb_cfg->exc_wp_region3_excen, awb_cfg->exc_wp_region3_excen);
    rk_kprintf("sw_rawawb_exc_wp_region3_measen = 0x%0x (%d)\n",             awb_cfg->exc_wp_region3_measen, awb_cfg->exc_wp_region3_measen);
    rk_kprintf("sw_rawawb_exc_wp_region3_domain = 0x%0x (%d)\n",             awb_cfg->exc_wp_region3_domain, awb_cfg->exc_wp_region3_domain);
    rk_kprintf("sw_rawawb_exc_wp_region3_weight = 0x%0x (%d)\n",             awb_cfg->exc_wp_region3_weight, awb_cfg->exc_wp_region3_weight);
    rk_kprintf("sw_rawawb_exc_wp_region4_excen = 0x%0x (%d)\n",              awb_cfg->exc_wp_region4_excen, awb_cfg->exc_wp_region4_excen);
    rk_kprintf("sw_rawawb_exc_wp_region4_domain = 0x%0x (%d)\n",             awb_cfg->exc_wp_region4_domain, awb_cfg->exc_wp_region4_domain);
    rk_kprintf("sw_rawawb_exc_wp_region4_weight = 0x%0x (%d)\n",             awb_cfg->exc_wp_region4_weight, awb_cfg->exc_wp_region4_weight);
    rk_kprintf("sw_rawawb_exc_wp_region5_excen = 0x%0x (%d)\n",              awb_cfg->exc_wp_region5_excen, awb_cfg->exc_wp_region5_excen);
    rk_kprintf("sw_rawawb_exc_wp_region5_domain = 0x%0x (%d)\n",             awb_cfg->exc_wp_region5_domain, awb_cfg->exc_wp_region5_domain);
    rk_kprintf("sw_rawawb_exc_wp_region5_weight = 0x%0x (%d)\n",             awb_cfg->exc_wp_region5_weight, awb_cfg->exc_wp_region5_weight);
    rk_kprintf("sw_rawawb_exc_wp_region6_excen = 0x%0x (%d)\n",              awb_cfg->exc_wp_region6_excen, awb_cfg->exc_wp_region6_excen);
    rk_kprintf("sw_rawawb_exc_wp_region6_domain = 0x%0x (%d)\n",             awb_cfg->exc_wp_region6_domain, awb_cfg->exc_wp_region6_domain);
    rk_kprintf("sw_rawawb_exc_wp_region6_weight = 0x%0x (%d)\n",             awb_cfg->exc_wp_region6_weight, awb_cfg->exc_wp_region6_weight);
    rk_kprintf("sw_rawawb_multiwindow_en = 0x%0x (%d)\n",                    awb_cfg->multiwindow_en, awb_cfg->multiwindow_en);
    rk_kprintf("sw_rawawb_multiwindow0_h_offs = 0x%0x (%d)\n",               awb_cfg->multiwindow0_h_offs, awb_cfg->multiwindow0_h_offs);
    rk_kprintf("sw_rawawb_multiwindow0_v_offs = 0x%0x (%d)\n",               awb_cfg->multiwindow0_v_offs, awb_cfg->multiwindow0_v_offs);
    rk_kprintf("sw_rawawb_multiwindow0_h_size = 0x%0x (%d)\n",               awb_cfg->multiwindow0_h_size, awb_cfg->multiwindow0_h_size);
    rk_kprintf("sw_rawawb_multiwindow1_v_size = 0x%0x (%d)\n",               awb_cfg->multiwindow1_v_size, awb_cfg->multiwindow1_v_size);
    rk_kprintf("sw_rawawb_multiwindow1_h_offs = 0x%0x (%d)\n",               awb_cfg->multiwindow1_h_offs, awb_cfg->multiwindow1_h_offs);
    rk_kprintf("sw_rawawb_multiwindow1_v_offs = 0x%0x (%d)\n",               awb_cfg->multiwindow1_v_offs, awb_cfg->multiwindow1_v_offs);
    rk_kprintf("sw_rawawb_multiwindow1_h_size = 0x%0x (%d)\n",               awb_cfg->multiwindow1_h_size, awb_cfg->multiwindow1_h_size);
    rk_kprintf("sw_rawawb_multiwindow1_v_size = 0x%0x (%d)\n",               awb_cfg->multiwindow1_v_size, awb_cfg->multiwindow1_v_size);
    rk_kprintf("sw_rawawb_multiwindow2_h_offs = 0x%0x (%d)\n",               awb_cfg->multiwindow2_h_offs, awb_cfg->multiwindow2_h_offs);
    rk_kprintf("sw_rawawb_multiwindow2_v_offs = 0x%0x (%d)\n",               awb_cfg->multiwindow2_v_offs, awb_cfg->multiwindow2_v_offs);
    rk_kprintf("sw_rawawb_multiwindow2_h_size = 0x%0x (%d)\n",               awb_cfg->multiwindow2_h_size, awb_cfg->multiwindow2_h_size);
    rk_kprintf("sw_rawawb_multiwindow2_v_size = 0x%0x (%d)\n",               awb_cfg->multiwindow2_v_size, awb_cfg->multiwindow2_v_size);
    rk_kprintf("sw_rawawb_multiwindow3_h_offs = 0x%0x (%d)\n",               awb_cfg->multiwindow3_h_offs, awb_cfg->multiwindow3_h_offs);
    rk_kprintf("sw_rawawb_multiwindow3_v_offs = 0x%0x (%d)\n",               awb_cfg->multiwindow3_v_offs, awb_cfg->multiwindow3_v_offs);
    rk_kprintf("sw_rawawb_multiwindow3_h_size = 0x%0x (%d)\n",               awb_cfg->multiwindow3_h_size, awb_cfg->multiwindow3_h_size);
    rk_kprintf("sw_rawawb_multiwindow3_v_size = 0x%0x (%d)\n",               awb_cfg->multiwindow3_v_size, awb_cfg->multiwindow3_v_size);
    rk_kprintf("sw_rawawb_exc_wp_region0_xu0 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region0_xu0, awb_cfg->exc_wp_region0_xu0);
    rk_kprintf("sw_rawawb_exc_wp_region0_xu1 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region0_xu1, awb_cfg->exc_wp_region0_xu1);
    rk_kprintf("sw_rawawb_exc_wp_region0_yv0 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region0_yv0, awb_cfg->exc_wp_region0_yv0);
    rk_kprintf("sw_rawawb_exc_wp_region0_yv1 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region0_yv1, awb_cfg->exc_wp_region0_yv1);
    rk_kprintf("sw_rawawb_exc_wp_region1_xu0 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region1_xu0, awb_cfg->exc_wp_region1_xu0);
    rk_kprintf("sw_rawawb_exc_wp_region1_xu1 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region1_xu1, awb_cfg->exc_wp_region1_xu1);
    rk_kprintf("sw_rawawb_exc_wp_region1_yv0 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region1_yv0, awb_cfg->exc_wp_region1_yv0);
    rk_kprintf("sw_rawawb_exc_wp_region1_yv1 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region1_yv1, awb_cfg->exc_wp_region1_yv1);
    rk_kprintf("sw_rawawb_exc_wp_region2_xu0 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region2_xu0, awb_cfg->exc_wp_region2_xu0);
    rk_kprintf("sw_rawawb_exc_wp_region2_xu1 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region2_xu1, awb_cfg->exc_wp_region2_xu1);
    rk_kprintf("sw_rawawb_exc_wp_region2_yv0 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region2_yv0, awb_cfg->exc_wp_region2_yv0);
    rk_kprintf("sw_rawawb_exc_wp_region2_yv1 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region2_yv1, awb_cfg->exc_wp_region2_yv1);
    rk_kprintf("sw_rawawb_exc_wp_region3_xu0 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region3_xu0, awb_cfg->exc_wp_region3_xu0);
    rk_kprintf("sw_rawawb_exc_wp_region3_xu1 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region3_xu1, awb_cfg->exc_wp_region3_xu1);
    rk_kprintf("sw_rawawb_exc_wp_region3_yv0 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region3_yv0, awb_cfg->exc_wp_region3_yv0);
    rk_kprintf("sw_rawawb_exc_wp_region3_yv1 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region3_yv1, awb_cfg->exc_wp_region3_yv1);
    rk_kprintf("sw_rawawb_exc_wp_region4_xu0 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region4_xu0, awb_cfg->exc_wp_region4_xu0);
    rk_kprintf("sw_rawawb_exc_wp_region4_xu1 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region4_xu1, awb_cfg->exc_wp_region4_xu1);
    rk_kprintf("sw_rawawb_exc_wp_region4_yv0 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region4_yv0, awb_cfg->exc_wp_region4_yv0);
    rk_kprintf("sw_rawawb_exc_wp_region4_yv1 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region4_yv1, awb_cfg->exc_wp_region4_yv1);
    rk_kprintf("sw_rawawb_exc_wp_region5_xu0 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region5_xu0, awb_cfg->exc_wp_region5_xu0);
    rk_kprintf("sw_rawawb_exc_wp_region5_xu1 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region5_xu1, awb_cfg->exc_wp_region5_xu1);
    rk_kprintf("sw_rawawb_exc_wp_region5_yv0 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region5_yv0, awb_cfg->exc_wp_region5_yv0);
    rk_kprintf("sw_rawawb_exc_wp_region5_yv1 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region5_yv1, awb_cfg->exc_wp_region5_yv1);
    rk_kprintf("sw_rawawb_exc_wp_region6_xu0 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region6_xu0, awb_cfg->exc_wp_region6_xu0);
    rk_kprintf("sw_rawawb_exc_wp_region6_xu1 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region6_xu1, awb_cfg->exc_wp_region6_xu1);
    rk_kprintf("sw_rawawb_exc_wp_region6_yv0 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region6_yv0, awb_cfg->exc_wp_region6_yv0);
    rk_kprintf("sw_rawawb_exc_wp_region6_yv1 = 0x%0x (%d)\n",                awb_cfg->exc_wp_region6_yv1, awb_cfg->exc_wp_region6_yv1);
    rk_kprintf("bls2 en(%d),val_rggb(%d,%d,%d,%d))\n", awb_cfg->bls2_en, awb_cfg->bls2_val.r,
               awb_cfg->bls2_val.gr, awb_cfg->bls2_val.gb, awb_cfg->bls2_val.b);

    rk_kprintf("awb_cfg->wp_blk_wei_w:");
    for (int i = 0; i < 225; i++)
    {
        if (i % 15 == 0)
        {
            rk_kprintf("\n");
        }
        rk_kprintf("0x%03x (%3d),", awb_cfg->wp_blk_wei_w[i], awb_cfg->wp_blk_wei_w[i]);

    }

    rk_kprintf("\n------------exit-----------\n\n");
#endif
}

void interpolation_f(const float *x, const float *y, int Num, float x0, float *y0)
{
    int i, index;
    float k;
    if (x0 <= x[0])
    {
        k = y[0];
    }
    else if (x0 >= x[Num - 1])
    {
        k = y[Num - 1];
    }
    else
    {
        for (i = 0; i < Num; i++)
        {
            if (x0 < x[i])
                break;
        }

        index = i - 1;
        if ((float)x[index + 1] - (float)x[index] < 0.001)
            k = (float)y[index];
        else
            k = ((float)x0 - (float)x[index]) / ((float)x[index + 1] - (float)x[index])
                * ((float)y[index + 1] - (float)y[index])
                + (float)y[index];
    }

    *y0 = k;
}
void calcInputBitIs12Bit(uint8_t *inputBitIs12Bit, const awb_Stats_t *awb_stat_calib,
                         int hdr_mode, bool ablc_en, float isp_ob_predgain)
{
    // inputShiftEnable is true for 20bit ipnut ,for CALIB_AWB_INPUT_BAYERNR + (hdr case  or
    // dgainInAwbGain) is enbale case
    // to do  inputShiftEnable = !inputBitIs12Bit;
    bool selectBayerNrData = (awb_stat_calib->hw_awbCfg_statsSrc_mode == awbStats_btnrOut_mode);
    bool dgainEn           = ablc_en && isp_ob_predgain > 1;
    if (selectBayerNrData &&
            (dgainEn || hdr_mode > 0))
    {
        *inputBitIs12Bit = false;
    }
    else
    {
        *inputBitIs12Bit = true;
    }
}
int filtOutFixed(float fval, awb_wpSpace_mode_t hw_awbT_wpSpace_mode)
{
    if (hw_awbT_wpSpace_mode == awbStats_xyWp_mode)
        return UtlFloatToFix_S0310(fval);
    else
        return (uint16_t)(fval * 16 + 0.5);
}
void ConfigBlc2(const awb_Stats_t *awb_stat_calib, float isp_dgain,
                float isp_ob_predgain, int hdr_mode, float aec_iso, bool ablc_en,
                struct isp33_rawawb_meas_cfg *awb_cfg)
{

    awb_cfg->bls2_en     = false;
    awb_cfg->bls2_val.r  = 0;
    awb_cfg->bls2_val.gr = 0;
    awb_cfg->bls2_val.gb = 0;
    awb_cfg->bls2_val.b  = 0;
    if (awb_stat_calib->hw_awbCfg_statsSrc_mode == awbStats_drcOut_mode ||
            (awb_stat_calib->hw_awbCfg_statsSrc_mode == awbStats_btnrOut_mode &&
             hdr_mode > 0))
    {
        // don't support to use blc2
        return;
    }
    awb_cfg->bls2_en   = true;

    float dgain2                = 1.0;
    // sw_blcT_obcPostTnr_mode == blc_autoOBCPostTnr_mode : use oboffset only
    // sw_blcT_obcPostTnr_mode == blc_manualOBCPostTnr_mode : use blc1 only
    if (ablc_en)
    {
        if (isp_ob_predgain > 1)
        {
            dgain2 = isp_ob_predgain;
        }
        // if (ablc->obcPostTnr.sw_blcT_obcPostTnr_en &&
        //     ablc->obcPostTnr.sw_blcT_obcPostTnr_mode == blc_manualOBCPostTnr_mode) {
        //     blc1[AWB_CHANNEL_R] =
        //         (float)ablc->obcPostTnr
        //             .hw_blcT_manualOBR_val;  // check blc1 was already multiplied by isp_dgain?
        //     blc1[AWB_CHANNEL_GR] = (float)ablc->obcPostTnr.hw_blcT_manualOBGr_val;
        //     blc1[AWB_CHANNEL_GB] = (float)ablc->obcPostTnr.hw_blcT_manualOBGb_val;
        //     blc1[AWB_CHANNEL_B]  = (float)ablc->obcPostTnr.hw_blcT_manualOBB_val;
        // }
    }

    const awb_blc_t *blc2 = &awb_stat_calib->blc2ForAwb;
    // 1 interpolation
    float offset[AWB_CHANNEL_MAX] = {0, 0, 0, 0};
    if (blc2->enable && blc2->offset.offset_len > 0)
    {
        interpolation_f(blc2->offset.iso, blc2->offset.r_val,
                        blc2->offset.offset_len, aec_iso * dgain2, &offset[AWB_CHANNEL_R]);
        interpolation_f(blc2->offset.iso, blc2->offset.gr_val,
                        blc2->offset.offset_len, aec_iso * dgain2, &offset[AWB_CHANNEL_GR]);
        interpolation_f(blc2->offset.iso, blc2->offset.b_val,
                        blc2->offset.offset_len, aec_iso * dgain2, &offset[AWB_CHANNEL_B]);
        interpolation_f(blc2->offset.iso, blc2->offset.gb_val,
                        blc2->offset.offset_len, aec_iso * dgain2, &offset[AWB_CHANNEL_GB]);
    }
    // 2 blc2 recalc base on ablc
    float blc1[AWB_CHANNEL_MAX] = {0, 0, 0, 0};
    if (awb_stat_calib->hw_awbCfg_statsSrc_mode == awbStats_btnrOut_mode)
    {
        // update by (offset +blc1)*dgain
        // hdr_mode = normal,so   applyPosition = IN_AWBGAIN1
        awb_cfg->bls2_val.r =
            (offset[AWB_CHANNEL_R] + blc1[AWB_CHANNEL_R]) * isp_dgain * dgain2 + 0.5;
        awb_cfg->bls2_val.b =
            (offset[AWB_CHANNEL_B] + blc1[AWB_CHANNEL_B]) * isp_dgain * dgain2 + 0.5;
        awb_cfg->bls2_val.gr =
            (offset[AWB_CHANNEL_GR] + blc1[AWB_CHANNEL_GR]) * isp_dgain * dgain2 + 0.5;
        awb_cfg->bls2_val.gb =
            (offset[AWB_CHANNEL_GB] + blc1[AWB_CHANNEL_GB]) * isp_dgain * dgain2 + 0.5;
    }
    else      // select raw
    {
        // update by offset +blc1
        awb_cfg->bls2_val.r  = (offset[AWB_CHANNEL_R] + blc1[AWB_CHANNEL_R]) + 0.5;
        awb_cfg->bls2_val.b  = (offset[AWB_CHANNEL_B] + blc1[AWB_CHANNEL_B]) + 0.5;
        awb_cfg->bls2_val.gr = (offset[AWB_CHANNEL_GR] + blc1[AWB_CHANNEL_GR]) + 0.5;
        awb_cfg->bls2_val.gb = (offset[AWB_CHANNEL_GB] + blc1[AWB_CHANNEL_GB]) + 0.5;
    }
}
// call after blc2 calc
void ConfigOverexposureValue(struct isp33_isp_params_cfg *cfg, float hdrmge_gain0_1,
                             float isp_ob_predgain, int hdr_mode,
                             const awb_Stats_t *awb_stat_calib,
                             bool ablc_en)
{
    struct isp33_rawawb_meas_cfg *awb_cfg =   &cfg->meas.rawawb;
    awb_cfg->in_overexposure_check_en = true;
    float dgain                           = 1.0;
    int max_blc                           = 0;
    short ob                              = 0;
    int overexposure_value                = 254;

    /*if (ablc)
        ob = ablc->isp_ob_offset;*/
    if (awb_stat_calib->hw_awbCfg_statsSrc_mode == awbStats_drcOut_mode)
    {
        awb_cfg->in_overexposure_threshold = overexposure_value;
    }
    else if (awb_stat_calib->hw_awbCfg_statsSrc_mode == awbStats_btnrOut_mode)
    {
        if (ablc_en && isp_ob_predgain > 1)
        {
            dgain = isp_ob_predgain;
        }
        if (hdr_mode > 0)
        {
            // hdr mode,awbgain0 will reset the out data to full range
            rk_kprintf("hdrmge_gain0_1 %f", hdrmge_gain0_1);
            awb_cfg->in_overexposure_threshold = overexposure_value * hdrmge_gain0_1 * dgain;
        }
        else
        {
            // need check
            short int blc[AWB_CHANNEL_MAX] = {0, 0, 0, 0};
            if (ablc_en)
            {
                blc[AWB_CHANNEL_R]  = cfg->others.bls_cfg.fixed_val.r * dgain - ob;
                blc[AWB_CHANNEL_GR] = cfg->others.bls_cfg.fixed_val.gr * dgain - ob;
                blc[AWB_CHANNEL_GB] = cfg->others.bls_cfg.fixed_val.gb * dgain - ob;
                blc[AWB_CHANNEL_B]  = cfg->others.bls_cfg.fixed_val.b * dgain - ob;
            }
            for (int i = 0; i < AWB_CHANNEL_MAX; i++)
            {
                if (blc[i] < 0)
                {
                    blc[i] = 0;
                }
            }
            blc[AWB_CHANNEL_B] += awb_cfg->bls2_val.b + ob;
            blc[AWB_CHANNEL_R] += awb_cfg->bls2_val.r + ob;
            blc[AWB_CHANNEL_GB] += awb_cfg->bls2_val.gb + ob;
            blc[AWB_CHANNEL_GR] += awb_cfg->bls2_val.gr + ob;
            for (int i = 0; i < AWB_CHANNEL_MAX; i++)
            {
                if (blc[i] > max_blc)
                {
                    max_blc = blc[i];
                }
            }
            if (max_blc < 0)
            {
                max_blc = 0;
            }
            awb_cfg->in_overexposure_threshold = overexposure_value * dgain - max_blc / 16;
        }

    }
    else
    {
        // raw
        // need check
        short int blc[AWB_CHANNEL_MAX] = {0, 0, 0, 0};
        if (ablc_en)
        {
            blc[AWB_CHANNEL_R]  = cfg->others.bls_cfg.fixed_val.r - ob;
            blc[AWB_CHANNEL_GR] = cfg->others.bls_cfg.fixed_val.gr - ob;
            blc[AWB_CHANNEL_GB] = cfg->others.bls_cfg.fixed_val.gb - ob;
            blc[AWB_CHANNEL_B]  = cfg->others.bls_cfg.fixed_val.b - ob;
        }

        for (int i = 0; i < AWB_CHANNEL_MAX; i++)
        {
            if (blc[i] < 0)
            {
                blc[i] = 0;
            }
        }
        blc[AWB_CHANNEL_B] += awb_cfg->bls2_val.b + ob;
        blc[AWB_CHANNEL_R] += awb_cfg->bls2_val.r + ob;
        blc[AWB_CHANNEL_GB] += awb_cfg->bls2_val.gb + ob;
        blc[AWB_CHANNEL_GR] += awb_cfg->bls2_val.gr + ob;
        for (int i = 0; i < AWB_CHANNEL_MAX; i++)
        {
            if (blc[i] > max_blc)
            {
                max_blc = blc[i];
            }
        }
        if (max_blc < 0)
        {
            max_blc = 0;
        }
        awb_cfg->in_overexposure_threshold = overexposure_value - max_blc / 16;
    }
}

void UvParaFixed32(const awbStats_uvRegion_t *wpRegion, unsigned short pu_region[5],
                   unsigned short pv_region[5], int slope_inv[4])
{
    // unsigned short pu_region[5];//12 bit
    // unsigned short pv_region[5];
    // int slope_inv[4];//10+10 signed 1/k
    for (int j = 0; j < 5; j++)
    {
        if (j == 4)
        {
            pu_region[4] = pu_region[0];
            pv_region[4] = pv_region[0];
        }
        else
        {
            pu_region[j] = (uint16_t)(wpRegion->regionVtx[j].hw_awbT_vtxU_val * 16 + 0.5);
            pv_region[j] = (uint16_t)(wpRegion->regionVtx[j].hw_awbT_vtxV_val * 16 + 0.5);
        }
        if (j > 0 && j < 5)
        {
            int tmp = pv_region[j] - pv_region[j - 1];
            if (tmp != 0)
            {
                float tmp2       = (float)(pu_region[j] - pu_region[j - 1]) / (float)tmp;
                slope_inv[j - 1] = tmp2 > 0
                                   ? (int32_t)(tmp2 * (1 << AWB_UV_RANGE_SLOVE_FRACTION) + 0.5)
                                   : (int32_t)(tmp2 * (1 << AWB_UV_RANGE_SLOVE_FRACTION) - 0.5);
            }
            else
            {
                slope_inv[j - 1] =
                    (1 << (AWB_UV_RANGE_SLOVE_FRACTION + AWB_UV_RANGE_SLOVE_SINTER - 1)) - 1;
            }
        }
    }
}

void ConfigPreWbgain3(struct isp33_rawawb_meas_cfg *awb_cfg,
                             const awb_Stats_t *awb_stat_calib,
                             struct isp_awbgain_cfg rtt_awbgain,
                             int hdr_mode) {

    // higher priority than ConfigPreWbgain2
    // call after wbgain  and frameChoose calculation,
    // 1) awb statistics before the awbgain application:
    //    prewbgain = iqMap2MainCam.wbgain
    // 2) awb statistics after the awbgain application:
    //    prewbgain =1/stat3aAwbGainOut *iqMap2MainCam.wbgain
    float preWbgainSw[4]={1,1,1,1};
    /*if (awb_stat_calib->mode == AWB_CFG_MODE_RK &&
        (awb_stat_calib->preWbgainSw[0] * awb_stat_calib->preWbgainSw[1] * awb_stat_calib->preWbgainSw[2] *
             awb_stat_calib->preWbgainSw[3] >
         0.0001)) {
        preWbgainSw[0] = awb_stat_calib->preWbgainSw[0];
        preWbgainSw[1] = awb_stat_calib->preWbgainSw[1];
        preWbgainSw[2] = awb_stat_calib->preWbgainSw[2];
        preWbgainSw[3] = awb_stat_calib->preWbgainSw[3];
    } else {
        if (awb_stat_calib->mode == AWB_CFG_MODE_RK) {
            LOGE_AWB("%s, wrong awb_stat_calib->preWbgainSw!!! ", __FUNCTION__);
        }
        preWbgainSw[0] = 1;
        preWbgainSw[1] = 1;
        preWbgainSw[2] = 1;
        preWbgainSw[3] = 1;
    }*/

    // pre_wbgain_inv_b is updating  for awb0-gain enable+select = bnr/hdrc case
    if (( rtt_awbgain.r * rtt_awbgain.gr * rtt_awbgain.b > 0.0001) &&
        (awb_stat_calib->hw_awbCfg_statsSrc_mode == awbStats_drcOut_mode ||
         (hdr_mode>0 &&
          awb_stat_calib->hw_awbCfg_statsSrc_mode == awbStats_btnrOut_mode))) {
        // for awb statistics after the awbgain application
        awb_cfg->pre_wbgain_inv_r =
            (1 << RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT) * (1 << RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT) /  (float)rtt_awbgain.r * preWbgainSw[0] + 0.5;
        awb_cfg->pre_wbgain_inv_g =
            (1 << RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT) * (1 << RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT) /  (float)rtt_awbgain.gr * preWbgainSw[1] + 0.5;
        awb_cfg->pre_wbgain_inv_b =
            (1 << RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT) * (1 << RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT)  /  (float)rtt_awbgain.b * preWbgainSw[3] + 0.5;

    } else {
        if ((awb_stat_calib->hw_awbCfg_statsSrc_mode == awbStats_drcOut_mode ||
             (hdr_mode>0 &&
              awb_stat_calib->hw_awbCfg_statsSrc_mode == awbStats_btnrOut_mode))) {
            rk_kprintf("%s, wrong awb_gain!!! ", __FUNCTION__);
        }
        // for awb statistics before the awbgain application
        awb_cfg->pre_wbgain_inv_r =
            (1 << RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT) * preWbgainSw[0] + 0.5;
        awb_cfg->pre_wbgain_inv_g =
            (1 << RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT) * preWbgainSw[1] + 0.5;
        awb_cfg->pre_wbgain_inv_b =
            (1 << RK_AIQ_AWB_PRE_WBGAIN_FRAC_BIT) * preWbgainSw[3] + 0.5;
    }
    rk_kprintf("rtt_awbgain:(%d, %d, %d,%d),pre_wbgain_inv_r(%d,%d,%d)",rtt_awbgain.r,rtt_awbgain.gr,rtt_awbgain.gb,rtt_awbgain.b,
     awb_cfg->pre_wbgain_inv_r, awb_cfg->pre_wbgain_inv_g, awb_cfg->pre_wbgain_inv_b);
/*
    if (otp_awb != NULL && otp_awb->flag) {
        rk_kprintf("%s before otp pre_wbgain_inv = [%d, %d, %d];", __FUNCTION__,
                 awb_cfg->pre_wbgain_inv_r, awb_cfg->pre_wbgain_inv_g,
                 awb_cfg->pre_wbgain_inv_b);
        awb_cfg->pre_wbgain_inv_r = (float)awb_cfg->pre_wbgain_inv_r *
                                            (float)otp_awb->golden_r_value /
                                            (float)otp_awb->r_value +
                                        0.5;
        awb_cfg->pre_wbgain_inv_g = (float)awb_cfg->pre_wbgain_inv_g *
                                            (float)(otp_awb->golden_gr_value + 1024) /
                                            (float)(otp_awb->gr_value + 1024) +
                                        0.5;
        awb_cfg->pre_wbgain_inv_b = (float)awb_cfg->pre_wbgain_inv_b *
                                            (float)otp_awb->golden_b_value /
                                            (float)otp_awb->b_value +
                                        0.5;
        rk_kprintf("otp pre_wbgain_inv = [%d, %d, %d] \n", awb_cfg->pre_wbgain_inv_r,
                 awb_cfg->pre_wbgain_inv_g, awb_cfg->pre_wbgain_inv_b);
    }
*/

}




void  ConfigLimitRange(const awb_rgbySpace_t *rgbySpace, awbStats_rgbyWp_t *wpDct_rgbySpace)
{

    if (rgbySpace->cfg.cfg_len <= 0)
    {
        rk_kprintf("no rgbySpace config !!! rgbyLimit->cfg.cfg_len = %d", rgbySpace->cfg.cfg_len);
        return;
    }
    if (rgbySpace->sw_awbCfg_rgbyLimit_en)
    {
        wpDct_rgbySpace->hw_awbT_wpMaxR_thred = rgbySpace->cfg.maxR_thred[0];
        wpDct_rgbySpace->hw_awbT_wpMinR_thred = rgbySpace->cfg.minR_thred[0];
        wpDct_rgbySpace->hw_awbT_wpMaxG_thred = rgbySpace->cfg.maxG_thred[0];
        wpDct_rgbySpace->hw_awbT_wpMinG_thred = rgbySpace->cfg.minG_thred[0];
        wpDct_rgbySpace->hw_awbT_wpMaxB_thred = rgbySpace->cfg.maxB_thred[0];
        wpDct_rgbySpace->hw_awbT_wpMinB_thred = rgbySpace->cfg.minB_thred[0];
        wpDct_rgbySpace->hw_awbT_wpMaxY_thred = rgbySpace->cfg.maxY_thred[0];
        wpDct_rgbySpace->hw_awbT_wpMinY_thred = rgbySpace->cfg.minY_thred[0];
    }
    else
    {
        wpDct_rgbySpace->hw_awbT_wpMaxR_thred = 255;
        wpDct_rgbySpace->hw_awbT_wpMinR_thred = 0.0625;
        wpDct_rgbySpace->hw_awbT_wpMaxG_thred = 255;
        wpDct_rgbySpace->hw_awbT_wpMinG_thred = 0.0625;
        wpDct_rgbySpace->hw_awbT_wpMaxB_thred = 255;
        wpDct_rgbySpace->hw_awbT_wpMinB_thred = 0.0625;
        wpDct_rgbySpace->hw_awbT_wpMaxY_thred = 255;
        wpDct_rgbySpace->hw_awbT_wpMinY_thred = 0.0625;
    }

}

uint8_t ConfigIncludeWei(const awb_extRange_t *extraWpRange, int i)
{
    uint8_t wgt;
    float hw_awbT_stats_wgt;
    if (extraWpRange->hw_awbCfg_wpFiltOut_en)
    {
        hw_awbT_stats_wgt = extraWpRange->wpRegionSet[i].wgtInculde.weight[0];
    }
    else
    {
        hw_awbT_stats_wgt = 1;
    }
    wgt = (uint8_t)(hw_awbT_stats_wgt * ((1 << RK_AIQ_WP_INCLUDE_BIS) - 1) + 0.5);
    return wgt;
}

//bool ablc_en = blc && blc->en;
//float isp_dgain=1, isp_ob_predgain=1, aec_iso=50;
int set_awb_params_for_kernel(struct isp33_isp_params_cfg *cfg, awb_api_attrib_t *wb_calib, int hdr_mode,
                              int width, int height, float isp_dgain, float isp_ob_predgain, float aec_iso, float hdrmge_gain0_1, bool ablc_en,struct isp_awbgain_cfg rtt_awbgain)
{

    cfg->module_ens |= ISP33_MODULE_RAWAWB;
    cfg->module_en_update |= ISP33_MODULE_RAWAWB;
    cfg->module_cfg_update |= ISP33_MODULE_RAWAWB;
    if (wb_calib == NULL)
        return 0;

    struct isp33_rawawb_meas_cfg *awb_cfg = &cfg->meas.rawawb;
    const awb_Stats_t *awb_stat_calib = &wb_calib->awbStats;
    //srcChooseCheck

    // to do :
    // config blc2 important
    // pCvt->isp_params.isp_cfg->module_ens |= ISP2X_MODULE_BLS;
    // pCvt->isp_params.isp_cfg->module_cfg_update |= ISP2X_MODULE_BLS;
    // pCvt->isp_params.isp_cfg->module_en_update |= ISP2X_MODULE_BLS;
    //TODO get from ae
    ConfigBlc2(awb_stat_calib,  isp_dgain, isp_ob_predgain, hdr_mode, aec_iso, ablc_en, awb_cfg);

    rk_kprintf("blc2_cfg %d %d,%d,%d,%d\n", awb_cfg->bls2_en, awb_cfg->bls2_val.r,
               awb_cfg->bls2_val.gr, awb_cfg->bls2_val.gb, awb_cfg->bls2_val.b);

    awb_cfg->rawawb_sel = 0;
    if (awb_stat_calib->hw_awbCfg_statsSrc_mode == awbStats_drcOut_mode)
    {
        awb_cfg->drc2awb_sel = 1;
    }
    else
    {
        awb_cfg->drc2awb_sel = 0;
        if (awb_stat_calib->hw_awbCfg_statsSrc_mode == awbStats_btnrOut_mode)
        {
            awb_cfg->bnr2awb_sel = 1;
        }
        else
        {
            awb_cfg->bnr2awb_sel = 0;
            awb_cfg->rawawb_sel  = awb_stat_calib->hw_awbCfg_statsSrc_mode;
            if (hdr_mode == 0 && awb_cfg->rawawb_sel > 0)
            {
                awb_cfg->rawawb_sel = 0;
            }
        }
    }

    calcInputBitIs12Bit(&awb_cfg->low12bit_val, awb_stat_calib, hdr_mode, ablc_en,
                        isp_ob_predgain);
    awb_cfg->in_rshift_to_12bit_en = !awb_cfg->low12bit_val;
    // awb_cfg->ddr_path_en = awb_stat_calib->write2ddrEnable;
    // awb_cfg->ddr_path_sel = awb_stat_calib->write2ddrSelc;
    ConfigOverexposureValue(cfg, hdrmge_gain0_1, isp_ob_predgain, hdr_mode, awb_stat_calib,
                            ablc_en);

    awb_cfg->xy_en0           = awb_stat_calib->hw_awbCfg_xyDct_en;
    awb_cfg->uv_en0           = awb_stat_calib->hw_awbCfg_uvDct_en;
    awb_cfg->xy_en1           = awb_stat_calib->hw_awbCfg_xyDct_en;
    awb_cfg->uv_en1           = awb_stat_calib->hw_awbCfg_uvDct_en;
    awb_cfg->wp_blk_wei_en0   = awb_stat_calib->zoneWgt.hw_awbCfg_zoneWgt_en;
    awb_cfg->wp_blk_wei_en1   = awb_stat_calib->zoneWgt.hw_awbCfg_zoneWgt_en;
    awb_cfg->rawlsc_bypass_en = awb_stat_calib->hw_awbCfg_statsSrc_mode == awbStats_drcOut_mode
                                ? true
                                : (!awb_stat_calib->hw_awbCfg_lsc_en);
    awb_cfg->blk_measure_enable = awb_stat_calib->hw_awbCfg_zoneStats_en;
    awb_cfg->blk_measure_mode =
        awb_stat_calib->hw_awbCfg_zoneStatsSrc_mode > awbStats_pixAll_mode;
    awb_cfg->blk_measure_xytype =
        awb_stat_calib->hw_awbCfg_zoneStatsSrc_mode >= awbStats_bigWpLs0_mode;
    awb_cfg->blk_measure_illu_idx = awb_stat_calib->hw_awbCfg_zoneStatsSrc_mode % 0x10;
    if (awb_stat_calib->hw_awbCfg_zoneStatsSrc_mode == awbStats_pixAll_mode)
    {
        awb_cfg->blk_with_luma_wei_en = false;
    }
    else
    {
        awb_cfg->blk_with_luma_wei_en = true;
    }
    //awb_cfg->blk_with_luma_wei_en = awb_stat_calib->pixEngine.hw_awbCfg_lumaWgt_en;
    awb_cfg->wp_luma_wei_en0      = awb_stat_calib->luma2WpWgt.luma2WpWgt_en;
    awb_cfg->wp_luma_wei_en1      = awb_stat_calib->luma2WpWgt.luma2WpWgt_en;
    awb_cfg->wp_hist_xytype       = awbStats_norWpHist_mode;
    awb_cfg->light_num            = awb_stat_calib->lightSources_len;//check

    if (awb_cfg->light_num > AWBSTATS_WPDCT_LS_NUM)
    {
        awb_cfg->light_num = AWBSTATS_WPDCT_LS_NUM;
        rk_kprintf("light_num should be < %d!!!\n", AWBSTATS_WPDCT_LS_NUM);
    }

    switch ((awbStats_mainWinSize_mode_e)awb_stat_calib->mainWin.hw_awbCfg_win_mode)
    {
    case awbStats_winSizeFull_mode:
        awb_cfg->h_offs = 0;
        awb_cfg->v_offs = 0;
        awb_cfg->h_size = width;
        awb_cfg->v_size = height;
        //wpDetectPara->mainWin.hw_awbCfg_nonROI_en = awb_stat_calib->mainWin.hw_awbCfg_nonROI_en; todo
        //memcpy(wpDetectPara->mainWin.nonROI,awb_stat_calib->mainWin.nonROI,sizeof(awb_stat_calib->mainWin.nonROI));
        break;
    case awbStats_winSizeFixed_mode:
        awb_cfg->h_offs               = awb_stat_calib->mainWin.hw_awbCfg_win_x;
        awb_cfg->v_offs               = awb_stat_calib->mainWin.hw_awbCfg_win_y;
        awb_cfg->h_size               = awb_stat_calib->mainWin.hw_awbCfg_win_width;
        awb_cfg->v_size               = awb_stat_calib->mainWin.hw_awbCfg_win_height;
        if (awb_cfg->h_offs > width ||
                awb_cfg->h_offs + awb_cfg->h_size > width ||
                awb_cfg->v_offs > height ||
                awb_cfg->v_offs + awb_cfg->v_size > height)
        {
            rk_kprintf("windowSet[%d,%d,%d,%d]is invaild!!!\n", awb_cfg->h_offs,
                       awb_cfg->v_offs, awb_cfg->h_size, awb_cfg->v_size);
            return (-1);
            break;
        }
        break;
    default:
        rk_kprintf("measeureWindow.mode is invaild!!!\n", awb_stat_calib->mainWin.hw_awbCfg_win_mode);
        return (-1);
    }

    if (awb_stat_calib->hw_awbCfg_ds_mode == awbStats_ds_4x4 ||
            awb_stat_calib->hw_awbCfg_ds_mode == awbStats_ds_8x8)
    {
        awb_cfg->wind_size      = awb_stat_calib->hw_awbCfg_ds_mode;
        awb_cfg->ds16x8_mode_en = 0;
    }
    else if (awb_stat_calib->hw_awbCfg_ds_mode == awbStats_ds_16x8)
    {
        awb_cfg->ds16x8_mode_en = 1;
        awb_cfg->wind_size      = awbStats_ds_8x8;
    }
    awbStats_rgbyWp_t wpDct_rgbySpace;
    rt_memset(&wpDct_rgbySpace, 0, sizeof(awbStats_rgbyWp_t));
    ConfigLimitRange(&awb_stat_calib->rgbyLimit, &wpDct_rgbySpace);
    awb_cfg->r_max = 16 * wpDct_rgbySpace.hw_awbT_wpMaxR_thred + 0.5;
    awb_cfg->g_max = 16 * wpDct_rgbySpace.hw_awbT_wpMaxG_thred + 0.5;
    awb_cfg->b_max = 16 * wpDct_rgbySpace.hw_awbT_wpMaxB_thred + 0.5;
    awb_cfg->y_max = 16 * wpDct_rgbySpace.hw_awbT_wpMaxY_thred + 0.5;
    awb_cfg->r_min = 16 * wpDct_rgbySpace.hw_awbT_wpMinR_thred + 0.5;
    awb_cfg->g_min = 16 * wpDct_rgbySpace.hw_awbT_wpMinG_thred + 0.5;
    awb_cfg->b_min = 16 * wpDct_rgbySpace.hw_awbT_wpMinB_thred + 0.5;
    awb_cfg->y_min = 16 * wpDct_rgbySpace.hw_awbT_wpMinY_thred + 0.5;

    unsigned short pu_region[5];
    unsigned short pv_region[5];
    int slope_inv[4];
    UvParaFixed32(&awb_stat_calib->lightSources[0].wpDct_uvSpace, pu_region, pv_region, slope_inv);
    awb_cfg->vertex0_u_0 = pu_region[0];
    awb_cfg->vertex0_v_0 = pv_region[0];
    awb_cfg->vertex1_u_0 = pu_region[1];
    awb_cfg->vertex1_v_0 = pv_region[1];
    awb_cfg->vertex2_u_0 = pu_region[2];
    awb_cfg->vertex2_v_0 = pv_region[2];
    awb_cfg->vertex3_u_0 = pu_region[3];
    awb_cfg->vertex3_v_0 = pv_region[3];
    awb_cfg->islope01_0  = slope_inv[0];
    awb_cfg->islope12_0  = slope_inv[1];
    awb_cfg->islope23_0  = slope_inv[2];
    awb_cfg->islope30_0  = slope_inv[3];
    UvParaFixed32(&awb_stat_calib->lightSources[1].wpDct_uvSpace, pu_region, pv_region, slope_inv);
    awb_cfg->vertex0_u_1 = pu_region[0];
    awb_cfg->vertex0_v_1 = pv_region[0];
    awb_cfg->vertex1_u_1 = pu_region[1];
    awb_cfg->vertex1_v_1 = pv_region[1];
    awb_cfg->vertex2_u_1 = pu_region[2];
    awb_cfg->vertex2_v_1 = pv_region[2];
    awb_cfg->vertex3_u_1 = pu_region[3];
    awb_cfg->vertex3_v_1 = pv_region[3];
    awb_cfg->islope01_1  = slope_inv[0];
    awb_cfg->islope12_1  = slope_inv[1];
    awb_cfg->islope23_1  = slope_inv[2];
    awb_cfg->islope30_1  = slope_inv[3];
    UvParaFixed32(&awb_stat_calib->lightSources[2].wpDct_uvSpace, pu_region, pv_region, slope_inv);
    awb_cfg->vertex0_u_2 = pu_region[0];
    awb_cfg->vertex0_v_2 = pv_region[0];
    awb_cfg->vertex1_u_2 = pu_region[1];
    awb_cfg->vertex1_v_2 = pv_region[1];
    awb_cfg->vertex2_u_2 = pu_region[2];
    awb_cfg->vertex2_v_2 = pv_region[2];
    awb_cfg->vertex3_u_2 = pu_region[3];
    awb_cfg->vertex3_v_2 = pv_region[3];
    awb_cfg->islope01_2  = slope_inv[0];
    awb_cfg->islope12_2  = slope_inv[1];
    awb_cfg->islope23_2  = slope_inv[2];
    awb_cfg->islope30_2  = slope_inv[3];
    UvParaFixed32(&awb_stat_calib->lightSources[3].wpDct_uvSpace, pu_region, pv_region, slope_inv);
    awb_cfg->vertex0_u_3 = pu_region[0];
    awb_cfg->vertex0_v_3 = pv_region[0];
    awb_cfg->vertex1_u_3 = pu_region[1];
    awb_cfg->vertex1_v_3 = pv_region[1];
    awb_cfg->vertex2_u_3 = pu_region[2];
    awb_cfg->vertex2_v_3 = pv_region[2];
    awb_cfg->vertex3_u_3 = pu_region[3];
    awb_cfg->vertex3_v_3 = pv_region[3];
    awb_cfg->islope01_3  = slope_inv[0];
    awb_cfg->islope12_3  = slope_inv[1];
    awb_cfg->islope23_3  = slope_inv[2];
    awb_cfg->islope30_3  = slope_inv[3];


    awb_cfg->wt0 =
        UtlFloatToFix_U0012(awb_stat_calib->rgb2xy.hw_awbCfg_rgb2xy_coeff[0]);
    awb_cfg->wt1 =
        UtlFloatToFix_U0012(awb_stat_calib->rgb2xy.hw_awbCfg_rgb2xy_coeff[1]);
    awb_cfg->wt2 =
        UtlFloatToFix_U0012(awb_stat_calib->rgb2xy.hw_awbCfg_rgb2xy_coeff[2]);
    awb_cfg->mat0_x = UtlFloatToFix_S0312(
                          awb_stat_calib->rgb2xy.hw_awbCfg_xyTransMatrix_coeff[0]);
    awb_cfg->mat1_x = UtlFloatToFix_S0312(
                          awb_stat_calib->rgb2xy.hw_awbCfg_xyTransMatrix_coeff[1]);
    awb_cfg->mat2_x = UtlFloatToFix_S0312(
                          awb_stat_calib->rgb2xy.hw_awbCfg_xyTransMatrix_coeff[2]);
    awb_cfg->mat0_y = UtlFloatToFix_S0312(
                          awb_stat_calib->rgb2xy.hw_awbCfg_xyTransMatrix_coeff[3]);
    awb_cfg->mat1_y = UtlFloatToFix_S0312(
                          awb_stat_calib->rgb2xy.hw_awbCfg_xyTransMatrix_coeff[4]);
    awb_cfg->mat2_y = UtlFloatToFix_S0312(
                          awb_stat_calib->rgb2xy.hw_awbCfg_xyTransMatrix_coeff[5]);
    const awbStats_xyRegion_t *norWpRegion = &awb_stat_calib->lightSources[0].wpDct_xySpace.normal;
    const awbStats_xyRegion_t *bigWpRegion = &awb_stat_calib->lightSources[0].wpDct_xySpace.big;
    awb_cfg->nor_x0_0 = UtlFloatToFix_S0310(norWpRegion->ltVtx.hw_awbT_vtxX_val);
    awb_cfg->nor_x1_0 = UtlFloatToFix_S0310(norWpRegion->rbVtx.hw_awbT_vtxX_val);
    awb_cfg->nor_y0_0 = UtlFloatToFix_S0310(norWpRegion->ltVtx.hw_awbT_vtxY_val);
    awb_cfg->nor_y1_0 = UtlFloatToFix_S0310(norWpRegion->rbVtx.hw_awbT_vtxY_val);
    awb_cfg->big_x0_0 = UtlFloatToFix_S0310(bigWpRegion->ltVtx.hw_awbT_vtxX_val);
    awb_cfg->big_x1_0 = UtlFloatToFix_S0310(bigWpRegion->rbVtx.hw_awbT_vtxX_val);
    awb_cfg->big_y0_0 = UtlFloatToFix_S0310(bigWpRegion->ltVtx.hw_awbT_vtxY_val);
    awb_cfg->big_y1_0 = UtlFloatToFix_S0310(bigWpRegion->rbVtx.hw_awbT_vtxY_val);
    norWpRegion           = &awb_stat_calib->lightSources[1].wpDct_xySpace.normal;
    bigWpRegion           = &awb_stat_calib->lightSources[1].wpDct_xySpace.big;
    awb_cfg->nor_x0_1 = UtlFloatToFix_S0310(norWpRegion->ltVtx.hw_awbT_vtxX_val);
    awb_cfg->nor_x1_1 = UtlFloatToFix_S0310(norWpRegion->rbVtx.hw_awbT_vtxX_val);
    awb_cfg->nor_y0_1 = UtlFloatToFix_S0310(norWpRegion->ltVtx.hw_awbT_vtxY_val);
    awb_cfg->nor_y1_1 = UtlFloatToFix_S0310(norWpRegion->rbVtx.hw_awbT_vtxY_val);
    awb_cfg->big_x0_1 = UtlFloatToFix_S0310(bigWpRegion->ltVtx.hw_awbT_vtxX_val);
    awb_cfg->big_x1_1 = UtlFloatToFix_S0310(bigWpRegion->rbVtx.hw_awbT_vtxX_val);
    awb_cfg->big_y0_1 = UtlFloatToFix_S0310(bigWpRegion->ltVtx.hw_awbT_vtxY_val);
    awb_cfg->big_y1_1 = UtlFloatToFix_S0310(bigWpRegion->rbVtx.hw_awbT_vtxY_val);
    norWpRegion           = &awb_stat_calib->lightSources[2].wpDct_xySpace.normal;
    bigWpRegion           = &awb_stat_calib->lightSources[2].wpDct_xySpace.big;
    awb_cfg->nor_x0_2 = UtlFloatToFix_S0310(norWpRegion->ltVtx.hw_awbT_vtxX_val);
    awb_cfg->nor_x1_2 = UtlFloatToFix_S0310(norWpRegion->rbVtx.hw_awbT_vtxX_val);
    awb_cfg->nor_y0_2 = UtlFloatToFix_S0310(norWpRegion->ltVtx.hw_awbT_vtxY_val);
    awb_cfg->nor_y1_2 = UtlFloatToFix_S0310(norWpRegion->rbVtx.hw_awbT_vtxY_val);
    awb_cfg->big_x0_2 = UtlFloatToFix_S0310(bigWpRegion->ltVtx.hw_awbT_vtxX_val);
    awb_cfg->big_x1_2 = UtlFloatToFix_S0310(bigWpRegion->rbVtx.hw_awbT_vtxX_val);
    awb_cfg->big_y0_2 = UtlFloatToFix_S0310(bigWpRegion->ltVtx.hw_awbT_vtxY_val);
    awb_cfg->big_y1_2 = UtlFloatToFix_S0310(bigWpRegion->rbVtx.hw_awbT_vtxY_val);
    norWpRegion           = &awb_stat_calib->lightSources[3].wpDct_xySpace.normal;
    bigWpRegion           = &awb_stat_calib->lightSources[3].wpDct_xySpace.big;
    awb_cfg->nor_x0_3 = UtlFloatToFix_S0310(norWpRegion->ltVtx.hw_awbT_vtxX_val);
    awb_cfg->nor_x1_3 = UtlFloatToFix_S0310(norWpRegion->rbVtx.hw_awbT_vtxX_val);
    awb_cfg->nor_y0_3 = UtlFloatToFix_S0310(norWpRegion->ltVtx.hw_awbT_vtxY_val);
    awb_cfg->nor_y1_3 = UtlFloatToFix_S0310(norWpRegion->rbVtx.hw_awbT_vtxY_val);
    awb_cfg->big_x0_3 = UtlFloatToFix_S0310(bigWpRegion->ltVtx.hw_awbT_vtxX_val);
    awb_cfg->big_x1_3 = UtlFloatToFix_S0310(bigWpRegion->rbVtx.hw_awbT_vtxX_val);
    awb_cfg->big_y0_3 = UtlFloatToFix_S0310(bigWpRegion->ltVtx.hw_awbT_vtxY_val);
    awb_cfg->big_y1_3 = UtlFloatToFix_S0310(bigWpRegion->rbVtx.hw_awbT_vtxY_val);
    ConfigPreWbgain3(awb_cfg,awb_stat_calib,rtt_awbgain,hdr_mode);
    awb_cfg->multiwindow_en      = awb_stat_calib->mainWin.hw_awbCfg_nonROI_en;
    awb_cfg->multiwindow0_h_offs = awb_stat_calib->mainWin.nonROI[0].hw_awbCfg_nonROI_x;
    awb_cfg->multiwindow0_v_offs = awb_stat_calib->mainWin.nonROI[0].hw_awbCfg_nonROI_y;
    awb_cfg->multiwindow0_h_size = awb_stat_calib->mainWin.nonROI[0].hw_awbCfg_nonROI_x +
                                   awb_stat_calib->mainWin.nonROI[0].hw_awbCfg_nonROI_width;
    awb_cfg->multiwindow0_v_size = awb_stat_calib->mainWin.nonROI[0].hw_awbCfg_nonROI_y +
                                   awb_stat_calib->mainWin.nonROI[0].hw_awbCfg_nonROI_height;
    awb_cfg->multiwindow1_h_offs = awb_stat_calib->mainWin.nonROI[1].hw_awbCfg_nonROI_x;
    awb_cfg->multiwindow1_v_offs = awb_stat_calib->mainWin.nonROI[1].hw_awbCfg_nonROI_y;
    awb_cfg->multiwindow1_h_size = awb_stat_calib->mainWin.nonROI[1].hw_awbCfg_nonROI_x +
                                   awb_stat_calib->mainWin.nonROI[1].hw_awbCfg_nonROI_width;
    awb_cfg->multiwindow1_v_size = awb_stat_calib->mainWin.nonROI[1].hw_awbCfg_nonROI_y +
                                   awb_stat_calib->mainWin.nonROI[1].hw_awbCfg_nonROI_height;
    awb_cfg->multiwindow2_h_offs = awb_stat_calib->mainWin.nonROI[2].hw_awbCfg_nonROI_x;
    awb_cfg->multiwindow2_v_offs = awb_stat_calib->mainWin.nonROI[2].hw_awbCfg_nonROI_y;
    awb_cfg->multiwindow2_h_size = awb_stat_calib->mainWin.nonROI[2].hw_awbCfg_nonROI_x +
                                   awb_stat_calib->mainWin.nonROI[2].hw_awbCfg_nonROI_width;
    awb_cfg->multiwindow2_v_size = awb_stat_calib->mainWin.nonROI[2].hw_awbCfg_nonROI_y +
                                   awb_stat_calib->mainWin.nonROI[2].hw_awbCfg_nonROI_height;
    awb_cfg->multiwindow3_h_offs = awb_stat_calib->mainWin.nonROI[3].hw_awbCfg_nonROI_x;
    awb_cfg->multiwindow3_v_offs = awb_stat_calib->mainWin.nonROI[3].hw_awbCfg_nonROI_y;
    awb_cfg->multiwindow3_h_size = awb_stat_calib->mainWin.nonROI[3].hw_awbCfg_nonROI_x +
                                   awb_stat_calib->mainWin.nonROI[3].hw_awbCfg_nonROI_width;
    awb_cfg->multiwindow3_v_size = awb_stat_calib->mainWin.nonROI[3].hw_awbCfg_nonROI_y +
                                   awb_stat_calib->mainWin.nonROI[3].hw_awbCfg_nonROI_height;

    int exc_wp_region0_excen0 = awb_stat_calib->extraWpRange.wpRegionSet[0].mode == awb_wpFiltOut_mode ? 1 : 0;
    int exc_wp_region0_excen1 = awb_stat_calib->extraWpRange.wpRegionSet[0].mode == awb_wpFiltOut_mode ? 1 : 0;
    awb_cfg->exc_wp_region0_excen =
        ((exc_wp_region0_excen1 << 1) + exc_wp_region0_excen0) & 0x3;
    awb_cfg->exc_wp_region0_measen =        !exc_wp_region0_excen0;
    awb_cfg->exc_wp_region0_domain =
        awb_stat_calib->extraWpRange.wpRegionSet[0].domain;
    awb_cfg->exc_wp_region0_xu0 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[0].wpRegion.ltVtx.hw_awbT_vtxXU_val, awb_stat_calib->extraWpRange.wpRegionSet[0].domain);
    awb_cfg->exc_wp_region0_xu1 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[0].wpRegion.rbVtx.hw_awbT_vtxXU_val, awb_stat_calib->extraWpRange.wpRegionSet[0].domain);
    awb_cfg->exc_wp_region0_yv0 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[0].wpRegion.ltVtx.hw_awbT_vtxYV_val, awb_stat_calib->extraWpRange.wpRegionSet[0].domain);
    awb_cfg->exc_wp_region0_yv1 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[0].wpRegion.rbVtx.hw_awbT_vtxYV_val, awb_stat_calib->extraWpRange.wpRegionSet[0].domain);
    awb_cfg->exc_wp_region0_weight = ConfigIncludeWei(&awb_stat_calib->extraWpRange, 0);
    int exc_wp_region1_excen0 = awb_stat_calib->extraWpRange.wpRegionSet[1].mode == awb_wpFiltOut_mode ? 1 : 0;
    int exc_wp_region1_excen1 = awb_stat_calib->extraWpRange.wpRegionSet[1].mode == awb_wpFiltOut_mode ? 1 : 0;
    awb_cfg->exc_wp_region1_excen =
        ((exc_wp_region1_excen1 << 1) + exc_wp_region1_excen0) & 0x3;
    awb_cfg->exc_wp_region1_measen =        !exc_wp_region1_excen0;
    awb_cfg->exc_wp_region1_domain =
        awb_stat_calib->extraWpRange.wpRegionSet[1].domain;
    awb_cfg->exc_wp_region1_xu0 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[1].wpRegion.ltVtx.hw_awbT_vtxXU_val, awb_stat_calib->extraWpRange.wpRegionSet[1].domain);
    awb_cfg->exc_wp_region1_xu1 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[1].wpRegion.rbVtx.hw_awbT_vtxXU_val, awb_stat_calib->extraWpRange.wpRegionSet[1].domain);
    awb_cfg->exc_wp_region1_yv0 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[1].wpRegion.ltVtx.hw_awbT_vtxYV_val, awb_stat_calib->extraWpRange.wpRegionSet[1].domain);
    awb_cfg->exc_wp_region1_yv1 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[1].wpRegion.rbVtx.hw_awbT_vtxYV_val, awb_stat_calib->extraWpRange.wpRegionSet[1].domain);
    awb_cfg->exc_wp_region1_weight = ConfigIncludeWei(&awb_stat_calib->extraWpRange, 1);
    int exc_wp_region2_excen0 = awb_stat_calib->extraWpRange.wpRegionSet[2].mode == awb_wpFiltOut_mode ? 1 : 0;
    int exc_wp_region2_excen1 = awb_stat_calib->extraWpRange.wpRegionSet[2].mode == awb_wpFiltOut_mode ? 1 : 0;
    awb_cfg->exc_wp_region2_excen =
        ((exc_wp_region2_excen1 << 1) + exc_wp_region2_excen0) & 0x3;
    awb_cfg->exc_wp_region2_measen = !exc_wp_region2_excen0;
    awb_cfg->exc_wp_region2_domain =
        awb_stat_calib->extraWpRange.wpRegionSet[2].domain;
    awb_cfg->exc_wp_region2_xu0 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[2].wpRegion.ltVtx.hw_awbT_vtxXU_val, awb_stat_calib->extraWpRange.wpRegionSet[2].domain);
    awb_cfg->exc_wp_region2_xu1 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[2].wpRegion.rbVtx.hw_awbT_vtxXU_val, awb_stat_calib->extraWpRange.wpRegionSet[2].domain);
    awb_cfg->exc_wp_region2_yv0 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[2].wpRegion.ltVtx.hw_awbT_vtxYV_val, awb_stat_calib->extraWpRange.wpRegionSet[2].domain);
    awb_cfg->exc_wp_region2_yv1 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[2].wpRegion.rbVtx.hw_awbT_vtxYV_val, awb_stat_calib->extraWpRange.wpRegionSet[2].domain);
    awb_cfg->exc_wp_region2_weight = ConfigIncludeWei(&awb_stat_calib->extraWpRange, 2);
    int exc_wp_region3_excen0 = awb_stat_calib->extraWpRange.wpRegionSet[3].mode == awb_wpFiltOut_mode ? 1 : 0;
    int exc_wp_region3_excen1 = awb_stat_calib->extraWpRange.wpRegionSet[3].mode == awb_wpFiltOut_mode ? 1 : 0;
    awb_cfg->exc_wp_region3_excen =
        ((exc_wp_region3_excen1 << 1) + exc_wp_region3_excen0) & 0x3;
    awb_cfg->exc_wp_region3_measen = !exc_wp_region3_excen0;
    awb_cfg->exc_wp_region3_domain =
        awb_stat_calib->extraWpRange.wpRegionSet[3].domain;
    awb_cfg->exc_wp_region3_xu0 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[3].wpRegion.ltVtx.hw_awbT_vtxXU_val, awb_stat_calib->extraWpRange.wpRegionSet[2].domain);
    awb_cfg->exc_wp_region3_xu1 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[3].wpRegion.rbVtx.hw_awbT_vtxXU_val, awb_stat_calib->extraWpRange.wpRegionSet[2].domain);
    awb_cfg->exc_wp_region3_yv0 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[3].wpRegion.ltVtx.hw_awbT_vtxYV_val, awb_stat_calib->extraWpRange.wpRegionSet[2].domain);
    awb_cfg->exc_wp_region3_yv1 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[3].wpRegion.rbVtx.hw_awbT_vtxYV_val, awb_stat_calib->extraWpRange.wpRegionSet[2].domain);
    awb_cfg->exc_wp_region3_weight = ConfigIncludeWei(&awb_stat_calib->extraWpRange, 3);
    int exc_wp_region4_excen0 = awb_stat_calib->extraWpRange.wpRegionSet[4].mode == awb_wpFiltOut_mode ? 1 : 0;
    int exc_wp_region4_excen1 = awb_stat_calib->extraWpRange.wpRegionSet[4].mode == awb_wpFiltOut_mode ? 1 : 0;
    awb_cfg->exc_wp_region4_excen =
        ((exc_wp_region4_excen1 << 1) + exc_wp_region4_excen0) & 0x3;
    awb_cfg->exc_wp_region4_domain =
        awb_stat_calib->extraWpRange.wpRegionSet[4].domain;
    awb_cfg->exc_wp_region4_xu0 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[4].wpRegion.ltVtx.hw_awbT_vtxXU_val, awb_stat_calib->extraWpRange.wpRegionSet[4].domain);
    awb_cfg->exc_wp_region4_xu1 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[4].wpRegion.rbVtx.hw_awbT_vtxXU_val, awb_stat_calib->extraWpRange.wpRegionSet[4].domain);
    awb_cfg->exc_wp_region4_yv0 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[4].wpRegion.ltVtx.hw_awbT_vtxYV_val, awb_stat_calib->extraWpRange.wpRegionSet[4].domain);
    awb_cfg->exc_wp_region4_yv1 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[4].wpRegion.rbVtx.hw_awbT_vtxYV_val, awb_stat_calib->extraWpRange.wpRegionSet[4].domain);
    awb_cfg->exc_wp_region4_weight = ConfigIncludeWei(&awb_stat_calib->extraWpRange, 4);
    int exc_wp_region5_excen0 = awb_stat_calib->extraWpRange.wpRegionSet[5].mode == awb_wpFiltOut_mode ? 1 : 0;
    int exc_wp_region5_excen1 = awb_stat_calib->extraWpRange.wpRegionSet[5].mode == awb_wpFiltOut_mode ? 1 : 0;
    awb_cfg->exc_wp_region5_excen =
        ((exc_wp_region5_excen1 << 1) + exc_wp_region5_excen0) & 0x3;
    awb_cfg->exc_wp_region5_domain =
        awb_stat_calib->extraWpRange.wpRegionSet[5].domain;
    awb_cfg->exc_wp_region5_xu0 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[5].wpRegion.ltVtx.hw_awbT_vtxXU_val, awb_stat_calib->extraWpRange.wpRegionSet[5].domain);
    awb_cfg->exc_wp_region5_xu1 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[5].wpRegion.rbVtx.hw_awbT_vtxXU_val, awb_stat_calib->extraWpRange.wpRegionSet[5].domain);
    awb_cfg->exc_wp_region5_yv0 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[5].wpRegion.ltVtx.hw_awbT_vtxYV_val, awb_stat_calib->extraWpRange.wpRegionSet[5].domain);
    awb_cfg->exc_wp_region5_yv1 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[5].wpRegion.rbVtx.hw_awbT_vtxYV_val, awb_stat_calib->extraWpRange.wpRegionSet[5].domain);
    awb_cfg->exc_wp_region5_weight = ConfigIncludeWei(&awb_stat_calib->extraWpRange, 5);
    int exc_wp_region6_excen0 = awb_stat_calib->extraWpRange.wpRegionSet[6].mode == awb_wpFiltOut_mode ? 1 : 0;
    int exc_wp_region6_excen1 = awb_stat_calib->extraWpRange.wpRegionSet[6].mode == awb_wpFiltOut_mode ? 1 : 0;
    awb_cfg->exc_wp_region6_excen =
        ((exc_wp_region6_excen1 << 1) + exc_wp_region6_excen0) & 0x3;
    awb_cfg->exc_wp_region6_domain =
        awb_stat_calib->extraWpRange.wpRegionSet[6].domain;
    awb_cfg->exc_wp_region6_xu0 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[6].wpRegion.ltVtx.hw_awbT_vtxXU_val, awb_stat_calib->extraWpRange.wpRegionSet[6].domain);
    awb_cfg->exc_wp_region6_xu1 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[6].wpRegion.rbVtx.hw_awbT_vtxXU_val, awb_stat_calib->extraWpRange.wpRegionSet[6].domain);
    awb_cfg->exc_wp_region6_yv0 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[6].wpRegion.ltVtx.hw_awbT_vtxYV_val, awb_stat_calib->extraWpRange.wpRegionSet[6].domain);
    awb_cfg->exc_wp_region6_yv1 =
        filtOutFixed(awb_stat_calib->extraWpRange.wpRegionSet[6].wpRegion.rbVtx.hw_awbT_vtxYV_val, awb_stat_calib->extraWpRange.wpRegionSet[6].domain);
    awb_cfg->exc_wp_region6_weight = ConfigIncludeWei(&awb_stat_calib->extraWpRange, 6);
    int hw_awbT_luma2WpWgt_ccm[9] = {64, 0, 0, 0, 64, 0, 0, 0, 64};
    awb_cfg->ccm_coeff0_r = hw_awbT_luma2WpWgt_ccm[0];
    awb_cfg->ccm_coeff1_r = hw_awbT_luma2WpWgt_ccm[1];
    awb_cfg->ccm_coeff2_r = hw_awbT_luma2WpWgt_ccm[2];
    awb_cfg->ccm_coeff0_g = hw_awbT_luma2WpWgt_ccm[3];
    awb_cfg->ccm_coeff1_g = hw_awbT_luma2WpWgt_ccm[4];
    awb_cfg->ccm_coeff2_g = hw_awbT_luma2WpWgt_ccm[5];
    awb_cfg->ccm_coeff0_b = hw_awbT_luma2WpWgt_ccm[6];
    awb_cfg->ccm_coeff1_b = hw_awbT_luma2WpWgt_ccm[7];
    awb_cfg->ccm_coeff2_b = hw_awbT_luma2WpWgt_ccm[8];
    awb_cfg->wp_luma_weicurve_y0 = awb_stat_calib->luma2WpWgt.luma2WpWgt_luma[0];
    awb_cfg->wp_luma_weicurve_y1 = awb_stat_calib->luma2WpWgt.luma2WpWgt_luma[1];
    awb_cfg->wp_luma_weicurve_y2 = awb_stat_calib->luma2WpWgt.luma2WpWgt_luma[2];
    awb_cfg->wp_luma_weicurve_y3 = awb_stat_calib->luma2WpWgt.luma2WpWgt_luma[3];
    awb_cfg->wp_luma_weicurve_y4 = awb_stat_calib->luma2WpWgt.luma2WpWgt_luma[4];
    awb_cfg->wp_luma_weicurve_y5 = awb_stat_calib->luma2WpWgt.luma2WpWgt_luma[5];
    awb_cfg->wp_luma_weicurve_y6 = awb_stat_calib->luma2WpWgt.luma2WpWgt_luma[6];
    awb_cfg->wp_luma_weicurve_y7 = awb_stat_calib->luma2WpWgt.luma2WpWgt_luma[7];
    awb_cfg->wp_luma_weicurve_y8 = awb_stat_calib->luma2WpWgt.luma2WpWgt_luma[8];
    awb_cfg->wp_luma_weicurve_w0 = 63;
    awb_cfg->wp_luma_weicurve_w1 = 63;
    awb_cfg->wp_luma_weicurve_w2 = 63;
    awb_cfg->wp_luma_weicurve_w3 = 63;
    awb_cfg->wp_luma_weicurve_w4 = 63;
    awb_cfg->wp_luma_weicurve_w5 = 63;
    awb_cfg->wp_luma_weicurve_w6 = 63;
    awb_cfg->wp_luma_weicurve_w7 = 63;
    awb_cfg->wp_luma_weicurve_w8 = 63;
    for (int i = 0; i < RK_AIQ_AWB_GRID_NUM_TOTAL; i++)
    {
        awb_cfg->wp_blk_wei_w[i] = awb_stat_calib->zoneWgt.hw_awbCfg_zone_wgt[i];
    }
    awb_cfg->blk_rtdw_measure_en = 0;

    WriteAwbReg(awb_cfg);
}

#endif
