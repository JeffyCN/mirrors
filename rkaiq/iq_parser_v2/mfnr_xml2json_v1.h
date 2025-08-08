#ifndef __MFNR_XML2JSON_V1_H__
#define __MFNR_XML2JSON_V1_H__

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

int mfnrV1_calibdb_to_calibdbV2(const CalibDb_MFNR_2_t *pCalibdb,  CalibDbV2_MFNR_t *pCalibdbV2, int mode_idx);

int mfnrV1_calibdbV2_to_calibdb(CalibDbV2_MFNR_t *pCalibdbV2,  CalibDb_MFNR_2_t *pCalibdb,  int mode_idx);

void mfnrV1_calibdbV2_free(CalibDbV2_MFNR_t *pCalibdbV2);



#endif

