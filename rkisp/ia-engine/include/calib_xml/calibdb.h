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
  CamCalibAwbGlobal_t awb_data;
  CamIlluProfile_t illu;
  CamLscProfile_t lsc_profile;
  CamCcProfile_t cc_profile;
  CamBlsProfile_t bls_profile;
  CamCacProfile_t cac_profile;
  CamDpfProfile_t dpf_profile;
  CamDpccProfile_t dpcc_profile;
  CamCalibSystemData_t system_data;
  CamCprocProfile_t cproc_profile;
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

  // parse helper
  bool parseEntryCell(const XMLElement*, int, parseCellContent, void* param = NULL);

  // parse Header
  bool parseEntryHeader(const XMLElement*, void* param = NULL);
  bool parseEntryResolution(const XMLElement*, void* param = NULL);

  bool parseEntryFramerates(const XMLElement*, void* param = NULL);

  // parse Sensor
  bool parseEntrySensor(const XMLElement*, void* param = NULL);

  // parse Sensor-AWB
  bool parseEntryAwb(const XMLElement*, void* param = NULL);
  bool parseEntryAwbGlobals(const XMLElement*, void* param = NULL);
  bool parseEntryAwbIllumination(const XMLElement*, void* param = NULL);
  bool parseEntryAwbIlluminationAlsc(const XMLElement*, void* param = NULL);
  bool parseEntryAwbIlluminationAcc(const XMLElement*, void* param = NULL);

  // parse Sensor-AEC
  bool parseEntryAec(const XMLElement*, void* param = NULL);
  bool parseEntryAecEcm(const XMLElement*, void* param = NULL);
  bool parseEntryAecEcmPriorityScheme(const XMLElement*, void* param = NULL);

  // parse Sensor-LSC
  bool parseEntryLsc(const XMLElement*, void* param = NULL);

  // parse Sensor-CC
  bool parseEntryCc(const XMLElement*, void* param = NULL);

  // parse Sensor-BLS
  bool parseEntryBls(const XMLElement*, void* param = NULL);

  // parse Sensor-CAC
  bool parseEntryCac(const XMLElement*, void* param = NULL);

  bool parseEntryFilter(const XMLElement* plement, void* param = NULL);

  // parse Sensor-3dnr
  bool parseEntry3DNR(const XMLElement* , void* param = NULL);
  
  // parse Sensor-DPF
  bool parseEntryDpf(const XMLElement*, void* param = NULL);

  // parse Sensor-DPCC
  bool parseEntryDpcc(const XMLElement*, void* param = NULL);
  bool parseEntryDpccRegisters(const XMLElement*, void* param = NULL);
  // parse Sensor-GOC
  bool parseEntryGoc(const XMLElement*, void* param = NULL);
  // parse Sensor-WDR
  bool parseEntryWdr(const XMLElement*, void* param = NULL);
  // parse System
  bool parseEntrySystem(const XMLElement*, void* param = NULL);
  // parse cproc
  bool parseEntryCproc(const XMLElement*, void* param = NULL);

  void characterDataHandler(void* userData, const char* s, int len);

  void startElementHandler(void* userData, const char* name, const char** atts);

 private:

  CamCalibDbHandle_t  m_CalibDbHandle;
  struct sensor_calib_info m_CalibInfo;
};


#endif /* __CALIBDB_H__ */
