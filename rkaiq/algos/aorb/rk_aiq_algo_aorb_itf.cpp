/*
 * rk_aiq_algo_orb_itf.c
 *
 *  Copyright (c) 2019 Rockchip Corporation
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
 *
 */

#include "rk_aiq_algo_aorb_itf.h"
#include "rk_aiq_algo_types.h"
#include "rk_aiq_types_priv.h"

#include "orb_algos.h"
#include "xcam_log.h"

#if OPENCV_SUPPORT
#include "orb_algos_opencv.h"
#endif

RKAIQ_BEGIN_DECLARE

typedef struct ORBContext_s {
    unsigned char initialized;
    int output_width;
    int output_height;

    unsigned char orb_en;
    unsigned char limit_value;
    unsigned int max_feature;

    double affine_mat_roi[9];
    orb_rect_t roi_rect;
    orb_rect_t ref_roi_rect;
    rk_aiq_isp_orb_stats_t orb_stats;

    ORBList* roi_points_list;

} ORBContext_t;

typedef struct ORBContext_s* ORBHandle_t;

typedef struct _RkAiqAlgoContext {
    ORBHandle_t hORB;
} RkAiqAlgoContext;

#define ORB_LIMIT_VALUE_DEF 15
#define ORB_MAX_FEATURE_DEF 1000

#if OPENCV_SUPPORT
using namespace cv;
using namespace std;
#endif

XCamReturn ORBCreateContext(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg) {
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LOGI_ORB("%s: (enter)\n", __FUNCTION__);
    RkAiqAlgoContext* ctx = new RkAiqAlgoContext();
    if (ctx == NULL) {
        LOGE_AORB("%s: create aorb context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->hORB = new ORBContext_t;
    if (ctx->hORB == NULL) {
        delete ctx;
        LOGE_AORB("%s: create aorb handle fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    memset(ctx->hORB, 0, sizeof(ORBContext_t));
    *context = ctx;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn ORBDestroyContext(RkAiqAlgoContext* context) {
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    DCT_ASSERT(context != NULL);
    LOGI_ORB("%s: (enter)\n", __FUNCTION__);

    if (NULL != context) {
        delete context->hORB;
        context->hORB = NULL;
        delete context;
        context = NULL;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn ORBPrepare(RkAiqAlgoCom* params) {
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    ORBHandle_t hORB                        = (ORBHandle_t)params->ctx->hORB;
    ORBContext_t* ORBctx                    = (ORBContext_t*)hORB;
    RkAiqAlgoConfigAorb* rkaiqAorbConfig = (RkAiqAlgoConfigAorb*)params;

    ORBctx->orb_en = rkaiqAorbConfig->orb_calib_cfg.param.orb_en;

    ORBctx->limit_value = ORB_LIMIT_VALUE_DEF;
    ORBctx->max_feature = ORB_MAX_FEATURE_DEF;

    ORBctx->output_width  = params->u.prepare.sns_op_width;
    ORBctx->output_height = params->u.prepare.sns_op_height;

    ORBctx->roi_rect.left   = (ORBctx->output_width / 15) * 5;
    ORBctx->roi_rect.top    = (ORBctx->output_height / 15) * 5;
    ORBctx->roi_rect.width  = (ORBctx->output_width / 15) * 5;
    ORBctx->roi_rect.height = (ORBctx->output_height / 15) * 5;
    ORBctx->roi_rect.right  = ORBctx->roi_rect.left + ORBctx->roi_rect.width;
    ORBctx->roi_rect.bottom = ORBctx->roi_rect.top + ORBctx->roi_rect.height;
    ORBctx->ref_roi_rect    = ORBctx->roi_rect;

    LOGI_ORB("%s: (enter) enable: %d\n", __FUNCTION__, ORBctx->orb_en);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn ORBPreProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn ORBProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    XCamReturn result                         = XCAM_RETURN_NO_ERROR;
    ORBHandle_t ORBctx                        = (ORBHandle_t)inparams->ctx->hORB;
    RkAiqAlgoProcAorb* rkaiqAorbProcParam  = (RkAiqAlgoProcAorb*)inparams;
    RkAiqAlgoProcResAorb* rkaiqAorbProcOut = (RkAiqAlgoProcResAorb*)outparams;

    if (!ORBctx->orb_en) {
        return XCAM_RETURN_NO_ERROR;
    }

    LOGI_ORB("%s: (enter)\n", __FUNCTION__);

    RkAiqOrbStats* orbStats = nullptr;
    if (rkaiqAorbProcParam->orb_stats_buf) {
        orbStats = (RkAiqOrbStats*)rkaiqAorbProcParam->orb_stats_buf->map(
            rkaiqAorbProcParam->orb_stats_buf);
    }

    if (!ORBctx->initialized) {
        rkaiqAorbProcOut->orb_pre_result.valid = 0;
        ORBctx->initialized                    = 1;
        ORBctx->orb_stats.num_points           = 0;
    } else {
        if (ORBctx->orb_stats.num_points > 0 && orbStats->orb_stats.num_points > 0) {
            rkaiqAorbProcOut->orb_pre_result.valid = 1;
#if OPENCV_SUPPORT
            vector<Point2f> m_queryPoints, m_trainPoints;

            push_orbpoint_cv(orbStats->orb_stats.num_points, orbStats->orb_stats.points,
                             m_queryPoints);

            push_orbpoint_cv(ORBctx->orb_stats.num_points, ORBctx->orb_stats.points, m_trainPoints);

            get_homography_matrix(m_queryPoints, m_trainPoints,
                                  rkaiqAorbPreOut->orb_pre_result.homography_matrix,
                                  HOMOGRAPHY_MATRIX_SIZE);

            ORBctx->orb_stats.homography_matrix = rkaiqAorbPreOut->orb_pre_result.homography_matrix;
#else

#if CALC_AFFINE_TRANSFORM
            int ret = 0;

            if (ORBctx->roi_points_list == NULL) {
                ORBctx->roi_points_list = get_roi_points_list(&ORBctx->orb_stats, ORBctx->roi_rect);
            }

            if ((ORBctx->roi_points_list->length < 6)) {
                LOGD_AORB("reset keypoint_num: %d", ORBctx->roi_points_list->length);
                freeList(ORBctx->roi_points_list);
                ORBctx->roi_points_list = NULL;
                ORBctx->orb_stats       = orbStats->orb_stats;
            } else {
                ORBList* matched_list = matching(ORBctx->roi_points_list,
                                                 &rkaiqAorbPreParam->orb_stats, ORBctx->roi_rect);

                ret = elimate_affine_transform(matched_list, ORBctx->affine_mat_roi);
                if (ret < 0) {
                    rkaiqAorbPreOut->orb_pre_result.valid = 0;
                } else {
                    for (int i = 0; i < 3; i++) {
                        LOGD_AORB("---[%f, %f, %f]", ORBctx->affine_mat_roi[i * 3],
                                  ORBctx->affine_mat_roi[i * 3 + 1],
                                  ORBctx->affine_mat_roi[i * 3 + 2]);
                    }
                }

                // fill result params
                if (rkaiqAorbPreOut->orb_pre_result.valid) {
                    ORBctx->ref_roi_rect = map_rect(ORBctx->affine_mat_roi, &ORBctx->roi_rect);
                    rkaiqAorbPreOut->orb_pre_result.tracking_roi_rect = ORBctx->ref_roi_rect;
                    // ORBctx->roi_rect = ORBctx->ref_roi_rect;

                    LOGD_AORB("keypoint_num: %d, matched_list: %d, roi=[%d-%d-%d-%d]",
                              ORBctx->roi_points_list->length, matched_list->length,
                              ORBctx->ref_roi_rect.left, ORBctx->ref_roi_rect.top,
                              ORBctx->ref_roi_rect.right, ORBctx->ref_roi_rect.bottom);
                } else {
                    ORBctx->ref_roi_rect = ORBctx->roi_rect;
                    // ORBctx->orb_stats = orbStats->orb_stats;
                }

                orb_matched_point_t* out_matched_keypoints =
                    rkaiqAorbPreOut->orb_pre_result.matched_keypoints;
                rkaiqAorbPreOut->orb_pre_result.matched_keypoints_num = 0;

                int index = 0;
                for (Node* point = matched_list->start; point != NULL; point = point->next) {
                    orb_matched_point_t* keypoints = (orb_matched_point_t*)point->data;
                    out_matched_keypoints[index]   = *keypoints;
                    rkaiqAorbPreOut->orb_pre_result.matched_keypoints_num++;
                    index++;
                }

                freeList(matched_list);
                // freeList(roi_points_list);
            }
#endif
#endif
        } else {
            ORBctx->orb_stats = orbStats->orb_stats;
        }
    }

    //    if (inparams->u.proc.init) {
    rkaiqAorbProcOut->aorb_meas.update      = 1;
    rkaiqAorbProcOut->aorb_meas.orb_en      = ORBctx->orb_en;
    rkaiqAorbProcOut->aorb_meas.limit_value = ORBctx->limit_value;
    rkaiqAorbProcOut->aorb_meas.max_feature = ORBctx->max_feature;
    //    } else {
    //        rkaiqAorbProcOut->aorb_meas.update = 0;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn ORBPostProcess(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams) {
    return XCAM_RETURN_NO_ERROR;
}

RkAiqAlgoDescription g_RkIspAlgoDescAorb = {
    .common =
        {
            .version         = RKISP_ALGO_ORB_VERSION,
            .vendor          = RKISP_ALGO_ORB_VENDOR,
            .description     = RKISP_ALGO_ORB_DESCRIPTION,
            .type            = RK_AIQ_ALGO_TYPE_AORB,
            .id              = 0,
            .create_context  = ORBCreateContext,
            .destroy_context = ORBDestroyContext,
        },
    .prepare      = ORBPrepare,
    .pre_process  = ORBPreProcess,
    .processing   = ORBProcessing,
    .post_process = ORBPostProcess,
};

RKAIQ_END_DECLARE
