/*
 * Copyright (C) 2016 Rockchip Electronics Co.Ltd
 * Authors:
 *	Zhiqin Wei <wzq@rock-chips.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */

#define LOG_NDEBUG 0
#define LOG_TAG "RockchipRgaTest"

#include <stdint.h>
#include <sys/types.h>
#include <math.h>
#include <fcntl.h>
#include <utils/misc.h>
#include <signal.h>
#include <time.h>

#include <cutils/properties.h>

#include <androidfw/AssetManager.h>
#include <binder/IPCThreadState.h>
#include <utils/Atomic.h>
#include <utils/Errors.h>
#include <utils/Log.h>

#include <ui/PixelFormat.h>
#include <ui/Rect.h>
#include <ui/Region.h>
#include <ui/DisplayInfo.h>
#include <ui/GraphicBufferMapper.h>

#include <gui/ISurfaceComposer.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>


// TODO: Fix Skia.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <SkBitmap.h>
#include <SkStream.h>
#include <SkImageDecoder.h>
#pragma GCC diagnostic pop

#include <GLES/gl.h>
#include <GLES/glext.h>
#include <EGL/eglext.h>

#include "RockchipRgaTest.h"
#include "AudioPlayer.h"

#define OEM_BOOTANIMATION_FILE "/oem/media/bootanimation.zip"
#define SYSTEM_BOOTANIMATION_FILE "/system/media/bootanimation.zip"
#define SYSTEM_ENCRYPTED_BOOTANIMATION_FILE "/system/media/bootanimation-encrypted.zip"
#define EXIT_PROP_NAME "service.bootanim.exit"

namespace android {

static const int ANIM_ENTRY_NAME_MAX = 256;

// ---------------------------------------------------------------------------

RockchipRgaTest::RockchipRgaTest() : Thread(false), mZip(NULL)
{
    rgaTest(true);
    mSession = new SurfaceComposerClient();
}

bool RockchipRgaTest::rgaTest(bool prepare)
{
    //rgaFillColorTest(prepare);
    //rgaCopyTest(prepare);
    //rgaConvert(prepare);
    //rgaScale(prepare);
    //rgaRotateScale(prepare);
    rgaNv12ScaleTest(prepare);
    return true;
}

RockchipRgaTest::~RockchipRgaTest() {
    if (mZip != NULL) {
        delete mZip;
    }
}

void RockchipRgaTest::onFirstRef() {
    status_t err = mSession->linkToComposerDeath(this);
    ALOGE_IF(err, "linkToComposerDeath failed (%s) ", strerror(-err));
    if (err == NO_ERROR) {
        run("RockchipRgaTest", PRIORITY_DISPLAY);
    }
}

sp<SurfaceComposerClient> RockchipRgaTest::session() const {
    return mSession;
}


void RockchipRgaTest::binderDied(const wp<IBinder>&)
{
    // woah, surfaceflinger died!
    ALOGD("SurfaceFlinger died, exiting...");

    // calling requestExit() is not enough here because the Surface code
    // might be blocked on a condition variable that will never be updated.
    kill( getpid(), SIGKILL );
    requestExit();
    if (mAudioPlayer != NULL) {
        mAudioPlayer->requestExit();
    }
}

status_t RockchipRgaTest::readyToRun() {
    mAssets.addDefaultAssets();

    sp<IBinder> dtoken(SurfaceComposerClient::getBuiltInDisplay(
            ISurfaceComposer::eDisplayIdMain));
    DisplayInfo dinfo;
    status_t status = SurfaceComposerClient::getDisplayInfo(dtoken, &dinfo);
    if (status)
        return -1;

    // create the native surface
    //sp<SurfaceControl> control = session()->createSurface(String8("RockchipRgaTest"),
            //dinfo.w, dinfo.h, mFormat);

    sp<SurfaceControl> control = session()->createSurface(String8("RockchipRgaTest"),
        mWidth, mHeight, mFormat);


    SurfaceComposerClient::openGlobalTransaction();
    control->setLayer(0x40000000);
    SurfaceComposerClient::closeGlobalTransaction();

    sp<Surface> s = control->getSurface();

    mFlingerSurfaceControl = control;
    mFlingerSurface = s;

    return NO_ERROR;
}

bool RockchipRgaTest::threadLoop()
{
    bool r;
    // We have no bootanimation file, so we use the stock android logo
    // animation.
 
    r = android(); 

    mFlingerSurface.clear();
    mFlingerSurfaceControl.clear();
    IPCThreadState::self()->stopProcess();
    return r;
}

bool RockchipRgaTest::android()
{
    int fenceFd = -1;
    ANativeWindowBuffer_t* mAndoridNativeWindowBuffer;
    sp<ANativeWindow> mAndoridNativeWindow(mFlingerSurface);

    GraphicBufferMapper &mgbMapper = GraphicBufferMapper::get();

    int index = 0;
    drm_rga_t rects;
    int data[4] = {0x5000000,0x00ff0000,0x0000ff00,0x000000ff};

    size_t bufferCount = 6;
    int ret = mAndoridNativeWindow->perform(mAndoridNativeWindow.get(),
                            NATIVE_WINDOW_SET_BUFFER_COUNT,bufferCount);
    if(ret) printf("setbuffercount buffer error : %s\n",strerror(errno));

    /*
    *init rga moudle
    */
    ret = hw_get_module(DRMRGA_HARDWARE_MODULE_ID, &module);
    if (ret) {
        printf("%s,%d faile get hw moudle\n",__func__,__LINE__);
        return -1;
    }

    ret = rga_open(module, &mRga);
    if (ret) {
        printf("device failed to initialize (%s)\n", strerror(-ret));
        return -1;
    }
    printf("init rga ok copy=%p,FillColor=%p\n",mRga->rgaCopy,mRga->rgaFillColor);

    rgaTest(false);

    return true;
}

bool RockchipRgaTest::rgaFillColorTest(bool prepare)
{
    int fenceFd = -1;
    int ret = 0;

    if (prepare) {
        mFormat = HAL_PIXEL_FORMAT_RGBX_8888;
        mWidth = 1200;
        mHeight = 1920;
        return true;
    }
    
    ANativeWindowBuffer_t* mAndoridNativeWindowBuffer;
    sp<ANativeWindow> mAndoridNativeWindow(mFlingerSurface);

    GraphicBufferMapper &mgbMapper = GraphicBufferMapper::get();

    int index = 0;
    drm_rga_t rects;
    int data[4] = {0x5000000,0x00ff0000,0x0000ff00,0x000000ff};

    while(1) {
        ret = mAndoridNativeWindow->dequeueBuffer(mAndoridNativeWindow.get(),
                                      &mAndoridNativeWindowBuffer,&fenceFd);

        if (fenceFd > -1) {
            sp<Fence> fence = new Fence(fenceFd);
            fence->wait(520);
            printf("fenceFd is not null fenceFd=%d\n",fenceFd);
        }

        if (ret)
            printf("dequeue buffer error : %s\n",strerror(errno));
        else {
            ALOGD("init rga ok copy=%p,FillColor=%p\n",mRga->rgaCopy,mRga->rgaFillColor);
            KeyedVector<buffer_handle_t,int>& dqList(deQueueList);
            dqList.add(mAndoridNativeWindowBuffer->handle,deQueue);
            KeyedVector<buffer_handle_t,int>& mpList(mappedList);
            if (mpList.indexOfKey(mAndoridNativeWindowBuffer->handle) < 0) {
                printf("registerBuffer hnd=%p\n",(void*)(mAndoridNativeWindowBuffer->handle));
                ALOGD("registerBuffer hnd=%p\n",(void*)(mAndoridNativeWindowBuffer->handle));
                mgbMapper.registerBuffer(mAndoridNativeWindowBuffer->handle);
                mpList.add(mAndoridNativeWindowBuffer->handle,maped);
            } else if (mpList.valueFor(mAndoridNativeWindowBuffer->handle) != maped) {
                printf("registerBuffer hnd=%p,get map=%d\n",(void*)(mAndoridNativeWindowBuffer->handle),
                                                  mpList.valueFor(mAndoridNativeWindowBuffer->handle));
                ALOGD("registerBuffer hnd=%p\n",(void*)(mAndoridNativeWindowBuffer->handle));
                mgbMapper.registerBuffer(mAndoridNativeWindowBuffer->handle);
                mpList.add(mAndoridNativeWindowBuffer->handle,maped);
            }
        }

        ret = mRga->rgaFillColor(mRga,mAndoridNativeWindowBuffer->handle,
                                                    data[(index++)%4],NULL);
        if (ret) {
            printf("rgaFillColor error : %s,hnd=%p\n",
                            strerror(errno),(void*)(mAndoridNativeWindowBuffer->handle));
            ALOGD("rgaFillColor error : %s,hnd=%p\n",
                            strerror(errno),(void*)(mAndoridNativeWindowBuffer->handle));
        }

        ret = mAndoridNativeWindow->queueBuffer(mAndoridNativeWindow.get(),
                                      mAndoridNativeWindowBuffer,-1);
        if (ret)
            printf("queue buffer error : %s\n",strerror(errno));
        else {
            KeyedVector<buffer_handle_t,int>& dqList(deQueueList);
            dqList.removeItem(mAndoridNativeWindowBuffer->handle);
        }

        printf("threadloop\n");
        usleep(500000);
    }    
}

bool RockchipRgaTest::rgaNv12ScaleTest(bool prepare)
{
    int fenceFd = -1;
    int ret = 0;

    if (prepare) {
        mFormat = HAL_PIXEL_FORMAT_RGBA_8888;
        mWidth = 1280;
        mHeight = 720;
        return true;
    }

    ANativeWindowBuffer_t* mDstAdNtBuffer;
    sp<ANativeWindow> mAndoridNativeWindow(mFlingerSurface);

    GraphicBufferMapper &mgbMapper = GraphicBufferMapper::get();

    int index = 0;
    drm_rga_t rects;
    rga_set_rect(&rects.src,0,0,1280,720,1280,HAL_PIXEL_FORMAT_YCrCb_NV12);
    //rga_set_rect(&rects.dst,0,0,1280,720,1280,HAL_PIXEL_FORMAT_YCrCb_NV12);
    rga_set_rect(&rects.dst,0,0,1280,720,1280,mFormat);


    char* buf = NULL;
    sp<GraphicBuffer> gb(new GraphicBuffer(1280,720,HAL_PIXEL_FORMAT_YCrCb_NV12,
                                                  GRALLOC_USAGE_SW_WRITE_OFTEN));

    if (gb->initCheck()) {
        printf("GraphicBuffer error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("GraphicBuffer ok : %s\n","*************************************");

    mgbMapper.registerBuffer(gb->handle);
    ret = gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);
    
    if (ret) {
        printf("lock buffer error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("lock buffer ok : %s\n","**************************************");

#if 1
    const char *yuvFilePath = "/data/fb1280x720.yuv";
    FILE *file = fopen(yuvFilePath, "rb");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", yuvFilePath);
        return false;
    }
    fread(buf, 2 * 1280 * 720, 1, file);
    fclose(file);
#else
    memset(buf,0x55,4*1200*1920);
#endif
    ret = gb->unlock();

    if (ret) {
        printf("unlock buffer error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("unlock buffer ok : %s\n","*************************************");

    while(1) {
        ret = mAndoridNativeWindow->dequeueBuffer(mAndoridNativeWindow.get(),
                                                    &mDstAdNtBuffer,&fenceFd);

        if (fenceFd > -1) {
            sp<Fence> fence = new Fence(fenceFd);
            fence->wait(520);
            printf("fenceFd is not null fenceFd=%d\n",fenceFd);
        }
        
        if (ret)
            printf("dequeue buffer error : %s\n",strerror(errno));
        else {
            ALOGD("init rga ok copy=%p,FillColor=%p\n",mRga->rgaCopy,mRga->rgaFillColor);
            KeyedVector<buffer_handle_t,int>& dqList(deQueueList);
            dqList.add(mDstAdNtBuffer->handle,deQueue);
            KeyedVector<buffer_handle_t,int>& mpList(mappedList);
            if (mpList.indexOfKey(mDstAdNtBuffer->handle) < 0) {
                printf("registerBuffer hnd=%p\n",(void*)(mDstAdNtBuffer->handle));
                ALOGD("registerBuffer hnd=%p\n",(void*)(mDstAdNtBuffer->handle));
                mgbMapper.registerBuffer(mDstAdNtBuffer->handle);
                mpList.add(mDstAdNtBuffer->handle,maped);
            } else if (mpList.valueFor(mDstAdNtBuffer->handle) != maped) {
                printf("registerBuffer hnd=%p,get map=%d\n",
                                                (void*)(mDstAdNtBuffer->handle),
                                        mpList.valueFor(mDstAdNtBuffer->handle));
                ALOGD("registerBuffer hnd=%p\n",(void*)(mDstAdNtBuffer->handle));
                mgbMapper.registerBuffer(mDstAdNtBuffer->handle);
                mpList.add(mDstAdNtBuffer->handle,maped);
            }
        }

        ret = mRga->rgaScale(mRga,gb->handle,mDstAdNtBuffer->handle,&rects);

        if (ret) {
            printf("rgaFillColor error : %s,hnd=%p\n",
                            strerror(errno),(void*)(mDstAdNtBuffer->handle));
            ALOGD("rgaFillColor error : %s,hnd=%p\n",
                            strerror(errno),(void*)(mDstAdNtBuffer->handle));
        }

        {
            char* buf = NULL;
            sp<GraphicBuffer> gb(new GraphicBuffer(mDstAdNtBuffer,false));
            ret = gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);
#if 0
            const char *yuvFilePath = "/data/fb1280x720.yuv";
            FILE *file = fopen(yuvFilePath, "rb");
            if (!file) {
                fprintf(stderr, "Could not open %s\n", yuvFilePath);
                return false;
            }
            fread(buf, 2 * 1280 * 720, 1, file);
            fclose(file);
#else
            const char *outFilePath = "/data/outBuffer.bin";
            FILE *file = fopen(outFilePath, "wb+");
            if (!file) {
                fprintf(stderr, "Could not open %s\n", outFilePath);
                return false;
            }
            fwrite(buf, 1.5 * 1920 * 1088, 1, file);
            fclose(file);
#endif
            ret = gb->unlock();
        }

        ret = mAndoridNativeWindow->queueBuffer(mAndoridNativeWindow.get(),
                                      mDstAdNtBuffer,-1);
        if (ret)
            printf("queue buffer error : %s\n",strerror(errno));
        else {
            KeyedVector<buffer_handle_t,int>& dqList(deQueueList);
            dqList.removeItem(mDstAdNtBuffer->handle);
        }

        printf("threadloop\n");
        usleep(500000);
    }    
}

bool RockchipRgaTest::rgaCopyTest(bool prepare)
{
    int fenceFd = -1;
    int ret = 0;

    if (prepare) {
        mFormat = HAL_PIXEL_FORMAT_RGBX_8888;
        mWidth = 1200;
        mHeight = 1920;
        return true;
    }
    
    ANativeWindowBuffer_t* mSrcAdNtBuffer;
    ANativeWindowBuffer_t* mDstAdNtBuffer;
    sp<ANativeWindow> mAndoridNativeWindow(mFlingerSurface);

    GraphicBufferMapper &mgbMapper = GraphicBufferMapper::get();

    int index = 0;
    drm_rga_t rects;

    ret = mAndoridNativeWindow->dequeueBuffer(mAndoridNativeWindow.get(),
                                                    &mSrcAdNtBuffer,&fenceFd);

    /*unable:just for test.it will tear down if not sync it*/
    if (fenceFd > -1) {
        sp<Fence> fence = new Fence(fenceFd);
        fence->wait(520);
        printf("fenceFd is not null fenceFd=%d\n",fenceFd);
    }
    
    if (ret) {
        printf("dequeue buffer error : %s\n",strerror(errno));
        return ret;
    } else {
        ALOGD("init rga ok copy=%p,FillColor=%p\n",mRga->rgaCopy,mRga->rgaFillColor);
        KeyedVector<buffer_handle_t,int>& dqList(deQueueList);
        dqList.add(mSrcAdNtBuffer->handle,deQueue);
        KeyedVector<buffer_handle_t,int>& mpList(mappedList);
        if (mpList.indexOfKey(mSrcAdNtBuffer->handle) < 0) {
            printf("registerBuffer hnd=%p\n",(void*)(mSrcAdNtBuffer->handle));
            ALOGD("registerBuffer hnd=%p\n",(void*)(mSrcAdNtBuffer->handle));
            mgbMapper.registerBuffer(mSrcAdNtBuffer->handle);
            mpList.add(mSrcAdNtBuffer->handle,maped);
        } else if (mpList.valueFor(mSrcAdNtBuffer->handle) != maped) {
            printf("registerBuffer hnd=%p,get map=%d\n",
                                            (void*)(mSrcAdNtBuffer->handle),
                                    mpList.valueFor(mSrcAdNtBuffer->handle));
            ALOGD("registerBuffer hnd=%p\n",(void*)(mSrcAdNtBuffer->handle));
            mgbMapper.registerBuffer(mSrcAdNtBuffer->handle);
            mpList.add(mSrcAdNtBuffer->handle,maped);
        }
    }

    char* buf = NULL;
    sp<GraphicBuffer> gb(new GraphicBuffer(mSrcAdNtBuffer,false));
    ret = gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);

    if (ret) {
        printf("lock buffer error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("lock buffer ok : %s\n","**************************************");

#if 1
    const char *yuvFilePath = "/data/dmlayer0_1920_1088_3.bin";
    FILE *file = fopen(yuvFilePath, "rb");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", yuvFilePath);
        return false;
    }
    fread(buf, 1.5 * 1920 * 1088, 1, file);
    fclose(file);
#else
    memset(buf,0x55,4*1200*1920);
#endif

    ret = gb->unlock();

    if (ret) {
        printf("unlock buffer error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("unlock buffer ok : %s\n","*************************************");

    while(1) {
        ret = mAndoridNativeWindow->dequeueBuffer(mAndoridNativeWindow.get(),
                                                    &mDstAdNtBuffer,&fenceFd);

        if (fenceFd > -1) {
            sp<Fence> fence = new Fence(fenceFd);
            fence->wait(520);
            printf("fenceFd is not null fenceFd=%d\n",fenceFd);
        }
        
        if (ret)
            printf("dequeue buffer error : %s\n",strerror(errno));
        else {
            ALOGD("init rga ok copy=%p,FillColor=%p\n",mRga->rgaCopy,mRga->rgaFillColor);
            KeyedVector<buffer_handle_t,int>& dqList(deQueueList);
            dqList.add(mDstAdNtBuffer->handle,deQueue);
            KeyedVector<buffer_handle_t,int>& mpList(mappedList);
            if (mpList.indexOfKey(mDstAdNtBuffer->handle) < 0) {
                printf("registerBuffer hnd=%p\n",(void*)(mDstAdNtBuffer->handle));
                ALOGD("registerBuffer hnd=%p\n",(void*)(mDstAdNtBuffer->handle));
                mgbMapper.registerBuffer(mDstAdNtBuffer->handle);
                mpList.add(mDstAdNtBuffer->handle,maped);
            } else if (mpList.valueFor(mDstAdNtBuffer->handle) != maped) {
                printf("registerBuffer hnd=%p,get map=%d\n",
                                                (void*)(mDstAdNtBuffer->handle),
                                        mpList.valueFor(mDstAdNtBuffer->handle));
                ALOGD("registerBuffer hnd=%p\n",(void*)(mDstAdNtBuffer->handle));
                mgbMapper.registerBuffer(mDstAdNtBuffer->handle);
                mpList.add(mDstAdNtBuffer->handle,maped);
            }
        }

        ret = mRga->rgaCopy(mRga,mSrcAdNtBuffer->handle,mDstAdNtBuffer->handle,NULL);

        if (ret) {
            printf("rgaFillColor error : %s,hnd=%p\n",
                            strerror(errno),(void*)(mDstAdNtBuffer->handle));
            ALOGD("rgaFillColor error : %s,hnd=%p\n",
                            strerror(errno),(void*)(mDstAdNtBuffer->handle));
        }

        {
            char* buf = NULL;
            sp<GraphicBuffer> gb(new GraphicBuffer(mDstAdNtBuffer,false));
            ret = gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);
            int *pBuffer = (int*)buf;
            printf("%x,%x,%x,%x",pBuffer[50],pBuffer[500],pBuffer[5000],pBuffer[50000]);
            ret = gb->unlock();
        }

        ret = mAndoridNativeWindow->queueBuffer(mAndoridNativeWindow.get(),
                                      mDstAdNtBuffer,-1);
        if (ret)
            printf("queue buffer error : %s\n",strerror(errno));
        else {
            KeyedVector<buffer_handle_t,int>& dqList(deQueueList);
            dqList.removeItem(mDstAdNtBuffer->handle);
        }

        printf("threadloop\n");
        usleep(500000);
    }    
}

bool RockchipRgaTest::rgaConvert(bool prepare)
{
    int fenceFd = -1;
    int ret = 0;

    if (prepare) {
        mFormat = HAL_PIXEL_FORMAT_RGBX_8888;
        mWidth = 1200;
        mHeight = 1920;
        return true;
    }

    ANativeWindowBuffer_t* mDstAdNtBuffer;
    sp<ANativeWindow> mAndoridNativeWindow(mFlingerSurface);

    GraphicBufferMapper &mgbMapper = GraphicBufferMapper::get();

    int index = 0;
    drm_rga_t rects;
    rga_set_rect(&rects.src,0,0,1200,1920,1200,HAL_PIXEL_FORMAT_RGBA_8888);
    rga_set_rect(&rects.dst,0,0,1200,1920,1200,mFormat);

    char* buf = NULL;
    sp<GraphicBuffer> gb(new GraphicBuffer(1200,1920,HAL_PIXEL_FORMAT_RGBA_8888,
                                                  GRALLOC_USAGE_SW_WRITE_OFTEN));

    if (gb->initCheck()) {
        printf("GraphicBuffer error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("GraphicBuffer ok : %s\n","*************************************");

    mgbMapper.registerBuffer(gb->handle);
    ret = gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);
    
    if (ret) {
        printf("lock buffer error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("lock buffer ok : %s\n","**************************************");

#if 1
    const char *yuvFilePath = "/data/inBuffer.bin";
    FILE *file = fopen(yuvFilePath, "rb");
    if (!file) {
        fprintf(stderr, "%d Could not open %s\n", __LINE__,yuvFilePath);
        return false;
    }
    fread(buf, 4 * 1920 * 1200, 1, file);
    fclose(file);
#else
    memset(buf,0x55,4*1200*1920);
#endif
    ret = gb->unlock();

    if (ret) {
        printf("unlock buffer error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("unlock buffer ok : %s\n","*************************************");

    while(1) {
        ret = mAndoridNativeWindow->dequeueBuffer(mAndoridNativeWindow.get(),
                                                    &mDstAdNtBuffer,&fenceFd);

        if (fenceFd > -1) {
            sp<Fence> fence = new Fence(fenceFd);
            fence->wait(520);
            printf("fenceFd is not null fenceFd=%d\n",fenceFd);
        }
        
        if (ret)
            printf("dequeue buffer error : %s\n",strerror(errno));
        else {
            ALOGD("init rga ok copy=%p,FillColor=%p\n",mRga->rgaCopy,mRga->rgaFillColor);
            KeyedVector<buffer_handle_t,int>& dqList(deQueueList);
            dqList.add(mDstAdNtBuffer->handle,deQueue);
            KeyedVector<buffer_handle_t,int>& mpList(mappedList);
            if (mpList.indexOfKey(mDstAdNtBuffer->handle) < 0) {
                printf("registerBuffer hnd=%p\n",(void*)(mDstAdNtBuffer->handle));
                ALOGD("registerBuffer hnd=%p\n",(void*)(mDstAdNtBuffer->handle));
                mgbMapper.registerBuffer(mDstAdNtBuffer->handle);
                mpList.add(mDstAdNtBuffer->handle,maped);
            } else if (mpList.valueFor(mDstAdNtBuffer->handle) != maped) {
                printf("registerBuffer hnd=%p,get map=%d\n",
                                                (void*)(mDstAdNtBuffer->handle),
                                        mpList.valueFor(mDstAdNtBuffer->handle));
                ALOGD("registerBuffer hnd=%p\n",(void*)(mDstAdNtBuffer->handle));
                mgbMapper.registerBuffer(mDstAdNtBuffer->handle);
                mpList.add(mDstAdNtBuffer->handle,maped);
            }
        }

        ret = mRga->rgaConvert(mRga,gb->handle,mDstAdNtBuffer->handle,&rects);

        if (ret) {
            printf("rgaFillColor error : %s,hnd=%p\n",
                            strerror(errno),(void*)(mDstAdNtBuffer->handle));
            ALOGD("rgaFillColor error : %s,hnd=%p\n",
                            strerror(errno),(void*)(mDstAdNtBuffer->handle));
        }

        {
            char* buf = NULL;
            sp<GraphicBuffer> gb(new GraphicBuffer(mDstAdNtBuffer,false));
            ret = gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);
#if 1
            const char *outFilePath = "/data/outBuffer.bin";
            FILE *file = fopen(outFilePath, "wb+");
            if (!file) {
                fprintf(stderr, "Could not open %s\n", outFilePath);
                return false;
            }
            fwrite(buf, 4 * 1920 * 1088, 1, file);
            fclose(file);
#endif
            ret = gb->unlock();
        }

        ret = mAndoridNativeWindow->queueBuffer(mAndoridNativeWindow.get(),
                                      mDstAdNtBuffer,-1);
        if (ret)
            printf("queue buffer error : %s\n",strerror(errno));
        else {
            KeyedVector<buffer_handle_t,int>& dqList(deQueueList);
            dqList.removeItem(mDstAdNtBuffer->handle);
        }

        printf("threadloop\n");
        usleep(500000);
    }    
}

bool RockchipRgaTest::rgaScale(bool prepare)
{
    int fenceFd = -1;
    int ret = 0;

    if (prepare) {
        mFormat = HAL_PIXEL_FORMAT_RGBX_8888;
        mWidth = 1200;
        mHeight = 1920;
        return true;
    }

    ANativeWindowBuffer_t* mDstAdNtBuffer;
    sp<ANativeWindow> mAndoridNativeWindow(mFlingerSurface);

    GraphicBufferMapper &mgbMapper = GraphicBufferMapper::get();

    int index = 0;
    drm_rga_t rects;
    rga_set_rect(&rects.src,0,0,1024,600,1024,HAL_PIXEL_FORMAT_RGBA_8888);
    rga_set_rect(&rects.dst,0,0,1200,1920,1200,HAL_PIXEL_FORMAT_RGBA_8888);

    char* buf = NULL;
    sp<GraphicBuffer> gb(new GraphicBuffer(1024,600,HAL_PIXEL_FORMAT_RGBA_8888,
                                                  GRALLOC_USAGE_SW_WRITE_OFTEN));

    if (gb->initCheck()) {
        printf("GraphicBuffer error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("GraphicBuffer ok : %s\n","*************************************");

    mgbMapper.registerBuffer(gb->handle);
    ret = gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);
    
    if (ret) {
        printf("lock buffer error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("lock buffer ok : %s\n","**************************************");

#if 1
    const char *yuvFilePath = "/data/d1024x600.yuv";
    FILE *file = fopen(yuvFilePath, "rb");
    if (!file) {
        fprintf(stderr, "Could not open %s\n", yuvFilePath);
        return false;
    }
    fread(buf, 4 * 1024 * 600, 1, file);
    fclose(file);
#else
    memset(buf,0x55,4*1200*1920);
#endif
    ret = gb->unlock();

    if (ret) {
        printf("unlock buffer error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("unlock buffer ok : %s\n","*************************************");

    while(1) {
        ret = mAndoridNativeWindow->dequeueBuffer(mAndoridNativeWindow.get(),
                                                    &mDstAdNtBuffer,&fenceFd);

        if (fenceFd > -1) {
            sp<Fence> fence = new Fence(fenceFd);
            fence->wait(520);
            printf("fenceFd is not null fenceFd=%d\n",fenceFd);
        }
        
        if (ret)
            printf("dequeue buffer error : %s\n",strerror(errno));
        else {
            ALOGD("init rga ok copy=%p,FillColor=%p\n",mRga->rgaCopy,mRga->rgaFillColor);
            KeyedVector<buffer_handle_t,int>& dqList(deQueueList);
            dqList.add(mDstAdNtBuffer->handle,deQueue);
            KeyedVector<buffer_handle_t,int>& mpList(mappedList);
            if (mpList.indexOfKey(mDstAdNtBuffer->handle) < 0) {
                printf("registerBuffer hnd=%p\n",(void*)(mDstAdNtBuffer->handle));
                ALOGD("registerBuffer hnd=%p\n",(void*)(mDstAdNtBuffer->handle));
                mgbMapper.registerBuffer(mDstAdNtBuffer->handle);
                mpList.add(mDstAdNtBuffer->handle,maped);
            } else if (mpList.valueFor(mDstAdNtBuffer->handle) != maped) {
                printf("registerBuffer hnd=%p,get map=%d\n",
                                                (void*)(mDstAdNtBuffer->handle),
                                        mpList.valueFor(mDstAdNtBuffer->handle));
                ALOGD("registerBuffer hnd=%p\n",(void*)(mDstAdNtBuffer->handle));
                mgbMapper.registerBuffer(mDstAdNtBuffer->handle);
                mpList.add(mDstAdNtBuffer->handle,maped);
            }
        }

        ret = mRga->rgaScale(mRga,gb->handle,mDstAdNtBuffer->handle,&rects);

        if (ret) {
            printf("rgaFillColor error : %s,hnd=%p\n",
                            strerror(errno),(void*)(mDstAdNtBuffer->handle));
            ALOGD("rgaFillColor error : %s,hnd=%p\n",
                            strerror(errno),(void*)(mDstAdNtBuffer->handle));
        }

        {
            char* buf = NULL;
            sp<GraphicBuffer> gb(new GraphicBuffer(mDstAdNtBuffer,false));
            ret = gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);
#if 1
            const char *outFilePath = "/data/outBuffer.bin";
            FILE *file = fopen(outFilePath, "wb+");
            if (!file) {
                fprintf(stderr, "Could not open %s\n", outFilePath);
                return false;
            }
            fwrite(buf, 4 * 1920 * 1088, 1, file);
            fclose(file);
#endif
            ret = gb->unlock();
        }

        ret = mAndoridNativeWindow->queueBuffer(mAndoridNativeWindow.get(),
                                      mDstAdNtBuffer,-1);
        if (ret)
            printf("queue buffer error : %s\n",strerror(errno));
        else {
            KeyedVector<buffer_handle_t,int>& dqList(deQueueList);
            dqList.removeItem(mDstAdNtBuffer->handle);
        }

        printf("threadloop\n");
        usleep(500000);
    }    
}

bool RockchipRgaTest::rgaRotateScale(bool prepare)
{
    int fenceFd = -1;
    int ret = 0;

    if (prepare) {
        mFormat = HAL_PIXEL_FORMAT_RGBX_8888;
        mWidth = 1200;
        mHeight = 1920;
        return true;
    }

    ANativeWindowBuffer_t* mDstAdNtBuffer;
    sp<ANativeWindow> mAndoridNativeWindow(mFlingerSurface);

    GraphicBufferMapper &mgbMapper = GraphicBufferMapper::get();

    int index = 0;
    drm_rga_t rects;
    rga_set_rect(&rects.src,0,0,1200,1920,1200,HAL_PIXEL_FORMAT_RGBA_8888);
    rga_set_rect(&rects.dst,0,0,800,1600,1200,mFormat);

    char* buf = NULL;
    sp<GraphicBuffer> gb(new GraphicBuffer(1200,1920,HAL_PIXEL_FORMAT_RGBA_8888,
                                                  GRALLOC_USAGE_SW_WRITE_OFTEN));

    if (gb->initCheck()) {
        printf("GraphicBuffer error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("GraphicBuffer ok : %s\n","*************************************");

    mgbMapper.registerBuffer(gb->handle);
    ret = gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);
    
    if (ret) {
        printf("lock buffer error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("lock buffer ok : %s\n","**************************************");

#if 1
    const char *yuvFilePath = "/data/inBuffer.bin";
    FILE *file = fopen(yuvFilePath, "rb");
    if (!file) {
        fprintf(stderr, "%d,Could not open %s\n",__LINE__,yuvFilePath);
        return false;
    }
    fread(buf, 4 * 1920 * 1200, 1, file);
    fclose(file);
#else
    memset(buf,0x55,4*1200*1920);
#endif
    ret = gb->unlock();

    if (ret) {
        printf("unlock buffer error : %s\n",strerror(errno));
        return ret;
    } else 
        printf("unlock buffer ok : %s\n","*************************************");

    while(1) {
        ret = mAndoridNativeWindow->dequeueBuffer(mAndoridNativeWindow.get(),
                                                    &mDstAdNtBuffer,&fenceFd);

        if (fenceFd > -1) {
            sp<Fence> fence = new Fence(fenceFd);
            fence->wait(520);
            printf("fenceFd is not null fenceFd=%d\n",fenceFd);
        }
        
        if (ret)
            printf("dequeue buffer error : %s\n",strerror(errno));
        else {
            ALOGD("init rga ok copy=%p,FillColor=%p\n",mRga->rgaCopy,mRga->rgaFillColor);
            KeyedVector<buffer_handle_t,int>& dqList(deQueueList);
            dqList.add(mDstAdNtBuffer->handle,deQueue);
            KeyedVector<buffer_handle_t,int>& mpList(mappedList);
            if (mpList.indexOfKey(mDstAdNtBuffer->handle) < 0) {
                printf("registerBuffer hnd=%p\n",(void*)(mDstAdNtBuffer->handle));
                ALOGD("registerBuffer hnd=%p\n",(void*)(mDstAdNtBuffer->handle));
                mgbMapper.registerBuffer(mDstAdNtBuffer->handle);
                mpList.add(mDstAdNtBuffer->handle,maped);
            } else if (mpList.valueFor(mDstAdNtBuffer->handle) != maped) {
                printf("registerBuffer hnd=%p,get map=%d\n",
                                                (void*)(mDstAdNtBuffer->handle),
                                        mpList.valueFor(mDstAdNtBuffer->handle));
                ALOGD("registerBuffer hnd=%p\n",(void*)(mDstAdNtBuffer->handle));
                mgbMapper.registerBuffer(mDstAdNtBuffer->handle);
                mpList.add(mDstAdNtBuffer->handle,maped);
            }
        }

        ret = mRga->rgaRotateScale(mRga,gb->handle,
                        mDstAdNtBuffer->handle,&rects,DRM_RGA_TRANSFORM_ROT_270 | DRM_RGA_TRANSFORM_FLIP_V);

        if (ret) {
            printf("rgaFillColor error : %s,hnd=%p\n",
                            strerror(errno),(void*)(mDstAdNtBuffer->handle));
            ALOGD("rgaFillColor error : %s,hnd=%p\n",
                            strerror(errno),(void*)(mDstAdNtBuffer->handle));
        }

        {
            char* buf = NULL;
            sp<GraphicBuffer> gb(new GraphicBuffer(mDstAdNtBuffer,false));
            ret = gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)&buf);
#if 1
            const char *outFilePath = "/data/outBuffer.bin";
            FILE *file = fopen(outFilePath, "wb+");
            if (!file) {
                fprintf(stderr, "Could not open %s\n", outFilePath);
                return false;
            }
            fwrite(buf, 4 * 1920 * 1088, 1, file);
            fclose(file);
#endif
            ret = gb->unlock();
        }

        ret = mAndoridNativeWindow->queueBuffer(mAndoridNativeWindow.get(),
                                      mDstAdNtBuffer,-1);
        if (ret)
            printf("queue buffer error : %s\n",strerror(errno));
        else {
            KeyedVector<buffer_handle_t,int>& dqList(deQueueList);
            dqList.removeItem(mDstAdNtBuffer->handle);
        }

        printf("threadloop\n");
        usleep(500000);
    }    
}
void RockchipRgaTest::checkExit() {
    // Allow surface flinger to gracefully request shutdown
    char value[PROPERTY_VALUE_MAX];
    property_get(EXIT_PROP_NAME, value, "0");
    int exitnow = atoi(value);
    if (exitnow) {
        requestExit();
        if (mAudioPlayer != NULL) {
            mAudioPlayer->requestExit();
        }
    }
} 

// ---------------------------------------------------------------------------

}
; // namespace android
