/******************************************************************************
 *
 * Copyright 2016, Fuzhou Rockchip Electronics Co.Ltd. All rights reserved.
 * No part of this work may be reproduced, modified, distributed, transmitted,
 * transcribed, or translated into any language or computer format, in any form
 * or by any means without written permission of:
 * Fuzhou Rockchip Electronics Co.Ltd .
 * 
 *
 *****************************************************************************/
/**
 * @file    calibtreewidget.h
 *
 *
 *****************************************************************************/
#ifndef __CALIBDB_H__
#define __CALIBDB_H__

//#include <QDomDocument>
//#include <QtXml>
#include <tinyxml2.h>
#include <ebase/builtins.h>
#include <ebase/dct_assert.h>

#include <common/return_codes.h>
#include <common/cam_types.h>

#include <cam_calibdb/cam_calibdb_api.h>
using namespace tinyxml2;

struct sensor_calib_info {
  CamCalibDbMetaData_t meta_data;
  CamResolution_t resolution;
  CamCalibAecGlobal_t aec_data;
  CamEcmProfile_t EcmProfile;
  CamEcmScheme_t EcmScheme;
  CamCalibAwb_V11_Global_t awb_v11_data;
  CamAwb_V11_IlluProfile_t awb_v11_illu;  
  CamCalibAwb_V10_Global_t awb_v10_data;
  CamAwb_V10_IlluProfile_t awb_v10_illu;
  CamLscProfile_t lsc_profile;
  CamCcProfile_t cc_profile;
  CamBlsProfile_t bls_profile;
  CamCacProfile_t cac_profile;
  CamDpfProfile_t dpf_profile;
  CamDpccProfile_t dpcc_profile;
  CamCalibSystemData_t system_data;
  CamCprocProfile_t cproc_profile;
  uint32_t IQMagicVerCode;
};


class CalibDb {
 public:
  CalibDb();
  ~CalibDb();

  CamCalibDbHandle_t GetCalibDbHandle(void) {
    return (m_CalibDbHandle);
  }

  bool SetCalibDbHandle(CamCalibDbHandle_t phandle) {
    if (phandle) {
      m_CalibDbHandle = phandle;
      return true;
    }

    return false;
  }

  bool CreateCalibDb(const XMLElement*);
  bool CreateCalibDb(const char* device);
  struct sensor_calib_info* GetCalibDbInfo() {
    return &(m_CalibInfo);
  }
  bool SetCalibDbInfo(struct sensor_calib_info* pCalibInfo) {
    MEMCPY(&m_CalibInfo, pCalibInfo, sizeof(struct sensor_calib_info));
    return true;
  }

 private:

  typedef bool (CalibDb::*parseCellContent)(const XMLElement*, void* param);

  bool parseCellNoElement(
    const XMLElement*   pelement,
    int                 noElements,
    int					*RealNo
  ) ;
  
  // parse helper
  bool parseEntryCell(const XMLElement*, int, parseCellContent, void* param = NULL);
    bool parseEntryCellForCheck(
	const XMLElement*   pelement,
	int                 noElements,
	uint32_t			cur_id,
	uint32_t	 parent_id	);
  bool parseEntryCell
  (
      const XMLElement*   pelement,
      int                 noElements,
      parseCellContent    func,
      void*                param,
      uint32_t	 cur_id,
      uint32_t	 parent_id			
  );

  // parse Header
  bool parseEntryHeader(const XMLElement*, void* param = NULL);
  bool parseEntryResolution(const XMLElement*, void* param = NULL);

  bool parseEntryFramerates(const XMLElement*, void* param = NULL);

  // parse Sensor
  bool parseEntrySensor(const XMLElement*, void* param = NULL);

  // parse Sensor-AWB
  bool parseEntryAwb_V10_IIR( const XMLElement*, void *param = NULL );
  bool parseEntryAwb_V11_IIR( const XMLElement*, void *param = NULL );
  bool parseEntryAwb(const XMLElement*, void* param = NULL);
  bool parseEntryAwb_V11_Para(const XMLElement*, void* param = NULL);
  bool parseEntryAwb_V10_Para(const XMLElement*, void* param = NULL);
  bool parseEntryAwb_V11_Globals(const XMLElement*, void* param = NULL);  
  bool parseEntryAwb_V10_Globals(const XMLElement*, void* param = NULL);
  
  bool parseEntryAwb_V10_IlluminationGMM(const XMLElement*, void* param = NULL);
  bool parseEntryAwb_V10_IlluminationSat(const XMLElement*, void* param = NULL);
  bool parseEntryAwb_V11_IlluminationSat(const XMLElement*, void* param = NULL);
  bool parseEntryAwb_V10_IlluminationVig(const XMLElement*, void* param = NULL);
  bool parseEntryAwb_V11_IlluminationVig(const XMLElement*, void* param = NULL);
  
  bool parseEntryAwb_V11_Illumination(const XMLElement*, void* param = NULL);
  
  bool parseEntryAwb_V10_Illumination(const XMLElement*, void* param = NULL);
  bool parseEntryAwb_V11_IlluminationAlsc(const XMLElement*, void* param = NULL);
  
  bool parseEntryAwb_V10_IlluminationAlsc(const XMLElement*, void* param = NULL);
  bool parseEntryAwb_V11_IlluminationAcc(const XMLElement*, void* param = NULL);
  
  bool parseEntryAwb_V10_IlluminationAcc(const XMLElement*, void* param = NULL);
  bool parseEntryAfWin( const XMLElement*, void *param = NULL, uint32_t parent_id = 0);
  bool parseEntryContrastAf( const XMLElement*, void *param = NULL );
  bool parseEntryLaserAf( const XMLElement*, void *param = NULL );
  bool parseEntryPdaf( const XMLElement*, void *param = NULL );
  bool parseEntryAf( const XMLElement*, void *param = NULL );
  // parse Sensor-AEC
  bool parseEntryAecDON( const XMLElement*, void *param = NULL );
  bool parseEntryAecFPSSetConfig( const XMLElement*, void *param = NULL );
  bool parseEntryAecHist2Hal( const XMLElement*, void *param = NULL );
  bool parseEntryAecNLSC( const XMLElement*, void *param = NULL );
  bool parseEntryAecIntervalAdjustStrategy( const XMLElement*, void *param = NULL );
  bool parseEntryAecBacklight( const XMLElement*, void *param = NULL );
  bool parseEntryAecLockAE( const XMLElement*, void *param = NULL );
  bool parseEntryAecHdrCtrlLframe( const XMLElement*, void *param = NULL );
  bool parseEntryAecHdrCtrlSframe( const XMLElement*, void *param = NULL );
  bool parseEntryAecHdrCtrl( const XMLElement*, void *param = NULL );
	
  bool parseEntryAec(const XMLElement*, void* param = NULL);
  bool parseEntryAecEcm(const XMLElement*, void* param = NULL);
  bool parseEntryAecEcmPriorityScheme(const XMLElement*, void* param = NULL);
  bool parseAECDySetpoint(const XMLElement*, void* param = NULL);
  bool parseAECExpSeparate(const XMLElement*, void* param = NULL);

  // parse Sensor-LSC
  bool parseEntryLsc(const XMLElement*, void* param = NULL);

  // parse Sensor-CC
  bool parseEntryCc(const XMLElement*, void* param = NULL);

  // parse Sensor-BLS
  bool parseEntryBls(const XMLElement*, void* param = NULL);

  // parse Sensor-CAC
  bool parseEntryCac(const XMLElement*, void* param = NULL);

  bool parseEntryFilter(const XMLElement* plement, void* param = NULL);
  bool parseEntryFilterDemosiacTH(const XMLElement* plement, void* param = NULL);
  bool parseEntryFilterSharpLevel(const XMLElement* plement, void* param = NULL);
  bool parseEntryFilterDenoiseLevel(const XMLElement* plement, void* param = NULL);
  bool parseEntryFilterRegConfig(const XMLElement* plement, void* param = NULL);

  // parse Sensor-3dnr
  bool parseEntry3DnrLevel(const XMLElement* plement, void* param = NULL) ;
  bool parseEntry3DnrLuma(const XMLElement* plement, void* param = NULL) ;
  bool parseEntry3DnrChrm(const XMLElement* plement, void* param = NULL) ;
  bool parseEntry3DnrSharp(const XMLElement* plement, void* param = NULL) ;
  bool parseEntry3DNR(const XMLElement* , void* param = NULL);
  
  //parse new 3dnr
  
  bool parseEntryNew3DnrYnr(const XMLElement* , void* param = NULL);
  bool parseEntryNew3DnrUVnr(const XMLElement* , void* param = NULL);
  bool parseEntryNew3DnrSharp(const XMLElement* , void* param = NULL);
  bool parseEntryNew3DNR(const XMLElement* plement, void* param = NULL) ;

  bool parseEntryDemosaicLPConfig(const XMLElement* plement, void* param = NULL); 
  
  // parse Sensor-DPF
  bool parseEntryDpf(const XMLElement*, void* param = NULL);

  // parse Sensor-DPCC
  bool parseEntryDpcc(const XMLElement*, void* param = NULL);
  bool parseEntryDpccRegisters(const XMLElement*, void* param = NULL);
  // parse Sensor-GOC
  bool parseEntryGoc(const XMLElement*, void* param = NULL);
  // parse Sensor-WDR
  bool parseEntryWdrMaxGain(const XMLElement*, void* param = NULL);
  bool parseEntryWdr(const XMLElement*, void* param = NULL);
  // parse System
  bool parseEntrySystem(const XMLElement*, void* param = NULL);
  bool parseEntrySystemAfps( const XMLElement*, void *param = NULL );
  // parse cproc
  bool parseEntryCproc(const XMLElement*, void* param = NULL);
  bool parseEntryRKsharpen( const XMLElement*, void *param = NULL );
  bool parseEntryOTP(const XMLElement*, void* param = NULL); 

  void characterDataHandler(void* userData, const char* s, int len);

  void startElementHandler(void* userData, const char* name, const char** atts);

 private:

  CamCalibDbHandle_t  m_CalibDbHandle;
  struct sensor_calib_info m_CalibInfo;
};


#endif /* __CALIBDB_H__ */
