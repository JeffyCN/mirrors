/******************************************************************************
 *
 * Copyright 2016, Fuzhou Rockchip Electronics Co.Ltd . All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 * 
 *
 *****************************************************************************/
/**
 * @file cam_engine_api.c
 *
 * @brief
 *   Implementation of the CamEngine API.
 *
 *****************************************************************************/
//#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <ebase/dct_assert.h>
#include <base/log.h>

#include "cam_calibdb_api.h"
#include "cam_calibdb.h"
#include <stdlib.h>
#include <string.h>

/******************************************************************************
 * local macro definitions
 *****************************************************************************/


/******************************************************************************
 * local type definitions
 *****************************************************************************/


/******************************************************************************
 * local variable declarations
 *****************************************************************************/


/******************************************************************************
 * local function prototypes
 *****************************************************************************/


/******************************************************************************
 * local functions
 *****************************************************************************/


/******************************************************************************
 * SearchForEqualFrameRate
 *****************************************************************************/
static int SearchForEqualFrameRate(List* l, void* key) {
  CamFrameRate_t* res = (CamFrameRate_t*)l;
  CamFrameRate_t* k   = (CamFrameRate_t*)key;

  return (((res->fps == k->fps) || (!strncmp(res->name, k->name, sizeof(k->name)))) ? 1 : 0);
}


/******************************************************************************
 * SearchForEqualResolution
 *****************************************************************************/
static int SearchForEqualResolution(List* l, void* key) {
  CamResolution_t* res = (CamResolution_t*)l;
  CamResolution_t* k = (CamResolution_t*)key;

  return (((res->id == k->id) || (!strncmp(res->name, k->name, sizeof(k->name)))) ? 1 : 0);
}



/******************************************************************************
 * SearchResolutionByName
 *****************************************************************************/
static int SearchResolutionByName(List* l, void* key) {
  CamResolution_t* res = (CamResolution_t*)l;
  char* k = (char*)key;

  return ((!strncmp(res->name, k, strlen(res->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchResolutionByWidthHeight
 *****************************************************************************/
static int SearchResolutionByWidthHeight(List* l, void* key) {
  CamResolution_t* search = (CamResolution_t*)key;
  CamResolution_t* res = (CamResolution_t*)l;

  return (((search->width == res->width) && (search->height == res->height)) ? 1 : 0);
}



/******************************************************************************
 * SearchForEqualAwbGlobal
 *****************************************************************************/
static int SearchForEqualAwbGlobal(List* l, void* key) {
  CamCalibAwbGlobal_t* awb = (CamCalibAwbGlobal_t*)l;
  CamCalibAwbGlobal_t* k = (CamCalibAwbGlobal_t*)key;

  return ((!strncmp(awb->name, k->name, sizeof(k->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchAwbGlobalByName
 *****************************************************************************/
static int SearchAwbGlobalByName(List* l, void* key) {
  CamCalibAwbGlobal_t* awb = (CamCalibAwbGlobal_t*)l;
  char* k = (char*)key;

  return ((!strncmp(awb->name, k, sizeof(awb->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchAwbGlobalByResolution
 *****************************************************************************/
static int SearchAwbGlobalByResolution(List* l, void* key) {
  CamCalibAwbGlobal_t* awb = (CamCalibAwbGlobal_t*)l;
  char* k = (char*)key;

  return ((!strncmp(awb->resolution, k, sizeof(awb->resolution))) ? 1 : 0);
}



/******************************************************************************
 * SearchForEqualEcmProfile
 *****************************************************************************/
static int SearchForEqualEcmProfile(List* l, void* key) {
  CamEcmProfile_t* ecm = (CamEcmProfile_t*)l;
  CamEcmProfile_t* k   = (CamEcmProfile_t*)key;

  return ((!strncmp(ecm->name, k->name, sizeof(k->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchEcmProfileByName
 *****************************************************************************/
static int SearchEcmProfileByName(List* l, void* key) {
  CamEcmProfile_t* ecm = (CamEcmProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(ecm->name, k, sizeof(ecm->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchForEqualEcmScheme
 *****************************************************************************/
static int SearchForEqualEcmScheme(List* l, void* key) {
  CamEcmScheme_t* ecm = (CamEcmScheme_t*)l;
  CamEcmScheme_t* k   = (CamEcmScheme_t*)key;

  return ((!strncmp(ecm->name, k->name, sizeof(k->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchEcmSchemeByName
 *****************************************************************************/
static int SearchEcmSchemeByName(List* l, void* key) {
  CamEcmScheme_t* ecm = (CamEcmScheme_t*)l;
  char* k = (char*)key;

  return ((!strncmp(ecm->name, k, sizeof(ecm->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchForEqualIllumination
 *****************************************************************************/
static int SearchForEqualIllumination(List* l, void* key) {
  CamIlluProfile_t* illu = (CamIlluProfile_t*)l;
  CamIlluProfile_t* k = (CamIlluProfile_t*)key;

  return ((!strncmp(illu->name, k->name, sizeof(k->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchIlluminationByName
 *****************************************************************************/
static int SearchIlluminationByName(List* l, void* key) {
  CamIlluProfile_t* illu = (CamIlluProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(illu->name, k, sizeof(illu->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchForEqualLscProfile
 *****************************************************************************/
static int SearchForEqualLscProfile(List* l, void* key) {
  CamLscProfile_t* lsc = (CamLscProfile_t*)l;
  CamLscProfile_t* k = (CamLscProfile_t*)key;

  return ((!strncmp(lsc->name, k->name, sizeof(k->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchLscProfileByName
 *****************************************************************************/
static int SearchLscProfileByName(List* l, void* key) {
  CamLscProfile_t* lsc = (CamLscProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(lsc->name, k, sizeof(lsc->name))) ? 1 : 0);
}


/******************************************************************************
 * SearchForEqualCcProfile
 *****************************************************************************/
static int SearchForEqualCcProfile(List* l, void* key) {
  CamCcProfile_t* cc = (CamCcProfile_t*)l;
  CamCcProfile_t* k = (CamCcProfile_t*)key;

  return ((!strncmp(cc->name, k->name, sizeof(k->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchCcProfileByName
 *****************************************************************************/
static int SearchCcProfileByName(List* l, void* key) {
  CamCcProfile_t* cc = (CamCcProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(cc->name, k, sizeof(cc->name))) ? 1 : 0);
}


/******************************************************************************
 * SearchForEqualBlsProfile
 *****************************************************************************/
static int SearchForEqualBlsProfile(List* l, void* key) {
  CamBlsProfile_t* bls = (CamBlsProfile_t*)l;
  CamBlsProfile_t* k = (CamBlsProfile_t*)key;

  return ((!strncmp(bls->name, k->name, sizeof(k->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchBlsProfileByName
 *****************************************************************************/
static int SearchBlsProfileByName(List* l, void* key) {
  CamBlsProfile_t* bls = (CamBlsProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(bls->name, k, sizeof(bls->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchBlsProfileByResolution
 *****************************************************************************/
static int SearchBlsProfileByResolution(List* l, void* key) {
  CamBlsProfile_t* bls = (CamBlsProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(bls->resolution, k, sizeof(bls->resolution))) ? 1 : 0);
}



/******************************************************************************
 * SearchForEqualCacProfile
 *****************************************************************************/
static int SearchForEqualCacProfile(List* l, void* key) {
  CamCacProfile_t* cac = (CamCacProfile_t*)l;
  CamCacProfile_t* k = (CamCacProfile_t*)key;

  return ((!strncmp(cac->name, k->name, sizeof(k->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchCacProfileByName
 *****************************************************************************/
static int SearchCacProfileByName(List* l, void* key) {
  CamCacProfile_t* cac = (CamCacProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(cac->name, k, sizeof(cac->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchCacProfileByResolution
 *****************************************************************************/
static int SearchCacProfileByResolution(List* l, void* key) {
  CamCacProfile_t* cac = (CamCacProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(cac->resolution, k, sizeof(cac->resolution))) ? 1 : 0);
}



/******************************************************************************
 * SearchForEqualDpfProfile
 *****************************************************************************/
static int SearchForEqualDpfProfile(List* l, void* key) {
  CamDpfProfile_t* dpf = (CamDpfProfile_t*)l;
  CamDpfProfile_t* k = (CamDpfProfile_t*)key;

  return ((!strncmp(dpf->name, k->name, sizeof(k->name))) ? 1 : 0);
}


/******************************************************************************
 * SearchForEqualDpccProfile
 *****************************************************************************/
static int SearchForEqualDpccProfile(List* l, void* key) {
  CamDpccProfile_t* dpcc = (CamDpccProfile_t*)l;
  CamDpccProfile_t* k = (CamDpccProfile_t*)key;

  return ((!strncmp(dpcc->name, k->name, sizeof(k->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchDpfProfileByName
 *****************************************************************************/
static int SearchDpfProfileByName(List* l, void* key) {
  CamDpfProfile_t* dpf = (CamDpfProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(dpf->name, k, sizeof(dpf->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchDpfProfileByResolution
 *****************************************************************************/
static int SearchDpfProfileByResolution(List* l, void* key) {
  CamDpfProfile_t* dpf = (CamDpfProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(dpf->resolution, k, sizeof(dpf->resolution))) ? 1 : 0);
}

/******************************************************************************
 * SearchForEqualEcmScheme
 *****************************************************************************/
static int SearchForEqualDsp3DNRSetting(List* l, void* key) {
  CamDsp3DNRSettingProfile_t* ecm = (CamDsp3DNRSettingProfile_t*)l;
  CamDsp3DNRSettingProfile_t* k   = (CamDsp3DNRSettingProfile_t*)key;

  return ((!strncmp(ecm->name, k->name, sizeof(k->name))) ? 1 : 0);
}

static int SearchDsp3DNRSettingByName(List* l, void* key) {
  CamDsp3DNRSettingProfile_t* dsp3DNR = (CamDsp3DNRSettingProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(dsp3DNR->name, k, sizeof(dsp3DNR->name))) ? 1 : 0);
}

static int SearchForEqualFilterProfile(List* l, void* key) {
  CamFilterProfile_t* ecm = (CamFilterProfile_t*)l;
  CamFilterProfile_t* k   = (CamFilterProfile_t*)key;

  return ((!strncmp(ecm->name, k->name, sizeof(k->name))) ? 1 : 0);
}

static int SearchFilterProfileByName(List* l, void* key) {
  CamFilterProfile_t* filter = (CamFilterProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(filter->name, k, sizeof(filter->name))) ? 1 : 0);
}


/******************************************************************************
 * SearchDpccProfileByName
 *****************************************************************************/
static int SearchDpccProfileByName(List* l, void* key) {
  CamDpccProfile_t* dpcc = (CamDpccProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(dpcc->name, k, sizeof(dpcc->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchDpccProfileByResolution
 *****************************************************************************/
static int SearchDpccProfileByResolution(List* l, void* key) {
  CamDpccProfile_t* dpcc = (CamDpccProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(dpcc->resolution, k, sizeof(dpcc->resolution))) ? 1 : 0);
}

static int SearchForEqualGocProfile(List* l, void* key) {
  CamCalibGocProfile_t* dpcc = (CamCalibGocProfile_t*)l;
  CamCalibGocProfile_t* k = (CamCalibGocProfile_t*)key;

  return ((!strncmp(dpcc->name, k->name, sizeof(k->name))) ? 1 : 0);
}

static int SearchGocProfileByName(List* l, void* key) {
  CamCalibGocProfile_t* dpf = (CamCalibGocProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(dpf->name, k, sizeof(dpf->name))) ? 1 : 0);
}


/******************************************************************************
 * ClearFrameRateList
 *****************************************************************************/
static void ClearFrameRateList(List* l) {
  if (!ListEmpty(l)) {
    CamFrameRate_t* pFrameRate = (CamFrameRate_t*)ListRemoveHead(l);
    while (pFrameRate) {
      /* 1.) free sub structures of item */
      /* nothing to free */

      /* 2.) free item */
      free(pFrameRate);

      /* 3.) get next item */
      pFrameRate = (CamFrameRate_t*)ListRemoveHead(l);
    }
  }

  ListInit(l);
}


/******************************************************************************
 * ClearResolutionList
 *****************************************************************************/
static void ClearResolutionList(List* l) {
  if (!ListEmpty(l)) {
    CamResolution_t* pResolution = (CamResolution_t*)ListRemoveHead(l);
    while (pResolution) {
      /* 1.) free sub structures of item */
      ClearFrameRateList(&pResolution->framerates);

      /* 2.) free item */
      free(pResolution);

      /* 3.) get next item */
      pResolution = (CamResolution_t*)ListRemoveHead(l);
    }
  }

  ListInit(l);
}



/******************************************************************************
 * ClearAwbGlobalList
 *****************************************************************************/
static void ClearAwbGlobalList(List* l) {
  if (!ListEmpty(l)) {
    CamCalibAwbGlobal_t* pAwbGlobal = (CamCalibAwbGlobal_t*)ListRemoveHead(l);
    while (pAwbGlobal) {
      /* 1.) free sub structures of AWB globals */
      free(pAwbGlobal->AwbClipParam.pRg1);
      free(pAwbGlobal->AwbClipParam.pMaxDist1);
      free(pAwbGlobal->AwbClipParam.pRg2);
      free(pAwbGlobal->AwbClipParam.pMaxDist2);

      free(pAwbGlobal->AwbGlobalFadeParm.pGlobalFade1);
      free(pAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance1);
      free(pAwbGlobal->AwbGlobalFadeParm.pGlobalFade2);
      free(pAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance2);

      free(pAwbGlobal->AwbFade2Parm.pFade);
      free(pAwbGlobal->AwbFade2Parm.pCbMinRegionMax);
      free(pAwbGlobal->AwbFade2Parm.pCrMinRegionMax);
      free(pAwbGlobal->AwbFade2Parm.pMaxCSumRegionMax);
      free(pAwbGlobal->AwbFade2Parm.pCbMinRegionMin);
      free(pAwbGlobal->AwbFade2Parm.pCrMinRegionMin);
      free(pAwbGlobal->AwbFade2Parm.pMaxCSumRegionMin);

      free(pAwbGlobal->AwbFade2Parm.pMinCRegionMax);
      free(pAwbGlobal->AwbFade2Parm.pMinCRegionMin);
      free(pAwbGlobal->AwbFade2Parm.pMaxYRegionMax);
      free(pAwbGlobal->AwbFade2Parm.pMaxYRegionMin);
      free(pAwbGlobal->AwbFade2Parm.pMinYMaxGRegionMax);
      free(pAwbGlobal->AwbFade2Parm.pMinYMaxGRegionMin);
      free(pAwbGlobal->AwbFade2Parm.pRefCb);
      free(pAwbGlobal->AwbFade2Parm.pRefCr);


      /* 2.) free AWB globals */
      free(pAwbGlobal);

      /* 3.) get next illumination */
      pAwbGlobal = (CamCalibAwbGlobal_t*)ListRemoveHead(l);
    }
  }

  ListInit(l);
}



/******************************************************************************
 * ClearEcmSchemeList
 *****************************************************************************/
static void ClearEcmSchemeList(List* l) {
  if (!ListEmpty(l)) {
    CamEcmScheme_t* pEcmScheme = (CamEcmScheme_t*)ListRemoveHead(l);
    while (pEcmScheme) {
      /* 1.) free sub structures of item */
      /* nothing to free */

      /* 2.) free item */
      free(pEcmScheme);

      /* 3.) get next item */
      pEcmScheme = (CamEcmScheme_t*)ListRemoveHead(l);
    }
  }

  ListInit(l);
}



/******************************************************************************
 * ClearEcmProfileList
 *****************************************************************************/
static void ClearEcmProfileList(List* l) {
  if (!ListEmpty(l)) {
    CamEcmProfile_t* pEcmProfile = (CamEcmProfile_t*)ListRemoveHead(l);
    while (pEcmProfile) {
      /* 1.) free sub structures of item */
      ClearEcmSchemeList(&pEcmProfile->ecm_scheme);

      /* 2.) free item */
      free(pEcmProfile);

      /* 3.) get next item */
      pEcmProfile = (CamEcmProfile_t*)ListRemoveHead(l);
    }
  }

  ListInit(l);
}



/******************************************************************************
 * ClearIlluminationList
 *****************************************************************************/
static void ClearIlluminationList(List* l) {
  if (!ListEmpty(l)) {
    CamIlluProfile_t* pIllumination = (CamIlluProfile_t*)ListRemoveHead(l);
    while (pIllumination) {
      /* 1.) free sub structures of illumination */
      free(pIllumination->SaturationCurve.pSensorGain);
      free(pIllumination->SaturationCurve.pSaturation);

      free(pIllumination->VignettingCurve.pSensorGain);
      free(pIllumination->VignettingCurve.pVignetting);

      /* 2.) free illumination */
      free(pIllumination);

      /* 3.) get next illumination */
      pIllumination = (CamIlluProfile_t*)ListRemoveHead(l);
    }
  }

  ListInit(l);
}



/******************************************************************************
 * ClearLscProfileList
 *****************************************************************************/
static void ClearLscProfileList(List* l) {
  if (!ListEmpty(l)) {
    CamLscProfile_t* pLscProfile = (CamLscProfile_t*)ListRemoveHead(l);
    while (pLscProfile) {
      free(pLscProfile);
      pLscProfile = (CamLscProfile_t*)ListRemoveHead(l);
    }
  }

  ListInit(l);
}



/******************************************************************************
 * ClearCcProfileList
 *****************************************************************************/
static void ClearCcProfileList(List* l) {
  if (!ListEmpty(l)) {
    CamCcProfile_t* pCcProfile = (CamCcProfile_t*)ListRemoveHead(l);
    while (pCcProfile) {
      free(pCcProfile);
      pCcProfile = (CamCcProfile_t*)ListRemoveHead(l);
    }
  }

  ListInit(l);
}



/******************************************************************************
 * ClearBlsProfileList
 *****************************************************************************/
static void ClearBlsProfileList(List* l) {
  if (!ListEmpty(l)) {
    CamBlsProfile_t* pBlsProfile = (CamBlsProfile_t*)ListRemoveHead(l);
    while (pBlsProfile) {
      free(pBlsProfile);
      pBlsProfile = (CamBlsProfile_t*)ListRemoveHead(l);
    }
  }

  ListInit(l);
}




/******************************************************************************
 * ClearCacProfileList
 *****************************************************************************/
static void ClearCacProfileList(List* l) {
  if (!ListEmpty(l)) {
    CamCacProfile_t* pCacProfile = (CamCacProfile_t*)ListRemoveHead(l);
    while (pCacProfile) {
      free(pCacProfile);
      pCacProfile = (CamCacProfile_t*)ListRemoveHead(l);
    }
  }

  ListInit(l);
}


/******************************************************************************
 * ClearDsp3DNRList
 *****************************************************************************/
static void ClearDsp3DNRList(List* l) {
  if (!ListEmpty(l)) {
    CamDsp3DNRSettingProfile_t * pDsp3DNR = (CamDsp3DNRSettingProfile_t*)ListRemoveHead(l);
    while (pDsp3DNR) {
      /* 1.) free sub structures of item */
      /* nothing to free */

      /* 2.) free item */
	  if(pDsp3DNR->pgain_Level){
		free(pDsp3DNR->pgain_Level);
  	  }
	  if(pDsp3DNR->pnoise_coef_denominator){
		free(pDsp3DNR->pnoise_coef_denominator);
  	  }
	  if(pDsp3DNR->pnoise_coef_numerator){
		free(pDsp3DNR->pnoise_coef_numerator);
  	  }
	  if(pDsp3DNR->sDefaultLevelSetting.pchrm_sp_nr_level){
		free(pDsp3DNR->sDefaultLevelSetting.pchrm_sp_nr_level);
  	  }
	  if(pDsp3DNR->sDefaultLevelSetting.pchrm_te_nr_level){
		free(pDsp3DNR->sDefaultLevelSetting.pchrm_te_nr_level);
  	  }
	  if(pDsp3DNR->sDefaultLevelSetting.pluma_sp_nr_level){
		free(pDsp3DNR->sDefaultLevelSetting.pluma_sp_nr_level);
  	  }
	  if(pDsp3DNR->sDefaultLevelSetting.pluma_te_nr_level){
		free(pDsp3DNR->sDefaultLevelSetting.pluma_te_nr_level);
  	  }
	  if(pDsp3DNR->sDefaultLevelSetting.pshp_level){
		free(pDsp3DNR->sDefaultLevelSetting.pshp_level);
  	  }
	  
	  if(pDsp3DNR->sLumaSetting.pluma_sp_rad){
		free(pDsp3DNR->sLumaSetting.pluma_sp_rad);
  	  }
	  if(pDsp3DNR->sLumaSetting.pluma_te_max_bi_num){
		free(pDsp3DNR->sLumaSetting.pluma_te_max_bi_num);
  	  }
	  

	  if(pDsp3DNR->sChrmSetting.pchrm_sp_rad){
		free(pDsp3DNR->sChrmSetting.pchrm_sp_rad);
  	  }
	  if(pDsp3DNR->sChrmSetting.pchrm_te_max_bi_num){
		free(pDsp3DNR->sChrmSetting.pchrm_te_max_bi_num);
  	  }
	 
  
	  if(pDsp3DNR->sSharpSetting.psrc_shp_c){
		free(pDsp3DNR->sSharpSetting.psrc_shp_c);
  	  }
	  if(pDsp3DNR->sSharpSetting.psrc_shp_div){
		free(pDsp3DNR->sSharpSetting.psrc_shp_div);
  	  }
	  if(pDsp3DNR->sSharpSetting.psrc_shp_l){
		free(pDsp3DNR->sSharpSetting.psrc_shp_l);
  	  }
	  if(pDsp3DNR->sSharpSetting.psrc_shp_thr){
		free(pDsp3DNR->sSharpSetting.psrc_shp_thr);
  	  }
	

	  for(int i=0; i<CAM_CALIBDB_3DNR_WEIGHT_NUM; i++){
		if(pDsp3DNR->sLumaSetting.pluma_weight[i]){
			free(pDsp3DNR->sLumaSetting.pluma_weight[i]);
		}

		if(pDsp3DNR->sChrmSetting.pchrm_weight[i]){
		  	free(pDsp3DNR->sChrmSetting.pchrm_weight[i]);
  	    }
		
		if(pDsp3DNR->sSharpSetting.psrc_shp_weight[i]){
		   free(pDsp3DNR->sSharpSetting.psrc_shp_weight[i]);
		}
	  }
      free(pDsp3DNR);

      /* 3.) get next item */
      pDsp3DNR = (CamDsp3DNRSettingProfile_t*)ListRemoveHead(l);
    }
  }

  ListInit(l);
}



/******************************************************************************
 * ClearDsp3DNRList
 *****************************************************************************/
static void ClearFilterList(List* l) {
  if (!ListEmpty(l)) {
    CamFilterProfile_t * pFilter = (CamFilterProfile_t*)ListRemoveHead(l);
    while (pFilter) {
      /* 1.) free sub structures of item */
      /* nothing to free */

      /* 2.) free item */
	  if(pFilter->DemosaicThCurve.pSensorGain){
		free(pFilter->DemosaicThCurve.pSensorGain);
  	  }
	  if(pFilter->DemosaicThCurve.pThlevel){
		free(pFilter->DemosaicThCurve.pThlevel);
  	  }
	   
	  if(pFilter->DenoiseLevelCurve.pSensorGain){
		free(pFilter->DenoiseLevelCurve.pSensorGain);
  	  }
	  if(pFilter->DenoiseLevelCurve.pDlevel){
		free(pFilter->DenoiseLevelCurve.pDlevel);
  	  }

	  if(pFilter->SharpeningLevelCurve.pSensorGain){
		free(pFilter->SharpeningLevelCurve.pSensorGain);
  	  }
	  if(pFilter->SharpeningLevelCurve.pSlevel){
		free(pFilter->SharpeningLevelCurve.pSlevel);
  	  }

	  if(pFilter->FiltLevelRegConf.p_chr_h_mode){
		free(pFilter->FiltLevelRegConf.p_chr_h_mode);
  	  }
	  if(pFilter->FiltLevelRegConf.p_chr_v_mode){
		free(pFilter->FiltLevelRegConf.p_chr_v_mode);
  	  }
	  if(pFilter->FiltLevelRegConf.p_fac_bl0){
		free(pFilter->FiltLevelRegConf.p_fac_bl0);
  	  }
	  if(pFilter->FiltLevelRegConf.p_fac_bl1){
		free(pFilter->FiltLevelRegConf.p_fac_bl1);
  	  }
	  if(pFilter->FiltLevelRegConf.p_fac_mid){
		free(pFilter->FiltLevelRegConf.p_fac_mid);
  	  }
	  if(pFilter->FiltLevelRegConf.p_fac_sh0){
		free(pFilter->FiltLevelRegConf.p_fac_sh0);
  	  }
	  if(pFilter->FiltLevelRegConf.p_fac_sh1){
		free(pFilter->FiltLevelRegConf.p_fac_sh1);
  	  }
	  if(pFilter->FiltLevelRegConf.p_FiltLevel){
		free(pFilter->FiltLevelRegConf.p_FiltLevel);
  	  }
	  if(pFilter->FiltLevelRegConf.p_grn_stage1){
		free(pFilter->FiltLevelRegConf.p_grn_stage1);
  	  }
	  if(pFilter->FiltLevelRegConf.p_thresh_bl0){
		free(pFilter->FiltLevelRegConf.p_thresh_bl0);
  	  }
	  if(pFilter->FiltLevelRegConf.p_thresh_bl1){
		free(pFilter->FiltLevelRegConf.p_thresh_bl1);
  	  }
	  if(pFilter->FiltLevelRegConf.p_thresh_sh0){
		free(pFilter->FiltLevelRegConf.p_thresh_sh0);
  	  }
	  if(pFilter->FiltLevelRegConf.p_thresh_sh1){
		free(pFilter->FiltLevelRegConf.p_thresh_sh1);
  	  }
	  
      free(pFilter);

      /* 3.) get next item */
      pFilter = (CamFilterProfile_t*)ListRemoveHead(l);
    }
  }

  ListInit(l);
}

/******************************************************************************
 * ClearDpfProfileList
 *****************************************************************************/
static void ClearDpfProfileList(List* l) {
  if (!ListEmpty(l)) {
    CamDpfProfile_t* pDpfProfile = (CamDpfProfile_t*)ListRemoveHead(l);
    while (pDpfProfile) {
	  ClearDsp3DNRList(&pDpfProfile->Dsp3DNRSettingProfileList);
	  ClearFilterList(&pDpfProfile->FilterList);
      free(pDpfProfile);
      pDpfProfile = (CamDpfProfile_t*)ListRemoveHead(l);
    }
  }

  ListInit(l);
}

/******************************************************************************
 * ClearDpfProfileList
 *****************************************************************************/
static void ClearDpccProfileList(List* l) {
  if (!ListEmpty(l)) {
    CamDpccProfile_t* pDpccProfile = (CamDpccProfile_t*)ListRemoveHead(l);
    while (pDpccProfile) {
      free(pDpccProfile);
      pDpccProfile = (CamDpccProfile_t*)ListRemoveHead(l);
    }
  }

  ListInit(l);
}

/******************************************************************************
 * ClearGocProfileList
 *****************************************************************************/
static void ClearGocProfileList(List* l) {
  if (!ListEmpty(l)) {
    CamCalibGocProfile_t* pGocProfile = (CamCalibGocProfile_t*)ListRemoveHead(l);
    while (pGocProfile) {
      free(pGocProfile);
      pGocProfile = (CamCalibGocProfile_t*)ListRemoveHead(l);
    }
  }

  ListInit(l);
}


/******************************************************************************
 * ValidateFrameRate
 *****************************************************************************/
static inline RESULT ValidateFrameRate(CamFrameRate_t* pFrameRate) {
  LOGV("%s (enter)\n", __func__);

  if (NULL == pFrameRate) {
    return (RET_NULL_POINTER);
  }

  if ((!strlen(pFrameRate->name))
      || (!pFrameRate->fps)) {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * ValidateResolution
 *****************************************************************************/
static inline RESULT ValidateResolution(CamResolution_t* pResolution) {
  LOGV("%s (enter)\n", __func__);

  if (NULL == pResolution) {
    return (RET_NULL_POINTER);
  }

  if ((!strlen(pResolution->name))
      || (!pResolution->width)
      || (!pResolution->height)) {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * ValidateAwbData
 *****************************************************************************/
static inline RESULT ValidateAwbData(CamCalibAwbGlobal_t* pAwbData) {
  LOGV("%s (enter)\n", __func__);

  if (NULL == pAwbData) {
    return (RET_NULL_POINTER);
  }

  if ((!strlen(pAwbData->name))
      || (!strlen(pAwbData->resolution))) {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * ValidateAecGlobalData
 *****************************************************************************/
static inline RESULT ValidateAecGlobalData(CamCalibAecGlobal_t* pAecData) {
  LOGV("%s (enter)\n", __func__);

  if (NULL == pAecData) {
    return (RET_NULL_POINTER);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * ValidateEcmProfile
 *****************************************************************************/
static inline RESULT ValidateEcmProfile(CamEcmProfile_t* pEcmProfile) {
  LOGV("%s (enter)\n", __func__);

  if (NULL == pEcmProfile) {
    return (RET_NULL_POINTER);
  }

  if (!strlen(pEcmProfile->name)) {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * ValidateEcmScheme
 *****************************************************************************/
static inline RESULT ValidateEcmScheme(CamEcmScheme_t* pEcmScheme) {
  LOGV("%s (enter)\n", __func__);

  if (NULL == pEcmScheme) {
    return (RET_NULL_POINTER);
  }

  if (!strlen(pEcmScheme->name)) {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * ValidateIllumination
 *****************************************************************************/
static inline RESULT ValidateIllumination(CamIlluProfile_t* pIllumination) {
  LOGV("%s (enter)\n", __func__);

  if (NULL == pIllumination) {
    return (RET_NULL_POINTER);
  }

  if (!strlen(pIllumination->name)) {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * ValidateLscProfile
 *****************************************************************************/
static inline RESULT ValidateLscProfile(CamLscProfile_t* pLscProfile) {
  LOGV("%s (enter)\n", __func__);

  if (NULL == pLscProfile) {
    return (RET_NULL_POINTER);
  }

  if ((!strlen(pLscProfile->name))
      || (!strlen(pLscProfile->resolution))
      || (!strlen(pLscProfile->illumination))) {
    return (RET_INVALID_PARM);
  }

  if (!(0.0f < pLscProfile->vignetting)) {
    return (RET_OUTOFRANGE);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * ValidateCcProfile
 *****************************************************************************/
static inline RESULT ValidateCcProfile(CamCcProfile_t* pCcProfile) {
  LOGV("%s (enter)\n", __func__);

  if (NULL == pCcProfile) {
    return (RET_NULL_POINTER);
  }

  if (!strlen(pCcProfile->name)) {
    return (RET_INVALID_PARM);
  }

  if (!(0.0f < pCcProfile->saturation)) {
    return (RET_OUTOFRANGE);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * ValidateBlsProfile
 *****************************************************************************/
static inline RESULT ValidateBlsProfile(CamBlsProfile_t* pBlsProfile) {
  LOGV("%s (enter)\n", __func__);

  if (NULL == pBlsProfile) {
    return (RET_NULL_POINTER);
  }

  if ((!strlen(pBlsProfile->name))
      || (!strlen(pBlsProfile->resolution))) {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * ValidateCacProfile
 *****************************************************************************/
static inline RESULT ValidateCacProfile(CamCacProfile_t* pCacProfile) {
  LOGV("%s (enter)\n", __func__);

  if (NULL == pCacProfile) {
    return (RET_NULL_POINTER);
  }

  if ((!strlen(pCacProfile->name))
      || (!strlen(pCacProfile->resolution))) {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * ValidateDpfProfile
 *****************************************************************************/
static inline RESULT ValidateDpfProfile(CamDpfProfile_t* pDpfProfile) {
  LOGV("%s (enter)\n", __func__);

  if (NULL == pDpfProfile) {
    return (RET_NULL_POINTER);
  }

  if ((!strlen(pDpfProfile->name))
      || (!strlen(pDpfProfile->resolution))) {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * ValidateEcmScheme
 *****************************************************************************/
static inline RESULT ValidateDsp3dnrSetting(CamDsp3DNRSettingProfile_t* pDsp3dnr) {
  LOGV("%s (enter)\n", __func__);

  if (NULL == pDsp3dnr) {
    return (RET_NULL_POINTER);
  }

  if (!strlen(pDsp3dnr->name)) {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

static inline RESULT ValidateFilterProfile(CamFilterProfile_t* pFilter) {
  LOGV("%s (enter)\n", __func__);

  if (NULL == pFilter) {
    return (RET_NULL_POINTER);
  }

  if (!strlen(pFilter->name)) {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * ValidateDpccProfile
 *****************************************************************************/
static inline RESULT ValidateDpccProfile(CamDpccProfile_t* pDpccProfile) {
  LOGV("%s (enter)\n", __func__);

  if (NULL == pDpccProfile) {
    return (RET_NULL_POINTER);
  }

  if ((!strlen(pDpccProfile->name))
      || (!strlen(pDpccProfile->resolution))) {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * ClearContext
 *****************************************************************************/
static RESULT ClearContext(CamCalibDbContext_t* pCamCalibDbCtx) {
  LOGV("%s (enter)\n", __func__);

  if (pCamCalibDbCtx == NULL) {
    return (RET_WRONG_HANDLE);
  }

  ClearResolutionList(&pCamCalibDbCtx->resolution);
  ClearAwbGlobalList(&pCamCalibDbCtx->awb_global);
  if (pCamCalibDbCtx->pAecGlobal) {
  	if(pCamCalibDbCtx->pAecGlobal->GainRange.pGainRange != NULL){
		free(pCamCalibDbCtx->pAecGlobal->GainRange.pGainRange);
	}
	if(pCamCalibDbCtx->pAecGlobal->GridWeights.pWeight){
		free(pCamCalibDbCtx->pAecGlobal->GridWeights.pWeight);
	}
    free(pCamCalibDbCtx->pAecGlobal);
  }
  
  if (pCamCalibDbCtx->pWdrGlobal) {
    if (pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.pfMaxGain_level != NULL) {
      free(pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.pfMaxGain_level);
    }
    if (pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.pfSensorGain_level != NULL) {
      free(pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.pfSensorGain_level);
    }
    free(pCamCalibDbCtx->pWdrGlobal);
  }

  if (pCamCalibDbCtx->pCprocGlobal)
    free(pCamCalibDbCtx->pCprocGlobal);
  ClearEcmProfileList(& pCamCalibDbCtx->ecm_profile);
  ClearIlluminationList(&pCamCalibDbCtx->illumination);
  ClearLscProfileList(&pCamCalibDbCtx->lsc_profile);
  ClearCcProfileList(&pCamCalibDbCtx->cc_profile);
  ClearBlsProfileList(&pCamCalibDbCtx->bls_profile);
  ClearCacProfileList(&pCamCalibDbCtx->cac_profile);
  ClearDpfProfileList(&pCamCalibDbCtx->dpf_profile);
  ClearDpccProfileList(&pCamCalibDbCtx->dpcc_profile);
  ClearGocProfileList(&pCamCalibDbCtx->gocProfile);

  MEMSET(pCamCalibDbCtx, 0, sizeof(CamCalibDbContext_t));

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * See header file for detailed comment.
 *****************************************************************************/

/******************************************************************************
 * CamCalibDbCreate
 *****************************************************************************/
RESULT CamCalibDbCreate
(
    CamCalibDbHandle_t*  hCamCalibDb
) {
  CamCalibDbContext_t* pCamCalibDbCtx;

  RESULT result = RET_SUCCESS;

  LOGV("%s (enter)\n", __func__);
  /* pre-check */
  if (hCamCalibDb == NULL) {
    return (RET_NULL_POINTER);
  }
  /* allocate control context */
  pCamCalibDbCtx = malloc(sizeof(CamCalibDbContext_t));
  if (pCamCalibDbCtx == NULL) {
    ALOGE("%s (allocating control context failed)\n", __func__);
    return (RET_OUTOFMEM);
  }
  MEMSET(pCamCalibDbCtx, 0, sizeof(CamCalibDbContext_t));
  ListInit(&pCamCalibDbCtx->resolution);
  ListInit(&pCamCalibDbCtx->awb_global);
  pCamCalibDbCtx->pAecGlobal = NULL;
  ListInit(&pCamCalibDbCtx->ecm_profile);
  ListInit(&pCamCalibDbCtx->illumination);
  ListInit(&pCamCalibDbCtx->lsc_profile);
  ListInit(&pCamCalibDbCtx->cc_profile);
  ListInit(&pCamCalibDbCtx->cac_profile);
  ListInit(&pCamCalibDbCtx->dpf_profile);
  ListInit(&pCamCalibDbCtx->dpcc_profile);

  *hCamCalibDb = (CamCalibDbHandle_t)pCamCalibDbCtx;
  LOGV("%s (exit)\n", __func__);

  return (result);
}



/******************************************************************************
 * CamCalibDbRelease
 *****************************************************************************/
RESULT CamCalibDbRelease
(
    CamCalibDbHandle_t*  handle
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)(*handle);

  RESULT result = RET_SUCCESS;

  LOGV("%s (enter)\n", __func__);

  if (pCamCalibDbCtx == NULL) {
    return (RET_WRONG_HANDLE);
  }

  result = ClearContext(pCamCalibDbCtx);
  free(pCamCalibDbCtx);
  *handle = NULL;

  LOGV("%s (exit)\n", __func__);

  return (result);
}



/******************************************************************************
 * CamCalibDbClear
 *****************************************************************************/
RESULT CamCalibDbClear
(
    CamCalibDbHandle_t  handle
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)handle;

  RESULT result = RET_SUCCESS;

  LOGV("%s (enter)\n", __func__);

  if (pCamCalibDbCtx == NULL) {
    return (RET_WRONG_HANDLE);
  }

  result = ClearContext(pCamCalibDbCtx);

  LOGV("%s (exit)\n", __func__);

  return (result);
}



/******************************************************************************
 * CamCalibDbSetMetaData
 *****************************************************************************/
RESULT CamCalibDbSetMetaData
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamCalibDbMetaData_t*    pMeta
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pMeta) {
    return (RET_INVALID_PARM);
  }

  strncpy(pCamCalibDbCtx->cdate, pMeta->cdate, sizeof(pCamCalibDbCtx->cdate));
  strncpy(pCamCalibDbCtx->cname, pMeta->cname, sizeof(pCamCalibDbCtx->cname));
  strncpy(pCamCalibDbCtx->cversion, pMeta->cversion, sizeof(pCamCalibDbCtx->cversion));
  strncpy(pCamCalibDbCtx->sname, pMeta->sname, sizeof(pCamCalibDbCtx->sname));
  strncpy(pCamCalibDbCtx->sid, pMeta->sid, sizeof(pCamCalibDbCtx->sid));

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

RESULT CamCalibDbGetMetaData
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamCalibDbMetaData_t*    pMeta
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pMeta) {
    return (RET_INVALID_PARM);
  }

  strncpy(pMeta->cdate, pCamCalibDbCtx->cdate, sizeof(pCamCalibDbCtx->cdate));
  strncpy(pMeta->cname, pCamCalibDbCtx->cname, sizeof(pCamCalibDbCtx->cname));
  strncpy(pMeta->cversion, pCamCalibDbCtx->cversion, sizeof(pCamCalibDbCtx->cversion));
  strncpy(pMeta->sname, pCamCalibDbCtx->sname, sizeof(pCamCalibDbCtx->sname));
  strncpy(pMeta->sid, pCamCalibDbCtx->sid, sizeof(pCamCalibDbCtx->sid));

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbSetSystemData
 *****************************************************************************/
RESULT CamCalibDbSetSystemData
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamCalibSystemData_t*    pData
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pData) {
    return (RET_INVALID_PARM);
  }

  MEMCPY(&pCamCalibDbCtx->system, pData, sizeof(CamCalibSystemData_t));

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetSystemData
 *****************************************************************************/
RESULT CamCalibDbGetSystemData
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamCalibSystemData_t*    pData
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);
  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }
  if (NULL == pData) {
    return (RET_INVALID_PARM);
  }
  *pData = pCamCalibDbCtx->system;

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbAddFrameRate
 *****************************************************************************/
RESULT CamCalibDbAddFrameRate
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamResolution_t*         pResolution,
    CamFrameRate_t*          pAddFrameRate
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamFrameRate_t* pNewFrameRate = NULL;

  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pResolution) {
    return (RET_INVALID_PARM);
  }

  result = ValidateFrameRate(pAddFrameRate);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if data already exists */
  pNewFrameRate = (CamFrameRate_t*)ListSearch(&pResolution->framerates, SearchForEqualFrameRate, (void*)pAddFrameRate);
  if (NULL != pNewFrameRate) {
    return (RET_NOTAVAILABLE);
  }

  /* finally allocate, copy & add scheme */
  pNewFrameRate = malloc(sizeof(CamFrameRate_t));
  if (NULL == pNewFrameRate) {
    return (RET_OUTOFMEM);
  }
  MEMCPY(pNewFrameRate, pAddFrameRate, sizeof(CamFrameRate_t));

  ListPrepareItem(pNewFrameRate);
  ListAddTail(&pResolution->framerates, pNewFrameRate);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbAddResolution
 *****************************************************************************/
RESULT CamCalibDbAddResolution
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamResolution_t*     pAddRes
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamResolution_t* pNewRes = NULL;

  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateResolution(pAddRes);
  if (result != RET_SUCCESS) {
    return (result);
  }

  LOGD("%s try to add resolution %s, id=%08x, w:%d, h:%d\n",
        __func__, pAddRes->name, pAddRes->id, pAddRes->width, pAddRes->height);

  /* check if resolution already exists */
  pNewRes = (CamResolution_t*)ListSearch(&pCamCalibDbCtx->resolution, SearchForEqualResolution, (void*)pAddRes);
  if (NULL != pNewRes) {
    LOGD("%s resolution %s, id=%08x, w:%d, h:%d\n",
          __func__, pNewRes->name, pNewRes->id, pNewRes->width, pNewRes->height);
    return (RET_NOTAVAILABLE);
  }

  pNewRes = malloc(sizeof(CamResolution_t));
  if (NULL == pNewRes) {
    return (RET_OUTOFMEM);
  }
  MEMCPY(pNewRes, pAddRes, sizeof(CamResolution_t));
  ListInit(&pNewRes->framerates);   // clear possibly not empty schemes list in copy

  ListPrepareItem(pNewRes);
  ListAddTail(&pCamCalibDbCtx->resolution, pNewRes);

  /* add already linked schemes as well */
  CamFrameRate_t* pFrameRate = (CamFrameRate_t*)ListHead(&pAddRes->framerates);
  while (pFrameRate) {
    result = CamCalibDbAddFrameRate(hCamCalibDb, pNewRes, pFrameRate);
    if (result != RET_SUCCESS) {
      return (result);
    }
    pFrameRate = (CamFrameRate_t*)pFrameRate->p_next;
  }

  LOGD("%s added resolution %s, id=%08x, w:%d, h:%d\n",
        __func__, pNewRes->name, pNewRes->id, pNewRes->width, pNewRes->height);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetNoOfResolutions
 *****************************************************************************/
RESULT CamCalibDbGetNoOfResolutions
(
    CamCalibDbHandle_t  hCamCalibDb,
    int32_t*             no
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == no) {
    return (RET_INVALID_PARM);
  }

  *no = (uint32_t)ListNoItems(&pCamCalibDbCtx->resolution);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetResolutionByName
 *****************************************************************************/
RESULT CamCalibDbGetResolutionByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamResolutionName_t     name,
    CamResolution_t**         pResolution
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pResolution) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pResolution = (CamResolution_t*)ListSearch(&pCamCalibDbCtx->resolution, SearchResolutionByName, (void*)name);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetResolutionByWidthHeight
 *****************************************************************************/
RESULT CamCalibDbGetResolutionByWidthHeight
(
    CamCalibDbHandle_t      hCamCalibDb,
    const uint16_t          width,
    const uint16_t          height,
    CamResolution_t**         pResolution
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamResolution_t SearchParam;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pResolution) {
    return (RET_INVALID_PARM);
  }

  if ((!width) || (!height)) {
    return (RET_INVALID_PARM);
  }

  MEMSET(&SearchParam, 0, sizeof(SearchParam));
  SearchParam.width      = width;
  SearchParam.height     = height;

  /* search resolution by name */
  *pResolution = (CamResolution_t*)ListSearch(&pCamCalibDbCtx->resolution,
                                              SearchResolutionByWidthHeight, (void*)&SearchParam);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetResolutionNameByWidthHeightFrameRate
 *****************************************************************************/
RESULT CamCalibDbGetResolutionNameByWidthHeight
(
    CamCalibDbHandle_t      hCamCalibDb,
    const uint16_t          width,
    const uint16_t          height,
    CamResolutionName_t*     pResolutionName
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamResolution_t     SearchParam;
  CamResolution_t*     pResolution;

  RESULT result = RET_SUCCESS;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pResolutionName) {
    return (RET_INVALID_PARM);
  }

  MEMSET(&SearchParam, 0, sizeof(SearchParam));
  SearchParam.width      = width;
  SearchParam.height     = height;

  /* search resolution by name */
  pResolution = (CamResolution_t*)ListSearch(&pCamCalibDbCtx->resolution,
                                             SearchResolutionByWidthHeight, (void*)&SearchParam);
  if (pResolution) {
    strncpy((char*)pResolutionName, (char*)pResolution->name, sizeof(CamResolutionName_t));
  } else {
    MEMSET(pResolutionName, 0, sizeof(CamResolutionName_t));
    result = RET_NOTSUPP;
  }

  LOGV("%s (exit)\n", __func__);

  return (result);
}



/******************************************************************************
 * CamCalibDbGetResolutionIdxByName
 *****************************************************************************/
RESULT CamCalibDbGetResolutionIdxByName
(
    CamCalibDbHandle_t          hCamCalibDb,
    const CamResolutionName_t   name,
    int32_t*                     pIdx
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamResolution_t     SearchParam;
  CamResolution_t*     pResolution;

  RESULT result = RET_SUCCESS;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pIdx) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pIdx = ListGetIdxByItem(&pCamCalibDbCtx->resolution, SearchResolutionByName, (void*)name);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbAddAwbGlobal
 *****************************************************************************/
RESULT CamCalibDbAddAwbGlobal
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamCalibAwbGlobal_t* pAddAwbGlobal
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamCalibAwbGlobal_t* pNewAwbGlobal = NULL;

  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateAwbData(pAddAwbGlobal);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if data already exists */
  pNewAwbGlobal = (CamCalibAwbGlobal_t*)ListSearch(&pCamCalibDbCtx->awb_global, SearchForEqualAwbGlobal, (void*)pAddAwbGlobal);
  if (NULL == pNewAwbGlobal) {
    CamAwbClipParm_t*        pAwbClipParam      = NULL;
    CamAwbGlobalFadeParm_t*  pAwbGlobalFadeParm = NULL;
    CamAwbFade2Parm_t*       pAwbFade2Parm      = NULL;

    int32_t nArraySize1;
    int32_t nArraySize2;

    pNewAwbGlobal = malloc(sizeof(CamCalibAwbGlobal_t));
    MEMCPY(pNewAwbGlobal, pAddAwbGlobal, sizeof(CamCalibAwbGlobal_t));

    pAwbClipParam       = &pNewAwbGlobal->AwbClipParam;
    pAwbGlobalFadeParm  = &pNewAwbGlobal->AwbGlobalFadeParm;
    pAwbFade2Parm       = &pNewAwbGlobal->AwbFade2Parm;

    // pAwbClipParam
    nArraySize1 = pAddAwbGlobal->AwbClipParam.ArraySize1;
    nArraySize2 = pAddAwbGlobal->AwbClipParam.ArraySize2;
    pAwbClipParam->pRg1 = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbClipParam->pRg1, pAddAwbGlobal->AwbClipParam.pRg1, sizeof(float) *  nArraySize1);
    pAwbClipParam->pMaxDist1 = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbClipParam->pMaxDist1, pAddAwbGlobal->AwbClipParam.pMaxDist1, sizeof(float) *  nArraySize1);
    pAwbClipParam->pRg2 = malloc(sizeof(float) *  nArraySize2);
    MEMCPY(pAwbClipParam->pRg2, pAddAwbGlobal->AwbClipParam.pRg2, sizeof(float) *  nArraySize2);
    pAwbClipParam->pMaxDist2 = malloc(sizeof(float) *  nArraySize2);
    MEMCPY(pAwbClipParam->pMaxDist2, pAddAwbGlobal->AwbClipParam.pMaxDist2, sizeof(float) *  nArraySize2);

    // pAwbGlobalFadeParm
    nArraySize1 = pAddAwbGlobal->AwbGlobalFadeParm.ArraySize1;
    nArraySize2 = pAddAwbGlobal->AwbGlobalFadeParm.ArraySize2;
    pAwbGlobalFadeParm->pGlobalFade1 = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbGlobalFadeParm->pGlobalFade1, pAddAwbGlobal->AwbGlobalFadeParm.pGlobalFade1, sizeof(float) *  nArraySize1);
    pAwbGlobalFadeParm->pGlobalGainDistance1 = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbGlobalFadeParm->pGlobalGainDistance1, pAddAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance1, sizeof(float) *  nArraySize1);
    pAwbGlobalFadeParm->pGlobalFade2 = malloc(sizeof(float) *  nArraySize2);
    MEMCPY(pAwbGlobalFadeParm->pGlobalFade2, pAddAwbGlobal->AwbGlobalFadeParm.pGlobalFade2, sizeof(float) *  nArraySize2);
    pAwbGlobalFadeParm->pGlobalGainDistance2 = malloc(sizeof(float) *  nArraySize2);
    MEMCPY(pAwbGlobalFadeParm->pGlobalGainDistance2, pAddAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance2, sizeof(float) *  nArraySize2);

    // pAwbFade2Parm
    nArraySize1 = pAddAwbGlobal->AwbFade2Parm.ArraySize;
    nArraySize2 = 0l;
    pAwbFade2Parm->pFade                = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pFade, pAddAwbGlobal->AwbFade2Parm.pFade, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pCbMinRegionMax      = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pCbMinRegionMax, pAddAwbGlobal->AwbFade2Parm.pCbMinRegionMax, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pCrMinRegionMax      = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pCrMinRegionMax, pAddAwbGlobal->AwbFade2Parm.pCrMinRegionMax, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMaxCSumRegionMax    = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pMaxCSumRegionMax, pAddAwbGlobal->AwbFade2Parm.pMaxCSumRegionMax, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pCbMinRegionMin      = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pCbMinRegionMin, pAddAwbGlobal->AwbFade2Parm.pCbMinRegionMin, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pCrMinRegionMin      = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pCrMinRegionMin, pAddAwbGlobal->AwbFade2Parm.pCrMinRegionMin, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMaxCSumRegionMin    = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pMaxCSumRegionMin, pAddAwbGlobal->AwbFade2Parm.pMaxCSumRegionMin, sizeof(float) *  nArraySize1);

    pAwbFade2Parm->pMinCRegionMax = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pMinCRegionMax, pAddAwbGlobal->AwbFade2Parm.pMinCRegionMax, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMinCRegionMin = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pMinCRegionMin, pAddAwbGlobal->AwbFade2Parm.pMinCRegionMin, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMaxYRegionMax = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pMaxYRegionMax, pAddAwbGlobal->AwbFade2Parm.pMaxYRegionMax, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMaxYRegionMin = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pMaxYRegionMin, pAddAwbGlobal->AwbFade2Parm.pMaxYRegionMin, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMinYMaxGRegionMax = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pMinYMaxGRegionMax, pAddAwbGlobal->AwbFade2Parm.pMinYMaxGRegionMax, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMinYMaxGRegionMin = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pMinYMaxGRegionMin, pAddAwbGlobal->AwbFade2Parm.pMinYMaxGRegionMin, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pRefCb = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pRefCb, pAddAwbGlobal->AwbFade2Parm.pRefCb, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pRefCr = malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pRefCr, pAddAwbGlobal->AwbFade2Parm.pRefCr, sizeof(float) *  nArraySize1);

    ListPrepareItem(pNewAwbGlobal);
    ListAddTail(&pCamCalibDbCtx->awb_global, pNewAwbGlobal);
  } else {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetAwbGlobalByResolution
 *****************************************************************************/
RESULT CamCalibDbGetAwbGlobalByResolution
(
    CamCalibDbHandle_t          hCamCalibDb,
    const CamResolutionName_t   ResName,
    CamCalibAwbGlobal_t**         pAwbGlobal
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pAwbGlobal) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pAwbGlobal = (CamCalibAwbGlobal_t*)ListSearch(&pCamCalibDbCtx->awb_global, SearchAwbGlobalByResolution, (void*)ResName);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbAddAecGlobal
 *****************************************************************************/
RESULT CamCalibDbAddAecGlobal
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamCalibAecGlobal_t* pAddAecGlobal
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateAecGlobalData(pAddAecGlobal);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if data already exists */
  if (NULL != pCamCalibDbCtx->pAecGlobal) {
    return (RET_INVALID_PARM);
  }

  /* finally allocate, copy & add data */
  CamCalibAecGlobal_t* pNewAecGlobal = malloc(sizeof(CamCalibAecGlobal_t));
  if (NULL == pNewAecGlobal) {
    return (RET_OUTOFMEM);
  }
  MEMCPY(pNewAecGlobal, pAddAecGlobal, sizeof(CamCalibAecGlobal_t));

  pCamCalibDbCtx->pAecGlobal = pNewAecGlobal;

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbGetAecGlobal
 *****************************************************************************/
RESULT CamCalibDbGetAecGlobal
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamCalibAecGlobal_t** ppAecGlobal
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == ppAecGlobal) {
    return (RET_INVALID_PARM);
  }

  /* return reference to global AEC configuration */
  *ppAecGlobal = pCamCalibDbCtx->pAecGlobal;

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbAddEcmProfile
 *****************************************************************************/
RESULT CamCalibDbAddEcmProfile
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamEcmProfile_t*         pAddEcmProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamEcmProfile_t* pNewEcmProfile = NULL;

  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateEcmProfile(pAddEcmProfile);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if data already exists */
  pNewEcmProfile = (CamEcmProfile_t*)ListSearch(&pCamCalibDbCtx->ecm_profile, SearchForEqualEcmProfile, (void*)pAddEcmProfile);
  if (NULL != pNewEcmProfile) {
    return (RET_NOTAVAILABLE);
  }

  /* finally allocate, copy & add profile */
  pNewEcmProfile = malloc(sizeof(CamEcmProfile_t));
  if (NULL == pNewEcmProfile) {
    return (RET_OUTOFMEM);
  }
  MEMCPY(pNewEcmProfile, pAddEcmProfile, sizeof(CamEcmProfile_t));
  ListInit(&pNewEcmProfile->ecm_scheme);   // clear possibly not empty schemes list in copy

  ListPrepareItem(pNewEcmProfile);
  ListAddTail(&pCamCalibDbCtx->ecm_profile, pNewEcmProfile);

  /* add already linked schemes as well */
  CamEcmScheme_t* pEcmScheme = (CamEcmScheme_t*)ListHead(&pAddEcmProfile->ecm_scheme);
  while (pEcmScheme) {
    result = CamCalibDbAddEcmScheme(hCamCalibDb, pNewEcmProfile, pEcmScheme);
    if (result != RET_SUCCESS) {
      return (result);
    }

    pEcmScheme = (CamEcmScheme_t*)pEcmScheme->p_next;
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetNoOfEcmProfiles
 *****************************************************************************/
RESULT CamCalibDbGetNoOfEcmProfiles
(
    CamCalibDbHandle_t  hCamCalibDb,
    int32_t*             no
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == no) {
    return (RET_INVALID_PARM);
  }

  *no = (uint32_t)ListNoItems(&pCamCalibDbCtx->ecm_profile);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbGetResolutionNameByWidthHeightFrameRate
 *****************************************************************************/
RESULT CamCalibDbGetEcmProfileNameByWidthHeightFrameRate
(
    CamCalibDbHandle_t      hCamCalibDb,
    const uint16_t          width,
    const uint16_t          height,
    const uint16_t          fps,
    CamEcmProfileName_t*     pEcmProfileName
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  RESULT result = RET_SUCCESS;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pEcmProfileName) {
    return (RET_INVALID_PARM);
  }

  if (!fps) {
    snprintf((char*)pEcmProfileName, sizeof(CamEcmProfileName_t), "%dx%d", width, height);
  } else {
    snprintf((char*)pEcmProfileName, sizeof(CamEcmProfileName_t), "%dx%d_FPS_%02d", width, height, fps);
  }

  LOGV("%s (exit)\n", __func__);

  return (result);
}


/******************************************************************************
 * CamCalibDbGetEcmProfileByName
 *****************************************************************************/
RESULT CamCalibDbGetEcmProfileByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamEcmProfileName_t     EcmProfileName,
    CamEcmProfile_t**         ppEcmProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == ppEcmProfile) {
    return (RET_INVALID_PARM);
  }

  /* search profile by name */
  *ppEcmProfile = (CamEcmProfile_t*)ListSearch(&pCamCalibDbCtx->ecm_profile, SearchEcmProfileByName, (void*)EcmProfileName);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetEcmProfileByIdx
 *****************************************************************************/
RESULT CamCalibDbGetEcmProfileByIdx
(
    CamCalibDbHandle_t      hCamCalibDb,
    const uint32_t          idx,
    CamEcmProfile_t**         ppEcmProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == ppEcmProfile) {
    return (RET_INVALID_PARM);
  }

  /* search profile by index */
  *ppEcmProfile = (CamEcmProfile_t*)ListGetItemByIdx(&pCamCalibDbCtx->ecm_profile, idx);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbAddEcmScheme
 *****************************************************************************/
RESULT CamCalibDbAddEcmScheme
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamEcmProfile_t*         pEcmProfile,
    CamEcmScheme_t*          pAddEcmScheme
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamEcmScheme_t* pNewEcmScheme = NULL;

  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pEcmProfile) {
    return (RET_INVALID_PARM);
  }

  result = ValidateEcmScheme(pAddEcmScheme);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if data already exists */
  pNewEcmScheme = (CamEcmScheme_t*)ListSearch(&pEcmProfile->ecm_scheme, SearchForEqualEcmScheme, (void*)pAddEcmScheme);
  if (NULL != pNewEcmScheme) {
    return (RET_NOTAVAILABLE);
  }

  /* finally allocate, copy & add scheme */
  pNewEcmScheme = malloc(sizeof(CamEcmScheme_t));
  if (NULL == pNewEcmScheme) {
    return (RET_OUTOFMEM);
  }
  MEMCPY(pNewEcmScheme, pAddEcmScheme, sizeof(CamEcmScheme_t));

  ListPrepareItem(pNewEcmScheme);
  ListAddTail(&pEcmProfile->ecm_scheme, pNewEcmScheme);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetNoOfEcmSchemes
 *****************************************************************************/
RESULT CamCalibDbGetNoOfEcmSchemes
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamEcmProfile_t*         pEcmProfile,
    int32_t*                 no
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pEcmProfile) {
    return (RET_INVALID_PARM);
  }

  if (NULL == no) {
    return (RET_INVALID_PARM);
  }

  *no = (uint32_t)ListNoItems(&pEcmProfile->ecm_scheme);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetEcmSchemeByName
 *****************************************************************************/
RESULT CamCalibDbGetEcmSchemeByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamEcmProfile_t*         pEcmProfile,
    CamEcmSchemeName_t      EcmSchemeName,
    CamEcmScheme_t**          ppEcmScheme
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == ppEcmScheme) {
    return (RET_INVALID_PARM);
  }

  /* search scheme by name */
  *ppEcmScheme = (CamEcmScheme_t*)ListSearch(&pEcmProfile->ecm_scheme, SearchEcmSchemeByName, (void*)EcmSchemeName);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetEcmSchemeByIdx
 *****************************************************************************/
RESULT CamCalibDbGetEcmSchemeByIdx
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamEcmProfile_t*         pEcmProfile,
    const uint32_t          idx,
    CamEcmScheme_t**          ppEcmScheme
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pEcmProfile) {
    return (RET_INVALID_PARM);
  }

  if (NULL == ppEcmScheme) {
    return (RET_INVALID_PARM);
  }

  /* search EC scheme by index */
  *ppEcmScheme = (CamEcmScheme_t*)ListGetItemByIdx(&pEcmProfile->ecm_scheme, idx);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetNoOfIlluminations
 *****************************************************************************/
RESULT CamCalibDbGetNoOfIlluminations
(
    CamCalibDbHandle_t  hCamCalibDb,
    int32_t*             no
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == no) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *no = (uint32_t)ListNoItems(&pCamCalibDbCtx->illumination);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbAddIllumination
 *****************************************************************************/
RESULT CamCalibDbAddIllumination
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamIlluProfile_t*    pAddIllu
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamIlluProfile_t* pNewIllu = NULL;

  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateIllumination(pAddIllu);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if resolution already exists */
  pNewIllu = (CamIlluProfile_t*)ListSearch(&pCamCalibDbCtx->illumination, SearchForEqualIllumination, (void*)pAddIllu);
  if (NULL == pNewIllu) {
    /* allocate and copy the illumination profile */
    pNewIllu = (CamIlluProfile_t*)malloc(sizeof(CamIlluProfile_t));
    MEMCPY(pNewIllu, pAddIllu, sizeof(CamIlluProfile_t));

    /* remove pointer from outside allocated memory,
     * we're allocating out own memory */
    pNewIllu->SaturationCurve.pSensorGain = NULL;
    pNewIllu->SaturationCurve.pSaturation = NULL;
    pNewIllu->VignettingCurve.pSensorGain = NULL;
    pNewIllu->VignettingCurve.pVignetting = NULL;

    /* allocate sub structure's of illumination profile */
    int n_items;
    int n_memsize;

    /* saturation over gain curve */
    n_items = pAddIllu->SaturationCurve.ArraySize;
    n_memsize = (n_items * sizeof(float));
    pNewIllu->SaturationCurve.ArraySize = n_items;
    pNewIllu->SaturationCurve.pSensorGain = malloc(n_memsize);
    pNewIllu->SaturationCurve.pSaturation = malloc(n_memsize);
    MEMCPY(pNewIllu->SaturationCurve.pSensorGain, pAddIllu->SaturationCurve.pSensorGain, n_memsize);
    MEMCPY(pNewIllu->SaturationCurve.pSaturation, pAddIllu->SaturationCurve.pSaturation, n_memsize);

    /* vignetting over gain curve */
    n_items = pAddIllu->VignettingCurve.ArraySize;
    n_memsize = (n_items * sizeof(float));
    pNewIllu->VignettingCurve.ArraySize = n_items;
    pNewIllu->VignettingCurve.pSensorGain = malloc(n_memsize);
    pNewIllu->VignettingCurve.pVignetting = malloc(n_memsize);
    MEMCPY(pNewIllu->VignettingCurve.pSensorGain, pAddIllu->VignettingCurve.pSensorGain, n_memsize);
    MEMCPY(pNewIllu->VignettingCurve.pVignetting, pAddIllu->VignettingCurve.pVignetting, n_memsize);

    /* add illumination to list */
    ListPrepareItem(pNewIllu);
    ListAddTail(&pCamCalibDbCtx->illumination, pNewIllu);
  } else {
    return (RET_INVALID_PARM);
  }

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetIlluminationByName
 *****************************************************************************/
RESULT CamCalibDbGetIlluminationByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamIlluminationName_t   name,
    CamIlluProfile_t**        pIllumination
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pIllumination) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pIllumination = (CamIlluProfile_t*)ListSearch(&pCamCalibDbCtx->illumination, SearchIlluminationByName, (void*)name);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetIlluminationByIdx
 *****************************************************************************/
RESULT CamCalibDbGetIlluminationByIdx
(
    CamCalibDbHandle_t  hCamCalibDb,
    const uint32_t      idx,
    CamIlluProfile_t**    pIllumination
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pIllumination) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pIllumination = (CamIlluProfile_t*)ListGetItemByIdx(&pCamCalibDbCtx->illumination, idx);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbAddLscProfile
 *****************************************************************************/
RESULT CamCalibDbAddLscProfile
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamLscProfile_t*     pAddLsc
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamLscProfile_t* pNewLsc = NULL;

  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateLscProfile(pAddLsc);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if resolution already exists */
  pNewLsc = (CamLscProfile_t*)ListSearch(&pCamCalibDbCtx->lsc_profile, SearchForEqualLscProfile, (void*)pAddLsc);
  if (NULL == pNewLsc) {
    pNewLsc = malloc(sizeof(CamLscProfile_t));
    MEMCPY(pNewLsc, pAddLsc, sizeof(CamLscProfile_t));

    ListPrepareItem(pNewLsc);
    ListAddTail(&pCamCalibDbCtx->lsc_profile, pNewLsc);
  } else {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetLscProfileByName
 *****************************************************************************/
RESULT CamCalibDbGetLscProfileByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamLscProfileName_t     name,
    CamLscProfile_t**         pLscProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pLscProfile) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pLscProfile = (CamLscProfile_t*)ListSearch(&pCamCalibDbCtx->lsc_profile, SearchLscProfileByName, (void*)name);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbAddCcProfile
 *****************************************************************************/
RESULT CamCalibDbAddCcProfile
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamCcProfile_t*      pAddCc
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamCcProfile_t* pNewCc = NULL;

  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateCcProfile(pAddCc);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if resolution already exists */
  pNewCc = (CamCcProfile_t*)ListSearch(&pCamCalibDbCtx->cc_profile, SearchForEqualCcProfile, (void*)pAddCc);
  if (NULL == pNewCc) {
    pNewCc = malloc(sizeof(CamCcProfile_t));
    MEMCPY(pNewCc, pAddCc, sizeof(CamCcProfile_t));

    ListPrepareItem(pNewCc);
    ListAddTail(&pCamCalibDbCtx->cc_profile, pNewCc);
  } else {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetCcProfileByName
 *****************************************************************************/
RESULT CamCalibDbGetCcProfileByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamLscProfileName_t     name,
    CamCcProfile_t**          pCcProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pCcProfile) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pCcProfile = (CamCcProfile_t*)ListSearch(&pCamCalibDbCtx->cc_profile, SearchCcProfileByName, (void*)name);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbAddBlsProfile
 *****************************************************************************/
RESULT CamCalibDbAddBlsProfile
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamBlsProfile_t*     pAddBls
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamBlsProfile_t* pNewBls = NULL;

  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateBlsProfile(pAddBls);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if resolution already exists */
  pNewBls = (CamBlsProfile_t*)ListSearch(&pCamCalibDbCtx->bls_profile, SearchForEqualBlsProfile, (void*)pAddBls);
  if (NULL == pNewBls) {
    pNewBls = (CamBlsProfile_t*)malloc(sizeof(CamBlsProfile_t));
    MEMCPY(pNewBls, pAddBls, sizeof(CamBlsProfile_t));

    ListPrepareItem(pNewBls);
    ListAddTail(&pCamCalibDbCtx->bls_profile, pNewBls);
  } else {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetBlsProfileByName
 *****************************************************************************/
RESULT CamCalibDbGetBlsProfileByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamBlsProfileName_t     name,
    CamBlsProfile_t**         pBlsProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pBlsProfile) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pBlsProfile = (CamBlsProfile_t*)ListSearch(&pCamCalibDbCtx->bls_profile, SearchBlsProfileByName, (void*)name);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetBlsProfileByResolution
 *****************************************************************************/
RESULT CamCalibDbGetBlsProfileByResolution
(
    CamCalibDbHandle_t          hCamCalibDb,
    const CamResolutionName_t   ResName,
    CamBlsProfile_t**             pBlsProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pBlsProfile) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pBlsProfile = (CamBlsProfile_t*)ListSearch(&pCamCalibDbCtx->bls_profile, SearchBlsProfileByResolution, (void*)ResName);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbAddCacProfile
 *****************************************************************************/
RESULT CamCalibDbAddCacProfile
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamCacProfile_t*     pAddCac
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamCacProfile_t* pNewCac = NULL;

  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateCacProfile(pAddCac);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if resolution already exists */
  pNewCac = (CamCacProfile_t*)ListSearch(&pCamCalibDbCtx->cac_profile, SearchForEqualCacProfile, (void*)pAddCac);
  if (NULL == pNewCac) {
    pNewCac = (CamCacProfile_t*)malloc(sizeof(CamCacProfile_t));
    MEMCPY(pNewCac, pAddCac, sizeof(CamCacProfile_t));

    ListPrepareItem(pNewCac);
    ListAddTail(&pCamCalibDbCtx->cac_profile, pNewCac);
  } else {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetCacProfileByName
 *****************************************************************************/
RESULT CamCalibDbGetCacProfileByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamCacProfileName_t     name,
    CamCacProfile_t**         pCacProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pCacProfile) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pCacProfile = (CamCacProfile_t*)ListSearch(&pCamCalibDbCtx->cac_profile, SearchCacProfileByName, (void*)name);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetCacProfileByResolution
 *****************************************************************************/
RESULT CamCalibDbGetCacProfileByResolution
(
    CamCalibDbHandle_t          hCamCalibDb,
    const CamResolutionName_t   ResName,
    CamCacProfile_t**             pCacProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pCacProfile) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pCacProfile = (CamCacProfile_t*)ListSearch(&pCamCalibDbCtx->cac_profile, SearchCacProfileByResolution, (void*)ResName);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbAddDpfProfile
 *****************************************************************************/
RESULT CamCalibDbAddDpfProfile
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamDpfProfile_t*     pAddDpf
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamDpfProfile_t* pNewDpf = NULL;

  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateDpfProfile(pAddDpf);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if resolution already exists */
  pNewDpf = (CamDpfProfile_t*)ListSearch(&pCamCalibDbCtx->dpf_profile, SearchForEqualDpfProfile, (void*)pAddDpf);
  if (NULL == pNewDpf) {
    pNewDpf = (CamDpfProfile_t*)malloc(sizeof(CamDpfProfile_t));
    MEMCPY(pNewDpf, pAddDpf, sizeof(CamDpfProfile_t));
	ListInit(&pNewDpf->Dsp3DNRSettingProfileList);   // clear possibly not empty schemes list in copy
	ListInit(&pNewDpf->FilterList);   // clear possibly not empty schemes list in copy
	
    ListPrepareItem(pNewDpf);
    ListAddTail(&pCamCalibDbCtx->dpf_profile, pNewDpf);

	 /* add already linked 3dnr as well */
    CamDsp3DNRSettingProfile_t* pDsp3DNR = (CamDsp3DNRSettingProfile_t*)ListHead(&pAddDpf->Dsp3DNRSettingProfileList);
    while (pDsp3DNR) {
      result = CamCalibDbAddDsp3DNRSetting(hCamCalibDb, pNewDpf, pDsp3DNR);
      if (result != RET_SUCCESS) {
        return (result);
      }
  
      pDsp3DNR = (CamDsp3DNRSettingProfile_t*)pDsp3DNR->p_next;
    }

	/* add already linked filter as well */
	CamFilterProfile_t* pFilter = (CamFilterProfile_t*)ListHead(&pAddDpf->FilterList);
    while (pFilter) {
      result = CamCalibDbAddFilterProfile(hCamCalibDb, pNewDpf, pFilter);
      if (result != RET_SUCCESS) {
        return (result);
      }
  
      pFilter = (CamFilterProfile_t*)pFilter->p_next;
    }
  } else {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetDpfProfileByName
 *****************************************************************************/
RESULT CamCalibDbGetDpfProfileByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamDpfProfileName_t     name,
    CamDpfProfile_t**         pDpfProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pDpfProfile) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pDpfProfile = (CamDpfProfile_t*)ListSearch(&pCamCalibDbCtx->dpf_profile, SearchDpfProfileByName, (void*)name);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetDpfProfileByResolution
 *****************************************************************************/
RESULT CamCalibDbGetDpfProfileByResolution
(
    CamCalibDbHandle_t          hCamCalibDb,
    const CamResolutionName_t   ResName,
    CamDpfProfile_t**             pDpfProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pDpfProfile) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pDpfProfile = (CamDpfProfile_t*)ListSearch(&pCamCalibDbCtx->dpf_profile, SearchDpfProfileByResolution, (void*)ResName);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbAddFilterProfile
 *****************************************************************************/
RESULT CamCalibDbAddFilterProfile
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamDpfProfile_t*         pDpfProfile,
    CamFilterProfile_t*      pAddFilter
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamFilterProfile_t* pNewFilter = NULL;

  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pDpfProfile) {
    return (RET_INVALID_PARM);
  }

  result = ValidateFilterProfile(pAddFilter);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if data already exists */
  pNewFilter = (CamFilterProfile_t*)ListSearch(&pDpfProfile->FilterList, SearchForEqualFilterProfile, (void*)pAddFilter);
  if (NULL != pNewFilter) {
    return (RET_NOTAVAILABLE);
  }

  /* finally allocate, copy & add scheme */
  pNewFilter = malloc(sizeof(CamFilterProfile_t));
  if (NULL == pNewFilter) {
    return (RET_OUTOFMEM);
  }
  MEMCPY(pNewFilter, pAddFilter, sizeof(CamFilterProfile_t));

  ListPrepareItem(pNewFilter);
  ListAddTail(&pDpfProfile->FilterList, pNewFilter);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbGetNoOfFilterProfile
 *****************************************************************************/
RESULT CamCalibDbGetNoOfFilterProfile
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamDpfProfile_t*         pDpfProfile,
    int32_t*                 no
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pDpfProfile) {
    return (RET_INVALID_PARM);
  }

  if (NULL == no) {
    return (RET_INVALID_PARM);
  }

  *no = (uint32_t)ListNoItems(&pDpfProfile->FilterList);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbGetFilterProfileByName
 *****************************************************************************/
RESULT CamCalibDbGetFilterProfileByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamDpfProfile_t*         pDpfProfile,
    CamFilterProfileName_t      FilterProfileName,
    CamFilterProfile_t**          ppFilterProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == ppFilterProfile) {
    return (RET_INVALID_PARM);
  }

  /* search scheme by name */
  *ppFilterProfile = (CamFilterProfile_t*)ListSearch(&pDpfProfile->FilterList, SearchFilterProfileByName, (void*)FilterProfileName);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbGetEcmSchemeByIdx
 *****************************************************************************/
RESULT CamCalibDbGetFilterProfileByIdx
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamDpfProfile_t*         pDpfProfile,
    const uint32_t          idx,
    CamFilterProfile_t**          ppFilterProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pDpfProfile) {
    return (RET_INVALID_PARM);
  }

  if (NULL == ppFilterProfile) {
    return (RET_INVALID_PARM);
  }

  /* search EC scheme by index */
  *ppFilterProfile = (CamFilterProfile_t*)ListGetItemByIdx(&pDpfProfile->FilterList, idx);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbAddDsp3DNRSetting
 *****************************************************************************/
RESULT CamCalibDbAddDsp3DNRSetting
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamDpfProfile_t*         pDpfProfile,
    CamDsp3DNRSettingProfile_t* pAddDsp3DNRSetting
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamDsp3DNRSettingProfile_t* pNewDsp3dnrSetting = NULL;

  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pDpfProfile) {
    return (RET_INVALID_PARM);
  }

  result = ValidateDsp3dnrSetting(pAddDsp3DNRSetting);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if data already exists */
  pNewDsp3dnrSetting = (CamDsp3DNRSettingProfile_t*)ListSearch(&pDpfProfile->Dsp3DNRSettingProfileList, SearchForEqualDsp3DNRSetting, (void*)pAddDsp3DNRSetting);
  if (NULL != pNewDsp3dnrSetting) {
    return (RET_NOTAVAILABLE);
  }

  /* finally allocate, copy & add scheme */
  pNewDsp3dnrSetting = malloc(sizeof(CamDsp3DNRSettingProfile_t));
  if (NULL == pNewDsp3dnrSetting) {
    return (RET_OUTOFMEM);
  }
  MEMCPY(pNewDsp3dnrSetting, pAddDsp3DNRSetting, sizeof(CamDsp3DNRSettingProfile_t));

  ListPrepareItem(pNewDsp3dnrSetting);
  ListAddTail(&pDpfProfile->Dsp3DNRSettingProfileList, pNewDsp3dnrSetting);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbGetNoOfEcmSchemes
 *****************************************************************************/
RESULT CamCalibDbGetNoOfDsp3DNRSetting
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamDpfProfile_t*         pDpfProfile,
    int32_t*                 no
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pDpfProfile) {
    return (RET_INVALID_PARM);
  }

  if (NULL == no) {
    return (RET_INVALID_PARM);
  }

  *no = (uint32_t)ListNoItems(&pDpfProfile->Dsp3DNRSettingProfileList);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetEcmSchemeByName
 *****************************************************************************/
RESULT CamCalibDbGetDsp3DNRSettingByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamDpfProfile_t*         pDpfProfile,
    CamDsp3dnrProfileName_t      Dsp3DNRSettingName,
    CamDsp3DNRSettingProfile_t**          ppDsp3DnrSetting
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == ppDsp3DnrSetting) {
    return (RET_INVALID_PARM);
  }

  /* search scheme by name */
  *ppDsp3DnrSetting = (CamDsp3DNRSettingProfile_t*)ListSearch(&pDpfProfile->Dsp3DNRSettingProfileList, SearchDsp3DNRSettingByName, (void*)Dsp3DNRSettingName);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetEcmSchemeByIdx
 *****************************************************************************/
RESULT CamCalibDbGetDsp3DNRByIdx
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamDpfProfile_t*         pDpfProfile,
    const uint32_t          idx,
    CamDsp3DNRSettingProfile_t**          ppDsp3DnrSetting
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pDpfProfile) {
    return (RET_INVALID_PARM);
  }

  if (NULL == ppDsp3DnrSetting) {
    return (RET_INVALID_PARM);
  }

  /* search EC scheme by index */
  *ppDsp3DnrSetting = (CamDsp3DNRSettingProfile_t*)ListGetItemByIdx(&pDpfProfile->Dsp3DNRSettingProfileList, idx);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbAddDpccProfile
 *****************************************************************************/
RESULT CamCalibDbAddDpccProfile
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamDpccProfile_t*    pAddDpcc
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamDpccProfile_t* pNewDpcc = NULL;

  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateDpccProfile(pAddDpcc);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if resolution already exists */
  pNewDpcc = (CamDpccProfile_t*)ListSearch(&pCamCalibDbCtx->dpcc_profile, SearchForEqualDpccProfile, (void*)pAddDpcc);
  if (NULL == pNewDpcc) {
    pNewDpcc = (CamDpccProfile_t*)malloc(sizeof(CamDpccProfile_t));
    MEMCPY(pNewDpcc, pAddDpcc, sizeof(CamDpccProfile_t));

    ListPrepareItem(pNewDpcc);
    ListAddTail(&pCamCalibDbCtx->dpcc_profile, pNewDpcc);
  } else {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetDpccProfileByName
 *****************************************************************************/
RESULT CamCalibDbGetDpccfProfileByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamDpccProfileName_t    name,
    CamDpccProfile_t**        pDpccProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pDpccProfile) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pDpccProfile = (CamDpccProfile_t*)ListSearch(&pCamCalibDbCtx->dpcc_profile, SearchDpccProfileByName, (void*)name);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetDpccProfileByResolution
 *****************************************************************************/
RESULT CamCalibDbGetDpccProfileByResolution
(
    CamCalibDbHandle_t          hCamCalibDb,
    const CamResolutionName_t   ResName,
    CamDpccProfile_t**             pDpccProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pDpccProfile) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pDpccProfile = (CamDpccProfile_t*)ListSearch(&pCamCalibDbCtx->dpcc_profile, SearchDpccProfileByResolution, (void*)ResName);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbGetSensorXmlVersion
 *****************************************************************************/
RESULT CamCalibDbGetSensorXmlVersion
(
    CamCalibDbHandle_t          hCamCalibDb,
    char (*pVersion)[64]
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if ((NULL == pVersion) || (NULL == *pVersion)) {
    return (RET_INVALID_PARM);
  }

  snprintf(*pVersion, 63, "%s_%s_%s_%s_%s", pCamCalibDbCtx->cdate, pCamCalibDbCtx->cname, pCamCalibDbCtx->sname, pCamCalibDbCtx->sid, pCamCalibDbCtx->cversion);
  return (RET_SUCCESS);
}

RESULT CamCalibDbGetAecPoint
(
    CamCalibDbHandle_t          hCamCalibDb,
    float* point
) {
  RESULT result;

  CamCalibAecGlobal_t* pAecGlobal;
  result =  CamCalibDbGetAecGlobal(hCamCalibDb, &pAecGlobal);
  if (RET_SUCCESS != result)
    return result;
  *point = pAecGlobal->SetPoint;

  return (RET_SUCCESS);
}

/******************************************************************************
 * ValidateAecGlobalData
 *****************************************************************************/
static inline RESULT ValidateGocProfileData(CamCalibGocProfile_t* pGocData) {
  LOGV("%s (enter)\n", __func__);

  if (NULL == pGocData) {
    return (RET_NULL_POINTER);
  }

  if (!strlen(pGocData->name)) {
    return (RET_INVALID_PARM);
  }
  
  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbAddGocProfile
 *****************************************************************************/
RESULT CamCalibDbAddGocProfile
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamCalibGocProfile_t* pAddGocProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamCalibGocProfile_t* pNewGoc = NULL;
  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateGocProfileData(pAddGocProfile);
  if (result != RET_SUCCESS) {
    return (result);
  }
  
  pNewGoc = (CamCalibGocProfile_t*)ListSearch(&pCamCalibDbCtx->gocProfile, SearchForEqualGocProfile, (void*)pAddGocProfile);
  if (NULL == pNewGoc) {
   pNewGoc = (CamCalibGocProfile_t*)malloc(sizeof(CamCalibGocProfile_t));
   if(pNewGoc != NULL){
     MEMCPY(pNewGoc, pAddGocProfile, sizeof(CamCalibGocProfile_t));
     ListPrepareItem(pNewGoc);
     ListAddTail(&pCamCalibDbCtx->gocProfile, pNewGoc);
   }else{
     ALOGE("%s malloc fail\n", __func__);
	 return (RET_INVALID_PARM);
   }
  } else {
   return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbGetNoOfGocProfile
 *****************************************************************************/
RESULT CamCalibDbGetNoOfGocProfile
(
    CamCalibDbHandle_t  hCamCalibDb,
    int32_t*             no
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == no) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *no = (uint32_t)ListNoItems(&pCamCalibDbCtx->gocProfile);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbGetGocProfileByName
 *****************************************************************************/
RESULT CamCalibDbGetGocProfileByName
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamGOCProfileName_t name,
    CamCalibGocProfile_t** ppGocProfile
){
   CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
 
   LOGV("%s (enter)\n", __func__);
 
   if (NULL == pCamCalibDbCtx) {
 	return (RET_WRONG_HANDLE);
   }
 
   if (NULL == ppGocProfile) {
 	return (RET_INVALID_PARM);
   }
 
   /* search resolution by name */
   *ppGocProfile = (CamCalibGocProfile_t*)ListSearch(&pCamCalibDbCtx->gocProfile, SearchGocProfileByName, (void*)name);
 
   LOGV("%s (exit)\n", __func__);
 
   return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbGetIlluminationByIdx
 *****************************************************************************/
RESULT CamCalibDbGetGocProfileByIdx
(
    CamCalibDbHandle_t  hCamCalibDb,
    const uint32_t      idx,
    CamCalibGocProfile_t** ppGocProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == ppGocProfile) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *ppGocProfile = (CamCalibGocProfile_t*)ListGetItemByIdx(&pCamCalibDbCtx->gocProfile, idx);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * ValidateAecGlobalData
 *****************************************************************************/
static inline RESULT ValidateWdrGlobalData(CamCalibWdrGlobal_t* pWdrData) {
  LOGV("%s (enter)\n", __func__);

  if (NULL == pWdrData) {
    return (RET_NULL_POINTER);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbAddAecGlobal
 *****************************************************************************/
RESULT CamCalibDbAddWdrGlobal
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamCalibWdrGlobal_t* pAddWdrGlobal
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateWdrGlobalData(pAddWdrGlobal);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if data already exists */
  if (NULL != pCamCalibDbCtx->pWdrGlobal) {
    return (RET_INVALID_PARM);
  }

  /* finally allocate, copy & add data */
  CamCalibWdrGlobal_t* pNewWdrGlobal = malloc(sizeof(CamCalibWdrGlobal_t));
  if (NULL == pNewWdrGlobal) {
    return (RET_OUTOFMEM);
  }
  MEMCPY(pNewWdrGlobal, pAddWdrGlobal, sizeof(CamCalibWdrGlobal_t));

  pCamCalibDbCtx->pWdrGlobal = pNewWdrGlobal;

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbGetAecGlobal
 *****************************************************************************/
RESULT CamCalibDbGetWdrGlobal
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamCalibWdrGlobal_t** ppWdrGlobal
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == ppWdrGlobal) {
    return (RET_INVALID_PARM);
  }

  /* return reference to global AEC configuration */
  *ppWdrGlobal = pCamCalibDbCtx->pWdrGlobal;

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbAddCproc
 *****************************************************************************/
RESULT CamCalibDbAddCproc
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamCprocProfile_t*   pAddCproc
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  /* check if data already exists */
  if (NULL != pCamCalibDbCtx->pCprocGlobal) {
    return (RET_INVALID_PARM);
  }

  /* finally allocate, copy & add data */
  CamCprocProfile_t* pNewCprocGlobal = malloc(sizeof(CamCprocProfile_t));
  if (NULL == pNewCprocGlobal) {
    return (RET_OUTOFMEM);
  }
  MEMCPY(pNewCprocGlobal, pAddCproc, sizeof(CamCprocProfile_t));

  pCamCalibDbCtx->pCprocGlobal = pNewCprocGlobal;

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

RESULT CamCalibDbGetCproc
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamCprocProfile_t**   ppAddCproc
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == ppAddCproc) {
    return (RET_INVALID_PARM);
  }

  /* return reference to global cproc configuration */
  *ppAddCproc = pCamCalibDbCtx->pCprocGlobal;

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


