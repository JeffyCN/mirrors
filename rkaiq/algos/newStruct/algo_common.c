#include "algo_common.h"
#include "aiq_base.h"
#include <stdlib.h>

void get_illu_estm_info(
        illu_estm_info_t *info,
        RkAiqAlgoProcResAwbShared_t *awbRes,
        RKAiqAecExpInfo_t *aeRes,
        int working_mode
) {
    info->awbGain[0] = 1.0;
    info->awbGain[1] = 1.0;
    info->awbIIRDampCoef = 0.0;
    info->varianceLuma   = 0.0;
    info->awbConverged   = 1;
    //swinfo->grayMode = inparams->u.proc.gray_mode;

    if (awbRes) {
        if (awbRes->awb_gain_algo.grgain < DIVMIN || awbRes->awb_gain_algo.gbgain < DIVMIN) {
            LOGW("get wrong awb gain from AWB module ,use default value ");
        } else {
            info->awbGain[0] =
                awbRes->awb_gain_algo.rgain / awbRes->awb_gain_algo.grgain;

            info->awbGain[1] =
                awbRes->awb_gain_algo.bgain / awbRes->awb_gain_algo.gbgain;
        }
        info->awbIIRDampCoef = awbRes->awb_smooth_factor;
        info->varianceLuma   = awbRes->varianceLuma;
        info->awbConverged   = awbRes->awbConverged;
    } else {
        LOGW("fail to get awb gain form AWB module,use default value ");
    }

    if (aeRes) {
        if ((rk_aiq_working_mode_t)working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            info->sensorGain =
                aeRes->LinearExp.exp_real_params.analog_gain *
                aeRes->LinearExp.exp_real_params.digital_gain *
                aeRes->LinearExp.exp_real_params.isp_dgain;
        } else if ((rk_aiq_working_mode_t)working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2 &&
                   (rk_aiq_working_mode_t)working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3) {
            LOGD("sensor gain choose from second hdr frame for acolor");
            info->sensorGain =
                aeRes->HdrExp[1].exp_real_params.analog_gain *
                aeRes->HdrExp[1].exp_real_params.digital_gain *
                aeRes->HdrExp[1].exp_real_params.isp_dgain;
        } else if ((rk_aiq_working_mode_t)working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2 &&
                   (rk_aiq_working_mode_t)working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR3) {
            LOGD("sensor gain choose from third hdr frame for acolor");
            info->sensorGain =
                aeRes->HdrExp[2].exp_real_params.analog_gain *
                aeRes->HdrExp[2].exp_real_params.digital_gain *
                aeRes->HdrExp[2].exp_real_params.isp_dgain;
        } else {
            LOGW(
                "working_mode (%d) is invaild ,fail to get sensor gain form AE module,use default "
                "value ", working_mode);
        }
    } else {
        LOGW("fail to get sensor gain form AE module,use default value ");
    }
}

float algo_strength_to_percent(float fStrength) {
    float fPercent = 0.0;
    float fslope = 4.0;
    if (fStrength <= 0.5) {
        fPercent = fStrength / 0.5;
    } else {
        if (fStrength >= 0.999999) fStrength = 0.999999;
        fPercent = 0.5 * fslope / (1.0 - fStrength) - fslope + 1;
    }
    return fPercent;
}

// 初始化选择器
void selector_init(Selector* selector) {
    memset(selector, 0, sizeof(Selector));
    selector->history_index = 0;
    selector->frame_num = 0;

    for (int i = 0; i < HISTORY_SIZE; i++) {
        memset(selector->illu_history[i], 0, ILLUM_NAME_LEN);
    }
}

// 添加光源
int selector_add_source(Selector* selector, int idx, const char* name, float r_gain, float b_gain)
{
    if (idx >= ILLUM_MAX_NUM) {
        return -1; // 超出最大光源数
    }

    LightSource* source = &selector->sources[idx];
    if (strcmp(source->name, name) != 0) {
        strncpy(source->name, name, ILLUM_NAME_LEN - 1);
        source->dist_count = 0;
        source->dist_start_idx = 0; // 初始化为无效值
    }

    source->std_wbgain[0] = r_gain;
    source->std_wbgain[1] = b_gain;

    return 0;
}

// 计算欧氏距离
float calculate_distance(const float* current, const float* standard) {
    float dr = current[0] - standard[0];
    float db = current[1] - standard[1];

    return sqrtf(dr * dr + db * db);
}

// 获取光源索引
int get_source_index(Selector* selector, const char* name) {
    for (int i = 0; i < selector->source_count; i++) {
        if (strcmp(selector->sources[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// 获取出现频次最高的光源
const char* get_most_frequent_illu(Selector* selector) {
    if (selector->history_count == 0) {
        return NULL;
    }

    int frequency[ILLUM_MAX_NUM] = {0};
    int max_freq = 0;

    // 统计频次
    for (int i = 0; i < selector->history_count; i++) {
        if (strlen(selector->illu_history[i]) > 0) {
            int idx = get_source_index(selector, selector->illu_history[i]);
            if (idx >= 0) {
                frequency[idx]++;
                if (frequency[idx] > max_freq) {
                    max_freq = frequency[idx];
                }
            }
        }
    }

    // 如果没有有效数据，返回NULL
    if (max_freq == 0) {
        return NULL;
    }

    // 在频次最高的光源中，选择最近出现的一个
    for (int i = selector->history_count - 1; i >= 0; i--) {
        if (strlen(selector->illu_history[i]) > 0) {
            int idx = get_source_index(selector, selector->illu_history[i]);
            if (idx >= 0 && frequency[idx] == max_freq) {
                return selector->sources[idx].name;
            }
        }
    }

    return NULL;
}

// method1. 多帧趋势分析函数
static DistanceTrend get_multi_frame_trend(Selector* selector, int idx, float current_distance) {
    LightSource* ls = &selector->sources[idx];

    // 如果历史数据不足，使用可用数据
    int available_frames = ls->dist_count;
    if (available_frames < 2) return DIS_TREND_STABLE;

    // 计算最近TREND_WINDOW_SIZE帧的线性回归斜率
    int window_size = (available_frames < TREND_WIN_SIZE) ? available_frames : TREND_WIN_SIZE;

    float sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;

    // 获取最近num_frames帧的距离数据
    for (int i = 0; i < window_size; i++) {
        int hist_idx = (ls->dist_start_idx + ls->dist_count - window_size + i) % HISTORY_SIZE;
        float x = i - (window_size - 1);  // 时间索引
        float y = ls->distances[hist_idx];  // 距离值

        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_x2 += x * x;
    }

    // 计算线性回归斜率
    float numerator = window_size * sum_xy - sum_x * sum_y;
    float denominator = window_size * sum_x2 - sum_x * sum_x;

    if (fabsf(denominator) < 1e-6) return DIS_TREND_STABLE;

    float slope = numerator / denominator;

    // 根据斜率判断趋势
    float slope_threshold = 0.005f;  // 斜率阈值，可调整

    if (slope > slope_threshold) {
        return DIS_TREND_INCREASE;
    } else if (slope < -slope_threshold) {
        return DIS_TREND_DECREASE;
    } else {
        return DIS_TREND_STABLE;
    }
}

// method2. 加权移动平均趋势分析
static DistanceTrend get_weighted_trend(Selector* selector, int idx, float current_distance) {
    LightSource* ls = &selector->sources[idx];
    int available_frames = ls->dist_count;

    if (available_frames < 2) return DIS_TREND_STABLE;

    // 计算加权平均值（近期帧权重更高）
    float weighted_sum = 0;
    float weight_sum = 0;

    for (int i = 0; i < available_frames; i++) {
        int hist_idx = (ls->dist_start_idx + i) % HISTORY_SIZE;
        float weight = expf(-0.5f * (available_frames - 1 - i));  // 指数衰减权重
        weighted_sum += ls->distances[hist_idx] * weight;
        weight_sum += weight;
    }

    float weighted_avg = weighted_sum / weight_sum;

    // 计算当前距离与加权平均值的相对变化
    float change_ratio = (current_distance - weighted_avg) / weighted_avg;
    float threshold = 0.02f;  // 2%的变化阈值

    if (change_ratio > threshold) {
        return DIS_TREND_INCREASE;
    } else if (change_ratio < -threshold) {
        return DIS_TREND_DECREASE;
    } else {
        return DIS_TREND_STABLE;
    }
}

// method3. 基于连续变化一致性的趋势判断
static DistanceTrend get_consistent_trend(Selector* selector, int idx, float current_distance) {
    LightSource* ls = &selector->sources[idx];
    int available_frames = ls->dist_count;

    if (available_frames < 3) return DIS_TREND_STABLE;  // 至少需要3帧进行一致性判断

    // 检查最近几帧的变化方向是否一致
    int increasing_count = 0;
    int decreasing_count = 0;

    int check_frames = (available_frames < 4) ? available_frames : 4;

    for (int i = 0; i < check_frames - 1; i++) {
        int idx1 = (ls->dist_start_idx + available_frames - check_frames + i) % HISTORY_SIZE;
        int idx2 = (idx1 + 1) % HISTORY_SIZE;

        float dist1 = ls->distances[idx1];
        float dist2 = ls->distances[idx2];
        float change = dist2 - dist1;
        float threshold = dist1 * 0.01f;  // 1%的变化阈值

        if (change > threshold) {
            increasing_count++;
        } else if (change < -threshold) {
            decreasing_count++;
        }
    }

    // 判断一致性
    if (increasing_count >= 2) {  // 至少连续2帧增加
        return DIS_TREND_INCREASE;
    } else if (decreasing_count >= 2) {  // 至少连续2帧减少
        return DIS_TREND_DECREASE;
    } else {
        return DIS_TREND_STABLE;
    }
}

// method4. 自适应阈值趋势分析
static DistanceTrend get_adaptive_trend(Selector* selector, int idx, float current_distance, float* confidence) {
    LightSource* ls = &selector->sources[idx];
    int available_frames = ls->dist_count;

    if (available_frames < 3) {
        *confidence = 0.5f;
        return DIS_TREND_STABLE;
    }

    // 计算历史距离的标准差，作为自适应阈值
    float mean = 0;
    for (int i = 0; i < available_frames; i++) {
        int hist_idx = (ls->dist_start_idx + i) % HISTORY_SIZE;
        mean += ls->distances[hist_idx];
    }
    mean /= available_frames;

    float variance = 0;
    for (int i = 0; i < available_frames; i++) {
        int hist_idx = (ls->dist_start_idx + i) % HISTORY_SIZE;
        float diff = ls->distances[hist_idx] - mean;
        variance += diff * diff;
    }
    variance /= available_frames;
    float std_dev = sqrtf(variance);

    // 方差越小，置信度越高
    *confidence = 1.0f / (1.0f + std_dev);

    // 自适应阈值：基于历史波动性
    float adaptive_threshold = std_dev * 0.5f;  // 可调整系数

    // 比较当前帧与最近几帧的平均值
    int recent_frames = (available_frames < 3) ? available_frames : 3;
    float recent_avg = 0;
    for (int i = 0; i < recent_frames; i++) {
        int hist_idx = (ls->dist_start_idx + available_frames - recent_frames + i) % HISTORY_SIZE;
        recent_avg += ls->distances[hist_idx];
    }
    recent_avg /= recent_frames;

    float change = current_distance - recent_avg;

    if (change > adaptive_threshold) {
        return DIS_TREND_INCREASE;
    } else if (change < -adaptive_threshold) {
        return DIS_TREND_DECREASE;
    } else {
        return DIS_TREND_STABLE;
    }
}

// 综合多种方法的趋势判断
static DistanceTrend get_comprehensive_trend(Selector* selector, int idx, float current_distance, float* confidence) {
    if (idx < 0) {
        *confidence = 0.0f;
        return DIS_TREND_STABLE;
    }
    // 获取多种趋势判断结果
    DistanceTrend trend1 = get_multi_frame_trend(selector, idx, current_distance);
    DistanceTrend trend2 = get_weighted_trend(selector, idx, current_distance);
    DistanceTrend trend3 = get_consistent_trend(selector, idx, current_distance);
    DistanceTrend trend4 = get_adaptive_trend(selector, idx, current_distance, confidence);

    // 投票机制
    int increasing_votes = 0;
    int decreasing_votes = 0;

    DistanceTrend trends[] = {trend1, trend2, trend3, trend4};
    for (int i = 0; i < 4; i++) {
        if (trends[i] == DIS_TREND_INCREASE) increasing_votes++;
        else if (trends[i] == DIS_TREND_DECREASE) decreasing_votes++;
    }

    // 根据投票结果决定最终趋势
    if (increasing_votes >= 3) return DIS_TREND_INCREASE;
    if (decreasing_votes >= 3) return DIS_TREND_DECREASE;

    // 如果没有明显多数，保守地返回稳定
    return DIS_TREND_STABLE;
}

// 辅助函数：计算历史平均距离
float get_average_distance(Selector* selector, const char* source) {
    int idx = get_source_index(selector, source);
    if (idx < 0) return 0.0f;

    LightSource* ls = &selector->sources[idx];
    if (ls->dist_count == 0) return 0.0f;

    float sum = 0;
    for (int i = 0; i < ls->dist_count; i++) {
        int hist_idx = (ls->dist_start_idx + i) % HISTORY_SIZE;
        sum += ls->distances[hist_idx];
    }

    return sum / ls->dist_count;
}

// 选择最终光源
const char* select_final_illu(Selector* selector, const char* hf_illu, const char* new_illu, const float* current_wbgain) {
    // 如果历史为空或是第一帧，直接返回新光源
    if (hf_illu == NULL || selector->frame_num <= 1) {
        return new_illu;
    }

    // 计算当前距离
    int hf_idx = get_source_index(selector, hf_illu);
    int new_idx = get_source_index(selector, new_illu);

    if (hf_idx < 0 || new_idx < 0) {
        return hf_illu ? hf_illu : new_illu;
    }

    float hf_distance = calculate_distance(current_wbgain, selector->sources[hf_idx].std_wbgain);
    float new_distance = calculate_distance(current_wbgain, selector->sources[new_idx].std_wbgain);

    // 检查趋势条件
    float hf_confidence = 0.0f;
    float new_confidence  = 0.0f;
    DistanceTrend hf_trend = get_comprehensive_trend(selector, hf_idx, hf_distance, &hf_confidence);
    DistanceTrend new_trend = get_comprehensive_trend(selector, new_idx, new_distance, &new_confidence);

    LOGW("TrendAnalyzer(stable0_incrs1_decrs-1): hfIllu(%s)=%d(conf:%f), newIllu(%s)=%d(conf:%f)\n",
           hf_illu, hf_trend, hf_confidence, new_illu, new_trend, new_confidence);

    // 增强的判断条件：考虑趋势强度和置信度
    if (hf_trend == DIS_TREND_INCREASE && new_trend == DIS_TREND_DECREASE) {
        if (hf_confidence < 0.7f || new_confidence > 0.8f) {
            // 额外检查：hfIllu的距离是否显著大于历史平均值
            float hf_avg = get_average_distance(selector, hf_illu);
            if (hf_distance > hf_avg * 1.05f) {  // 当前距离比历史平均大5%
                return new_illu;
            }
        }
    }
    // 如果hfIllu置信度很高，倾向于保持
    if (hf_confidence > 0.8f) {
        LOGD("  TrendAnalyzer: keep hfIllu (hf_conf > 0.8)\n");
        return hf_illu;
    }

    // 如果newIllu置信度明显高于hfIllu，考虑切换
    if (new_confidence > hf_confidence + 0.2f) {
        LOGD("  TrendAnalyzer: change to newIllu (new_conf > hf_conf + 0.2f )\n");
        return new_illu;
    }

    LOGD("  TrendAnalyzer: keep hfIllu (default)\n");
    return hf_illu;
}

// 处理每一帧
void selector_process_frame(Selector* selector, const float* current_wbgain, int* final_illu) {
    selector->frame_num++;

    // 1. 计算当前帧与各光源标准wbgain的距离
    float min_distance = 99999;
    const char* new_illu = NULL;

    LOGD("selector->frame %d: ", selector->frame_num);

    for (int i = 0; i < selector->source_count; i++) {
        float dist = calculate_distance(current_wbgain, selector->sources[i].std_wbgain);

        // 更新光源的距离历史
        if (selector->sources[i].dist_count < HISTORY_SIZE) {
            selector->sources[i].distances[selector->sources[i].dist_count++] = dist;
        } else {
            selector->sources[i].distances[selector->sources[i].dist_start_idx] = dist;
            selector->sources[i].dist_start_idx = (selector->sources[i].dist_start_idx + 1) %
                                                    HISTORY_SIZE;
        }

        LOGD("%s=%.3f (%.2f, %.2f) ", selector->sources[i].name, dist, selector->sources[i].std_wbgain[0], selector->sources[i].std_wbgain[1]);

        // 找到距离最小的光源
        if (dist < min_distance) {
            min_distance = dist;
            new_illu = selector->sources[i].name;
        }
    }
    LOGD("\n");

    // 2. 将新光源加入历史列表
    if (selector->history_count < HISTORY_SIZE) {
        strncpy(selector->illu_history[selector->history_count], new_illu, ILLUM_NAME_LEN - 1);
        selector->history_count++;
    } else {
        // 循环缓冲区
        strncpy(selector->illu_history[selector->history_index], new_illu, ILLUM_NAME_LEN - 1);
        selector->history_index = (selector->history_index + 1) % HISTORY_SIZE;
    }

    // 3. 计算历史频次最高的光源hfIllu
    const char* hf_illu = get_most_frequent_illu(selector);

    // 4. 判断趋势条件，选择最终光源
    const char* final = select_final_illu(selector, hf_illu, new_illu, current_wbgain);
    *final_illu = get_source_index(selector, final);

    LOGD("  newIllu=%s, hfIllu=%s, finalIllu=%s\n", new_illu, hf_illu ? hf_illu : "NULL", final);
}

// 打印选择器状态
void print_selector_status(const Selector* selector) {
    printf("======== lgtSrc status ======\n");
    printf("processed frames: %d\n", selector->frame_num);
    printf("lgtSrc select history: ");
    for (int i = 0; i < selector->history_count; i++) {
        printf("%s ", selector->illu_history[i]);
    }
    printf("\n");
}
