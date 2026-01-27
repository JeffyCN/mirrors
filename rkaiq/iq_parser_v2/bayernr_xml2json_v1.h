#ifndef __BAYERNR_XML2JSON_V1_H__
#define __BAYERNR_XML2JSON_V1_H__

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

int bayernrV1_calibdb_to_calibdbV2(const CalibDb_BayerNr_2_t *pCalibdb,  CalibDbV2_BayerNrV1_t *pCalibdbV2, int mode_idx);

int bayernrV1_calibdbV2_to_calibdb(CalibDbV2_BayerNrV1_t *pCalibdbV2,  CalibDb_BayerNr_2_t *pCalibdb,  int mode_idx);

void bayernrV1_calibdbV2_free(CalibDbV2_BayerNrV1_t *pCalibdbV2);



#endif

