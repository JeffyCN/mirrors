#ifndef __CNR_XML2JSON_V1_H__
#define __CNR_XML2JSON_V1_H__

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

int cnrV1_calibdb_to_calibdbV2(struct list_head *pCnrList,  CalibDbV2_CNR_t *pCalibdbV2, int mode_idx);

int cnrV1_calibdbV2_to_calibdb(CalibDbV2_CNR_t *pCalibdbV2,  struct list_head *pCnrList,   int mode_idx);

void cnrV1_calibdbV2_free(CalibDbV2_CNR_t *pCalibdbV2);

#endif

