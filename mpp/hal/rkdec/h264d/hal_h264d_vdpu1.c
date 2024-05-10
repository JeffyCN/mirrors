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

#define MODULE_TAG "hal_h264d_vdpu1_reg"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rk_type.h"
#include "mpp_err.h"
#include "mpp_mem.h"
#include "mpp_common.h"

#include "hal_h264d_global.h"
#include "hal_h264d_api.h"
#include "hal_h264d_vdpu_com.h"
#include "hal_h264d_vdpu1.h"
#include "hal_h264d_vdpu1_reg.h"
#include "mpp_dec_cb_param.h"

const RK_U32 vdpu1_ref_idx[16] = {
    14, 15, 16, 17, 18, 19, 20, 21,
    22, 23, 24, 25, 26, 27, 28, 29
};

MPP_RET vdpu1_h264d_deinit(void *hal);
static MPP_RET vdpu1_set_refer_pic_idx(H264dVdpu1Regs_t *p_regs, RK_U32 i,
                                       RK_U16 val)
{
    switch (i) {
    case 0:
        p_regs->SwReg30.sw_refer0_nbr = val;
        break;
    case 1:
        p_regs->SwReg30.sw_refer1_nbr = val;
        break;
    case 2:
        p_regs->SwReg31.sw_refer2_nbr = val;
        break;
    case 3:
        p_regs->SwReg31.sw_refer3_nbr = val;
        break;
    case 4:
        p_regs->SwReg32.sw_refer4_nbr = val;
        break;
    case 5:
        p_regs->SwReg32.sw_refer5_nbr = val;
        break;
    case 6:
        p_regs->SwReg33.sw_refer6_nbr = val;
        break;
    case 7:
        p_regs->SwReg33.sw_refer7_nbr = val;
        break;
    case 8:
        p_regs->SwReg34.sw_refer8_nbr = val;
        break;
    case 9:
        p_regs->SwReg34.sw_refer9_nbr = val;
        break;
    case 10:
        p_regs->SwReg35.sw_refer10_nbr = val;
        break;
    case 11:
        p_regs->SwReg35.sw_refer11_nbr = val;
        break;
    case 12:
        p_regs->SwReg36.sw_refer12_nbr = val;
        break;
    case 13:
        p_regs->SwReg36.sw_refer13_nbr = val;
        break;
    case 14:
        p_regs->SwReg37.sw_refer14_nbr = val;
        break;
    case 15:
        p_regs->SwReg37.sw_refer15_nbr = val;
        break;
    default:
        break;
    }

    return MPP_OK;
}

static MPP_RET vdpu1_set_refer_pic_list_p(H264dVdpu1Regs_t *p_regs, RK_U32 i,
                                          RK_U16 val)
{
    switch (i) {
    case 0:
        p_regs->SwReg47.sw_pinit_rlist_f0 = val;
        break;
    case 1:
        p_regs->SwReg47.sw_pinit_rlist_f1 = val;
        break;
    case 2:
        p_regs->SwReg47.sw_pinit_rlist_f2 = val;
        break;
    case 3:
        p_regs->SwReg47.sw_pinit_rlist_f3 = val;
        break;
    case 4:
        p_regs->SwReg10.sw_pinit_rlist_f4 = val;
        break;
    case 5:
        p_regs->SwReg10.sw_pinit_rlist_f5 = val;
        break;
    case 6:
        p_regs->SwReg10.sw_pinit_rlist_f6 = val;
        break;
    case 7:
        p_regs->SwReg10.sw_pinit_rlist_f7 = val;
        break;
    case 8:
        p_regs->SwReg10.sw_pinit_rlist_f8 = val;
        break;
    case 9:
        p_regs->SwReg10.sw_pinit_rlist_f9 = val;
        break;
    case 10:
        p_regs->SwReg11.sw_pinit_rlist_f10 = val;
        break;
    case 11:
        p_regs->SwReg11.sw_pinit_rlist_f11 = val;
        break;
    case 12:
        p_regs->SwReg11.sw_pinit_rlist_f12 = val;
        break;
    case 13:
        p_regs->SwReg11.sw_pinit_rlist_f13 = val;
        break;
    case 14:
        p_regs->SwReg11.sw_pinit_rlist_f14 = val;
        break;
    case 15:
        p_regs->SwReg11.sw_pinit_rlist_f15 = val;
        break;
    default:
        break;
    }

    return MPP_OK;
}

static MPP_RET vdpu1_set_refer_pic_list_b0(H264dVdpu1Regs_t *p_regs, RK_U32 i,
                                           RK_U16 val)
{
    switch (i) {
    case 0:
        p_regs->SwReg42.sw_binit_rlist_f0 = val;
        break;
    case 1:
        p_regs->SwReg42.sw_binit_rlist_f1 = val;
        break;
    case 2:
        p_regs->SwReg42.sw_binit_rlist_f2 = val;
        break;
    case 3:
        p_regs->SwReg43.sw_binit_rlist_f3 = val;
        break;
    case 4:
        p_regs->SwReg43.sw_binit_rlist_f4 = val;
        break;
    case 5:
        p_regs->SwReg43.sw_binit_rlist_f5 = val;
        break;
    case 6:
        p_regs->SwReg44.sw_binit_rlist_f6 = val;
        break;
    case 7:
        p_regs->SwReg44.sw_binit_rlist_f7 = val;
        break;
    case 8:
        p_regs->SwReg44.sw_binit_rlist_f8 = val;
        break;
    case 9:
        p_regs->SwReg45.sw_binit_rlist_f9 = val;
        break;
    case 10:
        p_regs->SwReg45.sw_binit_rlist_f10 = val;
        break;
    case 11:
        p_regs->SwReg45.sw_binit_rlist_f11 = val;
        break;
    case 12:
        p_regs->SwReg46.sw_binit_rlist_f12 = val;
        break;
    case 13:
        p_regs->SwReg46.sw_binit_rlist_f13 = val;
        break;
    case 14:
        p_regs->SwReg46.sw_binit_rlist_f14 = val;
        break;
    case 15:
        p_regs->SwReg47.sw_binit_rlist_f15 = val;
        break;
    default:
        break;
    }

    return MPP_OK;
}

static MPP_RET vdpu1_set_refer_pic_list_b1(H264dVdpu1Regs_t *p_regs, RK_U32 i,
                                           RK_U16 val)
{
    switch (i) {
    case 0:
        p_regs->SwReg42.sw_binit_rlist_b0 = val;
        break;
    case 1:
        p_regs->SwReg42.sw_binit_rlist_b1 = val;
        break;
    case 2:
        p_regs->SwReg42.sw_binit_rlist_b2 = val;
        break;
    case 3:
        p_regs->SwReg43.sw_binit_rlist_b3 = val;
        break;
    case 4:
        p_regs->SwReg43.sw_binit_rlist_b4 = val;
        break;
    case 5:
        p_regs->SwReg43.sw_binit_rlist_b5 = val;
        break;
    case 6:
        p_regs->SwReg44.sw_binit_rlist_b6 = val;
        break;
    case 7:
        p_regs->SwReg44.sw_binit_rlist_b7 = val;
        break;
    case 8:
        p_regs->SwReg44.sw_binit_rlist_b8 = val;
        break;
    case 9:
        p_regs->SwReg45.sw_binit_rlist_b9 = val;
        break;
    case 10:
        p_regs->SwReg45.sw_binit_rlist_b10 = val;
        break;
    case 11:
        p_regs->SwReg45.sw_binit_rlist_b11 = val;
        break;
    case 12:
        p_regs->SwReg46.sw_binit_rlist_b12 = val;
        break;
    case 13:
        p_regs->SwReg46.sw_binit_rlist_b13 = val;
        break;
    case 14:
        p_regs->SwReg46.sw_binit_rlist_b14 = val;
        break;
    case 15:
        p_regs->SwReg47.sw_binit_rlist_b15 = val;
        break;
    default:
        break;
    }

    return MPP_OK;
}

static MPP_RET vdpu1_set_refer_pic_base_addr(H264dVdpu1Regs_t *p_regs, RK_U32 i,
                                             RK_U32 val)
{
    switch (i) {
    case 0:
        p_regs->SwReg14.sw_refer0_base = val;
        break;
    case 1:
        p_regs->SwReg15.sw_refer1_base = val;
        break;
    case 2:
        p_regs->SwReg16.sw_refer2_base = val;
        break;
    case 3:
        p_regs->SwReg17.sw_refer3_base = val;
        break;
    case 4:
        p_regs->SwReg18.sw_refer4_base = val;
        break;
    case 5:
        p_regs->SwReg19.sw_refer5_base = val;
        break;
    case 6:
        p_regs->SwReg20.sw_refer6_base = val;
        break;
    case 7:
        p_regs->SwReg21.sw_refer7_base = val;
        break;
    case 8:
        p_regs->SwReg22.sw_refer8_base = val;
        break;
    case 9:
        p_regs->SwReg23.sw_refer9_base = val;
        break;
    case 10:
        p_regs->SwReg24.sw_refer10_base = val;
        break;
    case 11:
        p_regs->SwReg25.sw_refer11_base = val;
        break;
    case 12:
        p_regs->SwReg26.sw_refer12_base = val;
        break;
    case 13:
        p_regs->SwReg27.sw_refer13_base = val;
        break;
    case 14:
        p_regs->SwReg28.sw_refer14_base = val;
        break;
    case 15:
        p_regs->SwReg29.sw_refer15_base = val;
        break;
    default:
        break;
    }
    return MPP_OK;
}

static MPP_RET vdpu1_set_pic_regs(H264dHalCtx_t *p_hal,
                                  H264dVdpu1Regs_t *p_regs)
{
    MPP_RET ret = MPP_ERR_UNKNOW;

    p_regs->SwReg04.sw_pic_mb_width = p_hal->pp->wFrameWidthInMbsMinus1 + 1;
    p_regs->SwReg04.sw_pic_mb_height_p = (2 - p_hal->pp->frame_mbs_only_flag)
                                         * (p_hal->pp->wFrameHeightInMbsMinus1 + 1);

    return ret = MPP_OK;
}

static MPP_RET vdpu1_set_vlc_regs(H264dHalCtx_t *p_hal,
                                  H264dVdpu1Regs_t *p_regs)
{
    RK_U32 i = 0;
    MPP_RET ret = MPP_ERR_UNKNOW;
    DXVA_PicParams_H264_MVC *pp = p_hal->pp;

    p_regs->SwReg03.sw_dec_out_dis = 0;
    p_regs->SwReg03.sw_rlc_mode_e = 0;
    p_regs->SwReg06.sw_init_qp = pp->pic_init_qp_minus26 + 26;
    p_regs->SwReg09.sw_refidx0_active = pp->num_ref_idx_l0_active_minus1 + 1;
    p_regs->SwReg04.sw_ref_frames = pp->num_ref_frames;

    p_regs->SwReg07.sw_framenum_len = pp->log2_max_frame_num_minus4 + 4;
    p_regs->SwReg07.sw_framenum = pp->frame_num;

    p_regs->SwReg08.sw_const_intra_e = pp->constrained_intra_pred_flag;
    p_regs->SwReg08.sw_filt_ctrl_pres =
        pp->deblocking_filter_control_present_flag;
    p_regs->SwReg08.sw_rdpic_cnt_pres = pp->redundant_pic_cnt_present_flag;
    p_regs->SwReg08.sw_refpic_mk_len = p_hal->slice_long[0].drpm_used_bitlen;
    p_regs->SwReg08.sw_idr_pic_e = p_hal->slice_long[0].idr_flag;
    p_regs->SwReg08.sw_idr_pic_id = p_hal->slice_long[0].idr_pic_id;

    p_regs->SwReg09.sw_pps_id = p_hal->slice_long[0].active_pps_id;
    p_regs->SwReg09.sw_poc_length = p_hal->slice_long[0].poc_used_bitlen;

    /* reference picture flags, TODO separate fields */
    if (pp->field_pic_flag) {
        RK_U32 validTmp = 0, validFlags = 0;
        RK_U32 longTermTmp = 0, longTermflags = 0;
        for (i = 0; i < 32; i++) {
            if (pp->RefFrameList[i / 2].bPicEntry == 0xff) { //!< invalid
                longTermflags <<= 1;
                validFlags <<= 1;
            } else {
                longTermTmp = pp->RefFrameList[i / 2].AssociatedFlag; //!< get long term flag
                longTermflags = (longTermflags << 1) | longTermTmp;

                validTmp = ((pp->UsedForReferenceFlags >> i) & 0x01);
                validFlags = (validFlags << 1) | validTmp;
            }
        }
        p_regs->SwReg38.refpic_term_flag = longTermflags;
        p_regs->SwReg39.refpic_valid_flag = validFlags;
    } else {
        RK_U32 validTmp = 0, validFlags = 0;
        RK_U32 longTermTmp = 0, longTermflags = 0;
        for (i = 0; i < 16; i++) {
            if (pp->RefFrameList[i].bPicEntry == 0xff) {  //!< invalid
                longTermflags <<= 1;
                validFlags <<= 1;
            } else {
                longTermTmp = pp->RefFrameList[i].AssociatedFlag;
                longTermflags = (longTermflags << 1) | longTermTmp;
                validTmp = ((pp->UsedForReferenceFlags >> (2 * i)) & 0x03) > 0;
                validFlags = (validFlags << 1) | validTmp;
            }
        }
        p_regs->SwReg38.refpic_term_flag = (longTermflags << 16);
        p_regs->SwReg39.refpic_valid_flag = (validFlags << 16);
    }

    for (i = 0; i < 16; i++) {
        if (pp->RefFrameList[i].bPicEntry != 0xff) { //!< valid
            if (pp->RefFrameList[i].AssociatedFlag) { //!< longterm flag
                vdpu1_set_refer_pic_idx(p_regs, i, pp->LongTermPicNumList[i]); //!< pic_num
            } else {
                vdpu1_set_refer_pic_idx(p_regs, i, pp->FrameNumList[i]); //< frame_num
            }
        }
    }
    p_regs->SwReg03.sw_picord_count_e = 1;
    //!< set poc to buffer
    {
        H264dVdpuRegCtx_t *reg_ctx = (H264dVdpuRegCtx_t *)p_hal->reg_ctx;
        RK_U32 *pocBase = (RK_U32 *)reg_ctx->poc_ptr;

        //!< set reference reorder poc
        for (i = 0; i < 32; i++) {
            if (pp->RefFrameList[i / 2].bPicEntry != 0xff) {
                *pocBase++ = pp->FieldOrderCntList[i / 2][i & 0x1];
            } else {
                *pocBase++ = 0;
            }
        }

        //!< set current poc
        if (pp->field_pic_flag || !pp->MbaffFrameFlag) {
            if (pp->field_pic_flag)
                *pocBase++ = pp->CurrFieldOrderCnt[pp->CurrPic.AssociatedFlag ? 1 : 0];
            else
                *pocBase++ = MPP_MIN(pp->CurrFieldOrderCnt[0], pp->CurrFieldOrderCnt[1]);
        } else {
            *pocBase++ = pp->CurrFieldOrderCnt[0];
            *pocBase++ = pp->CurrFieldOrderCnt[1];
        }
    }

    p_regs->SwReg07.sw_cabac_e = pp->entropy_coding_mode_flag;

    //!< stream position update
    {
        MppBuffer bitstream_buf = NULL;
        p_regs->SwReg06.sw_start_code_e = 1;

        mpp_buf_slot_get_prop(p_hal->packet_slots, p_hal->in_task->input,
                              SLOT_BUFFER, &bitstream_buf);

        p_regs->SwReg05.sw_strm_start_bit = 0; /* sodb stream start bit */
        p_regs->SwReg12.rlc_vlc_st_adr = mpp_buffer_get_fd(bitstream_buf);

        p_regs->SwReg06.sw_stream_len = p_hal->strm_len;
    }

    return ret = MPP_OK;
}

static MPP_RET vdpu1_set_ref_regs(H264dHalCtx_t *p_hal,
                                  H264dVdpu1Regs_t *p_regs)
{
    MPP_RET ret = MPP_ERR_UNKNOW;
    RK_U32 i = 0;
    RK_U32 num_refs = 0;
    RK_U32 num_reorder = 0;
    H264dRefsList_t m_lists[3][16];
    DXVA_PicParams_H264_MVC  *pp = p_hal->pp;
    RK_U32 max_frame_num = 1 << (pp->log2_max_frame_num_minus4 + 4);

    // init list
    memset(m_lists, 0, sizeof(m_lists));
    for (i = 0; i < 16; i++) {
        RK_U32 ref_flag = pp->UsedForReferenceFlags >> (2 * i) & 0x3;

        m_lists[0][i].idx = i;
        if (ref_flag) {
            num_refs++;
            m_lists[0][i].cur_poc = pp->CurrPic.AssociatedFlag
                                    ? pp->CurrFieldOrderCnt[1] : pp->CurrFieldOrderCnt[0];
            m_lists[0][i].ref_flag = ref_flag;
            m_lists[0][i].lt_flag = pp->RefFrameList[i].AssociatedFlag;
            if (m_lists[0][i].lt_flag) {
                m_lists[0][i].ref_picnum = pp->LongTermPicNumList[i];
            } else {
                m_lists[0][i].ref_picnum = pp->FrameNumList[i] > pp->frame_num ?
                                           (pp->FrameNumList[i] - max_frame_num) :
                                           pp->FrameNumList[i];
            }

            if (ref_flag == 3) {
                m_lists[0][i].ref_poc = MPP_MIN(pp->FieldOrderCntList[i][0], pp->FieldOrderCntList[i][1]);
            } else if (ref_flag & 0x1) {
                m_lists[0][i].ref_poc = pp->FieldOrderCntList[i][0];
            } else if (ref_flag & 0x2) {
                m_lists[0][i].ref_poc = pp->FieldOrderCntList[i][1];
            }
            num_reorder = i + 1;
        }
    }
    /*
     * the value of num_reorder may be greater than num_refs,
     * e.g. v: valid  x: invalid
     *      num_refs = 3, num_reorder = 4
     *      the index 1 will be reorder to the end
     *   ┌─┬─┬─┬─┬─┬─┬─┐
     *   │0│1│2│3│.│.│F│
     *   ├─┼─┼─┼─┼─┼─┼─┤
     *   │v│x│v│v│x│x│x│
     *   └─┴─┴─┴─┴─┴─┴─┘
     */
    memcpy(m_lists[1], m_lists[0], sizeof(m_lists[0]));
    memcpy(m_lists[2], m_lists[0], sizeof(m_lists[0]));
    qsort(m_lists[0], num_reorder, sizeof(m_lists[0][0]), compare_p);
    qsort(m_lists[1], num_reorder, sizeof(m_lists[1][0]), compare_b0);
    qsort(m_lists[2], num_reorder, sizeof(m_lists[2][0]), compare_b1);
    if (num_refs > 1 && !p_hal->pp->field_pic_flag) {
        if (!memcmp(m_lists[1], m_lists[2], sizeof(m_lists[1]))) {
            MPP_SWAP(H264dRefsList_t, m_lists[2][0], m_lists[2][1]);
        }
    }
    //!< list0 list1 listP
    for (i = 0; i < 16; i++) {
        vdpu1_set_refer_pic_list_p(p_regs, i, m_lists[0][i].idx);
        vdpu1_set_refer_pic_list_b0(p_regs, i, m_lists[1][i].idx);
        vdpu1_set_refer_pic_list_b1(p_regs, i, m_lists[2][i].idx);
    }

    return ret = MPP_OK;
}

static MPP_RET vdpu1_set_asic_regs(H264dHalCtx_t *p_hal,
                                   H264dVdpu1Regs_t *p_regs)
{
    RK_U32 i = 0, j = 0;
    RK_U32 outPhyAddr = 0;
    MppBuffer frame_buf = NULL;
    MPP_RET ret = MPP_ERR_UNKNOW;
    DXVA_PicParams_H264_MVC *pp = p_hal->pp;
    DXVA_Slice_H264_Long *p_long = &p_hal->slice_long[0];

    /* reference picture physic address */
    for (i = 0, j = 0xff; i < MPP_ARRAY_ELEMS(pp->RefFrameList); i++) {
        RK_U32 val = 0;
        RK_U32 top_closer = 0;
        RK_U32 field_flag = 0;
        RK_S32 cur_poc = 0;
        RK_U32 used_flag = 0;

        if (pp->RefFrameList[i].bPicEntry != 0xff) {
            mpp_buf_slot_get_prop(p_hal->frame_slots,
                                  pp->RefFrameList[i].Index7Bits,
                                  SLOT_BUFFER, &frame_buf); //!< reference phy addr
            j = i;
        } else {
            mpp_buf_slot_get_prop(p_hal->frame_slots,
                                  pp->CurrPic.Index7Bits,
                                  SLOT_BUFFER, &frame_buf); //!< current out phy addr
        }

        field_flag = ((pp->RefPicFiledFlags >> i) & 0x1) ? 0x2 : 0;
        cur_poc = pp->CurrPic.AssociatedFlag
                  ? pp->CurrFieldOrderCnt[1] : pp->CurrFieldOrderCnt[0];
        used_flag = ((pp->UsedForReferenceFlags >> (2 * i)) & 0x3);
        if (used_flag & 0x3) {
            top_closer = MPP_ABS(pp->FieldOrderCntList[i][0] - cur_poc) <
                         MPP_ABS(pp->FieldOrderCntList[i][1] - cur_poc) ? 0x1 : 0;
        } else if (used_flag & 0x2) {
            top_closer = 0;
        } else if (used_flag & 0x1) {
            top_closer = 1;
        }
        val = top_closer | field_flag;
        if (val)
            mpp_dev_set_reg_offset(p_hal->dev, vdpu1_ref_idx[i], val);
        vdpu1_set_refer_pic_base_addr(p_regs, i, mpp_buffer_get_fd(frame_buf));
    }

    /* inter-view reference picture */
    {
        H264dVdpuPriv_t *priv = (H264dVdpuPriv_t *)p_hal->priv;
        if (pp->curr_layer_id && priv->ilt_dpb && priv->ilt_dpb->valid /*pp->inter_view_flag*/) {
            mpp_buf_slot_get_prop(p_hal->frame_slots,
                                  priv->ilt_dpb->slot_index,
                                  SLOT_BUFFER, &frame_buf);
            p_regs->SwReg29.sw_refer15_base = mpp_buffer_get_fd(frame_buf); //!< inter-view base, ref15
            p_regs->SwReg39.refpic_valid_flag |=
                (pp->field_pic_flag ? 0x3 : 0x10000);
        }
    }

    p_regs->SwReg03.sw_pic_fixed_quant = pp->curr_layer_id; //!< VDPU_MVC_E
    p_regs->SwReg03.sw_filtering_dis = 0;

    mpp_buf_slot_get_prop(p_hal->frame_slots,
                          pp->CurrPic.Index7Bits,
                          SLOT_BUFFER, &frame_buf); //!< current out phy addr
    outPhyAddr = mpp_buffer_get_fd(frame_buf);
    if (pp->field_pic_flag && pp->CurrPic.AssociatedFlag) {
        mpp_dev_set_reg_offset(p_hal->dev, 13, ((pp->wFrameWidthInMbsMinus1 + 1) * 16));
    }
    p_regs->SwReg13.dec_out_st_adr = outPhyAddr; //!< outPhyAddr, pp->CurrPic.Index7Bits

    p_regs->SwReg05.sw_ch_qp_offset = pp->chroma_qp_index_offset;
    p_regs->SwReg05.sw_ch_qp_offset2 = pp->second_chroma_qp_index_offset;

    /* set default value for register[41] to avoid illegal translation fd */
    {
        RK_U32 dirMvOffset = 0;
        RK_U32 picSizeInMbs = 0;

        picSizeInMbs = p_hal->pp->wFrameWidthInMbsMinus1 + 1;
        picSizeInMbs = picSizeInMbs * (2 - pp->frame_mbs_only_flag)
                       * (pp->wFrameHeightInMbsMinus1 + 1);
        dirMvOffset = picSizeInMbs
                      * ((p_hal->pp->chroma_format_idc == 0) ? 256 : 384);
        dirMvOffset += (pp->field_pic_flag && pp->CurrPic.AssociatedFlag)
                       ? (picSizeInMbs * 32) : 0;
        if (dirMvOffset) {
            RK_U32 offset = mpp_get_ioctl_version() ? dirMvOffset : dirMvOffset >> 4;
            mpp_dev_set_reg_offset(p_hal->dev, 41, offset);
        }
        p_regs->SwReg41.dmmv_st_adr = mpp_buffer_get_fd(frame_buf);
    }

    p_regs->SwReg03.sw_write_mvs_e = (p_long->nal_ref_idc != 0) ? 1 : 0; /* defalut set 1 */
    p_regs->SwReg07.sw_dir_8x8_infer_e = pp->direct_8x8_inference_flag;
    p_regs->SwReg07.sw_weight_pred_e = pp->weighted_pred_flag;
    p_regs->SwReg07.sw_weight_bipr_idc = pp->weighted_bipred_idc;
    p_regs->SwReg09.sw_refidx1_active = (pp->num_ref_idx_l1_active_minus1 + 1);
    p_regs->SwReg05.sw_fieldpic_flag_e = (!pp->frame_mbs_only_flag) ? 1 : 0;

    p_regs->SwReg03.sw_pic_interlace_e =
        (!pp->frame_mbs_only_flag
         && (pp->MbaffFrameFlag || pp->field_pic_flag)) ? 1 : 0;
    p_regs->SwReg03.sw_pic_fieldmode_e = pp->field_pic_flag;
    p_regs->SwReg03.sw_pic_topfield_e = (!pp->CurrPic.AssociatedFlag) ? 1 : 0; /* bottomFieldFlag */
    p_regs->SwReg03.sw_seq_mbaff_e = pp->MbaffFrameFlag;
    p_regs->SwReg08.sw_8x8trans_flag_e = pp->transform_8x8_mode_flag;
    p_regs->SwReg07.sw_blackwhite_e = (p_long->profileIdc >= 100
                                       && pp->chroma_format_idc == 0) ? 1 : 0;
    p_regs->SwReg05.sw_type1_quant_e = pp->scaleing_list_enable_flag;

    {
        H264dVdpuRegCtx_t *reg_ctx = (H264dVdpuRegCtx_t *)p_hal->reg_ctx;
        if (p_hal->pp->scaleing_list_enable_flag) {
            RK_U32 temp = 0;
            RK_U32 *ptr = (RK_U32 *)reg_ctx->sclst_ptr;

            for (i = 0; i < 6; i++) {
                for (j = 0; j < 4; j++) {
                    temp = (p_hal->qm->bScalingLists4x4[i][4 * j + 0] << 24) |
                           (p_hal->qm->bScalingLists4x4[i][4 * j + 1] << 16) |
                           (p_hal->qm->bScalingLists4x4[i][4 * j + 2] << 8) |
                           (p_hal->qm->bScalingLists4x4[i][4 * j + 3]);
                    *ptr++ = temp;
                }
            }

            for (i = 0; i < 2; i++) {
                for (j = 0; j < 16; j++) {
                    temp = (p_hal->qm->bScalingLists8x8[i][4 * j + 0] << 24) |
                           (p_hal->qm->bScalingLists8x8[i][4 * j + 1] << 16) |
                           (p_hal->qm->bScalingLists8x8[i][4 * j + 2] << 8) |
                           (p_hal->qm->bScalingLists8x8[i][4 * j + 3]);
                    *ptr++ = temp;
                }
            }
        }
        p_regs->SwReg40.qtable_st_adr = mpp_buffer_get_fd(reg_ctx->buf);
    }

    p_regs->SwReg03.sw_dec_out_dis = 0; /* set defalut 0 */
    p_regs->SwReg06.sw_ch_8pix_ileav_e = 0;
    p_regs->SwReg01.sw_dec_en = 1;

    return ret = MPP_OK;
}

static MPP_RET vdpu1_set_device_regs(H264dHalCtx_t *p_hal,
                                     H264dVdpu1Regs_t *p_reg)
{
    MPP_RET ret = MPP_ERR_UNKNOW;

    p_reg->SwReg03.sw_dec_mode = 0; /* set H264 mode */
    p_reg->SwReg02.sw_dec_out_endian = 1;  /* little endian */
    p_reg->SwReg02.sw_dec_in_endian = 0;  /* big endian */
    p_reg->SwReg02.sw_dec_strendian_e = 1; //!< little endian
    p_reg->SwReg02.sw_tiled_mode_msb = 0; /* 0: raster scan  1: tiled */

    /* bus_burst_length = 16, bus burst */
    p_reg->SwReg02.sw_dec_max_burst = 16; /* (0, 4, 8, 16) choice one */
    p_reg->SwReg02.sw_dec_scmd_dis = 0; /* disable */
    p_reg->SwReg02.sw_dec_adv_pre_dis = 0; /* disable */
    p_reg->SwReg55.sw_apf_threshold = 8;
    p_reg->SwReg02.sw_dec_latency = 0; /* compensation for bus latency; values up to 63 */
    p_reg->SwReg02.sw_dec_data_disc_e = 0;
    p_reg->SwReg02.sw_dec_out_endian = 1; /* little endian */
    p_reg->SwReg02.sw_dec_inswap32_e = 1; /* little endian */
    p_reg->SwReg02.sw_dec_outswap32_e = 1;
    p_reg->SwReg02.sw_dec_strswap32_e = 1;
    p_reg->SwReg02.sw_dec_strendian_e = 1; /* little endian */
    p_reg->SwReg02.sw_dec_timeout_e = 1;

    /* clock_gating  0:clock always on, 1: clock gating module control the key(turn off when decoder free) */
    p_reg->SwReg02.sw_dec_clk_gate_e = 1;
    p_reg->SwReg01.sw_dec_irq_dis_cfg = 0;

    //!< set AXI RW IDs
    p_reg->SwReg02.sw_dec_axi_rd_id = (0xFF & 0xFFU); /* 0-255 */
    p_reg->SwReg03.sw_dec_axi_wr_id = (0x00 & 0xFFU); /* 0-255 */

    ///!< Set prediction filter taps
    {
        RK_U32 val = 0;
        p_reg->SwReg49.sw_pred_bc_tap_0_0 = 1;

        val = (RK_U32)(-5);
        p_reg->SwReg49.sw_pred_bc_tap_0_1 = val;
        p_reg->SwReg49.sw_pred_bc_tap_0_2 = 20;
    }

    (void)p_hal;

    return ret = MPP_OK;
}

/*!
***********************************************************************
* \brief
*    init  VDPU granite decoder
***********************************************************************
*/
//extern "C"
MPP_RET vdpu1_h264d_init(void *hal, MppHalCfg *cfg)
{
    MPP_RET ret = MPP_ERR_UNKNOW;
    H264dHalCtx_t  *p_hal = (H264dHalCtx_t *)hal;
    INP_CHECK(ret, NULL == hal);
    (void) cfg;

    //!< malloc init registers
    MEM_CHECK(ret, p_hal->priv =
                  mpp_calloc_size(void, sizeof(H264dVdpuPriv_t)));

    MEM_CHECK(ret, p_hal->reg_ctx = mpp_calloc_size(void, sizeof(H264dVdpuRegCtx_t)));
    H264dVdpuRegCtx_t *reg_ctx = (H264dVdpuRegCtx_t *)p_hal->reg_ctx;
    //!< malloc buffers
    {
        RK_U32 i = 0;
        RK_U32 loop = p_hal->fast_mode ? MPP_ARRAY_ELEMS(reg_ctx->reg_buf) : 1;

        RK_U32 buf_size = VDPU_CABAC_TAB_SIZE +  VDPU_POC_BUF_SIZE + VDPU_SCALING_LIST_SIZE;
        for (i = 0; i < loop; i++) {
            FUN_CHECK(ret = mpp_buffer_get(p_hal->buf_group, &reg_ctx->reg_buf[i].buf,  buf_size));
            reg_ctx->reg_buf[i].cabac_ptr = mpp_buffer_get_ptr(reg_ctx->reg_buf[i].buf);
            reg_ctx->reg_buf[i].poc_ptr = reg_ctx->reg_buf[i].cabac_ptr + VDPU_CABAC_TAB_SIZE;
            reg_ctx->reg_buf[i].sclst_ptr = reg_ctx->reg_buf[i].poc_ptr + VDPU_POC_BUF_SIZE;
            reg_ctx->reg_buf[i].regs = mpp_calloc_size(void, sizeof(H264dVdpu1Regs_t));
            //!< copy cabac table bytes
            memcpy(reg_ctx->reg_buf[i].cabac_ptr, (void *)vdpu_cabac_table,  sizeof(vdpu_cabac_table));
        }
    }
    if (!p_hal->fast_mode) {
        reg_ctx->buf = reg_ctx->reg_buf[0].buf;
        reg_ctx->cabac_ptr = reg_ctx->reg_buf[0].cabac_ptr;
        reg_ctx->poc_ptr = reg_ctx->reg_buf[0].poc_ptr;
        reg_ctx->sclst_ptr = reg_ctx->reg_buf[0].sclst_ptr;
        reg_ctx->regs = reg_ctx->reg_buf[0].regs;
    }

    mpp_slots_set_prop(p_hal->frame_slots, SLOTS_HOR_ALIGN, vdpu_hor_align);
    mpp_slots_set_prop(p_hal->frame_slots, SLOTS_VER_ALIGN, vdpu_ver_align);

__RETURN:
    return MPP_OK;
__FAILED:
    vdpu1_h264d_deinit(hal);

    return ret;
}

/*!
***********************************************************************
* \brief
*    deinit
***********************************************************************
*/
//extern "C"
MPP_RET vdpu1_h264d_deinit(void *hal)
{
    H264dHalCtx_t  *p_hal = (H264dHalCtx_t *)hal;
    H264dVdpuRegCtx_t *reg_ctx = (H264dVdpuRegCtx_t *)p_hal->reg_ctx;

    RK_U32 i = 0;
    RK_U32 loop = p_hal->fast_mode ? MPP_ARRAY_ELEMS(reg_ctx->reg_buf) : 1;
    for (i = 0; i < loop; i++) {
        MPP_FREE(reg_ctx->reg_buf[i].regs);
        mpp_buffer_put(reg_ctx->reg_buf[i].buf);
    }
    MPP_FREE(p_hal->reg_ctx);
    MPP_FREE(p_hal->priv);

    return MPP_OK;
}

/*!
***********************************************************************
* \brief
*    generate register
***********************************************************************
*/
//extern "C"
MPP_RET vdpu1_h264d_gen_regs(void *hal, HalTaskInfo *task)
{
    MPP_RET ret = MPP_ERR_UNKNOW;

    H264dVdpuPriv_t *priv = NULL;
    H264dHalCtx_t *p_hal = (H264dHalCtx_t *)hal;
    INP_CHECK(ret, NULL == p_hal);
    p_hal->in_task = &task->dec;
    if (task->dec.flags.parse_err ||
        task->dec.flags.ref_err) {
        goto __RETURN;
    }
    priv = p_hal->priv;
    priv->layed_id = p_hal->pp->curr_layer_id;

    H264dVdpuRegCtx_t *reg_ctx = (H264dVdpuRegCtx_t *)p_hal->reg_ctx;
    if (p_hal->fast_mode) {
        RK_U32 i = 0;
        for (i = 0; i <  MPP_ARRAY_ELEMS(reg_ctx->reg_buf); i++) {
            if (!reg_ctx->reg_buf[i].valid) {
                task->dec.reg_index = i;
                reg_ctx->buf = reg_ctx->reg_buf[i].buf;
                reg_ctx->cabac_ptr = reg_ctx->reg_buf[i].cabac_ptr;
                reg_ctx->poc_ptr = reg_ctx->reg_buf[i].poc_ptr;
                reg_ctx->sclst_ptr = reg_ctx->reg_buf[i].sclst_ptr;
                reg_ctx->regs = reg_ctx->reg_buf[i].regs;
                reg_ctx->reg_buf[i].valid = 1;
                break;
            }
        }
    }

    FUN_CHECK(ret = adjust_input(priv, &p_hal->slice_long[0], p_hal->pp));
    FUN_CHECK(ret = vdpu1_set_device_regs(p_hal, (H264dVdpu1Regs_t *)reg_ctx->regs));
    FUN_CHECK(ret = vdpu1_set_pic_regs(p_hal, (H264dVdpu1Regs_t *)reg_ctx->regs));
    FUN_CHECK(ret = vdpu1_set_vlc_regs(p_hal, (H264dVdpu1Regs_t *)reg_ctx->regs));
    FUN_CHECK(ret = vdpu1_set_ref_regs(p_hal, (H264dVdpu1Regs_t *)reg_ctx->regs));
    FUN_CHECK(ret = vdpu1_set_asic_regs(p_hal, (H264dVdpu1Regs_t *)reg_ctx->regs));
    mpp_buffer_sync_end(reg_ctx->buf);

__RETURN:
    return ret = MPP_OK;
__FAILED:
    return ret;
}

/*!
***********************************************************************
* \brief h
*    start hard
***********************************************************************
*/
//extern "C"
MPP_RET vdpu1_h264d_start(void *hal, HalTaskInfo *task)
{
    MPP_RET ret = MPP_ERR_UNKNOW;
    H264dHalCtx_t *p_hal  = (H264dHalCtx_t *)hal;
    H264dVdpuRegCtx_t *reg_ctx = (H264dVdpuRegCtx_t *)p_hal->reg_ctx;
    H264dVdpu1Regs_t *p_regs = (H264dVdpu1Regs_t *)(p_hal->fast_mode ?
                                                    reg_ctx->reg_buf[task->dec.reg_index].regs :
                                                    reg_ctx->regs);

    if (task->dec.flags.parse_err ||
        task->dec.flags.ref_err) {
        goto __RETURN;
    }

    p_regs->SwReg57.sw_cache_en = 1;
    p_regs->SwReg57.sw_pref_sigchan = 1;
    p_regs->SwReg57.sw_intra_dbl3t = 1;
    p_regs->SwReg57.sw_inter_dblspeed = 1;
    p_regs->SwReg57.sw_intra_dblspeed = 1;
    p_regs->SwReg57.sw_paral_bus = 1;

    do {
        MppDevRegWrCfg wr_cfg;
        MppDevRegRdCfg rd_cfg;
        RK_U32 reg_size = DEC_VDPU1_REGISTERS * sizeof(RK_U32);

        wr_cfg.reg = reg_ctx->regs;
        wr_cfg.size = reg_size;
        wr_cfg.offset = 0;

        ret = mpp_dev_ioctl(p_hal->dev, MPP_DEV_REG_WR, &wr_cfg);
        if (ret) {
            mpp_err_f("set register write failed %d\n", ret);
            break;
        }

        rd_cfg.reg = reg_ctx->regs;
        rd_cfg.size = reg_size;
        rd_cfg.offset = 0;

        ret = mpp_dev_ioctl(p_hal->dev, MPP_DEV_REG_RD, &rd_cfg);
        if (ret) {
            mpp_err_f("set register read failed %d\n", ret);
            break;
        }

        ret = mpp_dev_ioctl(p_hal->dev, MPP_DEV_CMD_SEND, NULL);
        if (ret) {
            mpp_err_f("send cmd failed %d\n", ret);
            break;
        }
    } while (0);

__RETURN:
    (void)task;
    return ret = MPP_OK;
}

/*!
***********************************************************************
* \brief
*    wait hard
***********************************************************************
*/
//extern "C"
MPP_RET vdpu1_h264d_wait(void *hal, HalTaskInfo *task)
{
    MPP_RET ret = MPP_ERR_UNKNOW;
    H264dHalCtx_t  *p_hal = (H264dHalCtx_t *)hal;
    H264dVdpuRegCtx_t *reg_ctx = (H264dVdpuRegCtx_t *)p_hal->reg_ctx;
    H264dVdpu1Regs_t *p_regs = (H264dVdpu1Regs_t *)(p_hal->fast_mode ?
                                                    reg_ctx->reg_buf[task->dec.reg_index].regs :
                                                    reg_ctx->regs);

    if (task->dec.flags.parse_err ||
        task->dec.flags.ref_err) {
        goto __SKIP_HARD;
    }

    ret = mpp_dev_ioctl(p_hal->dev, MPP_DEV_CMD_POLL, NULL);
    if (ret)
        mpp_err_f("poll cmd failed %d\n", ret);

__SKIP_HARD:
    if (p_hal->dec_cb) {
        DecCbHalDone param;

        param.task = (void *)&task->dec;
        param.regs = (RK_U32 *)reg_ctx->regs;
        param.hard_err = !p_regs->SwReg01.sw_dec_rdy_int;

        mpp_callback(p_hal->dec_cb, &param);
    }
    memset(&p_regs->SwReg01, 0, sizeof(RK_U32));
    if (p_hal->fast_mode) {
        reg_ctx->reg_buf[task->dec.reg_index].valid = 0;
    }
    (void)task;

    return ret = MPP_OK;
}

/*!
***********************************************************************
* \brief
*    reset
***********************************************************************
*/
//extern "C"
MPP_RET vdpu1_h264d_reset(void *hal)
{
    MPP_RET ret = MPP_ERR_UNKNOW;
    H264dHalCtx_t *p_hal = (H264dHalCtx_t *)hal;

    INP_CHECK(ret, NULL == p_hal);
    memset(p_hal->priv, 0, sizeof(H264dVdpuPriv_t));

__RETURN:
    return ret = MPP_OK;
}

/*!
***********************************************************************
* \brief
*    flush
***********************************************************************
*/
//extern "C"
MPP_RET vdpu1_h264d_flush(void *hal)
{
    MPP_RET ret = MPP_ERR_UNKNOW;
    H264dHalCtx_t *p_hal = (H264dHalCtx_t *)hal;

    INP_CHECK(ret, NULL == p_hal);



__RETURN:
    return ret = MPP_OK;
}

/*!
***********************************************************************
* \brief
*    control
***********************************************************************
*/
//extern "C"
MPP_RET vdpu1_h264d_control(void *hal, MpiCmd cmd_type, void *param)
{
    MPP_RET ret = MPP_ERR_UNKNOW;
    H264dHalCtx_t *p_hal = (H264dHalCtx_t *)hal;

    INP_CHECK(ret, NULL == p_hal);

    (void)hal;
    (void)cmd_type;
    (void)param;
__RETURN:
    return ret = MPP_OK;
}

const MppHalApi hal_h264d_vdpu1 = {
    .name     = "h264d_vdpu1",
    .type     = MPP_CTX_DEC,
    .coding   = MPP_VIDEO_CodingAVC,
    .ctx_size = sizeof(H264dVdpuRegCtx_t),
    .flag     = 0,
    .init     = vdpu1_h264d_init,
    .deinit   = vdpu1_h264d_deinit,
    .reg_gen  = vdpu1_h264d_gen_regs,
    .start    = vdpu1_h264d_start,
    .wait     = vdpu1_h264d_wait,
    .reset    = vdpu1_h264d_reset,
    .flush    = vdpu1_h264d_flush,
    .control  = vdpu1_h264d_control,
};
