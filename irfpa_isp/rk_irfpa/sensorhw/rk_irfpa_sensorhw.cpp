#include "rk_irfpa_sensorhw.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <math.h>

typedef struct {
    const char *name;
    rk_irfpa_sensorhw_config_t cfg;
} sensor_cfg_map_t;

static sensor_cfg_map_t sensor_cfg_map[8];
static uint8_t sensor_cfg_map_cnt;

static void __attribute__((constructor(101))) init_sensor_cfg_map(void)
{
    sensor_cfg_map_cnt = 0;
}

void rk_irfpa_sensorhw_register(const char *name, rk_irfpa_sensorhw_config_t cfg)
{
    sensor_cfg_map[sensor_cfg_map_cnt].cfg = cfg;
    sensor_cfg_map[sensor_cfg_map_cnt].name = name;
    sensor_cfg_map_cnt++;
}

typedef struct {
    rk_irfpa_sensorhw_ctx_t ctx;
    rk_irfpa_streamhw_ctx_t *streamhw;
    rk_irfpa_log_ctx_t *log;
    bool initial = false;
} sensorhw_priv_t;

static bool sensorhw_delay(sensorhw_priv_t *priv, int delay)
{
    bool ret = false;
    rk_irfpa_log_trace(priv->log, "%s: %d", __func__, delay);
    if (delay > 0) {
        while (delay--) {
            ret = rk_irfpa_streamhw_getFrame(priv->streamhw);
            if (!ret)
                return false;
        }
    }
    return true;
}

static bool sensorhw_shutter(sensorhw_priv_t *priv, bool open)
{
    bool ret = false;
    rk_irfpa_sensorhw_shutter_t *shutter = &priv->ctx.cfg.shutter;
    if (shutter->ctrl == NULL)
        return false;

    if (open) {
        return rk_irfpa_streamhw_setControl(priv->streamhw, shutter->ctrl, shutter->open_value);
    } else {
        return rk_irfpa_streamhw_setControl(priv->streamhw, shutter->ctrl, shutter->close_value);
    }

    return ret;
}

static int sensorhw_calc_ave_center(sensorhw_priv_t *priv, int16_t rw, int16_t rh)
{
    rk_irfpa_streamhw_ctx_t *streamhw = priv->streamhw;
    uint16_t *p = streamhw->res.y16;
    uint16_t w = streamhw->info.vi_width;
    uint16_t h = streamhw->info.vi_height;

    uint16_t xs = (w - rw) / 2;
    uint16_t ys = (h - rw) / 2;

    uint16_t i, j;
    int count = 0;
    uint32_t sum = 0;

    for (i=xs; i<xs+rw; i++) {
        for (j=ys; j<ys+rh; j++) {
            uint16_t pix = p[j*w + i];
            count ++;
            sum += pix;
        }
    }
    return sum / count;
}

static int16_t sensorhw_calc_ave_step(sensorhw_priv_t *priv, int16_t step)
{
    rk_irfpa_streamhw_ctx_t *streamhw = priv->streamhw;
    uint16_t *p = streamhw->res.y16;
    uint16_t w = streamhw->info.vi_width;
    uint16_t h = streamhw->info.vi_height;

    int count = 0;
    uint32_t sum = 0;

    for (int i=0; i<w*h; i+=step) {
        uint16_t pix = p[i];
        count ++;
        sum += pix;
    }
    return sum / count;
}

static int16_t sensorhw_calc_ave(sensorhw_priv_t *priv)
{
    rk_irfpa_streamhw_ctx_t *streamhw = priv->streamhw;
    return sensorhw_calc_ave_step(priv, 1);
}

static float sensorhw_calc_std_step(sensorhw_priv_t *priv, int16_t step)
{
    rk_irfpa_streamhw_ctx_t *streamhw = priv->streamhw;
    uint16_t *p = streamhw->res.y16;
    uint16_t w = streamhw->info.vi_width;
    uint16_t h = streamhw->info.vi_height;

    int16_t ave = sensorhw_calc_ave_step(priv, step);

    int count = 0;
    double sum = 0;

    for (int i=0; i<w*h; i+=step) {
        uint16_t pix = p[i];
        int diff = pix - ave;
        int diff2 = diff * diff;
        sum += diff2;
        count ++;
    }

    sum /= count;
    float res = sqrt(sum);

    return res;
}

static float sensorhw_calc_std(sensorhw_priv_t *priv)
{
    return sensorhw_calc_std_step(priv, 1);
}

static bool sensorhw_adjust_gain(sensorhw_priv_t *priv, rk_irfpa_sensorhw_gain_t *ctrl)
{
    bool ret;
    int max = ctrl->adjust.max;
    int min = ctrl->adjust.min;
    int step = ctrl->adjust.step;
    int delay = ctrl->adjust.delay;
    int max_adj_cnt = max - min;
    int upper = ctrl->target.upper;
    int lower = ctrl->target.lower;
    int current_val = 0;
    rk_irfpa_adjust_mode_e mode = ctrl->adjust.mode;

    if (ctrl->ctrl == NULL)
        return false;

    ret = rk_irfpa_streamhw_getControl(priv->streamhw, ctrl->ctrl, &current_val);
    if (!ret)
        return false;

    rk_irfpa_log_debug(priv->log, "%s: adjust %s, current %d", __func__, ctrl->ctrl, current_val);

    while (max_adj_cnt --) {
        ret = rk_irfpa_streamhw_getFrame(priv->streamhw);
        if (!ret)
            return false;

        int ave = sensorhw_calc_ave_center(priv, 128, 128);

        if (ave >= lower && ave <= upper)
            break;

        if (ave > upper) {
            if (mode == RK_IRFPA_ADJUST_INCREASE_HIGHER)
                current_val -= step;
            else
                current_val += step;
        }

        if (ave < lower) {
            if (mode == RK_IRFPA_ADJUST_INCREASE_HIGHER)
                current_val += step;
            else
                current_val -= step;
        }

        rk_irfpa_log_trace(priv->log, "%s: ave %d, next %d", __func__, ave, current_val);

        if (current_val >= max || current_val <= min)
            return false;

        ret = rk_irfpa_streamhw_setControl(priv->streamhw, ctrl->ctrl, current_val);
        if (!ret)
            return false;

        ret = sensorhw_delay(priv, delay);
        if (!ret)
            return false;
    }

    return true;
}

static bool sensorhw_adjust_ooc_quick(sensorhw_priv_t *priv)
{
    bool ret = true;

    rk_irfpa_streamhw_ctx_t *streamhw = priv->streamhw;
    rk_irfpa_sensorhw_ooc_t *ooc = &priv->ctx.cfg.ooc;

    uint16_t w = streamhw->info.vi_width;
    uint16_t h = streamhw->info.vi_height;
    uint8_t *oocbuf = streamhw->oocbuf;
    uint16_t ooc_w = streamhw->info.ooc_width;
    uint16_t ooc_h = streamhw->info.ooc_height;
    int upper = ooc->target.upper;
    int lower = ooc->target.lower;
    int ix = ooc->start_x;
    int iy = ooc->start_y;
    int max = ooc->adjust.max;
    int min = ooc->adjust.min;
    int step = ooc->adjust.step;
    int delay = ooc->adjust.delay;
    int init_val = ooc->init_val;
    uint8_t ooc_step = (max - min) / 2;

    // clear
    memset(oocbuf, init_val, ooc_w * ooc_h);
    ret = rk_irfpa_streamhw_flush_ooc(streamhw);
    if (!ret)
        goto DONE;
    ret = sensorhw_delay(priv, delay);
    if (!ret)
        goto DONE;

    for (uint8_t s=0; s<ooc->adj_step; s++) {
        ooc_step /= 2;
        if (ooc_step == 0)
            ooc_step = 1;
        for (int j=0; j<h; j++) {
            for (int i=0; i<w; i++) {
                int16_t pix = streamhw->res.y16[i+j*w];
                uint8_t occ = oocbuf[i + ix + w * (j+iy)];

                if (pix >= upper) {
                    if (occ + ooc_step < max) {
                        occ += ooc_step;
                    } else {
                        occ = max;
                    }
                }
                if (pix <= lower) {
                    if (occ - ooc_step > min) {
                        occ -= ooc_step;
                    } else {
                        occ = min;
                    }
                }

                oocbuf[i + w * (j+4)] = occ;
            }
        }

        ret = rk_irfpa_streamhw_flush_ooc(streamhw);
        if (!ret)
            goto DONE;
        ret = sensorhw_delay(priv, delay);
        if (!ret)
            goto DONE;

        float std = sensorhw_calc_std(priv);
        rk_irfpa_log_debug(priv->log, "%s: OOC step %d, std %f", __func__, s, std);
    }

DONE:
    return ret;
}

static bool sensorhw_adjust_ooc(sensorhw_priv_t *priv)
{
    bool ret = true;
    rk_irfpa_streamhw_ctx_t *streamhw = priv->streamhw;
    rk_irfpa_sensorhw_ooc_t *ooc = &priv->ctx.cfg.ooc;

    int max = ooc->adjust.max;
    int min = ooc->adjust.min;
    int step = ooc->adjust.step;
    int delay = ooc->adjust.delay;
    int upper = ooc->target.upper;
    int lower = ooc->target.lower;
    int target_ave = (upper + lower) / 2;
    uint16_t w = streamhw->info.vi_width;
    uint16_t h = streamhw->info.vi_height;
    int ooclen = streamhw->info.ooc_width * streamhw->info.ooc_height;
    int ix = ooc->start_x;
    int iy = ooc->start_y;

    int16_t *diff_tbl = (int16_t *)malloc(w * h * sizeof(int16_t));
    uint8_t *oocbuf = (uint8_t *)malloc(w * h * sizeof(uint8_t));
    for (int i=0; i<w*h; i++) {
        diff_tbl[i] = 16383;
        oocbuf[i] = min;
    }

    for (uint8_t val=min; val<max; val+=step) {

        //printf("set ooc to %d\n", val);
        memset(streamhw->oocbuf, val, ooclen);
        ret = rk_irfpa_streamhw_flush_ooc(streamhw);
        if (!ret)
            goto DONE;

        ret = sensorhw_delay(priv, delay);
        if (!ret)
            goto DONE;

        ret = rk_irfpa_streamhw_getFrame(priv->streamhw);
        if (!ret)
            return false;

        for (int j=0; j<h; j++) {
            for (int i=0; i<w; i++) {
                int16_t pix = streamhw->res.y16[i+j*w];
                int16_t diff = pix - target_ave;
                if (diff < 0)
                    diff = -1 * diff;

                if (diff < diff_tbl[i+j*w]) {
                    diff_tbl[i+j*w] = diff;
                    oocbuf[i+j*w] = val;
                }
            }
        }
    }

    for (int j=0; j<h; j++) {
        for (int i=0; i<w; i++) {
            streamhw->oocbuf[i+ix + w * (j+iy)] = oocbuf[i+j*w];
        }
    }

    ret = rk_irfpa_streamhw_flush_ooc(streamhw);
    if (!ret)
        goto DONE;

    ret = sensorhw_delay(priv, delay);
    if (!ret)
        goto DONE;
DONE:
    free(diff_tbl);
    free(oocbuf);
    return ret;
}

static bool sensorhw_update_base(sensorhw_priv_t *priv)
{
    bool ret;
    rk_irfpa_streamhw_ctx_t *streamhw = priv->streamhw;
    rk_irfpa_sensorhw_base_t *base = &priv->ctx.cfg.base;
    uint16_t w = streamhw->info.vi_width;
    uint16_t h = streamhw->info.vi_height;
    uint16_t *basebuf = priv->ctx.base_buf;

    ret = rk_irfpa_streamhw_getFrame(priv->streamhw);
    if (!ret)
        return false;
    memcpy(basebuf, streamhw->res.y16, w * h * sizeof(uint16_t));

    for (uint8_t s=0; s<base->ave_num; s++) {
        ret = rk_irfpa_streamhw_getFrame(priv->streamhw);
        if (!ret)
            return false;

        for (int i = 0; i < w*h; i++) {
            uint16_t pix1 = streamhw->res.y16[i];
            uint16_t pix2 = basebuf[i];
            basebuf[i] = (pix1 + pix2) / 2;
        }
    }

    int count = 0;
    uint32_t sum = 0;
    int step = 2;
    for (int i=0; i<w*h; i+=step) {
        uint16_t pix = basebuf[i];
        count ++;
        sum += pix;
    }
    priv->ctx.base_ave = sum / count;
    rk_irfpa_log_debug(priv->log, "%s: base ave %d", __func__, priv->ctx.base_ave);
    return true;
}

static bool sensorhw_do_step(sensorhw_priv_t *priv, uint8_t step, uint8_t delay)
{
    bool ret = false;
    int16_t ave;
    float std;
    switch (step) {
    case RK_IRFPA_SENSORHW_OP_NONE:
        ret = true;
        break;
    case RK_IRFPA_SENSORHW_OP_SHUTTER_CLOSE:
        rk_irfpa_log_info(priv->log, "SHUTTER_CLOSE");
        ret = sensorhw_shutter(priv, false);
        break;
    case RK_IRFPA_SENSORHW_OP_SHUTTER_OPEN:
        rk_irfpa_log_info(priv->log, "SHUTTER_OPEN");
        ret = sensorhw_shutter(priv, true);
        break;
    case RK_IRFPA_SENSORHW_OP_GAIN1:
        ret = sensorhw_adjust_gain(priv, &priv->ctx.cfg.gain1);
        ave = sensorhw_calc_ave(priv);
        rk_irfpa_log_info(priv->log, "HW_CTRL1 done, ave %d", ave);
        break;
    case RK_IRFPA_SENSORHW_OP_GAIN2:
        ret = sensorhw_adjust_gain(priv, &priv->ctx.cfg.gain2);

        ave = sensorhw_calc_ave(priv);
        std = sensorhw_calc_std(priv);
        rk_irfpa_log_info(priv->log, "HW_CTRL2 done, ave %d, std %f", ave, std);
        break;
    case RK_IRFPA_SENSORHW_OP_OOC:
        ret = sensorhw_adjust_ooc_quick(priv);
        break;
    case RK_IRFPA_SENSORHW_OP_BASE:
        ret = sensorhw_update_base(priv);
        break;
    default:
        ret = false;
        break;
    }
    if (!ret) {
        rk_irfpa_log_error(priv->log, "%s: step %d false!", __func__, step);
        return false;
    }

    return sensorhw_delay(priv, delay);
}

static bool sensorhw_do_steps(sensorhw_priv_t *priv, const rk_irfpa_sensorhw_op_t *steps)
{
    if (steps == NULL)
        return false;

    uint8_t i = 0;
    bool ret = false;
    while (steps[i].step != RK_IRFPA_SENSORHW_OP_DONE) {
        ret = sensorhw_do_step(priv, steps[i].step, steps[i].delay);
        if (!ret)
            return false;
        i ++;
    }

    return true;
}

static bool sensorhw_initial(sensorhw_priv_t *priv)
{
    bool ret = false;
    rk_irfpa_sensorhw_initial_t *initial = &priv->ctx.cfg.initial;

    rk_irfpa_log_info(priv->log, "sensor init waiting %d frames ...\n", initial->delay);

    ret = sensorhw_delay(priv, initial->delay);
    if (!ret)
        return false;

    if (initial->steps)
        return sensorhw_do_steps(priv, initial->steps);
    return true;
}

rk_irfpa_sensorhw_ctx_t* rk_irfpa_sensorhw_create_ctx(void)
{
    sensorhw_priv_t *priv = new sensorhw_priv_t;
    if (!priv)
        return NULL;
    rk_irfpa_sensorhw_ctx_t *ctx = &priv->ctx;

    return ctx;
}

bool rk_irfpa_sensorhw_init(rk_irfpa_sensorhw_ctx_t *ctx, rk_irfpa_streamhw_ctx_t *streamhw)
{
    sensorhw_priv_t *priv = (sensorhw_priv_t *)ctx;
    priv->streamhw = streamhw;
    priv->log = streamhw->log;

    uint16_t w = streamhw->info.vi_width;
    uint16_t h = streamhw->info.vi_height;

    bool loaded = false;
    for (int i=0; i<sensor_cfg_map_cnt; i++) {
        rk_irfpa_log_trace(priv->log, "registed sensor %d: %s", i, sensor_cfg_map[i].name);
        if (strstr(streamhw->info.sensor_name, sensor_cfg_map[i].name)) {
            rk_irfpa_log_info(priv->log, "load sensor cfg %s !", sensor_cfg_map[i].name);
            loaded = true;

            ctx->cfg = sensor_cfg_map[i].cfg;
        }
    }
    if (!loaded) {
        rk_irfpa_log_warn(priv->log, "%s: no sensor cfg load!", __func__);
    }

    ctx->base_buf = (uint16_t *)malloc(w * h * sizeof(uint16_t));
    //ctx->x16 = (uint16_t *)malloc(w * h * sizeof(uint16_t));
    return true;
}

void rk_irfpa_sensorhw_deinit(rk_irfpa_sensorhw_ctx_t *ctx)
{
    sensorhw_priv_t *priv = (sensorhw_priv_t *)ctx;
    free(ctx->base_buf);
    //free(ctx->x16);
    delete priv;
}

bool rk_irfpa_sensorhw_process(rk_irfpa_sensorhw_ctx_t *ctx)
{
    bool ret;
    sensorhw_priv_t *priv = (sensorhw_priv_t *)ctx;
    rk_irfpa_streamhw_ctx_t *streamhw = priv->streamhw;

    if (priv->initial == false) {
        ret = sensorhw_initial(priv);
        if (!ret)
            return false;
        priv->initial = true;
    }

    ret = rk_irfpa_streamhw_getFrame(priv->streamhw);
    if (!ret)
        return false;

    uint16_t *p = streamhw->res.y16;
    uint16_t w = streamhw->info.vi_width;
    uint16_t h = streamhw->info.vi_height;

    for (int i = 0; i < w*h; i++) {
        int16_t pix = p[i];
        int16_t base = priv->ctx.base_buf[i];
        pix -= base;
        pix += (priv->ctx.base_ave);
        //priv->ctx.x16[i] = pix;
        p[i] = pix;
    }
    return true;
}

