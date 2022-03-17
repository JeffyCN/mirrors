#ifndef __SHARP_XML2JSON_V3_H__
#define __SHARP_XML2JSON_V3_H__

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

int sharpV3_calibdb_to_calibdbV2(struct list_head *pSharpList,   CalibDbV2_SharpV3_t *pCalibdbV2,  int mode_idx);

int sharpV3_calibdbV2_to_calibdb(CalibDbV2_SharpV3_t *pCalibdbV2, struct list_head *pSharpList, int mode_idx);

void sharpV3_calibdbV2_free(CalibDbV2_SharpV3_t *pCalibdbV2);

#endif

