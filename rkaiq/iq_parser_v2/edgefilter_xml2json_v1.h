#ifndef __EDGEFILTER_XML2JSON_V1_H__
#define __EDGEFILTER_XML2JSON_V1_H__

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

int edgefilterV1_calibdb_to_calibdbV2(const CalibDb_EdgeFilter_2_t *pCalibdb,  CalibDbV2_Edgefilter_t *pCalibdbV2, int mode_idx);

int edgefilterV1_calibdbV2_to_calibdb(CalibDbV2_Edgefilter_t *pCalibdbV2, CalibDb_EdgeFilter_2_t *pCalibdb,  int mode_idx);

void edgefilterV1_calibdbV2_free(CalibDbV2_Edgefilter_t *pCalibdbV2);

#endif

