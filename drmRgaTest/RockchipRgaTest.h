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

#ifndef _rockchip_rga_test_
#define _rockchip_rga_test_

#include <stdint.h>
#include <sys/types.h>

#include <system/window.h>

#include <androidfw/AssetManager.h>
#include <utils/Thread.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

///////////////////////////////////////////////////////
#include "../drmrga.h"
#include <hardware/hardware.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>

#include <sys/mman.h>
#include <linux/stddef.h>
///////////////////////////////////////////////////////

class SkBitmap;

namespace android {

class AudioPlayer;
class Surface;
class SurfaceComposerClient;
class SurfaceControl;

// ---------------------------------------------------------------------------

class RockchipRgaTest : public Thread, public IBinder::DeathRecipient
{
public:
                RockchipRgaTest();
    virtual     ~RockchipRgaTest();

    sp<SurfaceComposerClient> session() const;

private:
    virtual bool        threadLoop();
    virtual status_t    readyToRun();
    virtual void        onFirstRef();
    virtual void        binderDied(const wp<IBinder>& who);


    struct Texture {
        GLint   w;
        GLint   h;
        GLuint  name;
    };

    struct Animation {
        struct Frame {
            String8 name;
            FileMap* map;
            mutable GLuint tid;
            bool operator < (const Frame& rhs) const {
                return name < rhs.name;
            }
        };
        struct Part {
            int count;
            int pause;
            String8 path;
            SortedVector<Frame> frames;
            bool playUntilComplete;
            float backgroundColor[3];
            FileMap* audioFile;
        };
        int fps;
        int width;
        int height;
        Vector<Part> parts;
    };

    bool android();
    bool rgaTest(bool prepare);
    bool rgaFillColorTest(bool prepare);
    bool rgaCopyTest(bool prepare);
    bool rgaConvert(bool prepare);
    bool rgaScale(bool prepare);
    bool rgaRotateScale(bool prepare);
    bool rgaNv12ScaleTest(bool prepare);
    bool readFile(const char* name, String8& outString);
    bool movie();

    void checkExit();

    sp<SurfaceComposerClient>       mSession;
    sp<AudioPlayer>                 mAudioPlayer;
    AssetManager mAssets;
    Texture     mAndroid[2];
    unsigned int  mWidth;
    unsigned int  mHeight;

    sp<SurfaceControl> mFlingerSurfaceControl;
    sp<Surface> mFlingerSurface;
    sp<Surface> mAndroidNaitiveWindow;

    int maped = 0x8002;
    int deQueue = 0x8004;
    rga_device_t* mRga;
    //int mFormat = HAL_PIXEL_FORMAT_YCrCb_NV12;
    int mFormat = HAL_PIXEL_FORMAT_RGB_565;
    hw_module_t const* module;
    KeyedVector<buffer_handle_t,int> deQueueList;
    KeyedVector<buffer_handle_t,int> mappedList;

    ZipFileRO   *mZip;
};

// ---------------------------------------------------------------------------

}; // namespace android

#endif // ANDROID_BOOTANIMATION_H
