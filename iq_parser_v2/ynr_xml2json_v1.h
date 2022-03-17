#ifndef __YNR_XML2JSON_V1_H__
#define __YNR_XML2JSON_V1_H__

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

int ynrV1_calibdb_to_calibdbV2(const CalibDb_YNR_2_t *pCalibdb,  CalibDbV2_YnrV1_t *pCalibdbV2, int mode_idx);

int ynrV1_calibdbV2_to_calibdb(CalibDbV2_YnrV1_t *pCalibdbV2, CalibDb_YNR_2_t *pCalibdb, int mode_idx);

void ynrV1_calibdbV2_free(CalibDbV2_YnrV1_t *pCalibdbV2);

#endif

