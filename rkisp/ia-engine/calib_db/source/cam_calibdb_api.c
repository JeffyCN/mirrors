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
#include <base/xcam_log.h>

#include "cam_calibdb_api.h"
#include "cam_calibdb.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LOAD_IQ_TRACE_INFO_ON
// if defined, bin file should be included in following
// iq_xml_db array, and will be built into library
//#define USE_C_SOURCE_XML_BIN
#ifdef USE_C_SOURCE_XML_BIN
static const char iq_xml_db[] = {
    #include "imx258_GEIR180089_LG500627G.xml.db"
};
#endif

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
 * Where to store generated xml bin file
 *****************************************************************************/
static const char * GetXmlDbDir()
{
  return getenv("CAMERA_ENGINE_RKISP_XML_DB");
}


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
 * SearchResolutionByIdx
 *****************************************************************************/
static int SearchResolutionByIdx(List* l, void* key) {
    uint32_t id = *((uint32_t*)key);
    CamResolution_t* res = (CamResolution_t*)l;
    return ((res->id == id) ? 1 : 0);
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
static int SearchForEqualAwb_V11_Global(List* l, void* key) {
  CamCalibAwb_V11_Global_t* awb = (CamCalibAwb_V11_Global_t*)l;
  CamCalibAwb_V11_Global_t* k = (CamCalibAwb_V11_Global_t*)key;

  return ((!strncasecmp(awb->name, k->name, sizeof(k->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchForEqualAwb_V10_Global
 *****************************************************************************/
static int SearchForEqualAwb_V10_Global(List* l, void* key) {
  CamCalibAwb_V10_Global_t* awb = (CamCalibAwb_V10_Global_t*)l;
  CamCalibAwb_V10_Global_t* k = (CamCalibAwb_V10_Global_t*)key;

  return ((!strncasecmp(awb->name, k->name, sizeof(k->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchAwb_V11_GlobalByName
 *****************************************************************************/
static int SearchAwb_V11_GlobalByName(List* l, void* key) {
  CamCalibAwb_V11_Global_t* awb = (CamCalibAwb_V11_Global_t*)l;
  char* k = (char*)key;

  return ((!strncasecmp(awb->name, k, sizeof(awb->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchAwb_V10_GlobalByName
 *****************************************************************************/
static int SearchAwb_V10_GlobalByName(List* l, void* key) {
  CamCalibAwb_V10_Global_t* awb = (CamCalibAwb_V10_Global_t*)l;
  char* k = (char*)key;

  return ((!strncasecmp(awb->name, k, sizeof(awb->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchAwb_V11_GlobalByResolution
 *****************************************************************************/
static int SearchAwb_V11_GlobalByResolution(List* l, void* key) {
  CamCalibAwb_V11_Global_t* awb = (CamCalibAwb_V11_Global_t*)l;
  char* k = (char*)key;

  return ((!strncasecmp(awb->resolution, k, sizeof(awb->resolution))) ? 1 : 0);
}
/******************************************************************************
 * SearchAwb_V10_GlobalByResolution
 *****************************************************************************/
static int SearchAwb_V10_GlobalByResolution(List* l, void* key) {
  CamCalibAwb_V10_Global_t* awb = (CamCalibAwb_V10_Global_t*)l;
  char* k = (char*)key;

  return ((!strncasecmp(awb->resolution, k, sizeof(awb->resolution))) ? 1 : 0);
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
static int SearchForEqualAwb_V11_Illumination(List* l, void* key) {
  CamAwb_V11_IlluProfile_t* illu = (CamAwb_V11_IlluProfile_t*)l;
  CamAwb_V11_IlluProfile_t* k = (CamAwb_V11_IlluProfile_t*)key;

  return ((!strncmp(illu->name, k->name, sizeof(k->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchForEqualAwb_V10_Illumination
 *****************************************************************************/
static int SearchForEqualAwb_V10_Illumination(List* l, void* key) {
  CamAwb_V10_IlluProfile_t* illu = (CamAwb_V10_IlluProfile_t*)l;
  CamAwb_V10_IlluProfile_t* k = (CamAwb_V10_IlluProfile_t*)key;

  return ((!strncmp(illu->name, k->name, sizeof(k->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchAwb_V11_IlluminationByName
 *****************************************************************************/
static int SearchAwb_V11_IlluminationByName(List* l, void* key) {
  CamAwb_V11_IlluProfile_t* illu = (CamAwb_V11_IlluProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(illu->name, k, sizeof(illu->name))) ? 1 : 0);
}
/******************************************************************************
 * SearchAwb_V10_IlluminationByName
 *****************************************************************************/
static int SearchAwb_V10_IlluminationByName(List* l, void* key) {
  CamAwb_V10_IlluProfile_t* illu = (CamAwb_V10_IlluProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(illu->name, k, sizeof(illu->name))) ? 1 : 0);
}

/******************************************************************************
 * SearchForEqualIllumination
 *****************************************************************************/
static int SearchForEqualIllumination(List* l, void* key) {
  CamAwb_V10_IlluProfile_t* illu = (CamAwb_V10_IlluProfile_t*)l;
  CamAwb_V10_IlluProfile_t* k = (CamAwb_V10_IlluProfile_t*)key;

  return ((!strncmp(illu->name, k->name, sizeof(k->name))) ? 1 : 0);
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

  return ((!strncasecmp(lsc->name, k, sizeof(lsc->name))) ? 1 : 0);
}


/******************************************************************************
 * SearchForEqualCcProfile
 *****************************************************************************/
static int SearchForEqualCcProfile(List* l, void* key) {
  CamCcProfile_t* cc = (CamCcProfile_t*)l;
  CamCcProfile_t* k = (CamCcProfile_t*)key;

  return ((!strncasecmp(cc->name, k->name, sizeof(k->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchCcProfileByName
 *****************************************************************************/
static int SearchCcProfileByName(List* l, void* key) {
  CamCcProfile_t* cc = (CamCcProfile_t*)l;
  char* k = (char*)key;

  return ((!strncasecmp(cc->name, k, sizeof(cc->name))) ? 1 : 0);
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

  return ((!strncasecmp(dpf->name, k, sizeof(dpf->name))) ? 1 : 0);
}



/******************************************************************************
 * SearchDpfProfileByResolution
 *****************************************************************************/
static int SearchDpfProfileByResolution(List* l, void* key) {
  CamDpfProfile_t* dpf = (CamDpfProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(dpf->resolution, k, sizeof(dpf->resolution))) ? 1 : 0);
}

static int SearchForEqualNewDsp3DNRSetting(List* l, void* key) {
  CamNewDsp3DNRProfile_t* ecm = (CamNewDsp3DNRProfile_t*)l;
  CamNewDsp3DNRProfile_t* k   = (CamNewDsp3DNRProfile_t*)key;

  return ((!strncmp(ecm->name, k->name, sizeof(k->name))) ? 1 : 0);
}

static int SearchNewDsp3DNRSettingByName(List* l, void* key) {
  CamNewDsp3DNRProfile_t* dsp3DNR = (CamNewDsp3DNRProfile_t*)l;
  char* k = (char*)key;

  return ((!strncmp(dsp3DNR->name, k, sizeof(dsp3DNR->name))) ? 1 : 0);
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

  return ((!strncasecmp(filter->name, k, sizeof(filter->name))) ? 1 : 0);
}
static int SearchForEqualDySetpointProfile(List* l, void* key) {
  CamCalibAecDynamicSetpoint_t* pDySetpoint = (CamCalibAecDynamicSetpoint_t*)l;
  CamCalibAecDynamicSetpoint_t* k   = (CamCalibAecDynamicSetpoint_t*)key;

  return ((!strncmp(pDySetpoint->name, k->name, sizeof(k->name))) ? 1 : 0);
}

static int SearchDySetpointProfileByName(List* l, void* key) {
  CamCalibAecDynamicSetpoint_t* pDySetpoint = (CamCalibAecDynamicSetpoint_t*)l;
  char* k = (char*)key;

  return ((!strncmp(pDySetpoint->name, k, sizeof(pDySetpoint->name))) ? 1 : 0);
}

static int SearchForEqualExpSeparateProfile(List* l, void* key) {
  CamCalibAecExpSeparate_t* pExpSeparate = (CamCalibAecExpSeparate_t*)l;
  CamCalibAecExpSeparate_t* k   = (CamCalibAecExpSeparate_t*)key;

  return ((!strncmp(pExpSeparate->name, k->name, sizeof(k->name))) ? 1 : 0);
}

static int SearchExpSeparateProfileByName(List* l, void* key) {
  CamCalibAecExpSeparate_t* pExpSeparate = (CamCalibAecExpSeparate_t*)l;
  char* k = (char*)key;

  return ((!strncmp(pExpSeparate->name, k, sizeof(pExpSeparate->name))) ? 1 : 0);
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

  return ((!strncasecmp(dpf->name, k, sizeof(dpf->name))) ? 1 : 0);
}

static void ReplaceLscProfile(List* l, void* content) {
  CamLscProfile_t* oldpfl = (CamLscProfile_t*)l;
  CamLscProfile_t* newpfl = (CamLscProfile_t*)content;

  oldpfl->LscSectors = newpfl->LscSectors;
  oldpfl->LscNo = newpfl->LscNo;
  oldpfl->LscXo = newpfl->LscXo;
  oldpfl->LscYo = newpfl->LscYo;
  memcpy(&oldpfl->LscXSizeTbl[0], &newpfl->LscXSizeTbl[0], sizeof(newpfl->LscXSizeTbl));
  memcpy(&oldpfl->LscYSizeTbl[0], &newpfl->LscYSizeTbl[0], sizeof(newpfl->LscYSizeTbl));
  memcpy(&oldpfl->LscMatrix[0], &newpfl->LscMatrix[0], sizeof(newpfl->LscMatrix));
}

/******************************************************************************
 ******************************************************************************
 * SearchForEqualIesharpenProfile
 *****************************************************************************/
static int SearchForEqualIesharpenProfile( List *l, void *key )
{
    CamIesharpenProfile_t *dpf = (CamIesharpenProfile_t *)l;
    CamIesharpenProfile_t *k = (CamIesharpenProfile_t *)key;

    return ( (!strncmp(dpf->name, k->name, sizeof(k->name))) ? 1 : 0 );
}
/******************************************************************************
 * SearchIesharpenProfileByName
 *****************************************************************************/
static int SearchIesharpenProfileByName( List *l, void *key )
{
    CamIesharpenProfile_t *iesharpen = (CamIesharpenProfile_t *)l;
    char *k = (char *)key;

    return ( (!strncmp(iesharpen->name, k, sizeof(iesharpen->name))) ? 1 : 0 );
}
/******************************************************************************
 * SearchIesharpenProfileByResolution
 *****************************************************************************/
static int SearchIesharpenProfileByResolution( List *l, void *key )
{
    CamIesharpenProfile_t *iesharpen = (CamIesharpenProfile_t*)l;
    char *k = (char *)key;

    return ( (!strncmp(iesharpen->resolution, k, sizeof(iesharpen->resolution))) ? 1 : 0 );
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
static void ClearAwb_V10_GlobalList(List* l) {
  if (!ListEmpty(l)) {
    CamCalibAwb_V10_Global_t* pAwbGlobal = (CamCalibAwb_V10_Global_t*)ListRemoveHead(l);
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
      pAwbGlobal = (CamCalibAwb_V10_Global_t*)ListRemoveHead(l);
    }
  }

  ListInit(l);
}



/******************************************************************************
 * ClearAwb_V11_GlobalList
 *****************************************************************************/
static void ClearAwb_V11_GlobalList(List* l) {
  if (!ListEmpty(l)) {
    CamCalibAwb_V11_Global_t* pAwbGlobal = (CamCalibAwb_V11_Global_t*)ListRemoveHead(l);
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
      free(pAwbGlobal->AwbFade2Parm.pMaxCSum_br);
      free(pAwbGlobal->AwbFade2Parm.pMaxCSum_sr);
      free(pAwbGlobal->AwbFade2Parm.pMinC_br);
      free(pAwbGlobal->AwbFade2Parm.pMaxY_br);
      free(pAwbGlobal->AwbFade2Parm.pMinY_br);
      free(pAwbGlobal->AwbFade2Parm.pMinC_sr);
      free(pAwbGlobal->AwbFade2Parm.pMaxY_sr);
      free(pAwbGlobal->AwbFade2Parm.pMinY_sr);
      free(pAwbGlobal->AwbFade2Parm.pRefCb);
      free(pAwbGlobal->AwbFade2Parm.pRefCr);


      /* 2.) free AWB globals */
      free(pAwbGlobal);

      /* 3.) get next illumination */
      pAwbGlobal = (CamCalibAwb_V11_Global_t*)ListRemoveHead(l);
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
static void ClearAwb_V11_IlluminationList(List* l) {
  if (!ListEmpty(l)) {
    CamAwb_V11_IlluProfile_t* pIllumination = (CamAwb_V11_IlluProfile_t*)ListRemoveHead(l);
    while (pIllumination) {
      /* 1.) free sub structures of illumination */
      free(pIllumination->SaturationCurve.pSensorGain);
      free(pIllumination->SaturationCurve.pSaturation);

      free(pIllumination->VignettingCurve.pSensorGain);
      free(pIllumination->VignettingCurve.pVignetting);

      /* 2.) free illumination */
      free(pIllumination);

      /* 3.) get next illumination */
      pIllumination = (CamAwb_V11_IlluProfile_t*)ListRemoveHead(l);
    }
  }

  ListInit(l);
}

/******************************************************************************
 * ClearAwb_V10_IlluminationList
 *****************************************************************************/
static void ClearAwb_V10_IlluminationList(List* l) {
  if (!ListEmpty(l)) {
    CamAwb_V10_IlluProfile_t* pIllumination = (CamAwb_V10_IlluProfile_t*)ListRemoveHead(l);
    while (pIllumination) {
      /* 1.) free sub structures of illumination */
      free(pIllumination->SaturationCurve.pSensorGain);
      free(pIllumination->SaturationCurve.pSaturation);

      free(pIllumination->VignettingCurve.pSensorGain);
      free(pIllumination->VignettingCurve.pVignetting);

      /* 2.) free illumination */
      free(pIllumination);

      /* 3.) get next illumination */
      pIllumination = (CamAwb_V10_IlluProfile_t*)ListRemoveHead(l);
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
static void ClearNewDsp3DNRList(List* l) {
  if (!ListEmpty(l)) {
    CamNewDsp3DNRProfile_t * pNewDsp3DNR = (CamNewDsp3DNRProfile_t*)ListRemoveHead(l);
    while (pNewDsp3DNR) {
	  if(pNewDsp3DNR->pgain_Level){
		free(pNewDsp3DNR->pgain_Level);
  	  }

	  if(pNewDsp3DNR->ynr.pynr_time_weight_level){
		free(pNewDsp3DNR->ynr.pynr_time_weight_level);
	  }

	  if(pNewDsp3DNR->ynr.pynr_spat_weight_level){
		free(pNewDsp3DNR->ynr.pynr_spat_weight_level);
	  }

	  if(pNewDsp3DNR->uvnr.puvnr_weight_level){
		free(pNewDsp3DNR->uvnr.puvnr_weight_level);
	  }

	  if(pNewDsp3DNR->sharp.psharp_weight_level){
		free(pNewDsp3DNR->sharp.psharp_weight_level);
	  }

	  free(pNewDsp3DNR);
	  /* 3.) get next item */
      pNewDsp3DNR = (CamNewDsp3DNRProfile_t*)ListRemoveHead(l);
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
static void ClearDemosicLP(CamDemosaicLpProfile_t *pDemosaicLp) {

	if(pDemosaicLp->lu_divided){
		free(pDemosaicLp->lu_divided);
	}

	if(pDemosaicLp->gainsArray){
		free(pDemosaicLp->gainsArray);
	}

	if(pDemosaicLp->diff_divided0){
		free(pDemosaicLp->diff_divided0);
	}
	if(pDemosaicLp->diff_divided1){
		free(pDemosaicLp->diff_divided1);
	}
	if(pDemosaicLp->diff_divided2){
		free(pDemosaicLp->diff_divided2);
	}
	if(pDemosaicLp->diff_divided3){
		free(pDemosaicLp->diff_divided3);
	}
	if(pDemosaicLp->diff_divided4){
		free(pDemosaicLp->diff_divided4);
	}

	if(pDemosaicLp->thCSC_divided0){
		free(pDemosaicLp->thCSC_divided0);
	}
	if(pDemosaicLp->thCSC_divided1){
		free(pDemosaicLp->thCSC_divided1);
	}
	if(pDemosaicLp->thCSC_divided2){
		free(pDemosaicLp->thCSC_divided2);
	}
	if(pDemosaicLp->thCSC_divided3){
		free(pDemosaicLp->thCSC_divided3);
	}
	if(pDemosaicLp->thCSC_divided4){
		free(pDemosaicLp->thCSC_divided4);
	}

	if(pDemosaicLp->thH_divided0){
		free(pDemosaicLp->thH_divided0);
	}
	if(pDemosaicLp->thH_divided1){
		free(pDemosaicLp->thH_divided1);
	}
	if(pDemosaicLp->thH_divided2){
		free(pDemosaicLp->thH_divided2);
	}
	if(pDemosaicLp->thH_divided3){
		free(pDemosaicLp->thH_divided3);
	}
	if(pDemosaicLp->thH_divided4){
		free(pDemosaicLp->thH_divided4);
	}

	if(pDemosaicLp->varTh_divided0){
		free(pDemosaicLp->varTh_divided0);
	}
	if(pDemosaicLp->varTh_divided1){
		free(pDemosaicLp->varTh_divided1);
	}
	if(pDemosaicLp->varTh_divided2){
		free(pDemosaicLp->varTh_divided2);
	}
	if(pDemosaicLp->varTh_divided3){
		free(pDemosaicLp->varTh_divided3);
	}
	if(pDemosaicLp->varTh_divided4){
		free(pDemosaicLp->varTh_divided4);
	}

	if(pDemosaicLp->thdiff_b_fct){
		free(pDemosaicLp->thdiff_b_fct);
	}
	if(pDemosaicLp->thdiff_r_fct){
		free(pDemosaicLp->thdiff_r_fct);
	}
	if(pDemosaicLp->thgrad_b_fct){
		free(pDemosaicLp->thgrad_b_fct);
	}
	if(pDemosaicLp->thgrad_r_fct){
		free(pDemosaicLp->thgrad_r_fct);
	}
	if(pDemosaicLp->thvar_b_fct){
		free(pDemosaicLp->thvar_b_fct);
	}
	if(pDemosaicLp->thvar_r_fct){
		free(pDemosaicLp->thvar_r_fct);
	}

	if(pDemosaicLp->th_grad){
		free(pDemosaicLp->th_grad);
	}
	if(pDemosaicLp->th_diff){
		free(pDemosaicLp->th_diff);
	}
	if(pDemosaicLp->th_var){
		free(pDemosaicLp->th_var);
	}
	if(pDemosaicLp->th_csc){
		free(pDemosaicLp->th_csc);
	}

	if(pDemosaicLp->flat_level_sel){
		free(pDemosaicLp->flat_level_sel);
	}
	if(pDemosaicLp->pattern_level_sel){
		free(pDemosaicLp->pattern_level_sel);
	}
	if(pDemosaicLp->edge_level_sel){
		free(pDemosaicLp->edge_level_sel);
	}
	if(pDemosaicLp->similarity_th){
		free(pDemosaicLp->similarity_th);
	}

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

	  ClearDemosicLP(&pFilter->DemosaicLpConf);
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
	  ClearNewDsp3DNRList(&pDpfProfile->newDsp3DNRProfileList);
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

//werring.wu add iesharpen 2018 1 18
/******************************************************************************
 * ClearIesharpenProfileList
 *****************************************************************************/
static void ClearIesharpenProfileList(  List *l )
{
    if ( !ListEmpty( l ) )
    {
        CamIesharpenProfile_t *pIesharpenProfile = (CamIesharpenProfile_t*)ListRemoveHead( l );
        while ( pIesharpenProfile )
        {
            if(pIesharpenProfile->gauss_flat_coe!=NULL){
                free(pIesharpenProfile->gauss_flat_coe);
            }
            if(pIesharpenProfile->gauss_noise_coe!=NULL){
                free(pIesharpenProfile->gauss_noise_coe);
            }
            if(pIesharpenProfile->gauss_other_coe!=NULL){
                free(pIesharpenProfile->gauss_other_coe);
            }
            if(pIesharpenProfile->hgridconf.line1_filter_coe!=NULL){
                free(pIesharpenProfile->hgridconf.line1_filter_coe);
            }
            if(pIesharpenProfile->hgridconf.line2_filter_coe != NULL){
                free(pIesharpenProfile->hgridconf.line2_filter_coe);
            }
            if(pIesharpenProfile->hgridconf.line3_filter_coe!=NULL){
                free(pIesharpenProfile->hgridconf.line3_filter_coe);
            }
            if(pIesharpenProfile->hgridconf.p_grad!=NULL){
                free(pIesharpenProfile->hgridconf.p_grad);
            }
            if(pIesharpenProfile->hgridconf.sharp_factor!=NULL){
                free(pIesharpenProfile->hgridconf.sharp_factor);
            }
            if(pIesharpenProfile->lgridconf.line1_filter_coe!=NULL){
                free(pIesharpenProfile->lgridconf.line1_filter_coe);
            }
            if(pIesharpenProfile->lgridconf.line2_filter_coe!=NULL){
                free(pIesharpenProfile->lgridconf.line2_filter_coe);
            }
            if(pIesharpenProfile->lgridconf.line3_filter_coe!=NULL){
                free(pIesharpenProfile->lgridconf.line3_filter_coe);
            }
            if(pIesharpenProfile->lgridconf.p_grad!=NULL){
                free(pIesharpenProfile->lgridconf.p_grad);
            }
            if(pIesharpenProfile->lgridconf.sharp_factor!=NULL){
                free(pIesharpenProfile->lgridconf.sharp_factor);
            }
            if(pIesharpenProfile->pmaxnumber!=NULL){
                free(pIesharpenProfile->pmaxnumber);
            }
            if(pIesharpenProfile->pminnumber!=NULL){
                free(pIesharpenProfile->pminnumber);
            }
            if(pIesharpenProfile->P_delta1!=NULL){
                free(pIesharpenProfile->P_delta1);
            }
            if(pIesharpenProfile->P_delta2!=NULL){
                free(pIesharpenProfile->P_delta2);
            }
            if(pIesharpenProfile->uv_gauss_flat_coe!=NULL){
                free(pIesharpenProfile->uv_gauss_flat_coe);
            }
            if(pIesharpenProfile->uv_gauss_noise_coe!=NULL){
                free(pIesharpenProfile->uv_gauss_noise_coe);
            }
            if(pIesharpenProfile->uv_gauss_other_coe!=NULL){
                free(pIesharpenProfile->uv_gauss_other_coe);
            }
            if(pIesharpenProfile->yavg_thr!=NULL){
                free(pIesharpenProfile->yavg_thr);
            }

			if(pIesharpenProfile->hgridconf.lap_mat_coe){
				free(pIesharpenProfile->hgridconf.lap_mat_coe);
			}
			if(pIesharpenProfile->lgridconf.lap_mat_coe){
				free(pIesharpenProfile->lgridconf.lap_mat_coe);
			}

            free( pIesharpenProfile );
            pIesharpenProfile = (CamIesharpenProfile_t *)ListRemoveHead( l );
        }
    }

    ListInit( l );
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
static inline RESULT ValidateAwb_V11_Data(CamCalibAwb_V11_Global_t* pAwbData) {
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
 * ValidateAwb_V10_Data
 *****************************************************************************/
static inline RESULT ValidateAwb_V10_Data(CamCalibAwb_V10_Global_t* pAwbData) {
  LOGV( "%s (enter)\n", __func__);

  if (NULL == pAwbData) {
    return (RET_NULL_POINTER);
  }

  if ((!strlen(pAwbData->name))
      || (!strlen(pAwbData->resolution))) {
    return (RET_INVALID_PARM);
  }

  LOGV( "%s (exit)\n", __func__);

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
 * ValidateEcmScheme
 *****************************************************************************/
static inline RESULT ValidateDySetpoint(CamCalibAecDynamicSetpoint_t* pDySetpoint) {
  LOGV( "%s (enter)\n", __func__);

  if (NULL == pDySetpoint) {
    return (RET_NULL_POINTER);
  }

  if (!strlen(pDySetpoint->name)) {
    return (RET_INVALID_PARM);
  }

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * ValidateEcmScheme
 *****************************************************************************/
static inline RESULT ValidateExpSeparate(CamCalibAecExpSeparate_t* pExpSeparate) {
  LOGV( "%s (enter)\n", __func__);

  if (NULL == pExpSeparate) {
    return (RET_NULL_POINTER);
  }

  if (!strlen(pExpSeparate->name)) {
    return (RET_INVALID_PARM);
  }

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * ValidateAwb_V11_Illumination
 *****************************************************************************/
static inline RESULT ValidateAwb_V11_Illumination(CamAwb_V11_IlluProfile_t* pIllumination) {
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
 * ValidateAwb_V10_Illumination
 *****************************************************************************/
static inline RESULT ValidateAwb_V10_Illumination(CamAwb_V10_IlluProfile_t* pIllumination) {
  LOGV( "%s (enter)\n", __func__);

  if (NULL == pIllumination) {
    return (RET_NULL_POINTER);
  }

  if (!strlen(pIllumination->name)) {
    return (RET_INVALID_PARM);
  }

  LOGV( "%s (exit)\n", __func__);

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

static inline RESULT ValidateNewDsp3dnrSetting(CamNewDsp3DNRProfile_t* pNewDsp3dnr) {
  LOGV( "%s (enter)\n", __func__);

  if (NULL == pNewDsp3dnr) {
    return (RET_NULL_POINTER);
  }

  if (!strlen(pNewDsp3dnr->name)) {
    return (RET_INVALID_PARM);
  }

  LOGV( "%s (exit)\n", __func__);

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

//werring.wu add 2018 1 18
/******************************************************************************
 * ValidateIesharpenProfile
 *****************************************************************************/
static inline RESULT ValidateIesharpenProfile( CamIesharpenProfile_t *pIesharpenProfile )
{
    LOGV("%s (enter)\n", __func__);

    if ( NULL == pIesharpenProfile )
    {
        return ( RET_NULL_POINTER );
    }

    if ( (!strlen( pIesharpenProfile->name ))
            || (!strlen( pIesharpenProfile->resolution )) )
    {
        return ( RET_INVALID_PARM );
    }

    LOGV("%s (exit)\n", __func__);

    return ( RET_SUCCESS );
}

/******************************************************************************
 * ValidateAecGlobalData
 *****************************************************************************/
static inline RESULT ValidateOTPGlobalData(CamOTPGlobal_t* pOTPData) {
  LOGV("%s (enter)\n", __func__);

  if (NULL == pOTPData) {
    return (RET_NULL_POINTER);
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
  CamCalibAwbPara_t*          pAwbProfile_test=pCamCalibDbCtx->pAwbProfile;     /* AWB  profile*/
  CamAwbPara_V11_t           Para_V11_test =pAwbProfile_test->Para_V11;
  ClearAwb_V11_GlobalList(&pCamCalibDbCtx->pAwbProfile->Para_V11.awb_global);
  ClearAwb_V10_GlobalList(&pCamCalibDbCtx->pAwbProfile->Para_V10.awb_global);
  if (pCamCalibDbCtx->pAfGlobal) {
    if(pCamCalibDbCtx->pAfGlobal->contrast_af.FullRangeTbl!= NULL){
		free(pCamCalibDbCtx->pAfGlobal->contrast_af.FullRangeTbl);
	}
    if(pCamCalibDbCtx->pAfGlobal->contrast_af.AdaptRangeTbl!= NULL){
		free(pCamCalibDbCtx->pAfGlobal->contrast_af.AdaptRangeTbl);
	}
  	free(pCamCalibDbCtx->pAfGlobal);
    pCamCalibDbCtx->pAfGlobal = NULL;
  }
  if (pCamCalibDbCtx->pAecGlobal) {
  	if(pCamCalibDbCtx->pAecGlobal->GainRange.pGainRange != NULL){
		free(pCamCalibDbCtx->pAecGlobal->GainRange.pGainRange);
	}
	if(pCamCalibDbCtx->pAecGlobal->GridWeights.pWeight){
		free(pCamCalibDbCtx->pAecGlobal->GridWeights.pWeight);
	}
	if(pCamCalibDbCtx->pAecGlobal->NightGridWeights.pWeight){
		free(pCamCalibDbCtx->pAecGlobal->NightGridWeights.pWeight);
	}
	ClearDySetpointList(&pCamCalibDbCtx->pAecGlobal->DySetpointList);
	ClearExpSeparateList(&pCamCalibDbCtx->pAecGlobal->ExpSeparateList);
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

  if(pCamCalibDbCtx->pOTPGlobal){
	free(pCamCalibDbCtx->pOTPGlobal);
  }

  ClearEcmProfileList(& pCamCalibDbCtx->ecm_profile);
  ClearAwb_V11_IlluminationList(&pCamCalibDbCtx->pAwbProfile->Para_V11.illumination);
  ClearAwb_V10_IlluminationList(&pCamCalibDbCtx->pAwbProfile->Para_V10.illumination);
  free(pCamCalibDbCtx->pAwbProfile);
  ClearLscProfileList(&pCamCalibDbCtx->lsc_profile);
  ClearCcProfileList(&pCamCalibDbCtx->cc_profile);
  ClearBlsProfileList(&pCamCalibDbCtx->bls_profile);
  ClearCacProfileList(&pCamCalibDbCtx->cac_profile);
  ClearDpfProfileList(&pCamCalibDbCtx->dpf_profile);
  ClearDpccProfileList(&pCamCalibDbCtx->dpcc_profile);
  ClearGocProfileList(&pCamCalibDbCtx->gocProfile);
  ClearIesharpenProfileList(&pCamCalibDbCtx->iesharpen_profile);
  MEMSET(pCamCalibDbCtx, 0, sizeof(CamCalibDbContext_t));

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

static void DumpFrameRateList(List* l, FILE* fp) {
  LOGD("%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamFrameRate_t* pFrameRate = (CamFrameRate_t*)ListHead(l);
    while (pFrameRate) {
      LOGD("%s: pFrameRate->p_next 0x%x\n", __FUNCTION__, pFrameRate->p_next);
      fwrite(pFrameRate, sizeof(CamFrameRate_t), 1, fp);

      pFrameRate = pFrameRate->p_next;
    }
  }
  LOGD("%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpResolutionList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamResolution_t* pResolution = (CamResolution_t*)ListHead(l);
    while (pResolution) {
      LOGD("%s: pResolution->p_next 0x%x\n", __FUNCTION__, pResolution->p_next);
      fwrite(pResolution, sizeof(CamResolution_t), 1, fp);
      DumpFrameRateList(&pResolution->framerates, fp);

      pResolution = pResolution->p_next;
    }
  }
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpAwb_V10_GlobalList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamCalibAwb_V10_Global_t* pAwbGlobal = (CamCalibAwb_V10_Global_t*)ListHead(l);
    while (pAwbGlobal) {
      fwrite(pAwbGlobal, sizeof(CamCalibAwb_V10_Global_t), 1, fp);

      fwrite(pAwbGlobal->AwbClipParam.pRg1, sizeof(float),
        pAwbGlobal->AwbClipParam.ArraySize1, fp);
      fwrite(pAwbGlobal->AwbClipParam.pMaxDist1, sizeof(float),
        pAwbGlobal->AwbClipParam.ArraySize1, fp);
      fwrite(pAwbGlobal->AwbClipParam.pRg2, sizeof(float),
        pAwbGlobal->AwbClipParam.ArraySize2, fp);
      fwrite(pAwbGlobal->AwbClipParam.pMaxDist2, sizeof(float),
        pAwbGlobal->AwbClipParam.ArraySize2, fp);

      fwrite(pAwbGlobal->AwbGlobalFadeParm.pGlobalFade1, sizeof(float),
        pAwbGlobal->AwbGlobalFadeParm.ArraySize1, fp);
      fwrite(pAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance1, sizeof(float),
        pAwbGlobal->AwbGlobalFadeParm.ArraySize1, fp);
      fwrite(pAwbGlobal->AwbGlobalFadeParm.pGlobalFade2, sizeof(float),
        pAwbGlobal->AwbGlobalFadeParm.ArraySize2, fp);
      fwrite(pAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance2, sizeof(float),
        pAwbGlobal->AwbGlobalFadeParm.ArraySize2, fp);

      fwrite(pAwbGlobal->AwbFade2Parm.pFade, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pCbMinRegionMax, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pCrMinRegionMax, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pMaxCSumRegionMax, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pCbMinRegionMin, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pCrMinRegionMin, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pMaxCSumRegionMin, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pMinCRegionMax, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pMinCRegionMin, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pMaxYRegionMax, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pMaxYRegionMin, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pMinYMaxGRegionMax, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pMinYMaxGRegionMin, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pRefCb, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pRefCr, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);

      /* 3.) get next illumination */
      pAwbGlobal = pAwbGlobal->p_next;
    }
  }
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpAwb_V11_GlobalList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamCalibAwb_V11_Global_t* pAwbGlobal = (CamCalibAwb_V11_Global_t*)ListHead(l);
    while (pAwbGlobal) {
      fwrite(pAwbGlobal, sizeof(CamCalibAwb_V11_Global_t), 1, fp);

      fwrite(pAwbGlobal->AwbClipParam.pRg1, sizeof(float),
        pAwbGlobal->AwbClipParam.ArraySize1, fp);
      fwrite(pAwbGlobal->AwbClipParam.pMaxDist1, sizeof(float),
        pAwbGlobal->AwbClipParam.ArraySize1, fp);
      fwrite(pAwbGlobal->AwbClipParam.pRg2, sizeof(float),
        pAwbGlobal->AwbClipParam.ArraySize2, fp);
      fwrite(pAwbGlobal->AwbClipParam.pMaxDist2, sizeof(float),
        pAwbGlobal->AwbClipParam.ArraySize2, fp);

      fwrite(pAwbGlobal->AwbGlobalFadeParm.pGlobalFade1, sizeof(float),
        pAwbGlobal->AwbGlobalFadeParm.ArraySize1, fp);
      fwrite(pAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance1, sizeof(float),
        pAwbGlobal->AwbGlobalFadeParm.ArraySize1, fp);
      fwrite(pAwbGlobal->AwbGlobalFadeParm.pGlobalFade2, sizeof(float),
        pAwbGlobal->AwbGlobalFadeParm.ArraySize2, fp);
      fwrite(pAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance2, sizeof(float),
        pAwbGlobal->AwbGlobalFadeParm.ArraySize2, fp);

      fwrite(pAwbGlobal->AwbFade2Parm.pFade, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);

      fwrite(pAwbGlobal->AwbFade2Parm.pMaxCSum_br, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pMaxCSum_sr, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pMinC_br, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pMinC_sr, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);

      fwrite(pAwbGlobal->AwbFade2Parm.pMaxY_br, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pMaxY_sr, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pMinY_br, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pMinY_sr, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);

      fwrite(pAwbGlobal->AwbFade2Parm.pRefCb, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);
      fwrite(pAwbGlobal->AwbFade2Parm.pRefCr, sizeof(float),
        pAwbGlobal->AwbFade2Parm.ArraySize, fp);

      pAwbGlobal = pAwbGlobal->p_next;
    }
  }

  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpEcmSchemeList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamEcmScheme_t* pEcmScheme = (CamEcmScheme_t*)ListHead(l);
    while (pEcmScheme) {
      fwrite(pEcmScheme, sizeof(CamEcmScheme_t), 1, fp);

      pEcmScheme = pEcmScheme->p_next;
    }
  }
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpEcmProfileList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamEcmProfile_t* pEcmProfile = (CamEcmProfile_t*)ListHead(l);
    while (pEcmProfile) {
      fwrite(pEcmProfile, sizeof(CamEcmProfile_t), 1, fp);
      DumpEcmSchemeList(&pEcmProfile->ecm_scheme, fp);

      pEcmProfile = pEcmProfile->p_next;
    }
  }
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpAwb_V10_IlluminationList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamAwb_V10_IlluProfile_t* pIllumination = (CamAwb_V10_IlluProfile_t*)ListHead(l);
    while (pIllumination) {
      fwrite(pIllumination, sizeof(CamAwb_V10_IlluProfile_t), 1, fp);

      fwrite(pIllumination->SaturationCurve.pSensorGain, sizeof(float),
        pIllumination->SaturationCurve.ArraySize, fp);
      fwrite(pIllumination->SaturationCurve.pSaturation, sizeof(float),
        pIllumination->SaturationCurve.ArraySize, fp);

      fwrite(pIllumination->VignettingCurve.pSensorGain, sizeof(float),
        pIllumination->VignettingCurve.ArraySize, fp);
      fwrite(pIllumination->VignettingCurve.pVignetting, sizeof(float),
        pIllumination->VignettingCurve.ArraySize, fp);

      pIllumination = pIllumination->p_next;
    }
  }
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpAwb_V11_IlluminationList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamAwb_V11_IlluProfile_t* pIllumination = (CamAwb_V11_IlluProfile_t*)ListHead(l);
    while (pIllumination) {
      fwrite(pIllumination, sizeof(CamAwb_V11_IlluProfile_t), 1, fp);

      fwrite(pIllumination->SaturationCurve.pSensorGain, sizeof(float),
        pIllumination->SaturationCurve.ArraySize, fp);
      fwrite(pIllumination->SaturationCurve.pSaturation, sizeof(float),
        pIllumination->SaturationCurve.ArraySize, fp);

      fwrite(pIllumination->VignettingCurve.pSensorGain, sizeof(float),
        pIllumination->VignettingCurve.ArraySize, fp);
      fwrite(pIllumination->VignettingCurve.pVignetting, sizeof(float),
        pIllumination->VignettingCurve.ArraySize, fp);

      pIllumination = pIllumination->p_next;
    }
  }

  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpLscProfileList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamLscProfile_t* pLscProfile = (CamLscProfile_t*)ListHead(l);
    while (pLscProfile) {
      fwrite(pLscProfile, sizeof(CamLscProfile_t), 1, fp);
      pLscProfile = pLscProfile->p_next;
    }
  }
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpCcProfileList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamCcProfile_t* pCcProfile = (CamCcProfile_t*)ListHead(l);
    while (pCcProfile) {
      fwrite(pCcProfile, sizeof(CamCcProfile_t), 1, fp);
      pCcProfile = pCcProfile->p_next;
    }
  }
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpBlsProfileList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamBlsProfile_t* pBlsProfile = (CamBlsProfile_t*)ListHead(l);
    while (pBlsProfile) {
      fwrite(pBlsProfile, sizeof(CamBlsProfile_t), 1, fp);
      pBlsProfile = pBlsProfile->p_next;
    }
  }
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpCacProfileList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamCacProfile_t* pCacProfile = (CamCacProfile_t*)ListHead(l);
    while (pCacProfile) {
      fwrite(pCacProfile, sizeof(CamCacProfile_t), 1, fp);
      pCacProfile = pCacProfile->p_next;
    }
  }
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpDsp3DNRList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamDsp3DNRSettingProfile_t * pDsp3DNR = (CamDsp3DNRSettingProfile_t*)ListHead(l);
    while (pDsp3DNR) {
          fwrite(pDsp3DNR, sizeof(CamDsp3DNRSettingProfile_t), 1, fp);

          if(pDsp3DNR->pgain_Level){
            fwrite(pDsp3DNR->pgain_Level, sizeof(float),
                   pDsp3DNR->ArraySize, fp);
          }
          if(pDsp3DNR->pnoise_coef_denominator){
            fwrite(pDsp3DNR->pnoise_coef_denominator, sizeof(uint16_t),
                   pDsp3DNR->ArraySize, fp);
          }
          if(pDsp3DNR->pnoise_coef_numerator){
            fwrite(pDsp3DNR->pnoise_coef_numerator, sizeof(uint16_t),
                   pDsp3DNR->ArraySize, fp);
          }
          if(pDsp3DNR->sDefaultLevelSetting.pchrm_sp_nr_level){
            fwrite(pDsp3DNR->sDefaultLevelSetting.pchrm_sp_nr_level, sizeof(unsigned char),
                   pDsp3DNR->ArraySize, fp);
          }
          if(pDsp3DNR->sDefaultLevelSetting.pchrm_te_nr_level){
            fwrite(pDsp3DNR->sDefaultLevelSetting.pchrm_te_nr_level, sizeof(unsigned char),
                   pDsp3DNR->ArraySize, fp);
          }
          if(pDsp3DNR->sDefaultLevelSetting.pluma_sp_nr_level){
            fwrite(pDsp3DNR->sDefaultLevelSetting.pluma_sp_nr_level, sizeof(unsigned char),
                   pDsp3DNR->ArraySize, fp);
          }
          if(pDsp3DNR->sDefaultLevelSetting.pluma_te_nr_level){
            fwrite(pDsp3DNR->sDefaultLevelSetting.pluma_te_nr_level, sizeof(unsigned char),
                   pDsp3DNR->ArraySize, fp);
          }
          if(pDsp3DNR->sDefaultLevelSetting.pshp_level){
            fwrite(pDsp3DNR->sDefaultLevelSetting.pshp_level, sizeof(unsigned char),
                   pDsp3DNR->ArraySize, fp);
          }

          if(pDsp3DNR->sLumaSetting.pluma_sp_rad){
            fwrite(pDsp3DNR->sLumaSetting.pluma_sp_rad, sizeof(unsigned char),
                   pDsp3DNR->ArraySize, fp);
          }
          if(pDsp3DNR->sLumaSetting.pluma_te_max_bi_num){
            fwrite(pDsp3DNR->sLumaSetting.pluma_te_max_bi_num, sizeof(unsigned char),
                   pDsp3DNR->ArraySize, fp);
          }

          if(pDsp3DNR->sChrmSetting.pchrm_sp_rad){
            fwrite(pDsp3DNR->sChrmSetting.pchrm_sp_rad, sizeof(unsigned char),
                   pDsp3DNR->ArraySize, fp);
          }
          if(pDsp3DNR->sChrmSetting.pchrm_te_max_bi_num){
            fwrite(pDsp3DNR->sChrmSetting.pchrm_te_max_bi_num, sizeof(unsigned char),
                   pDsp3DNR->ArraySize, fp);
          }

          if(pDsp3DNR->sSharpSetting.psrc_shp_c){
            fwrite(pDsp3DNR->sSharpSetting.psrc_shp_c, sizeof(unsigned char),
                   pDsp3DNR->ArraySize, fp);
          }
          if(pDsp3DNR->sSharpSetting.psrc_shp_div){
            fwrite(pDsp3DNR->sSharpSetting.psrc_shp_div, sizeof(unsigned char),
                   pDsp3DNR->ArraySize, fp);
          }
          if(pDsp3DNR->sSharpSetting.psrc_shp_l){
            fwrite(pDsp3DNR->sSharpSetting.psrc_shp_l, sizeof(unsigned char),
                   pDsp3DNR->ArraySize, fp);
          }
          if(pDsp3DNR->sSharpSetting.psrc_shp_thr){
            fwrite(pDsp3DNR->sSharpSetting.psrc_shp_thr, sizeof(unsigned char),
                   pDsp3DNR->ArraySize, fp);
          }

          for(int i=0; i<CAM_CALIBDB_3DNR_WEIGHT_NUM; i++){
            if(pDsp3DNR->sLumaSetting.pluma_weight[i]){
              fwrite(pDsp3DNR->sLumaSetting.pluma_weight[i], sizeof(uint8_t),
                     pDsp3DNR->ArraySize, fp);
            }

            if(pDsp3DNR->sChrmSetting.pchrm_weight[i]){
              fwrite(pDsp3DNR->sChrmSetting.pchrm_weight[i], sizeof(uint8_t),
                     pDsp3DNR->ArraySize, fp);
            }

            if(pDsp3DNR->sSharpSetting.psrc_shp_weight[i]){
              fwrite(pDsp3DNR->sSharpSetting.psrc_shp_weight[i], sizeof(int8_t),
                     pDsp3DNR->ArraySize, fp);
            }
          }

      pDsp3DNR = pDsp3DNR->p_next;
    }
  }
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpNewDsp3DNRList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamNewDsp3DNRProfile_t * pNewDsp3DNR = (CamNewDsp3DNRProfile_t*)ListHead(l);

    while (pNewDsp3DNR) {
      fwrite(pNewDsp3DNR, sizeof(CamNewDsp3DNRProfile_t), 1, fp);
      if(pNewDsp3DNR->pgain_Level){
        fwrite(pNewDsp3DNR->pgain_Level, sizeof(float),
           pNewDsp3DNR->ArraySize, fp);
      }

      if(pNewDsp3DNR->ynr.pynr_time_weight_level){
        fwrite(pNewDsp3DNR->ynr.pynr_time_weight_level, sizeof(uint32_t),
           pNewDsp3DNR->ArraySize, fp);
      }

      if(pNewDsp3DNR->ynr.pynr_spat_weight_level){
        fwrite(pNewDsp3DNR->ynr.pynr_spat_weight_level, sizeof(uint32_t),
           pNewDsp3DNR->ArraySize, fp);
      }

      if(pNewDsp3DNR->uvnr.puvnr_weight_level){
        fwrite(pNewDsp3DNR->uvnr.puvnr_weight_level, sizeof(uint32_t),
           pNewDsp3DNR->ArraySize, fp);
      }

      if(pNewDsp3DNR->sharp.psharp_weight_level){
        fwrite(pNewDsp3DNR->sharp.psharp_weight_level, sizeof(uint32_t),
           pNewDsp3DNR->ArraySize, fp);
      }

      /* 3.) get next item */
      pNewDsp3DNR = pNewDsp3DNR->p_next;
    }
  }
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpFilterList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamFilterProfile_t * pFilter = (CamFilterProfile_t*)ListHead(l);
    while (pFilter) {
      fwrite(pFilter, sizeof(CamFilterProfile_t), 1, fp);

      if(pFilter->DemosaicThCurve.pSensorGain){
        fwrite(pFilter->DemosaicThCurve.pSensorGain, sizeof(float),
               pFilter->DemosaicThCurve.ArraySize, fp);
      }
      if(pFilter->DemosaicThCurve.pThlevel){
        fwrite(pFilter->DemosaicThCurve.pThlevel, sizeof(uint8_t),
               pFilter->DemosaicThCurve.ArraySize, fp);
      }

      if(pFilter->DenoiseLevelCurve.pSensorGain){
        fwrite(pFilter->DenoiseLevelCurve.pSensorGain, sizeof(float),
               pFilter->DenoiseLevelCurve.ArraySize, fp);
      }
      if(pFilter->DenoiseLevelCurve.pDlevel){
        fwrite(pFilter->DenoiseLevelCurve.pDlevel, sizeof(CamerIcIspFltDeNoiseLevel_t),
               pFilter->DenoiseLevelCurve.ArraySize, fp);
      }

      if(pFilter->SharpeningLevelCurve.pSensorGain){
        fwrite(pFilter->SharpeningLevelCurve.pSensorGain, sizeof(float),
               pFilter->SharpeningLevelCurve.ArraySize, fp);
      }
      if(pFilter->SharpeningLevelCurve.pSlevel){
        fwrite(pFilter->SharpeningLevelCurve.pSlevel, sizeof(CamerIcIspFltSharpeningLevel_t),
               pFilter->SharpeningLevelCurve.ArraySize, fp);
      }

      if(pFilter->FiltLevelRegConf.p_chr_h_mode){
        fwrite(pFilter->FiltLevelRegConf.p_chr_h_mode, sizeof(uint8_t),
               pFilter->FiltLevelRegConf.ArraySize, fp);
      }
      if(pFilter->FiltLevelRegConf.p_chr_v_mode){
        fwrite(pFilter->FiltLevelRegConf.p_chr_v_mode, sizeof(uint8_t),
               pFilter->FiltLevelRegConf.ArraySize, fp);
      }
      if(pFilter->FiltLevelRegConf.p_fac_bl0){
        fwrite(pFilter->FiltLevelRegConf.p_fac_bl0, sizeof(uint32_t),
               pFilter->FiltLevelRegConf.ArraySize, fp);
      }
      if(pFilter->FiltLevelRegConf.p_fac_bl1){
        fwrite(pFilter->FiltLevelRegConf.p_fac_bl1, sizeof(uint32_t),
               pFilter->FiltLevelRegConf.ArraySize, fp);
      }
      if(pFilter->FiltLevelRegConf.p_fac_mid){
        fwrite(pFilter->FiltLevelRegConf.p_fac_mid, sizeof(uint32_t),
               pFilter->FiltLevelRegConf.ArraySize, fp);
      }
      if(pFilter->FiltLevelRegConf.p_fac_sh0){
        fwrite(pFilter->FiltLevelRegConf.p_fac_sh0, sizeof(uint32_t),
               pFilter->FiltLevelRegConf.ArraySize, fp);
      }
      if(pFilter->FiltLevelRegConf.p_fac_sh1){
        fwrite(pFilter->FiltLevelRegConf.p_fac_sh1, sizeof(uint32_t),
               pFilter->FiltLevelRegConf.ArraySize, fp);
      }
      if(pFilter->FiltLevelRegConf.p_FiltLevel){
        fwrite(pFilter->FiltLevelRegConf.p_FiltLevel, sizeof(uint8_t),
               pFilter->FiltLevelRegConf.ArraySize, fp);
      }
      if(pFilter->FiltLevelRegConf.p_grn_stage1){
        fwrite(pFilter->FiltLevelRegConf.p_grn_stage1, sizeof(uint8_t),
               pFilter->FiltLevelRegConf.ArraySize, fp);
      }
      if(pFilter->FiltLevelRegConf.p_thresh_bl0){
        fwrite(pFilter->FiltLevelRegConf.p_thresh_bl0, sizeof(uint32_t),
               pFilter->FiltLevelRegConf.ArraySize, fp);
      }
      if(pFilter->FiltLevelRegConf.p_thresh_bl1){
        fwrite(pFilter->FiltLevelRegConf.p_thresh_bl1, sizeof(uint32_t),
               pFilter->FiltLevelRegConf.ArraySize, fp);
      }
      if(pFilter->FiltLevelRegConf.p_thresh_sh0){
        fwrite(pFilter->FiltLevelRegConf.p_thresh_sh0, sizeof(uint8_t),
               pFilter->FiltLevelRegConf.ArraySize, fp);
      }
      if(pFilter->FiltLevelRegConf.p_thresh_sh1){
        fwrite(pFilter->FiltLevelRegConf.p_thresh_sh1, sizeof(uint32_t),
               pFilter->FiltLevelRegConf.ArraySize, fp);
      }

      pFilter = pFilter->p_next;
    }
  }
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpDpfProfileList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamDpfProfile_t* pDpfProfile = (CamDpfProfile_t*)ListHead(l);
    while (pDpfProfile) {
      fwrite(pDpfProfile, sizeof(CamDpfProfile_t), 1, fp);
      DumpDsp3DNRList(&pDpfProfile->Dsp3DNRSettingProfileList, fp);
      DumpNewDsp3DNRList(&pDpfProfile->newDsp3DNRProfileList, fp);
      DumpFilterList(&pDpfProfile->FilterList, fp);

      pDpfProfile = pDpfProfile->p_next;
    }
  }
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpDpccProfileList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamDpccProfile_t* pDpccProfile = (CamDpccProfile_t*)ListHead(l);
    while (pDpccProfile) {
      fwrite(pDpccProfile, sizeof(CamDpccProfile_t), 1, fp);
      pDpccProfile = pDpccProfile->p_next;
    }
  }
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpGocProfileList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamCalibGocProfile_t* pGocProfile = (CamCalibGocProfile_t*)ListHead(l);
    while (pGocProfile) {
      fwrite(pGocProfile, sizeof(CamCalibGocProfile_t), 1, fp);
      pGocProfile = pGocProfile->p_next;
    }
  }
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpIeSharpenProfileList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamIesharpenProfile_t* pIeSharpenProfile = (CamIesharpenProfile_t*)ListHead(l);
    while (pIeSharpenProfile) {
      fwrite(pIeSharpenProfile, sizeof(CamIesharpenProfile_t), 1, fp);
      if (pIeSharpenProfile->yavg_thr)
          fwrite(pIeSharpenProfile->yavg_thr, pIeSharpenProfile->yavg_thr_ArraySize * sizeof(uint8_t), 1, fp);
      if (pIeSharpenProfile->P_delta1)
          fwrite(pIeSharpenProfile->P_delta1, pIeSharpenProfile->P_delta1_ArraySize * sizeof(uint8_t), 1, fp);
      if (pIeSharpenProfile->P_delta2)
          fwrite(pIeSharpenProfile->P_delta2, pIeSharpenProfile->P_delta2_ArraySize * sizeof(uint8_t), 1, fp);
      if (pIeSharpenProfile->pmaxnumber)
          fwrite(pIeSharpenProfile->pmaxnumber, pIeSharpenProfile->pmaxnumber_ArraySize * sizeof(uint8_t), 1, fp);
      if (pIeSharpenProfile->pminnumber)
          fwrite(pIeSharpenProfile->pminnumber, pIeSharpenProfile->pminnumber_ArraySize * sizeof(uint8_t), 1, fp);
      if (pIeSharpenProfile->gauss_flat_coe)
          fwrite(pIeSharpenProfile->gauss_flat_coe, pIeSharpenProfile->gauss_flat_coe_ArraySize * sizeof(uint8_t), 1, fp);
      if (pIeSharpenProfile->gauss_noise_coe)
          fwrite(pIeSharpenProfile->gauss_noise_coe, pIeSharpenProfile->gauss_noise_coe_ArraySize * sizeof(uint8_t), 1, fp);
      if (pIeSharpenProfile->gauss_other_coe)
          fwrite(pIeSharpenProfile->gauss_other_coe, pIeSharpenProfile->gauss_other_coe_ArraySize * sizeof(uint8_t), 1, fp);
      if (pIeSharpenProfile->uv_gauss_flat_coe)
          fwrite(pIeSharpenProfile->uv_gauss_flat_coe, pIeSharpenProfile->uv_gauss_flat_coe_ArraySize * sizeof(uint8_t), 1, fp);
      if (pIeSharpenProfile->uv_gauss_noise_coe)
          fwrite(pIeSharpenProfile->uv_gauss_noise_coe, pIeSharpenProfile->uv_gauss_noise_coe_ArraySize * sizeof(uint8_t), 1, fp);
      if (pIeSharpenProfile->uv_gauss_other_coe)
          fwrite(pIeSharpenProfile->uv_gauss_other_coe, pIeSharpenProfile->uv_gauss_other_coe_ArraySize * sizeof(uint8_t), 1, fp);
      {
         // CamIesharpenGridConf_t
          if (pIeSharpenProfile->lgridconf.p_grad)
              fwrite(pIeSharpenProfile->lgridconf.p_grad, pIeSharpenProfile->lgridconf.p_grad_ArraySize * sizeof(uint16_t), 1, fp);
          if (pIeSharpenProfile->lgridconf.sharp_factor)
              fwrite(pIeSharpenProfile->lgridconf.sharp_factor, pIeSharpenProfile->lgridconf.sharp_factor_ArraySize * sizeof(uint8_t), 1, fp);
          if (pIeSharpenProfile->lgridconf.line1_filter_coe)
              fwrite(pIeSharpenProfile->lgridconf.line1_filter_coe, pIeSharpenProfile->lgridconf.line1_filter_coe_ArraySize * sizeof(uint8_t), 1, fp);
          if (pIeSharpenProfile->lgridconf.line2_filter_coe)
              fwrite(pIeSharpenProfile->lgridconf.line2_filter_coe, pIeSharpenProfile->lgridconf.line2_filter_coe_ArraySize * sizeof(uint8_t), 1, fp);
          if (pIeSharpenProfile->lgridconf.line3_filter_coe)
              fwrite(pIeSharpenProfile->lgridconf.line3_filter_coe, pIeSharpenProfile->lgridconf.line3_filter_coe_ArraySize * sizeof(uint8_t), 1, fp);
          if (pIeSharpenProfile->lgridconf.lap_mat_coe)
              fwrite(pIeSharpenProfile->lgridconf.lap_mat_coe, pIeSharpenProfile->lgridconf.lap_mat_coe_ArraySize * sizeof(uint8_t), 1, fp);

          if (pIeSharpenProfile->hgridconf.p_grad)
              fwrite(pIeSharpenProfile->hgridconf.p_grad, pIeSharpenProfile->hgridconf.p_grad_ArraySize * sizeof(uint16_t), 1, fp);
          if (pIeSharpenProfile->hgridconf.sharp_factor)
              fwrite(pIeSharpenProfile->hgridconf.sharp_factor, pIeSharpenProfile->hgridconf.sharp_factor_ArraySize * sizeof(uint8_t), 1, fp);
          if (pIeSharpenProfile->hgridconf.line1_filter_coe)
              fwrite(pIeSharpenProfile->hgridconf.line1_filter_coe, pIeSharpenProfile->hgridconf.line1_filter_coe_ArraySize * sizeof(uint8_t), 1, fp);
          if (pIeSharpenProfile->hgridconf.line2_filter_coe)
              fwrite(pIeSharpenProfile->hgridconf.line2_filter_coe, pIeSharpenProfile->hgridconf.line2_filter_coe_ArraySize * sizeof(uint8_t), 1, fp);
          if (pIeSharpenProfile->hgridconf.line3_filter_coe)
              fwrite(pIeSharpenProfile->hgridconf.line3_filter_coe, pIeSharpenProfile->hgridconf.line3_filter_coe_ArraySize * sizeof(uint8_t), 1, fp);
          if (pIeSharpenProfile->hgridconf.lap_mat_coe)
              fwrite(pIeSharpenProfile->hgridconf.lap_mat_coe, pIeSharpenProfile->hgridconf.lap_mat_coe_ArraySize * sizeof(uint8_t), 1, fp);
      }
      pIeSharpenProfile = pIeSharpenProfile->p_next;
    }
  }
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpDySetpointList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamCalibAecDynamicSetpoint_t* pDySetpoint = (CamCalibAecDynamicSetpoint_t*)ListHead(l);
    while (pDySetpoint) {
      fwrite(pDySetpoint, sizeof(CamCalibAecDynamicSetpoint_t), 1, fp);
      if(pDySetpoint->pDySetpoint != NULL)
        fwrite(pDySetpoint->pDySetpoint, sizeof(float),
               pDySetpoint->array_size, fp);

      if(pDySetpoint->pExpValue != NULL)
        fwrite(pDySetpoint->pExpValue, sizeof(float),
               pDySetpoint->array_size, fp);

      pDySetpoint = pDySetpoint->p_next;
    }
  }
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static void DumpExpSeparateList(List* l, FILE* fp) {
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, ftell(fp));
  if (!ListEmpty(l)) {
    CamCalibAecExpSeparate_t* pExpSeparate = (CamCalibAecExpSeparate_t*)ListHead(l);
    while (pExpSeparate) {
      fwrite(pExpSeparate, sizeof(CamCalibAecExpSeparate_t), 1, fp);

      pExpSeparate = pExpSeparate->p_next;
    }
  }
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, ftell(fp));
}

static RESULT DumpBin2CSource(const char *bin_path, const char *c_path) {
  FILE* fp_in = NULL;
  FILE* fp_out = NULL;
  int dump_size, i, dump_idx;
  unsigned char *pDumpBuf;

  fp_in = fopen(bin_path, "rb");
  if (!fp_in) {
    LOGE( "%s:open %s failed!!\n", __func__, bin_path);
    return RET_FAILURE;
  }

  fseek(fp_in, 0L, SEEK_END);
  dump_size = ftell(fp_in);
  fseek(fp_in, 0L, SEEK_SET);

  pDumpBuf = malloc(dump_size);
  if (pDumpBuf == NULL) {
    LOGE( "%s:malloc failed!!\n", __func__);
    return RET_FAILURE;
  }

  fread(pDumpBuf, dump_size, 1, fp_in);
  fclose(fp_in);

  fp_out = fopen(c_path, "w");
  if (!fp_out) {
    LOGE( "%s:open %s failed!!\n", __func__, c_path);
    return RET_FAILURE;
  }

  dump_idx = 0;
  while (dump_idx < dump_size) {
    fprintf(fp_out, "\n");
    for (i = 0; i < 16; i++) {
      if (dump_idx < dump_size)
        fprintf(fp_out, " 0x%02x,", pDumpBuf[dump_idx++]);
    }
  }
  fprintf(fp_out, "\n");

  fclose(fp_out);
  free(pDumpBuf);

  LOGD( "%s (exit)\n", __FUNCTION__);
  return (RET_SUCCESS);
}

RESULT CamCalibDbDumpFile
(
    CamCalibDbHandle_t  hCamCalibDb,
    const char *dump_path
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  RESULT result;
  FILE* fp = NULL;
  List* l;
  char xml_dump_bin_file[128]; 
  char xml_dump_db_file[128]; 
  char* xml_path_split;

  LOGD( "%s (enter)\n", __FUNCTION__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == GetXmlDbDir())
    return RET_FAILURE;

  xml_path_split = strrchr(dump_path, '/');

  sprintf(xml_dump_bin_file, "%s/%s.bin", GetXmlDbDir(), xml_path_split + 1);
  
  fp = fopen(xml_dump_bin_file, "wb");
  if (!fp) {
    LOGE( "%s:open %s failed %s!!\n", __func__, xml_dump_bin_file, strerror(errno));
    return RET_FAILURE;
  }

  fwrite(pCamCalibDbCtx, sizeof(CamCalibDbContext_t), 1, fp);

  DumpResolutionList(&pCamCalibDbCtx->resolution, fp);
  fwrite(pCamCalibDbCtx->pAwbProfile, sizeof(CamCalibAwbPara_t), 1, fp);
  LOGD( "%s:%d: file pos 0x%x\n", __FUNCTION__, __LINE__, ftell(fp));
  DumpAwb_V10_GlobalList(&pCamCalibDbCtx->pAwbProfile->Para_V10.awb_global, fp);
  DumpAwb_V10_IlluminationList(&pCamCalibDbCtx->pAwbProfile->Para_V10.illumination, fp);
  DumpAwb_V11_GlobalList(&pCamCalibDbCtx->pAwbProfile->Para_V11.awb_global, fp);
  DumpAwb_V11_IlluminationList(&pCamCalibDbCtx->pAwbProfile->Para_V11.illumination, fp);
  if (pCamCalibDbCtx->pAfGlobal) {
    fwrite(pCamCalibDbCtx->pAfGlobal, sizeof(CamCalibAfGlobal_t), 1, fp);
    if (pCamCalibDbCtx->pAfGlobal->contrast_af.FullSteps > 0) {
        fwrite(pCamCalibDbCtx->pAfGlobal->contrast_af.FullRangeTbl, sizeof(uint16_t),
               pCamCalibDbCtx->pAfGlobal->contrast_af.FullSteps, fp);
    }

    if (pCamCalibDbCtx->pAfGlobal->contrast_af.AdaptiveSteps > 0) {
        fwrite(pCamCalibDbCtx->pAfGlobal->contrast_af.AdaptRangeTbl, sizeof(uint16_t),
               pCamCalibDbCtx->pAfGlobal->contrast_af.AdaptiveSteps, fp);
    }
  }
  if (pCamCalibDbCtx->pAecGlobal) {
    fwrite(pCamCalibDbCtx->pAecGlobal, sizeof(CamCalibAecGlobal_t), 1, fp);
    if(pCamCalibDbCtx->pAecGlobal->GridWeights.pWeight != NULL){
       fwrite(pCamCalibDbCtx->pAecGlobal->GridWeights.pWeight, sizeof(uint8_t),
              pCamCalibDbCtx->pAecGlobal->GridWeights.ArraySize, fp);
    }
    if(pCamCalibDbCtx->pAecGlobal->NightGridWeights.pWeight != NULL){
       fwrite(pCamCalibDbCtx->pAecGlobal->NightGridWeights.pWeight, sizeof(uint8_t),
              pCamCalibDbCtx->pAecGlobal->NightGridWeights.ArraySize, fp);
    }
    if(pCamCalibDbCtx->pAecGlobal->GainRange.pGainRange != NULL){
       fwrite(pCamCalibDbCtx->pAecGlobal->GainRange.pGainRange, sizeof(float),
              pCamCalibDbCtx->pAecGlobal->GainRange.array_size, fp);
    }
    DumpDySetpointList(&pCamCalibDbCtx->pAecGlobal->DySetpointList, fp);
    DumpExpSeparateList(&pCamCalibDbCtx->pAecGlobal->ExpSeparateList, fp);
  }
  LOGD( "%s:%d: file pos 0x%x\n", __FUNCTION__, __LINE__, ftell(fp));

  if (pCamCalibDbCtx->pWdrGlobal) {
    fwrite(pCamCalibDbCtx->pWdrGlobal, sizeof(CamCalibWdrGlobal_t), 1, fp);
    if (pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.pfMaxGain_level != NULL) {
      fwrite(pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.pfMaxGain_level, sizeof(float),
             pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.nSize, fp);
    }
    if (pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.pfSensorGain_level != NULL) {
      fwrite(pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.pfSensorGain_level, sizeof(float),
             pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.nSize, fp);
    }
  }
  LOGD( "%s:%d: file pos 0x%x\n", __FUNCTION__, __LINE__, ftell(fp));

  if (pCamCalibDbCtx->pCprocGlobal)
    fwrite(pCamCalibDbCtx->pCprocGlobal, sizeof(CamCprocProfile_t), 1, fp);
  LOGD( "%s:%d: file pos 0x%x\n", __FUNCTION__, __LINE__, ftell(fp));
  DumpEcmProfileList(& pCamCalibDbCtx->ecm_profile, fp);
  DumpLscProfileList(&pCamCalibDbCtx->lsc_profile, fp);
  DumpCcProfileList(&pCamCalibDbCtx->cc_profile, fp);
  DumpBlsProfileList(&pCamCalibDbCtx->bls_profile, fp);
  DumpCacProfileList(&pCamCalibDbCtx->cac_profile, fp);
  DumpDpfProfileList(&pCamCalibDbCtx->dpf_profile, fp);
  DumpDpccProfileList(&pCamCalibDbCtx->dpcc_profile, fp);
  DumpGocProfileList(&pCamCalibDbCtx->gocProfile, fp);
  DumpIeSharpenProfileList(&pCamCalibDbCtx->iesharpen_profile, fp);
  if (pCamCalibDbCtx->pOTPGlobal)
    fwrite(pCamCalibDbCtx->pOTPGlobal, sizeof(CamOTPGlobal_t), 1, fp);

  fclose(fp);
  { /* sync file data */
    int fd;

    fd = open(xml_dump_bin_file, O_RDWR);
    fdatasync(fd);
    close(fd);
  }

#ifdef USE_C_SOURCE_XML_BIN
  sprintf(xml_dump_db_file, "%s/%s.db", GetXmlDbDir(), xml_path_split + 1);
  DumpBin2CSource(xml_dump_bin_file, xml_dump_db_file);
#endif
  //remove(xml_dump_bin_file);
  LOGD( "%s (exit)\n", __FUNCTION__);

  return (RET_SUCCESS);
}

static unsigned int gCamCalibDbIqIdx;
static const char* gpCamCalibDbIqData = NULL;

static RESULT initCamCalibDbIq(const char* CamCalibDbIqData) {
  char* xml_path_split = strrchr(CamCalibDbIqData, '/');
  char xml_db_file[128];

#ifdef USE_C_SOURCE_XML_BIN
  // use built-in iq
  LOGD("%s: loading iq from built-in source", __FUNCTION__);
  gpCamCalibDbIqData = iq_xml_db;
#else
  sprintf(xml_db_file, "%s/%s.bin", GetXmlDbDir(), xml_path_split + 1);
  if (access(xml_db_file, R_OK) != -1) {
    FILE* fp_in = NULL;
    size_t bin_size;

    fp_in = fopen(xml_db_file, "rb");
    if (!fp_in) {
        LOGE( "%s:open %s failed!!\n", __func__, xml_db_file);
        return RET_FAILURE;
    }

    fseek(fp_in, 0L, SEEK_END);
    bin_size = ftell(fp_in);
    fseek(fp_in, 0L, SEEK_SET);

    gpCamCalibDbIqData = malloc(bin_size);
    if (gpCamCalibDbIqData == NULL) {
        LOGE( "%s:malloc failed!!\n", __func__);
        fclose(fp_in);
        return RET_FAILURE;
    }

    fread((void*)gpCamCalibDbIqData, bin_size, 1, fp_in);
    fclose(fp_in);
	LOGD("%s: loading iq from bin file %s", __FUNCTION__, xml_db_file);
  } else
    return RET_FAILURE;
#endif
  gCamCalibDbIqIdx = 0;
  return RET_SUCCESS;
}

static void readCamCalibDbIq(void* pBuf, unsigned int size) {
  memcpy(pBuf, &gpCamCalibDbIqData[gCamCalibDbIqIdx], size);
  gCamCalibDbIqIdx += size;
}

static unsigned int getCamCalibDbIqIdx() {
  return gCamCalibDbIqIdx;
}

static void LoadFrameRateList(List* l) {
  CamFrameRate_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
  if (!ListEmpty(l)) {
    CamFrameRate_t* pFrameRate = malloc(sizeof(CamFrameRate_t));
    l->p_next = (List*)pFrameRate;
    readCamCalibDbIq(pFrameRate, sizeof(CamFrameRate_t));
    while (pFrameRate->p_next) {
      pNew = malloc(sizeof(CamFrameRate_t));
      readCamCalibDbIq(pNew, sizeof(CamFrameRate_t));

      pFrameRate->p_next = pNew;
      pFrameRate = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadResolutionList(List* l) {
  CamResolution_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if (!ListEmpty(l)) {
    CamResolution_t* pResolution = malloc(sizeof(CamResolution_t));
    l->p_next = (List*)pResolution;
    readCamCalibDbIq(pResolution, sizeof(CamResolution_t));
    LOGD("pResolution->p_next %p, pResolution->list %p", pResolution->p_next,
        pResolution->framerates.p_next);
    LoadFrameRateList(&pResolution->framerates);
    while (pResolution->p_next) {
      pNew = malloc(sizeof(CamResolution_t));
      readCamCalibDbIq(pNew, sizeof(CamResolution_t));
      LoadFrameRateList(&pNew->framerates);

      pResolution->p_next = pNew;
      pResolution = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadAwb_V10_GlobalSubList(CamCalibAwb_V10_Global_t* pAwbGlobal) {
#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  pAwbGlobal->AwbClipParam.pRg1 = malloc(pAwbGlobal->AwbClipParam.ArraySize1 * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbClipParam.pRg1, pAwbGlobal->AwbClipParam.ArraySize1 * sizeof(float));
  pAwbGlobal->AwbClipParam.pMaxDist1 = malloc(pAwbGlobal->AwbClipParam.ArraySize1 * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbClipParam.pMaxDist1, pAwbGlobal->AwbClipParam.ArraySize1 * sizeof(float));
  pAwbGlobal->AwbClipParam.pRg2 = malloc(pAwbGlobal->AwbClipParam.ArraySize2 * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbClipParam.pRg2, pAwbGlobal->AwbClipParam.ArraySize2 * sizeof(float));
  pAwbGlobal->AwbClipParam.pMaxDist2 = malloc(pAwbGlobal->AwbClipParam.ArraySize2 * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbClipParam.pMaxDist2, pAwbGlobal->AwbClipParam.ArraySize2 * sizeof(float));

  pAwbGlobal->AwbGlobalFadeParm.pGlobalFade1 = malloc(pAwbGlobal->AwbGlobalFadeParm.ArraySize1 * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbGlobalFadeParm.pGlobalFade1, pAwbGlobal->AwbGlobalFadeParm.ArraySize1 * sizeof(float));
  pAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance1 = malloc(pAwbGlobal->AwbGlobalFadeParm.ArraySize1 * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance1, pAwbGlobal->AwbGlobalFadeParm.ArraySize1 * sizeof(float));
  pAwbGlobal->AwbGlobalFadeParm.pGlobalFade2 = malloc(pAwbGlobal->AwbGlobalFadeParm.ArraySize2 * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbGlobalFadeParm.pGlobalFade2, pAwbGlobal->AwbGlobalFadeParm.ArraySize2 * sizeof(float));
  pAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance2 = malloc(pAwbGlobal->AwbGlobalFadeParm.ArraySize2 * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance2, pAwbGlobal->AwbGlobalFadeParm.ArraySize2 * sizeof(float));

  pAwbGlobal->AwbFade2Parm.pFade = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pFade, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pCbMinRegionMax = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pCbMinRegionMax, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pCrMinRegionMax = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pCrMinRegionMax, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pMaxCSumRegionMax = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pMaxCSumRegionMax, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pCbMinRegionMin = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pCbMinRegionMin, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pCrMinRegionMin = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pCrMinRegionMin, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pMaxCSumRegionMin = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pMaxCSumRegionMin, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pMinCRegionMax = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pMinCRegionMax, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pMinCRegionMin = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pMinCRegionMin, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pMaxYRegionMax = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pMaxYRegionMax, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pMaxYRegionMin = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pMaxYRegionMin, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pMinYMaxGRegionMax = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pMinYMaxGRegionMax, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pMinYMaxGRegionMin = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pMinYMaxGRegionMin, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pRefCb = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pRefCb, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pRefCr = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pRefCr, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadAwb_V10_GlobalList(List* l) {
  CamCalibAwb_V10_Global_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
  if (!ListEmpty(l)) {
    CamCalibAwb_V10_Global_t* pAwbGlobal = malloc(sizeof(CamCalibAwb_V10_Global_t));
    l->p_next = (List*)pAwbGlobal;
    readCamCalibDbIq(pAwbGlobal, sizeof(CamCalibAwb_V10_Global_t));
    LoadAwb_V10_GlobalSubList(pAwbGlobal);
    while (pAwbGlobal->p_next) {
      pNew = malloc(sizeof(CamCalibAwb_V10_Global_t));
      readCamCalibDbIq(pNew, sizeof(CamCalibAwb_V10_Global_t));
      LoadAwb_V10_GlobalSubList(pNew);

      pAwbGlobal->p_next = pNew;
      pAwbGlobal = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit)\n", __FUNCTION__);
#endif
}

static void LoadAwb_V11_GlobalSubList(CamCalibAwb_V11_Global_t* pAwbGlobal) {
#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
  pAwbGlobal->AwbClipParam.pRg1 = malloc(pAwbGlobal->AwbClipParam.ArraySize1 * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbClipParam.pRg1, pAwbGlobal->AwbClipParam.ArraySize1 * sizeof(float));
  pAwbGlobal->AwbClipParam.pMaxDist1 = malloc(pAwbGlobal->AwbClipParam.ArraySize1 * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbClipParam.pMaxDist1, pAwbGlobal->AwbClipParam.ArraySize1 * sizeof(float));
  pAwbGlobal->AwbClipParam.pRg2 = malloc(pAwbGlobal->AwbClipParam.ArraySize2 * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbClipParam.pRg2, pAwbGlobal->AwbClipParam.ArraySize2 * sizeof(float));
  pAwbGlobal->AwbClipParam.pMaxDist2 = malloc(pAwbGlobal->AwbClipParam.ArraySize2 * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbClipParam.pMaxDist2, pAwbGlobal->AwbClipParam.ArraySize2 * sizeof(float));

  pAwbGlobal->AwbGlobalFadeParm.pGlobalFade1 = malloc(pAwbGlobal->AwbGlobalFadeParm.ArraySize1 * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbGlobalFadeParm.pGlobalFade1, pAwbGlobal->AwbGlobalFadeParm.ArraySize1 * sizeof(float));
  pAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance1 = malloc(pAwbGlobal->AwbGlobalFadeParm.ArraySize1 * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance1, pAwbGlobal->AwbGlobalFadeParm.ArraySize1 * sizeof(float));
  pAwbGlobal->AwbGlobalFadeParm.pGlobalFade2 = malloc(pAwbGlobal->AwbGlobalFadeParm.ArraySize2 * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbGlobalFadeParm.pGlobalFade2, pAwbGlobal->AwbGlobalFadeParm.ArraySize2 * sizeof(float));
  pAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance2 = malloc(pAwbGlobal->AwbGlobalFadeParm.ArraySize2 * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance2, pAwbGlobal->AwbGlobalFadeParm.ArraySize2 * sizeof(float));

  pAwbGlobal->AwbFade2Parm.pFade = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pFade, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));

  pAwbGlobal->AwbFade2Parm.pMaxCSum_br = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pMaxCSum_br, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pMaxCSum_sr = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pMaxCSum_sr, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pMinC_br = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pMinC_br, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pMinC_sr = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pMinC_sr, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));

  pAwbGlobal->AwbFade2Parm.pMaxY_br = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pMaxY_br, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pMaxY_sr = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pMaxY_sr, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pMinY_br = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pMinY_br, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pMinY_sr = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pMinY_sr, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));

  pAwbGlobal->AwbFade2Parm.pRefCb = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pRefCb, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  pAwbGlobal->AwbFade2Parm.pRefCr = malloc(pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));
  readCamCalibDbIq(pAwbGlobal->AwbFade2Parm.pRefCr, pAwbGlobal->AwbFade2Parm.ArraySize * sizeof(float));

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadAwb_V11_GlobalList(List* l) {
  CamCalibAwb_V11_Global_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
  if (!ListEmpty(l)) {
    CamCalibAwb_V11_Global_t* pAwbGlobal = malloc(sizeof(CamCalibAwb_V11_Global_t));
    l->p_next = (List*)pAwbGlobal;
    readCamCalibDbIq(pAwbGlobal, sizeof(CamCalibAwb_V11_Global_t));
    LoadAwb_V11_GlobalSubList(pAwbGlobal);
    while (pAwbGlobal->p_next) {
      pNew = malloc(sizeof(CamCalibAwb_V11_Global_t));
      readCamCalibDbIq(pNew, sizeof(CamCalibAwb_V11_Global_t));
      LoadAwb_V11_GlobalSubList(pNew);

      pAwbGlobal->p_next = pNew;
      pAwbGlobal = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadEcmSchemeList(List* l) {
  CamEcmScheme_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
  if (!ListEmpty(l)) {
    CamEcmScheme_t* pEcmScheme = malloc(sizeof(CamEcmScheme_t));
    l->p_next = (List*)pEcmScheme;
    readCamCalibDbIq(pEcmScheme, sizeof(CamEcmScheme_t));
    while (pEcmScheme->p_next) {
      pNew = malloc(sizeof(CamEcmScheme_t));
      readCamCalibDbIq(pNew, sizeof(CamEcmScheme_t));

      pEcmScheme->p_next = pNew;
      pEcmScheme = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadEcmProfileList(List* l) {
  CamEcmProfile_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if (!ListEmpty(l)) {
    CamEcmProfile_t* pEcmProfile = malloc(sizeof(CamEcmProfile_t));
    l->p_next = (List*)pEcmProfile;
    readCamCalibDbIq(pEcmProfile, sizeof(CamEcmProfile_t));
    LoadEcmSchemeList(&pEcmProfile->ecm_scheme);
    while (pEcmProfile->p_next) {
      pNew = malloc(sizeof(CamEcmProfile_t));
      readCamCalibDbIq(pNew, sizeof(CamEcmProfile_t));
      LoadEcmSchemeList(&pNew->ecm_scheme);

      pEcmProfile->p_next = pNew;
      pEcmProfile = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadAwb_V10_IlluminationSubList(CamAwb_V10_IlluProfile_t* pIllumination) {
#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  pIllumination->SaturationCurve.pSensorGain = malloc(pIllumination->SaturationCurve.ArraySize * sizeof(float));
  readCamCalibDbIq(pIllumination->SaturationCurve.pSensorGain,
    pIllumination->SaturationCurve.ArraySize * sizeof(float));
  pIllumination->SaturationCurve.pSaturation = malloc(pIllumination->SaturationCurve.ArraySize * sizeof(float));
  readCamCalibDbIq(pIllumination->SaturationCurve.pSaturation,
    pIllumination->SaturationCurve.ArraySize * sizeof(float));

  pIllumination->VignettingCurve.pSensorGain = malloc(pIllumination->VignettingCurve.ArraySize * sizeof(float));
  readCamCalibDbIq(pIllumination->VignettingCurve.pSensorGain,
    pIllumination->VignettingCurve.ArraySize * sizeof(float));
  pIllumination->VignettingCurve.pVignetting = malloc(pIllumination->VignettingCurve.ArraySize * sizeof(float));
  readCamCalibDbIq(pIllumination->VignettingCurve.pVignetting,
    pIllumination->VignettingCurve.ArraySize * sizeof(float));

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadAwb_V10_IlluminationList(List* l) {
  CamAwb_V10_IlluProfile_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if (!ListEmpty(l)) {
    CamAwb_V10_IlluProfile_t* pIllumination = malloc(sizeof(CamAwb_V10_IlluProfile_t));
    l->p_next = (List*)pIllumination;
    readCamCalibDbIq(pIllumination, sizeof(CamAwb_V10_IlluProfile_t));
    LoadAwb_V10_IlluminationSubList(pIllumination);
    while (pIllumination->p_next) {
      pNew = malloc(sizeof(CamAwb_V10_IlluProfile_t));
      readCamCalibDbIq(pNew, sizeof(CamAwb_V10_IlluProfile_t));
      LoadAwb_V10_IlluminationSubList(pNew);

      pIllumination->p_next = pNew;
      pIllumination = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadAwb_V11_IlluminationSubList(CamAwb_V11_IlluProfile_t* pIllumination) {
#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  pIllumination->SaturationCurve.pSensorGain = malloc(pIllumination->SaturationCurve.ArraySize * sizeof(float));
  readCamCalibDbIq(pIllumination->SaturationCurve.pSensorGain,
    pIllumination->SaturationCurve.ArraySize * sizeof(float));
  pIllumination->SaturationCurve.pSaturation = malloc(pIllumination->SaturationCurve.ArraySize * sizeof(float));
  readCamCalibDbIq(pIllumination->SaturationCurve.pSaturation,
    pIllumination->SaturationCurve.ArraySize * sizeof(float));

  pIllumination->VignettingCurve.pSensorGain = malloc(pIllumination->VignettingCurve.ArraySize * sizeof(float));
  readCamCalibDbIq(pIllumination->VignettingCurve.pSensorGain,
    pIllumination->VignettingCurve.ArraySize * sizeof(float));
  pIllumination->VignettingCurve.pVignetting = malloc(pIllumination->VignettingCurve.ArraySize * sizeof(float));
  readCamCalibDbIq(pIllumination->VignettingCurve.pVignetting,
    pIllumination->VignettingCurve.ArraySize * sizeof(float));

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadAwb_V11_IlluminationList(List* l) {
  CamAwb_V11_IlluProfile_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if (!ListEmpty(l)) {
    CamAwb_V11_IlluProfile_t* pIllumination = malloc(sizeof(CamAwb_V11_IlluProfile_t));
    l->p_next = (List*)pIllumination;
    readCamCalibDbIq(pIllumination, sizeof(CamAwb_V11_IlluProfile_t));
    LoadAwb_V11_IlluminationSubList(pIllumination);
    while (pIllumination->p_next) {
      pNew = malloc(sizeof(CamAwb_V11_IlluProfile_t));
      readCamCalibDbIq(pNew, sizeof(CamAwb_V11_IlluProfile_t));
      LoadAwb_V11_IlluminationSubList(pNew);

      pIllumination->p_next = pNew;
      pIllumination = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadLscProfileList(List* l) {
  CamLscProfile_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if (!ListEmpty(l)) {
    CamLscProfile_t* pLscProfile = malloc(sizeof(CamLscProfile_t));
    l->p_next = (List*)pLscProfile;
    readCamCalibDbIq(pLscProfile, sizeof(CamLscProfile_t));
    while (pLscProfile->p_next) {
      pNew = malloc(sizeof(CamLscProfile_t));
      readCamCalibDbIq(pNew, sizeof(CamLscProfile_t));

      pLscProfile->p_next = pNew;
      pLscProfile = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadCcProfileList(List* l) {
  CamCcProfile_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if (!ListEmpty(l)) {
    CamCcProfile_t* pCcProfile = malloc(sizeof(CamCcProfile_t));
    l->p_next = (List*)pCcProfile;
    readCamCalibDbIq(pCcProfile, sizeof(CamCcProfile_t));
    while (pCcProfile->p_next) {
      pNew = malloc(sizeof(CamCcProfile_t));
      readCamCalibDbIq(pNew, sizeof(CamCcProfile_t));

      pCcProfile->p_next = pNew;
      pCcProfile = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadBlsProfileList(List* l) {
  CamBlsProfile_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if (!ListEmpty(l)) {
    CamBlsProfile_t* pBlsProfile = malloc(sizeof(CamBlsProfile_t));
    l->p_next = (List*)pBlsProfile;
    readCamCalibDbIq(pBlsProfile, sizeof(CamBlsProfile_t));
    while (pBlsProfile->p_next) {
      pNew = malloc(sizeof(CamBlsProfile_t));
      readCamCalibDbIq(pNew, sizeof(CamBlsProfile_t));

      pBlsProfile->p_next = pNew;
      pBlsProfile = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadCacProfileList(List* l) {
  CamCacProfile_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if (!ListEmpty(l)) {
    CamCacProfile_t* pCacProfile = malloc(sizeof(CamCacProfile_t));
    l->p_next = (List*)pCacProfile;
    readCamCalibDbIq(pCacProfile, sizeof(CamCacProfile_t));
    while (pCacProfile->p_next) {
      pNew = malloc(sizeof(CamCacProfile_t));
      readCamCalibDbIq(pNew, sizeof(CamCacProfile_t));

      pCacProfile->p_next = pNew;
      pCacProfile = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadDsp3DNRSubList(CamDsp3DNRSettingProfile_t * pDsp3DNR) {
#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if(pDsp3DNR->pgain_Level){
    pDsp3DNR->pgain_Level = malloc(pDsp3DNR->ArraySize * sizeof(float));
    readCamCalibDbIq(pDsp3DNR->pgain_Level, pDsp3DNR->ArraySize * sizeof(float));
  }
  if(pDsp3DNR->pnoise_coef_denominator){
    pDsp3DNR->pnoise_coef_denominator = malloc(pDsp3DNR->ArraySize * sizeof(uint16_t));
    readCamCalibDbIq(pDsp3DNR->pnoise_coef_denominator, pDsp3DNR->ArraySize * sizeof(uint16_t));
  }
  if(pDsp3DNR->pnoise_coef_numerator){
    pDsp3DNR->pnoise_coef_numerator = malloc(pDsp3DNR->ArraySize * sizeof(uint16_t));
    readCamCalibDbIq(pDsp3DNR->pnoise_coef_numerator, pDsp3DNR->ArraySize * sizeof(uint16_t));
  }
  if(pDsp3DNR->sDefaultLevelSetting.pchrm_sp_nr_level){
    pDsp3DNR->sDefaultLevelSetting.pchrm_sp_nr_level = malloc(pDsp3DNR->ArraySize * sizeof(unsigned char));
    readCamCalibDbIq(pDsp3DNR->sDefaultLevelSetting.pchrm_sp_nr_level, pDsp3DNR->ArraySize * sizeof(unsigned char));
  }
  if(pDsp3DNR->sDefaultLevelSetting.pchrm_te_nr_level){
    pDsp3DNR->sDefaultLevelSetting.pchrm_te_nr_level = malloc(pDsp3DNR->ArraySize * sizeof(unsigned char));
    readCamCalibDbIq(pDsp3DNR->sDefaultLevelSetting.pchrm_te_nr_level, pDsp3DNR->ArraySize * sizeof(unsigned char));
  }
  if(pDsp3DNR->sDefaultLevelSetting.pluma_sp_nr_level){
    pDsp3DNR->sDefaultLevelSetting.pluma_sp_nr_level = malloc(pDsp3DNR->ArraySize * sizeof(unsigned char));
    readCamCalibDbIq(pDsp3DNR->sDefaultLevelSetting.pluma_sp_nr_level, pDsp3DNR->ArraySize * sizeof(unsigned char));
  }
  if(pDsp3DNR->sDefaultLevelSetting.pluma_te_nr_level){
    pDsp3DNR->sDefaultLevelSetting.pluma_te_nr_level = malloc(pDsp3DNR->ArraySize * sizeof(unsigned char));
    readCamCalibDbIq(pDsp3DNR->sDefaultLevelSetting.pluma_te_nr_level, pDsp3DNR->ArraySize * sizeof(unsigned char));
  }
  if(pDsp3DNR->sDefaultLevelSetting.pshp_level){
    pDsp3DNR->sDefaultLevelSetting.pshp_level = malloc(pDsp3DNR->ArraySize * sizeof(unsigned char));
    readCamCalibDbIq(pDsp3DNR->sDefaultLevelSetting.pshp_level, pDsp3DNR->ArraySize * sizeof(unsigned char));
  }

  if(pDsp3DNR->sLumaSetting.pluma_sp_rad){
    pDsp3DNR->sLumaSetting.pluma_sp_rad = malloc(pDsp3DNR->ArraySize * sizeof(unsigned char));
    readCamCalibDbIq(pDsp3DNR->sLumaSetting.pluma_sp_rad, pDsp3DNR->ArraySize * sizeof(unsigned char));
  }
  if(pDsp3DNR->sLumaSetting.pluma_te_max_bi_num){
    pDsp3DNR->sLumaSetting.pluma_te_max_bi_num = malloc(pDsp3DNR->ArraySize * sizeof(unsigned char));
    readCamCalibDbIq(pDsp3DNR->sLumaSetting.pluma_te_max_bi_num, pDsp3DNR->ArraySize * sizeof(unsigned char));
  }

  if(pDsp3DNR->sChrmSetting.pchrm_sp_rad){
    pDsp3DNR->sChrmSetting.pchrm_sp_rad = malloc(pDsp3DNR->ArraySize * sizeof(unsigned char));
    readCamCalibDbIq(pDsp3DNR->sChrmSetting.pchrm_sp_rad, pDsp3DNR->ArraySize * sizeof(unsigned char));
  }
  if(pDsp3DNR->sChrmSetting.pchrm_te_max_bi_num){
    pDsp3DNR->sChrmSetting.pchrm_te_max_bi_num = malloc(pDsp3DNR->ArraySize * sizeof(unsigned char));
    readCamCalibDbIq(pDsp3DNR->sChrmSetting.pchrm_te_max_bi_num, pDsp3DNR->ArraySize * sizeof(unsigned char));
  }

  if(pDsp3DNR->sSharpSetting.psrc_shp_c){
    pDsp3DNR->sSharpSetting.psrc_shp_c = malloc(pDsp3DNR->ArraySize * sizeof(unsigned char));
    readCamCalibDbIq(pDsp3DNR->sSharpSetting.psrc_shp_c, pDsp3DNR->ArraySize * sizeof(unsigned char));
  }
  if(pDsp3DNR->sSharpSetting.psrc_shp_div){
    pDsp3DNR->sSharpSetting.psrc_shp_div = malloc(pDsp3DNR->ArraySize * sizeof(unsigned char));
    readCamCalibDbIq(pDsp3DNR->sSharpSetting.psrc_shp_div, pDsp3DNR->ArraySize * sizeof(unsigned char));
  }
  if(pDsp3DNR->sSharpSetting.psrc_shp_l){
    pDsp3DNR->sSharpSetting.psrc_shp_l = malloc(pDsp3DNR->ArraySize * sizeof(unsigned char));
    readCamCalibDbIq(pDsp3DNR->sSharpSetting.psrc_shp_l, pDsp3DNR->ArraySize * sizeof(unsigned char));
  }
  if(pDsp3DNR->sSharpSetting.psrc_shp_thr){
    pDsp3DNR->sSharpSetting.psrc_shp_thr = malloc(pDsp3DNR->ArraySize * sizeof(unsigned char));
    readCamCalibDbIq(pDsp3DNR->sSharpSetting.psrc_shp_thr, pDsp3DNR->ArraySize * sizeof(unsigned char));
  }

  for(int i=0; i<CAM_CALIBDB_3DNR_WEIGHT_NUM; i++){
    if(pDsp3DNR->sLumaSetting.pluma_weight[i]){
       pDsp3DNR->sLumaSetting.pluma_weight[i] = malloc(pDsp3DNR->ArraySize * sizeof(uint8_t));
       readCamCalibDbIq(pDsp3DNR->sLumaSetting.pluma_weight[i], pDsp3DNR->ArraySize * sizeof(uint8_t));
    }

    if(pDsp3DNR->sChrmSetting.pchrm_weight[i]){
       pDsp3DNR->sChrmSetting.pchrm_weight[i] = malloc(pDsp3DNR->ArraySize * sizeof(uint8_t));
       readCamCalibDbIq(pDsp3DNR->sChrmSetting.pchrm_weight[i], pDsp3DNR->ArraySize * sizeof(uint8_t));
    }

    if(pDsp3DNR->sSharpSetting.psrc_shp_weight[i]){
       pDsp3DNR->sSharpSetting.psrc_shp_weight[i] = malloc(pDsp3DNR->ArraySize * sizeof(int8_t));
       readCamCalibDbIq(pDsp3DNR->sSharpSetting.psrc_shp_weight[i], pDsp3DNR->ArraySize * sizeof(int8_t));
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadDsp3DNRList(List* l) {
  CamDsp3DNRSettingProfile_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if (!ListEmpty(l)) {
    CamDsp3DNRSettingProfile_t * pDsp3DNR = malloc(sizeof(CamDsp3DNRSettingProfile_t));
    l->p_next = (List*)pDsp3DNR;
    readCamCalibDbIq(pDsp3DNR, sizeof(CamDsp3DNRSettingProfile_t));
    LoadDsp3DNRSubList(pDsp3DNR);
    while (pDsp3DNR->p_next) {
      pNew = malloc(sizeof(CamDsp3DNRSettingProfile_t));
      readCamCalibDbIq(pNew, sizeof(CamDsp3DNRSettingProfile_t));
      LoadDsp3DNRSubList(pNew);

      pDsp3DNR->p_next = pNew;
      pDsp3DNR = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadNewDsp3DNRSubList(CamNewDsp3DNRProfile_t * pNewDsp3DNR) {
#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if(pNewDsp3DNR->pgain_Level) {
    pNewDsp3DNR->pgain_Level = malloc(pNewDsp3DNR->ArraySize * sizeof(float));
    readCamCalibDbIq(pNewDsp3DNR->pgain_Level, pNewDsp3DNR->ArraySize * sizeof(float));
  }

  if(pNewDsp3DNR->ynr.pynr_time_weight_level) {
    pNewDsp3DNR->ynr.pynr_time_weight_level = malloc(pNewDsp3DNR->ArraySize * sizeof(uint32_t));
    readCamCalibDbIq(pNewDsp3DNR->ynr.pynr_time_weight_level, pNewDsp3DNR->ArraySize * sizeof(uint32_t));
  }

  if(pNewDsp3DNR->ynr.pynr_spat_weight_level) {
    pNewDsp3DNR->ynr.pynr_spat_weight_level = malloc(pNewDsp3DNR->ArraySize * sizeof(uint32_t));
    readCamCalibDbIq(pNewDsp3DNR->ynr.pynr_spat_weight_level, pNewDsp3DNR->ArraySize * sizeof(uint32_t));
  }

  if(pNewDsp3DNR->uvnr.puvnr_weight_level) {
    pNewDsp3DNR->uvnr.puvnr_weight_level = malloc(pNewDsp3DNR->ArraySize * sizeof(uint32_t));
    readCamCalibDbIq(pNewDsp3DNR->uvnr.puvnr_weight_level, pNewDsp3DNR->ArraySize * sizeof(uint32_t));
  }

  if(pNewDsp3DNR->sharp.psharp_weight_level) {
    pNewDsp3DNR->sharp.psharp_weight_level = malloc(pNewDsp3DNR->ArraySize * sizeof(uint32_t));
    readCamCalibDbIq(pNewDsp3DNR->sharp.psharp_weight_level, pNewDsp3DNR->ArraySize * sizeof(uint32_t));
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadNewDsp3DNRList(List* l) {
  CamNewDsp3DNRProfile_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if (!ListEmpty(l)) {
    CamNewDsp3DNRProfile_t * pNewDsp3DNR = malloc(sizeof(CamNewDsp3DNRProfile_t));
    l->p_next = (List*)pNewDsp3DNR;
    readCamCalibDbIq(pNewDsp3DNR, sizeof(CamNewDsp3DNRProfile_t));
    LoadNewDsp3DNRSubList(pNewDsp3DNR);
    while (pNewDsp3DNR->p_next) {
      pNew = malloc(sizeof(CamNewDsp3DNRProfile_t));
      readCamCalibDbIq(pNew, sizeof(CamNewDsp3DNRProfile_t));
      LoadNewDsp3DNRSubList(pNew);

      pNewDsp3DNR->p_next = pNew;
      pNewDsp3DNR = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadFilterSubList(CamFilterProfile_t * pFilter) {
#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if(pFilter->DemosaicThCurve.pSensorGain){
    pFilter->DemosaicThCurve.pSensorGain = malloc(pFilter->DemosaicThCurve.ArraySize * sizeof(float));
    readCamCalibDbIq(pFilter->DemosaicThCurve.pSensorGain, pFilter->DemosaicThCurve.ArraySize * sizeof(float));
  }
  if(pFilter->DemosaicThCurve.pThlevel){
    pFilter->DemosaicThCurve.pThlevel = malloc(pFilter->DemosaicThCurve.ArraySize * sizeof(uint8_t));
    readCamCalibDbIq(pFilter->DemosaicThCurve.pThlevel, pFilter->DemosaicThCurve.ArraySize * sizeof(uint8_t));
  }

  if(pFilter->DenoiseLevelCurve.pSensorGain){
    pFilter->DenoiseLevelCurve.pSensorGain = malloc(pFilter->DenoiseLevelCurve.ArraySize * sizeof(float));
    readCamCalibDbIq(pFilter->DenoiseLevelCurve.pSensorGain, pFilter->DenoiseLevelCurve.ArraySize * sizeof(float));
  }
  if(pFilter->DenoiseLevelCurve.pDlevel){
    pFilter->DenoiseLevelCurve.pDlevel = malloc(pFilter->DenoiseLevelCurve.ArraySize * sizeof(CamerIcIspFltDeNoiseLevel_t));
    readCamCalibDbIq(pFilter->DenoiseLevelCurve.pDlevel,
      pFilter->DenoiseLevelCurve.ArraySize * sizeof(CamerIcIspFltDeNoiseLevel_t));
  }

  if(pFilter->SharpeningLevelCurve.pSensorGain){
    pFilter->SharpeningLevelCurve.pSensorGain = malloc(pFilter->SharpeningLevelCurve.ArraySize * sizeof(float));
    readCamCalibDbIq(pFilter->SharpeningLevelCurve.pSensorGain, pFilter->SharpeningLevelCurve.ArraySize * sizeof(float));
  }
  if(pFilter->SharpeningLevelCurve.pSlevel){
    pFilter->SharpeningLevelCurve.pSlevel = malloc(pFilter->SharpeningLevelCurve.ArraySize * sizeof(CamerIcIspFltSharpeningLevel_t));
    readCamCalibDbIq(pFilter->SharpeningLevelCurve.pSlevel,
      pFilter->SharpeningLevelCurve.ArraySize * sizeof(CamerIcIspFltSharpeningLevel_t));
  }

  if(pFilter->FiltLevelRegConf.p_chr_h_mode){
    pFilter->FiltLevelRegConf.p_chr_h_mode = malloc(pFilter->FiltLevelRegConf.ArraySize * sizeof(uint8_t));
    readCamCalibDbIq(pFilter->FiltLevelRegConf.p_chr_h_mode, pFilter->FiltLevelRegConf.ArraySize * sizeof(uint8_t));
  }
  if(pFilter->FiltLevelRegConf.p_chr_v_mode){
    pFilter->FiltLevelRegConf.p_chr_v_mode = malloc(pFilter->FiltLevelRegConf.ArraySize * sizeof(uint8_t));
    readCamCalibDbIq(pFilter->FiltLevelRegConf.p_chr_v_mode, pFilter->FiltLevelRegConf.ArraySize * sizeof(uint8_t));
  }
  if(pFilter->FiltLevelRegConf.p_fac_bl0){
    pFilter->FiltLevelRegConf.p_fac_bl0 = malloc(pFilter->FiltLevelRegConf.ArraySize * sizeof(uint32_t));
    readCamCalibDbIq(pFilter->FiltLevelRegConf.p_fac_bl0, pFilter->FiltLevelRegConf.ArraySize * sizeof(uint32_t));
  }
  if(pFilter->FiltLevelRegConf.p_fac_bl1){
    pFilter->FiltLevelRegConf.p_fac_bl1 = malloc(pFilter->FiltLevelRegConf.ArraySize * sizeof(uint32_t));
    readCamCalibDbIq(pFilter->FiltLevelRegConf.p_fac_bl1, pFilter->FiltLevelRegConf.ArraySize * sizeof(uint32_t));
  }
  if(pFilter->FiltLevelRegConf.p_fac_mid){
    pFilter->FiltLevelRegConf.p_fac_mid = malloc(pFilter->FiltLevelRegConf.ArraySize * sizeof(uint32_t));
    readCamCalibDbIq(pFilter->FiltLevelRegConf.p_fac_mid, pFilter->FiltLevelRegConf.ArraySize * sizeof(uint32_t));
  }
  if(pFilter->FiltLevelRegConf.p_fac_sh0){
    pFilter->FiltLevelRegConf.p_fac_sh0 = malloc(pFilter->FiltLevelRegConf.ArraySize * sizeof(uint32_t));
    readCamCalibDbIq(pFilter->FiltLevelRegConf.p_fac_sh0, pFilter->FiltLevelRegConf.ArraySize * sizeof(uint32_t));
  }
  if(pFilter->FiltLevelRegConf.p_fac_sh1){
    pFilter->FiltLevelRegConf.p_fac_sh1 = malloc(pFilter->FiltLevelRegConf.ArraySize * sizeof(uint32_t));
    readCamCalibDbIq(pFilter->FiltLevelRegConf.p_fac_sh1, pFilter->FiltLevelRegConf.ArraySize * sizeof(uint32_t));
  }
  if(pFilter->FiltLevelRegConf.p_FiltLevel){
    pFilter->FiltLevelRegConf.p_FiltLevel = malloc(pFilter->FiltLevelRegConf.ArraySize * sizeof(uint8_t));
    readCamCalibDbIq(pFilter->FiltLevelRegConf.p_FiltLevel, pFilter->FiltLevelRegConf.ArraySize * sizeof(uint8_t));
  }
  if(pFilter->FiltLevelRegConf.p_grn_stage1){
    pFilter->FiltLevelRegConf.p_grn_stage1 = malloc(pFilter->FiltLevelRegConf.ArraySize * sizeof(uint8_t));
    readCamCalibDbIq(pFilter->FiltLevelRegConf.p_grn_stage1, pFilter->FiltLevelRegConf.ArraySize * sizeof(uint8_t));
  }
  if(pFilter->FiltLevelRegConf.p_thresh_bl0){
    pFilter->FiltLevelRegConf.p_thresh_bl0 = malloc(pFilter->FiltLevelRegConf.ArraySize * sizeof(uint32_t));
    readCamCalibDbIq(pFilter->FiltLevelRegConf.p_thresh_bl0, pFilter->FiltLevelRegConf.ArraySize * sizeof(uint32_t));
  }
  if(pFilter->FiltLevelRegConf.p_thresh_bl1){
    pFilter->FiltLevelRegConf.p_thresh_bl1 = malloc(pFilter->FiltLevelRegConf.ArraySize * sizeof(uint32_t));
    readCamCalibDbIq(pFilter->FiltLevelRegConf.p_thresh_bl1, pFilter->FiltLevelRegConf.ArraySize * sizeof(uint32_t));
  }
  if(pFilter->FiltLevelRegConf.p_thresh_sh0){
    pFilter->FiltLevelRegConf.p_thresh_sh0 = malloc(pFilter->FiltLevelRegConf.ArraySize * sizeof(uint8_t));
    readCamCalibDbIq(pFilter->FiltLevelRegConf.p_thresh_sh0, pFilter->FiltLevelRegConf.ArraySize * sizeof(uint8_t));
  }
  if(pFilter->FiltLevelRegConf.p_thresh_sh1){
    pFilter->FiltLevelRegConf.p_thresh_sh1 = malloc(pFilter->FiltLevelRegConf.ArraySize * sizeof(uint32_t));
    readCamCalibDbIq(pFilter->FiltLevelRegConf.p_thresh_sh1, pFilter->FiltLevelRegConf.ArraySize * sizeof(uint32_t));
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadFilterList(List* l) {
  CamFilterProfile_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if (!ListEmpty(l)) {
    CamFilterProfile_t * pFilter = malloc(sizeof(CamFilterProfile_t));
    l->p_next = (List*)pFilter;
    readCamCalibDbIq(pFilter, sizeof(CamFilterProfile_t));
    LoadFilterSubList(pFilter);
    while (pFilter->p_next) {
      pNew = malloc(sizeof(CamFilterProfile_t));
      readCamCalibDbIq(pNew, sizeof(CamFilterProfile_t));
      LoadFilterSubList(pNew);

      pFilter->p_next = pNew;
      pFilter = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadDpfProfileList(List* l) {
  CamDpfProfile_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if (!ListEmpty(l)) {
    CamDpfProfile_t* pDpfProfile = malloc(sizeof(CamDpfProfile_t));
    l->p_next = (List*)pDpfProfile;
    readCamCalibDbIq(pDpfProfile, sizeof(CamDpfProfile_t));
    LoadDsp3DNRList(&pDpfProfile->Dsp3DNRSettingProfileList);
    LoadNewDsp3DNRList(&pDpfProfile->newDsp3DNRProfileList);
    LoadFilterList(&pDpfProfile->FilterList);
    while (pDpfProfile->p_next) {
      pNew = malloc(sizeof(CamDpfProfile_t));
      readCamCalibDbIq(pNew, sizeof(CamDpfProfile_t));
      LoadDsp3DNRList(&pNew->Dsp3DNRSettingProfileList);
      LoadNewDsp3DNRList(&pNew->newDsp3DNRProfileList);
      LoadFilterList(&pNew->FilterList);

      pDpfProfile->p_next = pNew;
      pDpfProfile = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadDpccProfileList(List* l) {
  CamDpccProfile_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if (!ListEmpty(l)) {
    CamDpccProfile_t* pDpccProfile = malloc(sizeof(CamDpccProfile_t));
    l->p_next = (List*)pDpccProfile;
    readCamCalibDbIq(pDpccProfile, sizeof(CamDpccProfile_t));
    while (pDpccProfile->p_next) {
      pNew = malloc(sizeof(CamDpccProfile_t));
      readCamCalibDbIq(pNew, sizeof(CamDpccProfile_t));

      pDpccProfile->p_next = pNew;
      pDpccProfile = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadGocProfileList(List* l) {
  CamCalibGocProfile_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if (!ListEmpty(l)) {
    CamCalibGocProfile_t* pGocProfile = malloc(sizeof(CamCalibGocProfile_t));
    l->p_next = (List*)pGocProfile;
    readCamCalibDbIq(pGocProfile, sizeof(CamCalibGocProfile_t));
    while (pGocProfile->p_next) {
      pNew = malloc(sizeof(CamCalibGocProfile_t));
      readCamCalibDbIq(pNew, sizeof(CamCalibGocProfile_t));

      pGocProfile->p_next = pNew;
      pGocProfile = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadIeSharpenProfileList(List* l) {
  CamIesharpenProfile_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if (!ListEmpty(l)) {
    CamIesharpenProfile_t* pIeSharpenProfile = malloc(sizeof(CamIesharpenProfile_t));
    l->p_next = (List*)pIeSharpenProfile;
    readCamCalibDbIq(pIeSharpenProfile, sizeof(CamIesharpenProfile_t));
    while (pIeSharpenProfile->p_next) {
      pNew = malloc(sizeof(CamIesharpenProfile_t));
      readCamCalibDbIq(pNew, sizeof(CamIesharpenProfile_t));

      if (pIeSharpenProfile->yavg_thr) {
          pIeSharpenProfile->yavg_thr = malloc(pIeSharpenProfile->yavg_thr_ArraySize * sizeof(uint8_t));
          readCamCalibDbIq(pIeSharpenProfile->yavg_thr, pIeSharpenProfile->yavg_thr_ArraySize * sizeof(uint8_t));
      }

      if (pIeSharpenProfile->yavg_thr) {
          pIeSharpenProfile->yavg_thr = malloc(pIeSharpenProfile->yavg_thr_ArraySize * sizeof(uint8_t));
          readCamCalibDbIq(pIeSharpenProfile->yavg_thr, pIeSharpenProfile->yavg_thr_ArraySize * sizeof(uint8_t));
      }
      if (pIeSharpenProfile->P_delta1) {
          pIeSharpenProfile->P_delta1 = malloc(pIeSharpenProfile->P_delta1_ArraySize * sizeof(uint8_t));
          readCamCalibDbIq(pIeSharpenProfile->P_delta1, pIeSharpenProfile->P_delta1_ArraySize * sizeof(uint8_t));
      }
      if (pIeSharpenProfile->P_delta2) {
          pIeSharpenProfile->P_delta2 = malloc(pIeSharpenProfile->P_delta2_ArraySize * sizeof(uint8_t));
          readCamCalibDbIq(pIeSharpenProfile->P_delta2, pIeSharpenProfile->P_delta2_ArraySize * sizeof(uint8_t));
      }
      if (pIeSharpenProfile->pmaxnumber) {
          pIeSharpenProfile->pmaxnumber = malloc(pIeSharpenProfile->pmaxnumber_ArraySize * sizeof(uint8_t));
          readCamCalibDbIq(pIeSharpenProfile->pmaxnumber, pIeSharpenProfile->pmaxnumber_ArraySize * sizeof(uint8_t));
      }
      if (pIeSharpenProfile->pminnumber) {
          pIeSharpenProfile->pminnumber = malloc(pIeSharpenProfile->pminnumber_ArraySize * sizeof(uint8_t));
          readCamCalibDbIq(pIeSharpenProfile->pminnumber, pIeSharpenProfile->pminnumber_ArraySize * sizeof(uint8_t));
      }
      if (pIeSharpenProfile->gauss_flat_coe) {
          pIeSharpenProfile->gauss_flat_coe = malloc(pIeSharpenProfile->gauss_flat_coe_ArraySize * sizeof(uint8_t));
          readCamCalibDbIq(pIeSharpenProfile->gauss_flat_coe, pIeSharpenProfile->gauss_flat_coe_ArraySize * sizeof(uint8_t));
      }
      if (pIeSharpenProfile->gauss_noise_coe) {
          pIeSharpenProfile->gauss_noise_coe = malloc(pIeSharpenProfile->gauss_noise_coe_ArraySize * sizeof(uint8_t));
          readCamCalibDbIq(pIeSharpenProfile->gauss_noise_coe, pIeSharpenProfile->gauss_noise_coe_ArraySize * sizeof(uint8_t));
      }
      if (pIeSharpenProfile->gauss_other_coe) {
          pIeSharpenProfile->gauss_other_coe = malloc(pIeSharpenProfile->gauss_other_coe_ArraySize * sizeof(uint8_t));
          readCamCalibDbIq(pIeSharpenProfile->gauss_other_coe, pIeSharpenProfile->gauss_other_coe_ArraySize * sizeof(uint8_t));
      }
      if (pIeSharpenProfile->uv_gauss_flat_coe) {
          pIeSharpenProfile->uv_gauss_flat_coe = malloc(pIeSharpenProfile->uv_gauss_flat_coe_ArraySize * sizeof(uint8_t));
          readCamCalibDbIq(pIeSharpenProfile->uv_gauss_flat_coe, pIeSharpenProfile->uv_gauss_flat_coe_ArraySize * sizeof(uint8_t));
      }
      if (pIeSharpenProfile->uv_gauss_noise_coe) {
          pIeSharpenProfile->uv_gauss_noise_coe = malloc(pIeSharpenProfile->uv_gauss_noise_coe_ArraySize * sizeof(uint8_t));
          readCamCalibDbIq(pIeSharpenProfile->uv_gauss_noise_coe, pIeSharpenProfile->uv_gauss_noise_coe_ArraySize * sizeof(uint8_t));
      }
      if (pIeSharpenProfile->uv_gauss_other_coe) {
          pIeSharpenProfile->uv_gauss_other_coe = malloc(pIeSharpenProfile->uv_gauss_other_coe_ArraySize * sizeof(uint8_t));
          readCamCalibDbIq(pIeSharpenProfile->uv_gauss_noise_coe, pIeSharpenProfile->uv_gauss_other_coe_ArraySize * sizeof(uint8_t));
      }
      {
         // CamIesharpenGridConf_t
          if (pIeSharpenProfile->lgridconf.p_grad) {
              pIeSharpenProfile->lgridconf.p_grad = malloc(pIeSharpenProfile->lgridconf.p_grad_ArraySize * sizeof(uint16_t));
              readCamCalibDbIq(pIeSharpenProfile->lgridconf.p_grad, pIeSharpenProfile->lgridconf.p_grad_ArraySize * sizeof(uint16_t));
          }
          if (pIeSharpenProfile->lgridconf.sharp_factor) {
              pIeSharpenProfile->lgridconf.sharp_factor = malloc(pIeSharpenProfile->lgridconf.sharp_factor_ArraySize * sizeof(uint8_t));
              readCamCalibDbIq(pIeSharpenProfile->lgridconf.sharp_factor, pIeSharpenProfile->lgridconf.sharp_factor_ArraySize * sizeof(uint8_t));
          }
          if (pIeSharpenProfile->lgridconf.line1_filter_coe) {
              pIeSharpenProfile->lgridconf.line1_filter_coe = malloc(pIeSharpenProfile->lgridconf.line1_filter_coe_ArraySize * sizeof(uint8_t));
              readCamCalibDbIq(pIeSharpenProfile->lgridconf.line1_filter_coe, pIeSharpenProfile->lgridconf.line1_filter_coe_ArraySize * sizeof(uint8_t));
          }
          if (pIeSharpenProfile->lgridconf.line2_filter_coe) {
              pIeSharpenProfile->lgridconf.line2_filter_coe = malloc(pIeSharpenProfile->lgridconf.line2_filter_coe_ArraySize * sizeof(uint8_t));
              readCamCalibDbIq(pIeSharpenProfile->lgridconf.line2_filter_coe, pIeSharpenProfile->lgridconf.line2_filter_coe_ArraySize * sizeof(uint8_t));
          }
          if (pIeSharpenProfile->lgridconf.line3_filter_coe) {
              pIeSharpenProfile->lgridconf.line3_filter_coe = malloc(pIeSharpenProfile->lgridconf.line3_filter_coe_ArraySize * sizeof(uint8_t));
              readCamCalibDbIq(pIeSharpenProfile->lgridconf.line3_filter_coe, pIeSharpenProfile->lgridconf.line3_filter_coe_ArraySize * sizeof(uint8_t));
          }
          if (pIeSharpenProfile->lgridconf.lap_mat_coe) {
              pIeSharpenProfile->lgridconf.lap_mat_coe = malloc(pIeSharpenProfile->lgridconf.lap_mat_coe_ArraySize * sizeof(uint8_t));
              readCamCalibDbIq(pIeSharpenProfile->lgridconf.lap_mat_coe, pIeSharpenProfile->lgridconf.lap_mat_coe_ArraySize * sizeof(uint8_t));
          }

          if (pIeSharpenProfile->hgridconf.p_grad) {
              pIeSharpenProfile->hgridconf.p_grad = malloc(pIeSharpenProfile->hgridconf.p_grad_ArraySize * sizeof(uint16_t));
              readCamCalibDbIq(pIeSharpenProfile->hgridconf.p_grad, pIeSharpenProfile->hgridconf.p_grad_ArraySize * sizeof(uint16_t));
          }
          if (pIeSharpenProfile->hgridconf.sharp_factor) {
              pIeSharpenProfile->hgridconf.sharp_factor = malloc(pIeSharpenProfile->hgridconf.sharp_factor_ArraySize * sizeof(uint8_t));
              readCamCalibDbIq(pIeSharpenProfile->hgridconf.sharp_factor, pIeSharpenProfile->hgridconf.sharp_factor_ArraySize * sizeof(uint8_t));
          }
          if (pIeSharpenProfile->hgridconf.line1_filter_coe) {
              pIeSharpenProfile->hgridconf.line1_filter_coe = malloc(pIeSharpenProfile->hgridconf.line1_filter_coe_ArraySize * sizeof(uint8_t));
              readCamCalibDbIq(pIeSharpenProfile->hgridconf.line1_filter_coe, pIeSharpenProfile->hgridconf.line1_filter_coe_ArraySize * sizeof(uint8_t));
          }
          if (pIeSharpenProfile->hgridconf.line2_filter_coe) {
              pIeSharpenProfile->hgridconf.line2_filter_coe = malloc(pIeSharpenProfile->hgridconf.line2_filter_coe_ArraySize * sizeof(uint8_t));
              readCamCalibDbIq(pIeSharpenProfile->hgridconf.line2_filter_coe, pIeSharpenProfile->hgridconf.line2_filter_coe_ArraySize * sizeof(uint8_t));
          }
          if (pIeSharpenProfile->hgridconf.line3_filter_coe) {
              pIeSharpenProfile->hgridconf.line3_filter_coe = malloc(pIeSharpenProfile->hgridconf.line3_filter_coe_ArraySize * sizeof(uint8_t));
              readCamCalibDbIq(pIeSharpenProfile->hgridconf.line3_filter_coe, pIeSharpenProfile->hgridconf.line3_filter_coe_ArraySize * sizeof(uint8_t));
          }
          if (pIeSharpenProfile->hgridconf.lap_mat_coe) {
              pIeSharpenProfile->hgridconf.lap_mat_coe = malloc(pIeSharpenProfile->hgridconf.lap_mat_coe_ArraySize * sizeof(uint8_t));
              readCamCalibDbIq(pIeSharpenProfile->hgridconf.lap_mat_coe, pIeSharpenProfile->hgridconf.lap_mat_coe_ArraySize * sizeof(uint8_t));
          }
      }
      pIeSharpenProfile->p_next = pNew;
      pIeSharpenProfile = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadDySetpointList(List* l) {
  CamCalibAecDynamicSetpoint_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if (!ListEmpty(l)) {
    CamCalibAecDynamicSetpoint_t* pDySetpoint = malloc(sizeof(CamCalibAecDynamicSetpoint_t));
    l->p_next = (List*)pDySetpoint;
    readCamCalibDbIq(pDySetpoint, sizeof(CamCalibAecDynamicSetpoint_t));
    if(pDySetpoint->pDySetpoint != NULL) {
      pDySetpoint->pDySetpoint = malloc(pDySetpoint->array_size * sizeof(float));
      readCamCalibDbIq(pDySetpoint->pDySetpoint, pDySetpoint->array_size * sizeof(float));
    }
    if(pDySetpoint->pExpValue != NULL) {
      pDySetpoint->pExpValue = malloc(pDySetpoint->array_size * sizeof(float));
      readCamCalibDbIq(pDySetpoint->pExpValue, pDySetpoint->array_size * sizeof(float));
    }
    while (pDySetpoint->p_next) {
      pNew = malloc(sizeof(CamCalibAecDynamicSetpoint_t));
      readCamCalibDbIq(pNew, sizeof(CamCalibAecDynamicSetpoint_t));
      if(pNew->pDySetpoint != NULL) {
        pNew->pDySetpoint = malloc(pNew->array_size * sizeof(float));
        readCamCalibDbIq(pNew->pDySetpoint, pNew->array_size * sizeof(float));
      }
      if(pNew->pExpValue != NULL) {
        pNew->pExpValue = malloc(pNew->array_size * sizeof(float));
        readCamCalibDbIq(pNew->pExpValue, pNew->array_size * sizeof(float));
      }

      pDySetpoint->p_next = pNew;
      pDySetpoint = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

static void LoadExpSeparateList(List* l) {
  CamCalibAecExpSeparate_t* pNew;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif

  if (!ListEmpty(l)) {
    CamCalibAecExpSeparate_t* pExpSeparate = malloc(sizeof(CamCalibAecExpSeparate_t));
    l->p_next = (List*)pExpSeparate;
    readCamCalibDbIq(pExpSeparate, sizeof(CamCalibAecExpSeparate_t));
    while (pExpSeparate->p_next) {
      pNew = malloc(sizeof(CamCalibAecExpSeparate_t));
      readCamCalibDbIq(pNew, sizeof(CamCalibAecExpSeparate_t));

      pExpSeparate->p_next = pNew;
      pExpSeparate = pNew;
    }
  }

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit): file pos 0x%x\n", __FUNCTION__, getCamCalibDbIqIdx());
#endif
}

RESULT CamCalibDbLoadFile
(
    CamCalibDbHandle_t*  hCamCalibDb,
    const char* CamCalibDbIqData
) {
  char *pIqBuf;
  CamCalibDbContext_t* pCamCalibDbCtx;
  RESULT result;
  List* l;

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (enter)\n", __FUNCTION__);
#endif

  if (GetXmlDbDir() == NULL || initCamCalibDbIq(CamCalibDbIqData) != RET_SUCCESS)
    return (RET_FAILURE);
  pCamCalibDbCtx = malloc(sizeof(CamCalibDbContext_t));
  readCamCalibDbIq(pCamCalibDbCtx, sizeof(CamCalibDbContext_t));
  LoadResolutionList(&pCamCalibDbCtx->resolution);
  pCamCalibDbCtx->pAwbProfile = malloc(sizeof(CamCalibAwbPara_t));
  readCamCalibDbIq(pCamCalibDbCtx->pAwbProfile, sizeof(CamCalibAwbPara_t));
#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s:%d: file pos 0x%x\n", __FUNCTION__, __LINE__, getCamCalibDbIqIdx());
#endif
  LoadAwb_V10_GlobalList(&pCamCalibDbCtx->pAwbProfile->Para_V10.awb_global);
  LoadAwb_V10_IlluminationList(&pCamCalibDbCtx->pAwbProfile->Para_V10.illumination);
  LoadAwb_V11_GlobalList(&pCamCalibDbCtx->pAwbProfile->Para_V11.awb_global);
  LoadAwb_V11_IlluminationList(&pCamCalibDbCtx->pAwbProfile->Para_V11.illumination);
  if (pCamCalibDbCtx->pAfGlobal) {
    pCamCalibDbCtx->pAfGlobal = malloc(sizeof(CamCalibAfGlobal_t));
    readCamCalibDbIq(pCamCalibDbCtx->pAfGlobal, sizeof(CamCalibAfGlobal_t));
    if (pCamCalibDbCtx->pAfGlobal->contrast_af.FullSteps > 0) {
       pCamCalibDbCtx->pAfGlobal->contrast_af.FullRangeTbl =
         malloc(pCamCalibDbCtx->pAfGlobal->contrast_af.FullSteps * sizeof(uint16_t));
       readCamCalibDbIq(pCamCalibDbCtx->pAfGlobal->contrast_af.FullRangeTbl,
         pCamCalibDbCtx->pAfGlobal->contrast_af.FullSteps * sizeof(uint16_t));
    }

    if (pCamCalibDbCtx->pAfGlobal->contrast_af.AdaptiveSteps > 0) {
       pCamCalibDbCtx->pAfGlobal->contrast_af.AdaptRangeTbl =
         malloc(pCamCalibDbCtx->pAfGlobal->contrast_af.FullSteps * sizeof(uint16_t));
       readCamCalibDbIq(pCamCalibDbCtx->pAfGlobal->contrast_af.AdaptRangeTbl,
         pCamCalibDbCtx->pAfGlobal->contrast_af.AdaptiveSteps * sizeof(uint16_t));
    }
  }
#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s:%d: file pos 0x%x\n", __FUNCTION__, __LINE__, getCamCalibDbIqIdx());
#endif
  if (pCamCalibDbCtx->pAecGlobal) {
    pCamCalibDbCtx->pAecGlobal = malloc(sizeof(CamCalibAecGlobal_t));
    readCamCalibDbIq(pCamCalibDbCtx->pAecGlobal, sizeof(CamCalibAecGlobal_t));
    if(pCamCalibDbCtx->pAecGlobal->GridWeights.ArraySize != 0){
       pCamCalibDbCtx->pAecGlobal->GridWeights.pWeight =
         malloc(pCamCalibDbCtx->pAecGlobal->GridWeights.ArraySize * sizeof(uint8_t));
       readCamCalibDbIq(pCamCalibDbCtx->pAecGlobal->GridWeights.pWeight,
         pCamCalibDbCtx->pAecGlobal->GridWeights.ArraySize * sizeof(uint8_t));
    }
    if(pCamCalibDbCtx->pAecGlobal->NightGridWeights.ArraySize != 0){
       pCamCalibDbCtx->pAecGlobal->NightGridWeights.pWeight =
         malloc(pCamCalibDbCtx->pAecGlobal->NightGridWeights.ArraySize * sizeof(uint8_t));
       readCamCalibDbIq(pCamCalibDbCtx->pAecGlobal->NightGridWeights.pWeight,
         pCamCalibDbCtx->pAecGlobal->NightGridWeights.ArraySize * sizeof(uint8_t));
    }
    if(pCamCalibDbCtx->pAecGlobal->GainRange.array_size != 0){
       pCamCalibDbCtx->pAecGlobal->GainRange.pGainRange =
         malloc(pCamCalibDbCtx->pAecGlobal->GainRange.array_size * sizeof(float));
       readCamCalibDbIq(pCamCalibDbCtx->pAecGlobal->GainRange.pGainRange,
         pCamCalibDbCtx->pAecGlobal->GainRange.array_size * sizeof(float));
    }
    LoadDySetpointList(&pCamCalibDbCtx->pAecGlobal->DySetpointList);
    LoadExpSeparateList(&pCamCalibDbCtx->pAecGlobal->ExpSeparateList);
  }
#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s:%d: file pos 0x%x\n", __FUNCTION__, __LINE__, getCamCalibDbIqIdx());
#endif

  if (pCamCalibDbCtx->pWdrGlobal) {
    pCamCalibDbCtx->pWdrGlobal = malloc(sizeof(CamCalibWdrGlobal_t));
    readCamCalibDbIq(pCamCalibDbCtx->pWdrGlobal, sizeof(CamCalibWdrGlobal_t));
    if (pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.pfMaxGain_level != NULL) {
      pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.pfMaxGain_level =
        malloc(sizeof(float) * pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.nSize);
      readCamCalibDbIq(pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.pfMaxGain_level,
          sizeof(float) * pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.nSize);
    }
    if (pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.pfSensorGain_level != NULL) {
      pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.pfSensorGain_level =
        malloc(sizeof(float) * pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.nSize);
      readCamCalibDbIq(pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.pfSensorGain_level,
          sizeof(float) * pCamCalibDbCtx->pWdrGlobal->wdr_MaxGain_Level_curve.nSize);
    }
  }
#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s:%d: file pos 0x%x\n", __FUNCTION__, __LINE__, getCamCalibDbIqIdx());
#endif

  if (pCamCalibDbCtx->pCprocGlobal) {
    pCamCalibDbCtx->pCprocGlobal = malloc(sizeof(CamCprocProfile_t));
    readCamCalibDbIq(pCamCalibDbCtx->pCprocGlobal, sizeof(CamCprocProfile_t));
  }
#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s:%d: file pos 0x%x\n", __FUNCTION__, __LINE__, getCamCalibDbIqIdx());
#endif

  LoadEcmProfileList(& pCamCalibDbCtx->ecm_profile);
  LoadLscProfileList(&pCamCalibDbCtx->lsc_profile);
  LoadCcProfileList(&pCamCalibDbCtx->cc_profile);
  LoadBlsProfileList(&pCamCalibDbCtx->bls_profile);
  LoadCacProfileList(&pCamCalibDbCtx->cac_profile);
  LoadDpfProfileList(&pCamCalibDbCtx->dpf_profile);
  LoadDpccProfileList(&pCamCalibDbCtx->dpcc_profile);
  LoadGocProfileList(&pCamCalibDbCtx->gocProfile);
  LoadIeSharpenProfileList(&pCamCalibDbCtx->iesharpen_profile);
  if (pCamCalibDbCtx->pOTPGlobal) {
    pCamCalibDbCtx->pOTPGlobal = malloc(sizeof(CamOTPGlobal_t));
    readCamCalibDbIq(pCamCalibDbCtx->pOTPGlobal, sizeof(CamOTPGlobal_t));
  }

  *hCamCalibDb = (CamCalibDbHandle_t)pCamCalibDbCtx;

#ifndef USE_C_SOURCE_XML_BIN
  // free gpCamCalibDbIqData allocated in func initCamCalibDbIq
  if (gpCamCalibDbIqData) {
    free((void*)gpCamCalibDbIqData);
    gpCamCalibDbIqData = NULL;
  }
#endif

#ifdef LOAD_IQ_TRACE_INFO_ON
  LOGD( "%s (exit)\n", __FUNCTION__);
#endif

  return (RET_SUCCESS);
}

/******************************************************************************
 * See header file for detailed comment.
 *****************************************************************************/

/******************************************************************************
 * ClearEcmProfileList
 *****************************************************************************/
void ClearDySetpointList(List* l) {
  if (!ListEmpty(l)) {
	CamCalibAecDynamicSetpoint_t* pDySetpoint = (CamCalibAecDynamicSetpoint_t*)ListRemoveHead(l);
	while (pDySetpoint) {
	  if(pDySetpoint->pDySetpoint != NULL)
		free(pDySetpoint->pDySetpoint);

	  if(pDySetpoint->pExpValue != NULL)
		free(pDySetpoint->pExpValue);

	  /* 2.) free item */
	  free(pDySetpoint);

	  /* 3.) get next item */
	  pDySetpoint = (CamCalibAecDynamicSetpoint_t*)ListRemoveHead(l);
	}
  }

  ListInit(l);
}
/******************************************************************************
 * ClearEcmProfileList
 *****************************************************************************/
void ClearExpSeparateList(List* l) {
  if (!ListEmpty(l)) {
	CamCalibAecExpSeparate_t* pExpSeparate = (CamCalibAecExpSeparate_t*)ListRemoveHead(l);
	while (pExpSeparate) {

	  /* 2.) free item */
	  free(pExpSeparate);

	  /* 3.) get next item */
	  pExpSeparate = (CamCalibAecExpSeparate_t*)ListRemoveHead(l);
	}
  }

  ListInit(l);
}

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
  pCamCalibDbCtx = (CamCalibDbContext_t *)malloc(sizeof(CamCalibDbContext_t));
  if (pCamCalibDbCtx == NULL) {
    LOGE("%s (allocating control context failed)\n", __func__);
    return (RET_OUTOFMEM);
  }
  MEMSET(pCamCalibDbCtx, 0, sizeof(CamCalibDbContext_t));
  ListInit(&pCamCalibDbCtx->resolution);
  pCamCalibDbCtx->pAwbProfile = (CamCalibAwbPara_t*)malloc(sizeof(CamCalibAwbPara_t));
  ListInit(&pCamCalibDbCtx->pAwbProfile->Para_V11.awb_global);
  ListInit(&pCamCalibDbCtx->pAwbProfile->Para_V10.awb_global);
  pCamCalibDbCtx->pAecGlobal = NULL;
  pCamCalibDbCtx->pAfGlobal = NULL;
  pCamCalibDbCtx->pOTPGlobal = NULL;
  ListInit(&pCamCalibDbCtx->ecm_profile);
  ListInit(&pCamCalibDbCtx->pAwbProfile->Para_V11.illumination);
  ListInit(&pCamCalibDbCtx->pAwbProfile->Para_V10.illumination);
  ListInit(&pCamCalibDbCtx->lsc_profile);
  ListInit(&pCamCalibDbCtx->cc_profile);
  ListInit(&pCamCalibDbCtx->cac_profile);
  ListInit(&pCamCalibDbCtx->dpf_profile);
  ListInit(&pCamCalibDbCtx->dpcc_profile);
  ListInit(&pCamCalibDbCtx->iesharpen_profile);

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
  pCamCalibDbCtx->isp_output_type = pMeta->isp_output_type;
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
  pMeta->isp_output_type = pCamCalibDbCtx->isp_output_type;

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
  pNewFrameRate = (CamFrameRate_t *)malloc(sizeof(CamFrameRate_t));
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

  pNewRes = (CamResolution_t *)malloc(sizeof(CamResolution_t));
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
 * CamCalibDbGetResolutionNameByIdx
 *****************************************************************************/
RESULT CamCalibDbGetResolutionNameByIdx
(
    CamCalibDbHandle_t          hCamCalibDb,
    int32_t                     idx,
    const CamResolutionName_t*  pName
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamResolution_t*     pResolution;

  RESULT result = RET_SUCCESS;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pName) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  pResolution = (CamResolution_t*)ListSearch(&pCamCalibDbCtx->resolution, SearchResolutionByIdx, (void*)&idx);
  strncpy((char*)pName, (char*)pResolution->name, sizeof(CamResolutionName_t));

  LOGV("%s: (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbAddAwbGlobal
 *****************************************************************************/
RESULT CamCalibDbAddAwb_V10_Global
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamCalibAwb_V10_Global_t* pAddAwbGlobal
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamCalibAwb_V10_Global_t* pNewAwbGlobal = NULL;

  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateAwb_V10_Data(pAddAwbGlobal);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if data already exists */
  pNewAwbGlobal = (CamCalibAwb_V10_Global_t*)ListSearch(&pCamCalibDbCtx->pAwbProfile->Para_V10.awb_global, SearchForEqualAwb_V10_Global, (void*)pAddAwbGlobal);
  if (NULL == pNewAwbGlobal) {
    CamAwbClipParm_t*        pAwbClipParam      = NULL;
    CamAwbGlobalFadeParm_t*  pAwbGlobalFadeParm = NULL;
    CamAwb_V10_Fade2Parm_t*  pAwbFade2Parm      = NULL;

    int32_t nArraySize1;
    int32_t nArraySize2;

    pNewAwbGlobal = (CamCalibAwb_V10_Global_t *)malloc(sizeof(CamCalibAwb_V10_Global_t));
    MEMCPY(pNewAwbGlobal, pAddAwbGlobal, sizeof(CamCalibAwb_V10_Global_t));

    pAwbClipParam       = &pNewAwbGlobal->AwbClipParam;
    pAwbGlobalFadeParm  = &pNewAwbGlobal->AwbGlobalFadeParm;
    pAwbFade2Parm       = &pNewAwbGlobal->AwbFade2Parm;

    // pAwbClipParam
    nArraySize1 = pAddAwbGlobal->AwbClipParam.ArraySize1;
    nArraySize2 = pAddAwbGlobal->AwbClipParam.ArraySize2;
    pAwbClipParam->pRg1 = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbClipParam->pRg1, pAddAwbGlobal->AwbClipParam.pRg1, sizeof(float) *  nArraySize1);
    pAwbClipParam->pMaxDist1 = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbClipParam->pMaxDist1, pAddAwbGlobal->AwbClipParam.pMaxDist1, sizeof(float) *  nArraySize1);
    pAwbClipParam->pRg2 = (float *)malloc(sizeof(float) *  nArraySize2);
    MEMCPY(pAwbClipParam->pRg2, pAddAwbGlobal->AwbClipParam.pRg2, sizeof(float) *  nArraySize2);
    pAwbClipParam->pMaxDist2 = (float *)malloc(sizeof(float) *  nArraySize2);
    MEMCPY(pAwbClipParam->pMaxDist2, pAddAwbGlobal->AwbClipParam.pMaxDist2, sizeof(float) *  nArraySize2);

    // pAwbGlobalFadeParm
    nArraySize1 = pAddAwbGlobal->AwbGlobalFadeParm.ArraySize1;
    nArraySize2 = pAddAwbGlobal->AwbGlobalFadeParm.ArraySize2;
    pAwbGlobalFadeParm->pGlobalFade1 = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbGlobalFadeParm->pGlobalFade1, pAddAwbGlobal->AwbGlobalFadeParm.pGlobalFade1, sizeof(float) *  nArraySize1);
    pAwbGlobalFadeParm->pGlobalGainDistance1 = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbGlobalFadeParm->pGlobalGainDistance1, pAddAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance1, sizeof(float) *  nArraySize1);
    pAwbGlobalFadeParm->pGlobalFade2 = (float *)malloc(sizeof(float) *  nArraySize2);
    MEMCPY(pAwbGlobalFadeParm->pGlobalFade2, pAddAwbGlobal->AwbGlobalFadeParm.pGlobalFade2, sizeof(float) *  nArraySize2);
    pAwbGlobalFadeParm->pGlobalGainDistance2 = (float *)malloc(sizeof(float) *  nArraySize2);
    MEMCPY(pAwbGlobalFadeParm->pGlobalGainDistance2, pAddAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance2, sizeof(float) *  nArraySize2);

    // pAwbFade2Parm
    nArraySize1 = pAddAwbGlobal->AwbFade2Parm.ArraySize;
    nArraySize2 = 0l;
    pAwbFade2Parm->pFade                = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pFade, pAddAwbGlobal->AwbFade2Parm.pFade, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pCbMinRegionMax      = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pCbMinRegionMax, pAddAwbGlobal->AwbFade2Parm.pCbMinRegionMax, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pCrMinRegionMax      = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pCrMinRegionMax, pAddAwbGlobal->AwbFade2Parm.pCrMinRegionMax, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMaxCSumRegionMax    = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pMaxCSumRegionMax, pAddAwbGlobal->AwbFade2Parm.pMaxCSumRegionMax, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pCbMinRegionMin      = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pCbMinRegionMin, pAddAwbGlobal->AwbFade2Parm.pCbMinRegionMin, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pCrMinRegionMin      = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pCrMinRegionMin, pAddAwbGlobal->AwbFade2Parm.pCrMinRegionMin, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMaxCSumRegionMin    = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pMaxCSumRegionMin, pAddAwbGlobal->AwbFade2Parm.pMaxCSumRegionMin, sizeof(float) *  nArraySize1);

    pAwbFade2Parm->pMinCRegionMax = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pMinCRegionMax, pAddAwbGlobal->AwbFade2Parm.pMinCRegionMax, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMinCRegionMin = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pMinCRegionMin, pAddAwbGlobal->AwbFade2Parm.pMinCRegionMin, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMaxYRegionMax = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pMaxYRegionMax, pAddAwbGlobal->AwbFade2Parm.pMaxYRegionMax, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMaxYRegionMin = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pMaxYRegionMin, pAddAwbGlobal->AwbFade2Parm.pMaxYRegionMin, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMinYMaxGRegionMax = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pMinYMaxGRegionMax, pAddAwbGlobal->AwbFade2Parm.pMinYMaxGRegionMax, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMinYMaxGRegionMin = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pMinYMaxGRegionMin, pAddAwbGlobal->AwbFade2Parm.pMinYMaxGRegionMin, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pRefCb = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pRefCb, pAddAwbGlobal->AwbFade2Parm.pRefCb, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pRefCr = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pRefCr, pAddAwbGlobal->AwbFade2Parm.pRefCr, sizeof(float) *  nArraySize1);

    ListPrepareItem(pNewAwbGlobal);
    ListAddTail(&pCamCalibDbCtx->pAwbProfile->Para_V10.awb_global, pNewAwbGlobal);
  } else {
    return (RET_INVALID_PARM);
  }

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbAddAwb_VersionName
 *****************************************************************************/
RESULT CamCalibDbAddAwb_VersionName
(
    CamCalibDbHandle_t  hCamCalibDb,
    CAM_AwbVersion_t vName
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamCalibAwb_V10_Global_t* pNewAwbGlobal = NULL;

  RESULT result;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  LOGV( "valid_version :%d \n", vName);
  pCamCalibDbCtx->pAwbProfile->valid_version =	vName;


  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetAwb_FlashProfiles
 *****************************************************************************/
RESULT CamCalibDbGetAwb_FlashProfiles
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamAwbPara_Flash_t **flash
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  RESULT result;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }
  *flash = &pCamCalibDbCtx->pAwbProfile->Para_Flash;

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbGetAwb_FlashProfiles
 *****************************************************************************/
RESULT CamCalibDbAddAwb_FlashProfiles
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamAwbPara_Flash_t  flash
) {
    CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

    LOGV( "%s (enter)\n", __func__);

    if (NULL == pCamCalibDbCtx) {
      return (RET_WRONG_HANDLE);
    }

    MEMCPY(&pCamCalibDbCtx->pAwbProfile->Para_Flash,&flash,sizeof(CamAwbPara_Flash_t));

    return (RET_SUCCESS);
    LOGV( "%s (exit)\n", __func__);
}


/******************************************************************************
 * CamCalibDbGetAwb_VersionName
 *****************************************************************************/
RESULT CamCalibDbGetAwb_VersionName
(
    CamCalibDbHandle_t  hCamCalibDb,
    CAM_AwbVersion_t *vName
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamCalibAwb_V10_Global_t* pNewAwbGlobal = NULL;

  RESULT result;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  *vName = pCamCalibDbCtx->pAwbProfile->valid_version;

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbAddAwb_V11_Global
 *****************************************************************************/
RESULT CamCalibDbAddAwb_V11_Global
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamCalibAwb_V11_Global_t* pAddAwbGlobal
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamCalibAwb_V11_Global_t* pNewAwbGlobal = NULL;

  RESULT result;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateAwb_V11_Data(pAddAwbGlobal);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if data already exists */
  pNewAwbGlobal = (CamCalibAwb_V11_Global_t*)ListSearch(&pCamCalibDbCtx->pAwbProfile->Para_V11.awb_global, SearchForEqualAwb_V11_Global, (void*)pAddAwbGlobal);
  if (NULL == pNewAwbGlobal) {
    CamAwbClipParm_t*        pAwbClipParam      = NULL;
    CamAwbGlobalFadeParm_t*  pAwbGlobalFadeParm = NULL;
    CamAwb_V11_Fade2Parm_t*  pAwbFade2Parm      = NULL;

    int32_t nArraySize1;
    int32_t nArraySize2;

    pNewAwbGlobal = (CamCalibAwb_V11_Global_t *)malloc(sizeof(CamCalibAwb_V11_Global_t));
    MEMCPY(pNewAwbGlobal, pAddAwbGlobal, sizeof(CamCalibAwb_V11_Global_t));

    pAwbClipParam       = &pNewAwbGlobal->AwbClipParam;
    pAwbGlobalFadeParm  = &pNewAwbGlobal->AwbGlobalFadeParm;
    pAwbFade2Parm       = &pNewAwbGlobal->AwbFade2Parm;

    // pAwbClipParam
    nArraySize1 = pAddAwbGlobal->AwbClipParam.ArraySize1;
    nArraySize2 = pAddAwbGlobal->AwbClipParam.ArraySize2;
    pAwbClipParam->pRg1 = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbClipParam->pRg1, pAddAwbGlobal->AwbClipParam.pRg1, sizeof(float) *  nArraySize1);
    pAwbClipParam->pMaxDist1 = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbClipParam->pMaxDist1, pAddAwbGlobal->AwbClipParam.pMaxDist1, sizeof(float) *  nArraySize1);
    pAwbClipParam->pRg2 = (float *)malloc(sizeof(float) *  nArraySize2);
    MEMCPY(pAwbClipParam->pRg2, pAddAwbGlobal->AwbClipParam.pRg2, sizeof(float) *  nArraySize2);
    pAwbClipParam->pMaxDist2 = (float *)malloc(sizeof(float) *  nArraySize2);
    MEMCPY(pAwbClipParam->pMaxDist2, pAddAwbGlobal->AwbClipParam.pMaxDist2, sizeof(float) *  nArraySize2);

    // pAwbGlobalFadeParm
    nArraySize1 = pAddAwbGlobal->AwbGlobalFadeParm.ArraySize1;
    nArraySize2 = pAddAwbGlobal->AwbGlobalFadeParm.ArraySize2;
    pAwbGlobalFadeParm->pGlobalFade1 = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbGlobalFadeParm->pGlobalFade1, pAddAwbGlobal->AwbGlobalFadeParm.pGlobalFade1, sizeof(float) *  nArraySize1);
    pAwbGlobalFadeParm->pGlobalGainDistance1 = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbGlobalFadeParm->pGlobalGainDistance1, pAddAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance1, sizeof(float) *  nArraySize1);
    pAwbGlobalFadeParm->pGlobalFade2 = (float *)malloc(sizeof(float) *  nArraySize2);
    MEMCPY(pAwbGlobalFadeParm->pGlobalFade2, pAddAwbGlobal->AwbGlobalFadeParm.pGlobalFade2, sizeof(float) *  nArraySize2);
    pAwbGlobalFadeParm->pGlobalGainDistance2 = (float *)malloc(sizeof(float) *  nArraySize2);
    MEMCPY(pAwbGlobalFadeParm->pGlobalGainDistance2, pAddAwbGlobal->AwbGlobalFadeParm.pGlobalGainDistance2, sizeof(float) *  nArraySize2);

    // pAwbFade2Parm
    nArraySize1 = pAddAwbGlobal->AwbFade2Parm.ArraySize;
    nArraySize2 = 0l;
    pAwbFade2Parm->pFade                = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pFade, pAddAwbGlobal->AwbFade2Parm.pFade, sizeof(float) *  nArraySize1);

    pAwbFade2Parm->pMaxCSum_br = (float *)malloc(sizeof(float) * nArraySize1);
    MEMCPY(pAwbFade2Parm->pMaxCSum_br, pAddAwbGlobal->AwbFade2Parm.pMaxCSum_br, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMaxCSum_sr = (float *)malloc(sizeof(float) * nArraySize1);
    MEMCPY(pAwbFade2Parm->pMaxCSum_sr, pAddAwbGlobal->AwbFade2Parm.pMaxCSum_sr, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMinC_br    = (float *)malloc(sizeof(float) * nArraySize1);
    MEMCPY(pAwbFade2Parm->pMinC_br, pAddAwbGlobal->AwbFade2Parm.pMinC_br, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMinC_sr    = (float *)malloc(sizeof(float) * nArraySize1);
    MEMCPY(pAwbFade2Parm->pMinC_sr, pAddAwbGlobal->AwbFade2Parm.pMinC_sr, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMaxY_br    = (float *)malloc(sizeof(float) * nArraySize1);
    MEMCPY(pAwbFade2Parm->pMaxY_br, pAddAwbGlobal->AwbFade2Parm.pMaxY_br, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMaxY_sr    = (float *)malloc(sizeof(float) * nArraySize1);
    MEMCPY(pAwbFade2Parm->pMaxY_sr, pAddAwbGlobal->AwbFade2Parm.pMaxY_sr, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMinY_br    = (float *)malloc(sizeof(float) * nArraySize1);
    MEMCPY(pAwbFade2Parm->pMinY_br, pAddAwbGlobal->AwbFade2Parm.pMinY_br, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pMinY_sr    = (float *)malloc(sizeof(float) * nArraySize1);
    MEMCPY(pAwbFade2Parm->pMinY_sr, pAddAwbGlobal->AwbFade2Parm.pMinY_sr, sizeof(float) *  nArraySize1);
	pAwbFade2Parm->pRefCb = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pRefCb, pAddAwbGlobal->AwbFade2Parm.pRefCb, sizeof(float) *  nArraySize1);
    pAwbFade2Parm->pRefCr = (float *)malloc(sizeof(float) *  nArraySize1);
    MEMCPY(pAwbFade2Parm->pRefCr, pAddAwbGlobal->AwbFade2Parm.pRefCr, sizeof(float) *  nArraySize1);

    ListPrepareItem(pNewAwbGlobal);
    ListAddTail(&pCamCalibDbCtx->pAwbProfile->Para_V11.awb_global, pNewAwbGlobal);
  } else {
    return (RET_INVALID_PARM);
  }

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetAwbGlobalByResolution
 *****************************************************************************/
RESULT CamCalibDbGetAwb_V10_GlobalByResolution
(
    CamCalibDbHandle_t          hCamCalibDb,
    const CamResolutionName_t   ResName,
    CamCalibAwb_V10_Global_t**         pAwbGlobal
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
  *pAwbGlobal = (CamCalibAwb_V10_Global_t*)ListSearch(&pCamCalibDbCtx->pAwbProfile->Para_V10.awb_global, SearchAwb_V10_GlobalByResolution, (void*)ResName);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbGetAwb_V11_GlobalByResolution
 *****************************************************************************/
RESULT CamCalibDbGetAwb_V11_GlobalByResolution
(
    CamCalibDbHandle_t          hCamCalibDb,
    const CamResolutionName_t   ResName,
    CamCalibAwb_V11_Global_t**         pAwbGlobal
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  RESULT result;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pAwbGlobal) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pAwbGlobal = (CamCalibAwb_V11_Global_t*)ListSearch(&pCamCalibDbCtx->pAwbProfile->Para_V11.awb_global, SearchAwb_V11_GlobalByResolution, (void*)ResName);

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbAddAfGlobal
 *****************************************************************************/
RESULT CamCalibDbAddAfGlobal
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamCalibAfGlobal_t* pAddAfGlobal
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  RESULT result = RET_SUCCESS;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  /* check if data already exists */
  if (NULL != pCamCalibDbCtx->pAfGlobal) {
    return (RET_INVALID_PARM);
  }

  /* finally allocate, copy & add data */
  CamCalibAfGlobal_t* pNewAfGlobal = (CamCalibAfGlobal_t *)malloc(sizeof(CamCalibAfGlobal_t));
  if (NULL == pNewAfGlobal) {
    return (RET_OUTOFMEM);
  }

  MEMCPY(pNewAfGlobal, pAddAfGlobal, sizeof(CamCalibAfGlobal_t));

  pCamCalibDbCtx->pAfGlobal = pNewAfGlobal;

  LOGV( "%s (exit) %d\n", __func__, result);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbGetAfGlobal
 *****************************************************************************/
RESULT CamCalibDbGetAfGlobal
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamCalibAfGlobal_t** ppAfGlobal
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  RESULT result;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == ppAfGlobal) {
    return (RET_INVALID_PARM);
  }

  /* return reference to global AEC configuration */
  *ppAfGlobal = pCamCalibDbCtx->pAfGlobal;

  LOGV( "%s (exit)\n", __func__);

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
  CamCalibAecGlobal_t* pNewAecGlobal = (CamCalibAecGlobal_t *)malloc(sizeof(CamCalibAecGlobal_t));
  if (NULL == pNewAecGlobal) {
    return (RET_OUTOFMEM);
  }
  MEMCPY(pNewAecGlobal, pAddAecGlobal, sizeof(CamCalibAecGlobal_t));
  ListInit(&pNewAecGlobal->DySetpointList);   // clear possibly not empty schemes list in copy
  ListInit(&pNewAecGlobal->ExpSeparateList);   // clear possibly not empty scheme

  /* add already linked DySetpoint as well */
  CamCalibAecDynamicSetpoint_t* pDySetpoint = (CamCalibAecDynamicSetpoint_t*)ListHead(&pAddAecGlobal->DySetpointList);
  while (pDySetpoint) {
    result = CamCalibDbAddDySetpoint(hCamCalibDb, pNewAecGlobal, pDySetpoint);
    if (result != RET_SUCCESS) {
      return (result);
    }

    pDySetpoint = (CamCalibAecDynamicSetpoint_t*)pDySetpoint->p_next;
  }

  /* add already linked ExpSeparate as well */
  CamCalibAecExpSeparate_t* pExpSeparate = (CamCalibAecExpSeparate_t*)ListHead(&pAddAecGlobal->ExpSeparateList);
  while (pExpSeparate) {
    result = CamCalibDbAddExpSeparate(hCamCalibDb, pNewAecGlobal, pExpSeparate);
    if (result != RET_SUCCESS) {
      return (result);
    }

    pExpSeparate = (CamCalibAecExpSeparate_t*)pExpSeparate->p_next;
  }
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
  pNewEcmProfile = (CamEcmProfile_t *)malloc(sizeof(CamEcmProfile_t));
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
  pNewEcmScheme = (CamEcmScheme_t *)malloc(sizeof(CamEcmScheme_t));
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

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbAddDySetpoint
 *****************************************************************************/
RESULT CamCalibDbAddDySetpoint
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamCalibAecGlobal_t* pAecGlobal,
    CamCalibAecDynamicSetpoint_t* pAddDySetpoint
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamCalibAecDynamicSetpoint_t* pNewDySetpoint = NULL;
  float* pExpValue;
  float* pDySetpoint;

  RESULT result;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pAecGlobal) {
    return (RET_INVALID_PARM);
  }

  result = ValidateDySetpoint(pAddDySetpoint);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if data already exists */
  pNewDySetpoint = (CamCalibAecDynamicSetpoint_t*)ListSearch(&pAecGlobal->DySetpointList, SearchForEqualDySetpointProfile, (void*)pAddDySetpoint);
  if (NULL != pNewDySetpoint) {
    return (RET_NOTAVAILABLE);
  }

  /* finally allocate, copy & add scheme */
  pNewDySetpoint = (CamCalibAecDynamicSetpoint_t *)malloc(sizeof(CamCalibAecDynamicSetpoint_t));
  if (NULL == pNewDySetpoint) {
    return (RET_OUTOFMEM);
  }
  MEMCPY(pNewDySetpoint, pAddDySetpoint, sizeof(CamCalibAecDynamicSetpoint_t));

  if (0 != pAddDySetpoint->array_size) {
    pDySetpoint = (float *)malloc(pAddDySetpoint->array_size * sizeof(float));
    if (NULL == pDySetpoint) {
      free(pNewDySetpoint);
      return (RET_OUTOFMEM);
    }
    pExpValue = (float *)malloc(pAddDySetpoint->array_size * sizeof(float));
    if (NULL == pExpValue) {
      free(pNewDySetpoint);
      free(pDySetpoint);
      return (RET_OUTOFMEM);
    }

    pNewDySetpoint->pDySetpoint = pDySetpoint;
    pNewDySetpoint->pExpValue = pExpValue;
    MEMCPY(pNewDySetpoint->pDySetpoint, pAddDySetpoint->pDySetpoint, pAddDySetpoint->array_size * sizeof(float));
    MEMCPY(pNewDySetpoint->pExpValue, pAddDySetpoint->pExpValue, pAddDySetpoint->array_size * sizeof(float));
  }

  ListPrepareItem(pNewDySetpoint);
  ListAddTail(&pAecGlobal->DySetpointList, pNewDySetpoint);

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbGetNoOfDySetpoint
 *****************************************************************************/
RESULT CamCalibDbGetNoOfDySetpoint
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamCalibAecGlobal_t*         pAecGlobal,
    int32_t*                 no
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pAecGlobal) {
    return (RET_INVALID_PARM);
  }

  if (NULL == no) {
    return (RET_INVALID_PARM);
  }

  *no = (uint32_t)ListNoItems(&pAecGlobal->DySetpointList);

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbGetEcmSchemeByName
 *****************************************************************************/
RESULT CamCalibDbGetDySetpointByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamCalibAecGlobal_t*         pAecGlobal,
    CamDynamicSetpointName_t      DySetpointName,
    CamCalibAecDynamicSetpoint_t**          ppDySetpoint
)
{
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == ppDySetpoint) {
    return (RET_INVALID_PARM);
  }

  /* search scheme by name */
  *ppDySetpoint = (CamCalibAecDynamicSetpoint_t*)ListSearch(&pAecGlobal->DySetpointList, SearchDySetpointProfileByName, (void*)DySetpointName);

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbGetEcmSchemeByIdx
 *****************************************************************************/
RESULT CamCalibDbGetDySetpointByIdx
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamCalibAecGlobal_t*         pAecGlobal,
    const uint32_t          idx,
    CamCalibAecDynamicSetpoint_t**          ppDySetpoint
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pAecGlobal) {
    return (RET_INVALID_PARM);
  }

  if (NULL == ppDySetpoint) {
    return (RET_INVALID_PARM);
  }

  /* search EC scheme by index */
  *ppDySetpoint = (CamCalibAecDynamicSetpoint_t*)ListGetItemByIdx(&pAecGlobal->DySetpointList, idx);

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbAddExpSeparate
 *****************************************************************************/
RESULT CamCalibDbAddExpSeparate
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamCalibAecGlobal_t* pAecGlobal,
    CamCalibAecExpSeparate_t* pAddExpSeparate
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamCalibAecExpSeparate_t* pNewExpSeparate = NULL;

  RESULT result;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pAecGlobal) {
    return (RET_INVALID_PARM);
  }

  result = ValidateExpSeparate(pAddExpSeparate);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if data already exists */
  pNewExpSeparate = (CamCalibAecExpSeparate_t*)ListSearch(&pAecGlobal->ExpSeparateList, SearchForEqualExpSeparateProfile, (void*)pAddExpSeparate);
  if (NULL != pNewExpSeparate) {
    return (RET_NOTAVAILABLE);
  }

  /* finally allocate, copy & add scheme */
  pNewExpSeparate = (CamCalibAecExpSeparate_t *)malloc(sizeof(CamCalibAecExpSeparate_t));
  if (NULL == pNewExpSeparate) {
    return (RET_OUTOFMEM);
  }
  MEMCPY(pNewExpSeparate, pAddExpSeparate, sizeof(CamCalibAecExpSeparate_t));

  ListPrepareItem(pNewExpSeparate);
  ListAddTail(&pAecGlobal->ExpSeparateList, pNewExpSeparate);

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbGetNoOfExpSeparate
 *****************************************************************************/
RESULT CamCalibDbGetNoOfExpSeparate
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamCalibAecGlobal_t*         pAecGlobal,
    int32_t*                 no
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pAecGlobal) {
    return (RET_INVALID_PARM);
  }

  if (NULL == no) {
    return (RET_INVALID_PARM);
  }

  *no = (uint32_t)ListNoItems(&pAecGlobal->ExpSeparateList);

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbGetExpSeparateByName
 *****************************************************************************/
RESULT CamCalibDbGetExpSeparateByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamCalibAecGlobal_t*         pAecGlobal,
    CamExpSeparateName_t      ExpSeparateName,
    CamCalibAecExpSeparate_t**          ppExpSeparate
)
{
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == ppExpSeparate) {
    return (RET_INVALID_PARM);
  }

  /* search scheme by name */
  *ppExpSeparate = (CamCalibAecExpSeparate_t*)ListSearch(&pAecGlobal->ExpSeparateList, SearchExpSeparateProfileByName, (void*)ExpSeparateName);

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbGetExpSeparateByIdx
 *****************************************************************************/
RESULT CamCalibDbGetExpSeparateByIdx
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamCalibAecGlobal_t*         pAecGlobal,
    const uint32_t          idx,
    CamCalibAecExpSeparate_t**          ppExpSeparate
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pAecGlobal) {
    return (RET_INVALID_PARM);
  }

  if (NULL == ppExpSeparate) {
    return (RET_INVALID_PARM);
  }

  /* search EC scheme by index */
  *ppExpSeparate = (CamCalibAecExpSeparate_t*)ListGetItemByIdx(&pAecGlobal->ExpSeparateList, idx);

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbGetNoOfIlluminations
 *****************************************************************************/
RESULT CamCalibDbGetNoOfAwb_V11_Illuminations
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
  *no = (uint32_t)ListNoItems(&pCamCalibDbCtx->pAwbProfile->Para_V11.illumination);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbGetNoOfAwb_V10_Illuminations
 *****************************************************************************/
RESULT CamCalibDbGetNoOfAwb_V10_Illuminations
(
    CamCalibDbHandle_t  hCamCalibDb,
    int32_t*             no
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == no) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *no = (uint32_t)ListNoItems(&pCamCalibDbCtx->pAwbProfile->Para_V10.illumination);

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbGetNoOfAwbIlluminations
 *****************************************************************************/
RESULT CamCalibDbGetNoOfAwbIlluminations
(
    CamCalibDbHandle_t  hCamCalibDb,
    int32_t*             pNo
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  RESULT result = RET_SUCCESS;
  LOGV( "%s (enter)\n", __func__);

  switch (pCamCalibDbCtx->pAwbProfile->valid_version)
  {
	case CAM_AWB_VERSION_11:
		// get number of availabe illumination profiles from database
		result = CamCalibDbGetNoOfAwb_V11_Illuminations(hCamCalibDb, pNo);
		break;
	default :
		// get number of availabe illumination profiles from database
		result = CamCalibDbGetNoOfAwb_V10_Illuminations(hCamCalibDb, pNo);
  }

  LOGV( "%s (exit)\n", __func__);

  return (result);
}

/******************************************************************************
 * CamCalibDbAddAwb_V11_Illumination
 *****************************************************************************/
RESULT CamCalibDbAddAwb_V11_Illumination
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamAwb_V11_IlluProfile_t*    pAddIllu
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamAwb_V11_IlluProfile_t* pNewIllu = NULL;

  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateAwb_V11_Illumination(pAddIllu);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if resolution already exists */
  pNewIllu = (CamAwb_V11_IlluProfile_t*)ListSearch(&pCamCalibDbCtx->pAwbProfile->Para_V11.illumination, SearchForEqualAwb_V11_Illumination, (void*)pAddIllu);
  if (NULL == pNewIllu) {
    /* allocate and copy the illumination profile */
    pNewIllu = (CamAwb_V11_IlluProfile_t*)malloc(sizeof(CamAwb_V11_IlluProfile_t));
    MEMCPY(pNewIllu, pAddIllu, sizeof(CamAwb_V11_IlluProfile_t));

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
    pNewIllu->SaturationCurve.pSensorGain = (float *)malloc(n_memsize);
    pNewIllu->SaturationCurve.pSaturation = (float *)malloc(n_memsize);
    MEMCPY(pNewIllu->SaturationCurve.pSensorGain, pAddIllu->SaturationCurve.pSensorGain, n_memsize);
    MEMCPY(pNewIllu->SaturationCurve.pSaturation, pAddIllu->SaturationCurve.pSaturation, n_memsize);

    /* vignetting over gain curve */
    n_items = pAddIllu->VignettingCurve.ArraySize;
    n_memsize = (n_items * sizeof(float));
    pNewIllu->VignettingCurve.ArraySize = n_items;
    pNewIllu->VignettingCurve.pSensorGain = (float *)malloc(n_memsize);
    pNewIllu->VignettingCurve.pVignetting = (float *)malloc(n_memsize);
    MEMCPY(pNewIllu->VignettingCurve.pSensorGain, pAddIllu->VignettingCurve.pSensorGain, n_memsize);
    MEMCPY(pNewIllu->VignettingCurve.pVignetting, pAddIllu->VignettingCurve.pVignetting, n_memsize);

    /* add illumination to list */
    ListPrepareItem(pNewIllu);
    ListAddTail(&pCamCalibDbCtx->pAwbProfile->Para_V11.illumination, pNewIllu);
  } else {
    return (RET_INVALID_PARM);
  }

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbAddAwb_V10_Illumination
 *****************************************************************************/
RESULT CamCalibDbAddAwb_V10_Illumination
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamAwb_V10_IlluProfile_t*    pAddIllu
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamAwb_V10_IlluProfile_t* pNewIllu = NULL;

  RESULT result;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateAwb_V10_Illumination(pAddIllu);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if resolution already exists */
  pNewIllu = (CamAwb_V10_IlluProfile_t*)ListSearch(&pCamCalibDbCtx->pAwbProfile->Para_V10.illumination, SearchForEqualAwb_V10_Illumination, (void*)pAddIllu);
  if (NULL == pNewIllu) {
    /* allocate and copy the illumination profile */
    pNewIllu = (CamAwb_V10_IlluProfile_t*)malloc(sizeof(CamAwb_V10_IlluProfile_t));
    MEMCPY(pNewIllu, pAddIllu, sizeof(CamAwb_V10_IlluProfile_t));

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
    pNewIllu->SaturationCurve.pSensorGain = (float *)malloc(n_memsize);
    pNewIllu->SaturationCurve.pSaturation = (float *)malloc(n_memsize);
    MEMCPY(pNewIllu->SaturationCurve.pSensorGain, pAddIllu->SaturationCurve.pSensorGain, n_memsize);
    MEMCPY(pNewIllu->SaturationCurve.pSaturation, pAddIllu->SaturationCurve.pSaturation, n_memsize);

    /* vignetting over gain curve */
    n_items = pAddIllu->VignettingCurve.ArraySize;
    n_memsize = (n_items * sizeof(float));
    pNewIllu->VignettingCurve.ArraySize = n_items;
    pNewIllu->VignettingCurve.pSensorGain = (float *)malloc(n_memsize);
    pNewIllu->VignettingCurve.pVignetting = (float *)malloc(n_memsize);
    MEMCPY(pNewIllu->VignettingCurve.pSensorGain, pAddIllu->VignettingCurve.pSensorGain, n_memsize);
    MEMCPY(pNewIllu->VignettingCurve.pVignetting, pAddIllu->VignettingCurve.pVignetting, n_memsize);


    /* add illumination to list */
    ListPrepareItem(pNewIllu);
    ListAddTail(&pCamCalibDbCtx->pAwbProfile->Para_V10.illumination, pNewIllu);
  } else {
    return (RET_INVALID_PARM);
  }

  LOGV( "%s (exit)\n", __func__);
  return (RET_SUCCESS);
}
/******************************************************************************
 * CamCalibDbGetAwb_V11_IlluminationByName
 *****************************************************************************/
RESULT CamCalibDbGetAwb_V11_IlluminationByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamIlluminationName_t   name,
    CamAwb_V11_IlluProfile_t**        pIllumination
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
  *pIllumination = (CamAwb_V11_IlluProfile_t*)ListSearch(&pCamCalibDbCtx->pAwbProfile->Para_V11.illumination, SearchAwb_V11_IlluminationByName, (void*)name);

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetAwb_V10_IlluminationByName
 *****************************************************************************/
RESULT CamCalibDbGetAwb_V10_IlluminationByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamIlluminationName_t   name,
    CamAwb_V10_IlluProfile_t**        pIllumination
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pIllumination) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pIllumination = (CamAwb_V10_IlluProfile_t*)ListSearch(&pCamCalibDbCtx->pAwbProfile->Para_V10.illumination, SearchAwb_V10_IlluminationByName, (void*)name);

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbGetAwbIlluminationNameByIdx
 *****************************************************************************/
RESULT CamCalibDbGetAwbIlluminationNameByIdx
(
    CamCalibDbHandle_t  hCamCalibDb,
    const uint32_t      idx,
    char*               illName
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  RESULT result = RET_SUCCESS;
  LOGV( "%s (enter)\n", __func__);
  CamAwb_V11_IlluProfile_t* pIlluProfile11 = NULL;
  CamAwb_V10_IlluProfile_t* pIlluProfile10 = NULL;
  switch (pCamCalibDbCtx->pAwbProfile->valid_version)
  {
	case CAM_AWB_VERSION_11:
		result = CamCalibDbGetAwb_V11_IlluminationByIdx(hCamCalibDb, idx, &pIlluProfile11);
		strcpy(illName,pIlluProfile11->name);
		break;
	default :
		result = CamCalibDbGetAwb_V10_IlluminationByIdx(hCamCalibDb, idx, &pIlluProfile10);
		strcpy(illName,pIlluProfile10->name);
  }

  LOGV( "%s (exit)\n", __func__);

  return (result);
}



/******************************************************************************
 * CamCalibDbGetAwb_V11_IlluminationByIdx
 *****************************************************************************/
RESULT CamCalibDbGetAwb_V11_IlluminationByIdx
(
    CamCalibDbHandle_t  hCamCalibDb,
    const uint32_t      idx,
    CamAwb_V11_IlluProfile_t**    pIllumination
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pIllumination) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pIllumination = (CamAwb_V11_IlluProfile_t*)ListGetItemByIdx(&pCamCalibDbCtx->pAwbProfile->Para_V11.illumination, idx);

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}



/******************************************************************************
 * CamCalibDbGetAwb_V10_IlluminationByIdx
 *****************************************************************************/
RESULT CamCalibDbGetAwb_V10_IlluminationByIdx
(
    CamCalibDbHandle_t  hCamCalibDb,
    const uint32_t      idx,
    CamAwb_V10_IlluProfile_t**    pIllumination
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pIllumination) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pIllumination = (CamAwb_V10_IlluProfile_t*)ListGetItemByIdx(&pCamCalibDbCtx->pAwbProfile->Para_V10.illumination, idx);

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbReplaceIlluminationAll
 *****************************************************************************/
RESULT CamCalibDbReplaceAwb_V10_IlluminationAll
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamAwb_V10_IlluProfile_t    *pIllumination
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamAwb_V10_IlluProfile_t* pNewIllum = NULL;

  RESULT result;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }
  pNewIllum = (CamAwb_V10_IlluProfile_t*)ListHead(&pCamCalibDbCtx->pAwbProfile->Para_V10.illumination);
  while (pNewIllum) {
    pNewIllum->CrossTalkCoeff = pIllumination->CrossTalkCoeff;
    pNewIllum->CrossTalkOffset = pIllumination->CrossTalkOffset;
    pNewIllum = pNewIllum->p_next;
  }

  LOGV( "%s (exit)\n", __func__);
  return (RET_SUCCESS);
}

RESULT CamCalibDbReplaceAwb_V11_IlluminationAll
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamAwb_V11_IlluProfile_t    *pIllumination
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamAwb_V11_IlluProfile_t* pNewIllum = NULL;

  RESULT result;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }
  pNewIllum = (CamAwb_V11_IlluProfile_t*)ListHead(&pCamCalibDbCtx->pAwbProfile->Para_V11.illumination);
  while (pNewIllum) {
    pNewIllum->CrossTalkCoeff = pIllumination->CrossTalkCoeff;
    pNewIllum->CrossTalkOffset = pIllumination->CrossTalkOffset;
    pNewIllum = pNewIllum->p_next;
  }

  LOGV( "%s (exit)\n", __func__);
  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbReplaceIlluminationByName
 *****************************************************************************/
RESULT CamCalibDbReplaceAwb_V10_IlluminationByName
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamAwb_V10_IlluProfile_t    *pIllumination
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamAwb_V10_IlluProfile_t* pNewIllum = NULL;

  RESULT result;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  pNewIllum = (CamAwb_V10_IlluProfile_t*)ListSearch(&pCamCalibDbCtx->pAwbProfile->Para_V10.illumination, SearchForEqualIllumination, (void*)pIllumination);

  if (NULL != pNewIllum) {
    pNewIllum->CrossTalkCoeff = pIllumination->CrossTalkCoeff;
    pNewIllum->CrossTalkOffset = pIllumination->CrossTalkOffset;
    return (RET_SUCCESS);
  }

  LOGV( "%s (exit)\n", __func__);

  return (RET_NOTAVAILABLE);
}

RESULT CamCalibDbReplaceAwb_V11_IlluminationByName
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamAwb_V11_IlluProfile_t    *pIllumination
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamAwb_V11_IlluProfile_t* pNewIllum = NULL;

  RESULT result;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  pNewIllum = (CamAwb_V11_IlluProfile_t*)ListSearch(&pCamCalibDbCtx->pAwbProfile->Para_V11.illumination, SearchForEqualIllumination, (void*)pIllumination);

  if (NULL != pNewIllum) {
    pNewIllum->CrossTalkCoeff = pIllumination->CrossTalkCoeff;
    pNewIllum->CrossTalkOffset = pIllumination->CrossTalkOffset;
    return (RET_SUCCESS);
  }

  LOGV( "%s (exit)\n", __func__);

  return (RET_NOTAVAILABLE);
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
    pNewLsc = (CamLscProfile_t *)malloc(sizeof(CamLscProfile_t));
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

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbGetLscProfileByIdx
 *****************************************************************************/
RESULT CamCalibDbGetLscProfileByIdx
(
    CamCalibDbHandle_t  hCamCalibDb,
    const uint32_t      idx,
    CamLscProfile_t**   pLscProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pLscProfile) {
    return (RET_INVALID_PARM);
  }

  /* search resolution by name */
  *pLscProfile = (CamLscProfile_t*)ListGetItemByIdx(&pCamCalibDbCtx->lsc_profile, idx);

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbDelLscProfileByName
 *****************************************************************************/
RESULT CamCalibDbDelLscProfileByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamLscProfileName_t     name,
    CamLscProfile_t**       pLscProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  /* search resolution by name */
  *pLscProfile = (CamLscProfile_t*)ListRemoveItem(&pCamCalibDbCtx->lsc_profile, SearchLscProfileByName, (void*)name);

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbReplaceLscProfileAll
 *****************************************************************************/
RESULT CamCalibDbReplaceLscProfileAll
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamLscProfile_t*         pLscProfile
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  /* search resolution by name */
  ListForEach(&pCamCalibDbCtx->lsc_profile, ReplaceLscProfile, (void*)pLscProfile);

  LOGV( "%s (exit)\n", __func__);

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
    pNewCc = (CamCcProfile_t *)malloc(sizeof(CamCcProfile_t));
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
 * CamCalibDbReplaceCcProfile
 *****************************************************************************/
RESULT CamCalibDbReplaceCcProfileByName
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamCcProfile_t*      pAddCc
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamCcProfile_t* pNewCc = NULL;

  RESULT result;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateCcProfile(pAddCc);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if resolution already exists */
  pNewCc = (CamCcProfile_t*)ListSearch(&pCamCalibDbCtx->cc_profile, SearchForEqualCcProfile, (void*)pAddCc);
  if (NULL != pNewCc) {
    pNewCc->CrossTalkCoeff = pAddCc->CrossTalkCoeff;
    pNewCc->CrossTalkOffset = pAddCc->CrossTalkOffset;
    return (RET_SUCCESS);
  }

  LOGV( "%s (exit)\n", __func__);

  return (RET_NOTAVAILABLE);
}

/******************************************************************************
 * CamCalibDbReplaceCcProfileAll
 *****************************************************************************/
RESULT CamCalibDbReplaceCcProfileAll
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamCcProfile_t*      pAddCc
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamCcProfile_t* pNewCc = NULL;

  RESULT result;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateCcProfile(pAddCc);
  if (result != RET_SUCCESS) {
    return (result);
  }

  pNewCc = (CamCcProfile_t*)ListHead(&pCamCalibDbCtx->cc_profile);
  while (pNewCc) {
    pNewCc->CrossTalkCoeff = pAddCc->CrossTalkCoeff;
    pNewCc->CrossTalkOffset = pAddCc->CrossTalkOffset;
    pNewCc = (CamCcProfile_t *)pNewCc->p_next;
  }

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbGetCcProfileByName
 *****************************************************************************/
RESULT CamCalibDbGetCcProfileByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamCcProfileName_t      name,
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
 * CamCalibDbReplaceDpfProfile
 *****************************************************************************/
RESULT CamCalibDbReplaceDpfProfileAll
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamDpfProfile_t*        pRepDpf
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamDpfProfile_t* pNewDpf = NULL;
  RESULT result;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateDpfProfile(pRepDpf);
  if (result != RET_SUCCESS) {
    return (result);
  }

  pNewDpf = (CamDpfProfile_t *)ListHead(&pCamCalibDbCtx->dpf_profile);
  while (pNewDpf) {
    pNewDpf->nll_segmentation = pRepDpf->nll_segmentation;
    pNewDpf->nll_coeff = pRepDpf->nll_coeff;
    pNewDpf->SigmaGreen = pRepDpf->SigmaGreen;
    pNewDpf->SigmaRedBlue = pRepDpf->SigmaRedBlue;
    pNewDpf->fGradient = pRepDpf->fGradient;
    pNewDpf->fOffset = pRepDpf->fOffset;
    pNewDpf->NfGains = pRepDpf->NfGains;
    pNewDpf->ADPFEnable = pRepDpf->ADPFEnable;
    pNewDpf = (CamDpfProfile_t *)pNewDpf->p_next;
  }

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}


/******************************************************************************
 * CamCalibDbReplaceDpfProfile
 *****************************************************************************/
RESULT CamCalibDbReplaceDpfProfile
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamDpfProfile_t*        pRepDpf
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamDpfProfile_t* pNewDpf = NULL;
  RESULT result;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateDpfProfile(pRepDpf);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if resolution already exists */
  pNewDpf = (CamDpfProfile_t*)ListSearch(&pCamCalibDbCtx->dpf_profile, SearchForEqualDpfProfile, (void*)pRepDpf);
  if (NULL != pNewDpf) {
    pNewDpf->nll_segmentation = pRepDpf->nll_segmentation;
    pNewDpf->nll_coeff = pRepDpf->nll_coeff;
    pNewDpf->SigmaGreen = pRepDpf->SigmaGreen;
    pNewDpf->SigmaRedBlue = pRepDpf->SigmaRedBlue;
    pNewDpf->fGradient = pRepDpf->fGradient;
    pNewDpf->fOffset = pRepDpf->fOffset;
    pNewDpf->NfGains = pRepDpf->NfGains;
    pNewDpf->ADPFEnable = pRepDpf->ADPFEnable;
  } else {
    return (RET_INVALID_PARM);
  }

  LOGV( "%s (exit)\n", __func__);

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
	ListInit(&pNewDpf->newDsp3DNRProfileList);   // clear possibly not empty schemes list in copy
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

	/* add already linked 3dnr as well */
    CamNewDsp3DNRProfile_t* pNewDsp3DNR = (CamNewDsp3DNRProfile_t*)ListHead(&pAddDpf->newDsp3DNRProfileList);
    while (pNewDsp3DNR) {
      result = CamCalibDbAddNewDsp3DNRSetting(hCamCalibDb, pNewDpf, pNewDsp3DNR);
      if (result != RET_SUCCESS) {
        return (result);
      }

      pNewDsp3DNR = (CamNewDsp3DNRProfile_t*)pNewDsp3DNR->p_next;
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
  pNewFilter = (CamFilterProfile_t *)malloc(sizeof(CamFilterProfile_t));
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
RESULT CamCalibDbAddNewDsp3DNRSetting
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamDpfProfile_t*         pDpfProfile,
    CamNewDsp3DNRProfile_t* pAddNewDsp3DNRSetting
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  CamNewDsp3DNRProfile_t* pNewDsp3dnrSetting = NULL;

  RESULT result;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pDpfProfile) {
    return (RET_INVALID_PARM);
  }

  result = ValidateNewDsp3dnrSetting(pAddNewDsp3DNRSetting);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if data already exists */
  pNewDsp3dnrSetting = (CamNewDsp3DNRProfile_t*)ListSearch(&pDpfProfile->newDsp3DNRProfileList, SearchForEqualNewDsp3DNRSetting, (void*)pAddNewDsp3DNRSetting);
  if (NULL != pNewDsp3dnrSetting) {
    return (RET_NOTAVAILABLE);
  }

  /* finally allocate, copy & add scheme */
  pNewDsp3dnrSetting = (CamNewDsp3DNRProfile_t *)malloc(sizeof(CamNewDsp3DNRProfile_t));
  if (NULL == pNewDsp3dnrSetting) {
    return (RET_OUTOFMEM);
  }
  MEMCPY(pNewDsp3dnrSetting, pAddNewDsp3DNRSetting, sizeof(CamNewDsp3DNRProfile_t));

  ListPrepareItem(pNewDsp3dnrSetting);
  ListAddTail(&pDpfProfile->newDsp3DNRProfileList, pNewDsp3dnrSetting);

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbGetNoOfEcmSchemes
 *****************************************************************************/
RESULT CamCalibDbGetNoOfNewDsp3DNRSetting
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamDpfProfile_t*         pDpfProfile,
    int32_t*                 no
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pDpfProfile) {
    return (RET_INVALID_PARM);
  }

  if (NULL == no) {
    return (RET_INVALID_PARM);
  }

  *no = (uint32_t)ListNoItems(&pDpfProfile->newDsp3DNRProfileList);

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbGetEcmSchemeByName
 *****************************************************************************/
RESULT CamCalibDbGetNewDsp3DNRSettingByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamDpfProfile_t*         pDpfProfile,
    CamNewDsp3dnrProfileName_t      NewDsp3DNRSettingName,
    CamNewDsp3DNRProfile_t**          ppNewDsp3DnrSetting
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == ppNewDsp3DnrSetting) {
    return (RET_INVALID_PARM);
  }

  /* search scheme by name */
  *ppNewDsp3DnrSetting = (CamNewDsp3DNRProfile_t*)ListSearch(&pDpfProfile->newDsp3DNRProfileList, SearchNewDsp3DNRSettingByName, (void*)NewDsp3DNRSettingName);

  LOGV( "%s (exit)\n", __func__);

  return (RET_SUCCESS);
}

/******************************************************************************
 * CamCalibDbGetEcmSchemeByIdx
 *****************************************************************************/
RESULT CamCalibDbGetNewDsp3DNRByIdx
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamDpfProfile_t*         pDpfProfile,
    const uint32_t          idx,
    CamNewDsp3DNRProfile_t**          ppNewDsp3DnrSetting
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == pDpfProfile) {
    return (RET_INVALID_PARM);
  }

  if (NULL == ppNewDsp3DnrSetting) {
    return (RET_INVALID_PARM);
  }

  /* search EC scheme by index */
  *ppNewDsp3DnrSetting = (CamNewDsp3DNRProfile_t*)ListGetItemByIdx(&pDpfProfile->newDsp3DNRProfileList, idx);

  LOGV( "%s (exit)\n", __func__);

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
  pNewDsp3dnrSetting = (CamDsp3DNRSettingProfile_t *)malloc(sizeof(CamDsp3DNRSettingProfile_t));
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


//werring.wu add 2018 1 18
/******************************************************************************
 * CamCalibDbAddIesharpenProfile
 *****************************************************************************/
RESULT CamCalibDbAddRKsharpenProfile
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamIesharpenProfile_t     *pAddIesharpen
)
{
    CamCalibDbContext_t *pCamCalibDbCtx = (CamCalibDbContext_t *)hCamCalibDb;
    CamIesharpenProfile_t *pNewIesharpen = NULL;

    RESULT result;

    LOGV("%s (enter)\n", __func__);

    if ( NULL == pCamCalibDbCtx )
    {
        return ( RET_WRONG_HANDLE );
    }

    result = ValidateIesharpenProfile( pAddIesharpen );
    if ( result != RET_SUCCESS )
    {
        return ( result );
    }

    /* check if resolution already exists */
    pNewIesharpen = (CamIesharpenProfile_t *)ListSearch( &pCamCalibDbCtx->iesharpen_profile, SearchForEqualIesharpenProfile, (void *)pAddIesharpen );
    if ( NULL == pNewIesharpen )
    {
        pNewIesharpen = (CamIesharpenProfile_t *)malloc( sizeof(CamIesharpenProfile_t) );
        MEMCPY( pNewIesharpen, pAddIesharpen, sizeof(CamIesharpenProfile_t) );

        ListPrepareItem( pNewIesharpen );
        ListAddTail( &pCamCalibDbCtx->iesharpen_profile, pNewIesharpen );
    }
    else
    {
        return ( RET_INVALID_PARM );
    }

    LOGV("%s (exit)\n", __func__);

    return ( RET_SUCCESS );
}



/******************************************************************************
 * CamCalibDbGetIesharpenProfileByName
 *****************************************************************************/
RESULT CamCalibDbGetRKsharpenProfileByName
(
    CamCalibDbHandle_t      hCamCalibDb,
    CamIesharpenProfileName_t name,
    CamIesharpenProfile_t **pIesharpenProfile
)
{
    CamCalibDbContext_t *pCamCalibDbCtx = (CamCalibDbContext_t *)hCamCalibDb;

    LOGV("%s (enter)\n", __func__);

    if ( NULL == pCamCalibDbCtx )
    {
        return ( RET_WRONG_HANDLE );
    }

    if (  NULL == pIesharpenProfile )
    {
        return ( RET_INVALID_PARM );
    }

    /* search resolution by name */
    *pIesharpenProfile = (CamIesharpenProfile_t *)ListSearch( &pCamCalibDbCtx->iesharpen_profile, SearchIesharpenProfileByName, (void *)name );

    LOGV("%s (exit)\n", __func__);

    return ( RET_SUCCESS );
}



/******************************************************************************
 * CamCalibDbGetIesharpenProfileByResolution
 *****************************************************************************/
RESULT CamCalibDbGetRKsharpenProfileByResolution
(
    CamCalibDbHandle_t          hCamCalibDb,
    const CamResolutionName_t   ResName,
    CamIesharpenProfile_t **pIesharpenProfile
)
{
    CamCalibDbContext_t *pCamCalibDbCtx = (CamCalibDbContext_t *)hCamCalibDb;

    LOGV("%s (enter)\n", __func__);

    if ( NULL == pCamCalibDbCtx )
    {
        return ( RET_WRONG_HANDLE );
    }

    if (  NULL == pIesharpenProfile )
    {
        return ( RET_INVALID_PARM );
    }

    /* search resolution by name */
    *pIesharpenProfile = (CamIesharpenProfile_t *)ListSearch( &pCamCalibDbCtx->iesharpen_profile, SearchIesharpenProfileByResolution, (void *)ResName );

    LOGV("%s (exit)\n", __func__);

    return ( RET_SUCCESS );
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
     LOGE("%s malloc fail\n", __func__);
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
  CamCalibWdrGlobal_t* pNewWdrGlobal = (CamCalibWdrGlobal_t *)malloc(sizeof(CamCalibWdrGlobal_t));
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
  CamCprocProfile_t* pNewCprocGlobal = (CamCprocProfile_t *)malloc(sizeof(CamCprocProfile_t));
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

/******************************************************************************
 * CamCalibDbAddOTPGlobal
 *****************************************************************************/
RESULT CamCalibDbAddOTPGlobal
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamOTPGlobal_t* pAddOTPGlobal
) {
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  RESULT result = RET_SUCCESS;

  LOGV( "%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  result = ValidateOTPGlobalData(pAddOTPGlobal);
  if (result != RET_SUCCESS) {
    return (result);
  }

  /* check if data already exists */
  if (NULL != pCamCalibDbCtx->pOTPGlobal) {
    return (RET_INVALID_PARM);
  }

  /* finally allocate, copy & add data */
  CamOTPGlobal_t* pNewOTPGlobal = (CamOTPGlobal_t*)malloc(sizeof(CamOTPGlobal_t));
  if (NULL == pNewOTPGlobal) {
    return (RET_OUTOFMEM);
  }

  MEMCPY(pNewOTPGlobal, pAddOTPGlobal, sizeof(CamOTPGlobal_t));

  pCamCalibDbCtx->pOTPGlobal = pNewOTPGlobal;

  LOGV( "%s (exit) %d\n", __func__, result);

  return (RET_SUCCESS);
}

RESULT CamCalibDbGetOTPGlobal
(
    CamCalibDbHandle_t  hCamCalibDb,
    CamOTPGlobal_t**   ppOTPGlobal
){
  CamCalibDbContext_t* pCamCalibDbCtx = (CamCalibDbContext_t*)hCamCalibDb;
  RESULT result;

  LOGV("%s (enter)\n", __func__);

  if (NULL == pCamCalibDbCtx) {
    return (RET_WRONG_HANDLE);
  }

  if (NULL == ppOTPGlobal) {
    return (RET_INVALID_PARM);
  }

  /* return reference to global cproc configuration */
  *ppOTPGlobal = pCamCalibDbCtx->pOTPGlobal;

  LOGV("%s (exit)\n", __func__);

  return (RET_SUCCESS);
}
