#ifndef _CAM_IA10_ENGINE_H_
#define _CAM_IA10_ENGINE_H_

#include <calib_xml/calibdb.h>
#include <cam_calibdb/cam_calibdb_api.h>

#include "cam_ia10_engine_api.h"
#include "cameric.h"
#include <awb/awb.h>
//#include <awb/awbConvert.h>
#include <base/xcam_3a_description.h>

class CamIA10Engine: public CamIA10EngineItf {
 public:
  CamIA10Engine();
  virtual ~CamIA10Engine();

  virtual RESULT init();
  virtual RESULT deinit();
  virtual RESULT restart();

  virtual RESULT initStatic(char* aiqb_data_file, const char* sensor_entity_name, int isp_ver);
  virtual RESULT initDynamic(struct CamIA10_DyCfg* cfg);
  virtual RESULT setStatistics(struct CamIA10_Stats* stats);

  virtual RESULT updateAeConfig(struct CamIA10_DyCfg* cfg);
  virtual RESULT updateAwbConfig(struct CamIA10_DyCfg* cfg);
  virtual RESULT updateAfConfig(struct CamIA10_DyCfg* cfg);

  virtual RESULT runAe(XCamAeParam *param, AecResult_t* result, bool first = false);
  virtual RESULT runAwb(XCamAwbParam *param, CamIA10_AWB_Result_t* result, bool first = false);
  virtual RESULT runAf(XCamAfParam *param, XCam3aResultFocus* result, bool first = false);

  virtual void dumpAe();
  virtual void dumpAwb();

  virtual void setExternalAEHandlerDesc(XCamAEDescription* desc);
  virtual void setExternalAWBHandlerDesc(XCamAWBDescription* desc);
  virtual void setExternalAFHandlerDesc(XCamAFDescription* desc);

  virtual RESULT runAEC(HAL_AecCfg* config = NULL);
  virtual RESULT getAECResults(AecResult_t* result);

  virtual RESULT runAWB(HAL_AwbCfg* config = NULL);
  virtual RESULT getAWBResults(CamIA10_AWB_Result_t* result);

  virtual RESULT runADPF();
  virtual RESULT getADPFResults(AdpfResult_t* result);

  virtual RESULT runAF(HAL_AfcCfg* config = NULL);
  virtual RESULT getAFResults(XCam3aResultFocus* result);

  virtual RESULT runAWDR();
  virtual RESULT getAWDRResults(AwdrResult_t* result);

  /* manual ISP configs*/
  virtual RESULT runManISP(struct HAL_ISP_cfg_s* manCfg, struct CamIA10_Results* result);
  virtual void mapSensorExpToHal(
      int sensorGain,
      int sensorInttime,
      float& halGain,
      float& halInttime);

   struct CamIA10_Stats mStats;
   char g_aiqb_data_file[256];
 
 private:
  void convertAwbResult2Cameric
  (
      AwbRunningOutputResult_t* awbResult,
      CamIA10_AWB_Result_t* awbCamicResult
  );

  void updateAwbResults
  (
      CamIA10_AWB_Result_t* old,
      CamIA10_AWB_Result_t* newcfg,
      CamIA10_AWB_Result_t* update
  );

  void mapHalExpToSensor
  (
      float hal_gain,
      float hal_time,
      int& sensor_gain,
      int& sensor_time
  );

  uint32_t calcAfmTenengradShift
  (
      const uint32_t MaxPixelCnt
  );
  
  uint32_t calcAfmLuminanceShift
  (
      const uint32_t MaxPixelCnt
  );

  int mFrameId = 0;
  CamCalibDbHandle_t  hCamCalibDb;
  bool mInitDynamic;
  struct CamIA10_DyCfg  dCfg;
  struct CamIA10_DyCfg  dCfgShd;

  XCam3AContext        *aecContext;
  XCamAEDescription    *aecDesc;
  XCamAeParam          *aecParams;

  XCam3AContext        *awbContext;
  XCamAWBDescription   *awbDesc;
  XCamAwbParam         *awbParams;

  XCam3AContext        *afContext;
  XCamAFDescription    *afDesc;
  XCamAfParam          *afParams;

  AfHandle_t  hAf;
  AdpfHandle_t  hAdpf;
  AwdrHandle_t  hAwdr;
  AwbHandle_t hAwb;
  AfConfig_t afcCfg;
  AwbConfig_t awbcfg;
  AecConfig_t aecCfg;
  AdpfConfig_t adpfCfg;
  AwdrConfig_t awdrCfg;
  HAL_AecCfg mAECHalCfg;
  HAL_AwbCfg mAWBHalCfg;
  HAL_AfcCfg mAFCHalCfg;
  HAL_AecCfg mAECHalCfgShd;
  HAL_AwbCfg mAWBHalCfgShd;
  HAL_AfcCfg mAFCHalCfgShd;

  
  CamIA10_AWB_Result_t lastAwbResult;
  CamIA10_AWB_Result_t curAwbResult;
  AecResult_t      lastAecResult;
  AecResult_t      curAecResult;
  bool_t mStatisticsUpdated;

  bool_t  mWdrEnabledState;
  enum LIGHT_MODE mLightMode;
 private:
  RESULT initAEC();
  RESULT initAWB();
  RESULT initADPF();
  RESULT initAF();
  RESULT initAWDR();
  RESULT runManIspForBW(struct CamIA10_Results* result);
  RESULT runManIspForPreIsp(struct CamIA10_Results* result);
  RESULT runManIspForOTP(struct CamIA10_Results* result);
  const char* mSensorEntityName;
  int mIspVer;
  int mXMLIspOutputType;
  CamOTPGlobal_t* mOTPInfo;
};

#endif

