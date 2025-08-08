#ifndef __YNR_XML2JSON_V2_H__
#define __YNR_XML2JSON_V2_H__

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

int ynrV2_calibdb_to_calibdbV2(struct list_head* pYnrList,  CalibDbV2_YnrV2_t *pCalibdbV2, int mode_idx);

int ynrV2_calibdbV2_to_calibdb(CalibDbV2_YnrV2_t *pCalibdbV2, struct list_head* pYnrList, int mode_idx);

void ynrV2_calibdbV2_free(CalibDbV2_YnrV2_t *pCalibdbV2);

#endif

