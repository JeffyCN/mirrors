/*
 *
 * Copyright 2015 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define MODULE_TAG "HAL_JPEG_VDPU2"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "mpp_env.h"
#include "mpp_mem.h"
#include "mpp_debug.h"
#include "mpp_frame.h"
#include "mpp_common.h"

#include "jpegd_syntax.h"
#include "hal_jpegd_common.h"
#include "hal_jpegd_vdpu2.h"
#include "hal_jpegd_vdpu2_reg.h"
#include "mpp_dec_cb_param.h"

extern RK_U32 jpegd_debug;

static MPP_RET jpegd_regs_init(JpegRegSet *reg)
{
    jpegd_dbg_func("enter\n");
    memset(reg, 0, sizeof(JpegRegSet));
    reg->reg50_dec_ctrl.sw_dec_out_tiled_e = 0;
    reg->reg50_dec_ctrl.sw_dec_scmd_dis = DEC_SCMD_DISABLE;
    reg->reg50_dec_ctrl.sw_dec_latency = DEC_LATENCY_COMPENSATION;

    reg->reg54_endian.sw_dec_in_endian = DEC_BIG_ENDIAN;
    reg->reg54_endian.sw_dec_out_endian = DEC_LITTLE_ENDIAN;
    reg->reg54_endian.sw_dec_strendian_e = DEC_LITTLE_ENDIAN;
    reg->reg54_endian.sw_dec_outswap32_e = DEC_LITTLE_ENDIAN;
    reg->reg54_endian.sw_dec_inswap32_e = 1;
    reg->reg54_endian.sw_dec_strswap32_e = 1;

    reg->reg55_Interrupt.sw_dec_irq_dis = 0;

    reg->reg56_axi_ctrl.sw_dec_axi_rn_id = 0xff;
    reg->reg56_axi_ctrl.sw_dec_axi_wr_id = 0;
    reg->reg56_axi_ctrl.sw_dec_max_burst = DEC_BUS_BURST_LENGTH_16;
    reg->reg56_axi_ctrl.sw_dec_data_disc_e = DEC_DATA_DISCARD_ENABLE;

    reg->reg57_enable_ctrl.sw_dec_timeout_e = 1;
    reg->reg57_enable_ctrl.sw_dec_clk_gate_e = 1;

    jpegd_dbg_func("exit\n");
    return MPP_OK;
}

static void jpegd_write_code_word_number(JpegdHalCtx *ctx,
                                         JpegdSyntax *syntax)
{
    jpegd_dbg_func("enter\n");
    JpegdSyntax *s = syntax;
    AcTable *ac_ptr0 = NULL, *ac_ptr1 = NULL;
    DcTable *dc_ptr0 = NULL, *dc_ptr1 = NULL;
    JpegdIocRegInfo *info = (JpegdIocRegInfo *)ctx->regs;
    JpegRegSet *reg = &(info->regs);

    /* first, select the table we'll use.
     * this trick is done because hardware always wants luma
     * table as AC hardware table 0.
     */
    if (s->ac_index[0] == HUFFMAN_TABLE_ID_ZERO) {
        /* Luma's AC uses Huffman table zero */
        ac_ptr0 = &(s->ac_table[HUFFMAN_TABLE_ID_ZERO]);
        ac_ptr1 = &(s->ac_table[HUFFMAN_TABLE_ID_ONE]);
    } else {
        ac_ptr0 = &(s->ac_table[HUFFMAN_TABLE_ID_ONE]);
        ac_ptr1 = &(s->ac_table[HUFFMAN_TABLE_ID_ZERO]);
    }

    /* write AC table 0 (luma) */
    reg->reg134.sw_ac1_code1_cnt = ac_ptr0->bits[0];
    reg->reg134.sw_ac1_code2_cnt = ac_ptr0->bits[1];
    reg->reg134.sw_ac1_code3_cnt = ac_ptr0->bits[2];
    reg->reg134.sw_ac1_code4_cnt = ac_ptr0->bits[3];
    reg->reg134.sw_ac1_code5_cnt = ac_ptr0->bits[4];
    reg->reg134.sw_ac1_code6_cnt = ac_ptr0->bits[5];

    reg->reg135.sw_ac1_code7_cnt = ac_ptr0->bits[6];
    reg->reg135.sw_ac1_code8_cnt = ac_ptr0->bits[7];
    reg->reg135.sw_ac1_code9_cnt = ac_ptr0->bits[8];
    reg->reg135.sw_ac1_code10_cnt = ac_ptr0->bits[9];

    reg->reg136.sw_ac1_code11_cnt = ac_ptr0->bits[10];
    reg->reg136.sw_ac1_code12_cnt = ac_ptr0->bits[11];
    reg->reg136.sw_ac1_code13_cnt = ac_ptr0->bits[12];
    reg->reg136.sw_ac1_code14_cnt = ac_ptr0->bits[13];

    reg->reg137.sw_ac1_code15_cnt = ac_ptr0->bits[14];
    reg->reg137.sw_ac1_code16_cnt = ac_ptr0->bits[15];

    /* AC table 1 (the not-luma table) */
    reg->reg137.sw_ac2_code1_cnt = ac_ptr1->bits[0];
    reg->reg137.sw_ac2_code2_cnt = ac_ptr1->bits[1];
    reg->reg137.sw_ac2_code3_cnt = ac_ptr1->bits[2];
    reg->reg137.sw_ac2_code4_cnt = ac_ptr1->bits[3];

    reg->reg138.sw_ac2_code5_cnt = ac_ptr1->bits[4];
    reg->reg138.sw_ac2_code6_cnt = ac_ptr1->bits[5];
    reg->reg138.sw_ac2_code7_cnt = ac_ptr1->bits[6];
    reg->reg138.sw_ac2_code8_cnt = ac_ptr1->bits[7];

    reg->reg139.sw_ac2_code9_cnt = ac_ptr1->bits[8];
    reg->reg139.sw_ac2_code10_cnt = ac_ptr1->bits[9];
    reg->reg139.sw_ac2_code11_cnt = ac_ptr1->bits[10];
    reg->reg139.sw_ac2_code12_cnt = ac_ptr1->bits[11];

    reg->reg140.sw_ac2_code13_cnt = ac_ptr1->bits[12];
    reg->reg140.sw_ac2_code14_cnt = ac_ptr1->bits[13];
    reg->reg140.sw_ac2_code15_cnt = ac_ptr1->bits[14];
    reg->reg140.sw_ac2_code16_cnt = ac_ptr1->bits[15];

    /* first, select the table we'll use.
     * this trick is done because hardware always wants luma
     * table as DC hardware table 0.
     */
    if (s->dc_index[0] == HUFFMAN_TABLE_ID_ZERO) {
        /* Luma's DC uses Huffman table zero */
        dc_ptr0 = &(s->dc_table[HUFFMAN_TABLE_ID_ZERO]);
        dc_ptr1 = &(s->dc_table[HUFFMAN_TABLE_ID_ONE]);
    } else {
        dc_ptr0 = &(s->dc_table[HUFFMAN_TABLE_ID_ONE]);
        dc_ptr1 = &(s->dc_table[HUFFMAN_TABLE_ID_ZERO]);
    }

    /* write DC table 0 (luma) */
    reg->reg141.sw_dc1_code1_cnt = dc_ptr0->bits[0];
    reg->reg141.sw_dc1_code2_cnt = dc_ptr0->bits[1];
    reg->reg141.sw_dc1_code3_cnt = dc_ptr0->bits[2];
    reg->reg141.sw_dc1_code4_cnt = dc_ptr0->bits[3];
    reg->reg141.sw_dc1_code5_cnt = dc_ptr0->bits[4];
    reg->reg141.sw_dc1_code6_cnt = dc_ptr0->bits[5];
    reg->reg141.sw_dc1_code7_cnt = dc_ptr0->bits[6];
    reg->reg141.sw_dc1_code8_cnt = dc_ptr0->bits[7];

    reg->reg142.sw_dc1_code9_cnt = dc_ptr0->bits[8];
    reg->reg142.sw_dc1_code10_cnt = dc_ptr0->bits[9];
    reg->reg142.sw_dc1_code11_cnt = dc_ptr0->bits[10];
    reg->reg142.sw_dc1_code12_cnt = dc_ptr0->bits[11];
    reg->reg142.sw_dc1_code13_cnt = dc_ptr0->bits[12];
    reg->reg142.sw_dc1_code14_cnt = dc_ptr0->bits[13];
    reg->reg142.sw_dc1_code15_cnt = dc_ptr0->bits[14];
    reg->reg142.sw_dc1_code16_cnt = dc_ptr0->bits[15];

    /* DC table 1 (the not-luma table) */
    reg->reg143.sw_dc2_code1_cnt = dc_ptr1->bits[0];
    reg->reg143.sw_dc2_code2_cnt = dc_ptr1->bits[1];
    reg->reg143.sw_dc2_code3_cnt = dc_ptr1->bits[2];
    reg->reg143.sw_dc2_code4_cnt = dc_ptr1->bits[3];
    reg->reg143.sw_dc2_code5_cnt = dc_ptr1->bits[4];
    reg->reg143.sw_dc2_code6_cnt = dc_ptr1->bits[5];
    reg->reg143.sw_dc2_code7_cnt = dc_ptr1->bits[6];
    reg->reg143.sw_dc2_code8_cnt = dc_ptr1->bits[7];

    reg->reg144.sw_dc2_code9_cnt = dc_ptr1->bits[8];
    reg->reg144.sw_dc2_code10_cnt = dc_ptr1->bits[9];
    reg->reg144.sw_dc2_code11_cnt = dc_ptr1->bits[10];
    reg->reg144.sw_dc2_code12_cnt = dc_ptr1->bits[11];
    reg->reg144.sw_dc2_code13_cnt = dc_ptr1->bits[12];
    reg->reg144.sw_dc2_code14_cnt = dc_ptr1->bits[13];
    reg->reg144.sw_dc2_code15_cnt = dc_ptr1->bits[14];
    reg->reg144.sw_dc2_code16_cnt = dc_ptr1->bits[15];

    jpegd_dbg_func("exit\n");
    return;
}

static void
jpegd_set_chroma_table_id(JpegdHalCtx *ctx, JpegdSyntax *syntax)
{
    jpegd_dbg_func("enter\n");
    JpegdSyntax *s = syntax;
    JpegdIocRegInfo *info = (JpegdIocRegInfo *)ctx->regs;
    JpegRegSet *reg = &(info->regs);

    /* this trick is done because hardware always wants
     * luma table as ac hardward table 0
     */
    if (s->ac_index[0] == HUFFMAN_TABLE_ID_ZERO) {
        reg->reg122.sw_cb_ac_vlctable = s->ac_index[1];
        reg->reg122.sw_cr_ac_vlctable = s->ac_index[2];
    } else {
        if (s->ac_index[0] == s->ac_index[1])
            reg->reg122.sw_cb_ac_vlctable = 0;
        else
            reg->reg122.sw_cb_ac_vlctable = 1;

        if (s->ac_index[0] == s->ac_index[2])
            reg->reg122.sw_cr_ac_vlctable = 0;
        else
            reg->reg122.sw_cr_ac_vlctable = 1;
    }

    if (s->dc_index[0] == HUFFMAN_TABLE_ID_ZERO) {
        reg->reg122.sw_cb_dc_vlctable = s->dc_index[1];
        reg->reg122.sw_cr_dc_vlctable = s->dc_index[2];
    } else {
        if (s->dc_index[0] == s->dc_index[1])
            reg->reg122.sw_cb_dc_vlctable = 0;
        else
            reg->reg122.sw_cb_dc_vlctable = 1;

        if (s->dc_index[0] == s->dc_index[2])
            reg->reg122.sw_cr_dc_vlctable = 0;
        else
            reg->reg122.sw_cr_dc_vlctable = 1;
    }

    reg->reg122.sw_cr_dc_vlctable3 = 0;
    reg->reg122.sw_cb_dc_vlctable3 = 0;

    jpegd_dbg_func("exit\n");
    return;
}

static void
jpegd_set_stream_offset(JpegdHalCtx *ctx, JpegdSyntax *syntax)
{
    jpegd_dbg_func("enter\n");
    JpegdSyntax *s = syntax;

    JpegdIocRegInfo *info = (JpegdIocRegInfo *)ctx->regs;
    JpegRegSet *reg = &(info->regs);
    RK_U32 offset = 0, byte_cnt = 0;
    RK_U32 bit_pos_in_byte = 0;
    RK_U32 strm_len_by_hw = 0;
    /* calculate and set stream start address to hw,
     * the offset must be 8-byte aligned.
     */
    offset = (s->strm_offset & (~7));
    reg->reg64_rlc_vlc_base = ctx->pkt_fd;
    if (offset) {
        mpp_dev_set_reg_offset(ctx->dev, 64, offset);
    }
    /* calculate and set stream start bit to hardware
     * change current pos to bus address style
     * remove three lowest bits and add the difference to bitPosInWord
     * used as bit pos in word not as bit pos in byte actually...
     */
    byte_cnt = ((uintptr_t) s->cur_pos & (7));
    bit_pos_in_byte = byte_cnt * 8; /* 1 Byte = 8 bits */
    reg->reg122.sw_strm_start_bit = bit_pos_in_byte;

    /* set up stream length for HW.
     * length = size of original buffer - stream we already decoded in SW
     */
    strm_len_by_hw = s->pkt_len - offset;
    reg->reg51_stream_info.sw_stream_len = strm_len_by_hw;
    reg->reg122.sw_jpeg_stream_all = 1;

    jpegd_dbg_func("exit\n");
    return;
}

static MPP_RET jpegd_setup_pp(JpegdHalCtx *ctx, JpegdSyntax *syntax)
{
    jpegd_dbg_func("enter\n");
    JpegdIocRegInfo *info = (JpegdIocRegInfo *)ctx->regs;
    JpegRegSet *reg = &(info->regs);
    JpegdSyntax *s = syntax;

    RK_U32 in_color = ctx->pp_info.pp_in_fmt;
    RK_U32 out_color = ctx->pp_info.pp_out_fmt;
    RK_U32 dither = ctx->pp_info.dither_enable;
    RK_U32 crop_width = ctx->pp_info.crop_width;
    RK_U32 crop_height = ctx->pp_info.crop_height;
    RK_U32 crop_x = ctx->pp_info.crop_x;
    RK_U32 crop_y = ctx->pp_info.crop_y;
    RK_U32 in_width = s->hor_stride;
    RK_U32 in_height = s->ver_stride;
    RK_U32 out_width = s->hor_stride;
    RK_U32 out_height = s->ver_stride;
    RK_U32 uv_offset = s->hor_stride * s->ver_stride;

    reg->reg0.sw_pp_axi_rd_id = 0;
    reg->reg0.sw_pp_axi_wr_id = 0;
    reg->reg0.sw_pp_scmd_dis = 1;
    reg->reg0.sw_pp_max_burst = 16;

    reg->reg18_pp_in_lu_base = 0;
    reg->reg34.sw_ext_orig_width = in_width >> 4;

    reg->reg37.sw_pp_in_a2_endsel = 1;
    reg->reg37.sw_pp_in_a1_swap32 = 1;
    reg->reg37.sw_pp_in_a1_endian = 1;
    reg->reg37.sw_pp_in_swap32_e = 1;
    reg->reg37.sw_pp_in_endian = 1;
    reg->reg37.sw_pp_out_endian = 1;
    reg->reg37.sw_pp_out_swap32_e = 1;

    reg->reg41.sw_pp_clk_gate_e = 1;
    reg->reg41.sw_pp_ahb_hlock_e = 1;
    reg->reg41.sw_pp_data_disc_e = 1;

    if (crop_width <= 0) {
        reg->reg34.sw_pp_in_w_ext = (((in_width / 16) & 0xE00) >> 9);
        reg->reg34.sw_pp_in_width = ((in_width / 16) & 0x1FF);
        reg->reg34.sw_pp_in_h_ext = (((in_height / 16) & 0x700) >> 8);
        reg->reg34.sw_pp_in_height = ((in_height / 16) & 0x0FF);
    } else {
        reg->reg34.sw_pp_in_w_ext = (((crop_width / 16) & 0xE00) >> 9);
        reg->reg34.sw_pp_in_width = ((crop_width / 16) & 0x1FF);
        reg->reg34.sw_pp_in_h_ext = (((crop_height / 16) & 0x700) >> 8);
        reg->reg34.sw_pp_in_height = ((crop_height / 16) & 0x0FF);

        reg->reg14.sw_crop_startx_ext = (((crop_x / 16) & 0xE00) >> 9);
        reg->reg14.sw_crop_startx = ((crop_x / 16) & 0x1FF);
        reg->reg14.sw_crop_starty_ext = (((crop_y / 16) & 0x700) >> 8);
        reg->reg14.sw_crop_starty = ((crop_y / 16) & 0x0FF);

        if (crop_width & 0x0F) {
            reg->reg14.sw_pp_crop8_r_e = 1;
        } else {
            reg->reg14.sw_pp_crop8_r_e = 0;
        }
        if (crop_height & 0x0F) {
            reg->reg14.sw_pp_crop8_d_e = 1;
        } else {
            reg->reg14.sw_pp_crop8_d_e = 0;
        }
        in_width = crop_width;
        in_height = crop_height;
    }

    reg->reg39.sw_display_width = out_width;
    reg->reg35.sw_pp_out_width = out_width;
    reg->reg35.sw_pp_out_height = out_height;

    switch (in_color) {
    case    PP_IN_FORMAT_YUV422INTERLAVE:
    case    PP_IN_FORMAT_YUV420SEMI:
    case    PP_IN_FORMAT_YUV420PLANAR:
    case    PP_IN_FORMAT_YUV400:
    case    PP_IN_FORMAT_YUV422SEMI:
    case    PP_IN_FORMAT_YUV420SEMITIELED:
    case    PP_IN_FORMAT_YUV440SEMI:
        reg->reg38.sw_pp_in_format = in_color;
        break;
    case    PP_IN_FORMAT_YUV444_SEMI:
        reg->reg38.sw_pp_in_format = 7;
        reg->reg38.sw_pp_in_format_es = 0;
        break;
    case    PP_IN_FORMAT_YUV411_SEMI:
        reg->reg38.sw_pp_in_format = 0;
        reg->reg38.sw_pp_in_format_es = 1;
        break;
    default:
        mpp_err_f("unsupported format:%d", in_color);
        return MPP_NOK;
    }

    RK_U32 video_range = 1;

    reg->reg15.sw_rangemap_coef_y = 9;
    reg->reg15.sw_rangemap_coef_c = 9;
    reg->reg3.sw_pp_color_coefff = BRIGHTNESS;  /* brightness */

    if (out_color <= PP_OUT_FORMAT_ARGB) {
        /*Bt.601*/
        unsigned int    a = 298;
        unsigned int    b = 409;
        unsigned int    c = 208;
        unsigned int    d = 100;
        unsigned int    e = 516;

        /*Bt.709
        unsigned int    a = 298;
        unsigned int    b = 459;
        unsigned int    c = 137;
        unsigned int    d = 55;
        unsigned int    e = 544;*/

        int satur = 0, tmp;
        if (video_range != 0) {
            /*Bt.601*/
            a = 256;
            b = 350;
            c = 179;
            d = 86;
            e = 443;
            /*Bt.709
            a = 256;
            b = 403;
            c = 120;
            d = 48;
            e = 475;*/

            reg->reg15.sw_ycbcr_range = video_range;
        }
        int contrast = CONTRAST;
        if (contrast != 0) {
            int thr1y, thr2y, off1, off2, thr1, thr2, a1, a2;
            if (video_range == 0) {
                int tmp1, tmp2;
                /* Contrast */
                thr1 = (219 * (contrast + 128)) / 512;
                thr1y = (219 - 2 * thr1) / 2;
                thr2 = 219 - thr1;
                thr2y = 219 - thr1y;

                tmp1 = (thr1y * 256) / thr1;
                tmp2 = ((thr2y - thr1y) * 256) / (thr2 - thr1);
                off1 = ((thr1y - ((tmp2 * thr1) / 256)) * a) / 256;
                off2 = ((thr2y - ((tmp1 * thr2) / 256)) * a) / 256;

                tmp1 = (64 * (contrast + 128)) / 128;
                tmp2 = 256 * (128 - tmp1);
                a1 = (tmp2 + off2) / thr1;
                a2 = a1 + (256 * (off2 - 1)) / (thr2 - thr1);
            } else {
                /* Contrast */
                thr1 = (64 * (contrast + 128)) / 128;
                thr1y = 128 - thr1;
                thr2 = 256 - thr1;
                thr2y = 256 - thr1y;
                a1 = (thr1y * 256) / thr1;
                a2 = ((thr2y - thr1y) * 256) / (thr2 - thr1);
                off1 = thr1y - (a2 * thr1) / 256;
                off2 = thr2y - (a1 * thr2) / 256;
            }

            if (a1 > 1023)
                a1 = 1023;
            else if (a1 < 0)
                a1 = 0;

            if (a2 > 1023)
                a2 = 1023;
            else if (a2 < 0)
                a2 = 0;

            if (thr1 > 255)
                thr1 = 255;
            else if (thr1 < 0)
                thr1 = 0;

            if (thr2 > 255)
                thr2 = 255;
            else if (thr2 < 0)
                thr2 = 0;

            if (off1 > 511)
                off1 = 511;
            else if (off1 < -512)
                off1 = -512;

            if (off2 > 511)
                off2 = 511;
            else if (off2 < -512)
                off2 = -512;

            reg->reg31.sw_contrast_thr1 = thr1;
            reg->reg31.sw_contrast_thr2 = thr2;
            reg->reg32.sw_contrast_off1 = off1;
            reg->reg32.sw_contrast_off2 = off2;

            reg->reg1.sw_color_coeffa1 = a1;
            reg->reg1.sw_color_coeffa2 = a2;
        } else {
            reg->reg31.sw_contrast_thr1 = 55;
            reg->reg31.sw_contrast_thr2 = 165;
            reg->reg32.sw_contrast_off1 = 0;
            reg->reg32.sw_contrast_off2 = 0;

            tmp = a;
            if (tmp > 1023)
                tmp = 1023;
            else if (tmp < 0)
                tmp = 0;

            reg->reg1.sw_color_coeffa1 = tmp;
            reg->reg1.sw_color_coeffa2 = tmp;
        }

        reg->reg37.sw_pp_out_endian = 0;

        satur = 64 + SATURATION; /* saturation */
        tmp = (satur * (int) b) / 64;
        if (tmp > 1023)
            tmp = 1023;
        else if (tmp < 0)
            tmp = 0;
        reg->reg1.sw_color_coeffb = (unsigned int) tmp;

        tmp = (satur * (int) c) / 64;
        if (tmp > 1023)
            tmp = 1023;
        else if (tmp < 0)
            tmp = 0;
        reg->reg2.sw_color_coeffc = (unsigned int) tmp;

        tmp = (satur * (int) d) / 64;
        if (tmp > 1023)
            tmp = 1023;
        else if (tmp < 0)
            tmp = 0;
        reg->reg2.sw_color_coeffd = (unsigned int) tmp;

        tmp = (satur * (int) e) / 64;
        if (tmp > 1023)
            tmp = 1023;
        else if (tmp < 0)
            tmp = 0;
        reg->reg2.sw_color_coeffe = (unsigned int) tmp;
    }

    if (out_color <= PP_OUT_FORMAT_ARGB) {
        PpRgbCfg *cfg = get_pp_rgb_Cfg(ctx->output_fmt);
        reg->reg9_r_mask = cfg->r_mask;
        reg->reg10_g_mask = cfg->g_mask;
        reg->reg11_b_mask = cfg->b_mask;
        reg->reg16.sw_rgb_r_padd = cfg->r_padd;
        reg->reg16.sw_rgb_g_padd = cfg->g_padd;
        reg->reg16.sw_rgb_b_padd = cfg->b_padd;

        if (dither) {
            jpegd_dbg_hal("we do dither.");
            reg->reg36.sw_dither_select_r = cfg->r_dither;
            reg->reg36.sw_dither_select_g = cfg->g_dither;
            reg->reg36.sw_dither_select_b = cfg->b_dither;
        } else {
            jpegd_dbg_hal("we do not dither.");
        }

        reg->reg37.sw_rgb_pix_in32 = cfg->rgb_in_32;
        reg->reg37.sw_pp_out_swap16_e = cfg->swap_16;
        reg->reg37.sw_pp_out_swap32_e = cfg->swap_32;
        reg->reg37.sw_pp_out_endian = cfg->out_endian;

        reg->reg38.sw_pp_out_format = 0;

    } else if (out_color == PP_OUT_FORMAT_YUV422INTERLAVE) {
        reg->reg38.sw_pp_out_format = 3;
    } else if (out_color == PP_OUT_FORMAT_YUV420INTERLAVE) {
        reg->reg38.sw_pp_out_format = 5;
    } else {
        mpp_err_f("unsuppotred format:%d", out_color);
        return -1;
    }

    reg->reg38.sw_rotation_mode = 0;

    unsigned int inw, inh;
    unsigned int outw, outh;

    inw = in_width - 1;
    inh = in_height - 1;
    outw = out_width - 1;
    outh = out_height - 1;

    if (inw < outw) {
        reg->reg4.sw_hor_scale_mode = 1;
        reg->reg4.sw_scale_wratio = (outw << 16) / inw;
        reg->reg6.sw_wscale_invra = (inw << 16) / outw;
    } else if (inw > outw) {
        reg->reg4.sw_hor_scale_mode = 2;
        reg->reg6.sw_wscale_invra = ((outw + 1) << 16) / (inw + 1);
    } else
        reg->reg4.sw_hor_scale_mode = 0;

    if (inh < outh) {
        reg->reg4.sw_ver_scale_mode = 1;
        reg->reg5.sw_scale_hratio = (outh << 16) / inh;
        reg->reg6.sw_hscale_invra = (inh << 16) / outh;
    } else if (inh > outh) {
        reg->reg4.sw_ver_scale_mode = 2;
        reg->reg6.sw_hscale_invra = ((outh + 1) << 16) / (inh + 1) + 1;
    } else
        reg->reg4.sw_ver_scale_mode = 0;

    reg->reg41.sw_pp_pipeline_e = ctx->pp_info.pp_enable;

    jpegd_dbg_hal("pp_enable %d\n", ctx->pp_info.pp_enable);

    if (ctx->pp_info.pp_enable) {
        reg->reg41.sw_pp_pipeline_e = 1;
        reg->reg57_enable_ctrl.sw_dec_out_dis = 1;

        reg->reg63_dec_out_base = 0;
        reg->reg131_jpg_ch_out_base = 0;

        reg->reg21_pp_out_lu_base = ctx->frame_fd;
        reg->reg22_pp_out_ch_base = ctx->frame_fd;

        if (uv_offset)
            mpp_dev_set_reg_offset(ctx->dev, 22, uv_offset);

        jpegd_dbg_hal("output_frame_fd:%x, reg22:%x", ctx->frame_fd,
                      reg->reg22_pp_out_ch_base);
    } else {
        // output without pp
        reg->reg41.sw_pp_pipeline_e = 0;
        reg->reg57_enable_ctrl.sw_dec_out_dis = 0;

        reg->reg21_pp_out_lu_base = 0;
        reg->reg22_pp_out_ch_base = 0;

        reg->reg63_dec_out_base = ctx->frame_fd;
        reg->reg131_jpg_ch_out_base = ctx->frame_fd;

        if (uv_offset)
            mpp_dev_set_reg_offset(ctx->dev, 131, uv_offset);

        jpegd_dbg_hal("output_frame_fd:%x, reg131:%x", ctx->frame_fd,
                      reg->reg131_jpg_ch_out_base);
    }

    jpegd_dbg_func("exit\n");
    return MPP_OK;
}

static
MPP_RET jpegd_gen_regs(JpegdHalCtx *ctx, JpegdSyntax *syntax)
{
    MPP_RET ret = MPP_OK;
    jpegd_dbg_func("enter\n");
    JpegdIocRegInfo *info = (JpegdIocRegInfo *)ctx->regs;
    JpegRegSet *reg = &(info->regs);
    JpegdSyntax *s = syntax;

    jpegd_regs_init(reg);

    reg->reg50_dec_ctrl.sw_filtering_dis = 1;
    reg->reg53_dec_mode = DEC_MODE_JPEG;

    reg->reg57_enable_ctrl.sw_dec_e = 1;  /* Enable jpeg mode */
    reg->reg57_enable_ctrl.sw_pjpeg_e = 0;
    reg->reg57_enable_ctrl.sw_dec_out_dis = 0;
    reg->reg57_enable_ctrl.sw_rlc_mode_e = 0;

    /* frame size, round up the number of mbs */
    reg->reg120.sw_pic_mb_h_ext = ((((s->ver_stride) >> (4)) & 0x700) >> 8);
    reg->reg120.sw_pic_mb_w_ext = ((((s->hor_stride) >> (4)) & 0xE00) >> 9);
    reg->reg120.sw_pic_mb_width = ((s->hor_stride) >> (4)) & 0x1FF;
    reg->reg120.sw_pic_mb_hight_p = ((s->ver_stride) >> (4)) & 0x0FF;

    reg->reg121.sw_pjpeg_fildown_e = s->fill_bottom;
    /* Set spectral selection start coefficient */
    reg->reg121.sw_pjpeg_ss = s->scan_start;
    /* Set spectral selection end coefficient */
    reg->reg121.sw_pjpeg_se = s->scan_end;
    /* Set the point transform used in the preceding scan */
    reg->reg121.sw_pjpeg_ah = s->prev_shift;
    /* Set the point transform value */
    reg->reg121.sw_pjpeg_al = s->point_transform;

    reg->reg122.sw_jpeg_qtables = s->qtable_cnt;
    reg->reg122.sw_jpeg_mode = s->yuv_mode;
    reg->reg122.sw_jpeg_filright_e = s->fill_right;

    reg->reg148.sw_slice_h = 0;
    /* Set bit 21 of reg148 to 1, notifying hardware to decode jpeg
     * including DRI segment
     */
    reg->reg148.sw_syn_marker_e = 1;

    /* tell hardware that height is 8-pixel aligned,
     * but not 16-pixel aligned
     */
    if ((s->height % 16) && ((s->height % 16) <= 8) &&
        (!ctx->pp_info.pp_enable) &&
        (s->yuv_mode == JPEGDEC_YUV422 ||
         s->yuv_mode == JPEGDEC_YUV444 ||
         s->yuv_mode == JPEGDEC_YUV411)) {
        reg->reg148.sw_jpeg_height8_flag = 1;
    }

    /* write VLC code word number to register */
    jpegd_write_code_word_number(ctx, s);

    /* Create AC/DC/QP tables for hardware */
    jpegd_write_qp_ac_dc_table(ctx, s);

    /* Select which tables the chromas use */
    jpegd_set_chroma_table_id(ctx, s);

    /* write table base */
    reg->reg61_qtable_base = mpp_buffer_get_fd(ctx->pTableBase);
    if (reg->reg61_qtable_base <= 0) {
        mpp_err_f("get qtable_base failed\n");
        return MPP_NOK;
    }
    /* set up stream position for HW decode */
    jpegd_set_stream_offset(ctx, s);

    /* set restart interval */
    if (s->restart_interval) {
        reg->reg122.sw_sync_marker_e = 1;
        /* If exists DRI segment, bit 0 to bit 15 of reg123 is set
         * to restart interval */
        reg->reg123.sw_pjpeg_rest_freq = s->restart_interval;
    } else {
        reg->reg122.sw_sync_marker_e = 0;
    }

    jpegd_setup_pp(ctx, syntax);

    jpegd_dbg_func("exit\n");
    return ret;
}

MPP_RET hal_jpegd_vdpu2_init(void *hal, MppHalCfg *cfg)
{
    MPP_RET ret = MPP_OK;
    JpegdHalCtx *JpegHalCtx = (JpegdHalCtx *)hal;

    mpp_assert(JpegHalCtx);
    jpegd_dbg_func("enter\n");

    //configure
    JpegHalCtx->dec_cb       = cfg->dec_cb;
    JpegHalCtx->packet_slots = cfg->packet_slots;
    JpegHalCtx->frame_slots  = cfg->frame_slots;
    JpegHalCtx->have_pp      = cfg->hw_info->cap_jpg_pp_out;

    //init regs
    JpegdIocRegInfo *info = NULL;
    info = mpp_calloc(JpegdIocRegInfo, 1);
    if (info == NULL) {
        mpp_err_f("allocate jpegd ioctl info failed\n");
        return MPP_ERR_NOMEM;
    }
    memset(info, 0, sizeof(JpegdIocRegInfo));
    JpegHalCtx->regs = (void *)info;

    //malloc hw buf
    if (JpegHalCtx->group == NULL) {
        ret = mpp_buffer_group_get_internal(&JpegHalCtx->group,
                                            MPP_BUFFER_TYPE_ION);
        if (ret) {
            mpp_err_f("mpp_buffer_group_get failed\n");
            return ret;
        }
    }

    ret = mpp_buffer_get(JpegHalCtx->group, &JpegHalCtx->pTableBase,
                         JPEGD_BASELINE_TABLE_SIZE);
    if (ret) {
        mpp_err_f("get buffer failed\n");
        return ret;
    }

    PPInfo *pp_info = &(JpegHalCtx->pp_info);
    memset(pp_info, 0, sizeof(PPInfo));
    pp_info->pp_enable = 0;
    pp_info->pp_in_fmt = PP_IN_FORMAT_YUV420SEMI;
    pp_info->pp_out_fmt = PP_OUT_FORMAT_YUV420INTERLAVE;

    JpegHalCtx->output_fmt = MPP_FMT_YUV420SP;
    JpegHalCtx->set_output_fmt_flag = 0;

    //init dbg stuff
    JpegHalCtx->hal_debug_enable = 0;
    JpegHalCtx->frame_count = 0;
    JpegHalCtx->output_yuv_count = 0;

    jpegd_dbg_func("exit\n");
    return MPP_OK;
}

MPP_RET hal_jpegd_vdpu2_deinit(void *hal)
{
    MPP_RET ret = MPP_OK;
    JpegdHalCtx *JpegHalCtx = (JpegdHalCtx *)hal;

    jpegd_dbg_func("enter\n");

    if (JpegHalCtx->dev) {
        mpp_dev_deinit(JpegHalCtx->dev);
        JpegHalCtx->dev = NULL;
    }

    if (JpegHalCtx->pTableBase) {
        ret = mpp_buffer_put(JpegHalCtx->pTableBase);
        if (ret) {
            mpp_err_f("put table buffer failed\n");
            return ret;
        }
    }

    if (JpegHalCtx->group) {
        ret = mpp_buffer_group_put(JpegHalCtx->group);
        if (ret) {
            mpp_err_f("group free buffer failed\n");
            return ret;
        }
    }

    if (JpegHalCtx->regs) {
        mpp_free(JpegHalCtx->regs);
        JpegHalCtx->regs = NULL;
    }

    JpegHalCtx->set_output_fmt_flag = 0;
    JpegHalCtx->hal_debug_enable = 0;
    JpegHalCtx->frame_count = 0;
    JpegHalCtx->output_yuv_count = 0;

    jpegd_dbg_func("exit\n");
    return MPP_OK;
}

MPP_RET hal_jpegd_vdpu2_gen_regs(void *hal,  HalTaskInfo *syn)
{
    jpegd_dbg_func("enter\n");
    if (NULL == hal || NULL == syn) {
        mpp_err_f("NULL pointer");
        return MPP_ERR_NULL_PTR;
    }

    MPP_RET ret = MPP_OK;
    JpegdHalCtx *JpegHalCtx = (JpegdHalCtx *)hal;
    JpegdSyntax *syntax = (JpegdSyntax *)syn->dec.syntax.data;
    MppBuffer streambuf = NULL;
    MppBuffer outputBuf = NULL;

    if (syn->dec.flags.parse_err)
        goto RET;

    ret = jpeg_image_check_size(syntax->hor_stride, syntax->ver_stride);
    if (ret)
        goto RET;

    if (syn->dec.valid) {
        ret = jpegd_setup_output_fmt(JpegHalCtx, syntax, syn->dec.output);
        if (ret) {
            mpp_err_f("setup output format %x failed\n", syntax->output_fmt);
            goto RET;
        }

        /* input stream address */
        mpp_buf_slot_get_prop(JpegHalCtx->packet_slots, syn->dec.input,
                              SLOT_BUFFER, &streambuf);
        JpegHalCtx->pkt_fd = mpp_buffer_get_fd(streambuf);
        if (JpegHalCtx->pkt_fd <= 0) {
            mpp_err_f("get pkt_fd failed\n");
            goto RET;
        }
        syntax->pkt_len = jpegd_vdpu_tail_0xFF_patch(streambuf, syntax->pkt_len);

        /* output picture address */
        mpp_buf_slot_get_prop(JpegHalCtx->frame_slots, syn->dec.output,
                              SLOT_BUFFER, &outputBuf);
        JpegHalCtx->frame_fd = mpp_buffer_get_fd(outputBuf);
        if (JpegHalCtx->frame_fd <= 0) {
            mpp_err_f("get frame_fd failed\n");
            goto RET;
        }

        ret = jpegd_gen_regs(JpegHalCtx, syntax);
        mpp_buffer_sync_end(streambuf);
        mpp_buffer_sync_end(JpegHalCtx->pTableBase);
        if (ret != MPP_OK) {
            mpp_err_f("generate registers failed\n");
            goto RET;
        }
    }

    return ret;
RET:
    syn->dec.valid = 0;
    syn->dec.flags.parse_err = 1;

    jpegd_dbg_func("exit\n");
    return ret;
}

MPP_RET hal_jpegd_vdpu2_start(void *hal, HalTaskInfo *task)
{
    MPP_RET ret = MPP_OK;
    JpegdHalCtx *JpegHalCtx = (JpegdHalCtx *)hal;
    RK_U32 *regs = (RK_U32 *)JpegHalCtx->regs;

    jpegd_dbg_func("enter\n");
    if (task->dec.flags.parse_err)
        goto __RETURN;

    do {
        MppDevRegWrCfg wr_cfg;
        MppDevRegRdCfg rd_cfg;
        RK_U32 reg_size = mpp_get_ioctl_version() ?
                          sizeof(((JpegdIocRegInfo *)0)->regs) :
                          sizeof(JpegdIocRegInfo) - EXTRA_INFO_SIZE;

        wr_cfg.reg = regs;
        wr_cfg.size = reg_size;
        wr_cfg.offset = 0;

        ret = mpp_dev_ioctl(JpegHalCtx->dev, MPP_DEV_REG_WR, &wr_cfg);
        if (ret) {
            mpp_err_f("set register write failed %d\n", ret);
            goto __RETURN;
        }

        reg_size = sizeof(((JpegdIocRegInfo *)0)->regs);

        rd_cfg.reg = regs;
        rd_cfg.size = reg_size;
        rd_cfg.offset = 0;

        ret = mpp_dev_ioctl(JpegHalCtx->dev, MPP_DEV_REG_RD, &rd_cfg);
        if (ret) {
            mpp_err_f("set register read failed %d\n", ret);
            goto __RETURN;
        }

        ret = mpp_dev_ioctl(JpegHalCtx->dev, MPP_DEV_CMD_SEND, NULL);
        if (ret) {
            mpp_err_f("send cmd failed %d\n", ret);
            goto __RETURN;
        }
    } while (0);

    jpegd_dbg_func("exit\n");
    return ret;

__RETURN:
    task->dec.flags.parse_err = 1;
    jpegd_dbg_func("exit\n");
    return ret;
}

MPP_RET hal_jpegd_vdpu2_wait(void *hal, HalTaskInfo *task)
{
    MPP_RET ret = MPP_OK;
    JpegdHalCtx *JpegHalCtx = (JpegdHalCtx *)hal;
    JpegRegSet *reg_out = JpegHalCtx->regs;
    RK_U32 errinfo = 1;

    jpegd_dbg_func("enter\n");
    if (task->dec.flags.parse_err)
        goto __SKIP_HARD;

    ret = mpp_dev_ioctl(JpegHalCtx->dev, MPP_DEV_CMD_POLL, NULL);
    if (ret) {
        task->dec.flags.parse_err = 1;
        mpp_err_f("poll cmd failed %d\n", ret);
    }

__SKIP_HARD:
    if (JpegHalCtx->dec_cb) {
        DecCbHalDone param;

        param.task = (void *)&task->dec;
        param.regs = (RK_U32 *)reg_out;
        if (reg_out->reg55_Interrupt.sw_dec_bus_int) {
            mpp_err_f("IRQ BUS ERROR!");
        } else if (reg_out->reg55_Interrupt.sw_dec_error_int) {
            mpp_err_f("IRQ STREAM ERROR!");
        } else if (reg_out->reg55_Interrupt.sw_dec_timeout) {
            mpp_err_f("IRQ TIMEOUT!");
        } else if (reg_out->reg55_Interrupt.sw_dec_buffer_int) {
            mpp_err_f("IRQ BUFFER EMPTY!");
        } else if (reg_out->reg55_Interrupt.sw_dec_irq) {
            errinfo = 0;
            jpegd_dbg_result("DECODE SUCCESS!");
        }
        param.hard_err = errinfo;
        mpp_callback(JpegHalCtx->dec_cb, &param);
    }

    /* debug information */
    if (jpegd_debug & JPEGD_DBG_IO) {
        static FILE *jpg_file;
        static char name[64];
        MppBuffer outputBuf = NULL;
        void *base = NULL;
        mpp_buf_slot_get_prop(JpegHalCtx->frame_slots, task->dec.output,
                              SLOT_BUFFER, &outputBuf);
        base = mpp_buffer_get_ptr(outputBuf);

        snprintf(name, sizeof(name) - 1, "/data/tmp/output%02d.yuv",
                 JpegHalCtx->output_yuv_count);
        jpg_file = fopen(name, "wb+");
        if (jpg_file) {
            JpegdSyntax *syntax = (JpegdSyntax *)task->dec.syntax.data;
            RK_U32 width = syntax->hor_stride;
            RK_U32 height = syntax->ver_stride;

            fwrite(base, width * height * 3 / 2, 1, jpg_file);
            jpegd_dbg_io("frame_%02d output YUV(%d*%d) saving to %s\n",
                         JpegHalCtx->output_yuv_count,
                         width, height, name);
            fclose(jpg_file);
            JpegHalCtx->output_yuv_count++;
        }
    }

    memset(&reg_out->reg55_Interrupt, 0, sizeof(RK_U32));

    jpegd_dbg_func("exit\n");
    return ret;
}

MPP_RET hal_jpegd_vdpu2_reset(void *hal)
{
    jpegd_dbg_func("enter\n");
    MPP_RET ret = MPP_OK;
    JpegdHalCtx *JpegHalCtx = (JpegdHalCtx *)hal;
    (void)JpegHalCtx;

    jpegd_dbg_func("exit\n");
    return ret;
}

MPP_RET hal_jpegd_vdpu2_flush(void *hal)
{
    jpegd_dbg_func("enter\n");
    MPP_RET ret = MPP_OK;

    (void)hal;
    return ret;
}

MPP_RET hal_jpegd_vdpu2_control(void *hal, MpiCmd cmd_type,
                                void *param)
{
    jpegd_dbg_func("enter\n");
    MPP_RET ret = MPP_OK;
    JpegdHalCtx *JpegHalCtx = (JpegdHalCtx *)hal;
    if (NULL == JpegHalCtx) {
        mpp_err_f("NULL pointer");
        return MPP_ERR_NULL_PTR;
    }

    switch (cmd_type) {
    case MPP_DEC_SET_OUTPUT_FORMAT: {
        JpegHalCtx->output_fmt = *((MppFrameFormat *)param);
        JpegHalCtx->set_output_fmt_flag = 1;
        jpegd_dbg_hal("output_format: 0x%x\n", JpegHalCtx->output_fmt);

        if (!MPP_FRAME_FMT_IS_YUV(JpegHalCtx->output_fmt) && !MPP_FRAME_FMT_IS_RGB(JpegHalCtx->output_fmt)) {
            mpp_err_f("invalid output format 0x%x\n", JpegHalCtx->output_fmt);
            ret = MPP_ERR_VALUE;
        }
    } break;
    default :
        break;
    }

    jpegd_dbg_func("exit ret %d\n", ret);
    return  ret;
}
