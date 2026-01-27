#ifndef _LSC_ALOG_API_H_
#define _LSC_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#include "isp/rk_aiq_isp_lsc21.h"
#include "algos/rk_aiq_api_types_lsc.h"

#if 0
#include "common/list.h"
#include "RkAiqCalibDbTypes.h"

typedef enum LscHwVersion_e
{
    Lsc_HARDWARE_V20 = 0,  // rv1109 rv1126
    Lsc_HARDWARE_V21 = 1,
    Lsc_HARDWARE_MAX,
} LscHwVersion_t;

typedef const LscTableProfile_t* lscTableProfile_t;

typedef struct lsc_matrix
{
    Cam17x17UShortMatrix_t  LscMatrix[CAM_4CH_COLOR_COMPONENT_MAX];
} lsc_matrix_t;

/** @brief store LSC recent/last results*/
typedef struct lsc_rest_s {
    uint32_t caseIndex;
    float fVignetting;
    struct list_head dominateIlluList;//to record domain illuminant
    uint32_t estimateIlluCaseIdx;
    uint32_t resIdx;
    char LscProName1[32];
    char LscProName2[32];
    lsc_matrix_t undampedLscMatrixTable;
    lsc_matrix_t dampedLscMatrixTable;
} lsc_rest_t;

typedef struct illu_lsc_node_s {
    list_head node;       /**< for adding to a list */
    unsigned int value;
} illu_lsc_node_t;

typedef struct lsc_illu_case_resolution {
    lsc_resolution_t resolution;
    lscTableProfile_t *lsc_table_group;
    int lsc_table_count;
} lsc_illu_case_resolution_t;

/**
 * @brief lsc illumination case is different in <enum CalibDb_Used_For_Case_e>,
 *        <resolution>, <color temperature>.
 */
typedef struct lsc_illu_case {
    const LscCof_ill_t *lsc_cof;
    lsc_illu_case_resolution_t *res_group;
    uint32_t res_count;
    uint32_t current_res_idx;
} lsc_illu_case_t;

typedef lsc_illu_case_t* pIlluCase_t;
/** @brief depends on enum  CalibDb_Used_For_Case_e */
typedef struct lsc_mode_data_s
{
    //TODO: actually is const point and should add const
    pIlluCase_t     *illu_case;
    uint32_t        illu_case_count;
} lsc_mode_data_t;

typedef struct lsc_grad_s
{
    lsc_resolution_t    resolution;
    uint16_t        LscXGradTbl[LSC_GRAD_TBL_SIZE];
    uint16_t        LscYGradTbl[LSC_GRAD_TBL_SIZE];
} lsc_grad_t;

typedef struct lsc_otp_grad_s
{
    uint32_t flag;

    uint16_t width;
    uint16_t height;
    uint16_t table_size;

    uint16_t lsc_r[LSC_DATA_TBL_SIZE];
    uint16_t lsc_b[LSC_DATA_TBL_SIZE];
    uint16_t lsc_gr[LSC_DATA_TBL_SIZE];
    uint16_t lsc_gb[LSC_DATA_TBL_SIZE];
} lsc_otp_grad_t;

typedef struct lsc_smart_run_cfg_s {
    bool  enable;
    float gain_th;
    float wbgain_th;
} lsc_smart_run_cfg_t;

typedef struct lsc_smart_run_res_s {
    bool forceRunFlag;//update by api or prepare;
    float last_gain;
    float last_awbGain[2];
    bool res3aChg;
    bool lscTableConverge;
} lsc_smart_run_res_t;


#endif

typedef struct {
    uint32_t flag;

    uint16_t width;
    uint16_t height;
    uint16_t table_size;

    uint16_t *lsc_r;
    uint16_t *lsc_b;
    uint16_t *lsc_gr;
    uint16_t *lsc_gb;
} LscOtpInfo_t;

typedef struct lsc_sw_info_s {
    float sensorGain;
    float awbGain[2];
    float awbIIRDampCoef;
    float varianceLuma;
    bool grayMode;
    bool awbConverged;
    //int prepare_type;
    //RkAiqBayerPattern_t bayerPattern;
    //LscOtpInfo_t otpInfo;
} lsc_sw_info_t;

XCAM_BEGIN_DECLARE
XCamReturn algo_lsc_queryalscStatus(RkAiqAlgoContext* ctx, alsc_status_t* status);
XCamReturn algo_lsc_SetCalib(RkAiqAlgoContext* ctx, alsc_lscCalib_t *calib);
XCamReturn algo_lsc_GetCalib(RkAiqAlgoContext* ctx, alsc_lscCalib_t* calib);
#if 0
XCamReturn
algo_lsc_SetAttrib
(
    RkAiqAlgoContext* ctx,
    lsc_api_attrib_t *attr
);

XCamReturn
algo_lsc_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    lsc_api_attrib_t *attr
);
#endif
extern RkAiqAlgoDescription g_RkIspAlgoDescLsc;
XCAM_END_DECLARE

#endif
