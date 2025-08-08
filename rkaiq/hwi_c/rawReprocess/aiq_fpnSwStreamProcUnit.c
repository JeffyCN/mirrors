
#include "hwi_c/rawReprocess/aiq_fpnSwStreamProcUnit.h"
#include "c_base/aiq_list.h"
#include "c_base/aiq_mutex.h"
#include "c_base/aiq_thread.h"
#include "c_base/aiq_cond.h"
#include "xcore_c/aiq_v4l2_buffer.h"
#include "hwi_c/aiq_sensorHw.h"

#define RKAIQ_FPNSW_DEBUG 0

#if RKAIQ_FPNSW_DEBUG
static void *g_output_buf = NULL;
static size_t g_line_offset = 0;
#endif

#if 0
static void process_two_groups(uint32_t* data) {
    // Load 10x32-bit words (two groups)
    uint32x4_t q0 = vld1q_u32(data);      // Load first 4 words
    uint32x4_t q1 = vld1q_u32(data + 4);  // Load next 4 words
    uint32x2_t d4 = vld1_u32(data + 8);   // Load last 2 words

    // Constants
    const uint32x4_t mask_3FF = vdupq_n_u32(0x3FF);
    const uint16x8_t add10 = vdupq_n_u16(10);

    // Phase 1 unpack: First 8 values
    uint32x4_t s0 = vandq_u32(q0, mask_3FF);                  // [v00]
    uint32x4_t s1 = vandq_u32(vshrq_n_u32(q0, 10), mask_3FF); // [v01]
    uint32x4_t s2 = vandq_u32(vshrq_n_u32(q0, 20), mask_3FF); // [v02]
    uint32x4_t s3 = vshrq_n_u32(q0, 30);                      // [v03_high]

    // Cross-vector combination
    uint32x4_t t0 = vorrq_u32(s3, vshlq_n_u32(q1, 2));       // Combine v03
    uint32x4_t s4 = vandq_u32(t0, mask_3FF);                 // [v03]
    
    // Continue unpacking
    uint32x4_t s5 = vandq_u32(vshrq_n_u32(q1, 8), mask_3FF);  // [v04]
    uint32x4_t s6 = vandq_u32(vshrq_n_u32(q1, 18), mask_3FF); // [v05]
    uint32x4_t s7 = vshrq_n_u32(q1, 28);                      // [v06_high]
    
    // Prepare second vector
    uint32x4_t q2 = vcombine_u32(vget_low_u32(q1), d4);       // Combine q1+d4
    uint32x4_t t1 = vorrq_u32(s7, vshlq_n_u32(q2, 4));       // Combine v06
    uint32x4_t s8 = vandq_u32(t1, mask_3FF);                 // [v06]
    
    // Continue unpacking...
    uint32x4_t s9 = vandq_u32(vshrq_n_u32(q2, 6), mask_3FF);  // [v07]
    uint32x4_t s10 = vandq_u32(vshrq_n_u32(q2, 16), mask_3FF); // [v08]
    uint32x4_t s11 = vandq_u32(vshrq_n_u32(q2, 26), mask_3FF); // [v09]
    
    // Second group unpacking
    uint32x4_t s12 = vandq_u32(q2, mask_3FF);                 // [v10]
    uint32x4_t s13 = vandq_u32(vshrq_n_u32(q2, 10), mask_3FF); // [v11]
    uint32x4_t s14 = vandq_u32(vshrq_n_u32(q2, 20), mask_3FF); // [v12]
    uint32x4_t s15 = vshrq_n_u32(q2, 30);                      // [v13_high]
    
    // Prepare next vector (FIXED: Use uint32x2_t zero instead of uint32x4_t)
    uint32x2_t zero = vdup_n_u32(0);                          // Create 2-lane zero vector
    uint32x4_t q3 = vcombine_u32(d4, zero);                   // Extend d4 with zeros
    
    uint32x4_t t2 = vorrq_u32(s15, vshlq_n_u32(q3, 2));       // Combine v13
    uint32x4_t s16 = vandq_u32(t2, mask_3FF);                 // [v13]
    
    uint32x4_t s17 = vandq_u32(vshrq_n_u32(q3, 8), mask_3FF); // [v14]
    uint32x4_t s18 = vandq_u32(vshrq_n_u32(q3, 18), mask_3FF); // [v15]

    // Convert to 16-bit vectors
    uint16x8x2_t vals;
    vals.val[0] = vcombine_u16(
        vmovn_u32(s0), vmovn_u32(s1)  // v00-v03
    );
    vals.val[1] = vcombine_u16(
        vmovn_u32(s2), vmovn_u32(s4)  // v04-v07
    );
    
    // Add 10 with saturation
    vals.val[0] = vaddq_u16(vals.val[0], add10);
    vals.val[1] = vaddq_u16(vals.val[1], add10);
    
    // Second set of 16 values
    uint16x8x2_t vals2;
    vals2.val[0] = vcombine_u16(
        vmovn_u32(s5), vmovn_u32(s6)  // v08-v11
    );
    vals2.val[1] = vcombine_u16(
        vmovn_u32(s8), vmovn_u32(s9)  // v12-v15
    );
    
    vals2.val[0] = vaddq_u16(vals2.val[0], add10);
    vals2.val[1] = vaddq_u16(vals2.val[1], add10);

    // Repack to 10-bit format
    uint32x4_t r0 = vreinterpretq_u32_u16(vals.val[0]);
    uint32x4_t r1 = vreinterpretq_u32_u16(vals.val[1]);
    uint32x4_t r2 = vreinterpretq_u32_u16(vals2.val[0]);
    uint32x4_t r3 = vreinterpretq_u32_u16(vals2.val[1]);
    
    // Packing logic
    uint32x4_t out0 = vorrq_u32(
        vandq_u32(r0, mask_3FF),
        vshlq_n_u32(vandq_u32(r1, mask_3FF), 10)
    );
    out0 = vorrq_u32(out0, vshlq_n_u32(vandq_u32(r2, mask_3FF), 20));
    
    // Handle cross-word values
    uint32x4_t hi0 = vshrq_n_u32(r0, 10);
    uint32x4_t hi1 = vshrq_n_u32(r1, 10);
    uint32x4_t hi2 = vshrq_n_u32(r2, 10);
    
    uint32x4_t out1 = vorrq_u32(
        vandq_u32(hi0, mask_3FF),
        vshlq_n_u32(vandq_u32(hi1, mask_3FF), 10)
    );
    out1 = vorrq_u32(out1, vshlq_n_u32(vandq_u32(hi2, mask_3FF), 20));
    
    // Pack remaining values (FIX: Added out2 using r3)
    uint32x4_t hi3 = vshrq_n_u32(r3, 10);
    uint32x4_t out2 = vorrq_u32(
        vandq_u32(r3, mask_3FF),
        vshlq_n_u32(vandq_u32(hi3, mask_3FF), 10)
    );

    // Store results (FIX: Use declared out2)
    vst1q_u32(data, out0);            // Store first 4 words
    vst1q_u32(data + 4, out1);        // Store next 4 words
    vst1q_u32(data + 8, out2);        // Store last 2 words (uses low 64 bits)
}
#endif

#if RKAIQ_FPNSW_DEBUG

static inline void neon_unpack_10bit_8pix(const uint8_t* in, uint16_t* out) {

    uint8x16_t data = vld1q_u8(in);

    uint64_t lo = vgetq_lane_u64(vreinterpretq_u64_u8(data), 0);
    uint64_t hi = vgetq_lane_u64(vreinterpretq_u64_u8(data), 1);

    uint16_t hi16 = hi & 0xFFFF;

    out[0] = lo & 0x3FF;
    out[1] = (lo >> 10) & 0x3FF;
    out[2] = (lo >> 20) & 0x3FF;
    out[3] = (lo >> 30) & 0x3FF;
    out[4] = (lo >> 40) & 0x3FF;
    out[5] = (lo >> 50) & 0x3FF;

    uint32_t cross = (lo >> 60) | ((uint32_t)hi16 << 4);
    out[6] = cross & 0x3FF;
    out[7] = (cross >> 10) & 0x3FF;
}

static inline void neon_add10_1pix(uint16_t* data, size_t length) {

    // 计算可向量化处理的块数（每块8个uint16_t）
    size_t vector_blocks = length / 8;
    size_t remainder = length % 8;
    
    // 创建常量向量vdupq_n_u16(10)，包含8个16位的10
    uint16x8_t v_ten = vdupq_n_u16(10);
    
    // 向量化处理主循环
    for (size_t i = 0; i < vector_blocks; i++) {
        // 加载8个uint16_t元素到向量寄存器
        uint16x8_t v_data = vld1q_u16(&data[i * 8]);
        
        // 每个元素加10
        uint16x8_t v_result = vaddq_u16(v_data, v_ten);
        
        // 存储结果回内存
        vst1q_u16(&data[i * 8], v_result);
    }

    // 处理剩余不足8个元素的数据（标量处理）
    for (size_t i = vector_blocks * 8; i < length; i++) {
        data[i] += 10;
    }

}

static bool fpn_switch = false;

static int _get_int_default_frome_file(const char *path)
{
    if (!path) {
        return 0;
    }

    char level[64] = {'\0'};
    FILE* fp = fopen(path, "r");

    if (!fp)
        return 0;

    fseek(fp, 0, SEEK_SET);
    if (fp) {
        fread(level, 63, 1, fp);
        fclose(fp);
        // LOGK("MEMC FILE %s", level);
        int res = atoi(level);
        return res;
    }
    return 0;

}

#endif

static bool fpnSwStreamProcTh_func(void *user_data)
{
    AiqFpnSwStreamProcThread_t* pHdlTh = (AiqFpnSwStreamProcThread_t*)user_data;
    AiqFpnSwStreamProcUnit_t* pFpnProc = pHdlTh->mFpnProc;

    aiqMutex_lock(&pHdlTh->_mutex);
    while (!pHdlTh->bQuit && aiqList_size(pHdlTh->mMsgsQueue) <= 0) {
        aiqCond_wait(&pHdlTh->_cond, &pHdlTh->_mutex);
    }
    if (pHdlTh->bQuit) {
        aiqMutex_unlock(&pHdlTh->_mutex);
        LOGW_ANALYZER("quit %s !", __func__);
        return false;
    }
    aiqMutex_unlock(&pHdlTh->_mutex);

    fpn_proc_msg_t msg;
    int ret = aiqList_get(pHdlTh->mMsgsQueue, &msg);
    if (ret) {
        // ignore error
        return true;
    }

    int cur_fps = msg.expParam->aecExpInfo.pixel_clock_freq_mhz * 1000000.0 /
                   msg.expParam->aecExpInfo.frame_length_lines /
                   msg.expParam->aecExpInfo.line_length_pixels;

    if (pFpnProc->_fpn_init && pFpnProc->_fpn_attr->en && cur_fps <= pFpnProc->_fpn_attr->stAuto.sta.sw_fpnSw_fps_val) {
#if RKAIQ_FPNSW_DEBUG
        LOGK("run here %s:%d time %s", __func__, __LINE__, timeString());
        if (!fpn_switch) {
#endif
        rk_fpn_correction_param_t fpn_proc;
        fpn_proc.raw_wid         = pFpnProc->_width;
        fpn_proc.raw_hgt         = pFpnProc->_height;
        fpn_proc.sensor_gain     = msg.expParam->aecExpInfo.LinearExp.exp_real_params.analog_gain *
                                   msg.expParam->aecExpInfo.LinearExp.exp_real_params.digital_gain;
        fpn_proc.max_sensor_gain = pFpnProc->_csi_again_max * pFpnProc->_csi_dgain_max;
        fpn_proc.fpn_offset      = pFpnProc->_fpn_attr->stAuto.sta.sw_fpnSw_off;
#if RKAIQ_FPNSW_DEBUG
        fpn_proc.fpn_offset      = _get_int_default_frome_file("/data/fpn_offset");
#endif
        fpn_proc.current_raw_ptr = (uint8_t*)AiqV4l2Buffer_getExpbufUsrptr(msg.buf_s);
        rk_fpn_correction_proc(&fpn_proc);

        // LOGK("run here %s:%d offset %d time %s", __func__, __LINE__, pFpnProc->_fpn_attr->stAuto.sta.sw_fpnSw_off, timeString());

#if RKAIQ_FPNSW_DEBUG
        }

        else {

        // // 10bit -> 16bit -> +10 -> 10bit


        uint8_t* srcRaw = (uint8_t*)AiqV4l2Buffer_getExpbufUsrptr(msg.buf_s);
        // uint16_t* output_pix = (uint16_t*)g_output_buf;


        // for (int i = 0; i < pFpnProc->_height; i++) {
        //     for (int j = 0; j < pFpnProc->_width; j += 4, output_pix += 4, srcRaw += 5) {
        //         output_pix[0] = ((((srcRaw[0] & 0xff) >> 0) | ((srcRaw[1] & 0x03) << 8)));
        //         output_pix[1] = ((((srcRaw[1] & 0xfc) >> 2) | ((srcRaw[2] & 0x0f) << 6)));
        //         output_pix[2] = ((((srcRaw[2] & 0xf0) >> 4) | ((srcRaw[3] & 0x3f) << 4)));
        //         output_pix[3] = ((((srcRaw[3] & 0xc0) >> 6) | ((srcRaw[4] & 0xff) << 2)));

        //         // srcRaw[0] = output_pix[0] & 0xff;
        //         // srcRaw[1] = ((output_pix[0] >> 8) & 0x03) | ((output_pix[1] & 0x3f) << 2);
        //         // srcRaw[2] = ((output_pix[1] >> 6) & 0x0f) | ((output_pix[2] & 0x0f) << 4);
        //         // srcRaw[3] = ((output_pix[2] >> 4) & 0x3f) | ((output_pix[3] & 0x03) << 6);
        //         // srcRaw[4] = (output_pix[3] >> 2) & 0xff;
        //     }
        //     srcRaw = srcRaw + g_line_offset;
        // }

        // for (int i = 0; i < pFpnProc->_height; i++) {
        //     for (int j = 0; j < pFpnProc->_width; j += 8, srcRaw += 10) {
        //         uint16_t output_pix[8];

        //         // output_pix[0] = (((srcRaw[0] >> 0) | (srcRaw[1] << 8))) & 0x3ff;
        //         // output_pix[1] = (((srcRaw[1] >> 2) | (srcRaw[2] << 6))) & 0x3ff;
        //         // output_pix[2] = (((srcRaw[2] >> 4) | (srcRaw[3] << 4))) & 0x3ff;
        //         // output_pix[3] = (((srcRaw[3] >> 6) | (srcRaw[4] << 2))) & 0x3ff;
        //         // output_pix[4] = (((srcRaw[5] >> 0) | (srcRaw[6] << 8))) & 0x3ff;
        //         // output_pix[5] = (((srcRaw[6] >> 2) | (srcRaw[7] << 6))) & 0x3ff;
        //         // output_pix[6] = (((srcRaw[7] >> 4) | (srcRaw[8] << 4))) & 0x3ff;
        //         // output_pix[7] = (((srcRaw[8] >> 6) | (srcRaw[9] << 2))) & 0x3ff;

        //         output_pix[0] = (*(uint16_t*)(srcRaw)) & 0x3ff;
        //         output_pix[1] = ((*(uint16_t*)(srcRaw + 1)) >> 2) & 0x3ff;
        //         output_pix[2] = ((*(uint16_t*)(srcRaw + 2)) >> 4) & 0x3ff;
        //         output_pix[3] = (*(uint16_t*)(srcRaw + 3)) >> 6;
        //         output_pix[4] = (*(uint16_t*)(srcRaw + 5)) >> 0;
        //         output_pix[5] = ((*(uint16_t*)(srcRaw + 6)) >> 2) & 0x3ff;
        //         output_pix[6] = ((*(uint16_t*)(srcRaw + 7)) >> 4) & 0x3ff;
        //         output_pix[7] = (*(uint16_t*)(srcRaw + 8)) >> 6;
                

        //         output_pix[0] += 10;
        //         output_pix[1] += 10;
        //         output_pix[2] += 10;
        //         output_pix[3] += 10;
        //         output_pix[4] += 10;
        //         output_pix[5] += 10;
        //         output_pix[6] += 10;
        //         output_pix[7] += 10;

        //         // neon_unpack_10bit_8pix(srcRaw, output_pix);
        //         // neon_add10_1pix(output_pix, 8);
        //         srcRaw[0] = output_pix[0] & 0xff;
        //         srcRaw[1] = ((output_pix[0] >> 8) & 0x03) | ((output_pix[1] & 0x3f) << 2);
        //         srcRaw[2] = ((output_pix[1] >> 6) & 0x0f) | ((output_pix[2] & 0x0f) << 4);
        //         srcRaw[3] = ((output_pix[2] >> 4) & 0x3f) | ((output_pix[3] & 0x03) << 6);
        //         srcRaw[4] = (output_pix[3] >> 2) & 0xff;
        //         srcRaw[5] = output_pix[4] & 0xff;
        //         srcRaw[6] = ((output_pix[4] >> 8) & 0x03) | ((output_pix[5] & 0x3f) << 2);
        //         srcRaw[7] = ((output_pix[5] >> 6) & 0x0f) | ((output_pix[6] & 0x0f) << 4);
        //         srcRaw[8] = ((output_pix[6] >> 4) & 0x3f) | ((output_pix[7] & 0x03) << 6);
        //         srcRaw[9] = (output_pix[7] >> 2) & 0xff;
        //     }
        //     // for (int j = 0; j < pFpnProc->_width; j += 4, output_add10 += 4, srcRaw += 5) {
        //     //     output_raw10[0] = output_add10[0] & 0xff;
        //     //     output_raw10[1] = ((output_add10[0] >> 8) & 0x03) | ((output_pix[1] & 0x3f) << 2);
        //     //     output_raw10[2] = ((output_add10[1] >> 6) & 0x0f) | ((output_pix[2] & 0x0f) << 4);
        //     //     output_raw10[3] = ((output_add10[2] >> 4) & 0x3f) | ((output_pix[3] & 0x03) << 6);
        //     //     output_raw10[4] = (output_add10[3] >> 2) & 0xff;
        //     // }
        //     srcRaw = srcRaw + g_line_offset;
        // }

        // for (int i = 0; i < pFpnProc->_height; i++) {
        //     for (int j = 0; j < pFpnProc->_width; j += 8, srcRaw += 10) {
        //         uint16_t output_pix[8];

        //         // 10bit raw -> 16bit raw
        //         neon_unpack_10bit_8pix(srcRaw, output_pix);


        //         // pix + 10
        //         // neon_add10_1pix(output_pix, 8);
        //         output_pix[0] += 10;
        //         output_pix[1] += 10;
        //         output_pix[2] += 10;
        //         output_pix[3] += 10;
        //         output_pix[4] += 10;
        //         output_pix[5] += 10;
        //         output_pix[6] += 10;
        //         output_pix[7] += 10;

        //         // 16bit raw -> 10bit raw
        //         srcRaw[0] = output_pix[0] & 0xff;
        //         srcRaw[1] = ((output_pix[0] >> 8) & 0x03) | ((output_pix[1] & 0x3f) << 2);
        //         srcRaw[2] = ((output_pix[1] >> 6) & 0x0f) | ((output_pix[2] & 0x0f) << 4);
        //         srcRaw[3] = ((output_pix[2] >> 4) & 0x3f) | ((output_pix[3] & 0x03) << 6);
        //         srcRaw[4] = (output_pix[3] >> 2) & 0xff;
        //         srcRaw[5] = output_pix[4] & 0xff;
        //         srcRaw[6] = ((output_pix[4] >> 8) & 0x03) | ((output_pix[5] & 0x3f) << 2);
        //         srcRaw[7] = ((output_pix[5] >> 6) & 0x0f) | ((output_pix[6] & 0x0f) << 4);
        //         srcRaw[8] = ((output_pix[6] >> 4) & 0x3f) | ((output_pix[7] & 0x03) << 6);
        //         srcRaw[9] = (output_pix[7] >> 2) & 0xff;
        //     }
        //     srcRaw = srcRaw + g_line_offset;
        // }

        // for (int j = 0; j < pFpnProc->_width; j += 4, output_pix += 4, srcRaw += 5) {
        //     for (int i = 0; i < pFpnProc->_height; i++) {
        //         output_pix[0] = ((((srcRaw[0] & 0xff) >> 0) | ((srcRaw[1] & 0x03) << 8)));
        //         output_pix[1] = ((((srcRaw[1] & 0xfc) >> 2) | ((srcRaw[2] & 0x0f) << 6)));
        //         output_pix[2] = ((((srcRaw[2] & 0xf0) >> 4) | ((srcRaw[3] & 0x3f) << 4)));
        //         output_pix[3] = ((((srcRaw[3] & 0xc0) >> 6) | ((srcRaw[4] & 0xff) << 2)));
        //     }
        // }

        // LOGK("run here %s:%d time %s", __func__, __LINE__, timeString());


        // LOGK("run here %s:%d time %s", __func__, __LINE__, timeString());

        // srcRaw = (uint8_t*)AiqV4l2Buffer_getExpbufUsrptr(msg.buf_s);
        // output_pix = (uint16_t*)g_output_buf;

        // for (int i = 0; i < pFpnProc->_height; i++) {
        //     for (int j = 0; j < pFpnProc->_width; j += 4, output_pix += 4, srcRaw += 5) {
        //         srcRaw[0] = output_pix[0] & 0xff;
        //         srcRaw[1] = ((output_pix[0] >> 8) & 0x03) | ((output_pix[1] & 0x3f) << 2);
        //         srcRaw[2] = ((output_pix[1] >> 6) & 0x0f) | ((output_pix[2] & 0x0f) << 4);
        //         srcRaw[3] = ((output_pix[2] >> 4) & 0x3f) | ((output_pix[3] & 0x03) << 6);
        //         srcRaw[4] = (output_pix[3] >> 2) & 0xff;
        //     }
        //     srcRaw = srcRaw + g_line_offset;
        // }

        uint32_t stride = (pFpnProc->_width * 10 /8 + 255) / 256 * 256;
        uint8_t* src_buf = srcRaw;
        for (int i = 0; i < pFpnProc->_height; i++) {
            srcRaw = src_buf + stride * i;
            for (int j = 0; j < pFpnProc->_width; j += 32, srcRaw += 40) {
                process_two_groups((uint32_t*)srcRaw);
            }
        }

        // 16bit -> +10
        // uint16_t* output_pix = (uint16_t*)AiqV4l2Buffer_getExpbufUsrptr(msg.buf_s);

        // for (int i = 0; i < pFpnProc->_height; i++) {
        //     for (int j = 0; j < pFpnProc->_width; j++= 8, output_pix += 8) {
        //         output_pix[0] += 10;
        //         output_pix[1] += 10;
        //         output_pix[2] += 10;
        //         output_pix[3] += 10;
        //         output_pix[4] += 10;
        //         output_pix[5] += 10;
        //         output_pix[6] += 10;
        //         output_pix[7] += 10;
        //     }
        // }

        // neon_add10_1pix(output_pix, pFpnProc->_height * pFpnProc->_width);

        // process_pixels_neon(srcRaw, output_pix, pFpnProc->_width, pFpnProc->_height, g_line_offset);

        }
        LOGK("run here %s:%d time %s", __func__, __LINE__, timeString());
#endif
    }

    if (pFpnProc->_memc_manager && pFpnProc->_send_sync_buf_func) {
        pFpnProc->_send_sync_buf_func(pFpnProc->_memc_manager, msg.buf_s, msg.buf_m, msg.buf_l);
    } else if (pFpnProc->_proc_stream) {
        AiqRawStreamProcUnit_send_sync_buf(pFpnProc->_proc_stream, msg.buf_s, msg.buf_m, msg.buf_l);
    } else {
        LOGE_ANALYZER("no proc unit %s !", __func__);
    }

    if (msg.expParam) {
        AIQ_REF_BASE_UNREF(&msg.expParam->_base._ref_base);
    }

    if (msg.buf_s) AiqVideoBuffer_unref(msg.buf_s);
    if (msg.buf_m) AiqVideoBuffer_unref(msg.buf_m);
    if (msg.buf_l) AiqVideoBuffer_unref(msg.buf_l);

    return true;
}

static XCamReturn FpnSwStreamProcThread_init(AiqFpnSwStreamProcThread_t* pHdlTh, AiqFpnSwStreamProcUnit_t* pFpnProc)
{
    if (!pHdlTh || !pFpnProc)
        return XCAM_RETURN_ERROR_PARAM;

    aiqMutex_init(&pHdlTh->_mutex);
    aiqCond_init(&pHdlTh->_cond);

    pHdlTh->mFpnProc = pFpnProc;

    AiqListConfig_t msgqCfg;
    msgqCfg._name      = "fpnThMsg";
    msgqCfg._item_nums = 2;
    msgqCfg._item_size = sizeof(fpn_proc_msg_t);
    pHdlTh->mMsgsQueue = aiqList_init(&msgqCfg);
    if (!pHdlTh->mMsgsQueue) {
        LOGE_ANALYZER("init %s error", msgqCfg._name);
        return XCAM_RETURN_ERROR_PARAM;
    }

    pHdlTh->_base = aiqThread_init("fpnSwStreamProcTh", fpnSwStreamProcTh_func, pHdlTh);
    if (!pHdlTh->_base) {
        LOGW_ANALYZER("create thread failed %s !", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn FpnSwStreamProcThread_deinit(AiqFpnSwStreamProcThread_t* pHdlTh)
{
    if (!pHdlTh)
        return XCAM_RETURN_ERROR_PARAM;

    if (pHdlTh->_base) aiqThread_deinit(pHdlTh->_base);
    if (pHdlTh->mMsgsQueue) aiqList_deinit(pHdlTh->mMsgsQueue);

    aiqMutex_deInit(&pHdlTh->_mutex);
    aiqCond_deInit(&pHdlTh->_cond);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn FpnSwStreamProcThread_start(AiqFpnSwStreamProcThread_t* pHdlTh)
{
    if (!pHdlTh)
        return XCAM_RETURN_ERROR_PARAM;

    pHdlTh->bQuit = false;
    if (!aiqThread_start(pHdlTh->_base)) {
        LOGW_CAMHW("start thread failed %s !", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn FpnSwStreamProcThread_stop(AiqFpnSwStreamProcThread_t* pHdlTh)
{
    if (!pHdlTh)
        return XCAM_RETURN_ERROR_PARAM;

    aiqMutex_lock(&pHdlTh->_mutex);
    pHdlTh->bQuit = true;
    aiqMutex_unlock(&pHdlTh->_mutex);
    aiqCond_broadcast(&pHdlTh->_cond);
    aiqThread_stop(pHdlTh->_base);

    return XCAM_RETURN_NO_ERROR;
}

static bool FpnSwStreamProcThread_push_msg(AiqFpnSwStreamProcThread_t* pHdlTh, fpn_proc_msg_t* msg)
{
    if (!pHdlTh || !msg || !pHdlTh->_base->_started)
        return false;

    int ret = aiqList_push(pHdlTh->mMsgsQueue, msg);
    if (ret) {
        LOGW_CAMHW("push msg failed %s !", __func__);
        return false;
    }

    aiqCond_broadcast(&pHdlTh->_cond);

    return true;
}

XCamReturn AiqFpnSwStreamProcUnit_set_fpnSw_attrib(AiqFpnSwStreamProcUnit_t* pFpnProc, fpnSw_api_attrib_t* attr)
{
    if (!pFpnProc)
        return XCAM_RETURN_ERROR_PARAM;

    if (!attr) {
        LOGE_CAMHW("fpnSw attrib is NULL");
        return XCAM_RETURN_ERROR_PARAM;
    }

    pFpnProc->_fpn_attr = attr;

    // LOGK("run here en %d bypass %d opMode %d sta.off %d auto off %d", attr->en, attr->bypass, attr->opMode,
    //      attr->stMan.sta.sw_fpnSw_off, attr->stAuto.sta.sw_fpnSw_off);
    // LOGK("run here %s:%d offset %d time %s", __func__, __LINE__, pFpnProc->_fpn_attr->stMan.sta.sw_fpnSw_off, timeString());

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqFpnSwStreamProcUnit_set_csi_gain_max(AiqFpnSwStreamProcUnit_t* pFpnProc, float csi_again_max, float csi_dgain_max)
{
    if (!pFpnProc)
        return XCAM_RETURN_ERROR_PARAM;

    pFpnProc->_csi_again_max = csi_again_max;
    pFpnProc->_csi_dgain_max = csi_dgain_max;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqFpnSwStreamProcUnit_setCamPhyId(AiqFpnSwStreamProcUnit_t* pFpnProc, int cam_phy_id)
{
    if (!pFpnProc)
        return XCAM_RETURN_ERROR_PARAM;

    pFpnProc->mCamPhyId = cam_phy_id;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
AiqFpnSwStreamProcUnit_setSwStreamInfo(AiqFpnSwStreamProcUnit_t* pFpnProc, void* stream_ctx, rawStream_send_sync_buf_func func)
{
    if (!pFpnProc || !stream_ctx || !func)
        return XCAM_RETURN_ERROR_PARAM;

    pFpnProc->_memc_manager = stream_ctx;
    pFpnProc->_send_sync_buf_func = func;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqFpnSwStreamProcUnit_set_proc_stream(AiqFpnSwStreamProcUnit_t* pFpnProc, AiqRawStreamProcUnit_t* proc_stream)
{
    if (!pFpnProc || !proc_stream)
        return XCAM_RETURN_ERROR_PARAM;

    pFpnProc->_proc_stream = proc_stream;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqFpnSwStreamProcUnit_set_fpn_offset(AiqFpnSwStreamProcUnit_t* pFpnProc, int fpn_offset)
{
    if (!pFpnProc)
        return XCAM_RETURN_ERROR_PARAM;

    pFpnProc->_fpn_offset = fpn_offset;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqFpnSwStreamProcUnit_init(AiqFpnSwStreamProcUnit_t* pFpnProc, void *sns_dev)
{
    if (!pFpnProc)
        return XCAM_RETURN_ERROR_PARAM;

    memset(&pFpnProc->_fpn_param, 0, sizeof(rk_fpn_correction_param_t));

    pFpnProc->_fpn_en        = false;
    pFpnProc->_csi_again_max = 1.0;
    pFpnProc->_csi_dgain_max = 1.0;
    pFpnProc->mCamPhyId      = 0;
    pFpnProc->_width         = 0;
    pFpnProc->_height        = 0;
    pFpnProc->_proc_stream   = NULL;
    pFpnProc->_memc_manager  = NULL;
    pFpnProc->_fpn_offset    = 128;

    pFpnProc->_procTh = (AiqFpnSwStreamProcThread_t*)aiq_mallocz(sizeof(AiqFpnSwStreamProcThread_t));
    if (!pFpnProc->_procTh) {
        LOGE_ANALYZER("init %s error", __func__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    XCamReturn ret = FpnSwStreamProcThread_init(pFpnProc->_procTh, pFpnProc);
    if (ret) {
        LOGE_ANALYZER("init %s error", __func__);
        return ret;
    }

    pFpnProc->_sensor_dev = sns_dev;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqFpnSwStreamProcUnit_deinit(AiqFpnSwStreamProcUnit_t* pFpnProc)
{
    if (!pFpnProc)
        return XCAM_RETURN_ERROR_PARAM;

    if (pFpnProc->_procTh) {
        FpnSwStreamProcThread_deinit(pFpnProc->_procTh);
        aiq_free(pFpnProc->_procTh);
        pFpnProc->_procTh = NULL;
    }

#if RKAIQ_FPNSW_DEBUG
    if (g_output_buf) {
        free(g_output_buf);
        g_output_buf = NULL;
    }
#endif

    rk_fpn_correction_deinit(&pFpnProc->_fpn_param);

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqFpnSwStreamProcUnit_prepare(AiqFpnSwStreamProcUnit_t* pFpnProc, uint32_t width, uint32_t height)
{
    if (!pFpnProc)
        return XCAM_RETURN_ERROR_PARAM;

    pFpnProc->_width = width;
    pFpnProc->_height = height;

#if RKAIQ_FPNSW_DEBUG
    if (!g_output_buf) {
        g_output_buf = malloc(pFpnProc->_width * pFpnProc->_height * 2);
        memset(g_output_buf, 0, pFpnProc->_width * pFpnProc->_height * 2);
        LOGK("run here %s:%d %p", __func__, __LINE__, g_output_buf);
    }

    g_line_offset = (size_t)((pFpnProc->_width * 10 / 8 + 255) / 256 * 256) - (pFpnProc->_width * 10 / 8);
#endif

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqFpnSwStreamProcUnit_start(AiqFpnSwStreamProcUnit_t* pFpnProc)
{
    if (!pFpnProc)
        return XCAM_RETURN_ERROR_PARAM;

    if (strlen(pFpnProc->_fpn_attr->stAuto.sta.sw_fpnSw_raw_path) > 0 && !pFpnProc->_fpn_init) {
        rk_fpn_correction_param_t fpn_proc;
        fpn_proc.raw_wid = pFpnProc->_width;
        fpn_proc.raw_hgt = pFpnProc->_height;
        fpn_proc.max_sensor_gain = pFpnProc->_csi_again_max * pFpnProc->_csi_dgain_max;
        fpn_proc.current_raw_ptr = NULL;
        sprintf(fpn_proc.fpn_path, "%s", pFpnProc->_fpn_attr->stAuto.sta.sw_fpnSw_raw_path);
        rk_fpn_correction_init(&fpn_proc);
        pFpnProc->_fpn_init = true;
    } else {
        LOGE_CAMHW("fpnSw raw path %s is empty, please set it first!", pFpnProc->_fpn_attr->stAuto.sta.sw_fpnSw_raw_path);
    }

    return FpnSwStreamProcThread_start(pFpnProc->_procTh);
}

XCamReturn AiqFpnSwStreamProcUnit_stop(AiqFpnSwStreamProcUnit_t* pFpnProc)
{
    if (!pFpnProc)
        return XCAM_RETURN_ERROR_PARAM;

    return FpnSwStreamProcThread_stop(pFpnProc->_procTh);
}

XCamReturn AiqFpnSwStreamProcUnit_send_sync_buf(void* fpn_ctx, AiqV4l2Buffer_t* buf_s, AiqV4l2Buffer_t* buf_m, AiqV4l2Buffer_t* buf_l)
{
    if (!fpn_ctx)
        return XCAM_RETURN_ERROR_PARAM;

    AiqFpnSwStreamProcUnit_t* pFpnProc = (AiqFpnSwStreamProcUnit_t*)fpn_ctx;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (buf_s) AiqV4l2Buffer_ref(buf_s);
    if (buf_m) AiqV4l2Buffer_ref(buf_m);
    if (buf_l) AiqV4l2Buffer_ref(buf_l);

#if RKAIQ_FPNSW_DEBUG
    pFpnProc->_fpn_en = _get_int_default_frome_file("/data/fpn_en");
#endif

    if (pFpnProc->_procTh && pFpnProc->_procTh->_base->_started && pFpnProc->_fpn_attr->en) {
        fpn_proc_msg_t msg;
        msg.buf_s = buf_s;
        msg.buf_m = buf_m;
        msg.buf_l = buf_l;

        if (pFpnProc->_sensor_dev) {
            AiqSensorHw_t* mSensorSubdev = (AiqSensorHw_t*)pFpnProc->_sensor_dev;
            msg.expParam = mSensorSubdev->getEffectiveExpParams(mSensorSubdev,
                                                                AiqV4l2Buffer_getSequence(buf_s));
        }

#if RKAIQ_FPNSW_DEBUG
        if (AiqV4l2Buffer_getSequence(buf_s) % 50 == 0) {
            fpn_switch = !!_get_int_default_frome_file("/data/fpn_switch");
        }
#endif

        if (!FpnSwStreamProcThread_push_msg(pFpnProc->_procTh, &msg)) {
            if (buf_s) AiqVideoBuffer_unref(buf_s);
            if (buf_m) AiqVideoBuffer_unref(buf_m);
            if (buf_l) AiqVideoBuffer_unref(buf_l);
            ret = XCAM_RETURN_ERROR_PARAM;
        }
        return ret;

    } else if (pFpnProc->_memc_manager && pFpnProc->_send_sync_buf_func) {
        pFpnProc->_send_sync_buf_func(pFpnProc->_memc_manager, buf_s, buf_m, buf_l);
    } else if (pFpnProc->_proc_stream) {
        AiqRawStreamProcUnit_send_sync_buf(pFpnProc->_proc_stream, buf_s, buf_m, buf_l);
    } else {
        LOGE_ANALYZER("no proc unit %s !", __func__);
    }

    if (buf_s) AiqV4l2Buffer_unref(buf_s);
    if (buf_m) AiqV4l2Buffer_unref(buf_m);
    if (buf_l) AiqV4l2Buffer_unref(buf_l);

    return ret;
}
