 /*
 * Copyright (C) 2016 Rockchip Electronics Co.Ltd
 *                    
 * Authors:
 *	Zhiqin Wei <wzq@rock-chips.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 *Copyright (C) 2014 The Android Open Source Project
 *
 *Licensed under the Apache License, Version 2.0 (the "License");
 *you may not use this file except in compliance with the License.
 *You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *Unless required by applicable law or agreed to in writing, software
 *distributed under the License is distributed on an "AS IS" BASIS,
 *WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *See the License for the specific language governing permissions and
 *limitations under the License.
 */

#ifndef _rockchip_audio_player_
#define _rockchip_audio_player_

#include <utils/Thread.h>
#include <utils/FileMap.h>

namespace android {

class AudioPlayer : public Thread
{
public:
                AudioPlayer();
    virtual     ~AudioPlayer();
    bool        init(const char* config);

    void        playFile(FileMap* fileMap);

private:
    virtual bool        threadLoop();

private:
    int                 mCard;      // ALSA card to use
    int                 mDevice;    // ALSA device to use
    int                 mPeriodSize;
    int                 mPeriodCount;

    FileMap*            mCurrentFile;
};

} // namespace android

#endif // _BOOTANIMATION_AUDIOPLAYER_H
