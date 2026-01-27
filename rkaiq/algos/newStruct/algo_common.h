
#ifndef __ALGO_COMMON_H__
#define __ALGO_COMMON_H__
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "algo_types_priv.h"
#include "xcam_log.h"

#define ILLUM_MAX_NUM 14
#define HISTORY_SIZE 10
#define ILLUM_NAME_LEN 8
#define TREND_THRESHOLD_RATIO 0.01f
#define TREND_WIN_SIZE 9

typedef struct {
    char name[ILLUM_NAME_LEN];
    float std_wbgain[2];  // 标准白平衡增益
    float distances[HISTORY_SIZE];  // 距离历史
    int dist_count;     // 距离历史计数
    int dist_start_idx;
} LightSource;

// 选择器主结构体
typedef struct {
    LightSource sources[ILLUM_MAX_NUM];
    int source_count;
    char illu_history[HISTORY_SIZE][ILLUM_NAME_LEN]; // 光源选择历史
    int history_count;
    int history_index;
    int frame_num;
} Selector;

typedef enum {
    DIS_TREND_STABLE        = 0,
    DIS_TREND_INCREASE      = 1,
    DIS_TREND_DECREASE      = -1,
} DistanceTrend;

void get_illu_estm_info(
    illu_estm_info_t *info,
    RkAiqAlgoProcResAwbShared_t *awbRes,
    RKAiqAecExpInfo_t *aeRes,
    int working_mode
);

float algo_strength_to_percent(float fStrength);

void selector_init(Selector* selector);
int selector_add_source(Selector* selector, int idx, const char* name, float r_gain, float b_gain);
float calculate_distance(const float* current, const float* standard);
void selector_process_frame(Selector* selector, const float* current_wbgain, int* final_illu);
const char* get_most_frequent_illu(Selector* selector);
const char* select_final_illu(Selector* selector, const char* hf_illu, const char* new_illu, const float* current_wbgain);
void print_selector_status(const Selector* selector);

#ifdef __cplusplus
}
#endif

#endif
