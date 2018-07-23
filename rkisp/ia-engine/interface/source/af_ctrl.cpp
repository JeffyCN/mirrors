#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <sys/ioctl.h>
#include <calib_xml/calibdb.h>
#include <utils/Log.h>
#include <cam_ia_api/cam_ia10_engine_api.h>
#include <HAL/CamIsp10CtrItf.h>

#include <isp_ctrl.h>
#include <af_ctrl.h>

extern int mVideoFd;

int getCtrl(unsigned int id) {
  struct v4l2_control ctrl;
  int ret;

  ctrl.id = id;
  ret = ioctl(mVideoFd, VIDIOC_G_CTRL, &ctrl);
  if (ret < 0) {
    LOGV("ERR(%s): VIDIOC_G_CTRL(id = 0x%x (%d)) failed, ret = %d\n",
         __func__, id, id - V4L2_CID_PRIVATE_BASE, ret);
    return ret;
  }
  return ctrl.value;
}

int setCtrl(unsigned int id, unsigned int value) {
  struct v4l2_control ctrl;
  int ret;

  ctrl.id = id;
  ctrl.value = value;
  ret = ioctl(mVideoFd, VIDIOC_S_CTRL, &ctrl);
  if (ret < 0) {
    LOGE("ERR(%s):VIDIOC_S_CTRL(id = %#x (%d), value = %d) failed ret = %d\n",
         __func__, id, id - V4L2_CID_PRIVATE_BASE, value, ret);
  }

  return ret;
}

/* AF interface */
RESULT rkisp_af_focus_set(void* handle, const uint32_t AbsStep) {
  int ret;
  int position;

  position = AbsStep;
  ret = setCtrl(V4L2_CID_FOCUS_ABSOLUTE, position);
  if (ret < 0) {
    LOGE("Could not set focus, error %d", ret);
  }
  return ret;
}

RESULT rkisp_af_focus_get(void* handle, uint32_t* pAbsStep) {
  RESULT ret;
  int position;

  position = getCtrl(V4L2_CID_FOCUS_ABSOLUTE);
  if (ret < 0) {
    LOGE("Could not set focus, error %d", ret);
  }

  *pAbsStep = (uint32_t)position;
  return ret;
}

RESULT rkisp_init_moto_drive(void* handle) {
  return 0;
}

RESULT rkisp_setup_moto_drive(void* handle, uint32_t* pMaxStep) {
  RESULT ret;
  struct isp_supplemental_sensor_mode_data data;

  ret = getSensorModeData(mVideoFd, &data);
  if (ret < 0) {
    LOGE("Could not set focus, error %d", ret);
  }

  *pMaxStep = data.vcm_max_step;
  ret = rkisp_af_focus_set(handle, *pMaxStep & 0xFFFF);
  return ret;
}
