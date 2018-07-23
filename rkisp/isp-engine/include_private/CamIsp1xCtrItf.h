#ifndef _CAM_ISP_CTRL_ITF_IMC_H_
#define _CAM_ISP_CTRL_ITF_IMC_H_
#include "CamIspCtrItf.h"
#include "CamHwItf.h"
#include <cam_ia_api/cam_ia10_engine_api.h>
#include <sys/poll.h>
#include <base/xcam_3a_description.h>

using namespace std;

#define CAM_ISP_NUM_OF_STAT_BUFS  4

class CamIsp1xCtrItf: public CamIspCtrItf {

 public:
  CamIsp1xCtrItf();
  virtual ~CamIsp1xCtrItf();
  virtual bool init(const char* tuningFile,
                    const char* ispDev,
                    int devFd);
  virtual bool deInit();
  virtual bool configure(const Configuration& config);
  virtual bool start();
  virtual bool stop();
  /* control ISP module directly*/  
  virtual bool setISPDeviceFd(int ispFd);
  virtual void setExternalAEHandlerDesc(XCamAEDescription* desc);
  virtual void setExternalAWBHandlerDesc(XCamAWBDescription* desc);
  virtual void setExternalAFHandlerDesc(XCamAFDescription* desc);
  virtual struct CamIA10_DyCfg* getDynamicISPConfig() { return &mCamIA_DyCfg; }
  virtual bool configureISP(const void* config);
  virtual void transDrvMetaDataToHal(const void* drvMeta, struct HAL_Buffer_MetaData* halMeta);
  virtual bool runIA(struct CamIA10_DyCfg* ia_dcfg,
                     struct CamIA10_Stats* ia_stats,
                     struct CamIA10_Results* ia_results);
  virtual bool runISPManual(struct CamIA10_Results* ia_results, bool_t lock);
  /*
    virtual bool awbConfig(struct HAL_AwbCfg *cfg);
    virtual bool aecConfig(struct HAL_AecCfg *cfg);
    virtual bool afcConfig(struct HAL_AfcCfg *cfg);

    virtual bool ispFunLock(unsigned int fun_lock);
    virtual bool ispFunEnable(unsigned int fun_en);
    virtual bool ispFunDisable(unsigned int fun_dis);
    virtual bool getIspFunStats(unsigned int  *lock, unsigned int *enable);
    */
 protected:
/*
  virtual bool threadLoop(void) = 0;
  class ISP3AThread : public CamThread {
   public:
    ISP3AThread(CamIsp1xCtrItf* owener): mOwener(owener) {};
    virtual bool threadLoop(void) {return mOwener->threadLoop();};
   private:
    CamIsp1xCtrItf* mOwener;
  };
  shared_ptr<ISP3AThread> mISP3AThread;
*/

  virtual bool initISPStream(const char* ispDev);
  virtual bool getMeasurement(struct v4l2_buffer& v4l2_buf);
  virtual bool releaseMeasurement(struct v4l2_buffer* v4l2_buf);
  virtual bool stopMeasurements();
  virtual bool startMeasurements();

  unsigned int mSupportedSubDevs;
  int switchSubDevIrCutMode(int mode);
  int mIspFd;
  void* mIspStatBuf[CAM_ISP_NUM_OF_STAT_BUFS];
  unsigned int mIspStatBufSize;

  unsigned int mStartCnt;
  unsigned int mIspFunEn;
  unsigned int mIspFunLock;
  struct CamIA10_DyCfg mCamIA_DyCfg;

  osMutex mApiLock;
  bool mStreaming;
  unsigned short mFramesToDrop;
  int mInitialized;
  shared_ptr<CamIA10EngineItf> mCamIAEngine;
  //manual isp related,
  //notice that 3A algorithm result will be covered by manual isp settings
  /* black level subtraction */
  struct HAL_ISP_bls_cfg_s bls_cfg;
  bool_t mBlsNeededUpdate;
  enum HAL_ISP_ACTIVE_MODE  mBlsEnabled;
  /* sensor degamma */
  struct HAL_ISP_sdg_cfg_s sdg_cfg;
  bool_t mSdgNeededUpdate;
  enum HAL_ISP_ACTIVE_MODE  mSdgEnabled;
  /* filter */
  struct HAL_ISP_flt_cfg_s flt_cfg;
  bool_t mFltNeededUpdate;
  enum HAL_ISP_ACTIVE_MODE  mFltEnabled;
  /* gamma out correction*/
  struct HAL_ISP_goc_cfg_s goc_cfg;
  bool_t mGocNeededUpdate;
  enum HAL_ISP_ACTIVE_MODE  mGocEnabled;
  /*color process*/
  struct HAL_ISP_cproc_cfg_s cproc_cfg;
  bool_t mCprocNeededUpdate;
  enum HAL_ISP_ACTIVE_MODE  mCprocEnabled;
  /* image effect */
  struct HAL_ISP_ie_cfg_s ie_cfg;
  bool_t mIeNeededUpdate;
  enum HAL_ISP_ACTIVE_MODE  mIeEnabled;
  /* lens shading correct */
  struct HAL_ISP_lsc_cfg_s lsc_cfg;
  bool_t mLscNeededUpdate;
  enum HAL_ISP_ACTIVE_MODE  mLscEnabled;
  /* white balance gain */
  struct HAL_ISP_awb_gain_cfg_s awb_gain_cfg;
  bool_t mAwbGainNeededUpdate;
  enum HAL_ISP_ACTIVE_MODE  mAwbEnabled;
  /* cross talk(color correction matrix)*/
  struct HAL_ISP_ctk_cfg_s ctk_cfg;
  bool_t mCtkNeededUpdate;
  enum HAL_ISP_ACTIVE_MODE  mCtkEnabled;
  /* exposure measure*/
  struct HAL_ISP_aec_cfg_s aec_cfg;
  bool_t mAecNeededUpdate;
  enum HAL_ISP_ACTIVE_MODE  mAecEnabled;
  /* denoise pre filter*/
  struct HAL_ISP_dpf_cfg_s dpf_cfg;
  bool_t mDpfNeededUpdate;
  enum HAL_ISP_ACTIVE_MODE  mDpfEnabled;
  struct HAL_ISP_dpf_strength_cfg_s dpf_strength_cfg;
  bool_t mDpfStrengthNeededUpdate;
  enum HAL_ISP_ACTIVE_MODE  mDpfStrengthEnabled;
  /* auto focus control measure*/
  struct HAL_ISP_afc_cfg_s afc_cfg;
  bool_t mAfcNeededUpdate;
  enum HAL_ISP_ACTIVE_MODE  mAfcEnabled;
  /* white balance measure*/
  struct HAL_ISP_awb_meas_cfg_s awb_cfg;
  bool_t mAwbMeNeededUpdate;
  enum HAL_ISP_ACTIVE_MODE  mAwbMeEnabled;
  /* wide dynamic range*/
  struct HAL_ISP_wdr_cfg_s wdr_cfg;
  bool_t mWdrNeededUpdate;
  enum HAL_ISP_ACTIVE_MODE  mWdrEnabled;
  /* dead pixel correction */
  struct HAL_ISP_dpcc_cfg_s dpcc_cfg;
  bool_t mDpccNeededUpdate;
  enum HAL_ISP_ACTIVE_MODE  mDpccEnabled;
  /* histogram measure*/
  struct HAL_ISP_hst_cfg_s hst_cfg;
  bool_t mHstNeededUpdate;
  enum HAL_ISP_ACTIVE_MODE  mHstEnabled;
  /* bayer demosaic*/
  struct HAL_ISP_bdm_cfg_s bdm_cfg;
  bool_t mBdmNeededUpdate;
  enum HAL_ISP_ACTIVE_MODE  mBdmEnabled;
  /* used for switchSubDevIrCutMode*/
  enum HAL_WB_MODE mLastWbMode;
 private:

};


#endif
