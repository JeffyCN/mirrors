#ifndef __UVNR_XML2JSON_V1_H__
#define __UVNR_XML2JSON_V1_H__

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

int uvnrV1_calibdb_to_calibdbV2(const CalibDb_UVNR_2_t *pCalibdb,  CalibDbV2_UVNR_t *pCalibdbV2, int mode_idx);

int uvnrV1_calibdbV2_to_calibdb(CalibDbV2_UVNR_t *pCalibdbV2,  CalibDb_UVNR_2_t *pCalibdb,   int mode_idx);

void uvnrV1_calibdbV2_free(CalibDbV2_UVNR_t *pCalibdbV2);

#endif

