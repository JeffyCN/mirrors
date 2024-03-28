#ifndef __AF_XML_2JSON_H__
#define __AF_XML_2JSON_H__

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

void CalibV2AfFree(CamCalibDbV2Context_t *calibV2);
void convertAfCalibV1ToCalibV2(const CamCalibDbContext_t *calib, CamCalibDbV2Context_t *calibV2);

#endif
