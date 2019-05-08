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
 * @file    calibdb.cpp
 *
 *****************************************************************************/
#include <stdlib.h>
#include <ebase/builtins.h>
#include <ebase/dct_assert.h>

#include <common/return_codes.h>
#include <common/cam_types.h>

#include <cam_calibdb/cam_calibdb_api.h>
#include <string>
#include <iostream>

#include "calib_xml/calibdb.h"
#include "calibtags.h"
#include "xmltags.h"
#include <base/log.h>


#include   <fstream>


/*************************************************************************/
/**************************** version log ***********************************/
//v0.1.0:  1. old version, not support verify the calibdb version
//		  2. support multi cell for 3dnr
//	 	  3. support multi cell for goc
//
//v0.1.1:  1. support multi filter cell in dpf module
//		  2. add demosaic_th para in filter part
//     	  3. add 3dnr noise para in 3dnr part
//        	  4. add calibdb version from v0.1.1
//
//v0.1.2:  1. add night setpoint
//		  2. add aec interval adjust strategy
//     	  3. modify gain range num adapt to xml not fix num yet
//
//v0.1.a:  XML for HDR
//		  1. add AEC -> Lock_AE for Hdr test
//		  2. add LTimeDot/StimeDot in AEC
//		  3. add LGainDot/SGainDot in AEC
//		  4. add AEC -> HdrCtrl for Hdr Control
//v0.2.0:  sync calibdb with cifisp & hal00 demo
//		  1. awb add v10 & v11 awb params. But now only v10 awb params are used for awb althgorithm
//		  2. aec fix 9x9 grid weights, add night weidth, don, nlsc, hist_2_hal, exposure_separate,back_light
//		  3. af delete old params, add new parameters
//		  4. gamma fix 34 point for isp10 & isp11
//		  5. dpf add demosaic_lp, mfd&UVNR, new_3dnr_params.
//		  6. add isp11 rk ie sharp params.
//v0.2.1:  XML FOR HDR
//		  1. add SetPoint for L/Sframe
//		  2. add Dynamic value for OEROI/DarkROI
//v0.2.2:  XML FOR BW ISP OUTPUT
//                 1. add isp output type configuration in XML header
//                 2. add lsc ccm wb module cofiguration for BW illuminant in  in XML 
//v0.2.3:  add otp info in iq xml
//             modify rk ie sharp parameters type
//v0.2.4:  modify ie sharp, add gain and lap mat coe paras for control
//            modify demosaiclp, change thgrid_r_fct....from 1 to 6 to change with the gain value
//v0.2.5:  XML FOR AF
//         1. add af window configuration in xml
//         2. add af trigger thershold configuration in XML 
//v1.0.0:  add xml check & magic version code 635075
//		  start from this version, like va.b.c
//			a: show the big change version
//                b: show the little change version
//	             c: show that the content of xml is not changed, but fix some parse bugs.
/*************************************************************************/
/*************************************************************************/




#define CODE_XML_PARSE_VERSION "v1.0.0"

static std::ofstream redirectOut("/dev/null");

//#define DEBUG_LOG


/******************************************************************************
* xml check example while parse subtag
*  XML_CHECK_START(parent_id, p_parent_id);
*  while(){
*      XML_CHECK_WHILE_SUBTAG_MARK(tag_name, type, size);
*  }
*  XML_CHECK_END();
*****************************************************************************/


#define XML_CHECK_SET_CUR_ID(cur_id) \
	cur_tag_id = cur_id;

#define XML_CHECK_START(parent_id, p_parent_id) \
	CALIB_IQ_TAG_ID_T cur_tag_id = CALIB_IQ_TAG_END; \
  	CALIB_IQ_TAG_ID_T parent_tag_id = parent_id; \
	calib_check_nonleaf_tag_start(parent_tag_id, p_parent_id);

#define XML_CHECK_MARK_IF_NEED(type, size) \
	calib_tag_info_t *tag_info = &(g_calib_tag_infos[cur_tag_id]); \
	calib_check_tag_attrs(cur_tag_id, type, size, parent_tag_id); \
	if(tag_info->sub_tags_array == NULL) { \
		calib_check_tag_mark(cur_tag_id, parent_tag_id); \
	}

#define XML_CHECK_END() \
	calib_check_nonleaf_tag_end(parent_tag_id);

#define XML_CHECK_CELL_SET_SIZE(size) \
	calib_check_cell_set_size(cur_tag_id, parent_tag_id, cell_size);

#define XML_CHECK_GET_TAG_ID(tag_name) \
	calib_check_getID_by_name(tag_name, parent_tag_id, &cur_tag_id);

#define XML_CHECK_TAGID_COMPARE(tag_id) \
		cur_tag_id == tag_id

#define XML_CHECK_WHILE_SUBTAG_MARK(tag_name, type, size) \
		XML_CHECK_SET_CUR_ID(CALIB_IQ_TAG_END); \
		XML_CHECK_GET_TAG_ID(tag_name); \
		XML_CHECK_MARK_IF_NEED(type, size);

#define XML_CHECK_TOPTAG_MARK(tag_id, type, size) \
	XML_CHECK_SET_CUR_ID(tag_id); \
	XML_CHECK_MARK_IF_NEED(type, size);
	
	
	
/******************************************************************************
 * Toupper
 *****************************************************************************/
char* Toupper(const char* s) {
  int i = 0;
  char* p = (char*)s;
  char tmp;
  int len = 0;

  if (p) {
    len = strlen(p);
    for (i = 0; i < len; i++) {
      tmp = p[i];

      if (tmp >= 'a' && tmp <= 'z')
        tmp = tmp - 32;

      p[i] = tmp;
    }
  }
  return p;
}

static int ParseCharToHex
(
    XmlTag*  tag,          /**< trimmed c string */
    uint32_t*    reg_value
) {
  bool ok;

  *reg_value = tag->ValueToUInt(&ok);
  if (!ok) {
    LOGE( "%s(%d): parse error: invalid register value:\n", __FUNCTION__,__LINE__,tag->Value());
    return (false);
  } else {
#ifdef DEBUG_LOG
    LOGD( "%s(%d): parse reg valevalue:\n", __FUNCTION__,__LINE__,*reg_value);
#endif
  }

  return 0;
}


/******************************************************************************
 * ParseFloatArray
 *****************************************************************************/
static int ParseFloatArray
(
    const char*  c_string,          /**< trimmed c string */
    float*       values,            /**< pointer to memory */
    const int   num                 /**< number of expected float values */
) {
  float* value = values;
  char* str    = (char*)c_string;
  int last = strlen(str);
  /* calc. end adress of string */
  char* str_last = str + (last - 1);

  std::string s_string(str);
  size_t find_start = s_string.find("[", 0);
  size_t find_end = s_string.find("]", 0);

  if ((find_start == std::string::npos) || (find_end == std::string::npos)) {
#ifdef DEBUG_LOG
    LOGD( "%s(%d): start=%d,end=%d\n", __FUNCTION__,__LINE__,find_start,find_end);
#endif
    return -1;
  }

  str = (char*)c_string + find_start;
  str_last = (char*)c_string + find_end;

#if 0
  while (*str == 0x20) {
    str++;
  }

  while (*str_last == 0x20) {
    str_last--;
  }

  /* check for beginning/closing parenthesis */
  if ((str[0] != '[') || (str_last[0] != ']')) {
#ifdef DEBUG_LOG
    redirectOut << __FUNCTION__ << std::endl;
    redirectOut << "str[0]:(" << str[0] << ")" << std::endl;
    redirectOut << "str_last[0]:(" << str_last[0] << ")" << std::endl;
    redirectOut << "strings:(" << str << ")" << std::endl;
#endif
    return (-1);
  }
#endif

  /* skipped left parenthesis */
  str++;

  /* skip spaces */
  while (*str == 0x20 || *str == 0x09 || (*str == 0x0a) || (*str == 0x0d)) {
    str++;
  }

  int cnt = 0;
  int scanned;
  float f;

  /* parse the c-string */
  while ((str != str_last) && (cnt < num)) {
    scanned = sscanf(str, "%f", &f);
    if (scanned != 1) {
      LOGE( "%s(%d): %f err\n", __FUNCTION__,__LINE__,f);
      goto err1;
    } else {
      value[cnt] = f;
      cnt++;
    }

    /* remove detected float */
    while ((*str != 0x20)  && (*str != ',') && (*str != ']')) {
      str++;
    }

    /* skip spaces and comma */
    while ((*str == 0x20) || (*str == ',') || (*str == 0x09) || (*str == 0x0a) || (*str == 0x0d)) {
      str++;
    }
  }

#ifdef DEBUG_LOG
  for (int i = 0; i < cnt; i++) {
	LOGD( "%f,\n", value[i]);
  }
#endif

  return (cnt);

err1:
  MEMSET(values, 0, (sizeof(float) * num));

  return (0);

}

/******************************************************************************
 * ParseUintArray
 *****************************************************************************/
static int ParseUintArray
(
    const char*  c_string,          /**< trimmed c string */
    uint32_t*      values,            /**< pointer to memory */
    const int   num                 /**< number of expected float values */
) {
  uint32_t*  value  = values;
  char* str       = (char*)c_string;

  int last = strlen(str);
  char* str_last = str + (last - 1);

  std::string s_string(str);
  size_t find_start = s_string.find("[", 0);
  size_t find_end = s_string.find("]", 0);

  if ((find_start == std::string::npos) || (find_end == std::string::npos)) {
#ifdef DEBUG_LOG
	LOGE( "%s(%d): start=%d,end=%d\n", __FUNCTION__,__LINE__,find_start,find_end);
#endif
    return -1;
  }

  str = (char*)c_string + find_start;
  str_last = (char*)c_string + find_end;

#if 0
  while (*str == 0x20) {
    str++;
  }

  while (*str_last == 0x20) {
    str_last--;
  }

  /* check for beginning/closing parenthesis */
  if ((str[0] != '[') || (str_last[0] != ']')) {
#ifdef DEBUG_LOG
    redirectOut << __FUNCTION__ << std::endl;
    redirectOut << "str[0]:(" << str[0] << ")" << std::endl;
    redirectOut << "str_last[0]:(" << str_last[0] << ")" << std::endl;
    redirectOut << "strings:(" << str << ")" << std::endl;
#endif
    return (-1);
  }
#endif

  /* calc. end adress of string */
  // char *str_last = str + (last-1);

  /* skipped left parenthesis */
  str++;

  /* skip spaces */
  while (*str == 0x20 || *str == 0x09 || (*str == 0x0a) || (*str == 0x0d)) {
    str++;
  }

  int cnt = 0;
  int scanned;
  uint32_t f;

  /* parse the c-string */
  while ((str != str_last) && (cnt < num)) {
    scanned = sscanf(str, "%u", &f);
    if (scanned != 1) {
	  LOGE( "%s(%d): f:%f error\n", __FUNCTION__, __LINE__, f);
      goto err1;
    } else {
      value[cnt] = f;
      cnt++;
    }

    /* remove detected float */
    while ((*str != 0x20)  && (*str != ',') && (*str != ']')) {
      str++;
    }

    /* skip spaces and comma */
    while ((*str == 0x20) || (*str == ',') || (*str == 0x09) || (*str == 0x0a) || (*str == 0x0d)) {
      str++;
    }
  }

#ifdef DEBUG_LOG
  for (int i = 0; i < cnt; i++) {
	LOGD( "%u,\n", value[i]);
  }
#endif
  return (cnt);
err1:
  MEMSET(values, 0, (sizeof(uint32_t) * num));

  return (0);
}

/******************************************************************************
 * ParseUchartArray//cxf
 *****************************************************************************/
static int ParseUcharArray
(
    const char*  c_string,          /**< trimmed c string */
    unsigned char*      values,            /**< pointer to memory */
    const int   num                 /**< number of expected float values */
) {
  unsigned char*  value  = values;
  char* str       = (char*)c_string;

  int last = strlen(str);
  char* str_last = str + (last - 1);

  std::string s_string(str);
  size_t find_start = s_string.find("[", 0);
  size_t find_end = s_string.find("]", 0);

  if ((find_start == std::string::npos) || (find_end == std::string::npos)) {
#ifdef DEBUG_LOG
    LOGE( "%s(%d):start:%d,end:%d\n", __FUNCTION__,__LINE__,find_start,find_end);
#endif
    return -1;
  }

  str = (char*)c_string + find_start;
  str_last = (char*)c_string + find_end;

#if 0
  while (*str == 0x20) {
    str++;
  }

  while (*str_last == 0x20) {
    str_last--;
  }

  /* check for beginning/closing parenthesis */
  if ((str[0] != '[') || (str_last[0] != ']')) {
#ifdef DEBUG_LOG
    redirectOut << __FUNCTION__ << std::endl;
    redirectOut << "str[0]:(" << str[0] << ")" << std::endl;
    redirectOut << "str_last[0]:(" << str_last[0] << ")" << std::endl;
    redirectOut << "strings:(" << str << ")" << std::endl;
#endif
    return (-1);
  }
#endif

  /* calc. end adress of string */
  // char *str_last = str + (last-1);

  /* skipped left parenthesis */
  str++;

  /* skip spaces */
  while (*str == 0x20 || *str == 0x09 || (*str == 0x0a) || (*str == 0x0d)) {
    str++;
  }

  int cnt = 0;
  int scanned;
  uint8_t f;

  /* parse the c-string */
  while ((str != str_last) && (cnt < num)) {
    scanned = sscanf(str, "%hhu", &f);
    if (scanned != 1) {
      LOGD( "%s(%d):f:%f\n", __FUNCTION__,__LINE__,f);
      goto err1;
    } else {
      value[cnt] = f;
      cnt++;
    }

    /* remove detected float */
    while ((*str != 0x20)  && (*str != ',') && (*str != ']')) {
      str++;
    }

    /* skip spaces and comma */
    while ((*str == 0x20) || (*str == ',') || (*str == 0x09) || (*str == 0x0a) || (*str == 0x0d)) {
      str++;
    }
  }

#ifdef DEBUG_LOG
  for (int i = 0; i < cnt; i++) {
	LOGD( "%u,\n", value[i]);
  }
#endif
  return (cnt);
err1:
  MEMSET(values, 0, (sizeof(uint8_t) * num));

  return (0);
}
/******************************************************************************
 * ParseUchartArray//cxf
 *****************************************************************************/
static int ParseCharArray
(
    const char*  c_string,          /**< trimmed c string */
    int8_t*      values,            /**< pointer to memory */
    const int   num                 /**< number of expected float values */
) {
  int8_t*  value  = values;
  char* str       = (char*)c_string;

  int last = strlen(str);
  char* str_last = str + (last - 1);

  std::string s_string(str);
  size_t find_start = s_string.find("[", 0);
  size_t find_end = s_string.find("]", 0);

  if ((find_start == std::string::npos) || (find_end == std::string::npos)) {
#ifdef DEBUG_LOG
    LOGE( "%s(%d): start=%d,end=%d\n", __FUNCTION__,__LINE__,find_start,find_end);
#endif
    return -1;
  }

  str = (char*)c_string + find_start;
  str_last = (char*)c_string + find_end;

#if 0
  while (*str == 0x20) {
    str++;
  }

  while (*str_last == 0x20) {
    str_last--;
  }

  /* check for beginning/closing parenthesis */
  if ((str[0] != '[') || (str_last[0] != ']')) {
#ifdef DEBUG_LOG
    redirectOut << __FUNCTION__ << std::endl;
    redirectOut << "str[0]:(" << str[0] << ")" << std::endl;
    redirectOut << "str_last[0]:(" << str_last[0] << ")" << std::endl;
    redirectOut << "strings:(" << str << ")" << std::endl;
#endif
    return (-1);
  }
#endif

  /* calc. end adress of string */
  // char *str_last = str + (last-1);

  /* skipped left parenthesis */
  str++;

  /* skip spaces */
  while (*str == 0x20 || *str == 0x09 || (*str == 0x0a) || (*str == 0x0d)) {
    str++;
  }

  int cnt = 0;
  int scanned;
  int8_t f;

  /* parse the c-string */
  while ((str != str_last) && (cnt < num)) {
    scanned = sscanf(str, "%hhd", &f);
    if (scanned != 1) {
      redirectOut << __FUNCTION__ << "f" << f << "err" << std::endl;
      goto err1;
    } else {
      value[cnt] = f;
      cnt++;
    }

    /* remove detected float */
    while ((*str != 0x20)  && (*str != ',') && (*str != ']')) {
      str++;
    }

    /* skip spaces and comma */
    while ((*str == 0x20) || (*str == ',') || (*str == 0x09) || (*str == 0x0a) || (*str == 0x0d)) {
      str++;
    }
  }

#ifdef DEBUG_LOG
  for (int i = 0; i < cnt; i++) {
	LOGD( "%d,\n", value[i]);
  }
#endif
  return (cnt);
err1:
  MEMSET(values, 0, (sizeof(uint8_t) * num));

  return (0);
}


/******************************************************************************
 * ParseUshortArray
 *****************************************************************************/
static int ParseUshortArray
(
    const char*  c_string,          /**< trimmed c string */
    uint16_t*    values,            /**< pointer to memory */
    const int   num                 /**< number of expected float values */
) {
  uint16_t* value = values;
  char* str       = (char*)c_string;
  int last = strlen(str);
  char* str_last = str + (last - 1);

  std::string s_string(str);
  size_t find_start = s_string.find("[", 0);
  size_t find_end = s_string.find("]", 0);

  if ((find_start == std::string::npos) || (find_end == std::string::npos)) {
#ifdef DEBUG_LOG
    LOGE( "%s(%d): parse error! start:%s end:%s\n", 
		__FUNCTION__, __LINE__, find_start, find_end);
#endif
    return -1;
  }

  while (*str != '[') {
    printf("'%c'=%d\n", *str, *str);
    str++;
  }
  str = (char*)c_string + find_start;
  str_last = (char*)c_string + find_end;

#if 0
  while (*str == 0x20) {
    str++;
  }

  while (*str_last == 0x20) {
    str_last--;
  }

  /* check for beginning/closing parenthesis */
  if ((str[0] != '[') || (str_last[0] != ']')) {
#ifdef DEBUG_LOG
    redirectOut << __FUNCTION__ << std::endl;
    redirectOut << "str[0]:(" << str[0] << ")" << std::endl;
    redirectOut << "str_last[0]:(" << str_last[0] << ")" << std::endl;
    redirectOut << "strings:(" << str << ")" << std::endl;
#endif
    return (-1);
  }
#endif

  /* calc. end adress of string */
  //char *str_last = str + (last-1);

  /* skipped left parenthesis */
  str++;

  /* skip spaces */
  while (*str == 0x20 || *str == 0x09 || (*str == 0x0a) || (*str == 0x0d)) {
    str++;
  }

  int cnt = 0;
  int scanned;
  uint16_t f;

  /* parse the c-string */
  while ((str != str_last) && (cnt < num)) {
    scanned = sscanf(str, "%hu", &f);
    if (scanned != 1) {
      LOGD( "%s(%d): parse error!\n", __FUNCTION__,__LINE__);
      goto err1;
    } else {
      value[cnt] = f;
      cnt++;
    }

    /* remove detected float */
    while ((*str != 0x20)  && (*str != ',') && (*str != ']')) {
      str++;
    }

    /* skip spaces and comma */
    while ((*str == 0x20) || (*str == ',') || (*str == 0x09) || (*str == 0x0a) || (*str == 0x0d)) {
      str++;
    }
  }

#ifdef DEBUG_LOG
  for (int i = 0; i < cnt; i++) {
	LOGD( "%u,\n", value[i]);
  }
#endif
  return (cnt);

err1:
  MEMSET(values, 0, (sizeof(uint16_t) * num));

  return (0);

}


/******************************************************************************
 * ParseShortArray
 *****************************************************************************/
static int ParseShortArray
(
    const char*  c_string,          /**< trimmed c string */
    int16_t*     values,            /**< pointer to memory */
    const int   num                 /**< number of expected float values */
) {
  int16_t* value  = values;
  char* str       = (char*)c_string;

  int last = strlen(str);
  char* str_last = str + (last - 1);

  std::string s_string(str);
  size_t find_start = s_string.find("[", 0);
  size_t find_end = s_string.find("]", 0);

  if ((find_start == std::string::npos) || (find_end == std::string::npos)) {
#ifdef DEBUG_LOG
    LOGE( "%s(%d): parse error!\n", __FUNCTION__,__LINE__);
#endif
    return -1;
  }

  str = (char*)c_string + find_start;
  str_last = (char*)c_string + find_end;

#if 0
  while (*str == 0x20) {
    str++;
  }

  while (*str_last == 0x20) {
    str_last--;
  }

  /* check for beginning/closing parenthesis */
  if ((str[0] != '[') || (str_last[0] != ']')) {
#ifdef DEBUG_LOG
    redirectOut << __FUNCTION__ << std::endl;
    redirectOut << "str[0]:(" << str[0] << ")" << std::endl;
    redirectOut << "str_last[0]:(" << str_last[0] << ")" << std::endl;
    redirectOut << "strings:(" << str << ")" << std::endl;
#endif
    return (-1);
  }
#endif

  /* calc. end adress of string */
  // char *str_last = str + (last-1);

  /* skipped left parenthesis */
  str++;

  /* skip spaces */
  while (*str == 0x20 || *str == 0x09 || (*str == 0x0a) || (*str == 0x0d)) {
    str++;
  }

  int cnt = 0;
  int scanned;
  int16_t f;

  /* parse the c-string */
  while ((str != str_last) && (cnt < num)) {
    scanned = sscanf(str, "%hd", &f);
    if (scanned != 1) {
      LOGE( "%s(%d): parse error!\n", __FUNCTION__,__LINE__);
      goto err1;
    } else {
      value[cnt] = f;
      cnt++;
    }

    /* remove detected float */
    while ((*str != 0x20)  && (*str != ',') && (*str != ']')) {
      str++;
    }

    /* skip spaces and comma */
    while ((*str == 0x20) || (*str == ',') || (*str == 0x09) || (*str == 0x0a) || (*str == 0x0d)) {
      str++;
    }
  }

#ifdef DEBUG_LOG
  for (int i = 0; i < cnt; i++) {
	LOGD( "%d,\n", value[i]);
  }
#endif
  return (cnt);

err1:
  MEMSET(values, 0, (sizeof(uint16_t) * num));

  return (0);

}


/******************************************************************************
 * ParseByteArray
 *****************************************************************************/
static int ParseByteArray
(
    const char*  c_string,          /**< trimmed c string */
    uint8_t*     values,            /**< pointer to memory */
    const int   num                 /**< number of expected float values */
) {
  uint8_t* value  = values;
  char* str       = (char*)c_string;
  int last = strlen(str);
  char* str_last = str + (last - 1);

  std::string s_string(str);
  size_t find_start = s_string.find("[", 0);
  size_t find_end = s_string.find("]", 0);

  if ((find_start == std::string::npos) || (find_end == std::string::npos)) {
#ifdef DEBUG_LOG
    LOGE( "%s(%d): parse error!\n", __FUNCTION__,__LINE__);
#endif
    return -1;
  }

  str = (char*)c_string + find_start;
  str_last = (char*)c_string + find_end;

#if 0
  /* check for beginning/closing parenthesis */
  if ((str[0] != '[') || (str_last[0] != ']')) {
#ifdef DEBUG_LOG
    redirectOut << __FUNCTION__ << std::endl;
    redirectOut << "str[0]:(" << str[0] << ")" << std::endl;
    redirectOut << "str_last[0]:(" << str_last[0] << ")" << std::endl;
    redirectOut << "strings:(" << str << ")" << std::endl;
#endif
    return (-1);
  }
#endif

  /* calc. end adress of string */
  //char *str_last = str + (last-1);

  /* skipped left parenthesis */
  str++;

  /* skip spaces */
  while (*str == 0x20 || *str == 0x09 || (*str == 0x0a) || (*str == 0x0d)) {
    str++;
  }

  int cnt = 0;
  int scanned;
  uint16_t f;

  /* parse the c-string */
  while ((str != str_last) && (cnt < num)) {
    scanned = sscanf(str, "%hu", &f);
    if (scanned != 1) {
      LOGE( "%s(%d): parse error!\n", __FUNCTION__,__LINE__);
      goto err1;
    } else {
      value[cnt] = (uint8_t)f;
      cnt++;
    }

    /* remove detected float */
    while ((*str != 0x20)  && (*str != ',') && (*str != ']')) {
      str++;
    }

    /* skip spaces and comma */
    while ((*str == 0x20) || (*str == ',') || (*str == 0x09) || (*str == 0x0a) || (*str == 0x0d)) {
      str++;
    }
  }

#ifdef DEBUG_LOG
  for (int i = 0; i < cnt; i++) {
	LOGD( "%u,\n", value[i]);
  }
#endif
  return (cnt);

err1:
  MEMSET(values, 0, (sizeof(uint8_t) * num));

  return (0);

}


/******************************************************************************
 * ParseCcProfileArray
 *****************************************************************************/
static int ParseCcProfileArray
(
    const char*          c_string,          /**< trimmed c string */
    CamCcProfileName_t  values[],           /**< pointer to memory */
    const int           num                 /**< number of expected float values */
) {
  char* str = (char*)c_string;

  int last = strlen(str);

  /* calc. end adress of string */
  char* str_last = str + (last - 1);

  /* skip beginning spaces */
  while (*str == 0x20 || *str == 0x09 || (*str == 0x0a) || (*str == 0x0d)) {
    str++;
  }

  /* skip ending spaces */
  while (*str_last == 0x20 || *str_last == 0x09 || (*str_last == 0x0a) || (*str_last == 0x0d)) {
    str_last--;
  }

  int cnt = 0;
  int scanned;
  CamCcProfileName_t f;
  memset(f, 0, sizeof(f));

  /* parse the c-string */
  while ((str != str_last) && (cnt < num)) {
    scanned = sscanf(str, "%19s", f);
    if (scanned != 1) {
      LOGE( "%s(%d): parse error!\n", __FUNCTION__,__LINE__);
      goto err1;
    } else {
      strncpy(values[cnt], f, strlen(f));
      cnt++;
    }

    /* remove detected string */
    while ((*str != 0x20)  && (*str != ',') && (*str != ']') && (str != str_last)) {
      str++;
    }

    if (str != str_last) {
      /* skip spaces and comma */
      while ((*str == 0x20) || (*str == ',')) {
        str++;
      }
    }

    memset(f, 0, sizeof(f));
  }

#ifdef DEBUG_LOG
  for (int i = 0; i < cnt; i++) {
	LOGD( "%s,\n", values[i]);
  }
#endif
  return (cnt);

err1:
  memset(values, 0, (sizeof(uint16_t) * num));

  return (0);
}



/******************************************************************************
 * ParseLscProfileArray
 *****************************************************************************/
static int ParseLscProfileArray
(
    const char*          c_string,          /**< trimmed c string */
    CamLscProfileName_t values[],           /**< pointer to memory */
    const int           num                 /**< number of expected float values */
) {
  char* str = (char*)c_string;

  int last = strlen(str);

  /* calc. end adress of string */
  char* str_last = str + (last - 1);

  /* skip beginning spaces */
  while (*str == 0x20 || *str == 0x09 || (*str == 0x0a) || (*str == 0x0d)) {
    str++;
  }

  /* skip ending spaces */
  while (*str_last == 0x20 || *str_last == 0x09  || (*str_last == 0x0a) || (*str_last == 0x0d)) {
    str_last--;
  }

  int cnt = 0;
  int scanned;
  CamLscProfileName_t f;
  memset(f, 0, sizeof(f));

  /* parse the c-string */
  while ((str != str_last) && (cnt < num)) {
    scanned = sscanf(str, "%24s", f);
    if (scanned != 1) {
      LOGE( "%s(%d): parse error!\n", __FUNCTION__,__LINE__);
      goto err1;
    } else {
      strncpy(values[cnt], f, strlen(f));
      cnt++;
    }

    /* remove detected string */
    while ((*str != 0x20)  && (*str != ',') && (*str != ']') && (str != str_last)) {
      str++;
    }

    if (str != str_last) {
      /* skip spaces and comma */
      while ((*str == 0x20) || (*str == ',')) {
        str++;
      }
    }

    memset(f, 0, sizeof(f));
  }

#ifdef DEBUG_LOG
  for (int i = 0; i < cnt; i++) {
	LOGD( "%s,\n", values[i]);
  }
#endif
  return (cnt);

err1:
  memset(values, 0, (sizeof(uint16_t) * num));

  return (0);
}




/******************************************************************************
 * CalibDb::CalibDb
 *****************************************************************************/
CalibDb::CalibDb
(
) {
  m_CalibDbHandle = NULL;
  uint32_t MagicVerCode = calib_check_calc_checksum();
  LOGI("\n***************************************************************\n"
         "  Calibdb Version IS:%s   Magic Version Code IS %u"
         "\n***************************************************************\n"
         , CODE_XML_PARSE_VERSION, MagicVerCode);

  m_CalibInfo.IQMagicVerCode = MagicVerCode;
}



/******************************************************************************
 * CalibReader::CalibReader
 *****************************************************************************/
CalibDb::~CalibDb() {
  if (m_CalibDbHandle != NULL) {
    RESULT result = CamCalibDbRelease(&m_CalibDbHandle);
    DCT_ASSERT(result == RET_SUCCESS);
  }
}



/******************************************************************************
 * CalibDb::CalibDb
 *****************************************************************************/
bool CalibDb::CreateCalibDb
(
    const XMLElement*  root
) {
  bool res = true;

  // create calibration-database (c-code)
  RESULT result = CamCalibDbCreate(&m_CalibDbHandle);
  DCT_ASSERT(result == RET_SUCCESS);

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif
  
  XML_CHECK_START(CALIB_FILESTART_TAG_ID, CALIB_FILESTART_TAG_ID);
  
  // get and parse header section
  const XMLElement* header = root->FirstChildElement(TAG_NAME(CALIB_HEADER_TAG_ID));
  XmlTag tag = XmlTag(header);
  if (!header) {
    res = parseEntryHeader(header->ToElement(), NULL);
    if (!res) {
      return (res);
    }
	XML_CHECK_TOPTAG_MARK(CALIB_HEADER_TAG_ID, tag.Type(), tag.Size());
	
  }


  // get and parse sensor section
  const XMLElement* sensor = root->FirstChildElement(TAG_NAME(CALIB_SENSOR_TAG_ID));
  XmlTag tag1 = XmlTag(sensor);
  if (!sensor) {
    res = parseEntrySensor(sensor->ToElement(), NULL);
    if (!res) {
      return (res);
    }
	XML_CHECK_TOPTAG_MARK(CALIB_SENSOR_TAG_ID, tag1.Type(), tag1.Size());
	
  }

  // get and parse system section
  const XMLElement* system = root->FirstChildElement(TAG_NAME(CALIB_SYSTEM_TAG_ID));
  XmlTag tag2 = XmlTag(system);
  if (!system) {
    res = parseEntrySystem(system->ToElement(), NULL);
    if (!res) {
      return (res);
    }
	XML_CHECK_TOPTAG_MARK(CALIB_SYSTEM_TAG_ID, tag2.Type(), tag2.Size());
	
  }

  XML_CHECK_END();
  
#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}



/******************************************************************************
 * CalibDb::readFile
 *****************************************************************************/
bool CalibDb::CreateCalibDb
(
    const char* device
) {
  //QString errorString;
  int errorID;
  XMLDocument doc;

  bool res = true;
#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  RESULT result = CamCalibDbCreate(&m_CalibDbHandle);
  DCT_ASSERT(result == RET_SUCCESS);
  errorID = doc.LoadFile(device);
#ifdef DEBUG_LOG
  LOGD( "%s(%d): doc.LoadFile filename:%s  errorID:%d\n",
  	 	__FUNCTION__, __LINE__, device, errorID);
#endif
  if (doc.Error()) {
#if 0
    LOGD( "%s(%d): Error: Parse error errorID %d\n", 
              __FUNCTION__,__LINE__,errorID);
#endif
    return (false);
  }
  XMLElement* proot = doc.RootElement();
  std::string tagname(proot->Name());
  if (tagname != TAG_NAME(CALIB_FILESTART_TAG_ID)){
    LOGE( "%s(%d): Error: Not a calibration data file\n");
    return (false);
  }

  XML_CHECK_START(CALIB_FILESTART_TAG_ID, CALIB_FILESTART_TAG_ID);
  
  // parse header section
  XMLElement* pheader = proot->FirstChildElement(TAG_NAME(CALIB_HEADER_TAG_ID));
  XmlTag tag = XmlTag(pheader);
  if (pheader) {
    res = parseEntryHeader(pheader->ToElement(), NULL);
    if (!res) {
	  LOGE( "%s(%d): parse error in Header section\n", __FUNCTION__, __LINE__);
      return (res);
    }
	XML_CHECK_TOPTAG_MARK(CALIB_HEADER_TAG_ID, tag.Type(), tag.Size());
	
  }

  // parse sensor section
  XMLElement* psensor = proot->FirstChildElement(TAG_NAME(CALIB_SENSOR_TAG_ID));
  XmlTag tag1 = XmlTag(psensor);
  if (psensor) {
    res = parseEntrySensor(psensor->ToElement(), NULL);
    if (!res) {
  	  LOGE( "%s(%d): parse error in Sensor section\n", __FUNCTION__, __LINE__);
      return (res);
    }
	XML_CHECK_TOPTAG_MARK(CALIB_SENSOR_TAG_ID, tag1.Type(), tag1.Size());
	
  }

  // parse system section
  XMLElement* psystem = proot->FirstChildElement(TAG_NAME(CALIB_SYSTEM_TAG_ID));
  XmlTag tag2 = XmlTag(psystem);
  if (psystem) {
    res = parseEntrySystem(psystem->ToElement(), NULL);
    if (!res) {
  	  LOGE( "%s(%d): parse error in System section\n", __FUNCTION__, __LINE__);
      return (res);
    }
	XML_CHECK_TOPTAG_MARK(CALIB_SYSTEM_TAG_ID, tag2.Type(), tag2.Size());
	
  }

  XML_CHECK_END();

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (res);
}




/******************************************************************************
 * CalibDb::parseEntryCell  ---- old function not inclue xml check
 *****************************************************************************/
bool CalibDb::parseEntryCell
(
    const XMLElement*   pelement,
    int                 noElements,
    parseCellContent    func,
    void*                param
) {
  int cnt = 0;

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild && (cnt < noElements)) {
    XmlCellTag tag = XmlCellTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());

    if (tagname == TAG_NAME(CALIB_CELL_TAG_ID)){
      bool result = (this->*func)(pchild->ToElement(), param);
      if (!result) {
        return (result);
      }
    } else {
#if 1
      LOGE("%s(%d): unknown cell tag: %s\n", __FUNCTION__,__LINE__, tagname.c_str());
#endif
      return (false);
    }

    pchild = pchild->NextSibling();
    cnt ++;
  }

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}

bool CalibDb::parseEntryCellForCheck(
	const XMLElement*   pelement,
	int                 noElements,
	uint32_t			cur_id,
	uint32_t	 		parent_id	)
{
#ifdef DEBUG_LOG
	  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	int cell_size = 0; 
	CALIB_IQ_TAG_ID_T cur_tag_id = (CALIB_IQ_TAG_ID_T)cur_id;
	CALIB_IQ_TAG_ID_T parent_tag_id = (CALIB_IQ_TAG_ID_T)parent_id;
    parseCellNoElement(pelement, noElements, &cell_size); 
    XML_CHECK_CELL_SET_SIZE(cell_size);

	if(cell_size != noElements){
		LOGD("%s(%d): Warning: parent_tagname:%s tag_name:%s define %d cell, but only use %d cells !!!!\n",
			__FUNCTION__, __LINE__, 
			TAG_NAME(parent_tag_id), TAG_NAME(cur_tag_id),
			noElements, cell_size);
  	}
	
#ifdef DEBUG_LOG
	  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

	return true;
}


/******************************************************************************
 * CalibDb::parseCellNoElement
 *****************************************************************************/
bool CalibDb::parseCellNoElement
(
    const XMLElement*   pelement,
    int                 noElements,
    int					*RealNo
) {
  int cnt = 0;

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild && (cnt < noElements)) {
    pchild = pchild->NextSibling();
    cnt ++;
  }

  *RealNo = cnt;
  
#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return true;
}


/******************************************************************************
 * CalibDb::parseEntryCell  ---- new function inclue xml check
 *****************************************************************************/
bool CalibDb::parseEntryCell
(
    const XMLElement*   pelement,
    int                 noElements,
    parseCellContent    func,
    void*                param,
    uint32_t	 cur_tag_id,
    uint32_t	 parent_tag_id			
) {
#ifdef DEBUG_LOG
	  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif
	
	parseEntryCellForCheck(pelement, noElements, cur_tag_id, parent_tag_id);

	bool res = parseEntryCell(pelement, noElements, func, param);

#ifdef DEBUG_LOG
	  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

	return res;

}


/******************************************************************************
 * CalibDb::parseEntryHeader
 *****************************************************************************/
bool CalibDb::parseEntryHeader
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamCalibDbMetaData_t meta_data;
  MEMSET(&meta_data, 0, sizeof(meta_data));
  meta_data.isp_output_type = isp_color_output_type;//default value

  XML_CHECK_START(CALIB_HEADER_TAG_ID, CALIB_FILESTART_TAG_ID);

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    const char* value = tag.Value();
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif
	
    if (XML_CHECK_TAGID_COMPARE(CALIB_HEADER_CREATION_DATE_TAG_ID)){
      strncpy(meta_data.cdate, value, sizeof(meta_data.cdate));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_HEADER_CREATOR_TAG_ID)){
      strncpy(meta_data.cname, value, sizeof(meta_data.cname));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_HEADER_GENERATOR_VERSION_TAG_ID)){
      strncpy(meta_data.cversion, value, sizeof(meta_data.cversion));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_HEADER_SENSOR_NAME_TAG_ID)){
      strncpy(meta_data.sname, value, sizeof(meta_data.sname));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_HEADER_SAMPLE_NAME_TAG_ID)){
      strncpy(meta_data.sid, value, sizeof(meta_data.sid));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_HEADER_RESOLUTION_TAG_ID)){
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryResolution, NULL, 
	  		(uint32_t)(CALIB_HEADER_RESOLUTION_TAG_ID), (uint32_t)(CALIB_HEADER_TAG_ID))) {
#if 1
		LOGE( "%s(%d):parse error in header resolution section (unknow tag: %s)\n", 
    		__FUNCTION__,__LINE__,tagname.c_str());
#endif

        return (false);
      }
	  
    } else if( XML_CHECK_TAGID_COMPARE(CALIB_HEADER_CODE_XML_PARSE_VERSION_TAG_ID)){
	  strncpy(meta_data.code_xml_parse_version, value, sizeof(meta_data.code_xml_parse_version));
    } else if( XML_CHECK_TAGID_COMPARE(CALIB_HEADER_ISP_OUTPUT_TYPE_TAG_ID)){
               
	 std::string s_value(value);
     if(s_value == CALIB_HEADER_ISP_OUTPUT_COLOR_TYPE_TAG){
		    meta_data.isp_output_type = isp_color_output_type;
     }else if(s_value == CALIB_HEADER_ISP_OUTPUT_GRAY_TYPE_TAG){     	
		 	meta_data.isp_output_type = isp_gray_output_type;
     }else if(s_value == CALIB_HEADER_ISP_OUTPUT_MIXTURE_TYPE_TAG){
		 	meta_data.isp_output_type = isp_mixture_output_type;
     }else{
	     
#if 1
  		LOGD( "%s(%d):parse error in isp_output_type section (unknow tag: %s)\n", 
    		__FUNCTION__,__LINE__,tagname.c_str());
#endif		 	
		return (false);
     }	

	}else if( XML_CHECK_TAGID_COMPARE(CALIB_HEADER_SDK_IQ_VERIFY_ID)){
	  strncpy(meta_data.sdk_iq_verify, value, sizeof(meta_data.sdk_iq_verify));
    }else {
#if 1
	  LOGD( "%s(%d): parse error in header section (unknow tag: %s)\n", 
	  		__FUNCTION__,__LINE__,tagname.c_str());
#endif

      return (false);
    }

	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
  if(strcmp(meta_data.code_xml_parse_version, CODE_XML_PARSE_VERSION)){
	LOGD("%s(%d): code xml parse version is no match (%s) != (%s) \n", 
		__FUNCTION__, __LINE__, meta_data.code_xml_parse_version, CODE_XML_PARSE_VERSION);
	return (false);
  }

  RESULT result = CamCalibDbSetMetaData(m_CalibDbHandle, &meta_data);
  DCT_ASSERT(result == RET_SUCCESS);

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  


  return (true);
}



/******************************************************************************
 * CalibDb::parseEntryResolution
 *****************************************************************************/
bool CalibDb::parseEntryResolution
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  
  CamResolution_t resolution;
  MEMSET(&resolution, 0, sizeof(resolution));
  ListInit(&resolution.framerates);

  XML_CHECK_START(CALIB_HEADER_RESOLUTION_TAG_ID, CALIB_HEADER_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    const char* value = tag.Value();
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif
    if(XML_CHECK_TAGID_COMPARE(CALIB_HEADER_RESOLUTION_NAME_TAG_ID)){
      strncpy(resolution.name, value, sizeof(resolution.name));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_HEADER_RESOLUTION_WIDTH_TAG_ID)){
      int no = ParseUshortArray(value,  &resolution.width, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_HEADER_RESOLUTION_HEIGHT_TAG_ID)){
      int no = ParseUshortArray(value, &resolution.height, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_HEADER_RESOLUTION_FRATE_TAG_ID)){
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryFramerates, &resolution,
	  		(uint32_t)(CALIB_HEADER_RESOLUTION_FRATE_TAG_ID), (uint32_t)(CALIB_HEADER_RESOLUTION_TAG_ID))) {
		LOGE( "%s(%d):parse error in header resolution(unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
        return (false);
      }
	  
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_HEADER_RESOLUTION_ID_TAG_ID)){
      bool ok;

      resolution.id = tag.ValueToUInt(&ok);
      if (!ok) {
#if 1
        LOGE( "%s(%d): parse error: invalid resolution %s/%s\n", __FUNCTION__,__LINE__,tagname.c_str(),tag.Value());
#endif
        return (false);
      }
    } else {
      LOGD( "%s(%d): unknown tag: %s\n", __FUNCTION__,__LINE__,tagname.c_str());
      return (false);
    }
	
    pchild = pchild->NextSibling();

  }

  XML_CHECK_END();
   
  RESULT result = CamCalibDbAddResolution(m_CalibDbHandle, &resolution);
  DCT_ASSERT(result == RET_SUCCESS);

  // free linked framerates
  List* l = ListRemoveHead(&resolution.framerates);
  while (l) {
    List* tmp = ListRemoveHead(l);
    free(l);
    l = tmp;
  }

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}


/******************************************************************************
 * CalibDb::parseEntryDpccRegisters
 *****************************************************************************/
bool CalibDb::parseEntryFramerates
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamResolution_t* pResolution = (CamResolution_t*)param;
  CamFrameRate_t* pFrate = (CamFrameRate_t*) malloc(sizeof(CamFrameRate_t));
  if (!pFrate) {
    return false;
  }
  MEMSET(pFrate, 0, sizeof(*pFrate));

  XML_CHECK_START(CALIB_HEADER_RESOLUTION_FRATE_TAG_ID, CALIB_HEADER_RESOLUTION_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    const char* value = tag.Value();
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_HEADER_RESOLUTION_FRATE_NAME_TAG_ID)){
      snprintf(pFrate->name, CAM_FRAMERATE_NAME, "%s_%s",
               pResolution->name, value);
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_HEADER_RESOLUTION_FRATE_FPS_TAG_ID)){
      int no = ParseFloatArray(value, &pFrate->fps, 1);
      DCT_ASSERT((no == 1));
    } else {
#if 1
      LOGE( "%s(%d):parse error in framerate section (unknow tag: %s)\n",                                                  
      	__FUNCTION__,__LINE__,tagname.c_str());
#endif
      return (false);
    }
	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
  if (pResolution) {
    ListPrepareItem(pFrate);
    ListAddTail(&pResolution->framerates, pFrate);
  }

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}


/******************************************************************************
 * CalibDb::parseEntrySensor
 *****************************************************************************/
bool CalibDb::parseEntrySensor
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);

  XML_CHECK_START(CALIB_SENSOR_TAG_ID, CALIB_FILESTART_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_TAG_ID)){
      if (!parseEntryAwb(pchild->ToElement())) {
#if 1
	    LOGE( "%s(%d):parse error in AWB section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
        return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LSC_TAG_ID)){
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryLsc, NULL,
	  		(uint32_t)(CALIB_SENSOR_LSC_TAG_ID), (uint32_t)(CALIB_SENSOR_TAG_ID))) {
#if 1
	    LOGE( "%s(%d):parse error in LSC section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
		return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_CC_TAG_ID)){
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryCc, NULL,
	  		(uint32_t)(CALIB_SENSOR_CC_TAG_ID), (uint32_t)(CALIB_SENSOR_TAG_ID))) {
#if 1
	    LOGE( "%s(%d):parse error in CC section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
        return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_TAG_ID)){
      if (!parseEntryAf(pchild->ToElement())) {
#if 1
	    LOGE( "%s(%d):parse error in AF section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
        return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_TAG_ID)){
      if (!parseEntryAec(pchild->ToElement())) {
#if 1
	    LOGE( "%s(%d):parse error in AEC section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
        return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_BLS_TAG_ID)){
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryBls, NULL,
	  		(uint32_t)(CALIB_SENSOR_BLS_TAG_ID), (uint32_t)(CALIB_SENSOR_TAG_ID))) {
#if 1
      	LOGE( "%s(%d):parse error in BLS section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
        return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DEGAMMA_TAG_ID)){
    
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_TAG_ID)){
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryWdr, NULL,
	  		(uint32_t)(CALIB_SENSOR_WDR_TAG_ID), (uint32_t)(CALIB_SENSOR_TAG_ID))) {
#if 1
	  	LOGE( "%s(%d):parse error in WDR section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
        return ( false );
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_CAC_TAG_ID)){
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryCac, NULL,
	  		(uint32_t)(CALIB_SENSOR_CAC_TAG_ID), (uint32_t)(CALIB_SENSOR_TAG_ID))) {
#if 1
        LOGE( "%s(%d):parse error in CAC section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
        return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_TAG_ID)){
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryDpf, NULL,
	  		(uint32_t)(CALIB_SENSOR_DPF_TAG_ID), (uint32_t)(CALIB_SENSOR_TAG_ID))) {
#if 1
        LOGE( "%s(%d):parse error in DPF section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
        return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPCC_TAG_ID)){
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryDpcc, NULL,
	  		(uint32_t)(CALIB_SENSOR_DPCC_TAG_ID), (uint32_t)(CALIB_SENSOR_TAG_ID))) {
#if 1
        LOGE( "%s(%d):parse error in DPCC section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
        return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_GOC_TAG_ID)){
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryGoc, NULL,
	  		(uint32_t)(CALIB_SENSOR_GOC_TAG_ID), (uint32_t)(CALIB_SENSOR_TAG_ID))) {
#if 1
		LOGE( "%s(%d):parse error in GOC section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
        return ( false );
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_CPROC_TAG_ID)){
      CamCprocProfile_t cproc;
      cproc.num_items = 0;
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryCproc,  &cproc,
	  		(uint32_t)(CALIB_SENSOR_CPROC_TAG_ID), (uint32_t)(CALIB_SENSOR_TAG_ID))) {
#if 1
		LOGE( "%s(%d):parse error in CPROC section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
        return ( false );
      } else
        CamCalibDbAddCproc(m_CalibDbHandle, &cproc);

    }else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_TAG_ID))
    {
      if(!parseEntryCell(pchild->ToElement(),tag.Size(),&CalibDb::parseEntryRKsharpen, NULL,
	  		(uint32_t)(CALIB_SENSOR_IESHARPEN_TAG_ID), (uint32_t)(CALIB_SENSOR_TAG_ID)))
      {
#if 1
        LOGE( "%s(%d):parse error in IESHARPEN section (unknow tag: %s) werring.wu !!!\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
		return ( false );
      }
    }else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_OTP_TAG_ID))
    {
      if (!parseEntryOTP(pchild->ToElement())) {
#if 1
      	LOGE( "%s(%d):parse error in OTP section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif			
        return (false);
      }	
    }
	else {
#if 1
      LOGE( "%s(%d):parse error in matfile section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
      return (false);
    }
	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}

/******************************************************************************
 * CalibDb::parseEntryAfWin
 *****************************************************************************/
bool CalibDb::parseEntryAfWin
(
    const XMLElement*   pelement,
    void*                param,
    uint32_t       parent_id
) {

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  XML_CHECK_START((CALIB_IQ_TAG_ID_T)(parent_id), CALIB_SENSOR_AF_TAG_ID);
  
  Cam_Win* pAfwin = (Cam_Win*)param;

  if(NULL == pAfwin){
  	LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
  	return false;
  }

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_H_OFFS_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &pAfwin->h_offs, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_V_OFFS_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &pAfwin->v_offs, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_H_SIZE_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &pAfwin->h_size, 1);
	  DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_V_SIZE_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &pAfwin->v_size, 1);
	  DCT_ASSERT((no == tag.Size()));
    } else {
	#if 1
      LOGE( "%s(%d):parse error in af winsection (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
	#endif
      return (false);
    }
	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}


/******************************************************************************
 * CalibDb::parseEntryAf
 *****************************************************************************/
 bool CalibDb::parseEntryContrastAf
(
    const XMLElement*   pelement,
    void*                param
) {

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamCalibContrastAf_t* pContrastAf = (CamCalibContrastAf_t*)param;
  if(NULL == pContrastAf){
  	LOGD("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
  	return false;
  }

  XML_CHECK_START(CALIB_SENSOR_AF_CONTRAST_AF_TAG_ID, CALIB_SENSOR_AF_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_CONTRAST_ENABLE_TAG_ID)){
      int no = ParseUcharArray(tag.Value(), &pContrastAf->enable, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_SEARCH_STRATEGY_TAG_ID)){
      char* search_strategy = Toupper(tag.Value());
      if(strcmp(search_strategy,"FULLRANGE")==0) {
        pContrastAf->Afss = CAM_AFM_FSS_FULLRANGE;
      } else if (strcmp(search_strategy,"HILLCLIMBING")==0) {
        pContrastAf->Afss = CAM_AFM_FSS_HILLCLIMBING;
      } else if (strcmp(search_strategy,"ADAPTIVE_RANGE")==0) {
        pContrastAf->Afss = CAM_AFM_FSS_ADAPTIVE_RANGE;
      } else {
        pContrastAf->Afss = CAM_AFM_FSS_INVALID;
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_FULL_DIR_TAG_ID)){
      char* search_dir = Toupper(tag.Value());
      if(strcmp(search_dir,"POSITIVE")==0) {
        pContrastAf->FullDir = CAM_AFM_POSITIVE_SEARCH;
      } else if (strcmp(search_dir,"NEGATIVE")==0) {
        pContrastAf->FullDir = CAM_AFM_NEGATIVE_SEARCH;
      } else if (strcmp(search_dir,"ADAPTIVE")==0) {
        pContrastAf->FullDir = CAM_AFM_ADAPTIVE_SEARCH;
      } else {
        pContrastAf->FullDir = CAM_AFM_ADAPTIVE_SEARCH;
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_FULL_RANGE_TBL_TAG_ID)){
      int ArraySize     = tag.Size();
	  pContrastAf->FullSteps = ArraySize;
      pContrastAf->FullRangeTbl = (uint16_t *)malloc(ArraySize * sizeof(uint16_t));
	  if(pContrastAf->FullRangeTbl == NULL){
		LOGE("%s(%d): af full range table malloc fail! \n",  __FUNCTION__,__LINE__ );
	  }
      MEMSET( pContrastAf->FullRangeTbl, 0, (ArraySize * sizeof( uint16_t )) );
      int no = ParseUshortArray( tag.Value(), pContrastAf->FullRangeTbl, ArraySize );
      DCT_ASSERT( (no == ArraySize) );
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_ADAPTIVE_DIR_TAG_ID)){
      char* search_dir = Toupper(tag.Value());
      if(strcmp(search_dir,"POSITIVE")==0) {
        pContrastAf->AdaptiveDir = CAM_AFM_POSITIVE_SEARCH;
      } else if (strcmp(search_dir,"NEGATIVE")==0) {
        pContrastAf->AdaptiveDir = CAM_AFM_NEGATIVE_SEARCH;
      } else if (strcmp(search_dir,"ADAPTIVE")==0) {
        pContrastAf->AdaptiveDir = CAM_AFM_ADAPTIVE_SEARCH;
      } else {
        pContrastAf->AdaptiveDir = CAM_AFM_ADAPTIVE_SEARCH;
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_ADAPTIVE_RANGE_TBL_TAG_ID)){
      int ArraySize	  = tag.Size();
      pContrastAf->AdaptiveSteps = ArraySize;
      pContrastAf->AdaptRangeTbl = (uint16_t *)malloc(ArraySize * sizeof(uint16_t));
      if(pContrastAf->AdaptRangeTbl == NULL){
		LOGE("%s(%d): af adaptive range table malloc fail! \n",  __FUNCTION__,__LINE__ );
      }
      MEMSET( pContrastAf->AdaptRangeTbl, 0, (ArraySize * sizeof( uint16_t)) );
      int no = ParseUshortArray( tag.Value(), pContrastAf->AdaptRangeTbl, ArraySize );
      DCT_ASSERT( (no == ArraySize) );
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_TRIG_THERS_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &pContrastAf->TrigThers, tag.Size());
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_TRIG_VALUE_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &pContrastAf->TrigValue, tag.Size());	  
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_TRIG_FRAMES_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &pContrastAf->TrigFrames, tag.Size());
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_TRIG_ANTI_FLASH_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &pContrastAf->TrigAntiFlash, tag.Size());
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_FINISH_THERS_MAIN_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &pContrastAf->FinishThersMain, tag.Size());
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_FINISH_THERS_SUB_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &pContrastAf->FinishThersSub, tag.Size());
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_FINISH_THERS_OFFSET_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &pContrastAf->FinishThersOffset, tag.Size());
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_STABLE_THERS_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &pContrastAf->StableThers, tag.Size());
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_STABLE_VALUE_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &pContrastAf->StableValue, tag.Size());
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_STABLE_FRAMES_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &pContrastAf->StableFrames, tag.Size());
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_STABLE_TIME_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &pContrastAf->StableTime, tag.Size());
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_OUT_FOCUS_VALUE_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &pContrastAf->OutFocusValue, tag.Size());
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_OUT_FOCUS_LUMA_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &pContrastAf->OutFocusLuma, tag.Size());
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_OUT_FOCUS_POS_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &pContrastAf->OutFocusPos, tag.Size());
      DCT_ASSERT((no == tag.Size()));
    } else {
      LOGE( "%s(%d):parse error in ContrastAf section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
      return (false);
    }
	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}

 bool CalibDb::parseEntryPdaf
(
    const XMLElement*   pelement,
    void*                param
) {

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamCalibPdaf_t* pPdaf = (CamCalibPdaf_t*)param;
  if(NULL == pPdaf){
  	LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
  	return false;
  }

  XML_CHECK_START(CALIB_SENSOR_AF_PDAF_TAG_ID, CALIB_SENSOR_AF_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_PDAF_ENABLE_TAG_ID)){
      int no = ParseUcharArray(tag.Value(), &pPdaf->enable, 1);
      DCT_ASSERT((no == tag.Size()));
    } else {
      LOGE( "%s(%d):parse error in PDAF section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
      return (false);
    }
	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}

 bool CalibDb::parseEntryLaserAf
(
    const XMLElement*   pelement,
    void*                param
) {

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamCalibLaserAf_t* pLaserAf = (CamCalibLaserAf_t*)param;
  if(NULL == pLaserAf){
    LOGD("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
    return false;
  }

  XML_CHECK_START(CALIB_SENSOR_AF_LASER_AF_TAG_ID, CALIB_SENSOR_AF_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_LASERAF_ENABLE_TAG_ID)){
      int no = ParseUcharArray(tag.Value(), &pLaserAf->enable, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_LASER_AF_VCMDOT_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), pLaserAf->vcmDot, 7);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_LASER_AF_DISTANCEDOT_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), pLaserAf->distanceDot, 7);
      DCT_ASSERT((no == tag.Size()));
    } else {
      LOGE( "%s(%d):parse error in LaserAf section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
      return (false);
    }
	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}

bool CalibDb::parseEntryAf
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamCalibAfGlobal_t af_data;
  memset(&af_data, 0, sizeof(af_data));

  XML_CHECK_START(CALIB_SENSOR_AF_TAG_ID, CALIB_SENSOR_TAG_ID);

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_WINDOW_NUM_TAG_ID)){
      int no = ParseUcharArray(tag.Value(), &af_data.Window_Num, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_WINDOW_A_TAG_ID)){
      if (!parseEntryAfWin(pchild->ToElement(), &af_data.WindowA, CALIB_SENSOR_AF_WINDOW_A_TAG_ID)){
	    return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_WINDOW_B_TAG_ID)){
      if (!parseEntryAfWin(pchild->ToElement(), &af_data.WindowB, CALIB_SENSOR_AF_WINDOW_B_TAG_ID)){
        return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_WINDOW_C_TAG_ID)){
      if (!parseEntryAfWin(pchild->ToElement(), &af_data.WindowC, CALIB_SENSOR_AF_WINDOW_C_TAG_ID)){
        return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_CONTRAST_AF_TAG_ID)){
      if (!parseEntryContrastAf(pchild->ToElement(), &af_data.contrast_af)) {
        return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_LASER_AF_TAG_ID)){
      if (!parseEntryLaserAf(pchild->ToElement(), &af_data.laser_af)) {
        return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AF_PDAF_TAG_ID)){
      if (!parseEntryPdaf(pchild->ToElement(), &af_data.pdaf)) {
        return (false);
      }
    } else {
      LOGE( "%s(%d):parse error in AF section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
      return (false);
    }
	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  RESULT result = CamCalibDbAddAfGlobal(m_CalibDbHandle, &af_data);

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}

bool CalibDb::parseEntryAecDON
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	CamCalibAecGlobal_t *pAecData= (CamCalibAecGlobal_t *)param;
	if(NULL == pAecData){
		LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
		return false;
	}

	XML_CHECK_START(CALIB_SENSOR_AEC_DON_TAG_ID, CALIB_SENSOR_AEC_TAG_ID);
  
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif
		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_DON_NIGHT_TRIGGER_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &pAecData->DON_Night_Trigger, 1);
          DCT_ASSERT((no == tag.Size()));
        } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_DON_NIGHT_MODE_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &pAecData->DON_Night_Mode, 1);
          DCT_ASSERT((no == tag.Size()));
        } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_DON_DAY2NIGHT_FAC_TH_TAG_ID)){
          int no = ParseFloatArray(tag.Value(), &pAecData->DON_Day2Night_Fac_th, 1);
          DCT_ASSERT((no == tag.Size()));
        } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_DON_NIGHT2DAY_FAC_TH_TAG_ID)){
          int no = ParseFloatArray(tag.Value(), &pAecData->DON_Night2Day_Fac_th, 1);
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_DON_BOUNCING_TH_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &pAecData->DON_Bouncing_th, 1);
          DCT_ASSERT((no == tag.Size()));
        } else {
		  LOGE( "%s(%d):parse error in AEC DON section (unknow tag: %s)\n",
				  __FUNCTION__,__LINE__,tagname.c_str());
        }
		
        pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}



bool CalibDb::parseEntryAecFPSSetConfig
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	
	CamCalibAecGlobal_t *pAecData= (CamCalibAecGlobal_t *)param;
	if(NULL == pAecData){
		LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
		return false;
	}

	XML_CHECK_START(CALIB_SENSOR_AEC_FPS_FPS_SET_CONFIG_TAG_ID, CALIB_SENSOR_AEC_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();	
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    	if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_FPS_FPS_SET_ENABLE_TAG_ID))
		{
  			int no = ParseUcharArray(tag.Value(), &pAecData->FpsSetEnable, 1);
  			DCT_ASSERT((no == 1));
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_FPS_IS_FPS_FIX_TAG_ID))
		{
			int no = ParseUcharArray(tag.Value(), &pAecData->isFpsFix, 1);
  			DCT_ASSERT((no == 1));
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_FPS_FPS_FIX_TIMEDOT_TAG_ID))
		{
			int i = (sizeof(pAecData->FpsFixTimeDot) / sizeof(pAecData->FpsFixTimeDot.fCoeff[0]));
  			int no = ParseFloatArray(tag.Value(), pAecData->FpsFixTimeDot.fCoeff, i);
  			DCT_ASSERT((no == i));
		}else{
			 LOGE( "%s(%d):parse error in AEC fps setting section (unknow tag: %s)\n",
				  __FUNCTION__,__LINE__,tagname.c_str());
			 return false;
		}
		
		pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}



bool CalibDb::parseEntryAecHist2Hal
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	
	CamCalibAecGlobal_t *pAecData= (CamCalibAecGlobal_t *)param;
	if(NULL == pAecData){
		LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
		return false;
	}
	
	XML_CHECK_START(CALIB_SENSOR_AEC_HIST_2_HAL_TAG_ID, CALIB_SENSOR_AEC_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_HIST_2_HAL_ENABLE_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &pAecData->hist2Hal.enable, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_HIST_2_HAL_LOWHISTBINTH_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &pAecData->hist2Hal.lowHistBinTh, tag.Size());
          DCT_ASSERT((no == tag.Size()));
    	}else {
			LOGE( "%s(%d): parse error in AEC_HIST_2_HAL section (unknow tag:%s)\n", 
				__FUNCTION__,__LINE__,tagname.c_str());
			return false;
        }
		
		pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}


bool CalibDb::parseEntryAecNLSC
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	
	CamCalibAecGlobal_t *pAecData= (CamCalibAecGlobal_t *)param;
	if(NULL == pAecData){
		LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
		return false;
	}
	
	XML_CHECK_START(CALIB_SENSOR_AEC_NLSC_CONFIG_TAG_ID, CALIB_SENSOR_AEC_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_NLSC_ENABLE_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &pAecData->NLSC_config.enable, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_NLSC_IR_RG_TAG_ID)){
          int no = ParseFloatArray(tag.Value(), &pAecData->NLSC_config.IR_rg, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_NLSC_IR_BG_TAG_ID)){
          int no = ParseFloatArray(tag.Value(), &pAecData->NLSC_config.IR_bg, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_NLSC_MAX_DIS_TAG_ID)){
          int no = ParseFloatArray(tag.Value(), &pAecData->NLSC_config.Max_dis, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_NLSC_COLOR2BLACK_COUNT_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &pAecData->NLSC_config.C2B_count, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_NLSC_COLOR2BLACK_THRESH_TAG_ID)){
          int no = ParseFloatArray(tag.Value(), &pAecData->NLSC_config.C2B_thresh, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_NLSC_BLACK2COLOR_COUNT_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &pAecData->NLSC_config.B2C_count, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_NLSC_BLACK2COLOR_THRESH_TAG_ID)){
          int no = ParseFloatArray(tag.Value(), &pAecData->NLSC_config.B2C_thresh, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_NLSC_BLACK2COLOR_VB_PERCENT_TAG_ID)){
          int no = ParseFloatArray(tag.Value(), &pAecData->NLSC_config.B2C_vb_percent, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_NLSC_COLOR2BLACK_STABLE_FPS_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &pAecData->NLSC_config.C2B_stable_fps, tag.Size());
          DCT_ASSERT((no == tag.Size()));
		}else {
          LOGE( "%s(%d): parse error in AEC_NLSC_Config section (unknow tag:%s)\n", 
		  	__FUNCTION__,__LINE__,tagname.c_str());
		  return false;
        }	
		
		pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}



bool CalibDb::parseEntryAecIntervalAdjustStrategy
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	
	CamCalibAecGlobal_t *pAecData= (CamCalibAecGlobal_t *)param;
	if(NULL == pAecData){
		LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
		return false;
	}

	XML_CHECK_START(CALIB_SENSOR_AEC_INTERVAL_ADJUST_STRATEGY_TAG_ID, CALIB_SENSOR_AEC_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_INTERVAL_ADJUST_ENABLE_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &pAecData->InterAdjustStrategy.enable, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_INTERVAL_ADJUST_DLUMA_HIGH_TH_TAG_ID)){
          int no = ParseFloatArray(tag.Value(), &pAecData->InterAdjustStrategy.dluma_high_th, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_INTERVAL_ADJUST_DLUMA_LOW_TH_TAG_ID)){
          int no = ParseFloatArray(tag.Value(), &pAecData->InterAdjustStrategy.dluma_low_th, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_INTERVAL_ADJUST_TRIGGER_FRAME_TAG_ID)){
          int no = ParseUintArray(tag.Value(), &pAecData->InterAdjustStrategy.trigger_frame, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else {
		  LOGE( "%s(%d): parse error in AEC interval adjust strategy  section (unknow tag:%s)\n",
		  	  __FUNCTION__,__LINE__,tagname.c_str());
		  return false;
        }
		
		pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}




bool CalibDb::parseEntryAecBacklight
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	
	CamCalibAecGlobal_t *pAecData= (CamCalibAecGlobal_t *)param;
	if(NULL == pAecData){
		LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
		return false;
	}
	
	XML_CHECK_START(CALIB_SENSOR_AEC_BACKLIGHT_CONFIG_TAG_ID, CALIB_SENSOR_AEC_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_BACKLIGHT_ENABLE_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &pAecData->backLightConf.enable, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_BACKLIGHT_LUMALOWTH_TAG_ID)){
          int no = ParseFloatArray(tag.Value(), &pAecData->backLightConf.lumaLowTh, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_BACKLIGHT_LUMAHIGHTH_TAG_ID)){
          int no = ParseFloatArray(tag.Value(), &pAecData->backLightConf.lumaHighTh, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_BACKLIGHT_WEIGHTMINTH_TAG_ID)){
          int no = ParseFloatArray(tag.Value(), &pAecData->backLightConf.weightMinTh, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_BACKLIGHT_WEIGHTMAXTH_TAG_ID)){
          int no = ParseFloatArray(tag.Value(), &pAecData->backLightConf.weightMaxTh, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else {
          LOGE( "%s(%d): parse error in AEC_backlight section (unknow tag:%s)\n",
		  	 __FUNCTION__,__LINE__,tagname.c_str());
		  return false;
        }
		
		pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}


bool CalibDb::parseEntryAecLockAE
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif
	
	CamCalibAecGlobal_t *pAecData= (CamCalibAecGlobal_t *)param;
	if(NULL == pAecData){
		LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
		return false;
	}
	
	XML_CHECK_START(CALIB_SENSOR_LOCK_AE_TAG_ID, CALIB_SENSOR_AEC_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LOCK_AE_ENABEL_TAG_ID)){
			int no = ParseUcharArray(tag.Value(), &pAecData->LockAE_enable, 1);
			DCT_ASSERT((no == tag.Size()));
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LOCK_AE_GAINVALUE_TAG_ID)){
	      int i = (sizeof(pAecData->GainValue) / sizeof(pAecData->GainValue.fCoeff[0]));
	      int no = ParseFloatArray(tag.Value(), pAecData->GainValue.fCoeff, i);
	      DCT_ASSERT((no == tag.Size()));
	    }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LOCK_AE_TIMEVALUE_TAG_ID)){
	      int i = (sizeof(pAecData->TimeValue) / sizeof(pAecData->TimeValue.fCoeff[0]));
	      int no = ParseFloatArray(tag.Value(), pAecData->TimeValue.fCoeff, i);
	      DCT_ASSERT((no == tag.Size()));
	    }else {
		  LOGE( "%s(%d):parse error in AEC LockAE section (unknow tag: %s)\n",
			__FUNCTION__,__LINE__,tagname.c_str());
		  return false;
        }
		
		pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}


bool CalibDb::parseEntryAecHdrCtrlLframe
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	
	CamCalibAecGlobal_t *pAecData= (CamCalibAecGlobal_t *)param;
	if(NULL == pAecData){
		LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
		return false;
	}

	XML_CHECK_START(CALIB_SENSOR_HDRCTRL_LFRAMECTRL_TAG_ID, CALIB_SENSOR_HDRCTRL_TAG_ID);

	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

		if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_LGAINS_TAG_ID)){
	      	int i = (sizeof(pAecData->HdrCtrl.Lgains) / sizeof(pAecData->HdrCtrl.Lgains.fCoeff[0]));
	      	int no = ParseFloatArray(tag.Value(), pAecData->HdrCtrl.Lgains.fCoeff, i);
	      	DCT_ASSERT((no == tag.Size()));
		}else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_LEXPLEVEL_TAG_ID)){
	      	int i = (sizeof(pAecData->HdrCtrl.LExpLevel) / sizeof(pAecData->HdrCtrl.LExpLevel.fCoeff[0]));
	      	int no = ParseFloatArray(tag.Value(), pAecData->HdrCtrl.LExpLevel.fCoeff, i);
	      	DCT_ASSERT((no == tag.Size()));
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_LSETPOINT_TAG_ID)){
	      	int i = (sizeof(pAecData->HdrCtrl.LSetPoint) / sizeof(pAecData->HdrCtrl.LSetPoint.fCoeff[0]));
	      	int no = ParseFloatArray(tag.Value(), pAecData->HdrCtrl.LSetPoint.fCoeff, i);
	      	DCT_ASSERT((no == tag.Size()));
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_DARKLUMA_TAG_ID)){
	      	int i = (sizeof(pAecData->HdrCtrl.TargetDarkROILuma) / sizeof(pAecData->HdrCtrl.TargetDarkROILuma.fCoeff[0]));
	      	int no = ParseFloatArray(tag.Value(), pAecData->HdrCtrl.TargetDarkROILuma.fCoeff, i);
	      	DCT_ASSERT((no == tag.Size()));
		}else{
		   LOGE( "%s(%d):parse error in AEC HDRAE LframeCtrl section (unknow tag: %s)\n",
			  __FUNCTION__,__LINE__,tagname.c_str());
    	}
		
		pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}


bool CalibDb::parseEntryAecHdrCtrlSframe
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif
	
	CamCalibAecGlobal_t *pAecData= (CamCalibAecGlobal_t *)param;
	if(NULL == pAecData){
		LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
		return false;
	}
	
	XML_CHECK_START(CALIB_SENSOR_HDRCTRL_SFRAMECTRL_TAG_ID, CALIB_SENSOR_HDRCTRL_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();	
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

		if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_SGAINS_TAG_ID)){
	      	int i = (sizeof(pAecData->HdrCtrl.Sgains) / sizeof(pAecData->HdrCtrl.Sgains.fCoeff[0]));
	      	int no = ParseFloatArray(tag.Value(), pAecData->HdrCtrl.Sgains.fCoeff, i);
	      	DCT_ASSERT((no == tag.Size()));
		}else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_SEXPLEVEL_TAG_ID)){
	      	int i = (sizeof(pAecData->HdrCtrl.SExpLevel) / sizeof(pAecData->HdrCtrl.SExpLevel.fCoeff[0]));
	      	int no = ParseFloatArray(tag.Value(), pAecData->HdrCtrl.SExpLevel.fCoeff, i);
	      	DCT_ASSERT((no == tag.Size()));
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_OELUMA_TAG_ID)){
	      	int i = (sizeof(pAecData->HdrCtrl.TargetOELuma) / sizeof(pAecData->HdrCtrl.TargetOELuma.fCoeff[0]));
	      	int no = ParseFloatArray(tag.Value(), pAecData->HdrCtrl.TargetOELuma.fCoeff, i);
	      	DCT_ASSERT((no == tag.Size()));
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_SSETPOINT_TAG_ID)){
	      	int i = (sizeof(pAecData->HdrCtrl.SSetPoint) / sizeof(pAecData->HdrCtrl.SSetPoint.fCoeff[0]));
	      	int no = ParseFloatArray(tag.Value(), pAecData->HdrCtrl.SSetPoint.fCoeff, i);
	      	DCT_ASSERT((no == tag.Size()));
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_OELUMADIST_TAG_ID)){
	      	int no = ParseFloatArray(tag.Value(), &pAecData->HdrCtrl.OELumaDistTh, 1);
	  		DCT_ASSERT((no == tag.Size()));
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_OETOLERANCE_TAG_ID)){
	      	int no = ParseFloatArray(tag.Value(), &pAecData->HdrCtrl.OETolerance, 1);
	  		DCT_ASSERT((no == tag.Size()));
		}
		else{
		  LOGE( "%s(%d):parse error in AEC HDRAE SframeCtrl section (unknow tag: %s)\n",
			  __FUNCTION__,__LINE__,tagname.c_str());
		  return false;
    	}
		
		pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}


bool CalibDb::parseEntryAecHdrCtrl
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	
	CamCalibAecGlobal_t *pAecData= (CamCalibAecGlobal_t *)param;
	if(NULL == pAecData){
		LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
		return false;
	}
	
	XML_CHECK_START(CALIB_SENSOR_HDRCTRL_TAG_ID, CALIB_SENSOR_AEC_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_ENABEL_TAG_ID)){
			int no = ParseUcharArray(tag.Value(), &pAecData->HdrCtrl.Enable, 1);
			DCT_ASSERT((no == tag.Size()));
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_MODE_TAG_ID)){
	      char* value = Toupper(tag.Value());
	      std::string s_value(value);
#ifdef DEBUG_LOG
		  LOGD( "%s(%d): value: %s\n", __FUNCTION__,__LINE__,value);
		  LOGD( "%s(%d): s_value: %s\n", __FUNCTION__,__LINE__,s_value.c_str());
#endif
	      if (s_value == CALIB_SENSOR_HDRCTRL_MODE_DCG) {
	        pAecData->HdrCtrl.Mode= AEC_HDR_MODE_DCG;
	      } else if (s_value == CALIB_SENSOR_HDRCTRL_MODE_STAGGER) {
	        pAecData->HdrCtrl.Mode = AEC_HDR_MODE_STAGGER;
	      } else {
	        pAecData->HdrCtrl.Mode= AEC_HDR_MODE_INVALID;
			LOGE("%s(%d): invalid AEC HdrMode: %s \n", __FUNCTION__, __LINE__, s_value.c_str());
	      }
    	}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_FRAMENUM_TAG_ID)){
	      	int no = ParseUcharArray(tag.Value(), &pAecData->HdrCtrl.FrameNum, 1);
			DCT_ASSERT((no == tag.Size()));
	    }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_DCG_Ratio_TAG_ID)){
	      	int no = ParseFloatArray(tag.Value(), &pAecData->HdrCtrl.DCG_Ratio, 1);
			DCT_ASSERT((no == tag.Size()));
	    }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_M2S_Ratio_TAG_ID)){
	      int no = ParseFloatArray(tag.Value(), &pAecData->HdrCtrl.M2S_Ratio, 1);
		  DCT_ASSERT((no == tag.Size()));
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_L2M_Ratio_TAG_ID)){
	      int no = ParseFloatArray(tag.Value(), &pAecData->HdrCtrl.L2M_Ratio, 1);
		  DCT_ASSERT((no == tag.Size()));
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_LFRAMECTRL_TAG_ID)){
	      if (!parseEntryAecHdrCtrlLframe(pchild->ToElement(), pAecData)){
			LOGE( "%s(%d): parse error in hdr ctrl Lfrmae section\n",__FUNCTION__,__LINE__);
			return (false);
		  }
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_SFRAMECTRL_TAG_ID)){
	      if (!parseEntryAecHdrCtrlSframe(pchild->ToElement(), pAecData)){
			LOGE( "%s(%d): parse error in hdr ctrl Sfrmae section\n",__FUNCTION__,__LINE__);
			return (false);
		  }
		}else {
		  LOGE( "%s(%d):parse error in AEC HdrCtrl section (unknow tag: %s)\n",
				  __FUNCTION__,__LINE__,tagname.c_str());
        }
		
		pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}

/******************************************************************************
 * CalibDb::parseEntryAec
 *****************************************************************************/
bool CalibDb::parseEntryAec
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamCalibAecGlobal_t aec_data;
  memset(&aec_data, 0, sizeof(aec_data));
  ListInit(&aec_data.DySetpointList);
  ListInit(&aec_data.ExpSeparateList);

  XML_CHECK_START(CALIB_SENSOR_AEC_TAG_ID, CALIB_SENSOR_TAG_ID);
	
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_SETPOINT_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &aec_data.SetPoint, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_NIGHT_SETPOINT_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &aec_data.NightSetPoint, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_GAINRANGE_TAG_ID)){
      int i = tag.Size();
	  aec_data.GainRange.pGainRange = (float *)malloc(i*sizeof(float));
	  if(aec_data.GainRange.pGainRange == NULL){
		LOGD( "%s(%d): aec gain range malloc fail!\n", __FUNCTION__, __LINE__);
	  }
      int no = ParseFloatArray(tag.Value(), aec_data.GainRange.pGainRange, i);
      DCT_ASSERT((no == tag.Size()));
	  DCT_ASSERT(((i%7) == 0) );
	  aec_data.GainRange.array_size = i;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_TIMEFACTOR_TAG_ID)){
      int i = (sizeof(aec_data.TimeFactor) / sizeof(aec_data.TimeFactor[0]));
      int no = ParseFloatArray(tag.Value(), aec_data.TimeFactor, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_GRIDWEIGHTS_TAG_ID)){ //cxf
      uint8_t *pWeight  = NULL;
      int arraySize     = tag.Size();
      pWeight = (uint8_t *)malloc(arraySize * sizeof(uint8_t));
	  if(pWeight == NULL){
		LOGE( "%s(%d): aec gridWeight malloc fail!\n", __FUNCTION__, __LINE__);
	  }
      MEMSET( pWeight, 0, (arraySize * sizeof( uint8_t )) );
      int no = ParseUcharArray( tag.Value(), pWeight, arraySize );
      DCT_ASSERT( (no == arraySize) );

      aec_data.GridWeights.ArraySize = arraySize;
      aec_data.GridWeights.pWeight = pWeight;
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_NIHGT_GRIDWEIGHTS_TAG_ID)){ //cxf
      uint8_t *pNightWeight  = NULL;
      int nightArraySize     = tag.Size();
      pNightWeight = (uint8_t *)malloc(nightArraySize * sizeof(uint8_t));
	  if(pNightWeight == NULL){
		LOGE( "%s(%d): aec gridWeight malloc fail!\n", __FUNCTION__, __LINE__);
	  }
      MEMSET( pNightWeight, 0, (nightArraySize * sizeof( uint8_t )) );
      int no = ParseUcharArray( tag.Value(), pNightWeight, nightArraySize );
      DCT_ASSERT( (no == nightArraySize) );

      aec_data.NightGridWeights.ArraySize = nightArraySize;
      aec_data.NightGridWeights.pWeight = pNightWeight;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_CAMERICISPHISTMODE_TAG_ID)){ //cxf
      char* value = Toupper(tag.Value());
      char* value2 = Toupper(tag.Value());
      char* value3 = value2;
      int i = 0;
      while (*value != '\0') {
        if (*value == 'R') {
          *value2++ = 'R';
          i++;
        } else if (*value == 'G') {
          *value2++ = 'G';
          i++;
        } else if (*value == 'B') {
          *value2++ = 'B';
          i++;
        } else if (*value == 'Y') {
          *value2++ = 'Y';
          i++;
        }
        value++;
      }

      *value2 = '\0';
      std::string s_value(value3);
#ifdef DEBUG_LOG
	  LOGD( "%s(%d): value: %s\n", __FUNCTION__,__LINE__,value);
	  LOGD( "%s(%d): s_value: %s\n", __FUNCTION__,__LINE__,s_value.c_str());
#endif
      if (s_value == CALIB_SENSOR_AEC_CAMERICISPHISTMODE_R) {
        aec_data.CamerIcIspHistMode =  CAM_HIST_MODE_R;
      } else if (s_value == CALIB_SENSOR_AEC_CAMERICISPHISTMODE_G) {
        aec_data.CamerIcIspHistMode =  CAM_HIST_MODE_G;
      } else if (s_value == CALIB_SENSOR_AEC_CAMERICISPHISTMODE_B) {
        aec_data.CamerIcIspHistMode =  CAM_HIST_MODE_B;
      } else if (s_value == CALIB_SENSOR_AEC_CAMERICISPHISTMODE_RGB) {
        aec_data.CamerIcIspHistMode =  CAM_HIST_MODE_RGB_COMBINED;
      } else if (s_value == CALIB_SENSOR_AEC_CAMERICISPHISTMODE_Y) {
        aec_data.CamerIcIspHistMode =  CAM_HIST_MODE_Y;
      } else {
        aec_data.CamerIcIspHistMode = CAM_HIST_MODE_INVALID;
		LOGE( "%s(%d): invalid AEC CamerIcIspHistMode: %s\n", __FUNCTION__,__LINE__,s_value.c_str());
      }
    }
    else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_CAMERICISPEXPMEASURINGMODE_TAG_ID)){
      char* value = Toupper(tag.Value());
      char* value2 = Toupper(tag.Value());
      char* value3 = value2;
      int i = 0;
      while (*value != '\0') {
        if (*value == 'R') {
          *value2++ = 'R';
          i++;
        } else if (*value == 'G') {
          *value2++ = 'G';
          i++;
        } else if (*value == 'B') {
          *value2++ = 'B';
          i++;
        } else if (*value == 'Y') {
          *value2++ = 'Y';
          i++;
        }
        value++;
      }

      *value2 = '\0';
      std::string s_value(value3);
#ifdef DEBUG_LOG
	  LOGD( "%s(%d): value: %s\n", __FUNCTION__,__LINE__,value);
	  LOGD( "%s(%d): s_value: %s\n", __FUNCTION__,__LINE__,s_value.c_str());
#endif
      if (s_value == CALIB_SENSOR_AEC_CAMERICISPEXPMEASURINGMODE_1) {
        aec_data.CamerIcIspExpMeasuringMode = CAM_EXP_MEASURING_MODE_1;
      } else if (s_value == CALIB_SENSOR_AEC_CAMERICISPEXPMEASURINGMODE_2) {
        aec_data.CamerIcIspExpMeasuringMode = CAM_EXP_MEASURING_MODE_2;
      } else {
        aec_data.CamerIcIspExpMeasuringMode = CAM_EXP_MEASURING_MODE_INVALID;
		LOGE( "%s(%d): invalid CamerIcIspExpMeasuringMode: %s\n", __FUNCTION__,__LINE__,s_value.c_str());
      }
    }
    else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_CLM_TOLERANCE_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &aec_data.ClmTolerance, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_DAMP_OVER_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &aec_data.DampOverStill, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_DAMP_UNDER_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &aec_data.DampUnderStill, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_DAMP_OVER_VIDEO_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &aec_data.DampOverVideo, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_DAMP_UNDER_VIDEO_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &aec_data.DampUnderVideo, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_AFPS_MAX_GAIN_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &aec_data.AfpsMaxGain, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_FPS_FPS_SET_CONFIG_TAG_ID)){
	  if (!parseEntryAecFPSSetConfig(pchild->ToElement(), &aec_data)){
		LOGE( "%s(%d): parse error in Aec FPSSetConfig section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
	} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_ECM_TAG_ID)){
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryAecEcm, NULL,
	  		(uint32_t)(CALIB_SENSOR_AEC_ECM_TAG_ID), (uint32_t)(CALIB_SENSOR_AEC_TAG_ID))) {
#if 1
		LOGE( "%s(%d):parse error in AEC_ECM_TAG section (unknow tag: %s)\n",
			__FUNCTION__,__LINE__,tagname.c_str());

#endif

        return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_AOE_ENABLE_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &aec_data.AOE_Enable, 1);
      DCT_ASSERT((no == tag.Size()));

    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_AOE_MAX_POINT_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &aec_data.AOE_Max_point, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_AOE_MIN_POINT_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &aec_data.AOE_Min_point, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_AOE_Y_MAX_TH_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &aec_data.AOE_Y_Max_th, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_AOE_Y_MIN_TH_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &aec_data.AOE_Y_Min_th, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_AOE_STEP_INC_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &aec_data.AOE_Step_Inc, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_AOE_STEP_DEC_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &aec_data.AOE_Step_Dec, 1);
      DCT_ASSERT((no == tag.Size()));
    }/*zlj add for LockAE*/
	else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LOCK_AE_TAG_ID)){
	  if (!parseEntryAecLockAE(pchild->ToElement(), &aec_data)){
		LOGE( "%s(%d): parse error in Aec LockAE section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
	}/*zlj add for HDR-Ctrl*/
	else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_HDRCTRL_TAG_ID)){
	  if (!parseEntryAecHdrCtrl(pchild->ToElement(), &aec_data)){
		LOGE( "%s(%d): parse error in Aec DON section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
	}
	else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_DON_TAG_ID)){
      if (!parseEntryAecDON(pchild->ToElement(), &aec_data)){
		LOGE( "%s(%d): parse error in Aec DON section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
    }
	else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_INTERVAL_ADJUST_STRATEGY_TAG_ID)){
      if (!parseEntryAecIntervalAdjustStrategy(pchild->ToElement(), &aec_data)){
		LOGE( "%s(%d): parse error in Aec IntervalAdjustStrategy section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
	}
	else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_DYNAMIC_SETPOINT_TAG_ID))
	{
		if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseAECDySetpoint, &aec_data,
				(uint32_t)(CALIB_SENSOR_AEC_DYNAMIC_SETPOINT_TAG_ID), (uint32_t)(CALIB_SENSOR_AEC_TAG_ID))) {
			LOGE( "%s(%d): parse error in AEC_DYNAMIC_SETPOINT section (unknow tag:%s)\n", 
				__FUNCTION__,__LINE__,tagname.c_str());
			return (false);
	  	}
	}
	else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_EXP_SEPARATE_TAG_ID))
	{
		if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseAECExpSeparate, &aec_data,
				(uint32_t)(CALIB_SENSOR_AEC_EXP_SEPARATE_TAG_ID), (uint32_t)(CALIB_SENSOR_AEC_TAG_ID))) {
			LOGE( "%s(%d): parse error in AEC_EXP_SEPARATE section (unknow tag:%s)\n", 
				__FUNCTION__,__LINE__,tagname.c_str());
			return (false);
	  	}
	}else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_NLSC_CONFIG_TAG_ID)){
	  if (!parseEntryAecNLSC(pchild->ToElement(), &aec_data)){
		LOGE( "%s(%d): parse error in Aec NLSC section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
	}
	else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_BACKLIGHT_CONFIG_TAG_ID)){
      if (!parseEntryAecBacklight(pchild->ToElement(), &aec_data)){
		LOGE( "%s(%d): parse error in Aec Backlight section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
	}
	else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_HIST_2_HAL_TAG_ID)){
	  if (!parseEntryAecHist2Hal(pchild->ToElement(), &aec_data)){
		LOGE( "%s(%d): parse error in aec hist2Hal section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
	}
	else {
#if 1
	  LOGE( "%s(%d): parse error in AEC section (unknow tag:%s)\n", 
	  	__FUNCTION__,__LINE__,tagname.c_str());
#endif

      return ( false );
    }
	
    pchild = pchild->NextSibling();
  }
  XML_CHECK_END();
  
  RESULT result = CamCalibDbAddAecGlobal(m_CalibDbHandle, &aec_data);
  ClearDySetpointList(&aec_data.DySetpointList);
  ClearExpSeparateList(&aec_data.ExpSeparateList);

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);    
#endif
  DCT_ASSERT(result == RET_SUCCESS);


  return (true);
}



/******************************************************************************
 * CalibDb::parseEntryAecEcm
 *****************************************************************************/
bool CalibDb::parseEntryAecEcm
(
    const XMLElement* plement,
    void* param
) {
  (void)param;

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);    
#endif

  CamEcmProfile_t EcmProfile;
  MEMSET(&EcmProfile, 0, sizeof(EcmProfile));
  ListInit(&EcmProfile.ecm_scheme);
  
  XML_CHECK_START(CALIB_SENSOR_AEC_ECM_TAG_ID, CALIB_SENSOR_AEC_TAG_ID);
  
  const XMLNode* pchild = plement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_ECM_NAME_TAG_ID)){
      char* value = Toupper(tag.Value());
      strncpy(EcmProfile.name, value, sizeof(EcmProfile.name));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_ECM_SCHEMES_TAG_ID)){
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryAecEcmPriorityScheme, &EcmProfile,
	  		(uint32_t)(CALIB_SENSOR_AEC_ECM_SCHEMES_TAG_ID), (uint32_t)(CALIB_SENSOR_AEC_ECM_TAG_ID))) {
#if 1
        LOGE( "%s(%d): parse error in ECM  section (%s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
        return (false);
      }
    } else {
#if 1
      LOGE( "%s(%d): parse error in ECM section (unknow tag:%s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif

      return (false);
    }
	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
  RESULT result = CamCalibDbAddEcmProfile(m_CalibDbHandle, &EcmProfile);
  DCT_ASSERT(result == RET_SUCCESS);

  // free linked ecm_schemes
  List* l = ListRemoveHead(&EcmProfile.ecm_scheme);
  while (l) {
    List* temp = ListRemoveHead(l);
    free(l);
    l = temp;
  }

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}



/******************************************************************************
 * CalibDb::parseEntryAecEcmPriorityScheme
 *****************************************************************************/
bool CalibDb::parseEntryAecEcmPriorityScheme
(
    const XMLElement* pelement,
    void* param
) {
  CamEcmProfile_t* pEcmProfile = (CamEcmProfile_t*)param;

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamEcmScheme_t* pEcmScheme = (CamEcmScheme_t*) malloc(sizeof(CamEcmScheme_t));
  if (!pEcmScheme) {
    return false;
  }
  MEMSET(pEcmScheme, 0, sizeof(*pEcmScheme));

  XML_CHECK_START(CALIB_SENSOR_AEC_ECM_SCHEMES_TAG_ID, CALIB_SENSOR_AEC_ECM_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_ECM_SCHEME_NAME_TAG_ID)){
      char* value = Toupper(tag.Value());
      strncpy(pEcmScheme->name, value, sizeof(pEcmScheme->name));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_ECM_SCHEME_OFFSETT0FAC_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &pEcmScheme->OffsetT0Fac, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_ECM_SCHEME_SLOPEA0_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &pEcmScheme->SlopeA0, 1);
      DCT_ASSERT((no == tag.Size()));
    } else {
#if 1
      LOGE( "%s(%d): parse error in ECM section (%s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif

      free(pEcmScheme);
      pEcmScheme = NULL;

      return ( false );
    }
	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
  if (pEcmScheme) {
    ListPrepareItem(pEcmScheme);
    ListAddTail(&pEcmProfile->ecm_scheme, pEcmScheme);
  }

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}

bool CalibDb::parseAECDySetpoint
(
    const XMLElement* plement,
    void* param
)
{
#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif
  CamCalibAecGlobal_t *pAec_profile = (CamCalibAecGlobal_t *)param;
  if(NULL == pAec_profile){
	LOGD( "%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
    return false;
  }

  CamCalibAecDynamicSetpoint_t* pDySetpointFile = (CamCalibAecDynamicSetpoint_t*)malloc(sizeof(CamCalibAecDynamicSetpoint_t));
  if (NULL == pDySetpointFile) {
	LOGD( "%s(%d): malloc fail (exit)\n", __FUNCTION__,__LINE__);
    return false;
  }
  MEMSET(pDySetpointFile, 0, sizeof(*pDySetpointFile));

  XML_CHECK_START(CALIB_SENSOR_AEC_DYNAMIC_SETPOINT_TAG_ID, CALIB_SENSOR_AEC_TAG_ID);
  
  int nExpValue = 0;
  int nDysetpoint = 0;

  const XMLNode* pchild = plement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

	if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_DYNAMIC_SETPOINT_NAME_TAG_ID))
	{
	  char* value = Toupper(tag.Value());
      strncpy(pDySetpointFile->name, value, sizeof(pDySetpointFile->name));
#ifdef DEBUG_LOG
	  LOGD( "%s(%d): value:%s\n", __FUNCTION__, __LINE__, value);
	  LOGD( "%s(%d): %s\n", __FUNCTION__, __LINE__, pDySetpointFile->name );
#endif
	}else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_DYNAMIC_SETPOINT_ENABLE_TAG_ID))
	{
		int no = ParseUcharArray(tag.Value(), &pDySetpointFile->enable, 1);
		DCT_ASSERT((no == 1));
	}else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_DYNAMIC_SETPOINT_EXPVALUE_TAG_ID))
	{
		 pDySetpointFile->pExpValue = (float*)malloc((tag.Size() * sizeof(float)));
	  if(!pDySetpointFile->pExpValue){
		  LOGE( "%s(%d): malloc fail\n", __FUNCTION__,__LINE__);
  	  }else{
		  int no = ParseFloatArray(tag.Value(), pDySetpointFile->pExpValue, tag.Size());
	      DCT_ASSERT((no == tag.Size()));
		  nExpValue = no;
  	  }
	}else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_DYNAMIC_SETPOINT_DYSETPOINT_TAG_ID))
	{
		 pDySetpointFile->pDySetpoint = (float*)malloc((tag.Size() * sizeof(float)));
	  if(!pDySetpointFile->pDySetpoint){
	      LOGE( "%s(%d): malloc fail\n", __FUNCTION__,__LINE__);
  	  }else{
		  int no = ParseFloatArray(tag.Value(), pDySetpointFile->pDySetpoint, tag.Size());
	      DCT_ASSERT((no == tag.Size()));
		  nDysetpoint = no;
  	  }
	}else{
		LOGE( "%s(%d): parse error inDynamic Setpoint (%s)\n", __FUNCTION__,__LINE__,tagname.c_str());
	}
	
	pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
  DCT_ASSERT((nDysetpoint == nExpValue));
  pDySetpointFile->array_size = nDysetpoint;

  if (pDySetpointFile) {
	 ListPrepareItem(pDySetpointFile);
	 ListAddTail(&pAec_profile->DySetpointList, pDySetpointFile);
  }

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
 }


bool CalibDb::parseAECExpSeparate
(
    const XMLElement* plement,
    void* param
)
{
#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif
  CamCalibAecGlobal_t *pAec_profile = (CamCalibAecGlobal_t *)param;
  if(NULL == pAec_profile){
	LOGE( "%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
    return false;
  }

  CamCalibAecExpSeparate_t* pExpSeparate = (CamCalibAecExpSeparate_t*)malloc(sizeof(CamCalibAecExpSeparate_t));
  if (NULL == pExpSeparate) {
  	LOGE( "%s(%d): malloc fail (exit)\n", __FUNCTION__,__LINE__);
    return false;
  }
  MEMSET(pExpSeparate, 0, sizeof(*pExpSeparate));

  int nTimeDot = 0;
  int nGainDot = 0;
  int nLTimeDot = 0;
  int nLGainDot = 0;
  int nSTimeDot = 0;
  int nSGainDot = 0;

  XML_CHECK_START(CALIB_SENSOR_AEC_EXP_SEPARATE_TAG_ID, CALIB_SENSOR_AEC_TAG_ID);
  
  const XMLNode* pchild = plement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

	if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_EXP_SEPARATE_NAME_TAG_ID))
	{
	  char* value = Toupper(tag.Value());
      strncpy(pExpSeparate->name, value, sizeof(pExpSeparate->name));
#ifdef DEBUG_LOG
	  LOGD( "%s(%d): value:%s\n", __FUNCTION__, __LINE__, value);
	  LOGD( "%s(%d): %s\n", __FUNCTION__, __LINE__, pExpSeparate->name );
#endif
	}else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_EXP_SEPARATE_TIMEDOT_TAG_ID))
	{
	  int no = ParseFloatArray(tag.Value(), pExpSeparate->ecmTimeDot.fCoeff, tag.Size());
      DCT_ASSERT((no == tag.Size()));
	  nTimeDot = no;
	}else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_EXP_SEPARATE_GAINDOT_TAG_ID))
	{
	  int no = ParseFloatArray(tag.Value(), pExpSeparate->ecmGainDot.fCoeff, tag.Size());
      DCT_ASSERT((no == tag.Size()));
	  nGainDot = no;
	}else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_EXP_SEPARATE_LTIMEDOT_TAG_ID))
	{
	  int no = ParseFloatArray(tag.Value(), pExpSeparate->ecmLTimeDot.fCoeff, tag.Size());
      DCT_ASSERT((no == tag.Size()));
	  nLTimeDot = no;
	}else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_EXP_SEPARATE_LGAINDOT_TAG_ID))
	{
	  int no = ParseFloatArray(tag.Value(), pExpSeparate->ecmLGainDot.fCoeff, tag.Size());
      DCT_ASSERT((no == tag.Size()));
	  nLGainDot = no;
	}else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_EXP_SEPARATE_STIMEDOT_TAG_ID))
	{
	  int no = ParseFloatArray(tag.Value(), pExpSeparate->ecmSTimeDot.fCoeff, tag.Size());
      DCT_ASSERT((no == tag.Size()));
	  nSTimeDot = no;
	}else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AEC_EXP_SEPARATE_SGAINDOT_TAG_ID))
	{
	  int no = ParseFloatArray(tag.Value(), pExpSeparate->ecmSGainDot.fCoeff, tag.Size());
      DCT_ASSERT((no == tag.Size()));
	  nSGainDot = no;
	}else{
	  LOGE( "%s(%d): parse error AEC_EXP_SEPARATE Setpoint (%s)\n", __FUNCTION__,__LINE__,tagname.c_str());
	  return false;
	}
	
	pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
  DCT_ASSERT((nGainDot == nTimeDot));
  DCT_ASSERT((nGainDot == nLTimeDot));
  DCT_ASSERT((nGainDot == nSTimeDot));
  DCT_ASSERT((nGainDot == nLGainDot));
  DCT_ASSERT((nGainDot == nSGainDot));
  DCT_ASSERT((nGainDot == 6));

  if (pExpSeparate) {
	 ListPrepareItem(pExpSeparate);
	 ListAddTail(&pAec_profile->ExpSeparateList, pExpSeparate);
  }

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
 }


/******************************************************************************
 * CalibDb::parseEntryAwb
 *****************************************************************************/
bool CalibDb::parseEntryAwb
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);    
#endif
  CAM_AwbVersion_t vName;

  XML_CHECK_START(CALIB_SENSOR_AWB_TAG_ID, CALIB_SENSOR_TAG_ID);

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif
	  if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_VALID_VERSION_NAME_TAG_ID)){
	  	char* value = Toupper(tag.Value());
		std::string s_value(value);
#ifdef DEBUG_LOG
		redirectOut << "value:" << value << std::endl;
		redirectOut << s_value << std::endl;
#endif
		if (s_value == CALIB_SENDOR_AWB_VERSION_11_NAME_TAG) {
			vName =  CAM_AWB_VERSION_11;
		} else if (s_value == CALIB_SENDOR_AWB_VERSION_10_NAME_TAG) {
			vName =  CAM_AWB_VERSION_10;
		} else {
			LOGE("%s(%d): invalid awb version name:%s\n", __FUNCTION__, __LINE__, s_value.c_str());
			return (false);
		}
		RESULT result = CamCalibDbAddAwb_VersionName(m_CalibDbHandle, vName);
		DCT_ASSERT(result == RET_SUCCESS);
	} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_VERSION_11_TAG_ID)){
      if (!parseEntryAwb_V11_Para(pchild->ToElement())) {
#if 1	  	
	  	LOGE("%s(%d): parse error in AWB version_11 para:%s\n", __FUNCTION__, __LINE__, tagname.c_str());
#endif
        return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_VERSION_10_TAG_ID)){
      if(!parseEntryAwb_V10_Para(pchild->ToElement())) {
#if 1
    	LOGE("%s(%d): parse error in AWB version_10 para:%s\n", __FUNCTION__, __LINE__, tagname.c_str());
#endif
        return (false);
      }
    } else {
#if 1
      LOGE( "%s(%d): parse error in AWB section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
      return ( false );
    }
	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
	
#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}



/******************************************************************************
 * CalibDb::parseEntryAwb_V10_Para
 *****************************************************************************/
bool CalibDb::parseEntryAwb_V10_IIR
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

    CamCalibAwb_V10_Global_t *pAwbData= (CamCalibAwb_V10_Global_t *)param;
    if(NULL == pAwbData){
		LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
  		return false;
    }

    XML_CHECK_START(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_ID, CALIB_SENSOR_AWB_V10_GLOBALS_TAG_ID);
  
	const XMLNode* pchild = pelement->FirstChild();
  	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_DAMP_COEF_ADD_TAG_ID)){
		  int no = ParseFloatArray(tag.Value(), &pAwbData->IIR.fIIRDampCoefAdd, 1);
		  DCT_ASSERT((no == tag.Size()));
		} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_DAMP_COEF_SUB_TAG_ID)){
		  int no = ParseFloatArray(tag.Value(), &pAwbData->IIR.fIIRDampCoefSub, 1);
		  DCT_ASSERT((no == tag.Size()));
		} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_DAMP_FILTER_THRESHOLD_TAG_ID)){
		  int no = ParseFloatArray(tag.Value(), &pAwbData->IIR.fIIRDampFilterThreshold, 1);
		  DCT_ASSERT((no == tag.Size()));
		} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_DAMPING_COEF_MIN_TAG_ID)){
		  int no = ParseFloatArray(tag.Value(), &pAwbData->IIR.fIIRDampingCoefMin, 1);
		  DCT_ASSERT((no == tag.Size()));
		} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_DAMPING_COEF_MAX_TAG_ID)){
		  int no = ParseFloatArray(tag.Value(), &pAwbData->IIR.fIIRDampingCoefMax, 1);
		  DCT_ASSERT((no == tag.Size()));
		} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_DAMPING_COEF_INIT_TAG_ID)){
		  int no = ParseFloatArray(tag.Value(), &pAwbData->IIR.fIIRDampingCoefInit, 1);
		  DCT_ASSERT((no == tag.Size()));
		} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_EXP_PRIOR_FILTER_SIZE_MAX_TAG_ID)){
		  int no = ParseUshortArray(tag.Value(), &pAwbData->IIR.IIRFilterSize, 1);
		  DCT_ASSERT((no == tag.Size()));
		}  else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_EXP_PRIOR_FILTER_SIZE_MIN_TAG_ID)){
		  
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_EXP_PRIOR_MIDDLE_TAG_ID)){
		  int no = ParseFloatArray(tag.Value(), &pAwbData->IIR.fIIRFilterInitValue, 1);
		  DCT_ASSERT((no == tag.Size()));
		} else {
#if 1
		  LOGE( "%s(%d): parse error in AWB GLOBALS - IIR section (unknow tag: %s)\n", 
		   		__FUNCTION__,__LINE__,tagname.c_str());
#endif
		  return (false);
		}		
    	pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;
}




/******************************************************************************
 * CalibDb::parseEntryAwb_V10_Para
 *****************************************************************************/
bool CalibDb::parseEntryAwb_V11_IIR
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

    CamCalibAwb_V11_Global_t *pAwbData= (CamCalibAwb_V11_Global_t *)param;
    if(NULL == pAwbData){
		LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
  		return false;
    }

    XML_CHECK_START(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_ID, CALIB_SENSOR_AWB_V11_GLOBALS_TAG_ID);
  
	const XMLNode* pchild = pelement->FirstChild();
  	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_DAMP_COEF_ADD_TAG_ID)){
		  int no = ParseFloatArray(tag.Value(), &pAwbData->IIR.fIIRDampCoefAdd, 1);
		  DCT_ASSERT((no == tag.Size()));
		} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_DAMP_COEF_SUB_TAG_ID)){
		  int no = ParseFloatArray(tag.Value(), &pAwbData->IIR.fIIRDampCoefSub, 1);
		  DCT_ASSERT((no == tag.Size()));
		} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_DAMP_FILTER_THRESHOLD_TAG_ID)){
		  int no = ParseFloatArray(tag.Value(), &pAwbData->IIR.fIIRDampFilterThreshold, 1);
		  DCT_ASSERT((no == tag.Size()));
		} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_DAMPING_COEF_MIN_TAG_ID)){
		  int no = ParseFloatArray(tag.Value(), &pAwbData->IIR.fIIRDampingCoefMin, 1);
		  DCT_ASSERT((no == tag.Size()));
		} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_DAMPING_COEF_MAX_TAG_ID)){
		  int no = ParseFloatArray(tag.Value(), &pAwbData->IIR.fIIRDampingCoefMax, 1);
		  DCT_ASSERT((no == tag.Size()));
		} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_DAMPING_COEF_INIT_TAG_ID)){
		  int no = ParseFloatArray(tag.Value(), &pAwbData->IIR.fIIRDampingCoefInit, 1);
		  DCT_ASSERT((no == tag.Size()));
		} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_EXP_PRIOR_FILTER_SIZE_MAX_TAG_ID)){
		  int no = ParseUshortArray(tag.Value(), &pAwbData->IIR.IIRFilterSize, 1);
		  DCT_ASSERT((no == tag.Size()));
		} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_EXP_PRIOR_FILTER_SIZE_MIN_TAG_ID)){
		  
		} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_EXP_PRIOR_MIDDLE_TAG_ID)){
		  int no = ParseFloatArray(tag.Value(), &pAwbData->IIR.fIIRFilterInitValue, 1);
		  DCT_ASSERT((no == tag.Size()));
		} else {
#if 1
		  LOGE( "%s(%d): parse error in AWB GLOBALS - IIR section (unknow tag: %s)\n", 
		   		__FUNCTION__,__LINE__,tagname.c_str());
#endif
		  return (false);
		}		
    	pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;
}

/******************************************************************************
 * CalibDb::parseEntryAwb_V10_Para
 *****************************************************************************/
bool CalibDb::parseEntryAwb_V10_Para
(
    const XMLElement*   pelement,
    void*                param
) {

  (void)param;

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif
  CAM_AwbVersion_t vName;
  XML_CHECK_START(CALIB_SENSOR_AWB_VERSION_10_TAG_ID, CALIB_SENSOR_AWB_TAG_ID);
	
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

	if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_TAG_ID)){
		if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryAwb_V10_Globals, NULL,
				(uint32_t)(CALIB_SENSOR_AWB_V10_GLOBALS_TAG_ID), (uint32_t)(CALIB_SENSOR_AWB_VERSION_10_TAG_ID))) {
#if 1
          LOGE( "%s(%d): parse error in AWB version_10 globals (%s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
		  return (false);
		}
	} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_TAG_ID)){
		if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryAwb_V10_Illumination, NULL,
				(uint32_t)(CALIB_SENSOR_AWB_V10_ILLUMINATION_TAG_ID), (uint32_t)(CALIB_SENSOR_AWB_VERSION_10_TAG_ID))) {
#if 1
          LOGE( "%s(%d): parse error in AWB version_10 Illumination (%s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif

		  return (false);
		}
	}else{
#if 1
        LOGE( "%s(%d): parse error in AWB section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());         
#endif
		return (false);
	}	
	pchild = pchild->NextSibling();

  }

  XML_CHECK_END();
#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif
	return (true);

}
/******************************************************************************
 * CalibDb::parseEntryAwb_V11_Para
 *****************************************************************************/
bool CalibDb::parseEntryAwb_V11_Para
(
    const XMLElement*   pelement,
    void*                param
) {

  (void)param;

#ifdef DEBUG_LOG
  	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif
  CAM_AwbVersion_t vName;

  XML_CHECK_START(CALIB_SENSOR_AWB_VERSION_11_TAG_ID, CALIB_SENSOR_AWB_TAG_ID);

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

	if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V11_GLOBALS_TAG_ID)){
		if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryAwb_V11_Globals, NULL,
				(uint32_t)(CALIB_SENSOR_AWB_V11_GLOBALS_TAG_ID), (uint32_t)(CALIB_SENSOR_AWB_VERSION_11_TAG_ID))) {
#if 1
            LOGE( "%s(%d): parse error in AWB version_11 globals (%s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
			return (false);
		 }
	} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V11_ILLUMINATION_TAG_ID)){
		if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryAwb_V11_Illumination, NULL,
				(uint32_t)(CALIB_SENSOR_AWB_V11_ILLUMINATION_TAG_ID), (uint32_t)(CALIB_SENSOR_AWB_VERSION_11_TAG_ID))) {
#if 1
          	LOGE( "%s(%d): parse error in AWB version_11 Illumination (%s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
		  	return (false);
		}
	}else{
#if 1
        	LOGE( "%s(%d): parse error in AWB section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());         
#endif
			return (false);
	}	
	pchild = pchild->NextSibling();

  }

  XML_CHECK_END();
  
#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  return (true);

}

/******************************************************************************
 * CalibDb::parseEntryAwb_V11_Globals
 *****************************************************************************/
bool CalibDb::parseEntryAwb_V11_Globals
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamCalibAwb_V11_Global_t awb_data;

  /* CamAwbClipParm_t */
  float* pRg1         = NULL;
  int nRg1        = 0;
  float* pMaxDist1    = NULL;
  int nMaxDist1   = 0;
  float* pRg2         = NULL;
  int nRg2        = 0;
  float* pMaxDist2    = NULL;
  int nMaxDist2   = 0;

  /* CamAwbGlobalFadeParm_t */
  float* pGlobalFade1         = NULL;
  int nGlobalFade1         = 0;
  float* pGlobalGainDistance1 = NULL;
  int nGlobalGainDistance1 = 0;
  float* pGlobalFade2         = NULL;
  int nGlobalFade2         = 0;
  float* pGlobalGainDistance2 = NULL;
  int nGlobalGainDistance2 = 0;

  /* CamAwbFade2Parm_t */
  float* pFade                = NULL;
  int nFade                = 0;
  float* pMaxCSum_br          = NULL;
  int nMaxCSum_br          = 0;
  float* pMaxCSum_sr          = NULL;
  int nMaxCSum_sr          = 0;
  float* pMinC_br             = NULL;
  int nMinC_br             = 0;
  float* pMaxY_br             = NULL;
  int nMaxY_br             = 0;
  float* pMinY_br             = NULL;
  int nMinY_br             = 0;
  float* pMinC_sr             = NULL;
  int nMinC_sr             = 0;
  float* pMaxY_sr             = NULL;
  int nMaxY_sr             = 0;
  float* pMinY_sr             = NULL;
  int nMinY_sr             = 0;

  float* pRefCr = NULL;
  int nRefCr = 0;
  float* pRefCb = NULL;
  int nRefCb = 0;

  XML_CHECK_START(CALIB_SENSOR_AWB_V11_GLOBALS_TAG_ID, CALIB_SENSOR_AWB_VERSION_11_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    const char* value = tag.Value();
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_NAME_TAG_ID)){
      strncpy(awb_data.name, value, sizeof(awb_data.name));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RESOLUTION_TAG_ID)){
      strncpy(awb_data.resolution, value, sizeof(awb_data.resolution));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V11_GLOBALS_GAIN_CLIP_ENABLE_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &awb_data.AwbClipEnable, 1);

	  DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_CENTERLINE_TAG_ID)){
      int i = (sizeof(awb_data.CenterLine) / sizeof(awb_data.CenterLine.f_N0_Rg));
      int no = ParseFloatArray(tag.Value(), &awb_data.CenterLine.f_N0_Rg, i);

	  DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_KFACTOR_TAG_ID)){
      int i = (sizeof(awb_data.KFactor) / sizeof(awb_data.KFactor.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), awb_data.KFactor.fCoeff, i);

	  DCT_ASSERT((no == tag.Size()));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RG1_TAG_ID))
               && (NULL == pRg1)) {
      nRg1 = tag.Size();
      pRg1 = (float*)malloc(sizeof(float) * nRg1);

      int no = ParseFloatArray(tag.Value(), pRg1, nRg1);
      DCT_ASSERT((no == nRg1));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_MAXDIST1_TAG_ID))
               && (NULL == pMaxDist1)) {
      nMaxDist1 = tag.Size();
      pMaxDist1 = (float*)malloc(sizeof(float) * nMaxDist1);

      int no = ParseFloatArray(tag.Value(), pMaxDist1, nMaxDist1);
      DCT_ASSERT((no == nRg1));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RG2_TAG_ID))
               && (NULL == pRg2)) {
      nRg2 = tag.Size();
      pRg2 = (float*)malloc(sizeof(float) * nRg2);

      int no = ParseFloatArray(tag.Value(), pRg2, nRg2);
      DCT_ASSERT((no == nRg2));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_MAXDIST2_TAG_ID))
               && (NULL == pMaxDist2)) {
      nMaxDist2 = tag.Size();
      pMaxDist2 = (float*)malloc(sizeof(float) * nMaxDist2);

      int no = ParseFloatArray(tag.Value(), pMaxDist2, nMaxDist2);
      DCT_ASSERT((no == nMaxDist2));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_GLOBALFADE1_TAG_ID))
               && (NULL == pGlobalFade1)) {
      nGlobalFade1 = tag.Size();
      pGlobalFade1 = (float*)malloc(sizeof(float) * nGlobalFade1);

      int no = ParseFloatArray(tag.Value(), pGlobalFade1, nGlobalFade1);
      DCT_ASSERT((no == nGlobalFade1));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_GLOBALGAINDIST1_TAG_ID))
               && (NULL == pGlobalGainDistance1)) {
      nGlobalGainDistance1 = tag.Size();
      pGlobalGainDistance1 = (float*)malloc(sizeof(float) * nGlobalGainDistance1);

      int no = ParseFloatArray(tag.Value(), pGlobalGainDistance1, nGlobalGainDistance1);
      DCT_ASSERT((no == nGlobalGainDistance1));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_GLOBALFADE2_TAG_ID))
               && (NULL == pGlobalFade2)) {
      nGlobalFade2 = tag.Size();
      pGlobalFade2 = (float*)malloc(sizeof(float) * nGlobalFade2);

      int no = ParseFloatArray(tag.Value(), pGlobalFade2, nGlobalFade2);
      DCT_ASSERT((no == nGlobalFade2));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_GLOBALGAINDIST2_TAG_ID))
               && (NULL == pGlobalGainDistance2)) {
      nGlobalGainDistance2 = tag.Size();
      pGlobalGainDistance2 = (float*)malloc(sizeof(float) * nGlobalGainDistance2);

      int no = ParseFloatArray(tag.Value(), pGlobalGainDistance2, nGlobalGainDistance2);
      DCT_ASSERT((no == nGlobalGainDistance2));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_FADE2_TAG_ID))
               && (NULL == pFade)) {
      nFade = tag.Size();
      pFade = (float*)malloc(sizeof(float) * nFade);

      int no = ParseFloatArray(tag.Value(), pFade, nFade);
      DCT_ASSERT((no == nFade));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V11_GLOBALS_MAX_CSUM_BR_TAG_ID))
               && (NULL == pMaxCSum_br)) {
      nMaxCSum_br = tag.Size();
      pMaxCSum_br = (float*)malloc(sizeof(float) * nMaxCSum_br);

      int no = ParseFloatArray(tag.Value(), pMaxCSum_br, nMaxCSum_br);
      DCT_ASSERT((no == nMaxCSum_br));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V11_GLOBALS_MAX_CSUM_SR_TAG_ID))
               && (NULL == pMaxCSum_sr)) {
      nMaxCSum_sr = tag.Size();
      pMaxCSum_sr = (float*)malloc(sizeof(float) * nMaxCSum_sr);

      int no = ParseFloatArray(tag.Value(), pMaxCSum_sr, nMaxCSum_sr);
      DCT_ASSERT((no == nMaxCSum_sr));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V11_GLOBALS_MINC_BR_TAG_ID))
               && (NULL == pMinC_br)) {
      nMinC_br = tag.Size();
      pMinC_br = (float*)malloc(sizeof(float) * nMinC_br);

      int no = ParseFloatArray(tag.Value(), pMinC_br, nMinC_br);
      DCT_ASSERT((no == nMinC_br));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V11_GLOBALS_MAXY_BR_TAG_ID))
               && (NULL == pMaxY_br)) {
      nMaxY_br = tag.Size();
      pMaxY_br = (float*)malloc(sizeof(float) * nMaxY_br);

      int no = ParseFloatArray(tag.Value(), pMaxY_br, nMaxY_br);
      DCT_ASSERT((no == nMaxY_br));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V11_GLOBALS_MINY_BR_TAG_ID))
               && (NULL == pMinY_br)) {
      nMinY_br = tag.Size();
      pMinY_br = (float*)malloc(sizeof(float) * nMinY_br);

      int no = ParseFloatArray(tag.Value(), pMinY_br, nMinY_br);
      DCT_ASSERT((no == nMinY_br));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V11_GLOBALS_MINC_SR_TAG_ID))
               && (NULL == pMinC_sr)) {
      nMinC_sr = tag.Size();
      pMinC_sr = (float*)malloc(sizeof(float) * nMinC_sr);

      int no = ParseFloatArray(tag.Value(), pMinC_sr, nMinC_sr);
      DCT_ASSERT((no == nMinC_sr));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V11_GLOBALS_MAXY_SR_TAG_ID))
               && (NULL == pMaxY_sr)) {
      nMaxY_sr = tag.Size();
      pMaxY_sr = (float*)malloc(sizeof(float) * nMaxY_sr);

      int no = ParseFloatArray(tag.Value(), pMaxY_sr, nMaxY_sr);
      DCT_ASSERT((no == nMaxY_sr));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V11_GLOBALS_MINY_SR_TAG_ID))
               && (NULL == pMinY_sr)) {
      nMinY_sr = tag.Size();
      pMinY_sr = (float*)malloc(sizeof(float) * nMinY_sr);

      int no = ParseFloatArray(tag.Value(), pMinY_sr, nMinY_sr);
      DCT_ASSERT((no == nMinY_sr));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_REFCB_TAG_ID))
               && (NULL == pRefCb)) {
      nRefCb = tag.Size();
      pRefCb = (float*)malloc(sizeof(float) * nRefCb);

      int no = ParseFloatArray(tag.Value(), pRefCb, nRefCb);
      DCT_ASSERT((no == nRefCb));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_REFCR_TAG_ID))
               && (NULL == pRefCr)) {
      nRefCr = tag.Size();
      pRefCr = (float*)malloc(sizeof(float) * nRefCr);

      int no = ParseFloatArray(tag.Value(), pRefCr, nRefCr);
      DCT_ASSERT((no == nRefCr));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_INDOOR_MIN_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjIndoorMin, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_OUTDOOR_MIN_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjOutdoorMin, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_MAX_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjMax, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_MAX_SKY_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjMaxSky, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE( CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_A_LIMIT_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjALimit, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_YELLOW_LIMIT_ENABLE_ID)){
      int no = ParseUshortArray(tag.Value(), &awb_data.fRgProjYellowLimitEnable, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_A_WEIGHT_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjAWeight, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_YELLOW_LIMIT_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjYellowLimit, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_ILL_TO_CWF_ENABLE_ID)){
      int no = ParseUshortArray(tag.Value(), &awb_data.fRgProjIllToCwfEnable, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_ILL_TO_CWF_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjIllToCwf, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_ILL_TO_CWF_WEIGHT_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjIllToCwfWeight, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_CLIP_OUTDOOR_ID)){
      char* value = Toupper(tag.Value());
      strncpy(awb_data.outdoor_clipping_profile,
              value, sizeof(awb_data.outdoor_clipping_profile));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_REGION_SIZE_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRegionSize, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_REGION_SIZE_INC_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRegionSizeInc, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_REGION_SIZE_DEC_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRegionSizeDec, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_ID)){
      if (!parseEntryAwb_V11_IIR(pchild->ToElement(), &awb_data)){
		LOGE( "%s(%d): parse error in AWB IIR section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
    } else {
#if 1
      LOGE( "%s(%d): parse error in AWB section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());    
#endif
      return (false);
    }	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
  DCT_ASSERT((nRg1 == nMaxDist1));
  DCT_ASSERT((nRg2 == nMaxDist2));

  DCT_ASSERT((nGlobalFade1 == nGlobalGainDistance1));
  DCT_ASSERT((nGlobalFade2 == nGlobalGainDistance2));

  DCT_ASSERT((nFade == nMinC_br));
  DCT_ASSERT((nFade == nMaxY_br));
  DCT_ASSERT((nFade == nMinY_br));
  DCT_ASSERT((nFade == nMinC_sr));
  DCT_ASSERT((nFade == nMaxY_sr));
  DCT_ASSERT((nFade == nMinY_sr));
  DCT_ASSERT((nFade == nMaxCSum_br));
  DCT_ASSERT((nFade == nMaxCSum_sr));
  DCT_ASSERT((nFade == nRefCb));
  DCT_ASSERT((nFade == nRefCr));

  /* CamAwbClipParm_t */
  awb_data.AwbClipParam.ArraySize1    = nRg1;
  awb_data.AwbClipParam.pRg1          = pRg1;
  awb_data.AwbClipParam.pMaxDist1     = pMaxDist1;
  awb_data.AwbClipParam.ArraySize2    = nRg2;
  awb_data.AwbClipParam.pRg2          = pRg2;
  awb_data.AwbClipParam.pMaxDist2     = pMaxDist2;

  /* CamAwbGlobalFadeParm_t */
  awb_data.AwbGlobalFadeParm.ArraySize1           = nGlobalFade1;
  awb_data.AwbGlobalFadeParm.pGlobalFade1         = pGlobalFade1;
  awb_data.AwbGlobalFadeParm.pGlobalGainDistance1 = pGlobalGainDistance1;
  awb_data.AwbGlobalFadeParm.ArraySize2           = nGlobalFade2;
  awb_data.AwbGlobalFadeParm.pGlobalFade2         = pGlobalFade2;
  awb_data.AwbGlobalFadeParm.pGlobalGainDistance2 = pGlobalGainDistance2;

  /* CamAwbFade2Parm_t */
  awb_data.AwbFade2Parm.ArraySize         = nFade;
  awb_data.AwbFade2Parm.pFade             = pFade;
  awb_data.AwbFade2Parm.pMaxCSum_br       = pMaxCSum_br;
  awb_data.AwbFade2Parm.pMaxCSum_sr       = pMaxCSum_sr;
  awb_data.AwbFade2Parm.pMinC_br          = pMinC_br;
  awb_data.AwbFade2Parm.pMaxY_br          = pMaxY_br;
  awb_data.AwbFade2Parm.pMinY_br          = pMinY_br;
  awb_data.AwbFade2Parm.pMinC_sr          = pMinC_sr;
  awb_data.AwbFade2Parm.pMaxY_sr          = pMaxY_sr;
  awb_data.AwbFade2Parm.pMinY_sr          = pMinY_sr;
  awb_data.AwbFade2Parm.pRefCb            = pRefCb;
  awb_data.AwbFade2Parm.pRefCr            = pRefCr;

  RESULT result = CamCalibDbAddAwb_V11_Global(m_CalibDbHandle, &awb_data);
  DCT_ASSERT(result == RET_SUCCESS);

  /* cleanup */
  free(pRg1);
  free(pMaxDist1);
  free(pRg2);
  free(pMaxDist2);

  free(pGlobalFade1);
  free(pGlobalGainDistance1);
  free(pGlobalFade2);
  free(pGlobalGainDistance2);

  free(pFade);
  free(pMaxCSum_br);
  free(pMaxCSum_sr);
  free(pMinC_br);
  free(pMaxY_br);
  free(pMinY_br);
  free(pMinC_sr);
  free(pMaxY_sr);
  free(pMinY_sr);

  free(pRefCb);
  free(pRefCr);

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}

/******************************************************************************
 * CalibDb::parseEntryAwb_V10_Globals
 *****************************************************************************/
bool CalibDb::parseEntryAwb_V10_Globals
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamCalibAwb_V10_Global_t awb_data;

  /* CamAwbClipParm_t */
  float* pRg1         = NULL;
  int nRg1        = 0;
  float* pMaxDist1    = NULL;
  int nMaxDist1   = 0;
  float* pRg2         = NULL;
  int nRg2        = 0;
  float* pMaxDist2    = NULL;
  int nMaxDist2   = 0;

  /* CamAwbGlobalFadeParm_t */
  float* pGlobalFade1         = NULL;
  int nGlobalFade1         = 0;
  float* pGlobalGainDistance1 = NULL;
  int nGlobalGainDistance1 = 0;
  float* pGlobalFade2         = NULL;
  int nGlobalFade2         = 0;
  float* pGlobalGainDistance2 = NULL;
  int nGlobalGainDistance2 = 0;

  /* CamAwbFade2Parm_t */
  float* pFade                = NULL;
  int nFade                = 0;
  float* pCbMinRegionMax      = NULL;
  int nCbMinRegionMax      = 0;
  float* pCrMinRegionMax      = NULL;
  int nCrMinRegionMax      = 0;
  float* pMaxCSumRegionMax    = NULL;
  int nMaxCSumRegionMax    = 0;
  float* pCbMinRegionMin      = NULL;
  int nCbMinRegionMin      = 0;
  float* pCrMinRegionMin      = NULL;
  int nCrMinRegionMin      = 0;
  float* pMaxCSumRegionMin    = NULL;
  int nMaxCSumRegionMin    = 0;

  float* pMinCRegionMax = NULL;
  int nMinCRegionMax = 0;
  float* pMinCRegionMin = NULL;
  int nMinCRegionMin = 0;

  float* pMaxYRegionMax = NULL;
  int nMaxYRegionMax = 0;
  float* pMaxYRegionMin = NULL;
  int nMaxYRegionMin = 0;

  float* pMinYMaxGRegionMax = NULL;
  int nMinYMaxGRegionMax = 0;
  float* pMinYMaxGRegionMin = NULL;
  int nMinYMaxGRegionMin = 0;

  float* pRefCr = NULL;
  int nRefCr = 0;
  float* pRefCb = NULL;
  int nRefCb = 0;

  XML_CHECK_START(CALIB_SENSOR_AWB_V10_GLOBALS_TAG_ID, CALIB_SENSOR_AWB_VERSION_10_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    const char* value = tag.Value();
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_NAME_TAG_ID)){
      strncpy(awb_data.name, value, sizeof(awb_data.name));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RESOLUTION_TAG_ID)){
      strncpy(awb_data.resolution, value, sizeof(awb_data.resolution));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_SVDMEANVALUE_TAG_ID)){
      int i = (sizeof(awb_data.SVDMeanValue) / sizeof(awb_data.SVDMeanValue.fCoeff[0]));
      int no = ParseFloatArray(value, awb_data.SVDMeanValue.fCoeff, i);

      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_PCAMATRIX_TAG_ID)){
      int i = (sizeof(awb_data.PCAMatrix) / sizeof(awb_data.PCAMatrix.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), awb_data.PCAMatrix.fCoeff, i);

      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_CENTERLINE_TAG_ID)){
      int i = (sizeof(awb_data.CenterLine) / sizeof(awb_data.CenterLine.f_N0_Rg));
      int no = ParseFloatArray(tag.Value(), &awb_data.CenterLine.f_N0_Rg, i);

      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_KFACTOR_TAG_ID)){
      int i = (sizeof(awb_data.KFactor) / sizeof(awb_data.KFactor.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), awb_data.KFactor.fCoeff, i);

      DCT_ASSERT((no == tag.Size()));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RG1_TAG_ID))
               && (NULL == pRg1)) {
      nRg1 = tag.Size();
      pRg1 = (float*)malloc(sizeof(float) * nRg1);

      int no = ParseFloatArray(tag.Value(), pRg1, nRg1);
      DCT_ASSERT((no == nRg1));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_MAXDIST1_TAG_ID))
               && (NULL == pMaxDist1)) {
      nMaxDist1 = tag.Size();
      pMaxDist1 = (float*)malloc(sizeof(float) * nMaxDist1);

      int no = ParseFloatArray(tag.Value(), pMaxDist1, nMaxDist1);
      DCT_ASSERT((no == nRg1));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RG2_TAG_ID))
               && (NULL == pRg2)) {
      nRg2 = tag.Size();
      pRg2 = (float*)malloc(sizeof(float) * nRg2);

      int no = ParseFloatArray(tag.Value(), pRg2, nRg2);
      DCT_ASSERT((no == nRg2));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_MAXDIST2_TAG_ID))
               && (NULL == pMaxDist2)) {
      nMaxDist2 = tag.Size();
      pMaxDist2 = (float*)malloc(sizeof(float) * nMaxDist2);

      int no = ParseFloatArray(tag.Value(), pMaxDist2, nMaxDist2);
      DCT_ASSERT((no == nMaxDist2));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_GLOBALFADE1_TAG_ID))
               && (NULL == pGlobalFade1)) {
      nGlobalFade1 = tag.Size();
      pGlobalFade1 = (float*)malloc(sizeof(float) * nGlobalFade1);

      int no = ParseFloatArray(tag.Value(), pGlobalFade1, nGlobalFade1);
      DCT_ASSERT((no == nGlobalFade1));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_GLOBALGAINDIST1_TAG_ID))
               && (NULL == pGlobalGainDistance1)) {
      nGlobalGainDistance1 = tag.Size();
      pGlobalGainDistance1 = (float*)malloc(sizeof(float) * nGlobalGainDistance1);

      int no = ParseFloatArray(tag.Value(), pGlobalGainDistance1, nGlobalGainDistance1);
      DCT_ASSERT((no == nGlobalGainDistance1));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_GLOBALFADE2_TAG_ID))
               && (NULL == pGlobalFade2)) {
      nGlobalFade2 = tag.Size();
      pGlobalFade2 = (float*)malloc(sizeof(float) * nGlobalFade2);

      int no = ParseFloatArray(tag.Value(), pGlobalFade2, nGlobalFade2);
      DCT_ASSERT((no == nGlobalFade2));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_GLOBALGAINDIST2_TAG_ID))
               && (NULL == pGlobalGainDistance2)) {
      nGlobalGainDistance2 = tag.Size();
      pGlobalGainDistance2 = (float*)malloc(sizeof(float) * nGlobalGainDistance2);

      int no = ParseFloatArray(tag.Value(), pGlobalGainDistance2, nGlobalGainDistance2);
      DCT_ASSERT((no == nGlobalGainDistance2));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_FADE2_TAG_ID))
               && (NULL == pFade)) {
      nFade = tag.Size();
      pFade = (float*)malloc(sizeof(float) * nFade);

      int no = ParseFloatArray(tag.Value(), pFade, nFade);
      DCT_ASSERT((no == nFade));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_CB_MIN_REGIONMAX_TAG_ID))
               && (NULL == pCbMinRegionMax)) {
      nCbMinRegionMax = tag.Size();
      pCbMinRegionMax = (float*)malloc(sizeof(float) * nCbMinRegionMax);

      int no = ParseFloatArray(tag.Value(), pCbMinRegionMax, nCbMinRegionMax);
      DCT_ASSERT((no == nCbMinRegionMax));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_CR_MIN_REGIONMAX_TAG_ID))
               && (NULL == pCrMinRegionMax)) {
      nCrMinRegionMax = tag.Size();
      pCrMinRegionMax = (float*)malloc(sizeof(float) * nCrMinRegionMax);

      int no = ParseFloatArray(tag.Value(), pCrMinRegionMax, nCrMinRegionMax);
      DCT_ASSERT((no == nCrMinRegionMax));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_MAX_CSUM_REGIONMAX_TAG_ID))
               && (NULL == pMaxCSumRegionMax)) {
      nMaxCSumRegionMax = tag.Size();
      pMaxCSumRegionMax = (float*)malloc(sizeof(float) * nMaxCSumRegionMax);

      int no = ParseFloatArray(tag.Value(), pMaxCSumRegionMax, nMaxCSumRegionMax);
      DCT_ASSERT((no == nMaxCSumRegionMax));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_CB_MIN_REGIONMIN_TAG_ID))
               && (NULL == pCbMinRegionMin)) {
      nCbMinRegionMin = tag.Size();
      pCbMinRegionMin = (float*)malloc(sizeof(float) * nCbMinRegionMin);

      int no = ParseFloatArray(tag.Value(), pCbMinRegionMin, nCbMinRegionMin);
      DCT_ASSERT((no == nCbMinRegionMin));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_CR_MIN_REGIONMIN_TAG_ID))
               && (NULL == pCrMinRegionMin)) {
      nCrMinRegionMin = tag.Size();
      pCrMinRegionMin = (float*)malloc(sizeof(float) * nCrMinRegionMin);

      int no = ParseFloatArray(tag.Value(), pCrMinRegionMin, nCrMinRegionMin);
      DCT_ASSERT((no == nCrMinRegionMin));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_MAX_CSUM_REGIONMIN_TAG_ID))
               && (NULL == pMaxCSumRegionMin)) {
      nMaxCSumRegionMin = tag.Size();
      pMaxCSumRegionMin = (float*)malloc(sizeof(float) * nMaxCSumRegionMin);

      int no = ParseFloatArray(tag.Value(), pMaxCSumRegionMin, nMaxCSumRegionMin);
      DCT_ASSERT((no == nMaxCSumRegionMin));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_MINC_REGIONMAX_TAG_ID))
               && (NULL == pMinCRegionMax)) {
      nMinCRegionMax = tag.Size();
      pMinCRegionMax = (float*)malloc(sizeof(float) * nMinCRegionMax);

      int no = ParseFloatArray(tag.Value(), pMinCRegionMax, nMinCRegionMax);
      DCT_ASSERT((no == nMinCRegionMax));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_MINC_REGIONMIN_TAG_ID))
               && (NULL == pMinCRegionMin)) {
      nMinCRegionMin = tag.Size();
      pMinCRegionMin = (float*)malloc(sizeof(float) * nMinCRegionMin);

      int no = ParseFloatArray(tag.Value(), pMinCRegionMin, nMinCRegionMin);
      DCT_ASSERT((no == nMinCRegionMin));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_MAXY_REGIONMAX_TAG_ID))               
    		&& (NULL == pMaxYRegionMax)) {
      nMaxYRegionMax = tag.Size();
      pMaxYRegionMax = (float*)malloc(sizeof(float) * nMaxYRegionMax);

      int no = ParseFloatArray(tag.Value(), pMaxYRegionMax, nMaxYRegionMax);
      DCT_ASSERT((no == nMaxYRegionMax));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_MAXY_REGIONMIN_TAG_ID))
               && (NULL == pMaxYRegionMin)) {
      nMaxYRegionMin = tag.Size();
      pMaxYRegionMin = (float*)malloc(sizeof(float) * nMaxYRegionMin);

      int no = ParseFloatArray(tag.Value(), pMaxYRegionMin, nMaxYRegionMin);
      DCT_ASSERT((no == nMaxYRegionMin));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_MINY_MAXG_REGIONMAX_TAG_ID))
               && (NULL == pMinYMaxGRegionMax)) {
      nMinYMaxGRegionMax = tag.Size();
      pMinYMaxGRegionMax = (float*)malloc(sizeof(float) * nMinYMaxGRegionMax);

      int no = ParseFloatArray(tag.Value(), pMinYMaxGRegionMax, nMinYMaxGRegionMax);
      DCT_ASSERT((no == nMinYMaxGRegionMax));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_MINY_MAXG_REGIONMIN_TAG_ID))
               && (NULL == pMinYMaxGRegionMin)) {
      nMinYMaxGRegionMin = tag.Size();
      pMinYMaxGRegionMin = (float*)malloc(sizeof(float) * nMinYMaxGRegionMin);

      int no = ParseFloatArray(tag.Value(), pMinYMaxGRegionMin, nMinYMaxGRegionMin);
      DCT_ASSERT((no == nMinYMaxGRegionMin));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_REFCB_TAG_ID))
               && (NULL == pRefCb)) {
      nRefCb = tag.Size();
      pRefCb = (float*)malloc(sizeof(float) * nRefCb);

      int no = ParseFloatArray(tag.Value(), pRefCb, nRefCb);
      DCT_ASSERT((no == nRefCb));
    } else if ((XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_REFCR_TAG_ID))
               && (NULL == pRefCr)) {
      nRefCr = tag.Size();
      pRefCr = (float*)malloc(sizeof(float) * nRefCr);

      int no = ParseFloatArray(tag.Value(), pRefCr, nRefCr);
      DCT_ASSERT((no == nRefCr));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_INDOOR_MIN_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjIndoorMin, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_OUTDOOR_MIN_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjOutdoorMin, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_MAX_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjMax, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_MAX_SKY_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjMaxSky, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_A_LIMIT_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjALimit, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_YELLOW_LIMIT_ENABLE_ID)){
      int no = ParseUshortArray(tag.Value(), &awb_data.fRgProjYellowLimitEnable, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_A_WEIGHT_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjAWeight, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_YELLOW_LIMIT_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjYellowLimit, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_ILL_TO_CWF_ENABLE_ID)){
      int no = ParseUshortArray(tag.Value(), &awb_data.fRgProjIllToCwfEnable, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_ILL_TO_CWF_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjIllToCwf, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_RGPROJ_ILL_TO_CWF_WEIGHT_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjIllToCwfWeight, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_CLIP_OUTDOOR_ID)){
      char* value = Toupper(tag.Value());
      strncpy(awb_data.outdoor_clipping_profile,
              value, sizeof(awb_data.outdoor_clipping_profile));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_REGION_SIZE_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRegionSize, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_REGION_SIZE_INC_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRegionSizeInc, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_REGION_SIZE_DEC_ID)){
      int no = ParseFloatArray(tag.Value(), &awb_data.fRegionSizeDec, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_GLOBALS_IIR_ID)){
	  if (!parseEntryAwb_V10_IIR(pchild->ToElement(), &awb_data)){
		LOGE( "%s(%d): parse error in AWB IIR section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
    } else {
#if 1
      LOGE( "%s(%d): parse error in AWB section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
      return (false);
    }
	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
  DCT_ASSERT((nRg1 == nMaxDist1));
  DCT_ASSERT((nRg2 == nMaxDist2));

  DCT_ASSERT((nGlobalFade1 == nGlobalGainDistance1));
  DCT_ASSERT((nGlobalFade2 == nGlobalGainDistance2));

  DCT_ASSERT((nFade == nCbMinRegionMax));
  DCT_ASSERT((nFade == nCrMinRegionMax));
  DCT_ASSERT((nFade == nMaxCSumRegionMax));
  DCT_ASSERT((nFade == nCbMinRegionMin));
  DCT_ASSERT((nFade == nCrMinRegionMin));
  DCT_ASSERT((nFade == nMaxCSumRegionMin));
  DCT_ASSERT((nFade == nMinCRegionMax));
  DCT_ASSERT((nFade == nMinCRegionMin));
  DCT_ASSERT((nFade == nMaxYRegionMax));
  DCT_ASSERT((nFade == nMaxYRegionMin));
  DCT_ASSERT((nFade == nMinYMaxGRegionMax));
  DCT_ASSERT((nFade == nMinYMaxGRegionMin));
  DCT_ASSERT((nFade == nRefCr));
  DCT_ASSERT((nFade == nRefCb));

  /* CamAwbClipParm_t */
  awb_data.AwbClipParam.ArraySize1    = nRg1;
  awb_data.AwbClipParam.pRg1          = pRg1;
  awb_data.AwbClipParam.pMaxDist1     = pMaxDist1;
  awb_data.AwbClipParam.ArraySize2    = nRg2;
  awb_data.AwbClipParam.pRg2          = pRg2;
  awb_data.AwbClipParam.pMaxDist2     = pMaxDist2;

  /* CamAwbGlobalFadeParm_t */
  awb_data.AwbGlobalFadeParm.ArraySize1           = nGlobalFade1;
  awb_data.AwbGlobalFadeParm.pGlobalFade1         = pGlobalFade1;
  awb_data.AwbGlobalFadeParm.pGlobalGainDistance1 = pGlobalGainDistance1;
  awb_data.AwbGlobalFadeParm.ArraySize2           = nGlobalFade2;
  awb_data.AwbGlobalFadeParm.pGlobalFade2         = pGlobalFade2;
  awb_data.AwbGlobalFadeParm.pGlobalGainDistance2 = pGlobalGainDistance2;

  /* CamAwbFade2Parm_t */
  awb_data.AwbFade2Parm.ArraySize         = nFade;
  awb_data.AwbFade2Parm.pFade             = pFade;
  awb_data.AwbFade2Parm.pCbMinRegionMax   = pCbMinRegionMax;
  awb_data.AwbFade2Parm.pCrMinRegionMax   = pCrMinRegionMax;
  awb_data.AwbFade2Parm.pMaxCSumRegionMax = pMaxCSumRegionMax;
  awb_data.AwbFade2Parm.pCbMinRegionMin   = pCbMinRegionMin;
  awb_data.AwbFade2Parm.pCrMinRegionMin   = pCrMinRegionMin;
  awb_data.AwbFade2Parm.pMaxCSumRegionMin = pMaxCSumRegionMin;
  awb_data.AwbFade2Parm.pMinCRegionMax  = pMinCRegionMax;
  awb_data.AwbFade2Parm.pMinCRegionMin  = pMinCRegionMin;
  awb_data.AwbFade2Parm.pMaxYRegionMax    = pMaxYRegionMax;
  awb_data.AwbFade2Parm.pMaxYRegionMin    = pMaxYRegionMin;
  awb_data.AwbFade2Parm.pMinYMaxGRegionMax = pMinYMaxGRegionMax;
  awb_data.AwbFade2Parm.pMinYMaxGRegionMin = pMinYMaxGRegionMin;
  awb_data.AwbFade2Parm.pRefCb = pRefCb;
  awb_data.AwbFade2Parm.pRefCr = pRefCr;

  RESULT result = CamCalibDbAddAwb_V10_Global(m_CalibDbHandle, &awb_data);
  DCT_ASSERT(result == RET_SUCCESS);

  /* cleanup */
  free(pRg1);
  free(pMaxDist1);
  free(pRg2);
  free(pMaxDist2);

  free(pGlobalFade1);
  free(pGlobalGainDistance1);
  free(pGlobalFade2);
  free(pGlobalGainDistance2);

  free(pFade);
  free(pCbMinRegionMax);
  free(pCrMinRegionMax);
  free(pMaxCSumRegionMax);
  free(pCbMinRegionMin);
  free(pCrMinRegionMin);
  free(pMaxCSumRegionMin);

  free(pMinCRegionMax);
  free(pMinCRegionMin);
  free(pMaxYRegionMax);
  free(pMaxYRegionMin);
  free(pMinYMaxGRegionMax);
  free(pMinYMaxGRegionMin);
  free(pRefCb);
  free(pRefCr);

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}






/******************************************************************************
 * CalibDb::parseEntryAwb_V10_Para
 *****************************************************************************/
bool CalibDb::parseEntryAwb_V10_IlluminationGMM
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

    CamAwb_V10_IlluProfile_t *pIllu= (CamAwb_V10_IlluProfile_t *)param;
    if(NULL == pIllu){
		LOGD("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
  		return false;
    }

    XML_CHECK_START(CALIB_SENSOR_AWB_V10_ILLUMINATION_GMM_TAG_ID, CALIB_SENSOR_AWB_V10_ILLUMINATION_TAG_ID);
  
	const XMLNode* pchild = pelement->FirstChild();
  	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_GMM_GAUSSIAN_MVALUE_TAG_ID)){
          int i = (sizeof(pIllu->GaussMeanValue) / sizeof(pIllu->GaussMeanValue.fCoeff[0]));
          int no = ParseFloatArray(tag.Value(), pIllu->GaussMeanValue.fCoeff, i);
          DCT_ASSERT((no == tag.Size()));
        } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_GMM_INV_COV_MATRIX_TAG_ID)){
          int i = (sizeof(pIllu->CovarianceMatrix) / sizeof(pIllu->CovarianceMatrix.fCoeff[0]));
          int no = ParseFloatArray(tag.Value(), pIllu->CovarianceMatrix.fCoeff, i);
          DCT_ASSERT((no == tag.Size()));
        } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_GMM_GAUSSIAN_SFACTOR_TAG_ID)){
          int i = (sizeof(pIllu->GaussFactor) / sizeof(pIllu->GaussFactor.fCoeff[0]));
          int no = ParseFloatArray(tag.Value(), pIllu->GaussFactor.fCoeff, i);
          DCT_ASSERT((no == tag.Size()));
        } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_GMM_TAU_TAG_ID)){
          int i = (sizeof(pIllu->Threshold) / sizeof(pIllu->Threshold.fCoeff[0]));
          int no = ParseFloatArray(tag.Value(), pIllu->Threshold.fCoeff, i);
          DCT_ASSERT((no == tag.Size()));
        } else {
#if 1
		  LOGE( "%s(%d): parse error in AWB illumination GMM section (unknow tag: %s)\n", 
		  		__FUNCTION__,__LINE__,tagname.c_str());
#endif
		  return (false);
		}		
    	pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

return true;

}



bool CalibDb::parseEntryAwb_V10_IlluminationSat
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	float* afGain   = NULL;
    int n_gains     = 0;
    float* afSat    = NULL;
    int n_sats      = 0;
	  
    CamSaturationCurve_t *pSaturationCurve= (CamSaturationCurve_t *)param;
    if(NULL == pSaturationCurve){
		LOGD("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
  		return false;
    }

    XML_CHECK_START(CALIB_SENSOR_AWB_V10_ILLUMINATION_SAT_CT_TAG_ID, CALIB_SENSOR_AWB_V10_ILLUMINATION_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
  	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_SAT_CT_GAIN_TAG_ID)){
          if (!afGain) {
            n_gains = tag.Size();
            afGain  = (float*)malloc((n_gains * sizeof(float)));
            MEMSET(afGain, 0, (n_gains * sizeof(float)));
          }

          int no = ParseFloatArray(tag.Value(), afGain, n_gains);
          DCT_ASSERT((no == n_gains));
        } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_SAT_CT_SAT_TAG_ID)){
          if (!afSat) {
            n_sats = tag.Size();
            afSat = (float*)malloc((n_sats * sizeof(float)));
            MEMSET(afSat, 0, (n_sats * sizeof(float)));
          }

          int no = ParseFloatArray(tag.Value(), afSat, n_sats);
          DCT_ASSERT((no == n_sats));
        } else {
#if 1
		  LOGE( "%s(%d): parse error in awb illumination sat section (unknow tag: %s)\n", 
		  __FUNCTION__,__LINE__,tagname.c_str());
#endif
		  return (false);
		}		
    	pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
	
	DCT_ASSERT((n_gains == n_sats));
    pSaturationCurve->ArraySize      = n_gains;
    pSaturationCurve->pSensorGain    = afGain;
    pSaturationCurve->pSaturation    = afSat;
	  

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}




bool CalibDb::parseEntryAwb_V11_IlluminationSat
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	float* afGain   = NULL;
    int n_gains     = 0;
    float* afSat    = NULL;
    int n_sats      = 0;
	  
    CamSaturationCurve_t *pSaturationCurve= (CamSaturationCurve_t *)param;
    if(NULL == pSaturationCurve){
		LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
  		return false;
    }

    XML_CHECK_START(CALIB_SENSOR_AWB_V10_ILLUMINATION_SAT_CT_TAG_ID, CALIB_SENSOR_AWB_V11_ILLUMINATION_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
  	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_SAT_CT_GAIN_TAG_ID)){
          if (!afGain) {
            n_gains = tag.Size();
            afGain  = (float*)malloc((n_gains * sizeof(float)));
            MEMSET(afGain, 0, (n_gains * sizeof(float)));
          }

          int no = ParseFloatArray(tag.Value(), afGain, n_gains);
          DCT_ASSERT((no == n_gains));
        } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_SAT_CT_SAT_TAG_ID)){
          if (!afSat) {
            n_sats = tag.Size();
            afSat = (float*)malloc((n_sats * sizeof(float)));
            MEMSET(afSat, 0, (n_sats * sizeof(float)));
          }

          int no = ParseFloatArray(tag.Value(), afSat, n_sats);
          DCT_ASSERT((no == n_sats));
        } else {
#if 1
		  LOGE( "%s(%d): parse error in awb illumination sat section (unknow tag: %s)\n", 
		  __FUNCTION__,__LINE__,tagname.c_str());
#endif
		  return (false);
		}		
    	pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
	
	DCT_ASSERT((n_gains == n_sats));
    pSaturationCurve->ArraySize      = n_gains;
    pSaturationCurve->pSensorGain    = afGain;
    pSaturationCurve->pSaturation    = afSat;
	  

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}

bool CalibDb::parseEntryAwb_V10_IlluminationVig
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	float* afGain   = NULL;
    int n_gains     = 0;
    float* afVig    = NULL;
    int n_vigs      = 0;
	  
    CamVignettingCurve_t *pVignettingCurve= (CamVignettingCurve_t *)param;
    if(NULL == pVignettingCurve){
		LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
  		return false;
    }

    XML_CHECK_START(CALIB_SENSOR_AWB_V10_ILLUMINATION_VIG_CT_TAG_ID, CALIB_SENSOR_AWB_V10_ILLUMINATION_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
  	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_VIG_CT_GAIN_TAG_ID)){
          if (!afGain) {
            n_gains = tag.Size();
            afGain  = (float*)malloc((n_gains * sizeof(float)));
            MEMSET(afGain, 0, (n_gains * sizeof(float)));
          }

          int no = ParseFloatArray(tag.Value(), afGain, n_gains);
          DCT_ASSERT((no == n_gains));
        } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_VIG_CT_VIG_TAG_ID)){
          if (!afVig) {
            n_vigs = tag.Size();
            afVig = (float*)malloc((n_vigs * sizeof(float)));
            MEMSET(afVig, 0, (n_vigs * sizeof(float)));
          }

          int no = ParseFloatArray(tag.Value(), afVig, n_vigs);
          DCT_ASSERT((no == n_vigs));
        } else {
#if 1
		  LOGE( "%s(%d): parse error in awb illumination vig section (unknow tag: %s)\n", 
		  __FUNCTION__,__LINE__,tagname.c_str());
#endif
		  return (false);
		}		
    	pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
	
	DCT_ASSERT((n_gains == n_vigs));
    pVignettingCurve->ArraySize      = n_gains;
    pVignettingCurve->pSensorGain    = afGain;
    pVignettingCurve->pVignetting    = afVig;
	  

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}


bool CalibDb::parseEntryAwb_V11_IlluminationVig
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	float* afGain   = NULL;
    int n_gains     = 0;
    float* afVig    = NULL;
    int n_vigs      = 0;
	  
    CamVignettingCurve_t *pVignettingCurve= (CamVignettingCurve_t *)param;
    if(NULL == pVignettingCurve){
		LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
  		return false;
    }

    XML_CHECK_START(CALIB_SENSOR_AWB_V10_ILLUMINATION_VIG_CT_TAG_ID, CALIB_SENSOR_AWB_V11_ILLUMINATION_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
  	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_VIG_CT_GAIN_TAG_ID)){
          if (!afGain) {
            n_gains = tag.Size();
            afGain  = (float*)malloc((n_gains * sizeof(float)));
            MEMSET(afGain, 0, (n_gains * sizeof(float)));
          }

          int no = ParseFloatArray(tag.Value(), afGain, n_gains);
          DCT_ASSERT((no == n_gains));
        } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_VIG_CT_VIG_TAG_ID)){
          if (!afVig) {
            n_vigs = tag.Size();
            afVig = (float*)malloc((n_vigs * sizeof(float)));
            MEMSET(afVig, 0, (n_vigs * sizeof(float)));
          }

          int no = ParseFloatArray(tag.Value(), afVig, n_vigs);
          DCT_ASSERT((no == n_vigs));
        } else {
#if 1
		  LOGE( "%s(%d): parse error in awb illumination vig section (unknow tag: %s)\n", 
		  __FUNCTION__,__LINE__,tagname.c_str());
#endif
		  return (false);
		}		
    	pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
	
	DCT_ASSERT((n_gains == n_vigs));
    pVignettingCurve->ArraySize      = n_gains;
    pVignettingCurve->pSensorGain    = afGain;
    pVignettingCurve->pVignetting    = afVig;
	  

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}

/******************************************************************************
 * CalibDb::parseEntryAwbIllumination
 *****************************************************************************/
bool CalibDb::parseEntryAwb_V10_Illumination
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamAwb_V10_IlluProfile_t illu;
  MEMSET(&illu, 0, sizeof(illu));

  XML_CHECK_START(CALIB_SENSOR_AWB_V10_ILLUMINATION_TAG_ID, CALIB_SENSOR_AWB_VERSION_10_TAG_ID);
    
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_NAME_TAG_ID)){
      char* value = Toupper(tag.Value());
      strncpy(illu.name, value, sizeof(illu.name));
#ifdef DEBUG_LOG
      LOGE( "%s(%d): tag: %s\n", __FUNCTION__,__LINE__,value);
#endif
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_DOOR_TYPE_TAG_ID)){
      char* value = Toupper(tag.Value());
      std::string s_value(value);
#ifdef DEBUG_LOG
            LOGD( "%s(%d): tag: %s\n", __FUNCTION__,__LINE__,s_value.c_str());
#endif
      if (s_value == CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_INDOOR) {
        illu.DoorType = CAM_DOOR_TYPE_INDOOR;
      } else if (s_value == CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_OUTDOOR) {
        illu.DoorType = CAM_DOOR_TYPE_OUTDOOR;
      } else {
        LOGE( "%s(%d): invalid illumination doortype (%s)\n", __FUNCTION__,__LINE__,s_value.c_str());
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_AWB_TYPE_TAG_ID)){
      char* value = Toupper(tag.Value());
      std::string s_value(value);
#ifdef DEBUG_LOG
            LOGD( "%s(%d): tag: %s\n", __FUNCTION__,__LINE__,s_value.c_str());
#endif
      if (s_value == CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_MANUAL) {
        illu.AwbType = CAM_AWB_TYPE_MANUAL;
      } else if (s_value == CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_AUTO) {
        illu.AwbType = CAM_AWB_TYPE_AUTO;
      } else {
        LOGE( "%s(%d): invalid AWB type (%s)\n", __FUNCTION__,__LINE__,s_value.c_str());
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_MANUAL_WB_TAG_ID)){
      int i = (sizeof(illu.ComponentGain) / sizeof(illu.ComponentGain.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), illu.ComponentGain.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_MANUAL_CC_TAG_ID)){
      int i = (sizeof(illu.CrossTalkCoeff) / sizeof(illu.CrossTalkCoeff.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), illu.CrossTalkCoeff.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_MANUAL_CTO_TAG_ID)){
      int i = (sizeof(illu.CrossTalkOffset) / sizeof(illu.CrossTalkOffset.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), illu.CrossTalkOffset.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_GMM_TAG_ID)){
      if (!parseEntryAwb_V10_IlluminationGMM(pchild->ToElement(), &illu)){
		LOGE( "%s(%d): parse error in Awb V10 Illumination GMM section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_SAT_CT_TAG_ID)){
      if (!parseEntryAwb_V10_IlluminationSat(pchild->ToElement(), &illu.SaturationCurve)){
		LOGE( "%s(%d): parse error in Awb V10 Illumination Sat section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_VIG_CT_TAG_ID)){
      if (!parseEntryAwb_V10_IlluminationVig(pchild->ToElement(), &illu.VignettingCurve)){
		LOGE( "%s(%d): parse error in Awb V10 Illumination Vig section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_ALSC_TAG_ID)){
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryAwb_V10_IlluminationAlsc, &illu,
	  		(uint32_t)(CALIB_SENSOR_AWB_V10_ILLUMINATION_ALSC_TAG_ID), (uint32_t)(CALIB_SENSOR_AWB_V10_ILLUMINATION_TAG_ID))) {
#if 1
        LOGE( "%s(%d): parse error in AWB aLSC (%s)\n", __FUNCTION__,__LINE__,tagname.c_str());    
#endif
        return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_ACC_TAG_ID)){
      if (!parseEntryAwb_V10_IlluminationAcc(pchild->ToElement(), &illu)){
		LOGE( "%s(%d): parse error in Awb V10 Illumination Acc section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
    } else {
#if 1
      LOGE( "%s(%d): parse error in AWB illumination section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
      return (false);
    }	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
  RESULT result = CamCalibDbAddAwb_V10_Illumination(m_CalibDbHandle, &illu);
  DCT_ASSERT(result == RET_SUCCESS);

  /* cleanup */
  free(illu.SaturationCurve.pSensorGain);
  free(illu.SaturationCurve.pSaturation);
  free(illu.VignettingCurve.pSensorGain);
  free(illu.VignettingCurve.pVignetting);

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}



/******************************************************************************
 * CalibDb::parseEntryAwb_V11_Illumination
 *****************************************************************************/
bool CalibDb::parseEntryAwb_V11_Illumination
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamAwb_V11_IlluProfile_t illu;
  MEMSET(&illu, 0, sizeof(illu));

  XML_CHECK_START(CALIB_SENSOR_AWB_V11_ILLUMINATION_TAG_ID, CALIB_SENSOR_AWB_VERSION_11_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_NAME_TAG_ID)){
      char* value = Toupper(tag.Value());
      strncpy(illu.name, value, sizeof(illu.name));
#ifdef DEBUG_LOG
 	  LOGD( "%s(%d): tag: %s\n", __FUNCTION__,__LINE__,value);
#endif
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_DOOR_TYPE_TAG_ID)){
      char* value = Toupper(tag.Value());
      std::string s_value(value);
#ifdef DEBUG_LOG
      LOGD( "%s(%d): value: %s\n", __FUNCTION__,__LINE__,s_value.c_str());
#endif
      if (s_value == CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_INDOOR) {
        illu.DoorType = CAM_DOOR_TYPE_INDOOR;
      } else if (s_value == CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_OUTDOOR) {
        illu.DoorType = CAM_DOOR_TYPE_OUTDOOR;
      } else {
                LOGD( "%s(%d): invalid illumination doortype (%s)\n", __FUNCTION__,__LINE__,s_value.c_str());
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_AWB_TYPE_TAG_ID)){
      char* value = Toupper(tag.Value());
      std::string s_value(value);
#ifdef DEBUG_LOG
      LOGD( "%s(%d): value: %s\n", __FUNCTION__,__LINE__,s_value.c_str());                  
#endif
      if (s_value == CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_MANUAL) {
        illu.AwbType = CAM_AWB_TYPE_MANUAL;
      } else if (s_value == CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_AUTO) {
        illu.AwbType = CAM_AWB_TYPE_AUTO;
      } else {
        LOGE( "%s(%d): invalid AWB type (%s)\n", __FUNCTION__,__LINE__,s_value.c_str());
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_MANUAL_WB_TAG_ID)){
      int i = (sizeof(illu.ComponentGain) / sizeof(illu.ComponentGain.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), illu.ComponentGain.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_MANUAL_CC_TAG_ID)){
      int i = (sizeof(illu.CrossTalkCoeff) / sizeof(illu.CrossTalkCoeff.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), illu.CrossTalkCoeff.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_MANUAL_CTO_TAG_ID)){
      int i = (sizeof(illu.CrossTalkOffset) / sizeof(illu.CrossTalkOffset.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), illu.CrossTalkOffset.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V11_ILLUMINATION_REFERENCE_GAIN_TAG_ID)){
      int i = (sizeof(illu.referenceWBgain) / sizeof(illu.referenceWBgain.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), illu.referenceWBgain.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_SAT_CT_TAG_ID)){
      if (!parseEntryAwb_V11_IlluminationSat(pchild->ToElement(), &illu.SaturationCurve)){
		LOGE( "%s(%d): parse error in Awb V10 Illumination Sat section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_VIG_CT_TAG_ID)){
      if (!parseEntryAwb_V11_IlluminationVig(pchild->ToElement(), &illu.VignettingCurve)){
		LOGE( "%s(%d): parse error in Awb V10 Illumination Vig section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
    }  else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_ALSC_TAG_ID)){
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryAwb_V11_IlluminationAlsc, &illu,
	  	(uint32_t)(CALIB_SENSOR_AWB_V10_ILLUMINATION_ALSC_TAG_ID), (uint32_t)(CALIB_SENSOR_AWB_V11_ILLUMINATION_TAG_ID))) {
#if 1
        LOGE( "%s(%d): parse error in AWB aLSC (%s)\n", __FUNCTION__,__LINE__,tagname.c_str());    
#endif
        return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_ACC_TAG_ID)){
      if (!parseEntryAwb_V11_IlluminationAcc(pchild->ToElement(), &illu)){
		LOGE( "%s(%d): parse error in Awb V11 Illumination Acc section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
    } else {
#if 1
      LOGE( "%s(%d): parse error in AWB section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());         
#endif
      return (false);
    }	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
  RESULT result = CamCalibDbAddAwb_V11_Illumination(m_CalibDbHandle, &illu);
  DCT_ASSERT(result == RET_SUCCESS);

  /* cleanup */
  free(illu.SaturationCurve.pSensorGain);
  free(illu.SaturationCurve.pSaturation);
  free(illu.VignettingCurve.pSensorGain);
  free(illu.VignettingCurve.pVignetting);

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}
/******************************************************************************
 * CalibDb::parseEntryAwbIlluminationAlsc
 *****************************************************************************/
bool CalibDb::parseEntryAwb_V10_IlluminationAlsc
(
    const XMLElement*   pelement,
    void*                param
) {
#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  if (!param) {
    return (false);
  }

  CamAwb_V10_IlluProfile_t* pIllu = (CamAwb_V10_IlluProfile_t*)param;

  char* lsc_profiles;
  int resIdx = -1;

  XML_CHECK_START(CALIB_SENSOR_AWB_V10_ILLUMINATION_ALSC_TAG_ID, CALIB_SENSOR_AWB_V10_ILLUMINATION_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());	
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_ALSC_RES_LSC_PROFILE_LIST_TAG_ID)){
      lsc_profiles = Toupper(tag.Value());
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_ALSC_RES_TAG_ID)){
      const char* value = tag.Value();
#ifdef DEBUG_LOG
      LOGE( "%s(%d): value: %s\n", __FUNCTION__,__LINE__,value);
#endif
      RESULT result = CamCalibDbGetResolutionIdxByName(m_CalibDbHandle, value, &resIdx);
      DCT_ASSERT(result == RET_SUCCESS);
    } else {
      LOGE( "%s(%d): unknown awb v11 aLSC tag: (%s)\n", __FUNCTION__,__LINE__,tagname.c_str());
      return (false);
    }	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  DCT_ASSERT(resIdx != -1);

  int no = ParseLscProfileArray(lsc_profiles, pIllu->lsc_profiles[resIdx], CAM_NO_LSC_PROFILES);
  DCT_ASSERT((no <= CAM_NO_LSC_PROFILES));
  pIllu->lsc_no[resIdx] = no;

  pIllu->lsc_res_no++;

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}



/******************************************************************************
 * CalibDb::parseEntryAwb_V11_IlluminationAlsc
 *****************************************************************************/
bool CalibDb::parseEntryAwb_V11_IlluminationAlsc
(
    const XMLElement*   pelement,
    void*                param
) {
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  if (!param) {
    return (false);
  }

  CamAwb_V11_IlluProfile_t* pIllu = (CamAwb_V11_IlluProfile_t*)param;

  char* lsc_profiles;
  int resIdx = -1;

  XML_CHECK_START(CALIB_SENSOR_AWB_V10_ILLUMINATION_ALSC_TAG_ID, CALIB_SENSOR_AWB_V11_ILLUMINATION_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_ALSC_RES_LSC_PROFILE_LIST_TAG_ID)){
      lsc_profiles = Toupper(tag.Value());
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_ALSC_RES_TAG_ID)){
      const char* value = tag.Value();
#ifdef DEBUG_LOG
      LOGE( "%s(%d): value: %s\n", __FUNCTION__,__LINE__,value);
#endif
      RESULT result = CamCalibDbGetResolutionIdxByName(m_CalibDbHandle, value, &resIdx);
      DCT_ASSERT(result == RET_SUCCESS);
    } else {
      LOGE( "%s(%d): unknown awb v11 aLSC tag:  %s\n", __FUNCTION__,__LINE__,tagname.c_str());    
      return (false);
    }	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
  DCT_ASSERT(resIdx != -1);

  int no = ParseLscProfileArray(lsc_profiles, pIllu->lsc_profiles[resIdx], CAM_NO_LSC_PROFILES);
  DCT_ASSERT((no <= CAM_NO_LSC_PROFILES));
  pIllu->lsc_no[resIdx] = no;

  pIllu->lsc_res_no++;

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}

/******************************************************************************
 * CalibDb::parseEntryAwbIlluminationAcc
 *****************************************************************************/
bool CalibDb::parseEntryAwb_V10_IlluminationAcc
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamAwb_V10_IlluProfile_t *pIllu= (CamAwb_V10_IlluProfile_t *)param;
  if(NULL == pIllu){
  	LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
	return false;
  }

  XML_CHECK_START(CALIB_SENSOR_AWB_V10_ILLUMINATION_ACC_TAG_ID, CALIB_SENSOR_AWB_V10_ILLUMINATION_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG        
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_ACC_CC_PROFILE_LIST_TAG_ID)){
      char* value = Toupper(tag.Value());
      int no = ParseCcProfileArray(value, pIllu->cc_profiles, CAM_NO_CC_PROFILES);
      DCT_ASSERT((no <= CAM_NO_CC_PROFILES));
      pIllu->cc_no = no;
    }else {
      LOGE( "%s(%d): unknown awb v10 aCC tag: %s\n", __FUNCTION__,__LINE__,tagname.c_str());
      return (false);
    }
	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
	
#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}

/******************************************************************************
 * CalibDb::parseEntryAwb_V11_IlluminationAcc
 *****************************************************************************/
bool CalibDb::parseEntryAwb_V11_IlluminationAcc
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamAwb_V11_IlluProfile_t *pIllu= (CamAwb_V11_IlluProfile_t *)param;
  if(NULL == pIllu){
  	LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
	return false;
  }

  XML_CHECK_START(CALIB_SENSOR_AWB_V10_ILLUMINATION_ACC_TAG_ID, CALIB_SENSOR_AWB_V11_ILLUMINATION_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG    
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

   	if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_AWB_V10_ILLUMINATION_ACC_CC_PROFILE_LIST_TAG_ID)){
      char* value = Toupper(tag.Value());
      int no = ParseCcProfileArray(value, pIllu->cc_profiles, CAM_NO_CC_PROFILES);
      DCT_ASSERT((no <= CAM_NO_CC_PROFILES));
      pIllu->cc_no = no;
    }else {
      LOGE( "%s(%d): unknown aCC tag: %s\n", __FUNCTION__,__LINE__,tagname.c_str());
      return (false);
    }
	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}

/******************************************************************************
 * CalibDb::parseEntryLsc
 *****************************************************************************/
bool CalibDb::parseEntryLsc
(
    const XMLElement*   pelement,
    void*                param
) {
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamLscProfile_t lsc_profile;
  MEMSET(&lsc_profile, 0, sizeof(lsc_profile));

  XML_CHECK_START(CALIB_SENSOR_LSC_TAG_ID, CALIB_SENSOR_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LSC_PROFILE_NAME_TAG_ID)){
      char* value = Toupper(tag.Value());
      strncpy(lsc_profile.name, value, sizeof(lsc_profile.name));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LSC_PROFILE_RESOLUTION_TAG_ID)){
      const char* value = tag.Value();
      strncpy(lsc_profile.resolution, value, sizeof(lsc_profile.resolution));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LSC_PROFILE_ILLUMINATION_TAG_ID)){
      char* value = Toupper(tag.Value());
      strncpy(lsc_profile.illumination, value, sizeof(lsc_profile.illumination));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LSC_PROFILE_LSC_SECTORS_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &lsc_profile.LscSectors, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LSC_PROFILE_LSC_NO_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &lsc_profile.LscNo, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LSC_PROFILE_LSC_XO_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &lsc_profile.LscXo, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LSC_PROFILE_LSC_YO_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &lsc_profile.LscYo, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LSC_PROFILE_LSC_SECTOR_SIZE_X_TAG_ID)){
      int i = (sizeof(lsc_profile.LscXSizeTbl) / sizeof(lsc_profile.LscXSizeTbl[0]));
      int no = ParseUshortArray(tag.Value(), lsc_profile.LscXSizeTbl, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LSC_PROFILE_LSC_SECTOR_SIZE_Y_TAG_ID)){
      int i = (sizeof(lsc_profile.LscYSizeTbl) / sizeof(lsc_profile.LscYSizeTbl[0]));
      int no = ParseUshortArray(tag.Value(), lsc_profile.LscYSizeTbl, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LSC_PROFILE_LSC_VIGNETTING_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), (float*)(&lsc_profile.vignetting), 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_RED_TAG_ID)){
      int i = (sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_RED])
               / sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[0]));
      int no = ParseUshortArray(tag.Value(),
                                (lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff), i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_GREENR_TAG_ID)){
      int i = (sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR])
               / sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[0]));
      int no = ParseUshortArray(tag.Value(),
                                lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_GREENB_TAG_ID)){
      int i = (sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB])
               / sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[0]));
      int no = ParseUshortArray(tag.Value(),
                                (uint16_t*)(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff), i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_BLUE_TAG_ID)){
      int i = (sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE])
               / sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[0]));
      int no = ParseUshortArray(tag.Value(),
                                (uint16_t*)(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff), i);
      DCT_ASSERT((no == tag.Size()));
    } else {
#if 1
      LOGE( "%s(%d): parse error in LSC section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif

      return (false);
    }	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  RESULT result = CamCalibDbAddLscProfile(m_CalibDbHandle, &lsc_profile);
  DCT_ASSERT(result == RET_SUCCESS);

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}



/******************************************************************************
 * CalibDb::parseEntryCc
 *****************************************************************************/
bool CalibDb::parseEntryCc
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamCcProfile_t cc_profile;
  MEMSET(&cc_profile, 0, sizeof(cc_profile));

  XML_CHECK_START(CALIB_SENSOR_CC_TAG_ID, CALIB_SENSOR_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE( CALIB_SENSOR_CC_PROFILE_NAME_TAG_ID)){
      char* value = Toupper(tag.Value());
      strncpy(cc_profile.name, value, sizeof(cc_profile.name));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_CC_PROFILE_SATURATION_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &cc_profile.saturation, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_CC_PROFILE_CC_MATRIX_TAG_ID)){
      int i = (sizeof(cc_profile.CrossTalkCoeff) / sizeof(cc_profile.CrossTalkCoeff.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), cc_profile.CrossTalkCoeff.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_CC_PROFILE_CC_OFFSETS_TAG_ID)){
      int i = (sizeof(cc_profile.CrossTalkOffset) / sizeof(cc_profile.CrossTalkOffset.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), cc_profile.CrossTalkOffset.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_CC_PROFILE_WB_TAG_ID)){
      int i = (sizeof(cc_profile.ComponentGain) / sizeof(cc_profile.ComponentGain.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), cc_profile.ComponentGain.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else {
#if 1
      LOGE( "%s(%d): parse error in CC section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif

      return (false);
    }	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
  RESULT result = CamCalibDbAddCcProfile(m_CalibDbHandle, &cc_profile);
  DCT_ASSERT(result == RET_SUCCESS);

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}



/******************************************************************************
 * CalibDb::parseEntryBls
 *****************************************************************************/
bool CalibDb::parseEntryBls
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamBlsProfile_t bls_profile;
  MEMSET(&bls_profile, 0, sizeof(bls_profile));

  XML_CHECK_START(CALIB_SENSOR_BLS_TAG_ID, CALIB_SENSOR_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_BLS_NAME_TAG_ID)){
      char* value = Toupper(tag.Value());
      strncpy(bls_profile.name, value, sizeof(bls_profile.name));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_BLS_RESOLUTION_TAG_ID)){
      const char* value = tag.Value();
      strncpy(bls_profile.resolution, value, sizeof(bls_profile.resolution));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_BLS_DATA_TAG_ID)){
      int i = (sizeof(bls_profile.level) / sizeof(bls_profile.level.uCoeff[0]));
      int no = ParseUshortArray(tag.Value(), bls_profile.level.uCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else {
#if 1
      LOGE( "%s(%d): parse error in BLS section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif

      return (false);
    }	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
  RESULT result = CamCalibDbAddBlsProfile(m_CalibDbHandle, &bls_profile);
  DCT_ASSERT(result == RET_SUCCESS);

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}



/******************************************************************************
 * CalibDb::parseEntryCac
 *****************************************************************************/
bool CalibDb::parseEntryCac
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamCacProfile_t cac_profile;
  MEMSET(&cac_profile, 0, sizeof(cac_profile));

  XML_CHECK_START(CALIB_SENSOR_CAC_TAG_ID, CALIB_SENSOR_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif
    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_CAC_NAME_TAG_ID)){
      char* value = Toupper(tag.Value());
      strncpy(cac_profile.name, value, sizeof(cac_profile.name));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_CAC_RESOLUTION_TAG_ID)){
      const char* value = tag.Value();
      strncpy(cac_profile.resolution, value, sizeof(cac_profile.resolution));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SESNOR_CAC_X_NORMSHIFT_TAG_ID)){
      int no = ParseByteArray(tag.Value(), &cac_profile.x_ns, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SESNOR_CAC_X_NORMFACTOR_TAG_ID)){
      int no = ParseByteArray(tag.Value(), &cac_profile.x_nf, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SESNOR_CAC_Y_NORMSHIFT_TAG_ID)){
      int no = ParseByteArray(tag.Value(), &cac_profile.y_ns, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SESNOR_CAC_Y_NORMFACTOR_TAG_ID)){
      int no = ParseByteArray(tag.Value(), &cac_profile.y_nf, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SESNOR_CAC_X_OFFSET_TAG_ID)){
      int no = ParseShortArray(tag.Value(), &cac_profile.hCenterOffset, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SESNOR_CAC_Y_OFFSET_TAG_ID)){
      int no = ParseShortArray(tag.Value(), &cac_profile.vCenterOffset, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SESNOR_CAC_RED_PARAMETERS_TAG_ID)){
      int i = (sizeof(cac_profile.Red) / sizeof(cac_profile.Red.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), cac_profile.Red.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SESNOR_CAC_BLUE_PARAMETERS_TAG_ID)){
      int i = (sizeof(cac_profile.Blue) / sizeof(cac_profile.Blue.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), cac_profile.Blue.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else {
#if 1
      LOGE( "%s(%d): parse error in CAC section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif

      return (false);
    }	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
  RESULT result = CamCalibDbAddCacProfile(m_CalibDbHandle, &cac_profile);
  DCT_ASSERT(result == RET_SUCCESS);

#ifdef DEBUG_LOG
  redirectOut << __FUNCTION__ << " (exit)" << std::endl;
#endif

  return (true);
}

bool CalibDb::parseEntryFilterDemosiacTH
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	float* afGain   = NULL;
    int n_gains	  = 0;
    float* afThlevel	 = NULL;
    int n_Thlevels 	 = 0;
    int index = 0;
	  
	CamFilterProfile_t* pFilter = (CamFilterProfile_t*)param;
    if (NULL == pFilter) {
		LOGD("%s(%d):  null pointer (exit)\n", __FUNCTION__, __LINE__);
      	return false;
    }

  	XML_CHECK_START(CALIB_SENSOR_DPF_FILT_DEMOSAIC_TH_CONF_TAG_ID, CALIB_SENSOR_DPF_FILTERSETTING_TAG_ID);
  
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif		
		
		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILT_DEMOSAIC_TH_GAIN_TAG_ID)){
    	  if (!afGain) {
    		n_gains = tag.Size();
    		afGain	= (float*)malloc((n_gains * sizeof(float)));
    		MEMSET(afGain, 0, (n_gains * sizeof(float)));
    	  }

    	  int no = ParseFloatArray(tag.Value(), afGain, n_gains);
    	  DCT_ASSERT((no == n_gains));
    	} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILT_DEMOSAIC_TH_LEVEL_TAG_ID)){
    	  if (!afThlevel) {
    		n_Thlevels = tag.Size();
    		afThlevel = (float*)malloc((n_Thlevels * sizeof(float)));
    		MEMSET(afThlevel, 0, (n_Thlevels * sizeof(float)));
    	  }

    	  int no = ParseFloatArray(tag.Value(), afThlevel, n_Thlevels);
    	  DCT_ASSERT((no == n_Thlevels));
    	} else {		
#if 1
          LOGE( "%s(%d): parse error in filter demosaicTH section (unknow tag: %s)\n", 
          __FUNCTION__,__LINE__,tagname.c_str());
#endif
		}		
        pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
	
    DCT_ASSERT((n_gains == n_Thlevels));
    pFilter->DemosaicThCurve.ArraySize	  = n_gains;
    pFilter->DemosaicThCurve.pSensorGain	  = afGain;
    pFilter->DemosaicThCurve.pThlevel = (uint8_t*)malloc((n_Thlevels * sizeof(uint8_t)));
    for (index = 0; index < pFilter->DemosaicThCurve.ArraySize; index++) {
  		pFilter->DemosaicThCurve.pThlevel[index] = (uint8_t)((int)afThlevel[index]);
    }
    free(afThlevel);
	  
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}


bool CalibDb::parseEntryFilterSharpLevel
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	float* afGain   = NULL;
    int n_gains	  = 0;
    float* afSlevel	 = NULL;
    int n_Slevels 	 = 0;
    int index = 0;
	  
	CamFilterProfile_t* pFilter = (CamFilterProfile_t*)param;
    if (NULL == pFilter) {
		LOGD("%s(%d):  null pointer (exit)\n", __FUNCTION__, __LINE__);
      	return false;
    }

  	XML_CHECK_START(CALIB_SENSOR_DPF_SHARPENINGLEVEL_TAG_ID, CALIB_SENSOR_DPF_FILTERSETTING_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif
		
		
		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_SHARPENINGLEVEL_GAINS_TAG_ID)){
		  if (!afGain) {
			n_gains = tag.Size();
			afGain	= (float*)malloc((n_gains * sizeof(float)));
			MEMSET(afGain, 0, (n_gains * sizeof(float)));
		  }

		  int no = ParseFloatArray(tag.Value(), afGain, n_gains);
		  DCT_ASSERT((no == n_gains));
		} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_SHARPENINGLEVEL_SLEVEL_TAG_ID)){
		  if (!afSlevel) {
			n_Slevels = tag.Size();
			afSlevel = (float*)malloc((n_Slevels * sizeof(float)));
			MEMSET(afSlevel, 0, (n_Slevels * sizeof(float)));
		  }

		  int no = ParseFloatArray(tag.Value(), afSlevel, n_Slevels);
		  DCT_ASSERT((no == n_Slevels));
		} else {		
#if 1
          LOGE( "%s(%d): parse error in filter SharpLevel section (unknow tag: %s)\n", 
          __FUNCTION__,__LINE__,tagname.c_str());
#endif
		}
		
        pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
	
    DCT_ASSERT((n_gains == n_Slevels));
    pFilter->SharpeningLevelCurve.ArraySize	  = n_gains;
    pFilter->SharpeningLevelCurve.pSensorGain	  = afGain;
    pFilter->SharpeningLevelCurve.pSlevel =
    	(CamerIcIspFltSharpeningLevel_t*)malloc((n_Slevels * sizeof(CamerIcIspFltSharpeningLevel_t)));
    for (index = 0; index < pFilter->SharpeningLevelCurve.ArraySize; index++) {
  		pFilter->SharpeningLevelCurve.pSlevel[index] = (CamerIcIspFltSharpeningLevel_t)((int)afSlevel[index] + 1);
    }
    free(afSlevel);
	  
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}



bool CalibDb::parseEntryFilterDenoiseLevel
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	float* afGain   = NULL;
    int n_gains	  = 0;
    float* afDlevel	 = NULL;
    int n_Dlevels 	 = 0;
    int index = 0;
	  
	CamFilterProfile_t* pFilter = (CamFilterProfile_t*)param;
    if (NULL == pFilter) {
		LOGE("%s(%d):  null pointer (exit)\n", __FUNCTION__, __LINE__);
      	return false;
    }

  	XML_CHECK_START(CALIB_SENSOR_DPF_DENOISELEVEL_TAG_ID, CALIB_SENSOR_DPF_FILTERSETTING_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif
		
		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_DENOISELEVEL_GAINS_TAG_ID)){
		  if (!afGain) {
			n_gains = tag.Size();
			afGain	= (float*)malloc((n_gains * sizeof(float)));
			MEMSET(afGain, 0, (n_gains * sizeof(float)));
		  }

		  int no = ParseFloatArray(tag.Value(), afGain, n_gains);
		  DCT_ASSERT((no == n_gains));
		} else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_DENOISELEVEL_DLEVEL_TAG_ID)){
		  if (!afDlevel) {
			n_Dlevels = tag.Size();
			afDlevel = (float*)malloc((n_Dlevels * sizeof(float)));
			MEMSET(afDlevel, 0, (n_Dlevels * sizeof(float)));
		  }

		  int no = ParseFloatArray(tag.Value(), afDlevel, n_Dlevels);
		  DCT_ASSERT((no == n_Dlevels));
		} else {		
#if 1
          LOGE( "%s(%d): parse error in filter DenoiseLevel section (unknow tag: %s)\n", 
          __FUNCTION__,__LINE__,tagname.c_str());
#endif
		}		
        pchild = pchild->NextSibling();
	}

	 XML_CHECK_END();
	 
	 DCT_ASSERT((n_gains == n_Dlevels));
     pFilter->DenoiseLevelCurve.ArraySize	   = n_gains;
     pFilter->DenoiseLevelCurve.pSensorGain    = afGain;
     pFilter->DenoiseLevelCurve.pDlevel = (CamerIcIspFltDeNoiseLevel_t*)malloc((n_Dlevels * sizeof(CamerIcIspFltDeNoiseLevel_t)));
   
     for (index = 0; index < pFilter->DenoiseLevelCurve.ArraySize; index++) {
   		pFilter->DenoiseLevelCurve.pDlevel[index] = (CamerIcIspFltDeNoiseLevel_t)((int)afDlevel[index] + 1);
     }
   
     free(afDlevel);
	  
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}


bool CalibDb::parseEntryFilterRegConfig
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	
	CamFilterProfile_t* pFilter = (CamFilterProfile_t*)param;
    if (NULL == pFilter) {
    	LOGE("%s(%d):  null pointer (exit)\n", __FUNCTION__, __LINE__);
      	return false;
    }

  	XML_CHECK_START(CALIB_SENSOR_DPF_FILT_LEVEL_REG_CONF_TAG_ID, CALIB_SENSOR_DPF_FILTERSETTING_TAG_ID);
  	
	const XMLNode* pchild = pelement->FirstChild();
	
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILT_LEVEL_REG_CONF_ENABLE_TAG_ID))
		{
			int no = ParseUcharArray(tag.Value(), &pFilter->FiltLevelRegConf.FiltLevelRegConfEnable, 1);
			DCT_ASSERT((no == 1));
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILT_LEVEL_TAG_ID))
		{
			uint8_t* p_FiltLevel = NULL;
			if (!p_FiltLevel) {
				p_FiltLevel  = (uint8_t*)malloc((tag.Size() * sizeof(uint8_t)));
				MEMSET(p_FiltLevel, 0, (tag.Size() * sizeof(uint8_t)));
			}
			int no = ParseUcharArray(tag.Value(), p_FiltLevel, tag.Size());
			DCT_ASSERT((no == tag.Size()));
			pFilter->FiltLevelRegConf.ArraySize = no;
			pFilter->FiltLevelRegConf.p_FiltLevel = p_FiltLevel;

			for(int i=0; i < no; i++){
				if(p_FiltLevel[i] > 10){
					LOGD( "%s(%d): filter level only support 10 level, %d > 10 is invalid \n", 
						__FUNCTION__, __LINE__, p_FiltLevel[i]);
					p_FiltLevel[i] = 10;
				}
			}
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILT_GRN_STAGE1_TAG_ID))
		{
			uint8_t* p_grn_stage1 = NULL;
			if (!p_grn_stage1) {
				p_grn_stage1  = (uint8_t*)malloc((tag.Size() * sizeof(uint8_t)));
				MEMSET(p_grn_stage1, 0, (tag.Size() * sizeof(uint8_t)));
			}
			int no = ParseUcharArray(tag.Value(), p_grn_stage1, tag.Size());
			DCT_ASSERT((no == tag.Size()));
			DCT_ASSERT((no == pFilter->FiltLevelRegConf.ArraySize));
			pFilter->FiltLevelRegConf.p_grn_stage1 = p_grn_stage1;
			for(int i=0; i < no; i++){
				if(p_grn_stage1[i] > 8){
					LOGD( "%s(%d): grn_stage1 only support 0-8, %d > 8 is invalid \n", 
						__FUNCTION__, __LINE__, p_grn_stage1[i]);
					p_grn_stage1[i] = 8;
				}
			}
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILT_CHR_H_MODE_TAG_ID))
		{
			uint8_t* p_chr_h_mode = NULL;
			if (!p_chr_h_mode) {
				p_chr_h_mode  = (uint8_t*)malloc((tag.Size() * sizeof(uint8_t)));
				MEMSET(p_chr_h_mode, 0, (tag.Size() * sizeof(uint8_t)));
			}
			int no = ParseUcharArray(tag.Value(), p_chr_h_mode, tag.Size());
			DCT_ASSERT((no == tag.Size()));
			DCT_ASSERT((no == pFilter->FiltLevelRegConf.ArraySize));
			pFilter->FiltLevelRegConf.p_chr_h_mode = p_chr_h_mode;
			for(int i=0; i < no; i++){
				if(p_chr_h_mode[i] > 3){
					LOGD( "%s(%d): chr_h_mode only support 0-3, %d > 3 is invalid \n", 
						__FUNCTION__, __LINE__, p_chr_h_mode[i]);
					p_chr_h_mode[i] = 3;
				}
			}
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILT_CHR_V_MODE_TAG_ID))
		{
			uint8_t* p_chr_v_mode = NULL;
			if (!p_chr_v_mode) {
				p_chr_v_mode  = (uint8_t*)malloc((tag.Size() * sizeof(uint8_t)));
				MEMSET(p_chr_v_mode, 0, (tag.Size() * sizeof(uint8_t)));
			}
			int no = ParseUcharArray(tag.Value(), p_chr_v_mode, tag.Size());
			DCT_ASSERT((no == tag.Size()));
			DCT_ASSERT((no == pFilter->FiltLevelRegConf.ArraySize));
			pFilter->FiltLevelRegConf.p_chr_v_mode = p_chr_v_mode;
			for(int i=0; i < no; i++){
				if(p_chr_v_mode[i] > 3){
					LOGD( "%s(%d): chr_h_mode only support 0-3, %d > 3 is invalid \n", 
						__FUNCTION__, __LINE__, p_chr_v_mode[i]);
					p_chr_v_mode[i] = 3;
				}
			}
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILT_THRESH_BL0_TAG_ID))
		{
			uint32_t* p_thresh_bl0 = NULL;
			if (!p_thresh_bl0) {
				p_thresh_bl0  = (uint32_t*)malloc((tag.Size() * sizeof(uint32_t)));
				MEMSET(p_thresh_bl0, 0, (tag.Size() * sizeof(uint32_t)));
			}
			int no = ParseUintArray(tag.Value(), p_thresh_bl0, tag.Size());
			DCT_ASSERT((no == tag.Size()));
			DCT_ASSERT((no == pFilter->FiltLevelRegConf.ArraySize));
			pFilter->FiltLevelRegConf.p_thresh_bl0 = p_thresh_bl0;
			for(int i=0; i < no; i++){
				if(p_thresh_bl0[i] > 1023){
					LOGD( "%s(%d): thresh_bl0 only support 0-1023, %d > 1023 is invalid \n", 
						__FUNCTION__, __LINE__, p_thresh_bl0[i]);
					p_thresh_bl0[i] = 1023;
				}
			}
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILT_THRESH_BL1_TAG_ID))
		{
			uint32_t* p_thresh_bl1 = NULL;
			if (!p_thresh_bl1) {
				p_thresh_bl1  = (uint32_t*)malloc((tag.Size() * sizeof(uint32_t)));
				MEMSET(p_thresh_bl1, 0, (tag.Size() * sizeof(uint32_t)));
			}
			int no = ParseUintArray(tag.Value(), p_thresh_bl1, tag.Size());
			DCT_ASSERT((no == tag.Size()));
			DCT_ASSERT((no == pFilter->FiltLevelRegConf.ArraySize));
			pFilter->FiltLevelRegConf.p_thresh_bl1 = p_thresh_bl1;
			for(int i=0; i < no; i++){
				if(p_thresh_bl1[i] > 1023){
					LOGD( "%s(%d): thresh_bl1 only support 0-1023, %d > 1023 is invalid \n", 
						__FUNCTION__, __LINE__, p_thresh_bl1[i]);
					p_thresh_bl1[i] = 1023;
				}
			}
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILT_FAC_BL0_TAG_ID))
		{
			uint32_t* p_fac_bl0 = NULL;
			if (!p_fac_bl0) {
				p_fac_bl0  = (uint32_t*)malloc((tag.Size() * sizeof(uint32_t)));
				MEMSET(p_fac_bl0, 0, (tag.Size() * sizeof(uint32_t)));
			}
			int no = ParseUintArray(tag.Value(), p_fac_bl0, tag.Size());
			DCT_ASSERT((no == tag.Size()));
			DCT_ASSERT((no == pFilter->FiltLevelRegConf.ArraySize));
			pFilter->FiltLevelRegConf.p_fac_bl0 = p_fac_bl0;
			for(int i=0; i < no; i++){
				if(p_fac_bl0[i] > 63){
					LOGD( "%s(%d): fac_bl0 only support 0-63, %d > 63 is invalid \n", 
						__FUNCTION__, __LINE__, p_fac_bl0[i]);
					p_fac_bl0[i] = 63;
				}
			}
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILT_FAC_BL1_TAG_ID))
		{
			uint32_t* p_fac_bl1 = NULL;
			if (!p_fac_bl1) {
				p_fac_bl1  = (uint32_t*)malloc((tag.Size() * sizeof(uint32_t)));
				MEMSET(p_fac_bl1, 0, (tag.Size() * sizeof(uint32_t)));
			}
			int no = ParseUintArray(tag.Value(), p_fac_bl1, tag.Size());
			DCT_ASSERT((no == tag.Size()));
			DCT_ASSERT((no == pFilter->FiltLevelRegConf.ArraySize));
			pFilter->FiltLevelRegConf.p_fac_bl1 = p_fac_bl1;
			for(int i=0; i < no; i++){
				if(p_fac_bl1[i] > 63){
					LOGD( "%s(%d): fac_bl1 only support 0-63, %d > 63 is invalid \n", 
						__FUNCTION__, __LINE__, p_fac_bl1[i]);
					p_fac_bl1[i] = 63;
				}
			}
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILT_THRESH_SH0_TAG_ID))
		{
			uint32_t* p_thresh_sh0 = NULL;
			if (!p_thresh_sh0) {
				p_thresh_sh0  = (uint32_t*)malloc((tag.Size() * sizeof(uint32_t)));
				MEMSET(p_thresh_sh0, 0, (tag.Size() * sizeof(uint32_t)));
			}
			int no = ParseUintArray(tag.Value(), p_thresh_sh0, tag.Size());
			DCT_ASSERT((no == tag.Size()));
			DCT_ASSERT((no == pFilter->FiltLevelRegConf.ArraySize));
			pFilter->FiltLevelRegConf.p_thresh_sh0 = p_thresh_sh0;
			for(int i=0; i < no; i++){
				if(p_thresh_sh0[i] > 1023){
					LOGD( "%s(%d): thresh_sh0 only support 0-1023, %d > 1023 is invalid \n", 
						__FUNCTION__, __LINE__, p_thresh_sh0[i]);
					p_thresh_sh0[i] = 1023;
				}
			}
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILT_THRESH_SH1_TAG_ID))
		{
			uint32_t* p_thresh_sh1 = NULL;
			if (!p_thresh_sh1) {
				p_thresh_sh1  = (uint32_t*)malloc((tag.Size() * sizeof(uint32_t)));
				MEMSET(p_thresh_sh1, 0, (tag.Size() * sizeof(uint32_t)));
			}
			int no = ParseUintArray(tag.Value(), p_thresh_sh1, tag.Size());
			DCT_ASSERT((no == tag.Size()));
			DCT_ASSERT((no == pFilter->FiltLevelRegConf.ArraySize));
			pFilter->FiltLevelRegConf.p_thresh_sh1 = p_thresh_sh1;
			for(int i=0; i < no; i++){
				if(p_thresh_sh1[i] > 1023){
					LOGD( "%s(%d): thresh_sh2 only support 0-1023, %d > 1023 is invalid \n", 
						__FUNCTION__, __LINE__, p_thresh_sh1[i]);
					p_thresh_sh1[i] = 1023;
				}
			}
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILT_FAC_SH0_TAG_ID))
		{
			uint32_t* p_fac_sh0 = NULL;
			if (!p_fac_sh0) {
				p_fac_sh0  = (uint32_t*)malloc((tag.Size() * sizeof(uint32_t)));
				MEMSET(p_fac_sh0, 0, (tag.Size() * sizeof(uint32_t)));
			}
			int no = ParseUintArray(tag.Value(), p_fac_sh0, tag.Size());
			DCT_ASSERT((no == tag.Size()));
			DCT_ASSERT((no == pFilter->FiltLevelRegConf.ArraySize));
			pFilter->FiltLevelRegConf.p_fac_sh0 = p_fac_sh0;
			for(int i=0; i < no; i++){
				if(p_fac_sh0[i] > 63){
					LOGD( "%s(%d): fac_sh0 only support 0-63, %d > 63 is invalid \n", 
						__FUNCTION__, __LINE__, p_fac_sh0[i]);
					p_fac_sh0[i] = 63;
				}
			}
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILT_FAC_SH1_TAG_ID))
		{
			uint32_t* p_fac_sh1 = NULL;
			if (!p_fac_sh1) {
				p_fac_sh1  = (uint32_t*)malloc((tag.Size() * sizeof(uint32_t)));
				MEMSET(p_fac_sh1, 0, (tag.Size() * sizeof(uint32_t)));
			}
			int no = ParseUintArray(tag.Value(), p_fac_sh1, tag.Size());
			DCT_ASSERT((no == tag.Size()));
			DCT_ASSERT((no == pFilter->FiltLevelRegConf.ArraySize));
			pFilter->FiltLevelRegConf.p_fac_sh1 = p_fac_sh1;
			for(int i=0; i < no; i++){
				if(p_fac_sh1[i] > 63){
					LOGD( "%s(%d): fac_sh1 only support 0-63, %d > 63 is invalid \n", 
						__FUNCTION__, __LINE__, p_fac_sh1[i]);
					p_fac_sh1[i] = 63;
				}
			}
		}else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILT_FAC_MID_TAG_ID))
		{
			uint32_t* p_fac_mid = NULL;
			if (!p_fac_mid) {
				p_fac_mid  = (uint32_t*)malloc((tag.Size() * sizeof(uint32_t)));
				MEMSET(p_fac_mid, 0, (tag.Size() * sizeof(uint32_t)));
			}
			int no = ParseUintArray(tag.Value(), p_fac_mid, tag.Size());
			DCT_ASSERT((no == tag.Size()));
			DCT_ASSERT((no == pFilter->FiltLevelRegConf.ArraySize));
			pFilter->FiltLevelRegConf.p_fac_mid = p_fac_mid;
			for(int i=0; i < no; i++){
				if(p_fac_mid[i] > 63){
					LOGD( "%s(%d): fac_mid only support 0-63, %d > 63 is invalid \n", 
						__FUNCTION__, __LINE__, p_fac_mid[i]);
					p_fac_mid[i] = 63;
				}
			}
		}else {
#if 1
        	LOGE( "%s(%d): parse error in filter level reg conf section (unknow tag: %s)\n", 
        	__FUNCTION__,__LINE__,tagname.c_str());
#endif
		}		
		pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
	
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}


bool CalibDb::parseEntryFilter
(
    const XMLElement* plement,
    void* param
)
{
#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif
  CamDpfProfile_t *pdpf_profile = (CamDpfProfile_t *)param;
  if(NULL == pdpf_profile){
  	LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__, __LINE__);
    return false;
  }

  CamFilterProfile_t* pFilter = (CamFilterProfile_t*)malloc(sizeof(CamFilterProfile_t));
  if (NULL == pFilter) {
  	LOGE("%s(%d): malloc fail(exit)\n", __FUNCTION__, __LINE__);
    return false;
  }
  MEMSET(pFilter, 0, sizeof(*pFilter));

  XML_CHECK_START(CALIB_SENSOR_DPF_FILTERSETTING_TAG_ID, CALIB_SENSOR_DPF_TAG_ID);
	
  const XMLNode* pchild = plement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

	if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILTERSETTING_NAME_TAG_ID)){
	  char* value = Toupper(tag.Value());
      strncpy(pFilter->name, value, sizeof(pFilter->name));
	}
    else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILT_LEVEL_REG_CONF_TAG_ID)){
  	  if (!parseEntryFilterRegConfig(pchild->ToElement(), pFilter)){
		LOGE( "%s(%d): parse error in FilterRegConfig section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
	}
    else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILTERENABLE_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &pFilter->FilterEnable, 1);
      DCT_ASSERT((no == tag.Size()));

    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_DENOISELEVEL_TAG_ID)){
      if (!parseEntryFilterDenoiseLevel(pchild->ToElement(), pFilter)){
		LOGE( "%s(%d): parse error in FilterDenoiseLevel section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
    }
    else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_SHARPENINGLEVEL_TAG_ID)){
      if (!parseEntryFilterSharpLevel(pchild->ToElement(), pFilter)){
		LOGE( "%s(%d): parse error in FilterSharpLevel section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
    }
	else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILT_DEMOSAIC_TH_CONF_TAG_ID)){
      if (!parseEntryFilterDemosiacTH(pchild->ToElement(), pFilter)){
		LOGE( "%s(%d): parse error in FilterDemosiacTH section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
	}else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_DEMOSAIC_LP_CONF_TAG_ID)){
	  if(!parseEntryDemosaicLPConfig(pchild->ToElement(), pFilter)){
		LOGE( "%s(%d): parse error in Filter DemosiacLP section\n",__FUNCTION__,__LINE__);
		return (false);
	  }
	}else{
#if 1
      LOGE( "%s(%d): parse error in filter section (unknow tag: %s)\n", 
        	__FUNCTION__,__LINE__,tagname.c_str());
#endif
	}	
	pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
  if (pFilter) {
	 ListPrepareItem(pFilter);
	 ListAddTail(&pdpf_profile->FilterList, pFilter);
  }

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
 }



bool CalibDb::parseEntryNew3DnrYnr
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif
	  
	CamNewDsp3DNRProfile_t* pNew3Dnr= (CamNewDsp3DNRProfile_t*)param;
    if (NULL == pNew3Dnr) {
		LOGE("%s(%d): invalia pointer (exit)\n", __FUNCTION__, __LINE__);
      	return false;
    }

	int nYnrTimeLevel = 0;
    int nYnrSpaceLevel = 0;

  	XML_CHECK_START(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_YNR_SETTING_TAG_ID, CALIB_SENSOR_NEW_DSP_3DNR_SETTING_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif
				
		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_YNR_ENABLE_TAG_ID)){
          int no = ParseUintArray(tag.Value(), &pNew3Dnr->ynr.enable_ynr, 1);
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_TNR_ENABLE_TAG_ID)){
          int no = ParseUintArray(tag.Value(), &pNew3Dnr->ynr.enable_tnr, 1);
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_IIR_ENABLE_TAG_ID)){
          int no = ParseUintArray(tag.Value(), &pNew3Dnr->ynr.enable_iir, 1);
          DCT_ASSERT((no == tag.Size()));
        }
		else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_YNR_TIME_LEVEL_TAG_ID)){
          pNew3Dnr->ynr.pynr_time_weight_level = (unsigned int*)malloc((tag.Size() * sizeof(unsigned int)));
		  if(!pNew3Dnr->ynr.pynr_time_weight_level){
		      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
			  return false;
	  	  }else{
			  int no = ParseUintArray(tag.Value(), pNew3Dnr->ynr.pynr_time_weight_level, tag.Size());
	          DCT_ASSERT((no == tag.Size()));
			  nYnrTimeLevel = no;
	  	  }
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_YNR_SPACE_LEVEL_TAG_ID)){
          pNew3Dnr->ynr.pynr_spat_weight_level = (unsigned int*)malloc((tag.Size() * sizeof(unsigned int)));
		  if(!pNew3Dnr->ynr.pynr_spat_weight_level){
		      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
			  return false;
	  	  }else{
			  int no = ParseUintArray(tag.Value(), pNew3Dnr->ynr.pynr_spat_weight_level, tag.Size());
	          DCT_ASSERT((no == tag.Size()));
			  nYnrSpaceLevel = no;
	  	  }
        }else {		
#if 1
          LOGE( "%s(%d): parse error in new 3dnr ynr section (unknow tag: %s)\n", 
          __FUNCTION__,__LINE__,tagname.c_str());
#endif
		}		
        pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
	
    DCT_ASSERT((nYnrSpaceLevel == nYnrTimeLevel));
   
	pNew3Dnr->ArraySize = nYnrSpaceLevel;
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}

bool CalibDb::parseEntryNew3DnrUVnr
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	CamNewDsp3DNRProfile_t* pNew3Dnr= (CamNewDsp3DNRProfile_t*)param;
    if (NULL == pNew3Dnr) {
		LOGE("%s(%d):  null pointer (exit)\n", __FUNCTION__, __LINE__);
      	return false;
    }

	int nUVnrLevel = 0;

  	XML_CHECK_START(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_UVNR_SETTING_TAG_ID, CALIB_SENSOR_NEW_DSP_3DNR_SETTING_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif
				
		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_UVNR_ENABLE_TAG_ID)){
          int no = ParseUintArray(tag.Value(), &pNew3Dnr->uvnr.enable_uvnr, 1);
          DCT_ASSERT((no == tag.Size()));
        }
		else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_UVNR_LEVEL_TAG_ID)){
          pNew3Dnr->uvnr.puvnr_weight_level = (unsigned int*)malloc((tag.Size() * sizeof(unsigned int)));
		  if(!pNew3Dnr->uvnr.puvnr_weight_level){
		      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
			  return false;
	  	  }else{
			  int no = ParseUintArray(tag.Value(), pNew3Dnr->uvnr.puvnr_weight_level, tag.Size());
	          DCT_ASSERT((no == tag.Size()));
			  nUVnrLevel = no;
	  	  }
        }else {		
#if 1
          LOGE( "%s(%d): parse error in new 3dnr ynr section (unknow tag: %s)\n", 
          __FUNCTION__,__LINE__,tagname.c_str());
#endif
		}		
        pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
   
	pNew3Dnr->ArraySize = nUVnrLevel;
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}


bool CalibDb::parseEntryNew3DnrSharp
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	  
	CamNewDsp3DNRProfile_t* pNew3Dnr= (CamNewDsp3DNRProfile_t*)param;
    if (NULL == pNew3Dnr) {
		LOGE("%s(%d):  null pointer (exit)\n", __FUNCTION__, __LINE__);
      	return false;
    }

	int nSharpLevel = 0;
  
  	XML_CHECK_START(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_SHARP_SETTING_TAG_ID, CALIB_SENSOR_NEW_DSP_3DNR_SETTING_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif
				
		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_SHARP_ENABLE_TAG_ID)){
          int no = ParseUintArray(tag.Value(), &pNew3Dnr->sharp.enable_sharp, 1);
          DCT_ASSERT((no == tag.Size()));
        }
		else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_SHARP_LEVEL_TAG_ID)){
          pNew3Dnr->sharp.psharp_weight_level= (unsigned int*)malloc((tag.Size() * sizeof(unsigned int)));
		  if(!pNew3Dnr->sharp.psharp_weight_level){
		      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
			  return false;
	  	  }else{
			  int no = ParseUintArray(tag.Value(), pNew3Dnr->sharp.psharp_weight_level, tag.Size());
	          DCT_ASSERT((no == tag.Size()));
			  nSharpLevel = no;
	  	  }
        }else {		
#if 1
          LOGE( "%s(%d): parse error in new 3dnr ynr section (unknow tag: %s)\n", 
          __FUNCTION__,__LINE__,tagname.c_str());
#endif
		}		
        pchild = pchild->NextSibling();
	}

	XML_CHECK_END();
   
	pNew3Dnr->ArraySize = nSharpLevel;
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}


bool CalibDb::parseEntryNew3DNR
(
    const XMLElement* plement,
    void* param
)
{
#ifdef DEBUG_LOG
  redirectOut << __FUNCTION__ << " (enter)" << std::endl;
#endif

  CamDpfProfile_t *pdpf_profile = (CamDpfProfile_t *)param;
  if(NULL == pdpf_profile){
	redirectOut << __FUNCTION__ << " Invalid pointer (exit)" << std::endl;
    return false;
  }

  int nGainLevel = 0;
  int nYnrTimeLevel = 0;
  int nYnrSpaceLevel = 0;
  int nUVnrLevel = 0;
  int nSharpLevel = 0;

  CamNewDsp3DNRProfile_t* pNewDsp3DNRProfile = (CamNewDsp3DNRProfile_t*)malloc(sizeof(CamNewDsp3DNRProfile_t));
  if (!pNewDsp3DNRProfile) {
  	redirectOut << __FUNCTION__ << " malloc fail (exit)" << std::endl;
    return false;
  }
  MEMSET(pNewDsp3DNRProfile, 0, sizeof(*pNewDsp3DNRProfile));

  XML_CHECK_START(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_TAG_ID, CALIB_SENSOR_DPF_TAG_ID);
  
  const XMLNode* pchild = plement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_NAME_TAG_ID)){
	  char* value = Toupper(tag.Value());
      strncpy(pNewDsp3DNRProfile->name, value, sizeof(pNewDsp3DNRProfile->name));
    }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_3DNR_ENABLE_TAG_ID)){
      int no = ParseUintArray(tag.Value(), &pNewDsp3DNRProfile->enable_3dnr, 1);
      DCT_ASSERT((no == tag.Size()));
    }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_DPC_ENABLE_TAG_ID)){
      int no = ParseUintArray(tag.Value(), &pNewDsp3DNRProfile->enable_dpc, 1);
      DCT_ASSERT((no == tag.Size()));
    }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_GAIN_LEVEL_TAG_ID)){
      pNewDsp3DNRProfile->pgain_Level = (float*)malloc((tag.Size() * sizeof(float)));
	  if(!pNewDsp3DNRProfile->pgain_Level){
	      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
  	  }else{
		  int no = ParseFloatArray(tag.Value(), pNewDsp3DNRProfile->pgain_Level, tag.Size());
	      DCT_ASSERT((no == tag.Size()));
		  nGainLevel = no;
  	  }
    }else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_YNR_SETTING_TAG_ID)){
	   if(!parseEntryNew3DnrYnr(pchild->ToElement(), pNewDsp3DNRProfile)){
			LOGE("%s(%d): parse in new 3dnr ynr section\n", __FUNCTION__, __LINE__);
			return false;
	   }
	   nYnrTimeLevel = pNewDsp3DNRProfile->ArraySize;
	   nYnrSpaceLevel = pNewDsp3DNRProfile->ArraySize;
	 }
	 else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_UVNR_SETTING_TAG_ID)){
       if(!parseEntryNew3DnrUVnr(pchild->ToElement(), pNewDsp3DNRProfile)){
			LOGE("%s(%d): parse in new 3dnr uvnr section\n", __FUNCTION__, __LINE__);
			return false;
	   }
       nUVnrLevel = pNewDsp3DNRProfile->ArraySize;
	 }
	 else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_SHARP_SETTING_TAG_ID)){
       if(!parseEntryNew3DnrSharp(pchild->ToElement(), pNewDsp3DNRProfile)){
			LOGE("%s(%d): parse in new 3dnr sharp section\n", __FUNCTION__, __LINE__);
			return false;
	   }
	   nSharpLevel = pNewDsp3DNRProfile->ArraySize;
	 }
	 else{
#if 1
	   LOGD( "%s(%d): parse error in new 3dnr ynr section (unknow tag: %s)\n", 
	   __FUNCTION__,__LINE__,tagname.c_str());
#endif

	 }	 
	 pchild = pchild->NextSibling();

   }

   XML_CHECK_END();
   
   DCT_ASSERT(nGainLevel == nYnrTimeLevel);
   DCT_ASSERT(nGainLevel == nYnrSpaceLevel);
   DCT_ASSERT(nGainLevel == nUVnrLevel);
   DCT_ASSERT(nGainLevel == nSharpLevel);

   pNewDsp3DNRProfile->ArraySize = nGainLevel;

   if (pNewDsp3DNRProfile) {
	 ListPrepareItem(pNewDsp3DNRProfile);
	 ListAddTail(&pdpf_profile->newDsp3DNRProfileList, pNewDsp3DNRProfile);
   }

#ifdef DEBUG_LOG
  redirectOut << __FUNCTION__ << " (exit)" << std::endl;
#endif

  return (true);

}



bool CalibDb::parseEntry3DnrLevel
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	int nLumaSpNrLevel = 0;
    int nLumaTeNrLevel = 0;
    int nChrmSpNrLevel = 0;
    int nChrmTeNrLevel = 0;
    int nShpLevel = 0;
	  
	CamDsp3DNRSettingProfile_t* p3Dnr= (CamDsp3DNRSettingProfile_t*)param;
    if (NULL == p3Dnr) {
		LOGE("%s(%d):  null pointer (exit)\n", __FUNCTION__, __LINE__);
      	return false;
    }
  
  	XML_CHECK_START(CALIB_SENSOR_DSP_3DNR_SETTING_LEVEL_SETTING_TAG_ID, CALIB_SENSOR_DSP_3DNR_SETTING_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif
				
		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_SP_NR_EN_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &p3Dnr->sDefaultLevelSetting.luma_sp_nr_en, 1);
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_SP_NR_LEVEL_TAG_ID)){
          p3Dnr->sDefaultLevelSetting.pluma_sp_nr_level = (unsigned char*)malloc((tag.Size() * sizeof(unsigned char)));
		  if(!p3Dnr->sDefaultLevelSetting.pluma_sp_nr_level){
		      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
			  return false;
	  	  }else{
			  int no = ParseUcharArray(tag.Value(), p3Dnr->sDefaultLevelSetting.pluma_sp_nr_level, tag.Size());
	          DCT_ASSERT((no == tag.Size()));
			  nLumaSpNrLevel = no;
	  	  }
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_TE_NR_EN_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &p3Dnr->sDefaultLevelSetting.luma_te_nr_en, 1);
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_TE_NR_LEVEL_TAG_ID)){
          p3Dnr->sDefaultLevelSetting.pluma_te_nr_level = (unsigned char*)malloc((tag.Size() * sizeof(unsigned char)));
		  if(!p3Dnr->sDefaultLevelSetting.pluma_te_nr_level){
		      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
			  return false;
	  	  }else{
 			  int no = ParseUcharArray(tag.Value(), p3Dnr->sDefaultLevelSetting.pluma_te_nr_level, tag.Size());
 	          DCT_ASSERT((no == tag.Size()));
			  nLumaTeNrLevel = no;
	  	  }
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_SP_NR_EN_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &p3Dnr->sDefaultLevelSetting.chrm_sp_nr_en, 1);
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_SP_NR_LEVEL_TAG_ID)){
          p3Dnr->sDefaultLevelSetting.pchrm_sp_nr_level = (unsigned char*)malloc((tag.Size() * sizeof(unsigned char)));
		  if(!p3Dnr->sDefaultLevelSetting.pchrm_sp_nr_level){
		      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
			  return false;
	  	  }else{
			  int no = ParseUcharArray(tag.Value(), p3Dnr->sDefaultLevelSetting.pchrm_sp_nr_level, tag.Size());
			  DCT_ASSERT((no == tag.Size()));
			  nChrmSpNrLevel = no;
	  	  }
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_TE_NR_EN_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &p3Dnr->sDefaultLevelSetting.chrm_te_nr_en, 1);
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_TE_NR_LEVEL_TAG_ID)){
          p3Dnr->sDefaultLevelSetting.pchrm_te_nr_level = (unsigned char*)malloc((tag.Size() * sizeof(unsigned char)));
		  if(!p3Dnr->sDefaultLevelSetting.pchrm_te_nr_level){
		      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
			  return false;
	  	  }else{
			  int no = ParseUcharArray(tag.Value(), p3Dnr->sDefaultLevelSetting.pchrm_te_nr_level, tag.Size());
			  DCT_ASSERT((no == tag.Size()));
			  nChrmTeNrLevel = no;
	  	  }
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_SHP_EN_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &p3Dnr->sDefaultLevelSetting.shp_en, 1);
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_SHP_LEVEL_TAG_ID)){
          p3Dnr->sDefaultLevelSetting.pshp_level = (unsigned char*)malloc((tag.Size() * sizeof(unsigned char)));
		  if(!p3Dnr->sDefaultLevelSetting.pshp_level){
		      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
			  return false;
	  	  }else{
			  int no = ParseUcharArray(tag.Value(), p3Dnr->sDefaultLevelSetting.pshp_level, tag.Size());
			  DCT_ASSERT((no == tag.Size()));
			  nShpLevel = no;
	  	  }
        }else {		
#if 1
          LOGD( "%s(%d): parse error in new 3dnr ynr section (unknow tag: %s)\n", 
          __FUNCTION__,__LINE__,tagname.c_str());
#endif
		}		
        pchild = pchild->NextSibling();
    }

	XML_CHECK_END();

    DCT_ASSERT(nLumaSpNrLevel == nLumaTeNrLevel);
    DCT_ASSERT(nLumaSpNrLevel == nChrmSpNrLevel);
    DCT_ASSERT(nLumaSpNrLevel == nChrmTeNrLevel);
    DCT_ASSERT(nLumaSpNrLevel == nShpLevel);

	p3Dnr->ArraySize = nLumaSpNrLevel;

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}




bool CalibDb::parseEntry3DnrLuma
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	int nLumaSpRad = 0;
    int nLumaTeMaxBiNum = 0;
	int nLumaWeight[CAM_CALIBDB_3DNR_WEIGHT_NUM];
	MEMSET(nLumaWeight, 0x00, CAM_CALIBDB_3DNR_WEIGHT_NUM*sizeof(int));
	
	CamDsp3DNRSettingProfile_t* p3Dnr= (CamDsp3DNRSettingProfile_t*)param;
    if (NULL == p3Dnr) {
		LOGE("%s(%d):  invalid pointer (exit)\n", __FUNCTION__, __LINE__);
      	return false;
    }
  
  	XML_CHECK_START(CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_SETTING_TAG_ID, CALIB_SENSOR_DSP_3DNR_SETTING_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif
				
		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_DEFAULT_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &p3Dnr->sLumaSetting.luma_default, 1);
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_SP_RAD_TAG_ID)){
          p3Dnr->sLumaSetting.pluma_sp_rad = (unsigned char*)malloc((tag.Size() * sizeof(unsigned char)));
		  if(!p3Dnr->sLumaSetting.pluma_sp_rad){
		      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
			  return false;
	  	  }else{
			  int no = ParseUcharArray(tag.Value(), p3Dnr->sLumaSetting.pluma_sp_rad, tag.Size());
			  DCT_ASSERT((no == tag.Size()));
			  nLumaSpRad = no;
	  	  }
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_TE_MAX_BI_NUM_TAG_ID)){
          p3Dnr->sLumaSetting.pluma_te_max_bi_num = (unsigned char*)malloc((tag.Size() * sizeof(unsigned char)));
		  if(!p3Dnr->sLumaSetting.pluma_te_max_bi_num){
		      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
			  return false;
	  	  }else{
			  int no = ParseUcharArray(tag.Value(), p3Dnr->sLumaSetting.pluma_te_max_bi_num, tag.Size());
			  DCT_ASSERT((no == tag.Size()));
			  nLumaTeMaxBiNum = no;
	  	  }
        }else if( (tagname.find(CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_WEIGHT_TAG) != std::string::npos)){
			//parse weight num
			int find = strlen(CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_WEIGHT_TAG);
			std::string weight_col_string = tagname.substr(find, 1);
			std::string weight_row_string = tagname.substr(find+1, 1);
			int weight_col = atoi(weight_col_string.c_str());
			int weight_row = atoi(weight_row_string.c_str());
			int idx = weight_col*CAM_DSP_3DNR_SETTING_WEIGHT_ROW_NUM + weight_row;

			if(idx >= 0 && idx < CAM_CALIBDB_3DNR_WEIGHT_NUM){
				if(!p3Dnr->sLumaSetting.pluma_weight[idx])
					p3Dnr->sLumaSetting.pluma_weight[idx]= (uint8_t*)malloc((tag.Size() * sizeof(uint8_t)));
				if(!p3Dnr->sLumaSetting.pluma_weight[idx]){
				  LOGE("%s(%d): malloc fail, col:%d row:%d \n" ,
				  	__FUNCTION__, __LINE__, weight_col, weight_row);
			  	  return false;
   		  	    }else{
   				  int no = ParseUcharArray(tag.Value(), p3Dnr->sLumaSetting.pluma_weight[idx], tag.Size());
   				  DCT_ASSERT((no == tag.Size()));
				  nLumaWeight[idx] = no;
   		  	    }
			}
        }else {		
#if 1
          LOGE( "%s(%d): parse error in new 3dnr ynr section (unknow tag: %s)\n", 
          __FUNCTION__,__LINE__,tagname.c_str());
#endif
		}		
        pchild = pchild->NextSibling();
    }

	XML_CHECK_END();

    DCT_ASSERT(nLumaSpRad == nLumaTeMaxBiNum);
	for(int i=0; i<CAM_CALIBDB_3DNR_WEIGHT_NUM; i++){
		DCT_ASSERT(nLumaSpRad == nLumaWeight[i]);
	}
	p3Dnr->ArraySize = nLumaSpRad;

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}


bool CalibDb::parseEntry3DnrChrm
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	int nChrmSpRad = 0;
    int nChrmTeMaxBiNum = 0;
    int nChrmWeight[CAM_CALIBDB_3DNR_WEIGHT_NUM];
	MEMSET(nChrmWeight, 0x00, CAM_CALIBDB_3DNR_WEIGHT_NUM*sizeof(int));
	  
	CamDsp3DNRSettingProfile_t* p3Dnr= (CamDsp3DNRSettingProfile_t*)param;
    if (NULL == p3Dnr) {
		LOGE("%s(%d): inavlid pointer (exit)\n", __FUNCTION__, __LINE__);
      	return false;
    }
  
  	XML_CHECK_START(CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_SETTING_TAG_ID, CALIB_SENSOR_DSP_3DNR_SETTING_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif
				
		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_DEFAULT_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &p3Dnr->sChrmSetting.chrm_default, 1);
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_SP_RAD_TAG_ID)){
          p3Dnr->sChrmSetting.pchrm_sp_rad = (unsigned char*)malloc((tag.Size() * sizeof(unsigned char)));
		  if(!p3Dnr->sChrmSetting.pchrm_sp_rad){
		      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
			  return false;
	  	  }else{
			  int no = ParseUcharArray(tag.Value(), p3Dnr->sChrmSetting.pchrm_sp_rad, tag.Size());
			  DCT_ASSERT((no == tag.Size()));
			  nChrmSpRad = no;
	  	  }
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_TE_MAX_BI_NUM_TAG_ID)){
          p3Dnr->sChrmSetting.pchrm_te_max_bi_num = (unsigned char*)malloc((tag.Size() * sizeof(unsigned char)));
		  if(!p3Dnr->sChrmSetting.pchrm_te_max_bi_num){
		      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
			  return false;
	  	  }else{
			  int no = ParseUcharArray(tag.Value(), p3Dnr->sChrmSetting.pchrm_te_max_bi_num, tag.Size());
			  DCT_ASSERT((no == tag.Size()));
			  nChrmTeMaxBiNum = no;
	  	  }
        }
		else if( (tagname.find(CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_WEIGHT_TAG) != std::string::npos)){
			//parse weight num
			int find = strlen(CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_WEIGHT_TAG);
			std::string weight_col_string = tagname.substr(find, 1);
			std::string weight_row_string = tagname.substr(find+1, 1);
			int weight_col = atoi(weight_col_string.c_str());
			int weight_row = atoi(weight_row_string.c_str());
			int idx = weight_col*CAM_DSP_3DNR_SETTING_WEIGHT_ROW_NUM + weight_row;

			if(idx >= 0 && idx < CAM_CALIBDB_3DNR_WEIGHT_NUM){
				if(!p3Dnr->sChrmSetting.pchrm_weight[idx])
					p3Dnr->sChrmSetting.pchrm_weight[idx]= (uint8_t*)malloc((tag.Size() * sizeof(uint8_t)));
				if(!p3Dnr->sChrmSetting.pchrm_weight[idx]){
			      LOGE("%s(%d): malloc fail, col:%d row:%d \n" ,
				  	__FUNCTION__, __LINE__, weight_col, weight_row);
			  	  return false;
   		  	    }else{
   				  int no = ParseUcharArray(tag.Value(), p3Dnr->sChrmSetting.pchrm_weight[idx], tag.Size());
   				  DCT_ASSERT((no == tag.Size()));
				  nChrmWeight[idx] = no;
   		  	    }
			}
        }else {		
#if 1
          LOGE( "%s(%d): parse error in new 3dnr ynr section (unknow tag: %s)\n", 
          __FUNCTION__,__LINE__,tagname.c_str());
#endif
		}		
        pchild = pchild->NextSibling();
    }

	XML_CHECK_END();

    DCT_ASSERT(nChrmSpRad == nChrmTeMaxBiNum);
	for(int i=0; i<CAM_CALIBDB_3DNR_WEIGHT_NUM; i++){
		DCT_ASSERT(nChrmSpRad == nChrmWeight[i]);
	}
	
	p3Dnr->ArraySize = nChrmSpRad;

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}

bool CalibDb::parseEntry3DnrSharp
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	int nSrcShpDiv = 0;
  	int nSrcShpThr = 0;
  	int nSrcShpL = 0;
  	int nSrcShpC = 0;
  	int nSrcShpWeight[CAM_CALIBDB_3DNR_WEIGHT_NUM];
	MEMSET(nSrcShpWeight, 0x00, CAM_CALIBDB_3DNR_WEIGHT_NUM*sizeof(int));
	  
	CamDsp3DNRSettingProfile_t* p3Dnr= (CamDsp3DNRSettingProfile_t*)param;
    if (NULL == p3Dnr) {
		LOGE("%s(%d):  invalid pointer (exit)\n", __FUNCTION__, __LINE__);
      	return false;
    }
  
  	XML_CHECK_START(CALIB_SENSOR_DSP_3DNR_SETTING_SHP_SETTING_TAG_ID, CALIB_SENSOR_DSP_3DNR_SETTING_TAG_ID);
	
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif
				
		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_SHP_DEFAULT_TAG_ID)){
          int no = ParseUcharArray(tag.Value(), &p3Dnr->sSharpSetting.shp_default, 1);
          DCT_ASSERT((no == tag.Size()));
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_THR_TAG_ID)){
          p3Dnr->sSharpSetting.psrc_shp_thr = (unsigned char*)malloc((tag.Size() * sizeof(unsigned char)));
		  if(!p3Dnr->sSharpSetting.psrc_shp_thr){
		      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
			  return false;
	  	  }else{
	          int no = ParseUcharArray(tag.Value(), p3Dnr->sSharpSetting.psrc_shp_thr, tag.Size());
	          DCT_ASSERT((no == tag.Size()));
			  nSrcShpThr = no;
	  	  }
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_DIV_TAG_ID)){
          p3Dnr->sSharpSetting.psrc_shp_div = (unsigned char*)malloc((tag.Size() * sizeof(unsigned char)));
		  if(!p3Dnr->sSharpSetting.psrc_shp_div){
		      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
			  return false;
	  	  }else{
	          int no = ParseUcharArray(tag.Value(), p3Dnr->sSharpSetting.psrc_shp_div, tag.Size());
	          DCT_ASSERT((no == tag.Size()));
			  nSrcShpDiv = no;
	  	  }
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_L_TAG_ID)){
          p3Dnr->sSharpSetting.psrc_shp_l = (unsigned char*)malloc((tag.Size() * sizeof(unsigned char)));
		  if(!p3Dnr->sSharpSetting.psrc_shp_l){
		      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
			  return false;
	  	  }else{
	          int no = ParseUcharArray(tag.Value(), p3Dnr->sSharpSetting.psrc_shp_l, tag.Size());
	          DCT_ASSERT((no == tag.Size()));
			  nSrcShpL = no;
	  	  }
        }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_C_TAG_ID)){
          p3Dnr->sSharpSetting.psrc_shp_c = (unsigned char*)malloc((tag.Size() * sizeof(unsigned char)));
		  if(!p3Dnr->sSharpSetting.psrc_shp_c){
		      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
			  return false;
	  	  }else{
	          int no = ParseUcharArray(tag.Value(), p3Dnr->sSharpSetting.psrc_shp_c, tag.Size());
	          DCT_ASSERT((no == tag.Size()));
			  nSrcShpC = no;
	  	  }
        }
		else if( (tagname.find(CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_WEIGHT_TAG) != std::string::npos)){
			//parse weight num
			int find = strlen(CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_WEIGHT_TAG);
			std::string weight_col_string = tagname.substr(find, 1);
			std::string weight_row_string = tagname.substr(find+1, 1);
			int weight_col = atoi(weight_col_string.c_str());
			int weight_row = atoi(weight_row_string.c_str());
			int idx = weight_col*CAM_DSP_3DNR_SETTING_WEIGHT_ROW_NUM + weight_row;

			if(idx >= 0 && idx < CAM_CALIBDB_3DNR_WEIGHT_NUM){
				if(!p3Dnr->sSharpSetting.psrc_shp_weight[idx])
					p3Dnr->sSharpSetting.psrc_shp_weight[idx]= (int8_t*)malloc((tag.Size() * sizeof(int8_t)));
				if(!p3Dnr->sSharpSetting.psrc_shp_weight[idx]){
			      LOGE("%s(%d): malloc fail, col:%d row:%d \n" ,
				  	__FUNCTION__, __LINE__, weight_col, weight_row);
			  	  return false;
   		  	    }else{
   				  int no = ParseCharArray(tag.Value(), p3Dnr->sSharpSetting.psrc_shp_weight[idx], tag.Size());
   				  DCT_ASSERT((no == tag.Size()));
				  nSrcShpWeight[idx] = no;
   		  	    }
			}
        
        }else {		
#if 1
          LOGE( "%s(%d): parse error in new 3dnr ynr section (unknow tag: %s)\n", 
          __FUNCTION__,__LINE__,tagname.c_str());
#endif
		}		
        pchild = pchild->NextSibling();
    }

	XML_CHECK_END();

    DCT_ASSERT(nSrcShpThr == nSrcShpThr);
	DCT_ASSERT(nSrcShpThr == nSrcShpL);
	DCT_ASSERT(nSrcShpThr == nSrcShpC);

	for(int i=0; i<CAM_CALIBDB_3DNR_WEIGHT_NUM; i++){
		DCT_ASSERT(nSrcShpThr == nSrcShpWeight[i]);
 	}

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}


bool CalibDb::parseEntry3DNR
(
    const XMLElement* plement,
    void* param
)
{
#ifdef DEBUG_LOG
  redirectOut << __FUNCTION__ << " (enter)" << std::endl;
#endif
  CamDpfProfile_t *pdpf_profile = (CamDpfProfile_t *)param;
  if(NULL == pdpf_profile){
	redirectOut << __FUNCTION__ << " Invalid pointer (exit)" << std::endl;
    return false;
  }

  int nGainLevel = 0;
  int nNoiseCoefNum = 0;
  int nNoiseCoefDen = 0;
  int nLumaSpNrLevel = 0;
  int nLumaSpRad = 0;
  int nChrmSpRad = 0;
  int nSrcShpDiv = 0;

  CamDsp3DNRSettingProfile_t* pDsp3DNRProfile = (CamDsp3DNRSettingProfile_t*)malloc(sizeof(CamDsp3DNRSettingProfile_t));
  if (!pDsp3DNRProfile) {
  	LOGE("%s(%d):  invalid pointer (exit)\n", __FUNCTION__, __LINE__);
    return false;
  }
  MEMSET(pDsp3DNRProfile, 0, sizeof(*pDsp3DNRProfile));

  XML_CHECK_START(CALIB_SENSOR_DSP_3DNR_SETTING_TAG_ID, CALIB_SENSOR_DPF_TAG_ID);
  
  const XMLNode* pchild = plement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_NAME_TAG_ID)){
	  char* value = Toupper(tag.Value());
      strncpy(pDsp3DNRProfile->name, value, sizeof(pDsp3DNRProfile->name));
    }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_ENABLE_TAG_ID)){
      int no = ParseUcharArray(tag.Value(), &pDsp3DNRProfile->Enable, 1);
      DCT_ASSERT((no == tag.Size()));
    }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_GAIN_LEVEL_TAG_ID)){
      pDsp3DNRProfile->pgain_Level = (float*)malloc((tag.Size() * sizeof(float)));
	  if(!pDsp3DNRProfile->pgain_Level){
	      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
  	  }else{
		  int no = ParseFloatArray(tag.Value(), pDsp3DNRProfile->pgain_Level, tag.Size());
	      DCT_ASSERT((no == tag.Size()));
		  nGainLevel = no;
  	  }
    }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_NOISE_COEF_NUMERATOR_TAG_ID)){
      pDsp3DNRProfile->pnoise_coef_numerator = (uint16_t*)malloc((tag.Size() * sizeof(uint16_t)));
	  if(!pDsp3DNRProfile->pnoise_coef_numerator){
	      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
  	  }else{
		  int no = ParseUshortArray(tag.Value(), pDsp3DNRProfile->pnoise_coef_numerator, tag.Size());
	      DCT_ASSERT((no == tag.Size()));
		  nNoiseCoefNum = no;
  	  }
    }else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_NOISE_COEF_DENOMINATOR_TAG_ID)){
      pDsp3DNRProfile->pnoise_coef_denominator= (uint16_t*)malloc((tag.Size() * sizeof(uint16_t)));
	  if(!pDsp3DNRProfile->pnoise_coef_denominator){
	      LOGE("%s(%d): malloc fail:" ,__FUNCTION__, __LINE__ );
  	  }else{
		  int no = ParseUshortArray(tag.Value(), pDsp3DNRProfile->pnoise_coef_denominator, tag.Size());
	      DCT_ASSERT((no == tag.Size()));
		  nNoiseCoefDen = no;
  	  }
    }
	else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_LEVEL_SETTING_TAG_ID)){
      if(!parseEntry3DnrLevel(pchild->ToElement(), pDsp3DNRProfile)){
   		LOGE("%s(%d): parse in 3dnr level section\n", __FUNCTION__, __LINE__);
   		return false;
      }
      nLumaSpNrLevel = pDsp3DNRProfile->ArraySize;
	}
	else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_SETTING_TAG_ID)){
      if(!parseEntry3DnrLuma(pchild->ToElement(), pDsp3DNRProfile)){
   		LOGE("%s(%d): parse in 3dnr luma section\n", __FUNCTION__, __LINE__);
   		return false;
      }
      nLumaSpRad = pDsp3DNRProfile->ArraySize;
	}
	else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_SETTING_TAG_ID)){
      if(!parseEntry3DnrChrm(pchild->ToElement(), pDsp3DNRProfile)){
   		LOGE("%s(%d): parse in 3dnr chrm section\n", __FUNCTION__, __LINE__);
   		return false;
      }
      nChrmSpRad = pDsp3DNRProfile->ArraySize;  	
 	}
	else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_SHP_SETTING_TAG_ID)){
      if(!parseEntry3DnrSharp(pchild->ToElement(), pDsp3DNRProfile)){
   		LOGE("%s(%d): parse in 3dnr sharp section\n", __FUNCTION__, __LINE__);
   		return false;
      }
      nSrcShpDiv = pDsp3DNRProfile->ArraySize;	  
	}else{
#if 1
    LOGE( "%s(%d): parse error in filter section (unknow tag: %s)\n", 
        	__FUNCTION__,__LINE__,tagname.c_str());
#endif
	}	
	pchild = pchild->NextSibling();
 }

 XML_CHECK_END();

 DCT_ASSERT(nGainLevel == nNoiseCoefNum);
 DCT_ASSERT(nGainLevel == nNoiseCoefDen);
 DCT_ASSERT(nGainLevel == nLumaSpNrLevel);
 DCT_ASSERT(nGainLevel == nLumaSpRad);
 DCT_ASSERT(nGainLevel == nChrmSpRad);
 DCT_ASSERT(nGainLevel == nSrcShpDiv);


 pDsp3DNRProfile->ArraySize = nGainLevel;

 if (pDsp3DNRProfile) {
	 ListPrepareItem(pDsp3DNRProfile);
	 ListAddTail(&pdpf_profile->Dsp3DNRSettingProfileList, pDsp3DNRProfile);
 }

#ifdef DEBUG_LOG
  redirectOut << __FUNCTION__ << " (exit)" << std::endl;
#endif

  return (true);
}

bool CalibDb::parseEntryDemosaicLPConfig
(
    const XMLElement* plement,
    void* param
)
{
	int n_gains_level = 0;
	int n_thH_divided0 = 0;
	int n_thH_divided1 = 0;
	int n_thH_divided2 = 0;
	int n_thH_divided3 = 0;
	int n_thH_divided4 = 0;
	int n_thCSC_divided0 = 0;
	int n_thCSC_divided1 = 0;
	int n_thCSC_divided2 = 0;
	int n_thCSC_divided3 = 0;
	int n_thCSC_divided4 = 0;
	int n_diff_divided0 = 0;
	int n_diff_divided1 = 0;
	int n_diff_divided2 = 0;
	int n_diff_divided3 = 0;
	int n_diff_divided4 = 0;
	int n_varTh_divided0 = 0;
	int n_varTh_divided1 = 0;
	int n_varTh_divided2 = 0;
	int n_varTh_divided3 = 0;
	int n_varTh_divided4 = 0;
	int n_thgrad_r_fct = 0;
	int n_thdiff_r_fct = 0;
	int n_thvar_r_fct = 0;
	int n_thgrad_b_fct = 0;
	int n_thdiff_b_fct = 0;
	int n_thvar_b_fct = 0;
	int n_similarity_th = 0;
	int n_th_grad = 0;
	int n_th_diff = 0;
	int n_th_var = 0;
	int n_th_csc = 0;
	int n_flat_level_sel = 0;
	int n_pattern_level_sel = 0;
	int n_edge_level_sel = 0;

#ifdef DEBUG_LOG
	redirectOut << __FUNCTION__ << " (enter)" << std::endl;
#endif

	CamFilterProfile_t *pfilter_profile = (CamFilterProfile_t *)param;
	if(NULL == pfilter_profile){
		LOGE("%s(%d):  invalid pointer (exit)\n", __FUNCTION__, __LINE__);
		return false;
	}

	CamDemosaicLpProfile_t* pDemosaicLpConf = &pfilter_profile->DemosaicLpConf;
	if (!pDemosaicLpConf) {
		LOGE("%s(%d):  invalid pointer (exit)\n", __FUNCTION__, __LINE__);
		return false;
	}
	MEMSET(pDemosaicLpConf, 0, sizeof(*pDemosaicLpConf));

    XML_CHECK_START(CALIB_SENSOR_DPF_DEMOSAIC_LP_CONF_TAG_ID, CALIB_SENSOR_DPF_FILTERSETTING_TAG_ID);
  
	const XMLNode* pchild = plement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

        if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_EN_TAG_ID))
        {
            int no = ParseUcharArray(tag.Value(),&pDemosaicLpConf->lp_en,1);
            DCT_ASSERT((no == 1));
        } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_USE_OLD_VERSION_TAG_ID))
        {
            int no = ParseUcharArray(tag.Value(),&pDemosaicLpConf->use_old_lp,1);
            DCT_ASSERT((no == 1));
        } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_RB_FILTER_EN_TAG_ID))
		{
            int no = ParseUcharArray(tag.Value(),&pDemosaicLpConf->rb_filter_en,1);
            DCT_ASSERT((no == 1));
        } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_HP_FILTER_EN_TAG_ID))
        {
            int no = ParseUcharArray(tag.Value(),&pDemosaicLpConf->hp_filter_en,1);
            DCT_ASSERT((no == 1));
        } else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_LU_DIVIDED_TAG_ID))
        {
            uint8_t* p_lu_divided=NULL;
            p_lu_divided = (uint8_t*)malloc(tag.Size() * sizeof(uint8_t));
            DCT_ASSERT(p_lu_divided != NULL);
            MEMSET(p_lu_divided, 0, (tag.Size() * sizeof(uint8_t)));

            int no = ParseUcharArray(tag.Value(), p_lu_divided, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            pDemosaicLpConf->lu_divided = p_lu_divided;
            pDemosaicLpConf->lu_divided_ArraySize = no;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_GAINSARRAY_TAG_ID))
        {
            float* p_gainsArray=NULL;
            p_gainsArray = (float*)malloc(tag.Size() * sizeof(float));
            DCT_ASSERT(p_gainsArray != NULL);
            MEMSET(p_gainsArray,0,(tag.Size() * sizeof(float)));

            int no = ParseFloatArray(tag.Value(), p_gainsArray, tag.Size());
            DCT_ASSERT((no == tag.Size()));
			n_gains_level = no;
            pDemosaicLpConf->gainsArray = p_gainsArray;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_THH_DIVIDED0_TAG_ID))
        {
            float* thH_divided0=NULL;
            thH_divided0 = (float*)malloc(tag.Size() * sizeof(float));
            DCT_ASSERT(thH_divided0 != NULL);
            MEMSET(thH_divided0,0,(tag.Size() * sizeof(float)));
            int no = ParseFloatArray(tag.Value(), thH_divided0, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_thH_divided0 = no;
            pDemosaicLpConf->thH_divided0=thH_divided0;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_THH_DIVIDED1_TAG_ID))
        {
            float* thH_divided1=NULL;
            thH_divided1 = (float*)malloc(tag.Size() * sizeof(float));
            DCT_ASSERT(thH_divided1 != NULL);
            MEMSET(thH_divided1,0,(tag.Size() * sizeof(float)));

            int no = ParseFloatArray(tag.Value(), thH_divided1, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_thH_divided1 = no;
            pDemosaicLpConf->thH_divided1=thH_divided1;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_THH_DIVIDED2_TAG_ID))
        {
            float* thH_divided2=NULL;
            thH_divided2 = (float*)malloc(tag.Size() * sizeof(float));
            DCT_ASSERT(thH_divided2 != NULL);
            MEMSET(thH_divided2,0,(tag.Size()*sizeof(float)));

            int no = ParseFloatArray(tag.Value(), thH_divided2, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_thH_divided2 = no;
            pDemosaicLpConf->thH_divided2 = thH_divided2;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_THH_DIVIDED3_TAG_ID))
        {
            float* thH_divided3=NULL;
            thH_divided3 = (float*)malloc(tag.Size() * sizeof(float));
            DCT_ASSERT(thH_divided3 != NULL);
            MEMSET(thH_divided3,0,(tag.Size() * sizeof(float)));

            int no = ParseFloatArray(tag.Value(), thH_divided3, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_thH_divided3 = no;
            pDemosaicLpConf->thH_divided3 = thH_divided3;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_THH_DIVIDED4_TAG_ID))
        {
            float* thH_divided4=NULL;
            thH_divided4 = (float*)malloc(tag.Size() * sizeof(float));
            DCT_ASSERT(thH_divided4 != NULL);
            MEMSET(thH_divided4, 0, (tag.Size()*sizeof(float)));

            int no = ParseFloatArray(tag.Value(), thH_divided4, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_thH_divided4 = no;
            pDemosaicLpConf->thH_divided4 = thH_divided4;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_THCSC_DIVIDED0_TAG_ID))
        {
            float* thCSC_divided0=NULL;
            thCSC_divided0 = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT(thCSC_divided0 != NULL);
            MEMSET(thCSC_divided0,0,(tag.Size()*sizeof(float)));

            int no = ParseFloatArray(tag.Value(), thCSC_divided0, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_thCSC_divided0 = no;
            pDemosaicLpConf->thCSC_divided0 = thCSC_divided0;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_THCSC_DIVIDED1_TAG_ID))
        {
            float* thCSC_divided1=NULL;
            thCSC_divided1 = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT(thCSC_divided1 != NULL);
            MEMSET(thCSC_divided1,0,(tag.Size()*sizeof(float)));
            int no = ParseFloatArray(tag.Value(), thCSC_divided1, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_thCSC_divided1 = no;
            pDemosaicLpConf->thCSC_divided1 = thCSC_divided1;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_THCSC_DIVIDED2_TAG_ID))
        {
            float* thCSC_divided2=NULL;
            thCSC_divided2 = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT(thCSC_divided2 != NULL);
            MEMSET(thCSC_divided2,0,(tag.Size()*sizeof(float)));
            int no = ParseFloatArray(tag.Value(), thCSC_divided2, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_thCSC_divided2 = no;
            pDemosaicLpConf->thCSC_divided2 = thCSC_divided2;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_THCSC_DIVIDED3_TAG_ID))
        {
            float* thCSC_divided3=NULL;
            thCSC_divided3 = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT(thCSC_divided3 != NULL);
            MEMSET(thCSC_divided3,0,(tag.Size()*sizeof(float)));

            int no = ParseFloatArray(tag.Value(), thCSC_divided3, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_thCSC_divided3 = no;
            pDemosaicLpConf->thCSC_divided3=thCSC_divided3;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_THCSC_DIVIDED4_TAG_ID))
        {
            float* thCSC_divided4=NULL;
            thCSC_divided4 = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT(thCSC_divided4 != NULL);
            MEMSET(thCSC_divided4,0,(tag.Size()*sizeof(float)));

            int no = ParseFloatArray(tag.Value(), thCSC_divided4, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_thCSC_divided4 = no;
            pDemosaicLpConf->thCSC_divided4=thCSC_divided4;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_DIFF_DIVIDED0_TAG_ID))
        {
            float* diff_divided0=NULL;
            diff_divided0 = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT(diff_divided0 != NULL);
            MEMSET(diff_divided0,0,(tag.Size()*sizeof(float)));

            int no = ParseFloatArray(tag.Value(), diff_divided0, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_diff_divided0 = no;
            pDemosaicLpConf->diff_divided0 = diff_divided0;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_DIFF_DIVIDED1_TAG_ID))
        {
            float* diff_divided1=NULL;
            diff_divided1 = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT(diff_divided1 != NULL);
            MEMSET(diff_divided1,0,(tag.Size()*sizeof(float)));

            int no = ParseFloatArray(tag.Value(), diff_divided1, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_diff_divided1 = no;
            pDemosaicLpConf->diff_divided1 = diff_divided1;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_DIFF_DIVIDED2_TAG_ID))
        {
            float* diff_divided2=NULL;
            diff_divided2 = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT(diff_divided2 != NULL);
            MEMSET(diff_divided2,0,(tag.Size()*sizeof(float)));

            int no = ParseFloatArray(tag.Value(), diff_divided2, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_diff_divided2 = no;
            pDemosaicLpConf->diff_divided2 = diff_divided2;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_DIFF_DIVIDED3_TAG_ID))
        {
            float* diff_divided3=NULL;
            diff_divided3 = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT(diff_divided3 != NULL);
            MEMSET(diff_divided3,0,(tag.Size()*sizeof(float)));

            int no = ParseFloatArray(tag.Value(), diff_divided3, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_diff_divided3 = no;
            pDemosaicLpConf->diff_divided3 = diff_divided3;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_DIFF_DIVIDED4_TAG_ID))
        {
            float* diff_divided4=NULL;

            diff_divided4 = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((diff_divided4 != NULL));
            MEMSET(diff_divided4,0,(tag.Size()*sizeof(float)));

            int no = ParseFloatArray(tag.Value(), diff_divided4, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_diff_divided4 = no;
            pDemosaicLpConf->diff_divided4 = diff_divided4;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_VARTH_DIVIDED0_TAG_ID))
        {
            float* var_divided0=NULL;
            var_divided0 = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((var_divided0 != NULL));
            MEMSET(var_divided0,0,(tag.Size()*sizeof(float)));

            int no = ParseFloatArray(tag.Value(), var_divided0, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_varTh_divided0 = no;
            pDemosaicLpConf->varTh_divided0 = var_divided0;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_VARTH_DIVIDED1_TAG_ID))
        {
            float* var_divided1=NULL;
            var_divided1 = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((var_divided1 != NULL));
            MEMSET(var_divided1,0,(tag.Size()*sizeof(float)));

            int no = ParseFloatArray(tag.Value(), var_divided1, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_varTh_divided1 = no;
            pDemosaicLpConf->varTh_divided1 = var_divided1;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_VARTH_DIVIDED2_TAG_ID))
        {
            float* var_divided2=NULL;
            var_divided2 = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((var_divided2 != NULL));
            MEMSET(var_divided2,0,(tag.Size()*sizeof(float)));

            int no = ParseFloatArray(tag.Value(), var_divided2, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_varTh_divided2 = no;
            pDemosaicLpConf->varTh_divided2 = var_divided2;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_VARTH_DIVIDED3_TAG_ID))
        {
            float* var_divided3=NULL;
            var_divided3 = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((var_divided3 != NULL));
            MEMSET(var_divided3,0,(tag.Size()*sizeof(float)));
            int no = ParseFloatArray(tag.Value(), var_divided3, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_varTh_divided3 = no;
            pDemosaicLpConf->varTh_divided3 = var_divided3;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_VARTH_DIVIDED4_TAG_ID))
        {
            float* var_divided4=NULL;
            var_divided4 = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((var_divided4 != NULL));
            MEMSET(var_divided4,0,(tag.Size()*sizeof(float)));

            int no = ParseFloatArray(tag.Value(), var_divided4, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            n_varTh_divided4 = no;
            pDemosaicLpConf->varTh_divided4 = var_divided4;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_THGRAD_R_FCT_TAG_ID))     
		{
        	float* thgrad_r_fct = NULL;
            thgrad_r_fct = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((thgrad_r_fct != NULL));
            MEMSET(thgrad_r_fct, 0, (tag.Size()*sizeof(float)));
			
            int no = ParseFloatArray(tag.Value(), thgrad_r_fct, tag.Size());
            DCT_ASSERT((no == tag.Size()));
			n_thgrad_r_fct = no;
            pDemosaicLpConf->thgrad_r_fct = thgrad_r_fct;
    	}
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_THDIFF_R_FCT_TAG_ID))      
		{	
        	float* thdiff_r_fct = NULL;
            thdiff_r_fct = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((thdiff_r_fct != NULL));
            MEMSET(thdiff_r_fct, 0, (tag.Size()*sizeof(float)));
			
            int no = ParseFloatArray(tag.Value(), thdiff_r_fct, tag.Size());
            DCT_ASSERT((no == tag.Size()));
			n_thdiff_r_fct = no;
            pDemosaicLpConf->thdiff_r_fct = thdiff_r_fct;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_THVAR_R_FCT_TAG_ID))       
		{
        	float* thvar_r_fct = NULL;
            thvar_r_fct = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((thvar_r_fct != NULL));
            MEMSET(thvar_r_fct, 0, (tag.Size()*sizeof(float)));
			
            int no = ParseFloatArray(tag.Value(), thvar_r_fct, tag.Size());
            DCT_ASSERT((no == tag.Size()));
			n_thvar_r_fct = no;
            pDemosaicLpConf->thvar_r_fct = thvar_r_fct;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_THGRAD_B_FCT_TAG_ID))      
		{
        	float* thgrad_b_fct = NULL;
            thgrad_b_fct = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((thgrad_b_fct != NULL));
            MEMSET(thgrad_b_fct, 0, (tag.Size()*sizeof(float)));
			
            int no = ParseFloatArray(tag.Value(), thgrad_b_fct, tag.Size());
            DCT_ASSERT((no == tag.Size()));
			n_thgrad_b_fct = no;
            pDemosaicLpConf->thgrad_b_fct = thgrad_b_fct;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_THDIFF_B_FCT_TAG_ID))     
		{
        	float* thdiff_b_fct = NULL;
            thdiff_b_fct = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((thdiff_b_fct != NULL));
            MEMSET(thdiff_b_fct, 0, (tag.Size()*sizeof(float)));
			
            int no = ParseFloatArray(tag.Value(), thdiff_b_fct, tag.Size());
            DCT_ASSERT((no == tag.Size()));
			n_thdiff_b_fct = no;
            pDemosaicLpConf->thdiff_b_fct = thdiff_b_fct;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_THVAR_B_FCT_TAG_ID))      
		{
        	float* thvar_b_fct = NULL;
            thvar_b_fct = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((thvar_b_fct != NULL));
            MEMSET(thvar_b_fct, 0, (tag.Size()*sizeof(float)));
			
            int no = ParseFloatArray(tag.Value(), thvar_b_fct, tag.Size());
            DCT_ASSERT((no == tag.Size()));
			n_thvar_b_fct = no;
            pDemosaicLpConf->thvar_b_fct = thvar_b_fct;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_SIMILARITY_TH_TAG_ID))    
		{
        	float* similarity_th = NULL;
            similarity_th = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((similarity_th != NULL));
            MEMSET(similarity_th, 0, (tag.Size()*sizeof(float)));
			
            int no = ParseFloatArray(tag.Value(), similarity_th, tag.Size());
            DCT_ASSERT((no == tag.Size()));
			n_similarity_th = no;
            pDemosaicLpConf->similarity_th = similarity_th;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_TH_VAR_EN_TAG_ID))
        {
            int no = ParseUcharArray(tag.Value(),&pDemosaicLpConf->th_var_en,1);
            DCT_ASSERT((no == 1));
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_TH_CSC_EN_TAG_ID))
        {
            int no = ParseUcharArray(tag.Value(),&pDemosaicLpConf->th_csc_en,1);
            DCT_ASSERT((no == 1));
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_TH_DIFF_EN_TAG_ID))
        {
            int no = ParseUcharArray(tag.Value(),&pDemosaicLpConf->th_diff_en,1);
            DCT_ASSERT((no == 1));
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_TH_GRAD_EN_TAG_ID))
        {
            int no = ParseUcharArray(tag.Value(),&pDemosaicLpConf->th_grad_en,1);
            DCT_ASSERT((no == 1));
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_TH_GRAD_TAG_ID))
        {
        	float* th_grad = NULL;
            th_grad = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((th_grad != NULL));
            MEMSET(th_grad, 0, (tag.Size()*sizeof(float)));
			
            int no = ParseFloatArray(tag.Value(), th_grad, tag.Size());
            DCT_ASSERT((no == tag.Size()));
			n_th_grad = no;
            pDemosaicLpConf->th_grad = th_grad;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_TH_DIFF_TAG_ID))
        {
        	float* th_diff = NULL;
            th_diff = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((th_diff != NULL));
            MEMSET(th_diff, 0, (tag.Size()*sizeof(float)));
			
            int no = ParseFloatArray(tag.Value(), th_diff, tag.Size());
            DCT_ASSERT((no == tag.Size()));
			n_th_diff = no;
            pDemosaicLpConf->th_diff = th_diff;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_TH_CSC_TAG_ID))
        {
        	float* th_csc = NULL;
            th_csc = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((th_csc != NULL));
            MEMSET(th_csc, 0, (tag.Size()*sizeof(float)));
			
            int no = ParseFloatArray(tag.Value(), th_csc, tag.Size());
            DCT_ASSERT((no == tag.Size()));
			n_th_csc = no;
            pDemosaicLpConf->th_csc = th_csc;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_TH_VAR_TAG_ID))
        {
        	float* th_var = NULL;
            th_var = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((th_var != NULL));
            MEMSET(th_var, 0, (tag.Size()*sizeof(float)));
			
            int no = ParseFloatArray(tag.Value(), th_var, tag.Size());
            DCT_ASSERT((no == tag.Size()));
			n_th_var = no;
            pDemosaicLpConf->th_var = th_var;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_FLAT_LEVEL_SEL_TAG_ID))
        {
        	float* flat_level_sel = NULL;
            flat_level_sel = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((flat_level_sel != NULL));
            MEMSET(flat_level_sel, 0, (tag.Size()*sizeof(float)));
			
            int no = ParseFloatArray(tag.Value(), flat_level_sel, tag.Size());
            DCT_ASSERT((no == tag.Size()));
			n_flat_level_sel = no;
            pDemosaicLpConf->flat_level_sel = flat_level_sel;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_PATTERN_LEVEL_SEL_TAG_ID))
        {
        	float* pattern_level_sel = NULL;
            pattern_level_sel = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((pattern_level_sel != NULL));
            MEMSET(pattern_level_sel, 0, (tag.Size()*sizeof(float)));
			
            int no = ParseFloatArray(tag.Value(), pattern_level_sel, tag.Size());
            DCT_ASSERT((no == tag.Size()));
			n_pattern_level_sel = no;
            pDemosaicLpConf->pattern_level_sel = pattern_level_sel;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_LP_EDGE_LEVEL_SEL_TAG_ID))
        {
        	float* edge_level_sel = NULL;
            edge_level_sel = (float*)malloc(tag.Size()*sizeof(float));
            DCT_ASSERT((edge_level_sel != NULL));
            MEMSET(edge_level_sel, 0, (tag.Size()*sizeof(float)));
			
            int no = ParseFloatArray(tag.Value(), edge_level_sel, tag.Size());
            DCT_ASSERT((no == tag.Size()));
			n_edge_level_sel = no;
            pDemosaicLpConf->edge_level_sel = edge_level_sel;
        }
        else
        {
#if 1
			LOGE( "%s(%d): parse error in demosaiclp section (unknow tag: %s)\n", 
				__FUNCTION__,__LINE__,tagname.c_str());
#endif
        	return (false);
        }		
        pchild = pchild->NextSibling();
   }
   XML_CHECK_END();
   
   DCT_ASSERT(n_gains_level == n_thH_divided0);
   DCT_ASSERT(n_gains_level == n_thH_divided1);
   DCT_ASSERT(n_gains_level == n_thH_divided2);
   DCT_ASSERT(n_gains_level == n_thH_divided3);
   DCT_ASSERT(n_gains_level == n_thH_divided4);
   DCT_ASSERT(n_gains_level == n_thCSC_divided0);
   DCT_ASSERT(n_gains_level == n_thCSC_divided1);
   DCT_ASSERT(n_gains_level == n_thCSC_divided2);
   DCT_ASSERT(n_gains_level == n_thCSC_divided3);
   DCT_ASSERT(n_gains_level == n_thCSC_divided4);
   DCT_ASSERT(n_gains_level == n_diff_divided0);
   DCT_ASSERT(n_gains_level == n_diff_divided1);
   DCT_ASSERT(n_gains_level == n_diff_divided2);
   DCT_ASSERT(n_gains_level == n_diff_divided3);
   DCT_ASSERT(n_gains_level == n_diff_divided4);
   DCT_ASSERT(n_gains_level == n_varTh_divided0);
   DCT_ASSERT(n_gains_level == n_varTh_divided1);
   DCT_ASSERT(n_gains_level == n_varTh_divided2);
   DCT_ASSERT(n_gains_level == n_varTh_divided3);
   DCT_ASSERT(n_gains_level == n_varTh_divided4);
	
   DCT_ASSERT(n_gains_level == n_thgrad_r_fct);
   DCT_ASSERT(n_gains_level == n_thdiff_r_fct);
   DCT_ASSERT(n_gains_level == n_thvar_r_fct);
   DCT_ASSERT(n_gains_level == n_thgrad_b_fct);
   DCT_ASSERT(n_gains_level == n_thdiff_b_fct);
   DCT_ASSERT(n_gains_level == n_thvar_b_fct);
   DCT_ASSERT(n_gains_level == n_similarity_th);
   DCT_ASSERT(n_gains_level == n_th_grad);
   DCT_ASSERT(n_gains_level == n_th_diff);
   DCT_ASSERT(n_gains_level == n_th_var);
   DCT_ASSERT(n_gains_level == n_th_csc);
   DCT_ASSERT(n_gains_level == n_flat_level_sel);
   DCT_ASSERT(n_gains_level == n_pattern_level_sel);  
   DCT_ASSERT(n_gains_level == n_edge_level_sel);

   pDemosaicLpConf->gainsArray_ArraySize = n_gains_level;
   pDemosaicLpConf->thCSC_divided0_ArraySize = n_gains_level;
   pDemosaicLpConf->thCSC_divided1_ArraySize = n_gains_level;
   pDemosaicLpConf->thCSC_divided2_ArraySize = n_gains_level;
   pDemosaicLpConf->thCSC_divided3_ArraySize = n_gains_level;
   pDemosaicLpConf->thCSC_divided4_ArraySize = n_gains_level;
   pDemosaicLpConf->thH_divided0_ArraySize = n_gains_level;
   pDemosaicLpConf->thH_divided1_ArraySize = n_gains_level;
   pDemosaicLpConf->thH_divided2_ArraySize = n_gains_level;
   pDemosaicLpConf->thH_divided3_ArraySize = n_gains_level;
   pDemosaicLpConf->thH_divided4_ArraySize = n_gains_level;
   pDemosaicLpConf->diff_divided0_ArraySize = n_gains_level;
   pDemosaicLpConf->diff_divided1_ArraySize = n_gains_level;
   pDemosaicLpConf->diff_divided2_ArraySize = n_gains_level;
   pDemosaicLpConf->diff_divided3_ArraySize = n_gains_level;
   pDemosaicLpConf->diff_divided4_ArraySize = n_gains_level;
   pDemosaicLpConf->varTh_divided0_ArraySize = n_gains_level;
   pDemosaicLpConf->varTh_divided1_ArraySize = n_gains_level;
   pDemosaicLpConf->varTh_divided2_ArraySize = n_gains_level;
   pDemosaicLpConf->varTh_divided3_ArraySize = n_gains_level;
   pDemosaicLpConf->varTh_divided4_ArraySize = n_gains_level;

   pDemosaicLpConf->thgrad_r_fct_ArraySize = n_gains_level;
   pDemosaicLpConf->thdiff_r_fct_ArraySize = n_gains_level;
   pDemosaicLpConf->thvar_r_fct_ArraySize= n_gains_level;
   pDemosaicLpConf->thgrad_b_fct_ArraySize = n_gains_level;
   pDemosaicLpConf->thdiff_b_fct_ArraySize = n_gains_level;
   pDemosaicLpConf->thvar_b_fct_ArraySize = n_gains_level;
   pDemosaicLpConf->similarity_th_ArraySize = n_gains_level;
   pDemosaicLpConf->th_grad_ArraySize= n_gains_level;
   pDemosaicLpConf->th_diff_ArraySize= n_gains_level;
   pDemosaicLpConf->th_var_ArraySize= n_gains_level;
   pDemosaicLpConf->th_csc_ArraySize= n_gains_level;
   pDemosaicLpConf->flat_level_sel_ArraySize = n_gains_level;
   pDemosaicLpConf->pattern_level_sel_ArraySize = n_gains_level;
   pDemosaicLpConf->edge_level_sel_ArraySize = n_gains_level;

#ifdef DEBUG_LOG
  redirectOut << __FUNCTION__ << " (exit)" << std::endl;
#endif

  return (true);

}


/******************************************************************************
 * CalibDb::parseEntryDpf
 *****************************************************************************/
bool CalibDb::parseEntryDpf
(
    const XMLElement*  pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
  redirectOut << __FUNCTION__ << " (enter)" << std::endl;
#endif

  CamDpfProfile_t dpf_profile;
  MEMSET(&dpf_profile, 0, sizeof(dpf_profile));
  ListInit(&dpf_profile.Dsp3DNRSettingProfileList);
  ListInit(&dpf_profile.FilterList);

  XML_CHECK_START(CALIB_SENSOR_DPF_TAG_ID, CALIB_SENSOR_TAG_ID);
	
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif


    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_NAME_TAG_ID)){
      char* value = Toupper(tag.Value());
      strncpy(dpf_profile.name, value, sizeof(dpf_profile.name));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_RESOLUTION_TAG_ID)){
      const char* value = tag.Value();
      strncpy(dpf_profile.resolution, value, sizeof(dpf_profile.resolution));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_ADPF_ENABLE_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &dpf_profile.ADPFEnable, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_NLL_SEGMENTATION_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &dpf_profile.nll_segmentation, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_NLL_COEFF_TAG_ID)){
      int i = (sizeof(dpf_profile.nll_coeff) / sizeof(dpf_profile.nll_coeff.uCoeff[0]));
      int no = ParseUshortArray(tag.Value(), dpf_profile.nll_coeff.uCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_SIGMA_GREEN_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &dpf_profile.SigmaGreen, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_SIGMA_RED_BLUE_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &dpf_profile.SigmaRedBlue, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_GRADIENT_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &dpf_profile.fGradient, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_OFFSET_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &dpf_profile.fOffset, 1);
      DCT_ASSERT((no == 1));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_NLGAINS_TAG_ID)){
      int i = (sizeof(dpf_profile.NfGains) / sizeof(dpf_profile.NfGains.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), dpf_profile.NfGains.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if ( XML_CHECK_TAGID_COMPARE(CALIB_SESNOR_MFD_ENABLE_TAG_ID)){
      int no = ParseByteArray( tag.Value(), &dpf_profile.Mfd.enable, 1 );
      DCT_ASSERT( (no == tag.Size()) );
    } else if ( XML_CHECK_TAGID_COMPARE(CALIB_SESNOR_MFD_GAIN_TAG_ID)){
      int i = ( sizeof(dpf_profile.Mfd.gain) / sizeof(dpf_profile.Mfd.gain[0]) );
      int no = ParseFloatArray( tag.Value(), dpf_profile.Mfd.gain, i );
      DCT_ASSERT( (no == tag.Size()) );
    } else if ( XML_CHECK_TAGID_COMPARE(CALIB_SESNOR_MFD_FRAMES_TAG_ID)){
      int i = ( sizeof(dpf_profile.Mfd.frames) / sizeof(dpf_profile.Mfd.frames[0]) );
      int no = ParseFloatArray( tag.Value(), dpf_profile.Mfd.frames, i );
      DCT_ASSERT( (no == tag.Size()) );
    } else if ( XML_CHECK_TAGID_COMPARE(CALIB_SESNOR_UVNR_ENABLE_TAG_ID)){
      int no = ParseByteArray( tag.Value(), &dpf_profile.Uvnr.enable, 1 );
      DCT_ASSERT( (no == tag.Size()) );
    } else if ( XML_CHECK_TAGID_COMPARE(CALIB_SESNOR_UVNR_GAIN_TAG_ID)){
      int i = ( sizeof(dpf_profile.Uvnr.gain) / sizeof(dpf_profile.Uvnr.gain[0]) );
      int no = ParseFloatArray( tag.Value(), dpf_profile.Uvnr.gain, i );
      DCT_ASSERT( (no == tag.Size()) );
    } else if ( XML_CHECK_TAGID_COMPARE(CALIB_SESNOR_UVNR_RATIO_TAG_ID)){
      int i = ( sizeof(dpf_profile.Uvnr.ratio) / sizeof(dpf_profile.Uvnr.ratio[0]) );
      int no = ParseFloatArray( tag.Value(), dpf_profile.Uvnr.ratio, i );
      DCT_ASSERT( (no == tag.Size()) );
    } else if ( XML_CHECK_TAGID_COMPARE(CALIB_SESNOR_UVNR_DISTANCE_TAG_ID)){
      int i = ( sizeof(dpf_profile.Uvnr.distances) / sizeof(dpf_profile.Uvnr.distances[0]) );
      int no = ParseFloatArray( tag.Value(), dpf_profile.Uvnr.distances, i );
      DCT_ASSERT( (no == tag.Size()) );
    } else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPF_FILTERSETTING_TAG_ID)){
	  if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryFilter, &dpf_profile,
	  		(uint32_t)(CALIB_SENSOR_DPF_FILTERSETTING_TAG_ID), (uint32_t)(CALIB_SENSOR_DPF_TAG_ID))) {		
#if 1
		LOGE( "%s(%d): parse error in filter setting section (unknow tag: %s)\n", 
			__FUNCTION__,__LINE__,tagname.c_str());
#endif		
		return (false);
	  }
	} else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DSP_3DNR_SETTING_TAG_ID)){
	  if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntry3DNR, &dpf_profile,
	  		(uint32_t)(CALIB_SENSOR_DSP_3DNR_SETTING_TAG_ID), (uint32_t)(CALIB_SENSOR_DPF_TAG_ID))) {		
#if 1
		LOGE( "%s(%d): parse error in filter setting section (unknow tag: %s)\n", 
			__FUNCTION__,__LINE__,tagname.c_str());
#endif		
		return (false);
  	  }

	}else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_NEW_DSP_3DNR_SETTING_TAG_ID)){
 	  if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryNew3DNR, &dpf_profile,
	  		(uint32_t)CALIB_SENSOR_NEW_DSP_3DNR_SETTING_TAG_ID, (uint32_t)CALIB_SENSOR_DPF_TAG_ID)) { 	
 #if 1
 		LOGE( "%s(%d): parse error in filter setting section (unknow tag: %s)\n", 
 			__FUNCTION__,__LINE__,tagname.c_str());
 #endif		
 		return (false);
 	  }

	}else {
#if 1
		LOGE( "%s(%d): parse error in dpf section (unknow tag: %s)\n", 
			__FUNCTION__,__LINE__,tagname.c_str());
#endif	
    }	
    pchild = pchild->NextSibling();
  }
  XML_CHECK_END();
  
  RESULT result = CamCalibDbAddDpfProfile(m_CalibDbHandle, &dpf_profile);

  DCT_ASSERT(result == RET_SUCCESS);

  // free linked ecm_schemes
  List* l = ListRemoveHead(&dpf_profile.Dsp3DNRSettingProfileList);
  while (l) {
    List* temp = ListRemoveHead(l);
    free(l);
    l = temp;
  }

  List* l_new3dnr = ListRemoveHead(&dpf_profile.newDsp3DNRProfileList);
  while (l_new3dnr) {
    List* temp_new3dnr = ListRemoveHead(l_new3dnr);
    free(l_new3dnr);
    l_new3dnr = temp_new3dnr;
  }
   // free linked ecm_schemes
  List* l_filter = ListRemoveHead(&dpf_profile.FilterList);
  while (l_filter) {
    List* temp_filter = ListRemoveHead(l_filter);
    free(l_filter);
    l_filter = temp_filter;
  }

#ifdef DEBUG_LOG
  redirectOut << __FUNCTION__ << " (exit)" << std::endl;
#endif

  return (true);
}



/******************************************************************************
 * CalibDb::parseEntryDpcc
 *****************************************************************************/
bool CalibDb::parseEntryDpcc
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
  redirectOut << __FUNCTION__ << " (enter)" << std::endl;
#endif

  CamDpccProfile_t dpcc_profile;
  MEMSET(&dpcc_profile, 0, sizeof(dpcc_profile));

  XML_CHECK_START(CALIB_SENSOR_DPCC_TAG_ID, CALIB_SENSOR_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPCC_NAME_TAG_ID)){
      char* value = Toupper(tag.Value());
      strncpy(dpcc_profile.name, value, sizeof(dpcc_profile.name));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPCC_RESOLUTION_TAG_ID)){
      const char* value = tag.Value();
      strncpy(dpcc_profile.resolution, value, sizeof(dpcc_profile.resolution));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPCC_REGISTER_TAG_ID)){
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryDpccRegisters, &dpcc_profile,
	  		(uint32_t)(CALIB_SENSOR_DPCC_REGISTER_TAG_ID), (uint32_t)(CALIB_SENSOR_DPCC_TAG_ID))) {
#if 1
   	    LOGE( "%s(%d): parse error dpcc register section (unknow tag: %s)\n", 
   		    __FUNCTION__,__LINE__,tagname.c_str());
#endif	
        return (false);
      }
    } else {
#if 1
		LOGE( "%s(%d): parse error dpcc section (unknow tag: %s)\n", 
			__FUNCTION__,__LINE__,tagname.c_str());
#endif	
      return (false);
    }	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
  RESULT result = CamCalibDbAddDpccProfile(m_CalibDbHandle, &dpcc_profile);
  DCT_ASSERT(result == RET_SUCCESS);

#ifdef DEBUG_LOG
  redirectOut << __FUNCTION__ << " (exit)" << std::endl;
#endif

  return (true);
}



/******************************************************************************
 * CalibDb::parseEntryDpccRegisters
 *****************************************************************************/
bool CalibDb::parseEntryDpccRegisters
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
  redirectOut << __FUNCTION__ << " (enter)" << std::endl;
#endif

  CamDpccProfile_t* pDpcc_profile = (CamDpccProfile_t*)param;

  char*     reg_name;
  uint32_t    reg_value = 0U;
  
  XML_CHECK_START(CALIB_SENSOR_DPCC_REGISTER_TAG_ID, CALIB_SENSOR_DPCC_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPCC_REGISTER_NAME_TAG_ID)){
      reg_name = Toupper(tag.Value());
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_DPCC_REGISTER_VALUE_TAG_ID)){
      bool ok;

      reg_value = tag.ValueToUInt(&ok);
      if (!ok) {
#if 1
        LOGE( "%s(%d): parse error: invalid DPCC register value %s/%s\n", __FUNCTION__,__LINE__,tagname.c_str(),tag.Value());
#endif

        return (false);
      }
    } else {
#if 1
        LOGE( "%s(%d): parse error in DPCC register section (unknow tag: %s)\n", __FUNCTION__,__LINE__,pchild->ToElement()->Name());
#endif

      return (false);
    }	
    pchild = pchild->NextSibling();
  }
  XML_CHECK_END();
  
  std::string s_regname(reg_name);

  if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_MODE) {
    pDpcc_profile->isp_dpcc_mode = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_OUTPUT_MODE) {
    pDpcc_profile->isp_dpcc_output_mode = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_SET_USE) {
    pDpcc_profile->isp_dpcc_set_use = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_METHODS_SET_1) {
    pDpcc_profile->isp_dpcc_methods_set_1 = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_METHODS_SET_2) {
    pDpcc_profile->isp_dpcc_methods_set_2 = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_METHODS_SET_3) {
    pDpcc_profile->isp_dpcc_methods_set_3 = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_THRESH_1) {
    pDpcc_profile->isp_dpcc_line_thresh_1 = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_MAD_FAC_1) {
    pDpcc_profile->isp_dpcc_line_mad_fac_1 = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_PG_FAC_1) {
    pDpcc_profile->isp_dpcc_pg_fac_1 = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_THRESH_1) {
    pDpcc_profile->isp_dpcc_rnd_thresh_1 = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RG_FAC_1) {
    pDpcc_profile->isp_dpcc_rg_fac_1 = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_THRESH_2) {
    pDpcc_profile->isp_dpcc_line_thresh_2 = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_MAD_FAC_2) {
    pDpcc_profile->isp_dpcc_line_mad_fac_2 = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_PG_FAC_2) {
    pDpcc_profile->isp_dpcc_pg_fac_2 = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_THRESH_2) {
    pDpcc_profile->isp_dpcc_rnd_thresh_2 = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RG_FAC_2) {
    pDpcc_profile->isp_dpcc_rg_fac_2 = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_THRESH_3) {
    pDpcc_profile->isp_dpcc_line_thresh_3 = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_LINE_MAD_FAC_3) {
    pDpcc_profile->isp_dpcc_line_mad_fac_3 = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_PG_FAC_3) {
    pDpcc_profile->isp_dpcc_pg_fac_3 = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_THRESH_3) {
    pDpcc_profile->isp_dpcc_rnd_thresh_3 = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RG_FAC_3) {
    pDpcc_profile->isp_dpcc_rg_fac_3 = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RO_LIMITS) {
    pDpcc_profile->isp_dpcc_ro_limits = reg_value;
  } else if (s_regname == CALIB_SENSOR_DPCC_REGISTER_ISP_DPCC_RND_OFFS) {
    pDpcc_profile->isp_dpcc_rnd_offs = reg_value;
  } else {
#if 1
        LOGD( "%s(%d): unknown DPCC register (%s)\n", __FUNCTION__,__LINE__,s_regname.c_str());
#endif
  }
#if 0
  if (reg_name) {
    free(reg_name);
    reg_name = NULL;
  }
#endif
#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}

/******************************************************************************
 * CalibDb::parseEntryGoc
 *****************************************************************************/
bool CalibDb::parseEntryGoc
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
  redirectOut << __FUNCTION__ << " (enter)" << std::endl;
#endif

  CamCalibGocProfile_t goc_data;
  goc_data.def_cfg_mode = -1;
  goc_data.enable_mode = -1;
  memset(goc_data.GammaY, 0, sizeof(goc_data.GammaY));
  memset(goc_data.WdrOn_GammaY, 0, sizeof(goc_data.WdrOn_GammaY));

  XML_CHECK_START(CALIB_SENSOR_GOC_TAG_ID, CALIB_SENSOR_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif


	if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_GOC_NAME_TAG_ID)){
      char* value = Toupper(tag.Value());
      strncpy(goc_data.name, value, sizeof(goc_data.name));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_GOC_GAMMAY_TAG_ID)){
      int i = (sizeof(goc_data.GammaY) / sizeof(goc_data.GammaY[0]));
      int no = ParseUshortArray(tag.Value(), goc_data.GammaY, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_GOC_ENABLE_MODE_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &goc_data.enable_mode, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_GOC_DEF_CFG_MODE_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &goc_data.def_cfg_mode, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_GOC_WDRON_GAMMAY_TAG_ID)){
      int i = (sizeof(goc_data.WdrOn_GammaY) / sizeof(goc_data.WdrOn_GammaY[0]));
      int no = ParseUshortArray(tag.Value(), goc_data.WdrOn_GammaY, i);
      DCT_ASSERT((no == tag.Size()));
    }else{
		LOGE( "%s(%d): unknown GOC register (%s)\n", __FUNCTION__,__LINE__,tagname.c_str());
    }
	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
  RESULT result = CamCalibDbAddGocProfile(m_CalibDbHandle, &goc_data);
  DCT_ASSERT(result == RET_SUCCESS);

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}

bool CalibDb::parseEntryWdrMaxGain
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	CamCalibWdrGlobal_t *pWdr= (CamCalibWdrGlobal_t *)param;
	if(NULL == pWdr){
		LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
		return false;
	}

    int n_sensor_gains     = 0;
    float* pf_sensor_gain_level   = NULL;
    int n_maxgain      = 0;
    float* pf_maxgain_level    = NULL;
    float filter_enable = 0.0;
	  
	XML_CHECK_START(CALIB_SENSOR_WDR_MAXGAIN_FILTER_TAG_ID, CALIB_SENSOR_WDR_TAG_ID);
  
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif
		if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_MAXGAIN_FILTER_ENABLE_TAG_ID)){
          int no = ParseFloatArray(tag.Value(), &filter_enable, 1);
          DCT_ASSERT((no == 1));
        } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_MAXGAIN_SENSOR_GAIN_LEVEL_TAG_ID)){
          if (!pf_sensor_gain_level) {
            n_sensor_gains = tag.Size();
            pf_sensor_gain_level  = (float*)malloc((n_sensor_gains * sizeof(float)));
            MEMSET(pf_sensor_gain_level, 0, (n_sensor_gains * sizeof(float)));
          }

          int no = ParseFloatArray(tag.Value(), pf_sensor_gain_level, n_sensor_gains);
          DCT_ASSERT((no == n_sensor_gains));
        } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_MAXGAIN_MAXGAIN_LEVEL_TAG_ID)){
          if (!pf_maxgain_level) {
            n_maxgain = tag.Size();
            pf_maxgain_level = (float*)malloc((n_maxgain * sizeof(float)));
            MEMSET(pf_maxgain_level, 0, (n_maxgain * sizeof(float)));
          }

          int no = ParseFloatArray(tag.Value(), pf_maxgain_level, n_maxgain);
          DCT_ASSERT((no == n_maxgain));
        } else {
#if 1
          LOGE( "%s(%d): parse error in wdr max gain section unknow tag:%s \n",
                  __FUNCTION__,__LINE__, tagname.c_str());
#endif
          return (false);
        }		
        pchild = pchild->NextSibling();
	}

	XML_CHECK_END();

	DCT_ASSERT((n_sensor_gains == n_maxgain));
    pWdr->wdr_MaxGain_Level_curve.nSize = n_sensor_gains;
    if (filter_enable >= 1.0) {
      pWdr->wdr_MaxGain_Level_curve.filter_enable = 1;
    } else {
      pWdr->wdr_MaxGain_Level_curve.filter_enable = 0;
    }
    pWdr->wdr_MaxGain_Level_curve.pfSensorGain_level = pf_sensor_gain_level;
    pWdr->wdr_MaxGain_Level_curve.pfMaxGain_level = pf_maxgain_level;
  #ifdef DEBUG_LOG
    for (int index = 0; index < n_sensor_gains; index++) {
      //wdr_data.wdr_MaxGain_limit.pfMaxGain_level[index] = (uint16_t)(pf_maxgain_level[index]);
      LOGD("%s(%d): Sensorgain[%d] = %d \n", __FUNCTION__, __LINE__,
      		index, pWdr->wdr_MaxGain_Level_curve.pfSensorGain_level[index]);
	  LOGD("%s(%d): Maxgain[%d] = %d \n", __FUNCTION__, __LINE__,
      		index, pWdr->wdr_MaxGain_Level_curve.pfMaxGain_level[index]);
    }
  #endif

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}

/******************************************************************************
 * CalibDb::parseEntryWdr
 *****************************************************************************/
bool CalibDb::parseEntryWdr
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamCalibWdrGlobal_t wdr_data;
  memset(&wdr_data, 0, sizeof(wdr_data));
  uint32_t regValue = 0;
  
  XML_CHECK_START(CALIB_SENSOR_WDR_TAG_ID, CALIB_SENSOR_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_ENABLE_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &wdr_data.Enabled, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_MODE_TAG_ID)){
      int no = ParseUshortArray(tag.Value(), &wdr_data.Mode, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_LOCAL_CURVE_TAG_ID)){
      int i = (sizeof(wdr_data.LocalCurve) / sizeof(wdr_data.LocalCurve[0]));
      int no = ParseUshortArray(tag.Value(), wdr_data.LocalCurve, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_GLOBAL_CURVE_TAG_ID)){
      int i = (sizeof(wdr_data.GlobalCurve) / sizeof(wdr_data.GlobalCurve[0]));
      int no = ParseUshortArray(tag.Value(), wdr_data.GlobalCurve, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_NOISE_RATIO_TAG_ID)){
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_noiseratio = regValue;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_BEST_LIGHT_TAG_ID)){
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_bestlight = regValue;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_GAIN_OFF1_TAG_ID)){
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_gain_off1 = regValue;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_PYM_TAG_ID)){
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_pym_cc = regValue;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_EPSILON_TAG_ID)){
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_epsilon = regValue;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_LVL_TAG_ID)){
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_lvl_en = regValue;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_FLT_TAG_ID)){
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_flt_sel = regValue;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_GAIN_MAX_CLIP_ENABLE_TAG_ID)){
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_gain_max_clip_enable = regValue;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_GAIN_MAX_VALUE_TAG_ID)){
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_gain_max_value = regValue;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_BAVG_TAG_ID)){
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_bavg_clip = regValue;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_NONL_SEGM_TAG_ID)){
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_nonl_segm = regValue;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_NONL_OPEN_TAG_ID)){
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_nonl_open = regValue;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_NONL_MODE1_TAG_ID)){
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_nonl_mode1 = regValue;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_COE0_TAG_ID)){
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_coe0 = regValue;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_COE1_TAG_ID)){
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_coe1 = regValue;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_COE2_TAG_ID)){
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_coe2 = regValue;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_COE_OFF_TAG_ID)){
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_coe_off = regValue;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_MAXGAIN_FILTER_TAG_ID)){
      if(!parseEntryWdrMaxGain(pchild->ToElement(), &wdr_data)){
		LOGE( "%s(%d): parse error in wdr max gain section\n",__FUNCTION__,__LINE__);
		return (false);
      }
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_TBD_TAG_ID)){
     //do nothing
    }  
	else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_CURVE1_TAG_ID)){
      //do nothing
    }  
	else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_WDR_CURVE2_TAG_ID)){
      //do nothing
    }  
	else{
#if 1
	  LOGE( "%s(%d): parse error in rk ie sharp section (unknow tag: %s)\n", 
	  		__FUNCTION__,__LINE__,tagname.c_str());
#endif
    }
	
    pchild = pchild->NextSibling();
  }
  XML_CHECK_END();
  RESULT result = CamCalibDbAddWdrGlobal(m_CalibDbHandle, &wdr_data);
  DCT_ASSERT(result == RET_SUCCESS);

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}


bool CalibDb::parseEntryRKsharpen
(
  const XMLElement *pelement,
  void *param
)
{
    (void)param;

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

    CamIesharpenProfile_t iesharpen_profile;
    MEMSET(&iesharpen_profile,0,sizeof(iesharpen_profile));

    XML_CHECK_START(CALIB_SENSOR_IESHARPEN_TAG_ID, CALIB_SENSOR_TAG_ID);
	
    const XMLNode *pchild = pelement->FirstChild();
    while(pchild)
    {
        XmlTag tag = XmlTag(pchild->ToElement());
        std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

        if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_NAME_TAG_ID))
        {
            char * value = Toupper(tag.Value());
            strncpy(iesharpen_profile.name,value,sizeof(iesharpen_profile.name));
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_RESOLUTION_TAG_ID))
        {

            const char* value = tag.Value();
            strncpy(iesharpen_profile.resolution,value,sizeof(iesharpen_profile.resolution));
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_ENABLE_TAG_ID))
        {
            int no = ParseUcharArray(tag.Value(),&iesharpen_profile.iesharpen_en,tag.Size());
            DCT_ASSERT((no == 1));
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_CORING_THR_TAG_ID))
        {
            int no = ParseUcharArray(tag.Value(),&iesharpen_profile.coring_thr,tag.Size());
            DCT_ASSERT((no == 1));
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_FULL_RANGE_TAG_ID))
        {
            int no = ParseUcharArray(tag.Value(),&iesharpen_profile.full_range,tag.Size());
            DCT_ASSERT((no == 1));
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_SWITCH_AVG_TAG_ID))
        {
            int no = ParseUcharArray(tag.Value(),&iesharpen_profile.switch_avg,tag.Size());
            DCT_ASSERT((no == 1));
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_YAVG_THR_TAG_ID))
        {
            uint8_t* yavg_thr=NULL;
            yavg_thr = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(yavg_thr != NULL);
            MEMSET(yavg_thr,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), yavg_thr, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.yavg_thr_ArraySize = no;
            iesharpen_profile.yavg_thr=yavg_thr;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_P_DELTA1_TAG_ID))
        {
            uint8_t* p_delta1=NULL;
            p_delta1 = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(p_delta1 != NULL);
            MEMSET(p_delta1,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), p_delta1, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.P_delta1_ArraySize = no;
            iesharpen_profile.P_delta1=p_delta1;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_P_DELTA2_TAG_ID))
        {
            uint8_t* p_delta2=NULL;
            p_delta2 = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(p_delta2 != NULL);
            MEMSET(p_delta2,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), p_delta2, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.P_delta2_ArraySize = no;
            iesharpen_profile.P_delta2=p_delta2;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_P_MAX_NUMBER_TAG_ID))
        {
            uint8_t* pmaxnumber=NULL;
            pmaxnumber = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(pmaxnumber != NULL);
            MEMSET(pmaxnumber,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), pmaxnumber, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.pmaxnumber_ArraySize = no;
            iesharpen_profile.pmaxnumber=pmaxnumber;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_P_MIN_NUMBER_TAG_ID))
        {
            uint8_t* pminnumber=NULL;
            pminnumber = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(pminnumber != NULL);
            MEMSET(pminnumber,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), pminnumber, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.pminnumber_ArraySize = no;
            iesharpen_profile.pminnumber=pminnumber;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_GAUSS_FLAT_COE_TAG_ID))
        {
            uint8_t* pgauss_flat_coe=NULL;
            pgauss_flat_coe = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(pgauss_flat_coe != NULL);
            MEMSET(pgauss_flat_coe,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), pgauss_flat_coe, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.gauss_flat_coe_ArraySize = no;
            iesharpen_profile.gauss_flat_coe=pgauss_flat_coe;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_GAUSS_NOISE_COE_TAG_ID))
        {
            uint8_t* pgauss_noise_coe=NULL;
            pgauss_noise_coe = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(pgauss_noise_coe != NULL);
            MEMSET(pgauss_noise_coe,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), pgauss_noise_coe, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.gauss_noise_coe_ArraySize = no;
            iesharpen_profile.gauss_noise_coe=pgauss_noise_coe;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_GAUSS_OTHER_COE_TAG_ID))
        {
            uint8_t* pgauss_other_coe=NULL;
            pgauss_other_coe = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(pgauss_other_coe != NULL);
            MEMSET(pgauss_other_coe,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), pgauss_other_coe, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.gauss_other_coe_ArraySize = no;
            iesharpen_profile.gauss_other_coe=pgauss_other_coe;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_L_P_GRAD_TAG_ID))
        {
            uint16_t* pl_p_grad=NULL;
            pl_p_grad = (uint16_t*)malloc(tag.Size()*sizeof(uint16_t));
            DCT_ASSERT(pl_p_grad != NULL);
            MEMSET(pl_p_grad,0,(tag.Size()*sizeof(uint16_t)));
            int no = ParseUshortArray(tag.Value(), pl_p_grad, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.lgridconf.p_grad_ArraySize = no;
            iesharpen_profile.lgridconf.p_grad=pl_p_grad;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_L_SHARP_FACTOR_TAG_ID))
        {
            uint8_t* pl_sharp_factor=NULL;
            pl_sharp_factor = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(pl_sharp_factor != NULL);
            MEMSET(pl_sharp_factor,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), pl_sharp_factor, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.lgridconf.sharp_factor_ArraySize = no;
            iesharpen_profile.lgridconf.sharp_factor=pl_sharp_factor;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_L_LINE1_FILTER_COE_TAG_ID))
        {
            uint8_t* pl_line1_filter_coe=NULL;
            pl_line1_filter_coe = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(pl_line1_filter_coe != NULL);
            MEMSET(pl_line1_filter_coe,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), pl_line1_filter_coe, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.lgridconf.line1_filter_coe_ArraySize = no;
            iesharpen_profile.lgridconf.line1_filter_coe=pl_line1_filter_coe;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_L_LINE2_FILTER_COE_TAG_ID))
        {
            uint8_t* pl_line2_filter_coe=NULL;
            pl_line2_filter_coe = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(pl_line2_filter_coe != NULL);
            MEMSET(pl_line2_filter_coe,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), pl_line2_filter_coe, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.lgridconf.line2_filter_coe_ArraySize = no;
            iesharpen_profile.lgridconf.line2_filter_coe=pl_line2_filter_coe;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_L_LINE3_FILTER_COE_TAG_ID))
        {
            uint8_t* pl_line3_filter_coe=NULL;
            pl_line3_filter_coe = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(pl_line3_filter_coe != NULL);
            MEMSET(pl_line3_filter_coe,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), pl_line3_filter_coe, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.lgridconf.line3_filter_coe_ArraySize = no;
            iesharpen_profile.lgridconf.line3_filter_coe=pl_line3_filter_coe;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_H_P_GRAD_TAG_ID))
        {
            uint16_t* ph_p_grad=NULL;
            ph_p_grad = (uint16_t*)malloc(tag.Size()*sizeof(uint16_t));
            DCT_ASSERT(ph_p_grad != NULL);
            MEMSET(ph_p_grad,0,(tag.Size()*sizeof(uint16_t)));
            int no = ParseUshortArray(tag.Value(), ph_p_grad, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.hgridconf.p_grad_ArraySize = no;
            iesharpen_profile.hgridconf.p_grad=ph_p_grad;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_H_SHARP_FACTOR_TAG_ID))
        {
            uint8_t* ph_sharp_factor=NULL;
            ph_sharp_factor = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(ph_sharp_factor != NULL);
            MEMSET(ph_sharp_factor,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), ph_sharp_factor, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.hgridconf.sharp_factor_ArraySize = no;
            iesharpen_profile.hgridconf.sharp_factor=ph_sharp_factor;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_H_LINE1_FILTER_COE_TAG_ID))
        {
            uint8_t* ph_line1_filter_coe=NULL;
            ph_line1_filter_coe = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(ph_line1_filter_coe != NULL);
            MEMSET(ph_line1_filter_coe,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), ph_line1_filter_coe, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.hgridconf.line1_filter_coe_ArraySize = no;
            iesharpen_profile.hgridconf.line1_filter_coe=ph_line1_filter_coe;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_H_LINE2_FILTER_COE_TAG_ID))
        {
            uint8_t* ph_line2_filter_coe=NULL;
            ph_line2_filter_coe = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(ph_line2_filter_coe != NULL);
            MEMSET(ph_line2_filter_coe,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), ph_line2_filter_coe, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.hgridconf.line2_filter_coe_ArraySize = no;
            iesharpen_profile.hgridconf.line2_filter_coe=ph_line2_filter_coe;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_H_LINE3_FILTER_COE_TAG_ID))
        {
            uint8_t* ph_line3_filter_coe=NULL;
            ph_line3_filter_coe = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(ph_line3_filter_coe != NULL);
            MEMSET(ph_line3_filter_coe,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), ph_line3_filter_coe, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.hgridconf.line3_filter_coe_ArraySize = no;
            iesharpen_profile.hgridconf.line3_filter_coe=ph_line3_filter_coe;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_UV_GAUSS_FLAT_COE_TAG_ID))
        {
            uint8_t* puv_gauss_flat_coe=NULL;
            puv_gauss_flat_coe = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(puv_gauss_flat_coe != NULL);
            MEMSET(puv_gauss_flat_coe,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), puv_gauss_flat_coe, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.uv_gauss_flat_coe_ArraySize = no;
            iesharpen_profile.uv_gauss_flat_coe=puv_gauss_flat_coe;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_UV_GAUSS_NOISE_COE_TAG_ID))
        {
            uint8_t* puv_gauss_noise_coe=NULL;
            puv_gauss_noise_coe = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(puv_gauss_noise_coe != NULL);
            MEMSET(puv_gauss_noise_coe,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), puv_gauss_noise_coe, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.uv_gauss_noise_coe_ArraySize = no;
            iesharpen_profile.uv_gauss_noise_coe=puv_gauss_noise_coe;
        }
        else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_UV_GAUSS_OTHER_COE_TAG_ID))
        {
            uint8_t* puv_gauss_other_coe=NULL;
            puv_gauss_other_coe = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(puv_gauss_other_coe != NULL);
            MEMSET(puv_gauss_other_coe,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), puv_gauss_other_coe, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.uv_gauss_other_coe_ArraySize = no;
            iesharpen_profile.uv_gauss_other_coe=puv_gauss_other_coe;
        }else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_GAIN_DVIDE_TAG_ID))
        {
            int no = ParseFloatArray(tag.Value(), &iesharpen_profile.gain_dvide, 1);
            DCT_ASSERT((no == tag.Size()));
        }
		else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_L_LAP_MAT_COE_TAG_ID))
        {
            uint8_t* pl_lap_mat_coe=NULL;
            pl_lap_mat_coe = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(pl_lap_mat_coe != NULL);
            MEMSET(pl_lap_mat_coe,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), pl_lap_mat_coe, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.lgridconf.lap_mat_coe_ArraySize= no;
            iesharpen_profile.lgridconf.lap_mat_coe = pl_lap_mat_coe;
        }else if(XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_IESHARPEN_H_LAP_MAT_COE_TAG_ID))
        {
            uint8_t* ph_lap_mat_coe=NULL;
            ph_lap_mat_coe = (uint8_t*)malloc(tag.Size()*sizeof(uint8_t));
            DCT_ASSERT(ph_lap_mat_coe != NULL);
            MEMSET(ph_lap_mat_coe,0,(tag.Size()*sizeof(uint8_t)));
            int no = ParseUcharArray(tag.Value(), ph_lap_mat_coe, tag.Size());
            DCT_ASSERT((no == tag.Size()));
            iesharpen_profile.hgridconf.lap_mat_coe_ArraySize= no;
            iesharpen_profile.hgridconf.lap_mat_coe = ph_lap_mat_coe;
        }
        else
        {
#if 1
		  LOGE( "%s(%d): parse error in rk ie sharp section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
          return (false);
        }		
        pchild = pchild->NextSibling();
    }

	XML_CHECK_END();
	
    RESULT result = CamCalibDbAddRKsharpenProfile( m_CalibDbHandle, &iesharpen_profile);
    DCT_ASSERT( result == RET_SUCCESS );


#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

    return ( true );
}


bool CalibDb::parseEntrySystemAfps
(
    const XMLElement*   pelement,
    void*                param
)
{
#ifdef DEBUG_LOG
	LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

	CamCalibSystemData_t *pSystem= (CamCalibSystemData_t *)param;
	if(NULL == pSystem){
		LOGE("%s(%d): Invalid pointer (exit)\n", __FUNCTION__,__LINE__);
		return false;
	}
	  
	XML_CHECK_START(CALIB_SYSTEM_AFPS_TAG_ID, CALIB_SYSTEM_TAG_ID);
  
	const XMLNode* pchild = pelement->FirstChild();
	while (pchild) {
		XmlTag tag = XmlTag(pchild->ToElement());
		std::string tagname(pchild->ToElement()->Name());
		XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
		
#ifdef DEBUG_LOG
		LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif
		
		if (XML_CHECK_TAGID_COMPARE(CALIB_SYSTEM_AFPS_DEFAULT_TAG_ID)){
		  const char* value = tag.Value();
		  std::string s_value(value);
		  size_t find = s_value.find("on", 0);
		  pSystem->AfpsDefault = ((find == std::string::npos) ? BOOL_FALSE : BOOL_TRUE);
		}else {
#if 1
          LOGE( "%s(%d): parse error in system afps section unknow tag:%s \n",
                  __FUNCTION__,__LINE__, tagname.c_str());
#endif
          return (false);
        }		
        pchild = pchild->NextSibling();
	}

	XML_CHECK_END();

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif	

	return true;

}



/******************************************************************************
 * CalibDb::parseEntrySystem
 *****************************************************************************/
bool CalibDb::parseEntrySystem
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamCalibSystemData_t system_data;
  MEMSET(&system_data, 0, sizeof(CamCalibSystemData_t));

  XML_CHECK_START(CALIB_SYSTEM_TAG_ID, CALIB_FILESTART_TAG_ID);
	
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    const char* value = tag.Value();
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SYSTEM_AFPS_TAG_ID)){
	  if(!parseEntrySystemAfps(pchild->ToElement(), &system_data)){
		LOGE( "%s(%d): parse error in system afps section\n",__FUNCTION__,__LINE__);
		return (false);
      }
    } else {
#if 1
        LOGE( "%s(%d): parse error in system section (unknow tag: %s)\n", __FUNCTION__,__LINE__,tagname.c_str());
#endif
      return (false);
    }
	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  
  RESULT result = CamCalibDbSetSystemData(m_CalibDbHandle, &system_data);
  DCT_ASSERT(result == RET_SUCCESS);

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif

  return (true);
}


/******************************************************************************
 * CalibDb::parseEntrySystem
 *****************************************************************************/
bool CalibDb::parseEntryCproc
(
    const XMLElement*   pelement,
    void*                param
) {
#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamCprocProfile_t*  cprocProfile = (CamCprocProfile_t*)param;
  struct  CamCprocData_s* cproc_data = &(cprocProfile->cproc[cprocProfile->num_items++]);
  memset(cproc_data, 0, sizeof(struct CamCprocData_s));

  XML_CHECK_START(CALIB_SENSOR_CPROC_TAG_ID, CALIB_SENSOR_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_CPROC_NAME_TAG_ID)){
      strncpy(cproc_data->name, tag.Value(), sizeof(cproc_data->name));
      if (strcmp(cproc_data->name, "PREVIEW") == 0)
        cproc_data->cproc_scenario = CAM_CPROC_USECASE_PREVIEW;
      else if (strcmp(cproc_data->name, "CAPTURE") == 0)
        cproc_data->cproc_scenario = CAM_CPROC_USECASE_CAPTURE;
      else if (strcmp(cproc_data->name, "VIDEO") == 0)
        cproc_data->cproc_scenario = CAM_CPROC_USECASE_VIDEO;
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_CPROC_SATURATION_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &cproc_data->cproc_saturation, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_CPROC_CONTRAST_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &cproc_data->cproc_contrast, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_CPROC_BRIGHTNESS_TAG_ID)){
      float value = 0;
      int no = ParseFloatArray(tag.Value(), &value, 1);
      DCT_ASSERT((no == tag.Size()));
      cproc_data->cproc_brightness = (char)(value);
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_CPROC_HUE_TAG_ID)){
      int no = ParseFloatArray(tag.Value(), &cproc_data->cproc_hue, 1);
      DCT_ASSERT((no == tag.Size()));
    }else{
#if 1
      LOGE( "%s(%d): parse error in cproc section (unknow tag: %s)\n", 
          __FUNCTION__,__LINE__,tagname.c_str());
#endif

	}	
    pchild = pchild->NextSibling();
  }

  XML_CHECK_END();
  RESULT result = RET_SUCCESS;
  DCT_ASSERT(result == RET_SUCCESS);

#ifdef DEBUG_LOG
    LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif


  return (true);
}


/******************************************************************************
 * CalibDb::parseEntryOTP
 *****************************************************************************/
bool CalibDb::parseEntryOTP
(
    const XMLElement*   pelement,
    void*                param
) 
{
#ifdef DEBUG_LOG
  LOGD( "%s(%d): (enter)\n", __FUNCTION__,__LINE__);
#endif

  CamOTPGlobal_t  otpProfile;
  memset(&otpProfile, 0x00, sizeof(CamOTPGlobal_t));

  XML_CHECK_START(CALIB_SENSOR_OTP_TAG_ID, CALIB_SENSOR_TAG_ID);
  
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
	XML_CHECK_WHILE_SUBTAG_MARK((char *)(tagname.c_str()), tag.Type(), tag.Size());
	
#ifdef DEBUG_LOG
	LOGD( "%s(%d): tag: %s value:%s\n", __FUNCTION__,__LINE__,tagname.c_str(), tag.Value());
#endif

    if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_OTP_AWB_ENABLE_TAG_ID)){
      int no = ParseUcharArray(tag.Value(), &otpProfile.awb.enable, 1);
	  DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_OTP_AWB_GOLDEN_R_VALUE_TAG_ID)){
      int no = ParseUintArray(tag.Value(), &otpProfile.awb.golden_r_value, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_OTP_AWB_GOLDEN_GR_VALUE_TAG_ID)){
      int no = ParseUintArray(tag.Value(), &otpProfile.awb.golden_gr_value, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_OTP_AWB_GOLDEN_GB_VALUE_TAG_ID)){
      int no = ParseUintArray(tag.Value(), &otpProfile.awb.golden_gb_value, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_OTP_AWB_GOLDEN_B_VALUE_TAG_ID)){
      int no = ParseUintArray(tag.Value(), &otpProfile.awb.golden_b_value, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (XML_CHECK_TAGID_COMPARE(CALIB_SENSOR_OTP_LSC_ENABLE_TAG_ID)){
      int no = ParseUcharArray(tag.Value(), &otpProfile.lsc.enable, 1);
      DCT_ASSERT((no == tag.Size()));
    } else{
#if 1
	  LOGE( "%s(%d): parse error in cproc section (unknow tag: %s)\n", 
          __FUNCTION__,__LINE__,tagname.c_str());
#endif
	}    
    pchild = pchild->NextSibling();
  }
  
  XML_CHECK_END();
  RESULT result = CamCalibDbAddOTPGlobal(m_CalibDbHandle, &otpProfile);

#ifdef DEBUG_LOG
  LOGD( "%s(%d): (exit)\n", __FUNCTION__,__LINE__);
#endif


  return (true);
}


