#ifndef _V4L2_IOCTL_H
#define _V4L2_IOCTL_H

#include <sys/poll.h>
#include <linux/videodev2.h>
#include <linux/media/v4l2-controls_rockchip.h>
#include "CameraBuffer.h"
#include "../../include/oslayer/oslayer.h"
#include "cam_types.h"
#include <list>
#include <vector>

using namespace std;

class V4L2DevIoctr {
 public:
  V4L2DevIoctr(int devFp, const uint32_t v4l2BufType, const uint32_t v4l2MemType);
  virtual ~V4L2DevIoctr(void);
  int queueBuffer(shared_ptr<BufferBase>& buffer);
  int dequeueBuffer(shared_ptr<BufferBase>& buffer, unsigned long timeout_ms);
  int setFormat(uint32_t& v4l2PixFmt, uint32_t& v4l2ColorSpace, uint32_t& width, uint32_t& height, uint32_t stride) const;
  bool getDefFmt(uint32_t& fmt);
  bool findFmt(uint32_t fmt);
  void enumFormat(vector<uint32_t>& fmtVec);
  void enumFrmSize(uint32_t fmt, vector<frm_size_t>& frmSizeVec);
  void enumFrmFps(uint32_t fmt, uint32_t width, uint32_t height, vector<HAL_FPS_INFO_t>& fpsVec);
  int tryFormat(uint32_t& v4l2PixFmt, uint32_t& width, uint32_t& height);
  int tryFps(uint32_t v4l2PixFmt, uint32_t width, uint32_t height, HAL_FPS_INFO_t fps);
  int enumSTD(vector<struct v4l2_standard>& stds);
  int getSTD(v4l2_std_id& std);
  int setSTD(v4l2_std_id std);

  int requestBuffers(uint32_t numBuffers) const;
  int streamOn(void);
  int streamOff(void);
  int enumInput(struct v4l2_input* input);
  int setInput(int index);
  int getCtrl(uint32_t id);
  int setCtrl(uint32_t id, uint32_t value);
  int setExtCtrls(struct v4l2_ext_control* ctrls, unsigned int ctrClass, int ctrNum);
  int queryCap(unsigned int queryCap);
  int queryCtrl(struct v4l2_queryctrl* ctr);
  int queryMenu(struct v4l2_querymenu* menu);
  int setStrmPara(struct v4l2_streamparm* parm);
  int getStrmPara(struct v4l2_streamparm* parm);
  int queryCapInfo(struct v4l2_capability* cap);
  int queryBusInfo(unsigned char* busInfo);
  int queryCropCap(struct v4l2_cropcap& cropCap);
  int getCrop(struct v4l2_crop& crop);
  int setCrop(struct v4l2_crop& crop);
  bool error(void) const { return mError;};
  //add for V4L2_MEMORY_MMAP memory type
  virtual bool memMap(shared_ptr<BufferBase>& buffer);
  virtual bool memUnmap(shared_ptr<BufferBase>& buffer);
  uint32_t getCurFmt() const { return mFmt;}
  uint32_t getCurWidth() const { return mWidth;}
  uint32_t getCurHeight() const { return mHeight;}
  uint32_t getCurStride() const { return mStride;}
  int getSensorModeData(struct isp_supplemental_sensor_mode_data* data);
  int getCameraModuleInfo(struct camera_module_info_s* camera_module);
  static unsigned int halFmtToV4l2Fmt(unsigned int halFmt);
  static RK_FRMAE_FORMAT V4l2FmtToHalFmt(unsigned int v4l2fmt);
  static unsigned int halColorSpaceToV4l2ColorSpace(unsigned int halColorSpace);  
  virtual bool getBufferMetaData(int index, struct HAL_Buffer_MetaData* metaData)
  {return false;}
 protected:
  const int mDevFp;
  mutable int mReqBufCnt;
 private:
  static uint32_t bytesPerLine(uint32_t v4l2PixFmt, uint32_t width);
  static uint32_t bpp(uint32_t v4l2PixFmt);
  //const uint32_t mV4L2BufferType;
  const v4l2_buf_type mV4L2BufferType;
  //const uint32_t mV4L2MemType;
  const v4l2_memory mV4L2MemType;
  struct pollfd mEvents;
  mutable osMutex mBufferQueueLock;
  list<weak_ptr<BufferBase> > mBufferQueue;
  bool mError;
  bool mStreaming;
  mutable uint32_t mFmt;
  mutable uint32_t mWidth;
  mutable uint32_t mHeight;
  mutable uint32_t mStride;
};

class V4L2ISPDevIoctr : public V4L2DevIoctr {
 public:
  V4L2ISPDevIoctr(int devFp, const uint32_t v4l2BufType, const uint32_t v4l2MemType):
    V4L2DevIoctr(devFp, v4l2BufType, v4l2MemType) {
    memset(mMetaDataBuf, 0, sizeof(mMetaDataBuf));
    mMaped = 0;
  }
  virtual ~V4L2ISPDevIoctr(void) {}
  virtual bool getBufferMetaData(int index, struct HAL_Buffer_MetaData* metaData);
  /* used for meta data*/
  bool memMap(shared_ptr<BufferBase>& buffer);
  bool memUnmap(shared_ptr<BufferBase>& buffer);
 private:
#define MAX_ISP_META_DATA_BUF_NUM 32
  void* mMetaDataBuf[MAX_ISP_META_DATA_BUF_NUM];
  int mMaped;
  char* mMetaDataAddr;
};


#endif


