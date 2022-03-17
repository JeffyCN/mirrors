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

#ifndef __IQCONVERTER_H__
#define __IQCONVERTER_H__

#include "RkAiqCalibDb.h"
#include "RkAiqCalibDbV2.h"

#include "aec_xml2json.h"
#include "awb_xml2json.h"
#include "bayernr_xml2json_v1.h"
#include "mfnr_xml2json_v1.h"
#include "uvnr_xml2json_v1.h"
#include "ynr_xml2json_v1.h"
#include "sharp_xml2json_v1.h"
#include "edgefilter_xml2json_v1.h"
#include "ccm_xml2json.h"
#include "adrc_xml2json.h"
#include "af_xml2json.h"
#include "adehaze_xml2json.h"

#include "bayernr_xml2json_v2.h"
#include "cnr_xml2json_v1.h"
#include "ynr_xml2json_v2.h"
#include "sharp_xml2json_v3.h"
#include "lut3d_xml2json.h"



#define DEF_NEW_CONVERTER(module)                                              \
  class CalibConverter##module : public CalibConverter {                       \
  public:                                                                      \
    CalibConverter##module() { printf("[%s] processing...\n", #module); }      \
    virtual ~CalibConverter##module() {};                                       \
    virtual void convert(CamCalibDbV2Context_t *calibv2,                       \
                         CamCalibDbContext_t *calibv1);                        \
  }

/*
 * 1. create object
 * 2. call object->convert(newcalib, oldcalib)
 * 3. delete object
 * */
#define ADD_NEW_CONVERTER(module)                                              \
  CalibConverter##module *module##_converter = new CalibConverter##module();   \
  module##_converter->convert(calibv2, calibv1);                               \
  delete module##_converter;

namespace RkCam {

class CalibConverter;

class CalibConverter {
public:
    CalibConverter() = default;
    virtual ~CalibConverter() = default;

public:
    virtual void convert(CamCalibDbV2Context_t *calibv2,
                         CamCalibDbContext_t *calibv1) = 0;
};

// define you module here
DEF_NEW_CONVERTER(AE);
DEF_NEW_CONVERTER(AWB);
DEF_NEW_CONVERTER(Ablc);
DEF_NEW_CONVERTER(Adegamma);
DEF_NEW_CONVERTER(Agic);
DEF_NEW_CONVERTER(Adehaze);
DEF_NEW_CONVERTER(Adpcc);
DEF_NEW_CONVERTER(Amerge);
DEF_NEW_CONVERTER(Atmo);
DEF_NEW_CONVERTER(Agamma);
DEF_NEW_CONVERTER(Cpsl);
DEF_NEW_CONVERTER(BAYERNRV1);
DEF_NEW_CONVERTER(MFNRV1);
DEF_NEW_CONVERTER(UVNRV1);
DEF_NEW_CONVERTER(YNRV1);
DEF_NEW_CONVERTER(SHARPV1);
DEF_NEW_CONVERTER(EDGEFILTERV1);
DEF_NEW_CONVERTER(Debayer);
DEF_NEW_CONVERTER(Cproc);
DEF_NEW_CONVERTER(IE);
DEF_NEW_CONVERTER(ALSC);
DEF_NEW_CONVERTER(Aldch);
DEF_NEW_CONVERTER(Afec);
DEF_NEW_CONVERTER(LumaDetect);
DEF_NEW_CONVERTER(ColorAsGrey);
DEF_NEW_CONVERTER(CCM);
DEF_NEW_CONVERTER(LUT3D);
DEF_NEW_CONVERTER(Af);
DEF_NEW_CONVERTER(Thumbnails);
DEF_NEW_CONVERTER(BAYERNRV2);
DEF_NEW_CONVERTER(CNRV1);
DEF_NEW_CONVERTER(YNRV2);
DEF_NEW_CONVERTER(SHARPV3);
DEF_NEW_CONVERTER(Adrc);


class IQConverter {
public:
    explicit IQConverter(const char *xml, const char *json);
    ~IQConverter() = default;

public:
    int convert();

private:
    std::string ifile;
    std::string ofile;
    CamCalibDbContext_t *calibv1;
    CamCalibDbV2Context_t *calibv2;
    CamCalibDbProj_t  calibproj;

    static int addToScene(CamCalibDbProj_t* calibpj, const char* main_scene,
                          const char* sub_scene, CamCalibDbV2Context_t *calibv2);

    void doConvert() {
        // add you converter here
        ADD_NEW_CONVERTER(AE);
        ADD_NEW_CONVERTER(AWB);
        ADD_NEW_CONVERTER(Ablc);
        ADD_NEW_CONVERTER(Adegamma);
        ADD_NEW_CONVERTER(Agic);
        ADD_NEW_CONVERTER(Adehaze);
        ADD_NEW_CONVERTER(Adpcc);
        ADD_NEW_CONVERTER(Amerge);
        ADD_NEW_CONVERTER(Atmo);
        ADD_NEW_CONVERTER(Agamma);
        ADD_NEW_CONVERTER(Cpsl);
        ADD_NEW_CONVERTER(BAYERNRV1);
        ADD_NEW_CONVERTER(MFNRV1);
        ADD_NEW_CONVERTER(UVNRV1);
        ADD_NEW_CONVERTER(YNRV1);
        ADD_NEW_CONVERTER(SHARPV1);
        ADD_NEW_CONVERTER(EDGEFILTERV1);
        ADD_NEW_CONVERTER(Debayer);
        ADD_NEW_CONVERTER(Cproc);
        ADD_NEW_CONVERTER(IE);
        ADD_NEW_CONVERTER(ALSC);
        ADD_NEW_CONVERTER(Afec);
        ADD_NEW_CONVERTER(Aldch);
        ADD_NEW_CONVERTER(LumaDetect);
        ADD_NEW_CONVERTER(ColorAsGrey);
        ADD_NEW_CONVERTER(CCM);
        ADD_NEW_CONVERTER(LUT3D);
        ADD_NEW_CONVERTER(Af);
        ADD_NEW_CONVERTER(Thumbnails);
        ADD_NEW_CONVERTER(BAYERNRV2);
        ADD_NEW_CONVERTER(CNRV1);
        ADD_NEW_CONVERTER(YNRV2);
        ADD_NEW_CONVERTER(SHARPV3);
        ADD_NEW_CONVERTER(Adrc);
    };
};

} // namespace RkCam

#endif /*__IQCONVERTER_H__*/
