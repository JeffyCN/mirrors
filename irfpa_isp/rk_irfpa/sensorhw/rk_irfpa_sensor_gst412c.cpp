#include "rk_irfpa_sensorhw.h"
#include <cstdio>

static const rk_irfpa_sensorhw_op_t initial_steps[] = {
    { RK_IRFPA_SENSORHW_OP_SHUTTER_CLOSE,  3 },
    { RK_IRFPA_SENSORHW_OP_GAIN1,       5 },
    { RK_IRFPA_SENSORHW_OP_GAIN2,       5 },
    { RK_IRFPA_SENSORHW_OP_OOC,            5 },
    { RK_IRFPA_SENSORHW_OP_BASE,            1 },
    { RK_IRFPA_SENSORHW_OP_SHUTTER_OPEN,   3 },
    { RK_IRFPA_SENSORHW_OP_DONE,           0 }
};

void rk_irfpa_sensorhw_cfg_gst412c(rk_irfpa_sensorhw_ctx_t *sensorhw)
{
}

static void __attribute__((constructor(102))) register_gst412c(void)
{
    rk_irfpa_sensorhw_config_t cfg = {0};

    cfg.shutter.ctrl = "shutter";
    cfg.shutter.open_value = 0;
    cfg.shutter.close_value = 1;

    cfg.gain1.ctrl = "gst_sensor_ra_sel";
    cfg.gain1.adjust.mode = RK_IRFPA_ADJUST_INCREASE_LOWER;
    cfg.gain1.adjust.step = 1;
    cfg.gain1.adjust.delay = 10;
    cfg.gain1.adjust.min = 1;
    cfg.gain1.adjust.max = 15;
    cfg.gain1.target.lower = 2000;
    cfg.gain1.target.upper = 14000;

    cfg.gain2.ctrl = "gst_sensor_hssd";
    cfg.gain2.adjust.mode = RK_IRFPA_ADJUST_INCREASE_LOWER;
    cfg.gain2.adjust.step = 1;
    cfg.gain2.adjust.delay = 5;
    cfg.gain2.adjust.min = 1;
    cfg.gain2.adjust.max = 127;
    cfg.gain2.target.lower = 7000;
    cfg.gain2.target.upper = 9000;

    cfg.ooc.adjust.step = 1;
    cfg.ooc.adjust.min = 0;
    cfg.ooc.adjust.max = 62;
    cfg.ooc.adjust.delay = 3;
    cfg.ooc.start_x = 0;
    cfg.ooc.start_y = 4;
    cfg.ooc.init_val = 32;
    cfg.ooc.adj_step = 8;
    cfg.ooc.target.lower = 7200;
    cfg.ooc.target.upper = 7600;

    cfg.base.enable = true;
    cfg.base.ave_num = 10;
    cfg.base.ave_delay = 0;

    cfg.initial.delay = 30;
    cfg.initial.steps = initial_steps;

    rk_irfpa_sensorhw_register("gst412c", cfg);
}
