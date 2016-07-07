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

#define LOG_TAG "RockchipRgaTest"

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <cutils/properties.h>
#include <sys/resource.h>
#include <utils/Log.h>
#include <utils/threads.h>

#include "RockchipRgaTest.h"

using namespace android;

// ---------------------------------------------------------------------------

int main()
{
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_DISPLAY);

    char value[PROPERTY_VALUE_MAX];
    property_get("debug.sf.RockchipRgaTest", value, "0");
    int noRockchipRgaTest = atoi(value);
    ALOGI_IF(noRockchipRgaTest,  "boot animation disabled");
    if (!noRockchipRgaTest) {

        sp<ProcessState> proc(ProcessState::self());
        ProcessState::self()->startThreadPool();

        // create the boot animation object
        sp<RockchipRgaTest> boot = new RockchipRgaTest();

        IPCThreadState::self()->joinThreadPool();

    }
    return 0;
}
