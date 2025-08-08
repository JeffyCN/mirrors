

#ifndef __RK_AIQ_USER_API2_COMMON_H__
#define __RK_AIQ_USER_API2_COMMON_H__
#include "common/rk_aiq_comm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum opMode_e {
    OP_AUTO   = 0,
    OP_MANUAL = 1,
    OP_SEMI_AUTO = 2,
    OP_REG_MANUAL = 3,
    OP_INVAL
} opMode_t;

/*
*****************************
* Common
*****************************
*/

typedef enum dayNightScene_e {
    DAYNIGHT_SCENE_DAY   = 0,
    DAYNIGHT_SCENE_NIGHT = 1,
    DAYNIGHT_SCENE_INVAL,
} dayNightScene_t;

typedef struct paRange_s {
    float max;
    float min;
} paRange_t;

typedef enum awbRange_e {
    AWB_RANGE_0 = 0,
    AWB_RANGE_1 = 1,
    AWB_RANGE_INVAL,
} awbRange_t;

typedef enum aeMode_e {
    AE_AUTO          = 0,
    AE_IRIS_PRIOR    = 1,
    AE_SHUTTER_PRIOR = 2,
} aeMode_t;


/*
*****************
*   (-1000, -1000)    (1000, -1000)
*   -------------------
*   |                 |
*   |                 |
*   |       (0,0)     |
*   |                 |
*   |                 |
*   -------------------
*                     (1000, 1000)
*****************
*/
typedef struct paRect_s {
    int x;
    int y;
    unsigned int w;
    unsigned int h;
} paRect_t;

typedef enum aeMeasAreaType_e {
    AE_MEAS_AREA_AUTO = 0,
    AE_MEAS_AREA_UP,
    AE_MEAS_AREA_BOTTOM,
    AE_MEAS_AREA_LEFT,
    AE_MEAS_AREA_RIGHT,
    AE_MEAS_AREA_CENTER,
} aeMeasAreaType_t;

typedef enum expPwrLineFreq_e {
    EXP_PWR_LINE_FREQ_DIS   = 0,
    EXP_PWR_LINE_FREQ_50HZ  = 1,
    EXP_PWR_LINE_FREQ_60HZ  = 2,
} expPwrLineFreq_t;

typedef enum antiFlickerMode_e {
    ANTIFLICKER_NORMAL_MODE = 0,
    ANTIFLICKER_AUTO_MODE   = 1,
} antiFlickerMode_t;

typedef struct frameRateInfo_s {
    opMode_t         mode;
    unsigned int     fps; /* valid when manual mode*/
} frameRateInfo_t;

typedef struct rk_aiq_version_info_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(aiq_ver),
        M4_TYPE(string),
        M4_SIZE_EX(1,1),
        M4_DEFAULT(""),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: high))  */
    char aiq_ver[32];
} rk_aiq_version_info_t;

typedef struct rk_aiq_module_ctl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(type),
        M4_TYPE(enum),
        M4_ENUM_DEF(camAlgoResultType),
        M4_DEFAULT(RESULT_TYPE_INVALID),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Reference enum types.\n
		Freq of use: high))  */
    camAlgoResultType type;
    /* M4_GENERIC_DESC(
        M4_ALIAS(en),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: high))  */
    bool en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(bypass),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: high))  */
    bool bypass;
    /* M4_GENERIC_DESC(
        M4_ALIAS(opMode),
        M4_TYPE(enum),
        M4_ENUM_DEF(rk_aiq_op_mode_t),
        M4_DEFAULT(RK_AIQ_OP_MODE_AUTO),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The current operation mode))  */
    rk_aiq_op_mode_t opMode;
} rk_aiq_module_ctl_t;

typedef struct rk_aiq_module_list_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(module_ctl),
        M4_TYPE(struct),
        M4_SIZE_EX(1,47),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))   */
    rk_aiq_module_ctl_t module_ctl[RESULT_TYPE_MAX_PARAM];
} rk_aiq_module_list_t;

typedef struct algo_interp_iso_info_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(type),
        M4_TYPE(enum),
        M4_ENUM_DEF(camAlgoResultType),
        M4_DEFAULT(RESULT_TYPE_INVALID),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Reference enum types.\n
		Freq of use: high))  */
    camAlgoResultType type;
    /* M4_GENERIC_DESC(
        M4_ALIAS(interpIso),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 640000),
        M4_DEFAULT(50),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(original iso.\n
        Freq of use: high))  */
    uint32_t interpIso;
    /* M4_GENERIC_DESC(
        M4_ALIAS(interpIso),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 640000),
        M4_DEFAULT(50),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(interpolation iso high.\n
        Freq of use: high))  */
    uint32_t isoH;
    /* M4_GENERIC_DESC(
        M4_ALIAS(interpIso),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0, 640000),
        M4_DEFAULT(50),
        M4_DIGIT_EX(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(interpolation iso low.\n
        Freq of use: high))  */
    uint32_t isoL;
} algo_interp_iso_info_t;

typedef struct algo_interp_iso_list_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(interp_iso),
        M4_TYPE(struct),
        M4_SIZE_EX(1,55),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))   */
    algo_interp_iso_info_t isoInfo[RESULT_TYPE_MAX_PARAM];
} algo_interp_iso_list_t;

typedef enum cSpaceMode_e {
    CSPACE_MODE_BT601_FULL  = 0,
    CSPACE_MODE_BT601_LIMIT = 1,
    CSPACE_MODE_BT709_FULL  = 2,
    CSPACE_MODE_BT709_LIMIT = 3,
    CSPACE_MODE_OTHER_FULL  = 253,
    CSPACE_MODE_OTHER_LIMIT = 254,
    CSPACE_MODE_OTHERS      = 255,
} cSpaceMode_t;

#ifdef __cplusplus
}
#endif

#endif  /*__RK_AIQ_USER_API2_COMMON_H__*/
