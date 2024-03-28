#include "iqconverter.h"

#define DEFAULT_MAIN_SCENE  "normal"
#define DEFAULT_SUB_SCENE  "day"

#if defined(ISP_HW_V20)
int g_rkaiq_isp_hw_ver = 20;
#elif defined(ISP_HW_V21)
int g_rkaiq_isp_hw_ver = 21;
#elif defined(ISP_HW_V30)
int g_rkaiq_isp_hw_ver = 30;
#elif defined(ISP_HW_V32)
int g_rkaiq_isp_hw_ver = 32;
#elif defined(ISP_HW_V32_LITE)
int g_rkaiq_isp_hw_ver = 321;
#elif defined(ISP_HW_V39)
int g_rkaiq_isp_hw_ver = 39;
#else
#error "WRONG ISP_HW_VERSION, ONLY SUPPORT V20/V21/V30/V32 NOW !"
#endif

namespace RkCam {

IQConverter::IQConverter(const char *xml, const char *json)
    : ifile(std::string(xml)), ofile(std::string(json)), calibv1(nullptr) {
    calibv2 = calibdbV2_ctx_new();
    memset(&calibproj, 0, sizeof(CamCalibDbProj_t));
}

int IQConverter::convert() {
    calibv1 = RkAiqCalibDb::createCalibDb((char *)ifile.c_str());
    if (!calibv1) {
        printf("[XML]load %s failed!\n", ifile.c_str());
        return -1;
    } else {
        printf("[XML] %s load finished!\n", ifile.c_str());
    }
    doConvert();

    calibproj.sensor_calib = *calibv2->sensor_info;
    calibproj.module_calib = *calibv2->module_info;
    calibproj.sys_static_cfg = *calibv2->sys_cfg;
    addToScene(&calibproj, DEFAULT_MAIN_SCENE, DEFAULT_SUB_SCENE, calibv2);

    if (0 != RkAiqCalibDbV2::calibproj2json(ofile.c_str(), &calibproj)) {
        printf("convert %s to %s failed!\n", ifile.c_str(), ofile.c_str());
        return -1;
    }

    CalibV2AecFree(calibv2);

    if (CHECK_ISP_HW_V20())
        CalibV2AwbV20Free(calibv2);
    if (CHECK_ISP_HW_V21())
        CalibV2AwbV21Free(calibv2);

    //bayernrV1_calibdbV2_free(calibv2->bayernr_v1);
    //mfnrV1_calibdbV2_free(calibv2->mfnr_v1);
    //uvnrV1_calibdbV2_free(calibv2->uvnr_v1);
    //ynrV1_calibdbV2_free(calibv2->ynr_v1);

    CalibV2CCMFree(calibv2);
    CalibV2AfFree(calibv2);
    CalibV2Lut3DFree(calibv2);
    return 0;
}

void CalibConverterAE::convert(CamCalibDbV2Context_t *calibv2,
                               CamCalibDbContext_t *calibv1) {

    CalibDb_Aec_ParaV2_t* calibv2_ae_calib =
        (CalibDb_Aec_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, ae_calib));
    memset(calibv2_ae_calib, 0, sizeof(CalibDb_Aec_ParaV2_t));

    convertAecCalibV1ToCalibV2(calibv1, calibv2);

#if 1
    CalibDb_Sensor_ParaV2_t* calibv2_sensor_calib =
        (CalibDb_Sensor_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, sensor_calib));
    convertSensorinfoCalibV1ToCalibV2(calibv1, calibv2_sensor_calib);

    CalibDb_Module_ParaV2_t* calibv2_module_calib =
        (CalibDb_Module_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, module_calib));
    convertModuleinfoCalibV1ToCalibV2(calibv1, calibv2_module_calib);
#endif
}

void CalibConverterAWB::convert(CamCalibDbV2Context_t *calibv2,
                                CamCalibDbContext_t *calibv1) {
    if (CHECK_ISP_HW_V20()) {
        convertCalib2calibV20(calibv1, calibv2);
    }
    if (CHECK_ISP_HW_V21()) {
        convertCalib2calibV21(calibv1, calibv2);
    }
}

void CalibConverterAblc::convert(CamCalibDbV2Context_t *calibv2,
                                 CamCalibDbContext_t *calibv1) {
    CalibDb_Blc_t* calib_v1_blc =
        (CalibDb_Blc_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, blc);
    if (!calib_v1_blc)
        return;

    CalibDbV2_Ablc_t* calibv2_ablc_calib =
        (CalibDbV2_Ablc_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, ablc_calib));
    if (!calibv2_ablc_calib)
        return;
    //len
    calibv2_ablc_calib->BlcTuningPara.BLC_Data.ISO_len = 13;
    calibv2_ablc_calib->BlcTuningPara.BLC_Data.R_Channel_len = calibv2_ablc_calib->BlcTuningPara.BLC_Data.ISO_len;
    calibv2_ablc_calib->BlcTuningPara.BLC_Data.Gr_Channel_len = calibv2_ablc_calib->BlcTuningPara.BLC_Data.ISO_len;
    calibv2_ablc_calib->BlcTuningPara.BLC_Data.Gb_Channel_len = calibv2_ablc_calib->BlcTuningPara.BLC_Data.ISO_len;
    calibv2_ablc_calib->BlcTuningPara.BLC_Data.B_Channel_len = calibv2_ablc_calib->BlcTuningPara.BLC_Data.ISO_len;

    //malloc
    calibv2_ablc_calib->BlcTuningPara.BLC_Data.ISO = (float *) malloc(sizeof(float) * 13);
    calibv2_ablc_calib->BlcTuningPara.BLC_Data.R_Channel = (float *) malloc(sizeof(float) * 13);
    calibv2_ablc_calib->BlcTuningPara.BLC_Data.Gr_Channel = (float *) malloc(sizeof(float) * 13);
    calibv2_ablc_calib->BlcTuningPara.BLC_Data.Gb_Channel = (float *) malloc(sizeof(float) * 13);
    calibv2_ablc_calib->BlcTuningPara.BLC_Data.B_Channel = (float *) malloc(sizeof(float) * 13);

    calibv2_ablc_calib->BlcTuningPara.enable = calib_v1_blc->enable ? true : false;
    for(int i = 0; i < calibv2_ablc_calib->BlcTuningPara.BLC_Data.ISO_len; i++) {
        calibv2_ablc_calib->BlcTuningPara.BLC_Data.ISO[i] = calib_v1_blc->mode_cell[0].iso[i];
        calibv2_ablc_calib->BlcTuningPara.BLC_Data.R_Channel[i] = calib_v1_blc->mode_cell[0].level[0][i];
        calibv2_ablc_calib->BlcTuningPara.BLC_Data.Gr_Channel[i] = calib_v1_blc->mode_cell[0].level[1][i];
        calibv2_ablc_calib->BlcTuningPara.BLC_Data.Gb_Channel[i] = calib_v1_blc->mode_cell[0].level[2][i];
        calibv2_ablc_calib->BlcTuningPara.BLC_Data.B_Channel[i] = calib_v1_blc->mode_cell[0].level[3][i];
    }
}

void CalibConverterAdpcc::convert(CamCalibDbV2Context_t *calibv2,
                                  CamCalibDbContext_t *calibv1) {
    CalibDb_Dpcc_t* calibv1_dpcc =
        (CalibDb_Dpcc_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, dpcc);
    if (!calibv1_dpcc)
        return;

    CalibDbV2_Dpcc_t* calibv2_adpcc_calib =
        (CalibDbV2_Dpcc_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, adpcc_calib));
    if (!calibv2_adpcc_calib)
        return;

    //len
    calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Fast_Data.ISO_len = 13;
    calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Fast_Data.Single_level_len = calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Fast_Data.ISO_len;
    calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Fast_Data.Double_level_len = calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Fast_Data.ISO_len;
    calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Fast_Data.Triple_level_len = calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Fast_Data.ISO_len;
    calibv2_adpcc_calib->DpccTuningPara.Sensor_dpcc.SensorDpcc_Data.ISO_len = 13;
    calibv2_adpcc_calib->DpccTuningPara.Sensor_dpcc.SensorDpcc_Data.level_single_len = calibv2_adpcc_calib->DpccTuningPara.Sensor_dpcc.SensorDpcc_Data.ISO_len;
    calibv2_adpcc_calib->DpccTuningPara.Sensor_dpcc.SensorDpcc_Data.level_multiple_len = calibv2_adpcc_calib->DpccTuningPara.Sensor_dpcc.SensorDpcc_Data.ISO_len;

    //malloc
    calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Fast_Data.ISO = (float *) malloc(sizeof(float) * 13);
    calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Fast_Data.Single_level = (int *) malloc(sizeof(int) * 13);
    calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Fast_Data.Double_level = (int *) malloc(sizeof(int) * 13);
    calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Fast_Data.Triple_level = (int *) malloc(sizeof(int) * 13);
    calibv2_adpcc_calib->DpccTuningPara.Sensor_dpcc.SensorDpcc_Data.ISO = (float *) malloc(sizeof(float) * 13);
    calibv2_adpcc_calib->DpccTuningPara.Sensor_dpcc.SensorDpcc_Data.level_single = (int *) malloc(sizeof(int) * 13);
    calibv2_adpcc_calib->DpccTuningPara.Sensor_dpcc.SensorDpcc_Data.level_multiple = (int *) malloc(sizeof(int) * 13);

    //enable
    calibv2_adpcc_calib->DpccTuningPara.Enable = calibv1_dpcc->enable ? true : false;

    //fast mode
    calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Fast_mode_en = calibv1_dpcc->fast.fast_mode_double_en ? true : false;
    calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Single_enable = calibv1_dpcc->fast.fast_mode_single_en ? true : false;
    calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Double_enable = calibv1_dpcc->fast.fast_mode_double_en ? true : false;
    calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Triple_enable = calibv1_dpcc->fast.fast_mode_triple_en ? true : false;
    for(int i = 0; i < calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Fast_Data.ISO_len; i++) {
        calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Fast_Data.ISO[i] = calibv1_dpcc->fast.ISO[i];
        calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Fast_Data.Single_level[i] = calibv1_dpcc->fast.fast_mode_single_level[i];
        calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Fast_Data.Double_level[i] = calibv1_dpcc->fast.fast_mode_double_level[i];
        calibv2_adpcc_calib->DpccTuningPara.Fast_Mode.Fast_Data.Triple_level[i] = calibv1_dpcc->fast.fast_mode_triple_level[i];
    }

    //expert mode
    calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.grayscale_mode = calibv1_dpcc->expert.grayscale_mode ? true : false;
    calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.stage1_Enable = calibv1_dpcc->expert.stage1_Enable[0] ? true : false;
    calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.rk_out_sel = calibv1_dpcc->expert.rk_out_sel[0];
    calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.dpcc_out_sel = calibv1_dpcc->expert.dpcc_out_sel[0] ? true : false;
    calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.stage1_g_3x3 = calibv1_dpcc->expert.stage1_g_3x3[0] ? true : false;
    calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.stage1_rb_3x3 = calibv1_dpcc->expert.stage1_rb_3x3[0] ? true : false;
    calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.stage1_inc_g_center = calibv1_dpcc->expert.stage1_inc_g_center[0] ? true : false;
    calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.stage1_inc_rb_center = calibv1_dpcc->expert.stage1_inc_rb_center[0] ? true : false;
    for(int i = 0; i < CALIBDB_ADPCC_KNOTS_NUM; i++) {
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.SetEnable.ISO[i] = calibv1_dpcc->expert.iso[i];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.SetEnable.fix_set[i] = calibv1_dpcc->expert.stage1_use_fix_set[i];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.SetEnable.set1[i] = calibv1_dpcc->expert.stage1_use_set1[i];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.SetEnable.set2[i] = calibv1_dpcc->expert.stage1_use_set2[i];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.SetEnable.set3[i] = calibv1_dpcc->expert.stage1_use_set3[i];
    }
    for(int j = 0; j < CALIBDB_ADPCC_KNOTS_NUM; j++) {
        //set1
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.RK.RK_enable[j] = calibv1_dpcc->expert.set[0].rk.g_enable[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.RK.rb_sw_mindis[j] = calibv1_dpcc->expert.set[0].rk.rb_sw_mindis[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.RK.g_sw_mindis[j] = calibv1_dpcc->expert.set[0].rk.g_sw_mindis[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.RK.sw_dis_scale_min[j] = calibv1_dpcc->expert.set[0].rk.sw_dis_scale_min[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.RK.sw_dis_scale_max[j] = calibv1_dpcc->expert.set[0].rk.sw_dis_scale_max[j];

        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.LC.LC_enable[j] = calibv1_dpcc->expert.set[0].lc.g_enable[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.LC.g_line_thr[j] = calibv1_dpcc->expert.set[0].lc.g_line_thr[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.LC.rb_line_thr[j] = calibv1_dpcc->expert.set[0].lc.rb_line_thr[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.LC.g_line_mad_fac[j] = calibv1_dpcc->expert.set[0].lc.g_line_mad_fac[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.LC.rb_line_mad_fac[j] = calibv1_dpcc->expert.set[0].lc.rb_line_mad_fac[j];

        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.PG.PG_enable[j] = calibv1_dpcc->expert.set[0].pg.g_enable[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.PG.g_pg_fac[j] = calibv1_dpcc->expert.set[0].pg.g_pg_fac[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.PG.rb_pg_fac[j] = calibv1_dpcc->expert.set[0].pg.rb_pg_fac[j];

        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.RND.RND_enable[j] = calibv1_dpcc->expert.set[0].rnd.g_enable[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.RND.g_rnd_thr[j] = calibv1_dpcc->expert.set[0].rnd.g_rnd_thr[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.RND.rb_rnd_thr[j] = calibv1_dpcc->expert.set[0].rnd.rb_rnd_thr[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.RND.g_rnd_offs[j] = calibv1_dpcc->expert.set[0].rnd.g_rnd_offs[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.RND.rb_rnd_offs[j] = calibv1_dpcc->expert.set[0].rnd.rb_rnd_offs[j];

        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.RG.RG_enable[j] = calibv1_dpcc->expert.set[0].rg.g_enable[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.RG.g_rg_fac[j] = calibv1_dpcc->expert.set[0].rg.g_rg_fac[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.RG.rb_rg_fac[j] = calibv1_dpcc->expert.set[0].rg.rb_rg_fac[j];

        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.RO.RO_enable[j] = calibv1_dpcc->expert.set[0].ro.g_enable[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.RO.rb_ro_lim[j] = calibv1_dpcc->expert.set[0].ro.rb_ro_lim[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set1.RO.g_ro_lim[j] = calibv1_dpcc->expert.set[0].ro.g_ro_lim[j];
        //set2
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.RK.RK_enable[j] = calibv1_dpcc->expert.set[1].rk.g_enable[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.RK.rb_sw_mindis[j] = calibv1_dpcc->expert.set[1].rk.rb_sw_mindis[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.RK.g_sw_mindis[j] = calibv1_dpcc->expert.set[1].rk.g_sw_mindis[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.RK.sw_dis_scale_min[j] = calibv1_dpcc->expert.set[1].rk.sw_dis_scale_min[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.RK.sw_dis_scale_max[j] = calibv1_dpcc->expert.set[1].rk.sw_dis_scale_max[j];

        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.LC.LC_enable[j] = calibv1_dpcc->expert.set[1].lc.g_enable[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.LC.g_line_thr[j] = calibv1_dpcc->expert.set[1].lc.g_line_thr[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.LC.rb_line_thr[j] = calibv1_dpcc->expert.set[1].lc.rb_line_thr[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.LC.g_line_mad_fac[j] = calibv1_dpcc->expert.set[1].lc.g_line_mad_fac[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.LC.rb_line_mad_fac[j] = calibv1_dpcc->expert.set[1].lc.rb_line_mad_fac[j];

        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.PG.PG_enable[j] = calibv1_dpcc->expert.set[1].pg.g_enable[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.PG.g_pg_fac[j] = calibv1_dpcc->expert.set[1].pg.g_pg_fac[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.PG.rb_pg_fac[j] = calibv1_dpcc->expert.set[1].pg.rb_pg_fac[j];

        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.RND.RND_enable[j] = calibv1_dpcc->expert.set[1].rnd.g_enable[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.RND.g_rnd_thr[j] = calibv1_dpcc->expert.set[1].rnd.g_rnd_thr[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.RND.rb_rnd_thr[j] = calibv1_dpcc->expert.set[1].rnd.rb_rnd_thr[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.RND.g_rnd_offs[j] = calibv1_dpcc->expert.set[1].rnd.g_rnd_offs[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.RND.rb_rnd_offs[j] = calibv1_dpcc->expert.set[1].rnd.rb_rnd_offs[j];

        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.RG.RG_enable[j] = calibv1_dpcc->expert.set[1].rg.g_enable[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.RG.g_rg_fac[j] = calibv1_dpcc->expert.set[1].rg.g_rg_fac[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.RG.rb_rg_fac[j] = calibv1_dpcc->expert.set[1].rg.rb_rg_fac[j];

        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.RO.RO_enable[j] = calibv1_dpcc->expert.set[1].ro.g_enable[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.RO.rb_ro_lim[j] = calibv1_dpcc->expert.set[1].ro.rb_ro_lim[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set2.RO.g_ro_lim[j] = calibv1_dpcc->expert.set[1].ro.g_ro_lim[j];
        //set3
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.RK.RK_enable[j] = calibv1_dpcc->expert.set[2].rk.g_enable[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.RK.rb_sw_mindis[j] = calibv1_dpcc->expert.set[2].rk.rb_sw_mindis[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.RK.g_sw_mindis[j] = calibv1_dpcc->expert.set[2].rk.g_sw_mindis[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.RK.sw_dis_scale_min[j] = calibv1_dpcc->expert.set[2].rk.sw_dis_scale_min[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.RK.sw_dis_scale_max[j] = calibv1_dpcc->expert.set[2].rk.sw_dis_scale_max[j];

        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.LC.LC_enable[j] = calibv1_dpcc->expert.set[2].lc.g_enable[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.LC.g_line_thr[j] = calibv1_dpcc->expert.set[2].lc.g_line_thr[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.LC.rb_line_thr[j] = calibv1_dpcc->expert.set[2].lc.rb_line_thr[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.LC.g_line_mad_fac[j] = calibv1_dpcc->expert.set[2].lc.g_line_mad_fac[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.LC.rb_line_mad_fac[j] = calibv1_dpcc->expert.set[2].lc.rb_line_mad_fac[j];

        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.PG.PG_enable[j] = calibv1_dpcc->expert.set[2].pg.g_enable[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.PG.g_pg_fac[j] = calibv1_dpcc->expert.set[2].pg.g_pg_fac[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.PG.rb_pg_fac[j] = calibv1_dpcc->expert.set[2].pg.rb_pg_fac[j];

        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.RND.RND_enable[j] = calibv1_dpcc->expert.set[2].rnd.g_enable[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.RND.g_rnd_thr[j] = calibv1_dpcc->expert.set[2].rnd.g_rnd_thr[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.RND.rb_rnd_thr[j] = calibv1_dpcc->expert.set[2].rnd.rb_rnd_thr[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.RND.g_rnd_offs[j] = calibv1_dpcc->expert.set[2].rnd.g_rnd_offs[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.RND.rb_rnd_offs[j] = calibv1_dpcc->expert.set[2].rnd.rb_rnd_offs[j];

        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.RG.RG_enable[j] = calibv1_dpcc->expert.set[2].rg.g_enable[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.RG.g_rg_fac[j] = calibv1_dpcc->expert.set[2].rg.g_rg_fac[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.RG.rb_rg_fac[j] = calibv1_dpcc->expert.set[2].rg.rb_rg_fac[j];

        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.RO.RO_enable[j] = calibv1_dpcc->expert.set[2].ro.g_enable[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.RO.rb_ro_lim[j] = calibv1_dpcc->expert.set[2].ro.rb_ro_lim[j];
        calibv2_adpcc_calib->DpccTuningPara.Expert_Mode.set3.RO.g_ro_lim[j] = calibv1_dpcc->expert.set[2].ro.g_ro_lim[j];
    }

    //pdaf
    calibv2_adpcc_calib->DpccTuningPara.Dpcc_pdaf.en = calibv1_dpcc->pdaf.en ? true : false;
    calibv2_adpcc_calib->DpccTuningPara.Dpcc_pdaf.offsetx = calibv1_dpcc->pdaf.offsetx;
    calibv2_adpcc_calib->DpccTuningPara.Dpcc_pdaf.offsety = calibv1_dpcc->pdaf.offsety;
    calibv2_adpcc_calib->DpccTuningPara.Dpcc_pdaf.wrapx = calibv1_dpcc->pdaf.wrapx;
    calibv2_adpcc_calib->DpccTuningPara.Dpcc_pdaf.wrapy = calibv1_dpcc->pdaf.wrapy;
    calibv2_adpcc_calib->DpccTuningPara.Dpcc_pdaf.wrapx_num = calibv1_dpcc->pdaf.wrapx_num;
    calibv2_adpcc_calib->DpccTuningPara.Dpcc_pdaf.wrapy_num = calibv1_dpcc->pdaf.wrapy_num;
    calibv2_adpcc_calib->DpccTuningPara.Dpcc_pdaf.forward_med = calibv1_dpcc->pdaf.forward_med;
    for(int i = 0; i < 16; i++) {
        calibv2_adpcc_calib->DpccTuningPara.Dpcc_pdaf.point_en[i] = calibv1_dpcc->pdaf.point_en[i];
        calibv2_adpcc_calib->DpccTuningPara.Dpcc_pdaf.point_x[i] = calibv1_dpcc->pdaf.point_x[i];
        calibv2_adpcc_calib->DpccTuningPara.Dpcc_pdaf.point_y[i] = calibv1_dpcc->pdaf.point_y[i];
    }

    //sensor dpcc
    calibv2_adpcc_calib->DpccTuningPara.Sensor_dpcc.sensor_dpcc_auto_en = calibv1_dpcc->sensor_dpcc.en ? true : false;
    calibv2_adpcc_calib->DpccTuningPara.Sensor_dpcc.max_level = (int)(calibv1_dpcc->sensor_dpcc.max_level);
    for(int i = 0; i < calibv2_adpcc_calib->DpccTuningPara.Sensor_dpcc.SensorDpcc_Data.ISO_len; i++) {
        calibv2_adpcc_calib->DpccTuningPara.Sensor_dpcc.SensorDpcc_Data.ISO[i] = (int)(calibv1_dpcc->sensor_dpcc.iso[i]);
        calibv2_adpcc_calib->DpccTuningPara.Sensor_dpcc.SensorDpcc_Data.level_single[i] = (int)(calibv1_dpcc->sensor_dpcc.level_single[i]);
        calibv2_adpcc_calib->DpccTuningPara.Sensor_dpcc.SensorDpcc_Data.level_multiple[i] = (int)(calibv1_dpcc->sensor_dpcc.level_multiple[i]);
    }
}

void CalibConverterAdegamma::convert(CamCalibDbV2Context_t *calibv2,
                                     CamCalibDbContext_t *calibv1) {
    CalibDb_Degamma_t* calib_v1_degamma =
        (CalibDb_Degamma_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, degamma);
    if (!calib_v1_degamma)
        return;

    CalibDbV2_Adegmma_t* calibv2_adegamma_calib =
        (CalibDbV2_Adegmma_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, adegamma_calib));
    if (!calibv2_adegamma_calib)
        return;

    calibv2_adegamma_calib->DegammaTuningPara.degamma_en = calib_v1_degamma->degamma_en ? true : false;
    for(int i = 0; i < ADEGAMMA_CRUVE_KNOTS; i++) {
        calibv2_adegamma_calib->DegammaTuningPara.X_axis[i] = (int)(calib_v1_degamma->mode[0].X_axis[i] + 0.5);
        calibv2_adegamma_calib->DegammaTuningPara.curve_R[i] = (int)(calib_v1_degamma->mode[0].curve_R[i] + 0.5);
        calibv2_adegamma_calib->DegammaTuningPara.curve_G[i] = (int)(calib_v1_degamma->mode[0].curve_G[i] + 0.5);
        calibv2_adegamma_calib->DegammaTuningPara.curve_B[i] = (int)(calib_v1_degamma->mode[0].curve_B[i] + 0.5);
    }
}

void CalibConverterAgic::convert(CamCalibDbV2Context_t *calibv2,
                                 CamCalibDbContext_t *calibv1) {
    //len
    CalibDbV2_Gic_V20_t* calibv2_agic_calib_V20 =
        (CalibDbV2_Gic_V20_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, agic_calib_v20));
    if (calibv2_agic_calib_V20) {
        calibv2_agic_calib_V20->GicTuningPara.GicData.ISO_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.min_busy_thre_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.min_grad_thr1_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.min_grad_thr2_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.k_grad1_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.k_grad2_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.gb_thre_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.maxCorV_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.maxCorVboth_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.dark_thre_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.dark_threHi_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.k_grad1_dark_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.k_grad2_dark_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.min_grad_thr_dark1_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.min_grad_thr_dark2_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.noiseCurve_0_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.noiseCurve_1_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.GValueLimitLo_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.GValueLimitHi_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.textureStrength_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.ScaleHi_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.ScaleLo_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.globalStrength_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.noise_coea_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.noise_coeb_len = 9;
        calibv2_agic_calib_V20->GicTuningPara.GicData.diff_clip_len = 9;
    }

    CalibDbV2_Gic_V21_t* calibv2_agic_calib_V21 =
        (CalibDbV2_Gic_V21_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, agic_calib_v21));
    if (calibv2_agic_calib_V21) {
        calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len = 9;
        calibv2_agic_calib_V21->GicTuningPara.GicData.min_busy_thre_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.min_grad_thr1_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.min_grad_thr2_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.k_grad1_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.k_grad2_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.gb_thre_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.maxCorV_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.maxCorVboth_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.dark_thre_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.dark_threHi_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.k_grad1_dark_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.k_grad2_dark_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.min_grad_thr_dark1_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.min_grad_thr_dark2_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.noiseCurve_0_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.noiseCurve_1_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.NoiseScale_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.NoiseBase_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.globalStrength_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
        calibv2_agic_calib_V21->GicTuningPara.GicData.diff_clip_len = calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len;
    }

    //malloc
    if (calibv2_agic_calib_V20) {
        calibv2_agic_calib_V20->GicTuningPara.GicData.ISO = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.min_busy_thre = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.min_grad_thr1 = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.min_grad_thr2 = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.k_grad1 = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.k_grad2 = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.gb_thre = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.maxCorV = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.maxCorVboth = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.dark_thre = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.dark_threHi = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.k_grad1_dark = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.k_grad2_dark = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.min_grad_thr_dark1 = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.min_grad_thr_dark2 = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.noiseCurve_0 = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.noiseCurve_1 = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.GValueLimitLo = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.GValueLimitHi = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.textureStrength = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.ScaleHi = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.ScaleLo = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.globalStrength = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.noise_coea = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.noise_coeb = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V20->GicTuningPara.GicData.diff_clip = (float *) malloc(sizeof(float) * 9);
    }

    if (calibv2_agic_calib_V21) {
        calibv2_agic_calib_V21->GicTuningPara.GicData.ISO = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.min_busy_thre = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.min_grad_thr1 = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.min_grad_thr2 = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.k_grad1 = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.k_grad2 = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.gb_thre = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.maxCorV = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.maxCorVboth = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.dark_thre = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.dark_threHi = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.k_grad1_dark = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.k_grad2_dark = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.min_grad_thr_dark1 = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.min_grad_thr_dark2 = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.noiseCurve_0 = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.noiseCurve_1 = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.NoiseScale = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.NoiseBase = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.globalStrength = (float *) malloc(sizeof(float) * 9);
        calibv2_agic_calib_V21->GicTuningPara.GicData.diff_clip = (float *) malloc(sizeof(float) * 9);
    }
    //v20
    if (CHECK_ISP_HW_V20()) {
        CalibDb_Gic_t* calibv1_gic =
            (CalibDb_Gic_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, gic);
        calibv2_agic_calib_V20->GicTuningPara.enable = calibv1_gic->calib_v20[0].gic_en ? true : false;
        calibv2_agic_calib_V20->GicTuningPara.edge_en = calibv1_gic->calib_v20[0].edge_en ? true : false;
        calibv2_agic_calib_V20->GicTuningPara.noise_cut_en = calibv1_gic->calib_v20[0].noise_cut_en ? true : false;
        calibv2_agic_calib_V20->GicTuningPara.gr_ration = calibv1_gic->calib_v20[0].gr_ration;
        for(int i = 0; i < calibv2_agic_calib_V20->GicTuningPara.GicData.ISO_len; i++) {
            calibv2_agic_calib_V20->GicTuningPara.GicData.ISO[i] = calibv1_gic->calib_v20[0].setting.iso[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.min_busy_thre[i] = calibv1_gic->calib_v20[0].setting.min_busy_thre[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.min_grad_thr1[i] = calibv1_gic->calib_v20[0].setting.min_grad_thr1[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.min_grad_thr2[i] = calibv1_gic->calib_v20[0].setting.min_grad_thr2[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.k_grad1[i] = calibv1_gic->calib_v20[0].setting.k_grad1[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.k_grad2[i] = calibv1_gic->calib_v20[0].setting.k_grad2[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.gb_thre[i] = calibv1_gic->calib_v20[0].setting.gb_thre[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.maxCorV[i] = calibv1_gic->calib_v20[0].setting.maxCorV[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.maxCorVboth[i] = calibv1_gic->calib_v20[0].setting.maxCorVboth[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.dark_thre[i] = calibv1_gic->calib_v20[0].setting.dark_thre[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.dark_threHi[i] = calibv1_gic->calib_v20[0].setting.dark_threHi[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.k_grad1_dark[i] = calibv1_gic->calib_v20[0].setting.k_grad1_dark[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.k_grad2_dark[i] = calibv1_gic->calib_v20[0].setting.k_grad2_dark[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.min_grad_thr_dark1[i] = calibv1_gic->calib_v20[0].setting.min_grad_thr_dark1[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.min_grad_thr_dark2[i] = calibv1_gic->calib_v20[0].setting.min_grad_thr_dark2[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.noiseCurve_0[i] = calibv1_gic->calib_v20[0].setting.noiseCurve_0[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.noiseCurve_1[i] = calibv1_gic->calib_v20[0].setting.noiseCurve_1[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.GValueLimitLo[i] = calibv1_gic->calib_v20[0].setting.GValueLimitLo[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.GValueLimitHi[i] = calibv1_gic->calib_v20[0].setting.GValueLimitHi[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.textureStrength[i] = calibv1_gic->calib_v20[0].setting.textureStrength[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.ScaleLo[i] = calibv1_gic->calib_v20[0].setting.ScaleLo[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.ScaleHi[i] = calibv1_gic->calib_v20[0].setting.ScaleHi[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.globalStrength[i] = calibv1_gic->calib_v20[0].setting.globalStrength[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.noise_coea[i] = calibv1_gic->calib_v20[0].setting.noise_coea[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.noise_coeb[i] = calibv1_gic->calib_v20[0].setting.noise_coeb[i];
            calibv2_agic_calib_V20->GicTuningPara.GicData.diff_clip[i] = calibv1_gic->calib_v20[0].setting.diff_clip[i];
        }
    }

    //v21
    if (CHECK_ISP_HW_V21()) {
        CalibDb_Gic_Isp21_t* calibv1_gic_isp21 =
            (CalibDb_Gic_Isp21_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, gic);
        calibv2_agic_calib_V21->GicTuningPara.enable = calibv1_gic_isp21->calib_v21[0].gic_en ? true : false;
        calibv2_agic_calib_V21->GicTuningPara.gr_ration = calibv1_gic_isp21->calib_v21[0].gr_ration;
        for(int i = 0; i < calibv2_agic_calib_V21->GicTuningPara.GicData.ISO_len; i++) {
            calibv2_agic_calib_V21->GicTuningPara.GicData.ISO[i] = calibv1_gic_isp21->calib_v21[0].setting.iso[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.min_busy_thre[i] = calibv1_gic_isp21->calib_v21[0].setting.min_busy_thre[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.min_grad_thr1[i] = calibv1_gic_isp21->calib_v21[0].setting.min_grad_thr1[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.min_grad_thr2[i] = calibv1_gic_isp21->calib_v21[0].setting.min_grad_thr2[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.k_grad1[i] = calibv1_gic_isp21->calib_v21[0].setting.k_grad1[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.k_grad2[i] = calibv1_gic_isp21->calib_v21[0].setting.k_grad2[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.gb_thre[i] = calibv1_gic_isp21->calib_v21[0].setting.gb_thre[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.maxCorV[i] = calibv1_gic_isp21->calib_v21[0].setting.maxCorV[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.maxCorVboth[i] = calibv1_gic_isp21->calib_v21[0].setting.maxCorVboth[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.dark_thre[i] = calibv1_gic_isp21->calib_v21[0].setting.dark_thre[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.dark_threHi[i] = calibv1_gic_isp21->calib_v21[0].setting.dark_threHi[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.k_grad1_dark[i] = calibv1_gic_isp21->calib_v21[0].setting.k_grad1_dark[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.k_grad2_dark[i] = calibv1_gic_isp21->calib_v21[0].setting.k_grad2_dark[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.min_grad_thr_dark1[i] = calibv1_gic_isp21->calib_v21[0].setting.min_grad_thr_dark1[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.min_grad_thr_dark2[i] = calibv1_gic_isp21->calib_v21[0].setting.min_grad_thr_dark2[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.noiseCurve_0[i] = calibv1_gic_isp21->calib_v21[0].setting.noiseCurve_0[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.noiseCurve_1[i] = calibv1_gic_isp21->calib_v21[0].setting.noiseCurve_1[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.globalStrength[i] = calibv1_gic_isp21->calib_v21[0].setting.globalStrength[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.NoiseBase[i] = calibv1_gic_isp21->calib_v21[0].setting.NoiseBase[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.NoiseScale[i] = calibv1_gic_isp21->calib_v21[0].setting.NoiseScale[i];
            calibv2_agic_calib_V21->GicTuningPara.GicData.diff_clip[i] = calibv1_gic_isp21->calib_v21[0].setting.diff_clip[i];
        }
    }
}

void CalibConverterAdehaze::convert(CamCalibDbV2Context_t *calibv2,
                                    CamCalibDbContext_t *calibv1) {
    convertDehazeCalib2CalibV2(calibv1, calibv2);
}

void CalibConverterAmerge::convert(CamCalibDbV2Context_t *calibv2,
                                   CamCalibDbContext_t *calibv1) {
    CalibDb_Amerge_Para_t* calibv1_amerge =
        (CalibDb_Amerge_Para_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, amerge);
    if (!calibv1_amerge)
        return;

    CalibDbV2_merge_v10_t* calibv2_amerge_calib =
        (CalibDbV2_merge_v10_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, amerge_calib));
    if (!calibv2_amerge_calib)
        return;

    for(int i = 0; i < 13; i++) {
        calibv2_amerge_calib->MergeTuningPara.OECurve.EnvLv[i] = calibv1_amerge->envLevel[i];
        calibv2_amerge_calib->MergeTuningPara.OECurve.Smooth[i] = calibv1_amerge->oeCurve_smooth[i];
        calibv2_amerge_calib->MergeTuningPara.OECurve.Offset[i] = calibv1_amerge->oeCurve_offset[i];
        calibv2_amerge_calib->MergeTuningPara.MDCurve.MoveCoef[i] = calibv1_amerge->moveCoef[i];
        calibv2_amerge_calib->MergeTuningPara.MDCurve.LM_smooth[i] = calibv1_amerge->mdCurveLm_smooth[i];
        calibv2_amerge_calib->MergeTuningPara.MDCurve.LM_offset[i] = calibv1_amerge->mdCurveLm_offset[i];
        calibv2_amerge_calib->MergeTuningPara.MDCurve.MS_smooth[i] = calibv1_amerge->mdCurveMs_smooth[i];
        calibv2_amerge_calib->MergeTuningPara.MDCurve.MS_offset[i] = calibv1_amerge->mdCurveMs_offset[i];
    }
    calibv2_amerge_calib->MergeTuningPara.ByPassThr = 0;
    calibv2_amerge_calib->MergeTuningPara.OECurve_damp = calibv1_amerge->oeCurve_damp;
    calibv2_amerge_calib->MergeTuningPara.MDCurveLM_damp = calibv1_amerge->mdCurveLm_damp;
    calibv2_amerge_calib->MergeTuningPara.MDCurveMS_damp = calibv1_amerge->mdCurveMs_damp;
}

void CalibConverterAtmo::convert(CamCalibDbV2Context_t *calibv2,
                                 CamCalibDbContext_t *calibv1) {
    CalibDb_Atmo_Para_t* calibv1_atmo =
        (CalibDb_Atmo_Para_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, atmo);
    if (!calibv1_atmo )
        return;

    CalibDbV2_tmo_t* calibv2_atmo_calib =
        (CalibDbV2_tmo_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, atmo_calib));
    if (!calibv2_atmo_calib)
        return;

    //len
    calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len = 13;
    calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.ISO_len = calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.Strength_len = calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    calibv2_atmo_calib->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf_len = calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    calibv2_atmo_calib->TmoTuningPara.DetailsHighLight.HighLightData.EnvLv_len = calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    calibv2_atmo_calib->TmoTuningPara.DetailsHighLight.HighLightData.Strength_len = calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma_len = calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.LowLightData.DarkPdf_len = calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.LowLightData.ISO_len = calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.LowLightData.Strength_len = calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    calibv2_atmo_calib->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange_len = calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    calibv2_atmo_calib->TmoTuningPara.GlobaTMO.GlobalTmoData.EnvLv_len = calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    calibv2_atmo_calib->TmoTuningPara.GlobaTMO.GlobalTmoData.Strength_len = calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    calibv2_atmo_calib->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange_len = calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    calibv2_atmo_calib->TmoTuningPara.LocalTMO.LocalTmoData.EnvLv_len = calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;
    calibv2_atmo_calib->TmoTuningPara.LocalTMO.LocalTmoData.Strength_len = calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv_len;

    //malloc
    calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv = (float *) malloc(sizeof(float) * 13);
    calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.ISO = (float *) malloc(sizeof(float) * 13);
    calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.Strength = (float *) malloc(sizeof(float) * 13);
    calibv2_atmo_calib->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf = (float *) malloc(sizeof(float) * 13);
    calibv2_atmo_calib->TmoTuningPara.DetailsHighLight.HighLightData.EnvLv = (float *) malloc(sizeof(float) * 13);
    calibv2_atmo_calib->TmoTuningPara.DetailsHighLight.HighLightData.Strength = (float *) malloc(sizeof(float) * 13);
    calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma = (float *) malloc(sizeof(float) * 13);
    calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.LowLightData.DarkPdf = (float *) malloc(sizeof(float) * 13);
    calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.LowLightData.ISO = (float *) malloc(sizeof(float) * 13);
    calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.LowLightData.Strength = (float *) malloc(sizeof(float) * 13);
    calibv2_atmo_calib->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange = (float *) malloc(sizeof(float) * 13);
    calibv2_atmo_calib->TmoTuningPara.GlobaTMO.GlobalTmoData.EnvLv = (float *) malloc(sizeof(float) * 13);
    calibv2_atmo_calib->TmoTuningPara.GlobaTMO.GlobalTmoData.Strength = (float *) malloc(sizeof(float) * 13);
    calibv2_atmo_calib->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange = (float *) malloc(sizeof(float) * 13);
    calibv2_atmo_calib->TmoTuningPara.LocalTMO.LocalTmoData.EnvLv = (float *) malloc(sizeof(float) * 13);
    calibv2_atmo_calib->TmoTuningPara.LocalTMO.LocalTmoData.Strength = (float *) malloc(sizeof(float) * 13);


    calibv2_atmo_calib->TmoTuningPara.Enable = calibv1_atmo->en[0].en;
    //global luma
    calibv2_atmo_calib->TmoTuningPara.GlobalLuma.Mode = calibv1_atmo->luma[0].GlobalLumaMode ? GLOBALLUMAMODE_ISO : GLOBALLUMAMODE_ENVLV;
    for(int i = 0; i < 13; i++) {
        calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.EnvLv[i] = calibv1_atmo->luma[0].envLevel[i];
        calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.ISO[i] = calibv1_atmo->luma[0].ISO[i];
        calibv2_atmo_calib->TmoTuningPara.GlobalLuma.GlobalLumaData.Strength[i] = calibv1_atmo->luma[0].globalLuma[i];
    }
    calibv2_atmo_calib->TmoTuningPara.GlobalLuma.Tolerance = calibv1_atmo->luma[0].Tolerance;
    //details high light
    calibv2_atmo_calib->TmoTuningPara.DetailsHighLight.Mode = calibv1_atmo->HighLight[0].DetailsHighLightMode ? DETAILSHIGHLIGHTMODE_ENVLV : DETAILSHIGHLIGHTMODE_OEPDF;
    for(int i = 0; i < 13; i++) {

        calibv2_atmo_calib->TmoTuningPara.DetailsHighLight.HighLightData.OEPdf[i] = calibv1_atmo->HighLight[0].OEPdf[i];
        calibv2_atmo_calib->TmoTuningPara.DetailsHighLight.HighLightData.EnvLv[i] = calibv1_atmo->HighLight[0].EnvLv[i];
        calibv2_atmo_calib->TmoTuningPara.DetailsHighLight.HighLightData.Strength[i] = calibv1_atmo->HighLight[0].detailsHighLight[i];
    }
    calibv2_atmo_calib->TmoTuningPara.DetailsHighLight.Tolerance = calibv1_atmo->HighLight[0].Tolerance;
    //details low light
    if(calibv1_atmo->LowLight[0].DetailsLowLightMode == 0)
        calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.Mode = DETAILSLOWLIGHTMODE_FOCUSLUMA;
    else if(calibv1_atmo->LowLight[0].DetailsLowLightMode > 0 && calibv1_atmo->LowLight[0].DetailsLowLightMode < 2)
        calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.Mode = DETAILSLOWLIGHTMODE_DARKPDF;
    else if(calibv1_atmo->LowLight[0].DetailsLowLightMode >= 2)
        calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.Mode = DETAILSLOWLIGHTMODE_ISO;
    for(int i = 0; i < 13; i++) {

        calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.LowLightData.FocusLuma[i] = calibv1_atmo->LowLight[0].FocusLuma[i];
        calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.LowLightData.DarkPdf[i] = calibv1_atmo->LowLight[0].DarkPdf[i];
        calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.LowLightData.ISO[i] = calibv1_atmo->LowLight[0].ISO[i];
        calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.LowLightData.Strength[i] = calibv1_atmo->LowLight[0].detailsLowLight[i];
    }
    calibv2_atmo_calib->TmoTuningPara.DetailsLowLight.Tolerance = calibv1_atmo->LowLight[0].Tolerance;
    //global tmo
    calibv2_atmo_calib->TmoTuningPara.GlobaTMO.Enable = calibv1_atmo->GlobaTMO[0].en ? true : false;
    calibv2_atmo_calib->TmoTuningPara.GlobaTMO.IIR = calibv1_atmo->GlobaTMO[0].iir;
    calibv2_atmo_calib->TmoTuningPara.GlobaTMO.Mode = calibv1_atmo->GlobaTMO[0].mode ? TMOTYPEMODE_ENVLV : TMOTYPEMODE_DYNAMICRANGE;
    for(int i = 0; i < 13; i++) {

        calibv2_atmo_calib->TmoTuningPara.GlobaTMO.GlobalTmoData.DynamicRange[i] = calibv1_atmo->GlobaTMO[0].DynamicRange[i];
        calibv2_atmo_calib->TmoTuningPara.GlobaTMO.GlobalTmoData.EnvLv[i] = calibv1_atmo->GlobaTMO[0].EnvLv[i];
        calibv2_atmo_calib->TmoTuningPara.GlobaTMO.GlobalTmoData.Strength[i] = calibv1_atmo->GlobaTMO[0].Strength[i];
    }
    calibv2_atmo_calib->TmoTuningPara.GlobaTMO.Tolerance = calibv1_atmo->GlobaTMO[0].Tolerance;
    //local tmo
    calibv2_atmo_calib->TmoTuningPara.LocalTMO.Mode = calibv1_atmo->LocalTMO[0].LocalTMOMode ? TMOTYPEMODE_ENVLV : TMOTYPEMODE_DYNAMICRANGE;
    for(int i = 0; i < 13; i++) {

        calibv2_atmo_calib->TmoTuningPara.LocalTMO.LocalTmoData.DynamicRange[i] = calibv1_atmo->LocalTMO[0].DynamicRange[i];
        calibv2_atmo_calib->TmoTuningPara.LocalTMO.LocalTmoData.EnvLv[i] = calibv1_atmo->LocalTMO[0].EnvLv[i];
        calibv2_atmo_calib->TmoTuningPara.LocalTMO.LocalTmoData.Strength[i] = calibv1_atmo->LocalTMO[0].Strength[i];
    }
    calibv2_atmo_calib->TmoTuningPara.LocalTMO.Tolerance = calibv1_atmo->LocalTMO[0].Tolerance;
    calibv2_atmo_calib->TmoTuningPara.damp = calibv1_atmo->damp;
}

void CalibConverterCpsl::convert(CamCalibDbV2Context_t* calibv2,
                                 CamCalibDbContext_t* calibv1) {
    CalibDb_Cpsl_t* calibv1_cpsl =
        (CalibDb_Cpsl_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, cpsl);
    if (!calibv1_cpsl)
        return;

    CalibDbV2_Cpsl_t* calibv2_cpsl_db =
        (CalibDbV2_Cpsl_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, cpsl));
    if (!calibv2_cpsl_db)
        return;
    CalibDbV2_Cpsl_Param_t* calibv2_cpsl_calib = &calibv2_cpsl_db->param;

    calibv2_cpsl_calib->enable = !!calibv1_cpsl->support_en;
    if (calibv1_cpsl->mode == 0) {
        calibv2_cpsl_calib->mode = RK_AIQ_OP_MODE_AUTO;
    } else if (calibv1_cpsl->mode == 1) {
        calibv2_cpsl_calib->mode = RK_AIQ_OP_MODE_MANUAL;
    } else {
        calibv2_cpsl_calib->mode = RK_AIQ_OP_MODE_INVALID;
    }

    calibv2_cpsl_calib->force_gray = !!calibv1_cpsl->gray;
    if (calibv1_cpsl->lght_src == 0) {
        calibv2_cpsl_calib->light_src = LED;
    } else if (calibv1_cpsl->lght_src == 1) {
        calibv2_cpsl_calib->light_src = IR;
    } else if (calibv1_cpsl->lght_src == 2) {
        calibv2_cpsl_calib->light_src = MIX;
    } else {
        calibv2_cpsl_calib->light_src = INVALID;
    }
    calibv2_cpsl_calib->auto_adjust_sens = calibv1_cpsl->ajust_sens;
    calibv2_cpsl_calib->auto_on2off_th = calibv1_cpsl->on2off_th;
    calibv2_cpsl_calib->auto_off2on_th = calibv1_cpsl->off2on_th;
    calibv2_cpsl_calib->manual_on = !!calibv1_cpsl->cpsl_on;
    calibv2_cpsl_calib->manual_strength = calibv1_cpsl->strength;
}

void CalibConverterAgamma::convert(CamCalibDbV2Context_t *calibv2,
                                   CamCalibDbContext_t *calibv1) {
    CalibDb_Gamma_t* calibv1_gamma =
        (CalibDb_Gamma_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, gamma);
    if (!calibv1_gamma)
        return;

    CalibDbV2_gamma_v10_t* calibv2_agamma_calib =
        (CalibDbV2_gamma_v10_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, agamma_calib));
    if (!calibv2_agamma_calib)
        return;

    calibv2_agamma_calib->GammaTuningPara.Gamma_en = calibv1_gamma->gamma_en ? true : false;
    calibv2_agamma_calib->GammaTuningPara.Gamma_out_segnum = calibv1_gamma->gamma_out_segnum ? GAMMATYPE_EQU : GAMMATYPE_LOG;
    calibv2_agamma_calib->GammaTuningPara.Gamma_out_offset = calibv1_gamma->gamma_out_offset;
    for(int i = 0; i < 45; i++)
        calibv2_agamma_calib->GammaTuningPara.Gamma_curve[i] = (int)(calibv1_gamma->curve_normal[i] + 0.5);
}

void CalibConverterBAYERNRV1::convert(CamCalibDbV2Context_t *calibv2,
                                      CamCalibDbContext_t *calibv1) {
    if (CHECK_ISP_HW_V20()) {
        CalibDb_BayerNr_2_t* calibv1_bayerNr =
            (CalibDb_BayerNr_2_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, bayerNr);
        if (!calibv1_bayerNr)
            return;

        CalibDbV2_BayerNrV1_t* calibv2_bayernr_v1 =
            (CalibDbV2_BayerNrV1_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, bayernr_v1));
        if (!calibv2_bayernr_v1)
            return;

        // TODO:
        bayernrV1_calibdb_to_calibdbV2(calibv1_bayerNr, calibv2_bayernr_v1, 0);
    }

}

void CalibConverterMFNRV1::convert(CamCalibDbV2Context_t *calibv2,
                                   CamCalibDbContext_t *calibv1) {
    if (CHECK_ISP_HW_V20()) {
        CalibDb_MFNR_2_t* calibv1_mfnr =
            (CalibDb_MFNR_2_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, mfnr);
        if (!calibv1_mfnr)
            return;

        CalibDbV2_MFNR_t* calibv2_mfnr_v1 =
            (CalibDbV2_MFNR_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, mfnr_v1));
        if (!calibv2_mfnr_v1)
            return;

        // TODO:
        mfnrV1_calibdb_to_calibdbV2(calibv1_mfnr, calibv2_mfnr_v1, 0);
    }

}

void CalibConverterUVNRV1::convert(CamCalibDbV2Context_t *calibv2,
                                   CamCalibDbContext_t *calibv1) {
    if (CHECK_ISP_HW_V20()) {
        CalibDb_UVNR_2_t* calibv1_uvnr =
            (CalibDb_UVNR_2_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, uvnr);
        if (!calibv1_uvnr)
            return;

        CalibDbV2_UVNR_t* calibv2_uvnr_v1 =
            (CalibDbV2_UVNR_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, uvnr_v1));
        if (!calibv2_uvnr_v1)
            return;

        // TODO:
        uvnrV1_calibdb_to_calibdbV2(calibv1_uvnr, calibv2_uvnr_v1, 0);
    }

}

void CalibConverterYNRV1::convert(CamCalibDbV2Context_t *calibv2,
                                  CamCalibDbContext_t *calibv1) {
    if (CHECK_ISP_HW_V20()) {
        CalibDb_YNR_2_t* calibv1_ynr =
            (CalibDb_YNR_2_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, ynr);
        if (!calibv1_ynr)
            return;

        CalibDbV2_YnrV1_t* calibv2_ynr_v1 =
            (CalibDbV2_YnrV1_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, ynr_v1));
        if (!calibv2_ynr_v1)
            return;

        // TODO:
        ynrV1_calibdb_to_calibdbV2(calibv1_ynr, calibv2_ynr_v1, 0);
    }

}

void CalibConverterSHARPV1::convert(CamCalibDbV2Context_t *calibv2,
                                    CamCalibDbContext_t *calibv1) {
    if (CHECK_ISP_HW_V20()) {
        CalibDb_Sharp_2_t* calibv1_sharp =
            (CalibDb_Sharp_2_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, sharp);
        if (!calibv1_sharp)
            return;

        CalibDbV2_SharpV1_t* calibv2_sharp_v1 =
            (CalibDbV2_SharpV1_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, sharp_v1));
        if (!calibv2_sharp_v1)
            return;

        // TODO:
        sharpV1_calibdb_to_calibdbV2(calibv1_sharp, calibv2_sharp_v1, 0);
    }

}

void CalibConverterEDGEFILTERV1::convert(CamCalibDbV2Context_t *calibv2,
        CamCalibDbContext_t *calibv1) {
    if (CHECK_ISP_HW_V20()) {
        CalibDb_EdgeFilter_2_t* calibv1_edgeFilter =
            (CalibDb_EdgeFilter_2_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, edgeFilter);
        if (!calibv1_edgeFilter)
            return;

        CalibDbV2_Edgefilter_t* calibv2_edgefilter_v1 =
            (CalibDbV2_Edgefilter_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, edgefilter_v1));
        if (!calibv2_edgefilter_v1)
            return;

        // TODO:
        edgefilterV1_calibdb_to_calibdbV2(calibv1_edgeFilter, calibv2_edgefilter_v1, 0);
    }

}

void CalibConverterDebayer::convert(CamCalibDbV2Context_t* calibv2,
                                    CamCalibDbContext_t* calibv1) {
    CalibDb_RKDM_t* calibv1_dm =
        (CalibDb_RKDM_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, dm);
    if (!calibv1_dm)
        return;

    CalibDbV2_Debayer_t* calibv2_debayer =
        (CalibDbV2_Debayer_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, debayer));
    if (!calibv2_debayer)
        return;

    calibv2_debayer->param.debayer_en = !!calibv1_dm->debayer_en;
    for (int i = 0; i < 5; i++) {
        calibv2_debayer->param.debayer_filter1[i] = calibv1_dm->debayer_filter1[i];
        calibv2_debayer->param.debayer_filter2[i] = calibv1_dm->debayer_filter2[i];
    }
    calibv2_debayer->param.debayer_gain_offset = calibv1_dm->debayer_gain_offset;
    for (int i = 0; i < 9; i++) {
        calibv2_debayer->param.array.ISO[i] = calibv1_dm->ISO[i];
        calibv2_debayer->param.array.sharp_strength[i] = calibv1_dm->sharp_strength[i];
        calibv2_debayer->param.array.debayer_hf_offset[i] = calibv1_dm->debayer_hf_offset[i];

    }
    calibv2_debayer->param.debayer_gain_offset = calibv1_dm->debayer_gain_offset;
    for (int i = 0; i < 9; i++) {
        calibv2_debayer->param.array.ISO[i] = calibv1_dm->ISO[i];
        calibv2_debayer->param.array.sharp_strength[i] = calibv1_dm->sharp_strength[i];
        calibv2_debayer->param.array.debayer_hf_offset[i] = calibv1_dm->debayer_hf_offset[i];
    }
    calibv2_debayer->param.debayer_offset = calibv1_dm->debayer_offset;
    calibv2_debayer->param.debayer_clip_en = !!calibv1_dm->debayer_clip_en;
    calibv2_debayer->param.debayer_filter_g_en = !!calibv1_dm->debayer_filter_g_en;
    calibv2_debayer->param.debayer_filter_c_en = !!calibv1_dm->debayer_filter_c_en;
    calibv2_debayer->param.debayer_thed0 = calibv1_dm->debayer_thed0;
    calibv2_debayer->param.debayer_thed1 = calibv1_dm->debayer_thed1;
    calibv2_debayer->param.debayer_dist_scale = calibv1_dm->debayer_dist_scale;
    calibv2_debayer->param.debayer_cnr_strength = calibv1_dm->debayer_cnr_strength;
    calibv2_debayer->param.debayer_shift_num = calibv1_dm->debayer_shift_num;
}

void CalibConverterCproc::convert(CamCalibDbV2Context_t* calibv2,
                                  CamCalibDbContext_t* calibv1) {
    CalibDb_cProc_t* calibv1_cProc =
        (CalibDb_cProc_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, cProc);
    if (!calibv1_cProc)
        return;

    CalibDbV2_Cproc_t* calibv2_cproc =
        (CalibDbV2_Cproc_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, cproc));
    if (!calibv2_cproc)
        return;

    calibv2_cproc->param.enable = !!calibv1_cProc->enable;
    calibv2_cproc->param.brightness = calibv1_cProc->brightness;
    calibv2_cproc->param.contrast = calibv1_cProc->contrast;
    calibv2_cproc->param.saturation = calibv1_cProc->saturation;
    calibv2_cproc->param.hue = calibv1_cProc->hue;
}

void CalibConverterIE::convert(CamCalibDbV2Context_t* calibv2,
                               CamCalibDbContext_t* calibv1) {
    CalibDb_IE_t* calibv1_ie =
        (CalibDb_IE_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, ie);
    if (!calibv1_ie)
        return;

    CalibDbV2_IE_t* calibv2_ie =
        (CalibDbV2_IE_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, ie));
    if (!calibv2_ie)
        return;

    calibv2_ie->param.enable = !!calibv1_ie->enable;
    calibv2_ie->param.mode = calibv1_ie->mode;
}

void CalibConverterCCM::convert(CamCalibDbV2Context_t *calibv2,
                                CamCalibDbContext_t *calibv1) {

    convertCCMCalib2CalibV2(calibv1, calibv2);
}

void CalibConverterLUT3D::convert(CamCalibDbV2Context_t *calibv2,
                                  CamCalibDbContext_t *calibv1) {
    convertLut3DCalib2CalibV2(calibv1, calibv2);
}

void CalibConverterAdrc::convert(CamCalibDbV2Context_t *calibv2,
                                 CamCalibDbContext_t *calibv1) {

    convertAdrcCalibV1ToCalibV2(calibv1, calibv2);
}

/*

   void CalibConverterAF::convert(CamCalibDbV2Context_t* calibv2,
   CamCalibDbContext_t* calibv1)
   {

   }

   void CalibConverterASD::convert(CamCalibDbV2Context_t* calibv2,
   CamCalibDbContext_t* calibv1)
   {

   }

   .
   .
   .

*/

int IQConverter::addToScene(CamCalibDbProj_t *calibpj, const char *main_scene,
                            const char *sub_scene,
                            CamCalibDbV2Context_t *calibv2) {
    auto sub_vector = new CamCalibSubSceneList_t();
    auto main_vector = new CamCalibMainSceneList_t();

    sub_vector[0].name = strdup(sub_scene);
    memcpy(calibdbv2_get_scene_ptr(&sub_vector[0]),
           calibv2->calib_scene, calibdbV2_scene_ctx_size(calibv2));

    main_vector[0].name = strdup(main_scene);
    main_vector[0].sub_scene = sub_vector;
    main_vector[0].sub_scene_len = 1;

    calibpj->main_scene = main_vector;
    calibpj->main_scene_len = 1;

    return 0;
}

void CalibConverterALSC::convert(
    CamCalibDbV2Context_t *calibv2, CamCalibDbContext_t *calibv1)
{
    CalibDb_Lsc_t* calibv1_lsc =
        (CalibDb_Lsc_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, lsc);
    if (!calibv1_lsc)
        return;

    CalibDbV2_LSC_t* calibv2_lsc_v2 =
        (CalibDbV2_LSC_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, lsc_v2));
    if (!calibv2_lsc_v2)
        return;

    CalibDb_LscTableProfile_t* tableV1;
    CalibDbV2_LscTableProfile_t* tableV2;

    CalibDbV2_LSC_t& lsc_v2 = *calibv2_lsc_v2;
    CalibDbV2_Lsc_Common_t& common = lsc_v2.common;

    common.enable = calibv1_lsc->enable;
    common.resolutionAll_len = calibv1_lsc->aLscCof.lscResNum;
    common.resolutionAll = (CalibDbV2_Lsc_Resolution_t*)
                           malloc(common.resolutionAll_len * sizeof(CalibDbV2_Lsc_Resolution_t));
    memset(common.resolutionAll, '\0', common.resolutionAll_len * sizeof(CalibDbV2_Lsc_Resolution_t));
    for (int i = 0; i < common.resolutionAll_len; i++) {
        strcpy(common.resolutionAll[i].name, calibv1_lsc->aLscCof.lscResName[i]);
    }

    lsc_v2.alscCoef.damp_enable = calibv1_lsc->damp_enable;

    int illuNumTotal = 0;
    CalibDb_AlscCof_t& pAlscCof = calibv1_lsc->aLscCof;
    for (int i = 0; i < USED_FOR_CASE_MAX; i++) {
        illuNumTotal += pAlscCof.illuNum[i];
    }
    lsc_v2.alscCoef.illAll_len = illuNumTotal;
    lsc_v2.alscCoef.illAll = (CalibDbV2_AlscCof_ill_t*)
                             malloc(illuNumTotal * sizeof(CalibDbV2_AlscCof_ill_t));
    CalibDbV2_AlscCof_ill_t* ill_v2 = lsc_v2.alscCoef.illAll;

    for (int ucase = 0; ucase < USED_FOR_CASE_MAX; ucase++) {
        int illuNum = pAlscCof.illuNum[ucase];
        for (int i = 0; i < illuNum; i++) {
            ill_v2->usedForCase = ucase;
            CalibDb_AlscCof_ill_t& ill_v1 = pAlscCof.illAll[ucase][i];
            strcpy(ill_v2->name, ill_v1.illuName);
            ill_v2->wbGain[0] = ill_v1.awbGain[0];
            ill_v2->wbGain[1] = ill_v1.awbGain[1];
            ill_v2->tableUsed_len = ill_v1.tableUsedNO;
            ill_v2->tableUsed = (lsc_name_t*)malloc(ill_v2->tableUsed_len * sizeof(lsc_name_t));
            for (int t = 0; t < ill_v2->tableUsed_len; t++) {
                strcpy(ill_v2->tableUsed[t].name, ill_v1.tableUsed[t]);
            }
            ill_v2->gains_len = ill_v1.vignettingCurve.arraySize;
            ill_v2->vig_len = ill_v1.vignettingCurve.arraySize;
            ill_v2->gains = (float*)malloc(ill_v2->gains_len * sizeof(float));
            ill_v2->vig   = (float*)malloc(ill_v2->vig_len * sizeof(float));
            for (int gains_id = 0; gains_id < ill_v2->gains_len; gains_id++) {
                ill_v2->gains[gains_id] = ill_v1.vignettingCurve.pSensorGain[gains_id];
            }
            for (int vig_id = 0; vig_id < ill_v2->vig_len; vig_id++) {
                ill_v2->vig[vig_id] = ill_v1.vignettingCurve.pVignetting[vig_id];
            }

            ill_v2++;
        }
    }

    lsc_v2.tbl.tableAll_len = calibv1_lsc->tableAllNum;
    lsc_v2.tbl.tableAll = (CalibDbV2_LscTableProfile_t*)
                          malloc(lsc_v2.tbl.tableAll_len * sizeof(CalibDbV2_LscTableProfile_t));

    for (int i = 0; i < lsc_v2.tbl.tableAll_len; i++) {
        tableV1 = calibv1_lsc->tableAll + i;
        tableV2 = lsc_v2.tbl.tableAll + i;

        memset(tableV2->name, '\0', sizeof(tableV2->name));
        strcpy(tableV2->name, tableV1->name);
        strcpy(tableV2->resolution, tableV1->resolution);
        strcpy(tableV2->illumination, tableV1->illumination);
        tableV2->vignetting = tableV1->vignetting;

        for (int j = 0; j < lsc_v2.common.resolutionAll_len; j++) {
            int ret = strcmp(lsc_v2.common.resolutionAll[j].name, tableV2->resolution);
            if(0 == ret) {
                if (g_rkaiq_isp_hw_ver == 20 || g_rkaiq_isp_hw_ver == 22) {
                    memcpy(lsc_v2.common.resolutionAll[j].lsc_sect_size_x, tableV1->LscXSizeTbl, sizeof(tableV1->LscXSizeTbl));
                    memcpy(lsc_v2.common.resolutionAll[j].lsc_sect_size_y, tableV1->LscYSizeTbl, sizeof(tableV1->LscYSizeTbl));
                } else if (g_rkaiq_isp_hw_ver == 30 || g_rkaiq_isp_hw_ver == 32 || g_rkaiq_isp_hw_ver == 321) {
                    memcpy(lsc_v2.common.resolutionAll[j].lsc_sect_size_x, tableV1->LscXSizeTbl, sizeof(tableV1->LscXSizeTbl));
                    memcpy(lsc_v2.common.resolutionAll[j].lsc_sect_size_x+8, tableV1->LscXSizeTbl, sizeof(tableV1->LscXSizeTbl));
                    memcpy(lsc_v2.common.resolutionAll[j].lsc_sect_size_y, tableV1->LscYSizeTbl, sizeof(tableV1->LscYSizeTbl));
                    memcpy(lsc_v2.common.resolutionAll[j].lsc_sect_size_y+8, tableV1->LscYSizeTbl, sizeof(tableV1->LscYSizeTbl));
                }

            }
        }

        memcpy(&tableV2->lsc_samples_red,    &tableV1->LscMatrix[CAM_4CH_COLOR_COMPONENT_RED],    sizeof(Cam17x17UShortMatrix_t));
        memcpy(&tableV2->lsc_samples_greenR, &tableV1->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR], sizeof(Cam17x17UShortMatrix_t));
        memcpy(&tableV2->lsc_samples_greenB, &tableV1->LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB], sizeof(Cam17x17UShortMatrix_t));
        memcpy(&tableV2->lsc_samples_blue,   &tableV1->LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE],   sizeof(Cam17x17UShortMatrix_t));
    }
}

void CalibConverterAfec::convert(
    CamCalibDbV2Context_t *calibv2, CamCalibDbContext_t *calibv1)
{
    CalibDb_FEC_t* calibv1_fec =
        (CalibDb_FEC_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, afec);
    if (!calibv1_fec)
        return;

    CalibDbV2_FEC_t* calibv2_fec_db =
        (CalibDbV2_FEC_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, afec));
    if (!calibv2_fec_db)
        return;
    CalibDbV2_Fec_Param_t* calibv2_fec = &calibv2_fec_db->param;

    memcpy(calibv2_fec, calibv1_fec, sizeof(*calibv1_fec));
}

void CalibConverterAldch::convert(
    CamCalibDbV2Context_t *calibv2, CamCalibDbContext_t *calibv1)
{
    CalibDb_LDCH_t* calibv1_ldch =
        (CalibDb_LDCH_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, aldch);
    if (!calibv1_ldch)
        return;

    CalibDbV2_LDCH_t* calibv2_ldch_db =
        (CalibDbV2_LDCH_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, aldch));
    if (!calibv2_ldch_db)
        return;
    CalibDbV2_Ldch_Param_t* calibv2_ldch = &calibv2_ldch_db->param;

    calibv2_ldch->ldch_en           = calibv1_ldch->ldch_en;
    calibv2_ldch->correct_level     = calibv1_ldch->correct_level;
    calibv2_ldch->correct_level_max = calibv1_ldch->correct_level_max;
    memcpy(calibv2_ldch->meshfile, calibv1_ldch->meshfile, strlen(calibv1_ldch->meshfile));
    memcpy(calibv2_ldch->light_center, calibv1_ldch->light_center, sizeof(double) * 2);
    memcpy(calibv2_ldch->coefficient, calibv1_ldch->coefficient, sizeof(double) * 4);
}

void CalibConverterColorAsGrey::convert(
    CamCalibDbV2Context_t *calibv2, CamCalibDbContext_t *calibv1)
{
    CalibDb_ColorAsGrey_t* calibv1_colorAsGrey =
        (CalibDb_ColorAsGrey_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, colorAsGrey);
    if (!calibv1_colorAsGrey)
        return;

    CalibDbV2_ColorAsGrey_t* calibv2_colorAsGrey =
        (CalibDbV2_ColorAsGrey_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, colorAsGrey));
    if (!calibv2_colorAsGrey)
        return;

    memcpy(calibv2_colorAsGrey, calibv1_colorAsGrey, sizeof(*calibv1_colorAsGrey));
}

void CalibConverterLumaDetect::convert(
    CamCalibDbV2Context_t *calibv2, CamCalibDbContext_t *calibv1)
{
    CalibDb_LUMA_DETECT_t* calibv1_lumaDetect =
        (CalibDb_LUMA_DETECT_t*)CALIBDB_GET_MODULE_PTR((void*)calibv1, lumaDetect);
    if (!calibv1_lumaDetect)
        return;

    CalibDbV2_LUMA_DETECT_t* calibv2_lumaDetect =
        (CalibDbV2_LUMA_DETECT_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, lumaDetect));
    if (!calibv2_lumaDetect)
        return;

    memcpy(calibv2_lumaDetect, calibv1_lumaDetect, sizeof(*calibv1_lumaDetect));
}

void CalibConverterAf::convert(
    CamCalibDbV2Context_t *calibv2, CamCalibDbContext_t *calibv1)
{
    convertAfCalibV1ToCalibV2(calibv1, calibv2);
}

void CalibConverterThumbnails::convert(
    CamCalibDbV2Context_t *calibv2, CamCalibDbContext_t *calibv1)
{
    CalibDbV2_Thumbnails_t* calibv2_thumbnails_db =
        (CalibDbV2_Thumbnails_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, thumbnails));
    if (!calibv2_thumbnails_db)
        return;

    CalibDbV2_Thumbnails_Param_t* calibv2_thumbnails = &calibv2_thumbnails_db->param;
    calibv2_thumbnails->thumbnail_configs_len = 3;

    calibv2_thumbnails->thumbnail_configs = reinterpret_cast<rkaiq_thumbnails_config_t*>(calloc(
            calibv2_thumbnails->thumbnail_configs_len, sizeof(*calibv2_thumbnails->thumbnail_configs)));
    if (NULL == calibv2_thumbnails->thumbnail_configs) {
        goto failure;
        return;
    }

    for (uint32_t i = 0; i < calibv2_thumbnails->thumbnail_configs_len; i++) {
        auto* config = &calibv2_thumbnails->thumbnail_configs[i];
        config->owner_cookies = i;
        config->stream_type = 0;
        config->format[0] = 'N';
        config->format[1] = 'V';
        config->format[2] = '1';
        config->format[3] = '2';
        config->width_intfactor = 1 << (i + 1);
        config->height_intfactor = 1 << (i + 1);
    }

    return;

failure:
    if (calibv2_thumbnails->thumbnail_configs != NULL) {
        free(calibv2_thumbnails->thumbnail_configs);
    }

}


void CalibConverterBAYERNRV2::convert(CamCalibDbV2Context_t *calibv2,
                                      CamCalibDbContext_t *calibv1) {
    if (CHECK_ISP_HW_V21()) {
        struct list_head* bayernrV2_list =
            (struct list_head*)CALIBDB_GET_MODULE_PTR((void*)calibv1, list_bayernr_v2);
        if (!bayernrV2_list)
            return;

        CalibDbV2_BayerNrV2_t* calibv2_bayernr_v2 =
            (CalibDbV2_BayerNrV2_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, bayernr_v2));
        if (!calibv2_bayernr_v2)
            return;

        // TODO:
        bayernrV2_calibdb_to_calibdbV2(bayernrV2_list, calibv2_bayernr_v2, 0);
    }

}

void CalibConverterCNRV1::convert(CamCalibDbV2Context_t *calibv2,
                                  CamCalibDbContext_t *calibv1) {
    if (CHECK_ISP_HW_V21()) {
        struct list_head* calibv1_cnr_list =
            (struct list_head*)CALIBDB_GET_MODULE_PTR((void*)calibv1, list_cnr_v1);
        if (!calibv1_cnr_list)
            return;

        CalibDbV2_CNR_t* calibv2_cnr_v1 =
            (CalibDbV2_CNR_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, cnr_v1));
        if (!calibv2_cnr_v1)
            return;

        // TODO:
        cnrV1_calibdb_to_calibdbV2(calibv1_cnr_list, calibv2_cnr_v1, 0);
    }

}

void CalibConverterYNRV2::convert(CamCalibDbV2Context_t *calibv2,
                                  CamCalibDbContext_t *calibv1) {
    if (CHECK_ISP_HW_V21()) {
        struct list_head* calibv1_ynr_list =
            (struct list_head*)CALIBDB_GET_MODULE_PTR((void*)calibv1, list_ynr_v2);
        if (!calibv1_ynr_list)
            return;

        CalibDbV2_YnrV2_t* calibv2_ynr_v2 =
            (CalibDbV2_YnrV2_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, ynr_v2));
        if (!calibv2_ynr_v2)
            return;

        // TODO:
        ynrV2_calibdb_to_calibdbV2(calibv1_ynr_list, calibv2_ynr_v2, 0);
    }

}

void CalibConverterSHARPV3::convert(CamCalibDbV2Context_t *calibv2,
                                    CamCalibDbContext_t *calibv1) {
    if (CHECK_ISP_HW_V21()) {
        struct list_head* calibv1_sharp_list =
            (struct list_head*)CALIBDB_GET_MODULE_PTR((void*)calibv1, list_sharp_v3);
        if (!calibv1_sharp_list)
            return;

        CalibDbV2_SharpV3_t* calibv2_sharp_v2 =
            (CalibDbV2_SharpV3_t*)(CALIBDBV2_GET_MODULE_PTR(calibv2, sharp_v3));
        if (!calibv2_sharp_v2)
            return;

        // TODO:
        sharpV3_calibdb_to_calibdbV2(calibv1_sharp_list, calibv2_sharp_v2, 0);
    }
}

} // namespace RkCam
