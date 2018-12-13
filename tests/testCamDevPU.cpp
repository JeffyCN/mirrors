#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#include <linux/videodev2.h>
#include <rkisp_control_loop.h>
#include <rkisp_dev_manager.h>
#include "mediactl.h"

#include <CamDevManagerForPU.h>
#include <CameraBuffer.h>
#include <ProxyCameraBuffer.h>
#include <StrmPUBase.h>

using namespace std;
#define TEST_PU
//#define TEST_WRITE_TO_FILE
#define CLEAR(x) memset(&(x), 0, sizeof(x))

#define DECLAREPERFORMANCETRACK(name) \
  static int mFrameCount##name = 0;\
  static int mLastFrameCount##name = 0;\
  static long long mLastFpsTime##name = 0;\
  static float mFps##name = 0;

#define SHOWPERFORMACEFPS(name) \
  { \
    mFrameCount##name++;\
    if (!(mFrameCount##name & 0x1F)) { \
      struct timeval now; \
      gettimeofday(&now,NULL); \
      long long diff = now.tv_sec*1000000 + now.tv_usec - mLastFpsTime##name; \
      mFps##name = ((mFrameCount##name - mLastFrameCount##name) * float(1000*1000)) / diff; \
      mLastFpsTime##name = now.tv_sec*1000000 + now.tv_usec; \
      mLastFrameCount##name = mFrameCount##name; \
      printf("%s:%d Frames, %2.3f FPS\n",#name, mFrameCount##name, mFps##name); \
    } \
  }

#define MAX_RK_CAMS_DEV 4
#define MAX_MEDIA_INDEX 32
#define MAX_DEV_NOTE 32

enum device_tyep {
  TYPE_INVALID = 0,
  TYPE_ISP,
  TYPE_CIF,
  TYPE_UVC,
};

struct node_info {
  char name[128];
  char node[128];
};

struct rk_cams_dev_info {
  int type;
  int node_num;
  struct node_info nodes[MAX_DEV_NOTE];
};

static char iq_file[255] = "/etc/cam_iq.xml";

static frm_info_t g_frmFmt = {
  .frmSize = {1920, 1080},
  .frmFmt = HAL_FRMAE_FMT_NV12,
  .colorSpace = HAL_COLORSPACE_SMPTE170M,
  .fps = 30,
};

static const char* getNodeByName(const char *name, struct rk_cams_dev_info *cam_infos) {
  int i;
  const char *node = NULL;

  for (i = 0; i < cam_infos->node_num; i++)
    if (!strcmp(name, cam_infos->nodes[i].name)) {
      node = cam_infos->nodes[i].node;
      break;
    }
  return node;
}

static int getCameraInfos(struct rk_cams_dev_info *cam_infos) {
  char sys_path[64];
  struct media_device *device = NULL;
  uint32_t nents, k, j, i = 0;
  int ret, fd = -1, index = 0;

  // check media device as isp cif
  while (i < MAX_MEDIA_INDEX) {
    snprintf (sys_path, sizeof(sys_path), "/dev/media%d", i++);
    fd = open(sys_path, O_RDONLY);
    if (fd < 0)
      continue;
    close (fd);

    device = media_device_new (sys_path);
    /* Enumerate entities, pads and links. */
    media_device_enumerate (device);
    nents = media_get_entities_count (device);
    cam_infos->node_num = nents;
    for (j = 0; j < nents; ++j) {
      struct media_entity *entity = media_get_entity (device, j);
      const struct media_entity_desc *entity_info = media_entity_get_info(entity);
      const char *devname = media_entity_get_devname (entity);
      if (strstr(entity_info->name, "rkisp"))
        cam_infos->type = TYPE_ISP;
      else if (strstr(entity_info->name, "cif"))
        cam_infos->type = TYPE_CIF;
      memcpy(cam_infos->nodes[j].name, entity_info->name, sizeof(cam_infos->nodes[j].name));
      memcpy(cam_infos->nodes[j].node, devname, sizeof(cam_infos->nodes[j].node));
    }
    media_device_unref (device);
    if (index < MAX_RK_CAMS_DEV - 1) {
      index++;
      cam_infos++;
    }
  }

  // TODO: check uvc device
  return 0;
}

class TestPU : public StreamPUBase {
public:
  TestPU() : StreamPUBase("TestPU", true, true) {}
  ~TestPU() {}
  bool processFrame(shared_ptr<BufferBase> inBuf, shared_ptr<BufferBase> outBuf) {
    static int64_t pre_ts = 0; // us
    if (inBuf.get()) {
       printf("buff:%p index:%d sequence:%d fps:%d(us)\n",
         (char *)inBuf->getVirtAddr(),
         inBuf->getIndex(),
         inBuf->getSequence(),
         inBuf->getTimestamp() - pre_ts);
       pre_ts = inBuf->getTimestamp();
       // TODO: process frame
       #ifdef TEST_WRITE_TO_FILE
       char fname[50] = {0};
       static int frames = 0;
       snprintf(fname, 30, "/tmp/yuv_%dx%d.bin", inBuf->getWidth(), inBuf->getHeight());
       frames++;
       if ((frames > 25) && (frames < 29)) {
         FILE* yuv_file =  fopen(fname, "a+");
         if (yuv_file) {
           fwrite(inBuf->getVirtAddr(), inBuf->getDataSize(), 1, yuv_file);
           printf("write 0x%x bytes to file!\n", inBuf->getDataSize());
           fclose(yuv_file);
         } else
           printf("open file %s error\n", fname);
       }
       #endif
    }
    return true;
  }
};

static void testStreamPU(struct rk_cams_dev_info *cam_infos) {
  void *device_manager = NULL;
  bool enable_3a = true;
  const char *mp = getNodeByName("rkisp1_mainpath", cam_infos);
  if (!mp) {
    printf("err: don't find mp device\n");
    return;
  }

  if (enable_3a && access(iq_file, F_OK) == 0) {
    if (!rkisp_cl_init(&device_manager, iq_file, NULL)) {
      struct rkisp_cl_prepare_params_s params;
      CLEAR(params);
      params.isp_sd_node_path = getNodeByName("rkisp1-isp-subdev", cam_infos);
      printf("%s\n", params.isp_sd_node_path);
      params.isp_vd_params_path = getNodeByName("rkisp1-input-params", cam_infos);
      printf("%s\n", params.isp_vd_params_path);
      params.isp_vd_stats_path = getNodeByName("rkisp1-statistics", cam_infos);
      printf("%s\n", params.isp_vd_stats_path);
      /* assume the last is sensor_subdev */
      params.sensor_sd_node_path = cam_infos->nodes[cam_infos->node_num - 1].node;
      printf("%s\n", params.sensor_sd_node_path);

      rkisp_cl_prepare(device_manager, &params);
      if (rkisp_cl_start(device_manager) != 0) {
        printf("rkisp cl start fail\n");
        return;
      }
    } else {
      device_manager = NULL;
      printf("rkisp cl init fail\n");
      return;
    }
  }

  shared_ptr<CamDevManagerForPU> mpDev =
      shared_ptr<CamDevManagerForPU>(new CamDevManagerForPU());

  if (mpDev->init(mp) == false) {
    printf("capture mp dev init error !\n");
  } else {
    //frm_info_t frmFmt;
    //memcpy(&frmFmt, &(g_frmFmt), sizeof(frm_info_t));

    if (mpDev->prepare(g_frmFmt, 4)) {
      TestPU* mTestPU = new TestPU();
      mTestPU->setReqFmt(g_frmFmt);
      mTestPU->prepare(g_frmFmt, 0, NULL);
      mpDev->addBufferNotifier(mTestPU);

      mTestPU->start();
      if (mpDev->start())
        printf("mp dev start success!\n");

      getchar();

      mpDev->removeBufferNotifer(mTestPU);
      mTestPU->stop();
      if (device_manager)
        rkisp_cl_stop(device_manager);
      mpDev->stop();
      delete mTestPU;
    } else {
      printf("dev prepare fail!\n");
    }
    printf("test PU complete........\n");
  }
  mpDev->deinit();
  mpDev.reset();

  if (device_manager) {
    rkisp_cl_deinit(device_manager);
    device_manager = NULL;
  }
}

class CambufNotifierImp : public NewCameraBufferReadyNotifier {
 private:
  CamDevManagerForPU* mDev;
 public:
  CambufNotifierImp(CamDevManagerForPU* dev = NULL): mDev(dev) {}
  ~CambufNotifierImp() {}
  virtual bool bufferReady(weak_ptr<BufferBase> buffer, int status) {
    UNUSED_PARAM(status);
    DECLAREPERFORMANCETRACK(CambufNotifierImp);
    shared_ptr<BufferBase> mpCamBuf = buffer.lock();
    if (mpCamBuf.get()) {
      mpCamBuf->incUsedCnt();
      SHOWPERFORMACEFPS(CambufNotifierImp);
      // TODO process frame
      mpCamBuf->decUsedCnt();
    }
    return true;
  }
};

static void testStreamNotifier(struct rk_cams_dev_info *cam_infos) {
  const char *mp = getNodeByName("rkisp1_mainpath", cam_infos);
  if (!mp) {
    printf("err: don't find mp device\n");
    return;
  }

  shared_ptr<CamDevManagerForPU> mpDev =
      shared_ptr<CamDevManagerForPU>(new CamDevManagerForPU());

  if (mpDev->init(mp) == false) {
    printf("capture mp dev init error !\n");
  } else {
    //frm_info_t frmFmt;
    //memcpy(&frmFmt, &(g_frmFmt), sizeof(frm_info_t));

    if (mpDev->prepare(g_frmFmt, 4)) {
      NewCameraBufferReadyNotifier* mBufNotifer = new CambufNotifierImp(mpDev.get());
      mBufNotifer->setReqFmt(g_frmFmt);
      mpDev->addBufferNotifier(mBufNotifer);
      if (mpDev->start())
        printf("mp dev start success!\n");

      getchar();

      mpDev->stop();
      delete mBufNotifer;
    } else {
      printf("dev prepare fail!\n");
    }
    printf("test Notifier complete........\n");
  }
  mpDev->deinit();
  mpDev.reset();
}

int main(int argc, const char* argv[]) {
  int i;
  //get all connected camera infos
  struct rk_cams_dev_info g_cam_infos[MAX_RK_CAMS_DEV];
  memset(g_cam_infos, 0, sizeof(g_cam_infos));
  getCameraInfos(g_cam_infos);

  for (i = 0; i < MAX_RK_CAMS_DEV; i++)
    if (g_cam_infos[i].type == TYPE_ISP)
      break;
  if (i == MAX_RK_CAMS_DEV) {
    printf("err: don't find isp device\n");
    return -1;
  }

#ifdef TEST_PU
  testStreamPU(&g_cam_infos[i]);
#else
  testStreamNotifier(&g_cam_infos[i]);
#endif
  return 0;
}

