#ifndef __BAYERNR_XML2JSON_V2_H__
#define __BAYERNR_XML2JSON_V2_H__

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

int bayernrV2_calibdb_to_calibdbV2(struct list_head *pBayernrList,  CalibDbV2_BayerNrV2_t *pCalibdbV2, int mode_idx);

int bayernrV2_calibdbV2_to_calibdb(CalibDbV2_BayerNrV2_t *pCalibdbV2,  struct list_head *pBayernrList,  int mode_idx);

void bayernrV2_calibdbV2_free(CalibDbV2_BayerNrV2_t *pCalibdbV2);



#endif

