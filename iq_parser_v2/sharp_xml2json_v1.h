#ifndef __SHARP_XML2JSON_V1_H__
#define __SHARP_XML2JSON_V1_H__

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

int sharpV1_calibdb_to_calibdbV2(const CalibDb_Sharp_2_t *pCalibdb,   CalibDbV2_SharpV1_t *pCalibdbV2,  int mode_idx);

int sharpV1_calibdbV2_to_calibdb(CalibDbV2_SharpV1_t *pCalibdbV2, CalibDb_Sharp_2_t *pCalibdb, int mode_idx);

void sharpV1_calibdbV2_free(CalibDbV2_SharpV1_t *pCalibdbV2);

#endif

