#ifndef _V4L2_ISP10_IOCTL_H
#define _V4L2_ISP10_IOCTL_H

#include <linux/videodev2.h>
#include <linux/rk-isp10-ioctl.h>
#include "ebase/types.h"

using namespace std;

class V4l2Isp10Ioctl {
 public:
  V4l2Isp10Ioctl(int devFp);
  virtual ~V4l2Isp10Ioctl(void);
  /* dpcc */
  bool getDpccCfg(struct cifisp_dpcc_config& dpccCfg);
  bool setDpccCfg(struct cifisp_dpcc_config& dpccCfg, bool enable);
  /* bls */
  bool getBlsCfg(struct cifisp_bls_config& blsCfg);
  bool setBlsCfg(struct cifisp_bls_config& blsCfg, bool enable);
  /* degamma */
  bool getSdgCfg(struct cifisp_sdg_config& sdgCfg);
  bool setSdgCfg(struct cifisp_sdg_config& sdgCfg, bool enable);
  /* lsc */
  bool getLscCfg(struct cifisp_lsc_config& lscCfg);
  bool setLscCfg(struct cifisp_lsc_config& lscCfg, bool enable);
  /* awb measure */
  bool getAwbMeasCfg(struct cifisp_awb_meas_config& awbCfg);
  bool setAwbMeasCfg(struct cifisp_awb_meas_config& awbCfg, bool enable);
  /* filter */
  bool getFltCfg(struct cifisp_flt_config& fltCfg);
  bool setFltCfg(struct cifisp_flt_config& fltCfg, bool enable);
  /* demosaic */
  bool getBdmCfg(struct cifisp_bdm_config& bdmCfg);
  bool setBdmCfg(struct cifisp_bdm_config& bdmCfg, bool enable);
  /* cross talk */
  bool getCtkCfg(struct cifisp_ctk_config& ctkCfg);
  bool setCtkCfg(struct cifisp_ctk_config& ctkCfg, bool enable);
  /* gamma out */
  bool getGocCfg(struct cifisp_goc_config& gocCfg);
  bool setGocCfg(struct cifisp_goc_config& gocCfg, bool enable);
  /* Histogram Measurement */
  bool getHstCfg(struct cifisp_hst_config& hstCfg);
  bool setHstCfg(struct cifisp_hst_config& hstCfg, bool enable);
  /* Auto Exposure Measurements */
  bool getAecCfg(struct cifisp_aec_config& aecCfg);
  bool setAecCfg(struct cifisp_aec_config& aecCfg, bool enable);
  /* awb gain */
  bool getAwbGainCfg(struct cifisp_awb_gain_config& awbgCfg);
  bool setAwbGainCfg(struct cifisp_awb_gain_config& awbgCfg, bool enable);
  /* color process */
  bool getCprocCfg(struct cifisp_cproc_config& cprocCfg);
  bool setCprocCfg(struct cifisp_cproc_config& cprocCfg, bool enable);
  /* afc */
  bool getAfcCfg(struct cifisp_afc_config& afcCfg);
  bool setAfcCfg(struct cifisp_afc_config& afcCfg, bool enable);
  /* ie */
  bool getIeCfg(struct cifisp_ie_config& ieCfg);
  bool setIeCfg(struct cifisp_ie_config& ieCfg, bool enable);
  /* dpf */
  bool getDpfCfg(struct cifisp_dpf_config& dpfCfg);
  bool setDpfCfg(struct cifisp_dpf_config& dpfCfg, bool enable);

  bool getDpfStrengthCfg(struct cifisp_dpf_strength_config& dpfStrCfg);
  bool setDpfStrengthCfg(struct cifisp_dpf_strength_config& dpfStrCfg, bool enable);

  /* last capture config */
  bool getLastCapCfg(struct cifisp_last_capture_config& lastCapCfg);
  /* get statistics */
  bool getAwbStat(struct cifisp_awb_stat& awbStat);
  bool getAeStat(struct cifisp_ae_stat& aeStat);
  bool getAfStat(struct cifisp_af_stat& afStat);
 private:
  bool setControl(int id, bool enable);
  int mDevFp;

};
#endif

