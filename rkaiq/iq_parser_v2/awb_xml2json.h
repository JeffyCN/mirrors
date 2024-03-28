#ifndef __AWB_XML_2JSON_H__
#define __AWB_XML_2JSON_H__

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

void CalibV2AwbV21Free(CamCalibDbV2Context_t *calibV2);
void CalibV2AwbV20Free(CamCalibDbV2Context_t *calibV2);
void convertCalib2calibV21(const CamCalibDbContext_t *calib,CamCalibDbV2Context_t *calibV2);
void convertCalib2calibV20(const CamCalibDbContext_t *calib,CamCalibDbV2Context_t *calibV2);

#endif


