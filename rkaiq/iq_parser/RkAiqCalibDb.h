/*
 *  Copyright (c) 2019 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef _RK_AIQ_CALIB_H_
#define _RK_AIQ_CALIB_H_

#include "RkAiqCalibParser.h"
#include "RkAiqCalibDbTypes.h"
#include "xcam_mutex.h"
#include <map>
#include <string>

namespace RkCam {

using namespace std;

class RkAiqCalibDb
{
public:
    static CamCalibDbContext_t* createCalibDb(char* iqFile);
    static bool generateCalibDb(char* iqFileRef, char* iqFileOutput, CamCalibDbContext_t* pCalibDb);
    static void releaseCalibDb();
    static CamCalibDbContext_t* getCalibDb(char* iqFile);
    static void createCalibDbBinFromXml(char* iqFile);
    static void parseXmlandWriteXml(char* iqFile);
private:
    static void initCalibDb(CamCalibDbContext_t* pCalibDb);
    static map<string, CamCalibDbContext_t*> mCalibDbsMap;
    static XCam::Mutex mMutex;
};

} //namespace RkCam

#endif
