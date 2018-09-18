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

/*************************************************************************/
/*************************************************************************/




#define CODE_XML_PARSE_VERSION "v0.1.2"

static std::ofstream redirectOut("/dev/null");

//#define redirectOut std::cout

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
    redirectOut
        << "parse error: invalid register value "
        << "/" << tag->Value() << std::endl;

    return (false);
  } else {
    redirectOut
        << "parse reg vale:" << *reg_value << std::endl;
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
    redirectOut << __func__ << "start" << find_start << "end" << find_end << std::endl;
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
    redirectOut << __func__ << std::endl;
    redirectOut << "str[0]:(" << str[0] << ")" << std::endl;
    redirectOut << "str_last[0]:(" << str_last[0] << ")" << std::endl;
    redirectOut << "strings:(" << str << ")" << std::endl;
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
      redirectOut << __func__ << "f" << f << "err" << std::endl;
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

  for (int i = 0; i < cnt; i++) {
    redirectOut << value[i] << ", ";
  }
  redirectOut << std::endl;
  redirectOut << std::endl;

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
    redirectOut << __func__ << "start" << find_start << "end" << find_end << std::endl;
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
    redirectOut << __func__ << std::endl;
    redirectOut << "str[0]:(" << str[0] << ")" << std::endl;
    redirectOut << "str_last[0]:(" << str_last[0] << ")" << std::endl;
    redirectOut << "strings:(" << str << ")" << std::endl;
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
      redirectOut << __func__ << "f" << f << "err" << std::endl;
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

  for (int i = 0; i < cnt; i++) {
    redirectOut << value[i] << ", ";
  }
  redirectOut << std::endl;
  redirectOut << std::endl;
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
    redirectOut << __func__ << "start" << find_start << "end" << find_end << std::endl;
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
    redirectOut << __func__ << std::endl;
    redirectOut << "str[0]:(" << str[0] << ")" << std::endl;
    redirectOut << "str_last[0]:(" << str_last[0] << ")" << std::endl;
    redirectOut << "strings:(" << str << ")" << std::endl;
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
      redirectOut << __func__ << "f" << f << "err" << std::endl;
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

  for (int i = 0; i < cnt; i++) {
    int x = value[i];
    redirectOut << x << ", ";
  }
  redirectOut << std::endl;
  redirectOut << std::endl;
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
    redirectOut << __func__ << "start" << find_start << "end" << find_end << std::endl;
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
    redirectOut << __func__ << std::endl;
    redirectOut << "str[0]:(" << str[0] << ")" << std::endl;
    redirectOut << "str_last[0]:(" << str_last[0] << ")" << std::endl;
    redirectOut << "strings:(" << str << ")" << std::endl;
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
      redirectOut << __func__ << "f" << f << "err" << std::endl;
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

  for (int i = 0; i < cnt; i++) {
    int x = value[i];
    redirectOut  << x << ", ";
  }
  redirectOut  << std::endl;
  redirectOut  << std::endl;
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
    redirectOut << __func__ << "start" << find_start << "end" << find_end << std::endl;
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
    redirectOut << __func__ << std::endl;
    redirectOut << "str[0]:(" << str[0] << ")" << std::endl;
    redirectOut << "str_last[0]:(" << str_last[0] << ")" << std::endl;
    redirectOut << "strings:(" << str << ")" << std::endl;
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
      redirectOut << __func__ << "f" << f << "err" << std::endl;
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

  for (int i = 0; i < cnt; i++) {
    redirectOut << value[i] << ", ";
  }
  redirectOut << std::endl;
  redirectOut << std::endl;
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
    redirectOut << __func__ << "start" << find_start << "end" << find_end << std::endl;
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
    redirectOut << __func__ << std::endl;
    redirectOut << "str[0]:(" << str[0] << ")" << std::endl;
    redirectOut << "str_last[0]:(" << str_last[0] << ")" << std::endl;
    redirectOut << "strings:(" << str << ")" << std::endl;
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
      redirectOut << __func__ << "f" << f << "err" << std::endl;
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

  for (int i = 0; i < cnt; i++) {
    redirectOut << value[i] << ", ";
  }
  redirectOut << std::endl;
  redirectOut << std::endl;
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
    redirectOut << __func__ << "start" << find_start << "end" << find_end << std::endl;
    return -1;
  }

  str = (char*)c_string + find_start;
  str_last = (char*)c_string + find_end;

#if 0
  /* check for beginning/closing parenthesis */
  if ((str[0] != '[') || (str_last[0] != ']')) {
    redirectOut << __func__ << std::endl;
    redirectOut << "str[0]:(" << str[0] << ")" << std::endl;
    redirectOut << "str_last[0]:(" << str_last[0] << ")" << std::endl;
    redirectOut << "strings:(" << str << ")" << std::endl;
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
      redirectOut << __func__ << "f" << f << "err" << std::endl;
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

  for (int i = 0; i < cnt; i++) {
    redirectOut << value[i] << ", ";
  }
  redirectOut << std::endl;
  redirectOut << std::endl;
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
      redirectOut << __func__ << "f" << f << "err" << std::endl;
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

  for (int i = 0; i < cnt; i++) {
    redirectOut << values[i] << ", ";
  }
  redirectOut << std::endl;
  redirectOut << std::endl;
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
      redirectOut << __func__ << "f" << f << "err" << std::endl;
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

  for (int i = 0; i < cnt; i++) {
    redirectOut << values[i] << ", ";
  }
  redirectOut << std::endl;
  redirectOut << std::endl;
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

  redirectOut << __func__ << " (enter)";

  // get and parse header section
  const XMLElement* header = root->FirstChildElement(CALIB_HEADER_TAG);
  if (!header) {
    res = parseEntryHeader(header->ToElement(), NULL);
    if (!res) {
      return (res);
    }
  }


  // get and parse sensor section
  const XMLElement* sensor = root->FirstChildElement(CALIB_SENSOR_TAG);
  if (!sensor) {
    res = parseEntrySensor(sensor->ToElement(), NULL);
    if (!res) {
      return (res);
    }
  }

  // get and parse system section
  const XMLElement* system = root->FirstChildElement(CALIB_SYSTEM_TAG);
  if (!system) {
    res = parseEntrySystem(system->ToElement(), NULL);
    if (!res) {
      return (res);
    }
  }

  redirectOut << __func__ << " (exit)";

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
  redirectOut << __func__ << " (enter)" << std::endl;

  RESULT result = CamCalibDbCreate(&m_CalibDbHandle);
  DCT_ASSERT(result == RET_SUCCESS);
  errorID = doc.LoadFile(device);
  redirectOut << __func__ << " doc.LoadFile" << "filename" << device << "error" << errorID << std::endl;
  if (doc.Error()) {
#if 1
    redirectOut
        << "Error: Parse error errorID " << errorID
        << std::endl;
#endif
    return (false);
  }
  XMLElement* proot = doc.RootElement();
  std::string tagname(proot->Name());
  if (tagname != CALIB_FILESTART_TAG) {
    redirectOut << "Error: Not a calibration data file" << std::endl;

    return (false);
  }

  // parse header section
  XMLElement* pheader = proot->FirstChildElement(CALIB_HEADER_TAG);
  if (pheader) {
    res = parseEntryHeader(pheader->ToElement(), NULL);
    if (!res) {
      redirectOut << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>3333333333";
      return (res);
    }
  }

  // parse sensor section
  XMLElement* psensor = proot->FirstChildElement(CALIB_SENSOR_TAG);
  if (psensor) {
    res = parseEntrySensor(psensor->ToElement(), NULL);
    if (!res) {
      redirectOut << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>444444444444";
      return (res);
    }
  }

  // parse system section
  XMLElement* psystem = proot->FirstChildElement(CALIB_SYSTEM_TAG);
  if (psystem) {
    res = parseEntrySystem(psystem->ToElement(), NULL);
    if (!res) {
      redirectOut << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>555555555555";
      return (res);
    }
  }

  redirectOut << __func__ << " (exit)" << std::endl;

  return (res);
}




/******************************************************************************
 * CalibDb::parseEntryCell
 *****************************************************************************/
bool CalibDb::parseEntryCell
(
    const XMLElement*   pelement,
    int                 noElements,
    parseCellContent    func,
    void*                param
) {
  int cnt = 0;

  redirectOut << __func__ << " (enter)" << std::endl;

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild && (cnt < noElements)) {
    XmlCellTag tag = XmlCellTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());

    if (tagname == CALIB_CELL_TAG) {
      bool result = (this->*func)(pchild->ToElement(), param);
      if (!result) {
        return (result);
      }
    } else {
#if 1
      redirectOut << "unknown cell tag: " << tagname << std::endl;
#endif

      return (false);
    }

    pchild = pchild->NextSibling();
    cnt ++;
  }

  redirectOut << __func__ << " (exit)" << std::endl;

  return (true);
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

  redirectOut << __func__ << " (enter)" << std::endl;

  CamCalibDbMetaData_t meta_data;
  MEMSET(&meta_data, 0, sizeof(meta_data));

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    const char* value = tag.Value();
    std::string tagname(pchild->ToElement()->Name());

#if 1
    redirectOut << "tag: " << tagname << std::endl;
#endif

    if ((tagname == CALIB_HEADER_CREATION_DATE_TAG)
        && (tag.isType(XmlTag::TAG_TYPE_CHAR))
        && (tag.Size() > 0)) {
      strncpy(meta_data.cdate, value, sizeof(meta_data.cdate));
    } else if ((tagname == CALIB_HEADER_CREATOR_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      strncpy(meta_data.cname, value, sizeof(meta_data.cname));
    } else if ((tagname == CALIB_HEADER_GENERATOR_VERSION_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      strncpy(meta_data.cversion, value, sizeof(meta_data.cversion));
    } else if ((tagname == CALIB_HEADER_SENSOR_NAME_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      strncpy(meta_data.sname, value, sizeof(meta_data.sname));
    } else if ((tagname == CALIB_HEADER_SAMPLE_NAME_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      strncpy(meta_data.sid, value, sizeof(meta_data.sid));
    } else if (tagname == CALIB_HEADER_RESOLUTION_TAG) {
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryResolution)) {
#if 1
        redirectOut
            << "parse error in header resolution section ("
            << tagname
            << ")"
            << std::endl;
#endif

        return (false);
      }
    } else if( (tagname == CALIB_HEADER_CODE_XML_PARSE_VERSION_TAG)
    		   && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)){
	  strncpy(meta_data.code_xml_parse_version, value, sizeof(meta_data.code_xml_parse_version));
	}else {
#if 1
      redirectOut
          << "parse error in header section (unknow tag: "
          << tagname
          << ")"
          << std::endl;
#endif

      return (false);
    }

    pchild = pchild->NextSibling();
  }

  if(strcmp(meta_data.code_xml_parse_version, CODE_XML_PARSE_VERSION)){
		std::cout
            << " code xml parse version is no match ("
            << meta_data.code_xml_parse_version
            << ")"
            << " != ("
            << CODE_XML_PARSE_VERSION
            << ")"
            << std::endl;
		return (false);
  }
  
  RESULT result = CamCalibDbSetMetaData(m_CalibDbHandle, &meta_data);
  DCT_ASSERT(result == RET_SUCCESS);

  redirectOut << __func__ << " (exit)" << std::endl;

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

  redirectOut << __func__ << " (enter)" << std::endl;

  CamResolution_t resolution;
  MEMSET(&resolution, 0, sizeof(resolution));
  ListInit(&resolution.framerates);

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    const char* value = tag.Value();
    std::string tagname(pchild->ToElement()->Name());

    redirectOut << " tag: " << tagname << std::endl;
    if ((tagname == CALIB_HEADER_RESOLUTION_NAME_TAG)
        && (tag.isType(XmlTag::TAG_TYPE_CHAR))
        && (tag.Size() > 0)) {
      strncpy(resolution.name, value, sizeof(resolution.name));
    } else if ((tagname == CALIB_HEADER_RESOLUTION_WIDTH_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseUshortArray(value,  &resolution.width, 1);
      DCT_ASSERT((no == 1));
    } else if ((tagname == CALIB_HEADER_RESOLUTION_HEIGHT_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseUshortArray(value, &resolution.height, 1);
      DCT_ASSERT((no == 1));
    } else if ((tagname == CALIB_HEADER_RESOLUTION_FRATE_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_CELL))
               && (tag.Size() > 0)) {
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryFramerates, &resolution)) {
        redirectOut
            << "parse error in header resolution(unknow tag: "
            << tagname
            << ")" << std::endl;

        return (false);
      }


    } else if (tagname == CALIB_HEADER_RESOLUTION_ID_TAG) {
      bool ok;

      resolution.id = tag.ValueToUInt(&ok);
      if (!ok) {
#if 1
        redirectOut
            << "parse error: invalid resolution "
            << tagname << "/" << tag.Value() << std::endl;
#endif

        return (false);
      }
    } else {
      redirectOut << "unknown tag: " << tagname << std::endl;
      return (false);
    }

    pchild = pchild->NextSibling();

  }

  RESULT result = CamCalibDbAddResolution(m_CalibDbHandle, &resolution);
  DCT_ASSERT(result == RET_SUCCESS);

  // free linked framerates
  List* l = ListRemoveHead(&resolution.framerates);
  while (l) {
    List* tmp = ListRemoveHead(l);
    free(l);
    l = tmp;
  }

  redirectOut << __func__ << " (exit)" << std::endl;

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

  redirectOut << __func__ << " (enter)" << std::endl;

  CamResolution_t* pResolution = (CamResolution_t*)param;
  CamFrameRate_t* pFrate = (CamFrameRate_t*) malloc(sizeof(CamFrameRate_t));
  if (!pFrate) {
    return false;
  }
  MEMSET(pFrate, 0, sizeof(*pFrate));

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    const char* value = tag.Value();
    std::string tagname(pchild->ToElement()->Name());

#if 1
    redirectOut
        << "tag: "
        << tagname
        << std::endl;
#endif

    if ((tagname == CALIB_HEADER_RESOLUTION_FRATE_NAME_TAG)
        && (tag.isType(XmlTag::TAG_TYPE_CHAR))
        && (tag.Size() > 0)) {
      snprintf(pFrate->name, CAM_FRAMERATE_NAME, "%s_%s",
               pResolution->name, value);
    } else if ((tagname == CALIB_HEADER_RESOLUTION_FRATE_FPS_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(value, &pFrate->fps, 1);
      DCT_ASSERT((no == 1));
    } else {
#if 1
      redirectOut
          << "parse error in framerate section (unknow tag: "
          << tagname
          << ")"
          << std::endl;
#endif

      return (false);
    }

    pchild = pchild->NextSibling();
  }

  if (pResolution) {
    ListPrepareItem(pFrate);
    ListAddTail(&pResolution->framerates, pFrate);
  }

  redirectOut << __func__ << " (exit)" << std::endl;

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

  redirectOut << __func__ << " (enter)" << std::endl;

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
    redirectOut << "tag: " << tagname << std::endl;

    if (tagname == CALIB_SENSOR_AWB_TAG) {
      if (!parseEntryAwb(pchild->ToElement())) {
        return (false);
      }
    } else if (tagname == CALIB_SENSOR_LSC_TAG) {
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryLsc)) {
        return (false);
      }
    } else if (tagname == CALIB_SENSOR_CC_TAG) {
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryCc)) {
        return (false);
      }
    } else if (tagname == CALIB_SENSOR_AF_TAG) {
      redirectOut << "tag: " << tagname << std::endl;
    } else if (tagname == CALIB_SENSOR_AEC_TAG) {
      redirectOut << "tag: " << tagname << std::endl;
      if (!parseEntryAec(pchild->ToElement())) {
        return (false);
      }
    } else if (tagname == CALIB_SENSOR_BLS_TAG) {
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryBls)) {
#if 1
        redirectOut
            << "parse error in BLS section (unknow tag: "
            << pchild->ToElement()->Name()
            << ")"
            << std::endl;
#endif

        return (false);
      }
    } else if (tagname == CALIB_SENSOR_DEGAMMA_TAG) {
      redirectOut << "tag: " << tagname << std::endl;
    } else if (tagname == CALIB_SENSOR_WDR_TAG) {
      redirectOut << "tag: " << tagname << std::endl;

      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryWdr)) {
#if 1
        redirectOut
            << "parse error in WDR section (unknow tag: "
            << tagname
            << ")"
            << std::endl;
#endif

        //return ( false );
      }

    } else if (tagname == CALIB_SENSOR_CAC_TAG) {
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryCac)) {
#if 1
        redirectOut
            << "parse error in CAC section (unknow tag: "
            << tagname
            << ")"
            << std::endl;
#endif

        return (false);
      }
    } else if (tagname == CALIB_SENSOR_DPF_TAG) {
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryDpf)) {
#if 1
        redirectOut
            << "parse error in DPF section (unknow tag: "
            << tagname
            << ")"
            << std::endl;
#endif

        return (false);
      }
    } else if (tagname == CALIB_SENSOR_DPCC_TAG) {
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryDpcc)) {
#if 1
        redirectOut
            << "parse error in DPF section (unknow tag: "
            << tagname
            << ")"
            << std::endl;
#endif

        return (false);
      }
    } else if (tagname == CALIB_SENSOR_GOC_TAG) {

      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryGoc)) {
#if 1
        redirectOut
            << "parse error in GOC section (unknow tag: "
            << tagname
            << ")"
            << std::endl;
#endif

        //return ( false );
      }

    } else if (tagname == CALIB_SENSOR_CPROC_TAG) {
      CamCprocProfile_t cproc;
      cproc.num_items = 0;
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryCproc,  &cproc)) {
#if 1
        redirectOut
            << "parse error in GOC section (unknow tag: "
            << tagname
            << ")"
            << std::endl;
#endif

        //return ( false );
      } else
        CamCalibDbAddCproc(m_CalibDbHandle, &cproc);
    } else {
#if 1
      redirectOut
          << "parse error in header section (unknow tag: "
          << tagname
          << ")"
          << std::endl;
#endif

      return (false);
    }

    pchild = pchild->NextSibling();
  }

  redirectOut << __func__ << " (exit)" << std::endl;

  return (true);
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

  redirectOut << __func__ << " (enter)" << std::endl;

  CamCalibAecGlobal_t aec_data;
  memset(&aec_data, 0, sizeof(aec_data));

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
    redirectOut << "tag: " << tagname<< std::endl;

    if ((tagname == CALIB_SENSOR_AEC_SETPOINT_TAG)
        && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
        && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &aec_data.SetPoint, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AEC_NIGHT_SETPOINT_TAG)
        && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
        && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &aec_data.NightSetPoint, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AEC_SEMMODE_TAG) //cxf
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      char* value = Toupper(tag.Value());
      std::string s_value(value);
      redirectOut << "value:" << value << std::endl;
      redirectOut << s_value << std::endl;
      if (s_value == CALIB_SENSOR_AEC_SEMMODE_ADAPTIVE) {
        aec_data.SemMode = AEC_SCENE_EVALUATION_ADAPTIVE;
      } else if (s_value == CALIB_SENSOR_AEC_SEMMODE_FIX) {
        aec_data.SemMode = AEC_SCENE_EVALUATION_FIX;
      } else if (s_value == CALIB_SENSOR_AEC_SEMMODE_DISABLED) {
        aec_data.SemMode = AEC_SCENE_EVALUATION_DISABLED;
      } else {
        aec_data.SemMode = AEC_SCENE_EVALUATION_INVALID;
        redirectOut << "invalid AEC SemMode (" << s_value << ")" << std::endl;
      }

    } else if (tagname == CALIB_SENSOR_AEC_GAINRANGE_TAG 
    			&& (tag.Size() > 0) ) {
      int i = tag.Size();
	  aec_data.GainRange.pGainRange = (float *)malloc(i*sizeof(float));
	  if(aec_data.GainRange.pGainRange == NULL){
		std::cout << "aec gain range malloc fail!" << std::endl;
	  }
      int no = ParseFloatArray(tag.Value(), aec_data.GainRange.pGainRange, i);
      DCT_ASSERT((no == tag.Size()));
	  DCT_ASSERT(((i%7) == 0) );
	  aec_data.GainRange.array_size = i;
    } else if (tagname == CALIB_SENSOR_AEC_TIMEFACTOR_TAG) {
      int i = (sizeof(aec_data.TimeFactor) / sizeof(aec_data.TimeFactor[0]));
      int no = ParseFloatArray(tag.Value(), aec_data.TimeFactor, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_AEC_GRIDWEIGHTS_TAG
                && (tag.Size() > 0)) { //cxf
      uint8_t *pWeight  = NULL;
      int arraySize     = tag.Size();
      
      pWeight = (uint8_t *)malloc(arraySize * sizeof(uint8_t));
      MEMSET( pWeight, 0, (arraySize * sizeof( uint8_t )) );
      
      int no = ParseUcharArray( tag.Value(), pWeight, arraySize );
      DCT_ASSERT( (no == arraySize) );
      
      aec_data.GridWeights.ArraySize = arraySize;
      aec_data.GridWeights.pWeight = pWeight;
    } else if (tagname == CALIB_SENSOR_AEC_MEASURINGWINWIDTHSCALE_TAG) { //cxf
      int no = ParseFloatArray(tag.Value(), &aec_data.MeasuringWinWidthScale, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_AEC_MEASURINGWINHEIGHTSCALE_TAG) { //cxf
      int no = ParseFloatArray(tag.Value(), &aec_data.MeasuringWinHeightScale, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AEC_CAMERICISPHISTMODE_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) { //cxf
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
      redirectOut << "value:" << value << std::endl;
      redirectOut << s_value << std::endl;
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
        redirectOut << "invalid AEC CamerIcIspHistMode (" << s_value << ")" << std::endl;
      }

    }

    else if ((tagname == CALIB_SENSOR_AEC_CAMERICISPEXPMEASURINGMODE_TAG) //cxf
             && (tag.isType(XmlTag::TAG_TYPE_CHAR))
             && (tag.Size() > 0)) {
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
      redirectOut << "value:" << value << std::endl;
      redirectOut << s_value << std::endl;
      if (s_value == CALIB_SENSOR_AEC_CAMERICISPEXPMEASURINGMODE_1) {
        aec_data.CamerIcIspExpMeasuringMode = CAM_EXP_MEASURING_MODE_1;
      } else if (s_value == CALIB_SENSOR_AEC_CAMERICISPEXPMEASURINGMODE_2) {
        aec_data.CamerIcIspExpMeasuringMode = CAM_EXP_MEASURING_MODE_2;
      } else {
        aec_data.CamerIcIspExpMeasuringMode = CAM_EXP_MEASURING_MODE_INVALID;
        redirectOut << "CamerIcIspExpMeasuringMode (" << s_value << ")" << std::endl;
      }
    }

    else if ((tagname == CALIB_SENSOR_AEC_CLM_TOLERANCE_TAG)
             && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
             && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &aec_data.ClmTolerance, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AEC_DAMP_OVER_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &aec_data.DampOverStill, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AEC_DAMP_UNDER_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &aec_data.DampUnderStill, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AEC_DAMP_OVER_VIDEO_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &aec_data.DampOverVideo, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AEC_DAMP_UNDER_VIDEO_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &aec_data.DampUnderVideo, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AEC_AFPS_MAX_GAIN_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &aec_data.AfpsMaxGain, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AEC_AFPS_GAIN_FACTOR_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &aec_data.GainFactor, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AEC_AFPS_GAIN_BIAS_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &aec_data.GainBias, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AEC_AFPS_MAX_INTTIME_TAG)//cxf
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &aec_data.AfpsMaxIntTime, 1);
      DCT_ASSERT((no == tag.Size()));
    }
    /*else if ( (tagname == CALIB_SENSOR_AEC_FRAMERATEVIDEO_TAG )//cxf
                && (tag.isType( XmlTag::TAG_TYPE_DOUBLE ))
                && (tag.Size() > 0) )
    {
        int no = ParseFloatArray( tag.Value(), &aec_data.FrameRateVideo, 1 );
        DCT_ASSERT( (no == tag.Size()) );
    }*/
    else if ((tagname == CALIB_SENSOR_AEC_ECMTIMEDOT_TAG)  //cxf
             && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
             && (tag.Size() > 0)) {
      int i = (sizeof(aec_data.EcmTimeDot) / sizeof(aec_data.EcmTimeDot.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), aec_data.EcmTimeDot.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AEC_ECMGAINDOT_TAG)//cxf
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(aec_data.EcmGainDot) / sizeof(aec_data.EcmGainDot.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), aec_data.EcmGainDot.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AEC_FPS_FPS_SET_CONFIG)//oyyf
               && (tag.Size() > 0)) {
	  const XMLNode* psubchild = pchild->ToElement()->FirstChild();
	  redirectOut << "FpsSet Tagname: " << tagname << std::endl;
      while (psubchild) 
	  {
	        XmlTag subtag = XmlTag(psubchild->ToElement());
	        std::string subTagname(psubchild->ToElement()->Name());
	        redirectOut << "FpsSet subTagname: " << subTagname << std::endl;

        	if ((subTagname == CALIB_SENSOR_AEC_FPS_FPS_SET_ENABLE)	
             	&& (subtag.Size() > 0))
    		{
      			int no = ParseUcharArray(subtag.Value(), &aec_data.FpsSetEnable, 1);
      			DCT_ASSERT((no == 1));
			}else if ((subTagname == CALIB_SENSOR_AEC_FPS_IS_FPS_FIX)	
             	&& (subtag.Size() > 0))
			{
				int no = ParseUcharArray(subtag.Value(), &aec_data.isFpsFix, 1);
      			DCT_ASSERT((no == 1));
			}else if ((subTagname == CALIB_SENSOR_AEC_FPS_FPS_FIX_TIMEDOT)	
             	&& (subtag.Size() > 0))
			{
				int i = (sizeof(aec_data.FpsFixTimeDot) / sizeof(aec_data.FpsFixTimeDot.fCoeff[0]));
      			int no = ParseFloatArray(subtag.Value(), aec_data.FpsFixTimeDot.fCoeff, i);
      			DCT_ASSERT((no == i));
			}
			psubchild = psubchild->NextSibling();
      }
	}else if ((tagname == CALIB_SENSOR_AEC_ECMMODE_TAG)//cxf
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      char* value = Toupper(tag.Value());
      std::string s_value(value);
      redirectOut << "value:" << value << std::endl;
      redirectOut << s_value << std::endl;
      if (s_value == CALIB_SENSOR_AEC_ECMMODE_1) {
        aec_data.EcmMode = CAM_ECM_MODE_1 ;
      } else if (s_value == CALIB_SENSOR_AEC_ECMMODE_2) {
        aec_data.EcmMode = CAM_ECM_MODE_2;
      } else {
        aec_data.EcmMode = CAM_ECM_MODE_INVALID;
        redirectOut << "CamerIcIspEcmMode (" << s_value << ")" << std::endl;
      }
    }

    else if (tagname == CALIB_SENSOR_AEC_ECM_TAG) {
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryAecEcm)) {
#if 1
        redirectOut
            << "parse error in AEC section ("
            << tagname
            << ")"
            << std::endl;
#endif

        return (false);
      }
    } else if (tagname == CALIB_SENSOR_AEC_AOE_ENABLE
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &aec_data.AOE_Enable, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_AEC_AOE_MAX_POINT
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &aec_data.AOE_Max_point, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_AEC_AOE_MIN_POINT
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &aec_data.AOE_Min_point, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_AEC_AOE_Y_MAX_TH
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &aec_data.AOE_Y_Max_th, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_AEC_AOE_Y_MIN_TH
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &aec_data.AOE_Y_Min_th, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_AEC_AOE_STEP_INC
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &aec_data.AOE_Step_Inc, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_AEC_AOE_STEP_DEC
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &aec_data.AOE_Step_Dec, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_AEC_DON) {
      const XMLNode* psubchild = pchild->ToElement()->FirstChild();
      while (psubchild) {
        XmlTag tag = XmlTag(psubchild->ToElement());
        std::string subTagname(psubchild->ToElement()->Name());

        redirectOut << "subTagname: " << subTagname << std::endl;
        if (subTagname == CALIB_SENSOR_AEC_DON_ENABLE
            && (tag.isType(XmlTag::TAG_TYPE_CHAR))
            && (tag.Size() > 0)) {
          int no = ParseByteArray(tag.Value(), &aec_data.DON_Enable, 1);
          DCT_ASSERT((no == tag.Size()));
        } else if (subTagname == CALIB_SENSOR_AEC_DON_DAY2NIGHT_GAIN_TH
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          int no = ParseFloatArray(tag.Value(), &aec_data.DON_Day2Night_Gain_th, 1);
          DCT_ASSERT((no == tag.Size()));
        } else if (subTagname == CALIB_SENSOR_AEC_DON_DAY2NIGHT_INTTIME_TH
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          int no = ParseFloatArray(tag.Value(), &aec_data.DON_Day2Night_Inttime_th, 1);
          DCT_ASSERT((no == tag.Size()));
        } else if (subTagname == CALIB_SENSOR_AEC_DON_DAY2NIGHT_LUMA_TH
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          int no = ParseFloatArray(tag.Value(), &aec_data.DON_Day2Night_Luma_th, 1);
          DCT_ASSERT((no == tag.Size()));
        } else if (subTagname == CALIB_SENSOR_AEC_DON_NIGHT2DAY_GAIN_TH
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          int no = ParseFloatArray(tag.Value(), &aec_data.DON_Night2Day_Gain_th, 1);
          DCT_ASSERT((no == tag.Size()));
        } else if (subTagname == CALIB_SENSOR_AEC_DON_NIGHT2DAY_INTTIME_TH
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          int no = ParseFloatArray(tag.Value(), &aec_data.DON_Night2Day_Inttime_th, 1);
          DCT_ASSERT((no == tag.Size()));
        } else if (subTagname == CALIB_SENSOR_AEC_DON_NIGHT2DAY_LUMA_TH
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          int no = ParseFloatArray(tag.Value(), &aec_data.DON_Night2Day_Luma_th, 1);
          DCT_ASSERT((no == tag.Size()));
        } else if (subTagname == CALIB_SENSOR_AEC_DON_BOUNCING_TH
                   && (tag.isType(XmlTag::TAG_TYPE_CHAR))
                   && (tag.Size() > 0)) {
          int no = ParseByteArray(tag.Value(), &aec_data.DON_Bouncing_th, 1);
          DCT_ASSERT((no == tag.Size()));
        } else {
          redirectOut
              << "parse error in AEC DON section (unknow tag: "
              << tagname
              << ")"
              << std::endl;
        }
        psubchild = psubchild->NextSibling();
      }
    } else if (tagname == CALIB_SENSOR_AEC_INTERVAL_ADJUST_STRATEGY) {
      const XMLNode* psubchild = pchild->ToElement()->FirstChild();
      while (psubchild) {
        XmlTag tag = XmlTag(psubchild->ToElement());
        std::string subTagname(psubchild->ToElement()->Name());

        redirectOut << "subTagname: " << subTagname << std::endl;
        if (subTagname == CALIB_SENSOR_AEC_INTERVAL_ADJUST_ENABLE
            && (tag.Size() > 0)) {
          int no = ParseUcharArray(tag.Value(), &aec_data.InterAdjustStrategy.enable, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (subTagname == CALIB_SENSOR_AEC_INTERVAL_ADJUST_DLUMA_HIGH_TH
            && (tag.Size() > 0)) {
          int no = ParseFloatArray(tag.Value(), &aec_data.InterAdjustStrategy.dluma_high_th, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (subTagname == CALIB_SENSOR_AEC_INTERVAL_ADJUST_DLUMA_LOW_TH
            && (tag.Size() > 0)) {
          int no = ParseFloatArray(tag.Value(), &aec_data.InterAdjustStrategy.dluma_low_th, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else if (subTagname == CALIB_SENSOR_AEC_INTERVAL_ADJUST_TRIGGER_FRAME
            && (tag.Size() > 0)) {
          int no = ParseUintArray(tag.Value(), &aec_data.InterAdjustStrategy.trigger_frame, tag.Size());
          DCT_ASSERT((no == tag.Size()));
        }else {
          redirectOut
              << "parse error in AEC interval adjust strategy section (unknow tag: "
              << tagname
              << ")"
              << std::endl;
        }
        psubchild = psubchild->NextSibling(); 
      }
	}else {
#if 1
      redirectOut
          << "parse error in AEC section (unknow tag: "
          << tagname
          << ")"
          << std::endl;
#endif

      //return ( false );
    }

    pchild = pchild->NextSibling();
  }

  RESULT result = CamCalibDbAddAecGlobal(m_CalibDbHandle, &aec_data);
  DCT_ASSERT(result == RET_SUCCESS);

  redirectOut << __func__ << " (exit)" << std::endl;

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

  redirectOut << __func__ << " (enter)" << std::endl;

  CamEcmProfile_t EcmProfile;
  MEMSET(&EcmProfile, 0, sizeof(EcmProfile));
  ListInit(&EcmProfile.ecm_scheme);

  const XMLNode* pchild = plement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
    redirectOut << "tag: " << tagname << std::endl;

    if ((tagname == CALIB_SENSOR_AEC_ECM_NAME_TAG)
        && (tag.isType(XmlTag::TAG_TYPE_CHAR))
        && (tag.Size() > 0)) {
      char* value = Toupper(tag.Value());
      strncpy(EcmProfile.name, value, sizeof(EcmProfile.name));
      redirectOut << "value:" << value << std::endl;
      redirectOut << EcmProfile.name << std::endl;
    } else if (tagname == CALIB_SENSOR_AEC_ECM_SCHEMES_TAG) {
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryAecEcmPriorityScheme, &EcmProfile)) {
#if 1
        redirectOut
            << "parse error in ECM section ("
            << tagname
            << ")"
            << std::endl;
#endif

        return (false);
      }
    } else {
#if 1
      redirectOut
          << "parse error in ECM section (unknow tag: "
          << tagname
          << ")"
          << std::endl;
#endif

      return (false);
    }

    pchild = pchild->NextSibling();
  }

  RESULT result = CamCalibDbAddEcmProfile(m_CalibDbHandle, &EcmProfile);
  DCT_ASSERT(result == RET_SUCCESS);

  // free linked ecm_schemes
  List* l = ListRemoveHead(&EcmProfile.ecm_scheme);
  while (l) {
    List* temp = ListRemoveHead(l);
    free(l);
    l = temp;
  }

  redirectOut << __func__ << " (exit)" << std::endl;

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

  redirectOut << __func__ << " (enter)" << std::endl;

  CamEcmScheme_t* pEcmScheme = (CamEcmScheme_t*) malloc(sizeof(CamEcmScheme_t));
  if (!pEcmScheme) {
    return false;
  }
  MEMSET(pEcmScheme, 0, sizeof(*pEcmScheme));

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());

    redirectOut << "tag: " << tagname << std::endl;

    if ((tagname == CALIB_SENSOR_AEC_ECM_SCHEME_NAME_TAG)
        && (tag.isType(XmlTag::TAG_TYPE_CHAR))
        && (tag.Size() > 0)) {
      char* value = Toupper(tag.Value());
      strncpy(pEcmScheme->name, value, sizeof(pEcmScheme->name));
      redirectOut << "value:" << value << std::endl;
      redirectOut << pEcmScheme->name << std::endl;
    } else if ((tagname == CALIB_SENSOR_AEC_ECM_SCHEME_OFFSETT0FAC_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &pEcmScheme->OffsetT0Fac, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AEC_ECM_SCHEME_SLOPEA0_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &pEcmScheme->SlopeA0, 1);
      DCT_ASSERT((no == tag.Size()));
    } else {
#if 1
      redirectOut
          << "parse error in ECM section (unknow tag: "
          << tagname
          << ")"
          << std::endl;
#endif

      free(pEcmScheme);
      pEcmScheme = NULL;

      //return ( false );
    }

    pchild = pchild->NextSibling();
  }

  if (pEcmScheme) {
    ListPrepareItem(pEcmScheme);
    ListAddTail(&pEcmProfile->ecm_scheme, pEcmScheme);
  }

  redirectOut << __func__ << " (exit)" << std::endl;

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

  redirectOut << __func__ << " (enter)" << std::endl;

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());

    redirectOut << "tag: " << tagname << std::endl;

    if (tagname == CALIB_SENSOR_AWB_GLOBALS_TAG) {
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryAwbGlobals)) {
#if 1
        redirectOut
            << "parse error in AWB globals ("
            << tagname
            << ")"
            << std::endl;
#endif

        return (false);
      }
    } else if (tagname == CALIB_SENSOR_AWB_ILLUMINATION_TAG) {
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryAwbIllumination)) {
#if 1
        redirectOut
            << "parse error in AWB illumination ("
            << tagname
            << ")"
            << std::endl;
#endif

        return (false);
      }
    } else {
#if 1
      redirectOut
          << "parse error in AWB section (unknow tag: "
          << tagname
          << ")"
          << std::endl;
#endif

      //return ( false );
    }

    pchild = pchild->NextSibling();
  }

  redirectOut << __func__ << " (exit)" << std::endl;

  return (true);
}


/******************************************************************************
 * CalibDb::parseEntryAwbGlobals
 *****************************************************************************/
bool CalibDb::parseEntryAwbGlobals
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

  redirectOut << __func__ << " (enter)" << std::endl;

  CamCalibAwbGlobal_t awb_data;

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



  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    const char* value = tag.Value();
    std::string tagname(pchild->ToElement()->Name());

    redirectOut << "tag: " << tagname << std::endl;

    if ((tagname == CALIB_SENSOR_AWB_GLOBALS_NAME_TAG)
        && (tag.isType(XmlTag::TAG_TYPE_CHAR))
        && (tag.Size() > 0)) {
      strncpy(awb_data.name, value, sizeof(awb_data.name));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_RESOLUTION_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      strncpy(awb_data.resolution, value, sizeof(awb_data.resolution));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_SENSOR_FILENAME_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      // do nothing
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_SVDMEANVALUE_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0))

    {
      int i = (sizeof(awb_data.SVDMeanValue) / sizeof(awb_data.SVDMeanValue.fCoeff[0]));
      int no = ParseFloatArray(value, awb_data.SVDMeanValue.fCoeff, i);

      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_PCAMATRIX_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(awb_data.PCAMatrix) / sizeof(awb_data.PCAMatrix.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), awb_data.PCAMatrix.fCoeff, i);

      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_CENTERLINE_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(awb_data.CenterLine) / sizeof(awb_data.CenterLine.f_N0_Rg));
      int no = ParseFloatArray(tag.Value(), &awb_data.CenterLine.f_N0_Rg, i);

      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_KFACTOR_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(awb_data.KFactor) / sizeof(awb_data.KFactor.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), awb_data.KFactor.fCoeff, i);

      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_RG1_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pRg1)) {
      nRg1 = tag.Size();
      pRg1 = (float*)malloc(sizeof(float) * nRg1);

      int no = ParseFloatArray(tag.Value(), pRg1, nRg1);
      DCT_ASSERT((no == nRg1));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_MAXDIST1_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pMaxDist1)) {
      nMaxDist1 = tag.Size();
      pMaxDist1 = (float*)malloc(sizeof(float) * nMaxDist1);

      int no = ParseFloatArray(tag.Value(), pMaxDist1, nMaxDist1);
      DCT_ASSERT((no == nRg1));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_RG2_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pRg2)) {
      nRg2 = tag.Size();
      pRg2 = (float*)malloc(sizeof(float) * nRg2);

      int no = ParseFloatArray(tag.Value(), pRg2, nRg2);
      DCT_ASSERT((no == nRg2));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_MAXDIST2_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pMaxDist2)) {
      nMaxDist2 = tag.Size();
      pMaxDist2 = (float*)malloc(sizeof(float) * nMaxDist2);

      int no = ParseFloatArray(tag.Value(), pMaxDist2, nMaxDist2);
      DCT_ASSERT((no == nMaxDist2));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_GLOBALFADE1_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pGlobalFade1)) {
      nGlobalFade1 = tag.Size();
      pGlobalFade1 = (float*)malloc(sizeof(float) * nGlobalFade1);

      int no = ParseFloatArray(tag.Value(), pGlobalFade1, nGlobalFade1);
      DCT_ASSERT((no == nGlobalFade1));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_GLOBALGAINDIST1_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pGlobalGainDistance1)) {
      nGlobalGainDistance1 = tag.Size();
      pGlobalGainDistance1 = (float*)malloc(sizeof(float) * nGlobalGainDistance1);

      int no = ParseFloatArray(tag.Value(), pGlobalGainDistance1, nGlobalGainDistance1);
      DCT_ASSERT((no == nGlobalGainDistance1));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_GLOBALFADE2_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pGlobalFade2)) {
      nGlobalFade2 = tag.Size();
      pGlobalFade2 = (float*)malloc(sizeof(float) * nGlobalFade2);

      int no = ParseFloatArray(tag.Value(), pGlobalFade2, nGlobalFade2);
      DCT_ASSERT((no == nGlobalFade2));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_GLOBALGAINDIST2_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pGlobalGainDistance2)) {
      nGlobalGainDistance2 = tag.Size();
      pGlobalGainDistance2 = (float*)malloc(sizeof(float) * nGlobalGainDistance2);

      int no = ParseFloatArray(tag.Value(), pGlobalGainDistance2, nGlobalGainDistance2);
      DCT_ASSERT((no == nGlobalGainDistance2));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_FADE2_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pFade)) {
      nFade = tag.Size();
      pFade = (float*)malloc(sizeof(float) * nFade);

      int no = ParseFloatArray(tag.Value(), pFade, nFade);
      DCT_ASSERT((no == nFade));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_CB_MIN_REGIONMAX_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pCbMinRegionMax)) {
      nCbMinRegionMax = tag.Size();
      pCbMinRegionMax = (float*)malloc(sizeof(float) * nCbMinRegionMax);

      int no = ParseFloatArray(tag.Value(), pCbMinRegionMax, nCbMinRegionMax);
      DCT_ASSERT((no == nCbMinRegionMax));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_CR_MIN_REGIONMAX_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pCrMinRegionMax)) {
      nCrMinRegionMax = tag.Size();
      pCrMinRegionMax = (float*)malloc(sizeof(float) * nCrMinRegionMax);

      int no = ParseFloatArray(tag.Value(), pCrMinRegionMax, nCrMinRegionMax);
      DCT_ASSERT((no == nCrMinRegionMax));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_MAX_CSUM_REGIONMAX_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pMaxCSumRegionMax)) {
      nMaxCSumRegionMax = tag.Size();
      pMaxCSumRegionMax = (float*)malloc(sizeof(float) * nMaxCSumRegionMax);

      int no = ParseFloatArray(tag.Value(), pMaxCSumRegionMax, nMaxCSumRegionMax);
      DCT_ASSERT((no == nMaxCSumRegionMax));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_CB_MIN_REGIONMIN_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pCbMinRegionMin)) {
      nCbMinRegionMin = tag.Size();
      pCbMinRegionMin = (float*)malloc(sizeof(float) * nCbMinRegionMin);

      int no = ParseFloatArray(tag.Value(), pCbMinRegionMin, nCbMinRegionMin);
      DCT_ASSERT((no == nCbMinRegionMin));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_CR_MIN_REGIONMIN_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pCrMinRegionMin)) {
      nCrMinRegionMin = tag.Size();
      pCrMinRegionMin = (float*)malloc(sizeof(float) * nCrMinRegionMin);

      int no = ParseFloatArray(tag.Value(), pCrMinRegionMin, nCrMinRegionMin);
      DCT_ASSERT((no == nCrMinRegionMin));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_MAX_CSUM_REGIONMIN_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pMaxCSumRegionMin)) {
      nMaxCSumRegionMin = tag.Size();
      pMaxCSumRegionMin = (float*)malloc(sizeof(float) * nMaxCSumRegionMin);

      int no = ParseFloatArray(tag.Value(), pMaxCSumRegionMin, nMaxCSumRegionMin);
      DCT_ASSERT((no == nMaxCSumRegionMin));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_MINC_REGIONMAX_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pMinCRegionMax)) {
      nMinCRegionMax = tag.Size();
      pMinCRegionMax = (float*)malloc(sizeof(float) * nMinCRegionMax);

      int no = ParseFloatArray(tag.Value(), pMinCRegionMax, nMinCRegionMax);
      DCT_ASSERT((no == nMinCRegionMax));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_MINC_REGIONMIN_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pMinCRegionMin)) {
      nMinCRegionMin = tag.Size();
      pMinCRegionMin = (float*)malloc(sizeof(float) * nMinCRegionMin);

      int no = ParseFloatArray(tag.Value(), pMinCRegionMin, nMinCRegionMin);
      DCT_ASSERT((no == nMinCRegionMin));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_MAXY_REGIONMAX_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pMaxYRegionMax)) {
      nMaxYRegionMax = tag.Size();
      pMaxYRegionMax = (float*)malloc(sizeof(float) * nMaxYRegionMax);

      int no = ParseFloatArray(tag.Value(), pMaxYRegionMax, nMaxYRegionMax);
      DCT_ASSERT((no == nMaxYRegionMax));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_MAXY_REGIONMIN_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pMaxYRegionMin)) {
      nMaxYRegionMin = tag.Size();
      pMaxYRegionMin = (float*)malloc(sizeof(float) * nMaxYRegionMin);

      int no = ParseFloatArray(tag.Value(), pMaxYRegionMin, nMaxYRegionMin);
      DCT_ASSERT((no == nMaxYRegionMin));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_MINY_MAXG_REGIONMAX_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pMinYMaxGRegionMax)) {
      nMinYMaxGRegionMax = tag.Size();
      pMinYMaxGRegionMax = (float*)malloc(sizeof(float) * nMinYMaxGRegionMax);

      int no = ParseFloatArray(tag.Value(), pMinYMaxGRegionMax, nMinYMaxGRegionMax);
      DCT_ASSERT((no == nMinYMaxGRegionMax));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_MINY_MAXG_REGIONMIN_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pMinYMaxGRegionMin)) {
      nMinYMaxGRegionMin = tag.Size();
      pMinYMaxGRegionMin = (float*)malloc(sizeof(float) * nMinYMaxGRegionMin);

      int no = ParseFloatArray(tag.Value(), pMinYMaxGRegionMin, nMinYMaxGRegionMin);
      DCT_ASSERT((no == nMinYMaxGRegionMin));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_REFCB_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pRefCb)) {
      nRefCb = tag.Size();
      pRefCb = (float*)malloc(sizeof(float) * nRefCb);

      int no = ParseFloatArray(tag.Value(), pRefCb, nRefCb);
      DCT_ASSERT((no == nRefCb));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_REFCR_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)
               && (NULL == pRefCr)) {
      nRefCr = tag.Size();
      pRefCr = (float*)malloc(sizeof(float) * nRefCr);

      int no = ParseFloatArray(tag.Value(), pRefCr, nRefCr);
      DCT_ASSERT((no == nRefCr));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_INDOOR_MIN_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjIndoorMin, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_OUTDOOR_MIN_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjOutdoorMin, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_MAX_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjMax, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_MAX_SKY_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjMaxSky, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_A_LIMIT) //oyyf
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjALimit, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_YELLOW_LIMIT_ENABLE)
               && (tag.Size() > 0)) {
      int no = ParseUshortArray(tag.Value(), &awb_data.fRgProjYellowLimitEnable, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_A_WEIGHT)    //oyyf
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjAWeight, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_YELLOW_LIMIT)    //oyyf
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjYellowLimit, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_ILL_TO_CWF_ENABLE)
               && (tag.Size() > 0)) {
      int no = ParseUshortArray(tag.Value(), &awb_data.fRgProjIllToCwfEnable, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_ILL_TO_CWF)  //oyyf
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjIllToCwf, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_RGPROJ_ILL_TO_CWF_WEIGHT)   //oyyf
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &awb_data.fRgProjIllToCwfWeight, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_CLIP_OUTDOOR)
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      char* value = Toupper(tag.Value());
      strncpy(awb_data.outdoor_clipping_profile,
              value, sizeof(awb_data.outdoor_clipping_profile));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_REGION_SIZE)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &awb_data.fRegionSize, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_REGION_SIZE_INC)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &awb_data.fRegionSizeInc, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_GLOBALS_REGION_SIZE_DEC)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &awb_data.fRegionSizeDec, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_AWB_GLOBALS_IIR) {
      const XMLNode* psubchild = pchild->ToElement()->FirstChild();
      while (psubchild) {
        XmlTag tag = XmlTag(psubchild->ToElement());
        std::string subTagname(psubchild->ToElement()->Name());

        redirectOut << "subTagname: " << subTagname << std::endl;

        if ((subTagname == CALIB_SENSOR_AWB_GLOBALS_IIR_DAMP_COEF_ADD)
            && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
            && (tag.Size() > 0)) {
          int no = ParseFloatArray(tag.Value(), &awb_data.IIR.fIIRDampCoefAdd, 1);
          DCT_ASSERT((no == tag.Size()));
        } else if ((subTagname == CALIB_SENSOR_AWB_GLOBALS_IIR_DAMP_COEF_SUB)
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          int no = ParseFloatArray(tag.Value(), &awb_data.IIR.fIIRDampCoefSub, 1);
          DCT_ASSERT((no == tag.Size()));
        } else if ((subTagname == CALIB_SENSOR_AWB_GLOBALS_IIR_DAMP_FILTER_THRESHOLD)
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          int no = ParseFloatArray(tag.Value(), &awb_data.IIR.fIIRDampFilterThreshold, 1);
          DCT_ASSERT((no == tag.Size()));
        } else if ((subTagname == CALIB_SENSOR_AWB_GLOBALS_IIR_DAMPING_COEF_MIN)
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          int no = ParseFloatArray(tag.Value(), &awb_data.IIR.fIIRDampingCoefMin, 1);
          DCT_ASSERT((no == tag.Size()));
        } else if ((subTagname == CALIB_SENSOR_AWB_GLOBALS_IIR_DAMPING_COEF_MAX)
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          int no = ParseFloatArray(tag.Value(), &awb_data.IIR.fIIRDampingCoefMax, 1);
          DCT_ASSERT((no == tag.Size()));
        } else if ((subTagname == CALIB_SENSOR_AWB_GLOBALS_IIR_DAMPING_COEF_INIT)
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          int no = ParseFloatArray(tag.Value(), &awb_data.IIR.fIIRDampingCoefInit, 1);
          DCT_ASSERT((no == tag.Size()));
        } else if ((subTagname == CALIB_SENSOR_AWB_GLOBALS_IIR_EXP_PRIOR_FILTER_SIZE_MAX)
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          int no = ParseUshortArray(tag.Value(), &awb_data.IIR.IIRFilterSize, 1);
          DCT_ASSERT((no == tag.Size()));
        } else if ((subTagname == CALIB_SENSOR_AWB_GLOBALS_IIR_EXP_PRIOR_FILTER_SIZE_MIN)
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
        } else if ((subTagname == CALIB_SENSOR_AWB_GLOBALS_IIR_EXP_PRIOR_MIDDLE)
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          int no = ParseFloatArray(tag.Value(), &awb_data.IIR.fIIRFilterInitValue, 1);
          DCT_ASSERT((no == tag.Size()));
        } else {
#if 1
          redirectOut
              << "parse error in AWB GLOBALS - IIR section (unknow tag: "
              << subTagname
              << ")"
              << std::endl;
#endif
          return (false);
        }

        psubchild = psubchild->NextSibling();
      }
    } else {
#if 1
      redirectOut
          << "parse error in AWB section (unknow tag: "
          << tagname
          << ")"
          << std::endl;
#endif
      return (false);
    }

    pchild = pchild->NextSibling();
  }

  redirectOut << __func__ << "end pchild" << std::endl;

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
  awb_data.AwbFade2Parm.pMaxCSumRegionMin = pMaxCSumRegionMin;
  awb_data.AwbFade2Parm.pMinCRegionMax  = pMinCRegionMax;
  awb_data.AwbFade2Parm.pMinCRegionMin  = pMinCRegionMax;
  awb_data.AwbFade2Parm.pMaxYRegionMax    = pMaxYRegionMax;
  awb_data.AwbFade2Parm.pMaxYRegionMin    = pMaxYRegionMin;
  awb_data.AwbFade2Parm.pMinYMaxGRegionMax = pMinYMaxGRegionMax;
  awb_data.AwbFade2Parm.pMinYMaxGRegionMin = pMinYMaxGRegionMin;
  awb_data.AwbFade2Parm.pRefCb = pRefCb;
  awb_data.AwbFade2Parm.pRefCr = pRefCr;
  redirectOut << __func__ << "CamCalibDbAddAwbGlobal" << std::endl;

  RESULT result = CamCalibDbAddAwbGlobal(m_CalibDbHandle, &awb_data);
  DCT_ASSERT(result == RET_SUCCESS);
  redirectOut << __func__ << "CamCalibDbAddAwbGlobal exit" << std::endl;

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

  redirectOut << __func__ << " (exit)" << std::endl;

  return (true);
}



/******************************************************************************
 * CalibDb::parseEntryAwbIllumination
 *****************************************************************************/
bool CalibDb::parseEntryAwbIllumination
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

  redirectOut << __func__ << " (enter)" << std::endl;

  CamIlluProfile_t illu;
  MEMSET(&illu, 0, sizeof(illu));

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());

    redirectOut << "tagname: " << tagname << std::endl;

    if ((tagname == CALIB_SENSOR_AWB_ILLUMINATION_NAME_TAG)
        && (tag.isType(XmlTag::TAG_TYPE_CHAR))
        && (tag.Size() > 0)) {
      char* value = Toupper(tag.Value());
      strncpy(illu.name, value, sizeof(illu.name));
      redirectOut << value << std::endl;
    } else if ((tagname == CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      char* value = Toupper(tag.Value());
      std::string s_value(value);
      redirectOut << "value:" << value << std::endl;
      redirectOut << s_value << std::endl;
      if (s_value == CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_INDOOR) {
        illu.DoorType = CAM_DOOR_TYPE_INDOOR;
      } else if (s_value == CALIB_SENSOR_AWB_ILLUMINATION_DOOR_TYPE_OUTDOOR) {
        illu.DoorType = CAM_DOOR_TYPE_OUTDOOR;
      } else {
        redirectOut << "invalid illumination doortype (" << s_value << ")" << std::endl;
      }

    } else if ((tagname == CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      char* value = Toupper(tag.Value());
      std::string s_value(value);
      redirectOut << "value:" << value << std::endl;
      redirectOut << s_value << std::endl;
      if (s_value == CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_MANUAL) {
        illu.AwbType = CAM_AWB_TYPE_MANUAL;
      } else if (s_value == CALIB_SENSOR_AWB_ILLUMINATION_AWB_TYPE_AUTO) {
        illu.AwbType = CAM_AWB_TYPE_AUTO;
      } else {
        redirectOut << "invalid AWB type (" << s_value << ")" << std::endl;
      }

    } else if ((tagname == CALIB_SENSOR_AWB_ILLUMINATION_MANUAL_WB_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(illu.ComponentGain) / sizeof(illu.ComponentGain.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), illu.ComponentGain.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_ILLUMINATION_MANUAL_CC_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(illu.CrossTalkCoeff) / sizeof(illu.CrossTalkCoeff.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), illu.CrossTalkCoeff.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_AWB_ILLUMINATION_MANUAL_CTO_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(illu.CrossTalkOffset) / sizeof(illu.CrossTalkOffset.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), illu.CrossTalkOffset.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_AWB_ILLUMINATION_GMM_TAG) {
      const XMLNode* psubchild = pchild->ToElement()->FirstChild();
      while (psubchild) {
        XmlTag tag = XmlTag(psubchild->ToElement());
        std::string subTagname(psubchild->ToElement()->Name());

        redirectOut << "subTagname: " << subTagname << std::endl;

        if ((subTagname == CALIB_SENSOR_AWB_ILLUMINATION_GMM_GAUSSIAN_MVALUE_TAG)
            && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
            && (tag.Size() > 0)) {
          int i = (sizeof(illu.GaussMeanValue) / sizeof(illu.GaussMeanValue.fCoeff[0]));
          int no = ParseFloatArray(tag.Value(), illu.GaussMeanValue.fCoeff, i);
          DCT_ASSERT((no == tag.Size()));
        } else if ((subTagname == CALIB_SENSOR_AWB_ILLUMINATION_GMM_INV_COV_MATRIX_TAG)
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          int i = (sizeof(illu.CovarianceMatrix) / sizeof(illu.CovarianceMatrix.fCoeff[0]));
          int no = ParseFloatArray(tag.Value(), illu.CovarianceMatrix.fCoeff, i);
          DCT_ASSERT((no == tag.Size()));
        } else if ((subTagname == CALIB_SENSOR_AWB_ILLUMINATION_GMM_GAUSSIAN_SFACTOR_TAG)
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          int i = (sizeof(illu.GaussFactor) / sizeof(illu.GaussFactor.fCoeff[0]));
          int no = ParseFloatArray(tag.Value(), illu.GaussFactor.fCoeff, i);
          DCT_ASSERT((no == tag.Size()));
        } else if ((subTagname == CALIB_SENSOR_AWB_ILLUMINATION_GMM_TAU_TAG)
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          int i = (sizeof(illu.Threshold) / sizeof(illu.Threshold.fCoeff[0]));
          int no = ParseFloatArray(tag.Value(), illu.Threshold.fCoeff, i);
          DCT_ASSERT((no == tag.Size()));
        } else {
#if 1
          redirectOut
              << "parse error in AWB gaussian mixture modell section (unknow tag: "
              << subTagname
              << ")"
              << std::endl;
#endif
          return (false);
        }

        psubchild = psubchild->NextSibling();
      }
    } else if (tagname == CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_TAG) {
      float* afGain   = NULL;
      int n_gains     = 0;
      float* afSat    = NULL;
      int n_sats      = 0;

      const XMLNode* psubchild = pchild->ToElement()->FirstChild();
      while (psubchild) {
        XmlTag tag = XmlTag(psubchild->ToElement());
        std::string subTagname(psubchild->ToElement()->Name());

        redirectOut << "subTagname: " << subTagname << std::endl;

        if ((subTagname == CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_GAIN_TAG)
            && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
            && (tag.Size() > 0)) {
          if (!afGain) {
            n_gains = tag.Size();
            afGain  = (float*)malloc((n_gains * sizeof(float)));
            MEMSET(afGain, 0, (n_gains * sizeof(float)));
          }

          int no = ParseFloatArray(tag.Value(), afGain, n_gains);
          DCT_ASSERT((no == n_gains));
        } else if ((subTagname == CALIB_SENSOR_AWB_ILLUMINATION_SAT_CT_SAT_TAG)
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          if (!afSat) {
            n_sats = tag.Size();
            afSat = (float*)malloc((n_sats * sizeof(float)));
            MEMSET(afSat, 0, (n_sats * sizeof(float)));
          }

          int no = ParseFloatArray(tag.Value(), afSat, n_sats);
          DCT_ASSERT((no == n_sats));
        } else {
#if 1
          redirectOut
              << "parse error in AWB saturation curve section (unknow tag: "
              << subTagname
              << ")"
              << std::endl;
#endif
          return (false);
        }

        psubchild = psubchild->NextSibling();
      }

      DCT_ASSERT((n_gains == n_sats));
      illu.SaturationCurve.ArraySize      = n_gains;
      illu.SaturationCurve.pSensorGain    = afGain;
      illu.SaturationCurve.pSaturation    = afSat;
    } else if (tagname == CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_TAG) {
      float* afGain   = NULL;
      int n_gains     = 0;
      float* afVig    = NULL;
      int n_vigs      = 0;

      const XMLNode* psubchild = pchild->ToElement()->FirstChild();
      while (psubchild) {
        XmlTag tag = XmlTag(psubchild->ToElement());
        std::string subTagname(psubchild->ToElement()->Name());

        redirectOut << "subTagname: " << subTagname << std::endl;

        if ((subTagname == CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_GAIN_TAG)
            && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
            && (tag.Size() > 0)) {
          if (!afGain) {
            n_gains = tag.Size();
            afGain  = (float*)malloc((n_gains * sizeof(float)));
            MEMSET(afGain, 0, (n_gains * sizeof(float)));
          }

          int no = ParseFloatArray(tag.Value(), afGain, n_gains);
          DCT_ASSERT((no == n_gains));
        } else if ((subTagname == CALIB_SENSOR_AWB_ILLUMINATION_VIG_CT_VIG_TAG)
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          if (!afVig) {
            n_vigs = tag.Size();
            afVig = (float*)malloc((n_vigs * sizeof(float)));
            MEMSET(afVig, 0, (n_vigs * sizeof(float)));
          }

          int no = ParseFloatArray(tag.Value(), afVig, n_vigs);
          DCT_ASSERT((no == n_vigs));
        } else {
#if 1
          redirectOut
              << "parse error in AWB vignetting curve section (unknow tag: "
              << subTagname
              << ")"
              << std::endl;
#endif
          return (false);
        }

        psubchild = psubchild->NextSibling();
      }

      DCT_ASSERT((n_gains == n_vigs));
      illu.VignettingCurve.ArraySize      = n_gains;
      illu.VignettingCurve.pSensorGain    = afGain;
      illu.VignettingCurve.pVignetting    = afVig;
    } else if (tagname == CALIB_SENSOR_AWB_ILLUMINATION_ALSC_TAG) {
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryAwbIlluminationAlsc, &illu)) {
#if 1
        redirectOut
            << "parse error in AWB aLSC ("
            << tagname
            << ")"
            << std::endl;
#endif

        return (false);
      }
    } else if (tagname == CALIB_SENSOR_AWB_ILLUMINATION_ACC_TAG) {
      const XMLNode* psubchild = pchild->ToElement()->FirstChild();
      while (psubchild) {
        XmlTag tag = XmlTag(psubchild->ToElement());
        std::string subTagname(psubchild->ToElement()->Name());

        redirectOut << "subTagname: " << subTagname << std::endl;

        if ((subTagname == CALIB_SENSOR_AWB_ILLUMINATION_ACC_CC_PROFILE_LIST_TAG)
            && (tag.isType(XmlTag::TAG_TYPE_CHAR))
            && (tag.Size() > 0)) {
          char* value = Toupper(tag.Value());
          int no = ParseCcProfileArray(value, illu.cc_profiles, CAM_NO_CC_PROFILES);
          DCT_ASSERT((no <= CAM_NO_CC_PROFILES));
          illu.cc_no = no;
        } else {
#if 1
          redirectOut
              << "parse error in AWB aCC ("
              << subTagname
              << ")"
              << std::endl;
#endif
          return (false);
        }

        psubchild = psubchild->NextSibling();
      }
    } else {
#if 1
      redirectOut
          << "parse error in AWB illumination section (unknow tag: "
          << tagname
          << ")"
          << std::endl;
#endif
      return (false);
    }

    pchild = pchild->NextSibling();
  }

  RESULT result = CamCalibDbAddIllumination(m_CalibDbHandle, &illu);
  DCT_ASSERT(result == RET_SUCCESS);

  /* cleanup */
  free(illu.SaturationCurve.pSensorGain);
  free(illu.SaturationCurve.pSaturation);
  free(illu.VignettingCurve.pSensorGain);
  free(illu.VignettingCurve.pVignetting);

  redirectOut << __func__ << " (exit)" << std::endl;

  return (true);
}



/******************************************************************************
 * CalibDb::parseEntryAwbIlluminationAlsc
 *****************************************************************************/
bool CalibDb::parseEntryAwbIlluminationAlsc
(
    const XMLElement*   pelement,
    void*                param
) {
  redirectOut << __func__ << " (enter)" << std::endl;

  if (!param) {
    return (false);
  }

  CamIlluProfile_t* pIllu = (CamIlluProfile_t*)param;

  char* lsc_profiles;
  int resIdx = -1;

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());

    redirectOut << "tagname: " << tagname << std::endl;

    if (tagname == CALIB_SENSOR_AWB_ILLUMINATION_ALSC_RES_LSC_PROFILE_LIST_TAG) {
      lsc_profiles = Toupper(tag.Value());
    } else if (tagname == CALIB_SENSOR_AWB_ILLUMINATION_ALSC_RES_TAG) {
      const char* value = tag.Value();
      redirectOut << "value:" << value << "..." << std::endl;
      RESULT result = CamCalibDbGetResolutionIdxByName(m_CalibDbHandle, value, &resIdx);
      DCT_ASSERT(result == RET_SUCCESS);
    } else {
      redirectOut << "unknown aLSC tag: " << tagname << std::endl;
      return (false);
    }

    pchild = pchild->NextSibling();
  }

  DCT_ASSERT(resIdx != -1);

  int no = ParseLscProfileArray(lsc_profiles, pIllu->lsc_profiles[resIdx], CAM_NO_LSC_PROFILES);
  DCT_ASSERT((no <= CAM_NO_LSC_PROFILES));
  pIllu->lsc_no[resIdx] = no;

  pIllu->lsc_res_no++;

  redirectOut << __func__ << " (exit)" << std::endl;

  return (true);
}



/******************************************************************************
 * CalibDb::parseEntryAwbIlluminationAcc
 *****************************************************************************/
bool CalibDb::parseEntryAwbIlluminationAcc
(
    const XMLElement*   pelement,
    void*                param
) {
  (void)param;

  redirectOut << __func__ << " (enter)" << std::endl;

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());

    redirectOut << "tagname: " << tagname << std::endl;

    if (tagname == CALIB_SENSOR_AWB_ILLUMINATION_ACC_CC_PROFILE_LIST_TAG) {
    } else {
      redirectOut << "unknown aCC tag: " << tagname << std::endl;
      return (false);
    }

    pchild = pchild->NextSibling();
  }

  redirectOut << __func__ << " (exit)" << std::endl;

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
  redirectOut << __func__ << " (enter)" << std::endl;

  CamLscProfile_t lsc_profile;
  MEMSET(&lsc_profile, 0, sizeof(lsc_profile));

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());

    redirectOut << "tagname: " << tagname << std::endl;

    if ((tagname == CALIB_SENSOR_LSC_PROFILE_NAME_TAG)
        && (tag.isType(XmlTag::TAG_TYPE_CHAR))
        && (tag.Size() > 0)) {
      char* value = Toupper(tag.Value());
      strncpy(lsc_profile.name, value, sizeof(lsc_profile.name));
      redirectOut << "value:" << value << std::endl;
      redirectOut << lsc_profile.name << std::endl;
    } else if ((tagname == CALIB_SENSOR_LSC_PROFILE_RESOLUTION_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      const char* value = tag.Value();
      strncpy(lsc_profile.resolution, value, sizeof(lsc_profile.resolution));
      redirectOut << "value:" << value << std::endl;
      redirectOut << lsc_profile.resolution << std::endl;
    } else if ((tagname == CALIB_SENSOR_LSC_PROFILE_ILLUMINATION_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      char* value = Toupper(tag.Value());
      strncpy(lsc_profile.illumination, value, sizeof(lsc_profile.illumination));
      redirectOut << "value:" << value << std::endl;
      redirectOut << lsc_profile.illumination << std::endl;
    } else if ((tagname == CALIB_SENSOR_LSC_PROFILE_LSC_SECTORS_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseUshortArray(tag.Value(), &lsc_profile.LscSectors, 1);
      DCT_ASSERT((no == 1));
    } else if ((tagname == CALIB_SENSOR_LSC_PROFILE_LSC_NO_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseUshortArray(tag.Value(), &lsc_profile.LscNo, 1);
      DCT_ASSERT((no == 1));
    } else if ((tagname == CALIB_SENSOR_LSC_PROFILE_LSC_XO_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseUshortArray(tag.Value(), &lsc_profile.LscXo, 1);
      DCT_ASSERT((no == 1));
    } else if (tagname == CALIB_SENSOR_LSC_PROFILE_LSC_YO_TAG) {
      int no = ParseUshortArray(tag.Value(), &lsc_profile.LscYo, 1);
      DCT_ASSERT((no == 1));
    } else if ((tagname == CALIB_SENSOR_LSC_PROFILE_LSC_SECTOR_SIZE_X_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(lsc_profile.LscXSizeTbl) / sizeof(lsc_profile.LscXSizeTbl[0]));
      int no = ParseUshortArray(tag.Value(), lsc_profile.LscXSizeTbl, i);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_LSC_PROFILE_LSC_SECTOR_SIZE_Y_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(lsc_profile.LscYSizeTbl) / sizeof(lsc_profile.LscYSizeTbl[0]));
      int no = ParseUshortArray(tag.Value(), lsc_profile.LscYSizeTbl, i);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_LSC_PROFILE_LSC_VIGNETTING_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), (float*)(&lsc_profile.vignetting), 1);
      DCT_ASSERT((no == 1));
    } else if ((tagname == CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_RED_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_RED])
               / sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff[0]));
      int no = ParseUshortArray(tag.Value(),
                                (lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_RED].uCoeff), i);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_GREENR_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR])
               / sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff[0]));
      int no = ParseUshortArray(tag.Value(),
                                lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENR].uCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_GREENB_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB])
               / sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff[0]));
      int no = ParseUshortArray(tag.Value(),
                                (uint16_t*)(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_GREENB].uCoeff), i);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_LSC_PROFILE_LSC_SAMPLES_BLUE_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE])
               / sizeof(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff[0]));
      int no = ParseUshortArray(tag.Value(),
                                (uint16_t*)(lsc_profile.LscMatrix[CAM_4CH_COLOR_COMPONENT_BLUE].uCoeff), i);
      DCT_ASSERT((no == tag.Size()));
    } else {
#if 1
      redirectOut
          << "parse error in LSC section (unknow tag: "
          << tagname
          << ")"
          << std::endl;
#endif

      return (false);
    }

    pchild = pchild->NextSibling();
  }

  RESULT result = CamCalibDbAddLscProfile(m_CalibDbHandle, &lsc_profile);
  DCT_ASSERT(result == RET_SUCCESS);

  redirectOut << __func__ << " (exit)" << std::endl;

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

  redirectOut << __func__ << " (enter)" << std::endl;

  CamCcProfile_t cc_profile;
  MEMSET(&cc_profile, 0, sizeof(cc_profile));

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());

    redirectOut << "tagname: " << tagname << std::endl;

    if ((tagname == CALIB_SENSOR_CC_PROFILE_NAME_TAG)
        && (tag.isType(XmlTag::TAG_TYPE_CHAR))
        && (tag.Size() > 0)) {
      char* value = Toupper(tag.Value());
      strncpy(cc_profile.name, value, sizeof(cc_profile.name));
      redirectOut << "value:" << value << std::endl;
      redirectOut << cc_profile.name << std::endl;
    } else if ((tagname == CALIB_SENSOR_CC_PROFILE_SATURATION_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &cc_profile.saturation, 1);
      DCT_ASSERT((no == 1));
    } else if ((tagname == CALIB_SENSOR_CC_PROFILE_CC_MATRIX_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(cc_profile.CrossTalkCoeff) / sizeof(cc_profile.CrossTalkCoeff.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), cc_profile.CrossTalkCoeff.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_CC_PROFILE_CC_OFFSETS_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(cc_profile.CrossTalkOffset) / sizeof(cc_profile.CrossTalkOffset.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), cc_profile.CrossTalkOffset.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SENSOR_CC_PROFILE_WB_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(cc_profile.ComponentGain) / sizeof(cc_profile.ComponentGain.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), cc_profile.ComponentGain.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else {
#if 1
      redirectOut
          << "parse error in CC section (unknow tag: "
          << tagname
          << ")"
          << std::endl;
#endif

      return (false);
    }

    pchild = pchild->NextSibling();
  }

  RESULT result = CamCalibDbAddCcProfile(m_CalibDbHandle, &cc_profile);
  DCT_ASSERT(result == RET_SUCCESS);

  redirectOut << __func__ << " (exit)" << std::endl;

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

  redirectOut << __func__ << " (enter)" << std::endl;

  CamBlsProfile_t bls_profile;
  MEMSET(&bls_profile, 0, sizeof(bls_profile));

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());

    redirectOut << "tagname: " << tagname << std::endl;

    if ((tagname == CALIB_SENSOR_BLS_NAME_TAG)
        && (tag.isType(XmlTag::TAG_TYPE_CHAR))
        && (tag.Size() > 0)) {
      char* value = Toupper(tag.Value());
      strncpy(bls_profile.name, value, sizeof(bls_profile.name));
      redirectOut << "value:" << value << std::endl;
      redirectOut << bls_profile.name << std::endl;
    } else if ((tagname == CALIB_SENSOR_BLS_RESOLUTION_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      const char* value = tag.Value();
      strncpy(bls_profile.resolution, value, sizeof(bls_profile.resolution));
      redirectOut << "value:" << value << std::endl;
      redirectOut << bls_profile.resolution << std::endl;
    } else if ((tagname == CALIB_SENSOR_BLS_DATA_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(bls_profile.level) / sizeof(bls_profile.level.uCoeff[0]));
      int no = ParseUshortArray(tag.Value(), bls_profile.level.uCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else {
#if 1
      redirectOut
          << "parse error in BLS section (unknow tag: "
          << tagname
          << ")"
          << std::endl;
#endif

      return (false);
    }

    pchild = pchild->NextSibling();
  }

  RESULT result = CamCalibDbAddBlsProfile(m_CalibDbHandle, &bls_profile);
  DCT_ASSERT(result == RET_SUCCESS);

  redirectOut << __func__ << " (exit)" << std::endl;

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

  redirectOut << __func__ << " (enter)" << std::endl;

  CamCacProfile_t cac_profile;
  MEMSET(&cac_profile, 0, sizeof(cac_profile));

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());

    redirectOut << "tagname: " << tagname << std::endl;

    if ((tagname == CALIB_SENSOR_CAC_NAME_TAG)
        && (tag.isType(XmlTag::TAG_TYPE_CHAR))
        && (tag.Size() > 0)) {
      char* value = Toupper(tag.Value());
      strncpy(cac_profile.name, value, sizeof(cac_profile.name));
      redirectOut << "value:" << value << std::endl;
      redirectOut << cac_profile.name << std::endl;
    } else if ((tagname == CALIB_SENSOR_CAC_RESOLUTION_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      const char* value = tag.Value();
      strncpy(cac_profile.resolution, value, sizeof(cac_profile.resolution));
      redirectOut << "value:" << value << std::endl;
      redirectOut << cac_profile.resolution << std::endl;
    } else if ((tagname == CALIB_SESNOR_CAC_X_NORMSHIFT_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseByteArray(tag.Value(), &cac_profile.x_ns, 1);
      DCT_ASSERT((no == 1));
    } else if ((tagname == CALIB_SESNOR_CAC_X_NORMFACTOR_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseByteArray(tag.Value(), &cac_profile.x_nf, 1);
      DCT_ASSERT((no == 1));
    } else if ((tagname == CALIB_SESNOR_CAC_Y_NORMSHIFT_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseByteArray(tag.Value(), &cac_profile.y_ns, 1);
      DCT_ASSERT((no == 1));
    } else if ((tagname == CALIB_SESNOR_CAC_Y_NORMFACTOR_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseByteArray(tag.Value(), &cac_profile.y_nf, 1);
      DCT_ASSERT((no == 1));
    } else if ((tagname == CALIB_SESNOR_CAC_X_OFFSET_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseShortArray(tag.Value(), &cac_profile.hCenterOffset, 1);
      DCT_ASSERT((no == 1));
    } else if ((tagname == CALIB_SESNOR_CAC_Y_OFFSET_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseShortArray(tag.Value(), &cac_profile.vCenterOffset, 1);
      DCT_ASSERT((no == 1));
    } else if ((tagname == CALIB_SESNOR_CAC_RED_PARAMETERS_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(cac_profile.Red) / sizeof(cac_profile.Red.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), cac_profile.Red.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if ((tagname == CALIB_SESNOR_CAC_BLUE_PARAMETERS_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int i = (sizeof(cac_profile.Blue) / sizeof(cac_profile.Blue.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), cac_profile.Blue.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else {
#if 1
      redirectOut
          << "parse error in CAC section (unknow tag: "
          << tagname
          << ")"
          << std::endl;
#endif

      return (false);
    }

    pchild = pchild->NextSibling();
  }

  RESULT result = CamCalibDbAddCacProfile(m_CalibDbHandle, &cac_profile);
  DCT_ASSERT(result == RET_SUCCESS);

  redirectOut << __func__ << " (exit)" << std::endl;

  return (true);
}

bool CalibDb::parseEntryFilter
(
    const XMLElement* plement,
    void* param
)
{
  redirectOut << __func__ << " (enter)" << std::endl;
  CamDpfProfile_t *pdpf_profile = (CamDpfProfile_t *)param;
  if(NULL == pdpf_profile){
	redirectOut << __func__ << " Invalid pointer (exit)" << std::endl;
    return false;
  }
  
  CamFilterProfile_t* pFilter = (CamFilterProfile_t*)malloc(sizeof(CamFilterProfile_t));
  if (NULL == pFilter) {
  	redirectOut << __func__ << " malloc fail (exit)" << std::endl;
    return false;
  }
  MEMSET(pFilter, 0, sizeof(*pFilter));
	 
  const XMLNode* pchild = plement->FirstChild();
  while (pchild) {				
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name()); 
    redirectOut  << "subTagname: " << tagname << std::endl;

	if(tagname == CALIB_SENSOR_DPF_FILTERSETTING_NAME_TAG
		 && (tag.isType(XmlTag::TAG_TYPE_CHAR))
		 && (tag.Size() > 0))
	{
	  char* value = Toupper(tag.Value());
      strncpy(pFilter->name, value, sizeof(pFilter->name));
      redirectOut << "value:" << value << std::endl;
      redirectOut << pFilter->name << std::endl;
	}
    else if(tagname == CALIB_SENSOR_DPF_FILT_LEVEL_REG_CONF_TAG
		 && (tag.isType(XmlTag::TAG_TYPE_STRUCT))
		 && (tag.Size() > 0))
    {
  	  const XMLNode* psubchild = pchild->ToElement()->FirstChild();
  	  redirectOut << "filter subTagname: " << tagname << std::endl;
  	  while (psubchild) 
  	  {
		XmlTag tag = XmlTag(psubchild->ToElement());
		std::string subTagname(psubchild->ToElement()->Name());
		redirectOut << "filter subTagname: " << subTagname << std::endl;

		if ((subTagname == CALIB_SENSOR_DPF_FILT_LEVEL_REG_CONF_ENABLE_TAG) 
			&& (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
			&& (tag.Size() > 0))
		{
			int no = ParseUcharArray(tag.Value(), &pFilter->FiltLevelRegConf.FiltLevelRegConfEnable, 1);
			DCT_ASSERT((no == 1));
		}else if ((subTagname == CALIB_SENSOR_DPF_FILT_LEVEL_TAG)	
			&& (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
			&& (tag.Size() > 0))
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
					redirectOut << "filter level only support 10 level," 
						<< p_FiltLevel[i] << " > 10 is invalid: " << std::endl;
				}
			}
		}else if ((subTagname == CALIB_SENSOR_DPF_FILT_GRN_STAGE1_TAG)	
			&& (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
			&& (tag.Size() > 0))
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
					redirectOut << "grn_stage1 only support 0-8," 
						<< p_grn_stage1[i] << " > 8 is invalid: " << std::endl;
					p_grn_stage1[i] = 8;
				}
			}
		}else if ((subTagname == CALIB_SENSOR_DPF_FILT_CHR_H_MODE_TAG)	
			&& (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
			&& (tag.Size() > 0))
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
					redirectOut << "chr_h_mode only support 0-3," 
						<< p_chr_h_mode[i] << " > 3 is invalid: " << std::endl;
					p_chr_h_mode[i] = 3;
				}
			}
		}else if ((subTagname == CALIB_SENSOR_DPF_FILT_CHR_V_MODE_TAG)	
			&& (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
			&& (tag.Size() > 0))
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
					redirectOut << "chr_v_mode only support 0-3," 
						<< p_chr_v_mode[i] << " > 3 is invalid: " << std::endl;
					p_chr_v_mode[i] = 3;
				}
			}
		}else if ((subTagname == CALIB_SENSOR_DPF_FILT_THRESH_BL0_TAG)	
			&& (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
			&& (tag.Size() > 0))
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
					redirectOut << "thresh_bl0 only support 0-1023," 
						<< p_thresh_bl0[i] << " > 1023 is invalid: " << std::endl;
					p_thresh_bl0[i] = 1023;
				}
			}
		}else if ((subTagname == CALIB_SENSOR_DPF_FILT_THRESH_BL1_TAG)	
			&& (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
			&& (tag.Size() > 0))
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
					redirectOut << "thresh_bl1 only support 0-1023," 
						<< p_thresh_bl1[i] << " > 1023 is invalid: " << std::endl;
					p_thresh_bl1[i] = 1023;
				}
			}
		}else if ((subTagname == CALIB_SENSOR_DPF_FILT_FAC_BL0_TAG) 
			&& (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
			&& (tag.Size() > 0))
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
					redirectOut << "fac_bl0 only support 0-63," 
						<< p_fac_bl0[i] << " > 63 is invalid: " << std::endl;
					p_fac_bl0[i] = 63;
				}
			}
		}else if ((subTagname == CALIB_SENSOR_DPF_FILT_FAC_BL1_TAG) 
			&& (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
			&& (tag.Size() > 0))
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
					redirectOut << "fac_bl1 only support 0-63," 
						<< p_fac_bl1[i] << " > 63 is invalid: " << std::endl;
					p_fac_bl1[i] = 63;
				}
			}
		}else if ((subTagname == CALIB_SENSOR_DPF_FILT_THRESH_SH0_TAG)	
			&& (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
			&& (tag.Size() > 0))
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
					redirectOut << "thresh_sh0 only support 0-1023," 
						<< p_thresh_sh0[i] << " > 1023 is invalid: " << std::endl;
					p_thresh_sh0[i] = 1023;
				}
			}
		}else if ((subTagname == CALIB_SENSOR_DPF_FILT_THRESH_SH1_TAG)	
			&& (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
			&& (tag.Size() > 0))
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
					redirectOut << "thresh_sh1 only support 0-1023," 
						<< p_thresh_sh1[i] << " > 1023 is invalid: " << std::endl;
					p_thresh_sh1[i] = 1023;
				}
			}
		}else if ((subTagname == CALIB_SENSOR_DPF_FILT_FAC_SH0_TAG) 
			&& (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
			&& (tag.Size() > 0))
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
					redirectOut << "fac_sh0 only support 0-63," 
						<< p_fac_sh0[i] << " > 63 is invalid: " << std::endl;
					p_fac_sh0[i] = 63;
				}
			}
		}else if ((subTagname == CALIB_SENSOR_DPF_FILT_FAC_SH1_TAG) 
			&& (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
			&& (tag.Size() > 0))
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
					redirectOut << "fac_sh1 only support 0-63," 
						<< p_fac_sh1[i] << " > 63 is invalid: " << std::endl;
					p_fac_sh1[i] = 63;
				}
			}
		}else if ((subTagname == CALIB_SENSOR_DPF_FILT_FAC_MID_TAG) 
			&& (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
			&& (tag.Size() > 0))
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
					redirectOut << "fac_mid only support 0-63," 
						<< p_fac_mid[i] << " > 63 is invalid: " << std::endl;
					p_fac_mid[i] = 63;
				}
			}
		}else {
			redirectOut
			<< "parse error in filter level reg conf section (unknow tag: "
			<< subTagname
			<< ")"
			<< std::endl;
		}
		psubchild = psubchild->NextSibling();
	  }
	}		 
    else if (tagname == CALIB_SENSOR_DPF_FILTERENABLE_TAG
    		 && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
    		 && (tag.Size() > 0)) {
      int no = ParseFloatArray(tag.Value(), &pFilter->FilterEnable, 1);
      redirectOut << "dpf filterenable: " << tag.Value() << pFilter->FilterEnable << std::endl;
      DCT_ASSERT((no == tag.Size()));
    
    } else if (tagname == CALIB_SENSOR_DPF_DENOISELEVEL_TAG) {
      float* afGain   = NULL;
      int n_gains	  = 0;
      float* afDlevel	 = NULL;
      int n_Dlevels 	 = 0;
      int index = 0;
      const XMLNode* psubchild = pchild->ToElement()->FirstChild();
      while (psubchild) {
    	XmlTag tag = XmlTag(psubchild->ToElement());
    	std::string subTagname(psubchild->ToElement()->Name());
    
    	redirectOut << "subTagname: " << subTagname << std::endl;
    
    	if ((subTagname == CALIB_SENSOR_DPF_DENOISELEVEL_GAINS_TAG)
    		&& (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
    		&& (tag.Size() > 0)) {
    	  if (!afGain) {
    		n_gains = tag.Size();
    		afGain	= (float*)malloc((n_gains * sizeof(float)));
    		MEMSET(afGain, 0, (n_gains * sizeof(float)));
    	  }
    
    	  int no = ParseFloatArray(tag.Value(), afGain, n_gains);
    	  DCT_ASSERT((no == n_gains));
    	} else if ((subTagname == CALIB_SENSOR_DPF_DENOISELEVEL_DLEVEL_TAG)
    			   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
    			   && (tag.Size() > 0)) {
    	  if (!afDlevel) {
    		n_Dlevels = tag.Size();
    		afDlevel = (float*)malloc((n_Dlevels * sizeof(float)));
    		MEMSET(afDlevel, 0, (n_Dlevels * sizeof(float)));
    	  }
    
    	  int no = ParseFloatArray(tag.Value(), afDlevel, n_Dlevels);
    	  DCT_ASSERT((no == n_Dlevels));
    	} else {
    	  redirectOut
    		  << "parse error in Dpf denoiselevel curve section (unknow tag: "
    		  << subTagname
    		  << ")"
    		  << std::endl;
    	}
    
    	psubchild = psubchild->NextSibling();
      }
  
      DCT_ASSERT((n_gains == n_Dlevels));
      pFilter->DenoiseLevelCurve.ArraySize	   = n_gains;
      pFilter->DenoiseLevelCurve.pSensorGain    = afGain;
      pFilter->DenoiseLevelCurve.pDlevel = (CamerIcIspFltDeNoiseLevel_t*)malloc((n_Dlevels * sizeof(CamerIcIspFltDeNoiseLevel_t)));
    
      for (index = 0; index < pFilter->DenoiseLevelCurve.ArraySize; index++) {
    	pFilter->DenoiseLevelCurve.pDlevel[index] = (CamerIcIspFltDeNoiseLevel_t)((int)afDlevel[index] + 1);
      }
    
      free(afDlevel);
    } 
    else if (tagname == CALIB_SENSOR_DPF_SHARPENINGLEVEL_TAG) {
      float* afGain   = NULL;
      int n_gains	  = 0;
      float* afSlevel	 = NULL;
      int n_Slevels 	 = 0;
      int index = 0;
      const XMLNode* psubchild = pchild->ToElement()->FirstChild();
      while (psubchild) {
    	XmlTag tag = XmlTag(psubchild->ToElement());
    	std::string subTagname(psubchild->ToElement()->Name());
    
    	redirectOut << "subTagname: " << subTagname << std::endl;
    
    	if ((subTagname == CALIB_SENSOR_DPF_SHARPENINGLEVEL_GAINS_TAG)
    		&& (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
    		&& (tag.Size() > 0)) {
    	  if (!afGain) {
    		n_gains = tag.Size();
    		afGain	= (float*)malloc((n_gains * sizeof(float)));
    		MEMSET(afGain, 0, (n_gains * sizeof(float)));
    	  }
    
    	  int no = ParseFloatArray(tag.Value(), afGain, n_gains);
    	  DCT_ASSERT((no == n_gains));
    	} else if ((subTagname == CALIB_SENSOR_DPF_SHARPENINGLEVEL_SLEVEL_TAG)
    			   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
    			   && (tag.Size() > 0)) {
    	  if (!afSlevel) {
    		n_Slevels = tag.Size();
    		afSlevel = (float*)malloc((n_Slevels * sizeof(float)));
    		MEMSET(afSlevel, 0, (n_Slevels * sizeof(float)));
    	  }
    
    	  int no = ParseFloatArray(tag.Value(), afSlevel, n_Slevels);
    	  DCT_ASSERT((no == n_Slevels));
    	} else {
    	  redirectOut
    		  << "parse error in Dpf sharpeninglevel curve section (unknow tag: "
    		  << subTagname
    		  << ")"
    		  << std::endl;
    	}
    
    	psubchild = psubchild->NextSibling();
      }
    
      DCT_ASSERT((n_gains == n_Slevels));
      pFilter->SharpeningLevelCurve.ArraySize	  = n_gains;
      pFilter->SharpeningLevelCurve.pSensorGain	  = afGain;
      pFilter->SharpeningLevelCurve.pSlevel = 
	  	(CamerIcIspFltSharpeningLevel_t*)malloc((n_Slevels * sizeof(CamerIcIspFltSharpeningLevel_t)));
      for (index = 0; index < pFilter->SharpeningLevelCurve.ArraySize; index++) {
    	pFilter->SharpeningLevelCurve.pSlevel[index] = (CamerIcIspFltSharpeningLevel_t)((int)afSlevel[index] + 1);
      }
      free(afSlevel);
    }
	else if (tagname == CALIB_SENSOR_DPF_FILT_DEMOSAIC_TH_CONF_TAG) {
      float* afGain   = NULL;
      int n_gains	  = 0;
      float* afThlevel	 = NULL;
      int n_Thlevels 	 = 0;
      int index = 0;
      const XMLNode* psubchild = pchild->ToElement()->FirstChild();
      while (psubchild) {
    	XmlTag tag = XmlTag(psubchild->ToElement());
    	std::string subTagname(psubchild->ToElement()->Name());
    
    	redirectOut << "subTagname: " << subTagname << std::endl;
    
    	if ((subTagname == CALIB_SENSOR_DPF_FILT_DEMOSAIC_TH_GAIN_TAG)
    		&& (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
    		&& (tag.Size() > 0)) {
    	  if (!afGain) {
    		n_gains = tag.Size();
    		afGain	= (float*)malloc((n_gains * sizeof(float)));
    		MEMSET(afGain, 0, (n_gains * sizeof(float)));
    	  }
    
    	  int no = ParseFloatArray(tag.Value(), afGain, n_gains);
    	  DCT_ASSERT((no == n_gains));
    	} else if ((subTagname == CALIB_SENSOR_DPF_FILT_DEMOSAIC_TH_LEVEL_TAG)
    			   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
    			   && (tag.Size() > 0)) {
    	  if (!afThlevel) {
    		n_Thlevels = tag.Size();
    		afThlevel = (float*)malloc((n_Thlevels * sizeof(float)));
    		MEMSET(afThlevel, 0, (n_Thlevels * sizeof(float)));
    	  }
    
    	  int no = ParseFloatArray(tag.Value(), afThlevel, n_Thlevels);
    	  DCT_ASSERT((no == n_Thlevels));
    	} else {
    	  redirectOut
    		  << "parse error in demosaic th curve section (unknow tag: "
    		  << subTagname
    		  << ")"
    		  << std::endl;
    	}
    
    	psubchild = psubchild->NextSibling();
      }
    
      DCT_ASSERT((n_gains == n_Thlevels));
      pFilter->DemosaicThCurve.ArraySize	  = n_gains;
      pFilter->DemosaicThCurve.pSensorGain	  = afGain;
      pFilter->DemosaicThCurve.pThlevel = (uint8_t*)malloc((n_Thlevels * sizeof(uint8_t)));
      for (index = 0; index < pFilter->DemosaicThCurve.ArraySize; index++) {
    	pFilter->DemosaicThCurve.pThlevel[index] = (uint8_t)((int)afThlevel[index]);
      }
      free(afThlevel);
	}else{
		redirectOut
          << "parse error in filter section (unknow tag: "
          << tagname
          << std::endl;
	}	
	pchild = pchild->NextSibling();
  }

  if (pFilter) {
	 ListPrepareItem(pFilter);
	 ListAddTail(&pdpf_profile->FilterList, pFilter);
  }

  redirectOut << __func__ << " (exit)" << std::endl;

  return (true);
 }

bool CalibDb::parseEntry3DNR
(
    const XMLElement* plement,
    void* param
) 
{
  redirectOut << __func__ << " (enter)" << std::endl;
  CamDpfProfile_t *pdpf_profile = (CamDpfProfile_t *)param;
  if(NULL == pdpf_profile){
	redirectOut << __func__ << " Invalid pointer (exit)" << std::endl;
    return false;
  }
  
  int nGainLevel = 0;
  int nNoiseCoefNum = 0;
  int nNoiseCoefDen = 0;
  int nLumaSpNrLevel = 0;
  int nLumaTeNrLevel = 0;
  int nChrmSpNrLevel = 0;
  int nChrmTeNrLevel = 0;
  int nShpLevel = 0;
  int nLumaSpRad = 0;
  int nLumaTeMaxBiNum = 0;
  int nLumaWeight[CAM_CALIBDB_3DNR_WEIGHT_NUM];
  int nChrmSpRad = 0;
  int nChrmTeMaxBiNum = 0;
  int nChrmWeight[CAM_CALIBDB_3DNR_WEIGHT_NUM];
  int nSrcShpDiv = 0;
  int nSrcShpThr = 0;
  int nSrcShpL = 0;
  int nSrcShpC = 0;
  int nSrcShpWeight[CAM_CALIBDB_3DNR_WEIGHT_NUM];

  MEMSET(nLumaWeight, 0x00, CAM_CALIBDB_3DNR_WEIGHT_NUM*sizeof(int));
  MEMSET(nChrmWeight, 0x00, CAM_CALIBDB_3DNR_WEIGHT_NUM*sizeof(int));
  MEMSET(nSrcShpWeight, 0x00, CAM_CALIBDB_3DNR_WEIGHT_NUM*sizeof(int));
  
  CamDsp3DNRSettingProfile_t* pDsp3DNRProfile = (CamDsp3DNRSettingProfile_t*)malloc(sizeof(CamDsp3DNRSettingProfile_t));
  if (!pDsp3DNRProfile) {
  	redirectOut << __func__ << " malloc fail (exit)" << std::endl;
    return false;
  }
  MEMSET(pDsp3DNRProfile, 0, sizeof(*pDsp3DNRProfile));

  const XMLNode* pchild = plement->FirstChild();
  while (pchild) {				
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());

    redirectOut  << "subTagname: " << tagname << std::endl;

    if ((tagname == CALIB_SENSOR_DSP_3DNR_SETTING_NAME_TAG)
        && (tag.Size() > 0)) {

	  char* value = Toupper(tag.Value());
      strncpy(pDsp3DNRProfile->name, value, sizeof(pDsp3DNRProfile->name));
      redirectOut << "value:" << value << std::endl;
      redirectOut << pDsp3DNRProfile->name << std::endl;
    }else if ((tagname == CALIB_SENSOR_DSP_3DNR_SETTING_ENABLE_TAG)
        && (tag.Size() > 0)) {
      int no = ParseUcharArray(tag.Value(), &pDsp3DNRProfile->Enable, 1);
      DCT_ASSERT((no == tag.Size()));
    }else if ((tagname == CALIB_SENSOR_DSP_3DNR_SETTING_GAIN_LEVEL_TAG)
       && (tag.Size() > 0)) {
      pDsp3DNRProfile->pgain_Level = (float*)malloc((tag.Size() * sizeof(float)));
	  if(!pDsp3DNRProfile->pgain_Level){
	      redirectOut << "malloc fail:" <<__LINE__ << std::endl;
  	  }else{
		  int no = ParseFloatArray(tag.Value(), pDsp3DNRProfile->pgain_Level, tag.Size());
	      DCT_ASSERT((no == tag.Size()));
		  nGainLevel = no;
  	  }
    }else if ((tagname == CALIB_SENSOR_DSP_3DNR_SETTING_NOISE_COEF_NUMERATOR_TAG)
       && (tag.Size() > 0)) {
      pDsp3DNRProfile->pnoise_coef_numerator = (uint16_t*)malloc((tag.Size() * sizeof(uint16_t)));
	  if(!pDsp3DNRProfile->pnoise_coef_numerator){
	      redirectOut << "malloc fail:" <<__LINE__ << std::endl;
  	  }else{
		  int no = ParseUshortArray(tag.Value(), pDsp3DNRProfile->pnoise_coef_numerator, tag.Size());
	      DCT_ASSERT((no == tag.Size()));
		  nNoiseCoefNum = no;
  	  }
    }else if ((tagname == CALIB_SENSOR_DSP_3DNR_SETTING_NOISE_COEF_DENOMINATOR_TAG)
       && (tag.Size() > 0)) {
      pDsp3DNRProfile->pnoise_coef_denominator= (uint16_t*)malloc((tag.Size() * sizeof(uint16_t)));
	  if(!pDsp3DNRProfile->pnoise_coef_denominator){
	      redirectOut << "malloc fail:" <<__LINE__ << std::endl;
  	  }else{
		  int no = ParseUshortArray(tag.Value(), pDsp3DNRProfile->pnoise_coef_denominator, tag.Size());
	      DCT_ASSERT((no == tag.Size()));
		  nNoiseCoefDen = no;
  	  }
    }
	else if((tagname == CALIB_SENSOR_DSP_3DNR_SETTING_LEVEL_SETTING_TAG)
       && (tag.Size() > 0)) {
       const XMLNode* psubchild = pchild->ToElement()->FirstChild();
	   while (psubchild) {
            XmlTag subtag = XmlTag(psubchild->ToElement());
        	std::string subTagname(psubchild->ToElement()->Name());
        	redirectOut  << "subTagname: " << subTagname <<"size: " << subtag.Size() << std::endl;
			if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_SP_NR_EN_TAG)
               && (subtag.Size() > 0)) {
	          int no = ParseUcharArray(subtag.Value(), &pDsp3DNRProfile->sDefaultLevelSetting.luma_sp_nr_en, 1);
	          DCT_ASSERT((no == subtag.Size()));
	        }else if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_SP_NR_LEVEL_TAG)
               && (subtag.Size() > 0)) {
              pDsp3DNRProfile->sDefaultLevelSetting.pluma_sp_nr_level = (unsigned char*)malloc((subtag.Size() * sizeof(unsigned char)));
			  if(!pDsp3DNRProfile->sDefaultLevelSetting.pluma_sp_nr_level){
			      redirectOut << "malloc fail:" <<__LINE__ << std::endl;
		  	  }else{
				  int no = ParseUcharArray(subtag.Value(), pDsp3DNRProfile->sDefaultLevelSetting.pluma_sp_nr_level, subtag.Size());
		          DCT_ASSERT((no == subtag.Size()));
				  nLumaSpNrLevel = no;
		  	  }
	        }else if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_TE_NR_EN_TAG)
               && (subtag.Size() > 0)) {
	          int no = ParseUcharArray(subtag.Value(), &pDsp3DNRProfile->sDefaultLevelSetting.luma_te_nr_en, 1);
	          DCT_ASSERT((no == subtag.Size()));
	        }else if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_TE_NR_LEVEL_TAG)
               && (subtag.Size() > 0)) {
              pDsp3DNRProfile->sDefaultLevelSetting.pluma_te_nr_level = (unsigned char*)malloc((subtag.Size() * sizeof(unsigned char)));
			  if(!pDsp3DNRProfile->sDefaultLevelSetting.pluma_te_nr_level){
			      redirectOut << "malloc fail:" <<__LINE__ << std::endl;
		  	  }else{
	 			  int no = ParseUcharArray(subtag.Value(), pDsp3DNRProfile->sDefaultLevelSetting.pluma_te_nr_level, subtag.Size());
	 	          DCT_ASSERT((no == subtag.Size()));
				  nLumaTeNrLevel = no;
		  	  }
	        }else if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_SP_NR_EN_TAG)
               && (subtag.Size() > 0)) {
	          int no = ParseUcharArray(subtag.Value(), &pDsp3DNRProfile->sDefaultLevelSetting.chrm_sp_nr_en, 1);
	          DCT_ASSERT((no == subtag.Size()));
	        }else if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_SP_NR_LEVEL_TAG)
               && (subtag.Size() > 0)) {
              pDsp3DNRProfile->sDefaultLevelSetting.pchrm_sp_nr_level = (unsigned char*)malloc((subtag.Size() * sizeof(unsigned char)));
			  if(!pDsp3DNRProfile->sDefaultLevelSetting.pchrm_sp_nr_level){
			      redirectOut << "malloc fail:" <<__LINE__ << std::endl;
		  	  }else{
				  int no = ParseUcharArray(subtag.Value(), pDsp3DNRProfile->sDefaultLevelSetting.pchrm_sp_nr_level, subtag.Size());
				  DCT_ASSERT((no == subtag.Size()));
				  nChrmSpNrLevel = no;
		  	  }
	        }else if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_TE_NR_EN_TAG)
               && (subtag.Size() > 0)) {
	          int no = ParseUcharArray(subtag.Value(), &pDsp3DNRProfile->sDefaultLevelSetting.chrm_te_nr_en, 1);
	          DCT_ASSERT((no == subtag.Size()));
	        }else if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_TE_NR_LEVEL_TAG)
               && (subtag.Size() > 0)) {
              pDsp3DNRProfile->sDefaultLevelSetting.pchrm_te_nr_level = (unsigned char*)malloc((subtag.Size() * sizeof(unsigned char)));
			  if(!pDsp3DNRProfile->sDefaultLevelSetting.pchrm_te_nr_level){
			      redirectOut << "malloc fail:" <<__LINE__ << std::endl;
		  	  }else{
				  int no = ParseUcharArray(subtag.Value(), pDsp3DNRProfile->sDefaultLevelSetting.pchrm_te_nr_level, subtag.Size());
				  DCT_ASSERT((no == subtag.Size()));
				  nChrmTeNrLevel = no;
		  	  }
	        }else if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_SHP_EN_TAG)
               && (subtag.Size() > 0)) {
	          int no = ParseUcharArray(subtag.Value(), &pDsp3DNRProfile->sDefaultLevelSetting.shp_en, 1);
	          DCT_ASSERT((no == subtag.Size()));
	        }else if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_SHP_LEVEL_TAG)
               && (subtag.Size() > 0)) {
              pDsp3DNRProfile->sDefaultLevelSetting.pshp_level = (unsigned char*)malloc((subtag.Size() * sizeof(unsigned char)));
			  if(!pDsp3DNRProfile->sDefaultLevelSetting.pshp_level){
			      redirectOut << "malloc fail:" <<__LINE__ << std::endl;
		  	  }else{
				  int no = ParseUcharArray(subtag.Value(), pDsp3DNRProfile->sDefaultLevelSetting.pshp_level, subtag.Size());
				  DCT_ASSERT((no == subtag.Size()));
				  nShpLevel = no;
		  	  }
	        }else{
				redirectOut
		          << "parse error in DPF section (unknow tag: "
		          << tagname
		          << std::endl;
        	}	
			psubchild = psubchild->NextSibling();
		}
	}
	else if ((tagname == CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_SETTING_TAG)
		&& (tag.Size() > 0)) {
		const XMLNode* psubchild = pchild->ToElement()->FirstChild();
	    while (psubchild) {
            XmlTag subtag = XmlTag(psubchild->ToElement());
        	std::string subTagname(psubchild->ToElement()->Name());
        	redirectOut  << "subTagname: " << subTagname << std::endl;	
			if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_DEFAULT_TAG)
               && (subtag.Size() > 0)) {
	          int no = ParseUcharArray(subtag.Value(), &pDsp3DNRProfile->sLumaSetting.luma_default, 1);
	          DCT_ASSERT((no == subtag.Size()));
	        }else if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_SP_RAD_TAG)
               && (subtag.Size() > 0)) {
              pDsp3DNRProfile->sLumaSetting.pluma_sp_rad = (unsigned char*)malloc((subtag.Size() * sizeof(unsigned char)));
			  if(!pDsp3DNRProfile->sLumaSetting.pluma_sp_rad){
			      redirectOut << "malloc fail:" <<__LINE__ << std::endl;
		  	  }else{
				  int no = ParseUcharArray(subtag.Value(), pDsp3DNRProfile->sLumaSetting.pluma_sp_rad, subtag.Size());
				  DCT_ASSERT((no == subtag.Size()));
				  nLumaSpRad = no;
		  	  }
	        }else if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_TE_MAX_BI_NUM_TAG)
               && (subtag.Size() > 0)) {
              pDsp3DNRProfile->sLumaSetting.pluma_te_max_bi_num = (unsigned char*)malloc((subtag.Size() * sizeof(unsigned char)));
			  if(!pDsp3DNRProfile->sLumaSetting.pluma_te_max_bi_num){
			      redirectOut << "malloc fail:" <<__LINE__ << std::endl;
		  	  }else{
				  int no = ParseUcharArray(subtag.Value(), pDsp3DNRProfile->sLumaSetting.pluma_te_max_bi_num, subtag.Size());
				  DCT_ASSERT((no == subtag.Size()));
				  nLumaTeMaxBiNum = no;
		  	  }
	        }else if( (subTagname.find(CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_WEIGHT_TAG) != std::string::npos) 
	        	&& (subtag.Size() > 0)){
				//parse weight num
				int find = strlen(CALIB_SENSOR_DSP_3DNR_SETTING_LUMA_WEIGHT_TAG);
				std::string weight_col_string = subTagname.substr(find, 1);
				std::string weight_row_string = subTagname.substr(find+1, 1);
				int weight_col = atoi(weight_col_string.c_str());
				int weight_row = atoi(weight_row_string.c_str());
				int idx = weight_col*CAM_DSP_3DNR_SETTING_WEIGHT_ROW_NUM + weight_row;

				if(idx >= 0 && idx < CAM_CALIBDB_3DNR_WEIGHT_NUM){
					if(!pDsp3DNRProfile->sLumaSetting.pluma_weight[idx])
						pDsp3DNRProfile->sLumaSetting.pluma_weight[idx]= (uint8_t*)malloc((subtag.Size() * sizeof(uint8_t)));
					if(!pDsp3DNRProfile->sLumaSetting.pluma_weight[idx]){
				      redirectOut << "malloc fail, col:"<< weight_col << " row:" 
					  	<< weight_row << " line:" <<__LINE__ << std::endl;
	   		  	    }else{
	   				  int no = ParseUcharArray(subtag.Value(), pDsp3DNRProfile->sLumaSetting.pluma_weight[idx], subtag.Size());
	   				  DCT_ASSERT((no == subtag.Size()));
					  nLumaWeight[idx] = no;
	   		  	    }
				}
	        }else{
				redirectOut
		          << "parse error in DPF section (unknow tag: "
		          << tagname
		          << std::endl;
        	}
			
			psubchild = psubchild->NextSibling();
			
    	}
	}
	else if((tagname == CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_SETTING_TAG)
               && (tag.Size() > 0)) {
               
    	const XMLNode* psubchild = pchild->ToElement()->FirstChild();
	    while (psubchild) {
            XmlTag subtag = XmlTag(psubchild->ToElement());
        	std::string subTagname(psubchild->ToElement()->Name());
        	redirectOut  << "subTagname: " << subTagname << std::endl;
			if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_DEFAULT_TAG)
               && (subtag.Size() > 0)) {
	          int no = ParseUcharArray(subtag.Value(), &pDsp3DNRProfile->sChrmSetting.chrm_default, 1);
	          DCT_ASSERT((no == subtag.Size()));
	        }else if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_SP_RAD_TAG)
               && (subtag.Size() > 0)) {
              pDsp3DNRProfile->sChrmSetting.pchrm_sp_rad = (unsigned char*)malloc((subtag.Size() * sizeof(unsigned char)));
			  if(!pDsp3DNRProfile->sChrmSetting.pchrm_sp_rad){
			      redirectOut << "malloc fail:" <<__LINE__ << std::endl;
		  	  }else{
				  int no = ParseUcharArray(subtag.Value(), pDsp3DNRProfile->sChrmSetting.pchrm_sp_rad, subtag.Size());
				  DCT_ASSERT((no == subtag.Size()));
				  nChrmSpRad = no;
		  	  }
	        }else if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_TE_MAX_BI_NUM_TAG)
               && (subtag.Size() > 0)) {
              pDsp3DNRProfile->sChrmSetting.pchrm_te_max_bi_num = (unsigned char*)malloc((subtag.Size() * sizeof(unsigned char)));
			  if(!pDsp3DNRProfile->sChrmSetting.pchrm_te_max_bi_num){
			      redirectOut << "malloc fail:" <<__LINE__ << std::endl;
		  	  }else{
				  int no = ParseUcharArray(subtag.Value(), pDsp3DNRProfile->sChrmSetting.pchrm_te_max_bi_num, subtag.Size());
				  DCT_ASSERT((no == subtag.Size()));
				  nChrmTeMaxBiNum = no;
		  	  }
	        }
			else if( (subTagname.find(CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_WEIGHT_TAG) != std::string::npos) 
	        	&& (subtag.Size() > 0)){
				//parse weight num
				int find = strlen(CALIB_SENSOR_DSP_3DNR_SETTING_CHRM_WEIGHT_TAG);
				std::string weight_col_string = subTagname.substr(find, 1);
				std::string weight_row_string = subTagname.substr(find+1, 1);
				int weight_col = atoi(weight_col_string.c_str());
				int weight_row = atoi(weight_row_string.c_str());
				int idx = weight_col*CAM_DSP_3DNR_SETTING_WEIGHT_ROW_NUM + weight_row;

				if(idx >= 0 && idx < CAM_CALIBDB_3DNR_WEIGHT_NUM){
					if(!pDsp3DNRProfile->sChrmSetting.pchrm_weight[idx])
						pDsp3DNRProfile->sChrmSetting.pchrm_weight[idx]= (uint8_t*)malloc((subtag.Size() * sizeof(uint8_t)));
					if(!pDsp3DNRProfile->sChrmSetting.pchrm_weight[idx]){
				      redirectOut << "malloc fail, col:"<< weight_col << " row:" 
					  	<< weight_row << " line:" <<__LINE__ << std::endl;
	   		  	    }else{
	   				  int no = ParseUcharArray(subtag.Value(), pDsp3DNRProfile->sChrmSetting.pchrm_weight[idx], subtag.Size());
	   				  DCT_ASSERT((no == subtag.Size()));
					  nChrmWeight[idx] = no;
	   		  	    }
				}
	        }
	        else{
				redirectOut
		          << "parse error in DPF section (unknow tag: "
		          << tagname
		          << std::endl;
        	}	
			psubchild = psubchild->NextSibling();

		}
 	}
	else if((tagname == CALIB_SENSOR_DSP_3DNR_SETTING_SHP_SETTING_TAG)
		   && (tag.Size() > 0)) {							   
		const XMLNode* psubchild = pchild->ToElement()->FirstChild();
		while (psubchild) {
			XmlTag subtag = XmlTag(psubchild->ToElement());
			std::string subTagname(psubchild->ToElement()->Name());
			redirectOut  << "subTagname: " << subTagname << std::endl;
			if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_SHP_DEFAULT_TAG)
               && (subtag.Size() > 0)) {
	          int no = ParseUcharArray(subtag.Value(), &pDsp3DNRProfile->sSharpSetting.shp_default, 1);
	          DCT_ASSERT((no == subtag.Size()));
	        }else if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_THR_TAG)
               && (subtag.Size() > 0)) {
              pDsp3DNRProfile->sSharpSetting.psrc_shp_thr = (unsigned char*)malloc((subtag.Size() * sizeof(unsigned char)));
			  if(!pDsp3DNRProfile->sSharpSetting.psrc_shp_thr){
			      redirectOut << "malloc fail:" <<__LINE__ << std::endl;
		  	  }else{
		          int no = ParseUcharArray(subtag.Value(), pDsp3DNRProfile->sSharpSetting.psrc_shp_thr, subtag.Size());
		          DCT_ASSERT((no == subtag.Size()));
				  nSrcShpThr = no;
		  	  }
	        }else if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_DIV_TAG)
               && (subtag.Size() > 0)) {
              pDsp3DNRProfile->sSharpSetting.psrc_shp_div = (unsigned char*)malloc((subtag.Size() * sizeof(unsigned char)));
			  if(!pDsp3DNRProfile->sSharpSetting.psrc_shp_div){
			      redirectOut << "malloc fail:" <<__LINE__ << std::endl;
		  	  }else{
		          int no = ParseUcharArray(subtag.Value(), pDsp3DNRProfile->sSharpSetting.psrc_shp_div, subtag.Size());
		          DCT_ASSERT((no == subtag.Size()));
				  nSrcShpDiv = no;
		  	  }
	        }else if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_L_TAG)
               && (subtag.Size() > 0)) {
              pDsp3DNRProfile->sSharpSetting.psrc_shp_l = (unsigned char*)malloc((subtag.Size() * sizeof(unsigned char)));
			  if(!pDsp3DNRProfile->sSharpSetting.psrc_shp_l){
			      redirectOut << "malloc fail:" <<__LINE__ << std::endl;
		  	  }else{
		          int no = ParseUcharArray(subtag.Value(), pDsp3DNRProfile->sSharpSetting.psrc_shp_l, subtag.Size());
		          DCT_ASSERT((no == subtag.Size()));
				  nSrcShpL = no;
		  	  }
	        }else if ((subTagname == CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_C_TAG)
               && (subtag.Size() > 0)) {
              pDsp3DNRProfile->sSharpSetting.psrc_shp_c = (unsigned char*)malloc((subtag.Size() * sizeof(unsigned char)));
			  if(!pDsp3DNRProfile->sSharpSetting.psrc_shp_c){
			      redirectOut << "malloc fail:" <<__LINE__ << std::endl;
		  	  }else{
		          int no = ParseUcharArray(subtag.Value(), pDsp3DNRProfile->sSharpSetting.psrc_shp_c, subtag.Size());
		          DCT_ASSERT((no == subtag.Size()));
				  nSrcShpC = no;
		  	  }
	        }
			else if( (subTagname.find(CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_WEIGHT_TAG) != std::string::npos) 
	        	&& (subtag.Size() > 0)){
				//parse weight num
				int find = strlen(CALIB_SENSOR_DSP_3DNR_SETTING_SRC_SHP_WEIGHT_TAG);
				std::string weight_col_string = subTagname.substr(find, 1);
				std::string weight_row_string = subTagname.substr(find+1, 1);
				int weight_col = atoi(weight_col_string.c_str());
				int weight_row = atoi(weight_row_string.c_str());
				int idx = weight_col*CAM_DSP_3DNR_SETTING_WEIGHT_ROW_NUM + weight_row;

				if(idx >= 0 && idx < CAM_CALIBDB_3DNR_WEIGHT_NUM){
					if(!pDsp3DNRProfile->sSharpSetting.psrc_shp_weight[idx])
						pDsp3DNRProfile->sSharpSetting.psrc_shp_weight[idx]= (int8_t*)malloc((subtag.Size() * sizeof(int8_t)));
					if(!pDsp3DNRProfile->sSharpSetting.psrc_shp_weight[idx]){
				      redirectOut << "malloc fail, col:"<< weight_col << " row:"
					  	<< weight_row << " line:" <<__LINE__ << std::endl;
	   		  	    }else{
	   				  int no = ParseCharArray(subtag.Value(), pDsp3DNRProfile->sSharpSetting.psrc_shp_weight[idx], subtag.Size());
	   				  DCT_ASSERT((no == subtag.Size()));
					  nSrcShpWeight[idx] = no;
	   		  	    }
				}
	        }
	        else{
				redirectOut
		          << "parse error in 3dnr section (unknow tag: "
		          << tagname
		          << std::endl;
        	}	
			psubchild = psubchild->NextSibling();                                                                
 		}

	}else{
		redirectOut
          << "parse error in 3dnr section (unknow tag: "
          << tagname
          << std::endl;
	}	

	pchild = pchild->NextSibling();   
}

 DCT_ASSERT(nGainLevel == nNoiseCoefNum);
 DCT_ASSERT(nGainLevel == nNoiseCoefDen);
 DCT_ASSERT(nGainLevel == nLumaSpNrLevel);
 DCT_ASSERT(nGainLevel == nLumaTeNrLevel);
 DCT_ASSERT(nGainLevel == nChrmSpNrLevel);
 DCT_ASSERT(nGainLevel == nChrmTeNrLevel);
 DCT_ASSERT(nGainLevel == nShpLevel);
 DCT_ASSERT(nGainLevel == nLumaSpRad);
 DCT_ASSERT(nGainLevel == nLumaTeMaxBiNum);
 DCT_ASSERT(nGainLevel == nChrmSpRad);
 DCT_ASSERT(nGainLevel == nChrmTeMaxBiNum);
 DCT_ASSERT(nGainLevel == nSrcShpC);
 DCT_ASSERT(nGainLevel == nSrcShpL);
 DCT_ASSERT(nGainLevel == nSrcShpDiv);
 DCT_ASSERT(nGainLevel == nSrcShpThr);

 for(int i=0; i<CAM_CALIBDB_3DNR_WEIGHT_NUM; i++){
 	DCT_ASSERT(nGainLevel == nLumaWeight[i]);
	DCT_ASSERT(nGainLevel == nChrmWeight[i]);
	DCT_ASSERT(nGainLevel == nSrcShpWeight[i]);
 }

 pDsp3DNRProfile->ArraySize = nGainLevel;
 
 if (pDsp3DNRProfile) {
	 ListPrepareItem(pDsp3DNRProfile);
	 ListAddTail(&pdpf_profile->Dsp3DNRSettingProfileList, pDsp3DNRProfile);
 }

  redirectOut << __func__ << " (exit)" << std::endl;

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

  redirectOut << __func__ << " (enter)" << std::endl;

  CamDpfProfile_t dpf_profile;
  MEMSET(&dpf_profile, 0, sizeof(dpf_profile));
  ListInit(&dpf_profile.Dsp3DNRSettingProfileList);

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());

    redirectOut << "tagname: " << tagname << std::endl;

    if ((tagname == CALIB_SENSOR_DPF_NAME_TAG)
        && (tag.isType(XmlTag::TAG_TYPE_CHAR))
        && (tag.Size() > 0)) {
      char* value = Toupper(tag.Value());
      strncpy(dpf_profile.name, value, sizeof(dpf_profile.name));
      redirectOut << "value:" << value << std::endl;
      redirectOut << dpf_profile.name << std::endl;
    } else if ((tagname == CALIB_SENSOR_DPF_RESOLUTION_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      const char* value = tag.Value();
      strncpy(dpf_profile.resolution, value, sizeof(dpf_profile.resolution));
      redirectOut << "value:" << value << std::endl;
      redirectOut << dpf_profile.resolution << std::endl;
    } else if (tagname == CALIB_SENSOR_DPF_ADPF_ENABLE_TAG) {
      int no = ParseUshortArray(tag.Value(), &dpf_profile.ADPFEnable, 1);
      DCT_ASSERT((no == 1));
    } else if ((tagname == CALIB_SENSOR_DPF_NLL_SEGMENTATION_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
               && (tag.Size() > 0)) {
      int no = ParseUshortArray(tag.Value(), &dpf_profile.nll_segmentation, 1);
      DCT_ASSERT((no == 1));
    } else if (tagname == CALIB_SENSOR_DPF_NLL_COEFF_TAG) {
      int i = (sizeof(dpf_profile.nll_coeff) / sizeof(dpf_profile.nll_coeff.uCoeff[0]));
      int no = ParseUshortArray(tag.Value(), dpf_profile.nll_coeff.uCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_DPF_SIGMA_GREEN_TAG) {
      int no = ParseUshortArray(tag.Value(), &dpf_profile.SigmaGreen, 1);
      DCT_ASSERT((no == 1));
    } else if (tagname == CALIB_SENSOR_DPF_SIGMA_RED_BLUE_TAG) {
      int no = ParseUshortArray(tag.Value(), &dpf_profile.SigmaRedBlue, 1);
      DCT_ASSERT((no == 1));
    } else if (tagname == CALIB_SENSOR_DPF_GRADIENT_TAG) {
      int no = ParseFloatArray(tag.Value(), &dpf_profile.fGradient, 1);
      DCT_ASSERT((no == 1));
    } else if (tagname == CALIB_SENSOR_DPF_OFFSET_TAG) {
      int no = ParseFloatArray(tag.Value(), &dpf_profile.fOffset, 1);
      DCT_ASSERT((no == 1));
    } else if (tagname == CALIB_SENSOR_DPF_NLGAINS_TAG) {
      int i = (sizeof(dpf_profile.NfGains) / sizeof(dpf_profile.NfGains.fCoeff[0]));
      int no = ParseFloatArray(tag.Value(), dpf_profile.NfGains.fCoeff, i);
      DCT_ASSERT((no == tag.Size()));
    } else if(tagname == CALIB_SENSOR_DPF_FILTERSETTING_TAG) {
	  if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryFilter, &dpf_profile)) {
		redirectOut
			<< "parse error in filter section ("
			<< tagname
			<< ")"
			<< std::endl;
		return (false);
	  }
	}else if(tagname == CALIB_SENSOR_DSP_3DNR_SETTING_TAG) {
	  if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntry3DNR, &dpf_profile)) {
        redirectOut
            << "parse error in 3dnr section ("
            << tagname
            << ")"
            << std::endl;
        return (false);
      }
	}else {
      redirectOut
          << "parse error in DPF section (unknow tag: "
          << tagname
          << std::endl;
    }
    pchild = pchild->NextSibling();
  }

  RESULT result = CamCalibDbAddDpfProfile(m_CalibDbHandle, &dpf_profile);
  
  DCT_ASSERT(result == RET_SUCCESS);

  // free linked ecm_schemes
  List* l = ListRemoveHead(&dpf_profile.Dsp3DNRSettingProfileList);
  while (l) {
    List* temp = ListRemoveHead(l);
    free(l);
    l = temp;
  }

   // free linked ecm_schemes
  List* l_filter = ListRemoveHead(&dpf_profile.FilterList);
  while (l_filter) {
    List* temp_filter = ListRemoveHead(l_filter);
    free(l_filter);
    l_filter = temp_filter;
  }
  
  redirectOut << __func__ << " (exit)" << std::endl;

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

  redirectOut << __func__ << " (enter)" << std::endl;

  CamDpccProfile_t dpcc_profile;
  MEMSET(&dpcc_profile, 0, sizeof(dpcc_profile));

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());

    redirectOut << "tagname: " << tagname << std::endl;

    if ((tagname == CALIB_SENSOR_DPCC_NAME_TAG)
        && (tag.isType(XmlTag::TAG_TYPE_CHAR))
        && (tag.Size() > 0)) {
      char* value = Toupper(tag.Value());
      strncpy(dpcc_profile.name, value, sizeof(dpcc_profile.name));
      redirectOut << "value:" << value << std::endl;
      redirectOut << dpcc_profile.name << std::endl;
    } else if ((tagname == CALIB_SENSOR_DPCC_RESOLUTION_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      const char* value = tag.Value();
      strncpy(dpcc_profile.resolution, value, sizeof(dpcc_profile.resolution));
      redirectOut << "value:" << value << std::endl;
      redirectOut << dpcc_profile.resolution << std::endl;
    } else if ((tagname == CALIB_SENSOR_DPCC_REGISTER_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_CELL))
               && (tag.Size() > 0)) {
      if (!parseEntryCell(pchild->ToElement(), tag.Size(), &CalibDb::parseEntryDpccRegisters, &dpcc_profile)) {
#if 1
        redirectOut
            << "parse error in DPF section (unknow tag: "
            << tagname
            << ")"
            << std::endl;
#endif

        return (false);
      }
    } else {
#if 1
      redirectOut
          << "parse error in DPCC section (unknow tag: "
          << tagname
          << ")"
          << std::endl;
#endif

      return (false);
    }

    pchild = pchild->NextSibling();
  }

  RESULT result = CamCalibDbAddDpccProfile(m_CalibDbHandle, &dpcc_profile);
  DCT_ASSERT(result == RET_SUCCESS);

  redirectOut << __func__ << " (exit)" << std::endl;

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

  redirectOut << __func__ << " (enter)" << std::endl;

  CamDpccProfile_t* pDpcc_profile = (CamDpccProfile_t*)param;

  char*     reg_name;
  uint32_t    reg_value = 0U;

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());

    redirectOut << "tagname: " << tagname << std::endl;

    if ((tagname == CALIB_SENSOR_DPCC_REGISTER_NAME_TAG)
        && (tag.isType(XmlTag::TAG_TYPE_CHAR))
        && (tag.Size() > 0)) {
      reg_name = Toupper(tag.Value());
    } else if ((tagname == CALIB_SENSOR_DPCC_REGISTER_VALUE_TAG)
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      bool ok;

      reg_value = tag.ValueToUInt(&ok);
      if (!ok) {
#if 1
        redirectOut
            << "parse error: invalid DPCC register value "
            << tagname << "/" << tag.Value() << std::endl;
#endif

        return (false);
      }
    } else {
#if 1
      redirectOut
          << "parse error in DPCC register section (unknow tag: "
          << pchild->ToElement()->Name()
          << ")";
#endif

      return (false);
    }

    pchild = pchild->NextSibling();
  }

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
    redirectOut
        << "unknown DPCC register ("
        << s_regname
        << ")"
        << std::endl;
#endif
  }
#if 0
  if (reg_name) {
    free(reg_name);
    reg_name = NULL;
  }
#endif
  redirectOut << __func__ << " (exit)" << std::endl;

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

  redirectOut << __func__ << " (enter)" << std::endl;

  CamCalibGocProfile_t goc_data;
  goc_data.def_cfg_mode = -1;
  goc_data.enable_mode = -1;
  memset(goc_data.GammaY, 0, sizeof(goc_data.GammaY));
  memset(goc_data.WdrOn_GammaY, 0, sizeof(goc_data.WdrOn_GammaY));

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
    redirectOut << tagname << std::endl;
	
	if (tagname == CALIB_SENSOR_GOC_NAME_TAG) {
      char* value = Toupper(tag.Value());
      strncpy(goc_data.name, value, sizeof(goc_data.name));
      redirectOut << "value:" << value << std::endl;
      redirectOut << goc_data.name << std::endl;
    } else if (tagname == CALIB_SENSOR_GOC_GAMMAY_TAG) {
      int i = (sizeof(goc_data.GammaY) / sizeof(goc_data.GammaY[0]));
      int no = ParseUshortArray(tag.Value(), goc_data.GammaY, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_GOC_ENABLE_MODE_TAG) {
      int no = ParseUshortArray(tag.Value(), &goc_data.enable_mode, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_GOC_DEF_CFG_MODE_TAG) {
      int no = ParseUshortArray(tag.Value(), &goc_data.def_cfg_mode, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_GOC_WDRON_GAMMAY_TAG) {
      int i = (sizeof(goc_data.WdrOn_GammaY) / sizeof(goc_data.WdrOn_GammaY[0]));
      int no = ParseUshortArray(tag.Value(), goc_data.WdrOn_GammaY, i);
      DCT_ASSERT((no == tag.Size()));
    }
    pchild = pchild->NextSibling();
  }

  RESULT result = CamCalibDbAddGocProfile(m_CalibDbHandle, &goc_data);
  DCT_ASSERT(result == RET_SUCCESS);

  redirectOut << __func__ << " (exit)" << std::endl;

  return (true);
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

  redirectOut << __func__ << " (enter)" << std::endl;

  CamCalibWdrGlobal_t wdr_data;
  memset(&wdr_data, 0, sizeof(wdr_data));
  uint32_t regValue = 0;

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());

    redirectOut << __func__ << " tagname: " << tagname << std::endl;
    if (tagname == CALIB_SENSOR_WDR_ENABLE_TAG) {
      int no = ParseUshortArray(tag.Value(), &wdr_data.Enabled, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_WDR_MODE_TAG) {
      int no = ParseUshortArray(tag.Value(), &wdr_data.Mode, 1);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_WDR_LOCAL_CURVE_TAG) {
      int i = (sizeof(wdr_data.LocalCurve) / sizeof(wdr_data.LocalCurve[0]));
      int no = ParseUshortArray(tag.Value(), wdr_data.LocalCurve, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_WDR_GLOBAL_CURVE_TAG) {
      int i = (sizeof(wdr_data.GlobalCurve) / sizeof(wdr_data.GlobalCurve[0]));
      int no = ParseUshortArray(tag.Value(), wdr_data.GlobalCurve, i);
      DCT_ASSERT((no == tag.Size()));
    } else if (tagname == CALIB_SENSOR_WDR_NOISE_RATIO_TAG
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_noiseratio = regValue;
    } else if (tagname == CALIB_SENSOR_WDR_BEST_LIGHT_TAG
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_bestlight = regValue;
    } else if (tagname == CALIB_SENSOR_WDR_GAIN_OFF1_TAG
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_gain_off1 = regValue;
    } else if (tagname == CALIB_SENSOR_WDR_PYM_TAG
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_pym_cc = regValue;
    } else if (tagname == CALIB_SENSOR_WDR_EPSILON_TAG
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_epsilon = regValue;
    } else if (tagname == CALIB_SENSOR_WDR_LVL_TAG
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_lvl_en = regValue;
    } else if (tagname == CALIB_SENSOR_WDR_FLT_TAG
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_flt_sel = regValue;
    } else if (tagname == CALIB_SENSOR_WDR_GAIN_MAX_CLIP_ENABLE_TAG
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_gain_max_clip_enable = regValue;
    } else if (tagname == CALIB_SENSOR_WDR_GAIN_MAX_VALUE_TAG
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_gain_max_value = regValue;
    } else if (tagname == CALIB_SENSOR_WDR_BAVG_TAG
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_bavg_clip = regValue;
    } else if (tagname == CALIB_SENSOR_WDR_NONL_SEGM_TAG
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_nonl_segm = regValue;
    } else if (tagname == CALIB_SENSOR_WDR_NONL_OPEN_TAG
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_nonl_open = regValue;
    } else if (tagname == CALIB_SENSOR_WDR_NONL_MODE1_TAG
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_nonl_mode1 = regValue;
    } else if (tagname == CALIB_SENSOR_WDR_COE0_TAG
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_coe0 = regValue;
    } else if (tagname == CALIB_SENSOR_WDR_COE1_TAG
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_coe1 = regValue;
    } else if (tagname == CALIB_SENSOR_WDR_COE2_TAG
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_coe2 = regValue;
    } else if (tagname == CALIB_SENSOR_WDR_COE_OFF_TAG
               && (tag.isType(XmlTag::TAG_TYPE_CHAR))
               && (tag.Size() > 0)) {
      regValue = 0;
      ParseCharToHex(&tag, &regValue);
      wdr_data.wdr_coe_off = regValue;
    } else if (tagname == CALIB_SENSOR_WDR_MAXGAIN_FILTER_TAG) {
      int n_sensor_gains     = 0;
      float* pf_sensor_gain_level   = NULL;
      int n_maxgain      = 0;
      float* pf_maxgain_level    = NULL;
      float filter_enable = 0.0;

      const XMLNode* psubchild = pchild->ToElement()->FirstChild();

      while (psubchild) {
        XmlTag tag = XmlTag(psubchild->ToElement());
        std::string subTagname(psubchild->ToElement()->Name());

        redirectOut << "subTagname: " << subTagname << std::endl;

        if ((subTagname == CALIB_SENSOR_WDR_MAXGAIN_FILTER_ENABLE)
            && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
            && (tag.Size() > 0)) {
          int no = ParseFloatArray(tag.Value(), &filter_enable, 1);
          DCT_ASSERT((no == 1));
          redirectOut << "subTagname: " << filter_enable << std::endl;
        } else if ((subTagname == CALIB_SENSOR_WDR_MAXGAIN_SENSOR_GAIN_LEVEL_TAG)
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          if (!pf_sensor_gain_level) {
            n_sensor_gains = tag.Size();
            pf_sensor_gain_level  = (float*)malloc((n_sensor_gains * sizeof(float)));
            MEMSET(pf_sensor_gain_level, 0, (n_sensor_gains * sizeof(float)));
          }

          int no = ParseFloatArray(tag.Value(), pf_sensor_gain_level, n_sensor_gains);
          DCT_ASSERT((no == n_sensor_gains));
        } else if ((subTagname == CALIB_SENSOR_WDR_MAXGAIN_MAXGAIN_LEVEL_TAG)
                   && (tag.isType(XmlTag::TAG_TYPE_DOUBLE))
                   && (tag.Size() > 0)) {
          if (!pf_maxgain_level) {
            n_maxgain = tag.Size();
            pf_maxgain_level = (float*)malloc((n_maxgain * sizeof(float)));
            MEMSET(pf_maxgain_level, 0, (n_maxgain * sizeof(float)));
          }

          int no = ParseFloatArray(tag.Value(), pf_maxgain_level, n_maxgain);
          DCT_ASSERT((no == n_maxgain));
        } else {
#if 1
          redirectOut
              << "parse error in Dpf denoiselevel curve section (unknow tag: "
              << subTagname
              << ")"
              << std::endl;
#endif
          return (false);
        }
        psubchild = psubchild->NextSibling();
      }

      DCT_ASSERT((n_sensor_gains == n_maxgain));
      wdr_data.wdr_MaxGain_Level_curve.nSize = n_sensor_gains;
      if (filter_enable >= 1.0) {
        wdr_data.wdr_MaxGain_Level_curve.filter_enable = 1;
      } else {
        wdr_data.wdr_MaxGain_Level_curve.filter_enable = 0;
      }
      wdr_data.wdr_MaxGain_Level_curve.pfSensorGain_level = pf_sensor_gain_level;
      wdr_data.wdr_MaxGain_Level_curve.pfMaxGain_level = pf_maxgain_level;
      for (int index = 0; index < n_sensor_gains; index++) {
        //wdr_data.wdr_MaxGain_limit.pfMaxGain_level[index] = (uint16_t)(pf_maxgain_level[index]);
        redirectOut << "Sensorgain[: " << index << "]= " << wdr_data.wdr_MaxGain_Level_curve.pfSensorGain_level[index] << std::endl;
        redirectOut << "Maxgain[: " << index << "]= " << wdr_data.wdr_MaxGain_Level_curve.pfMaxGain_level[index] << std::endl;
      }

    }
    pchild = pchild->NextSibling();
  }

  RESULT result = CamCalibDbAddWdrGlobal(m_CalibDbHandle, &wdr_data);
  DCT_ASSERT(result == RET_SUCCESS);

  redirectOut << __func__ << " (exit)" << std::endl;

  return (true);
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

  redirectOut << __func__ << " (enter)" << std::endl;

  CamCalibSystemData_t system_data;
  MEMSET(&system_data, 0, sizeof(CamCalibSystemData_t));

  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    const char* value = tag.Value();
    std::string tagname(pchild->ToElement()->Name());

    redirectOut << "tagname: " << tagname << std::endl;

    if (tagname == CALIB_SYSTEM_AFPS_TAG) {
      const XMLNode* firstchild = pchild->ToElement()->FirstChild();
      if (firstchild) {
        XmlTag firstTag = XmlTag(firstchild->ToElement());
        std::string firstTagname(firstchild->ToElement()->Name());
        redirectOut << "tag: " << firstTagname << std::endl;

        if ((firstTagname == CALIB_SYSTEM_AFPS_DEFAULT_TAG)
            && (firstTag.isType(XmlTag::TAG_TYPE_CHAR))
            && (firstTag.Size() > 0)) {
          const char* value = firstTag.Value();
          std::string s_value(value);
          size_t find = s_value.find("on", 0);
          system_data.AfpsDefault = ((find == std::string::npos) ? BOOL_FALSE : BOOL_TRUE);
        }

      }
    } else {
#if 1
      redirectOut
          << "parse error in system section (unknow tag: "
          << tagname
          << ")"
          << std::endl;
#endif

      return (false);
    }

    pchild = pchild->NextSibling();
  }

  RESULT result = CamCalibDbSetSystemData(m_CalibDbHandle, &system_data);
  DCT_ASSERT(result == RET_SUCCESS);

  redirectOut << __func__ << " (exit)" << std::endl;

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
  redirectOut << __func__ << " (enter)" << std::endl;
  CamCprocProfile_t*  cprocProfile = (CamCprocProfile_t*)param;
  struct  CamCprocData_s* cproc_data = &(cprocProfile->cproc[cprocProfile->num_items++]);
  memset(cproc_data, 0, sizeof(struct CamCprocData_s));
  const XMLNode* pchild = pelement->FirstChild();
  while (pchild) {
    XmlTag tag = XmlTag(pchild->ToElement());
    std::string tagname(pchild->ToElement()->Name());
    redirectOut << tagname << std::endl;

    if ((tagname == CALIB_SENSOR_CPROC_NAME_TAG)
        && (tag.isType(XmlTag::TAG_TYPE_CHAR))
        && (tag.Size() > 0)) {
      strncpy(cproc_data->name, tag.Value(), sizeof(cproc_data->name));
      if (strcmp(cproc_data->name, "PREVIEW") == 0)
        cproc_data->cproc_scenario = CAM_CPROC_USECASE_PREVIEW;
      else if (strcmp(cproc_data->name, "CAPTURE") == 0)
        cproc_data->cproc_scenario = CAM_CPROC_USECASE_CAPTURE;
      else if (strcmp(cproc_data->name, "VIDEO") == 0)
        cproc_data->cproc_scenario = CAM_CPROC_USECASE_VIDEO;
      redirectOut << __func__ << " name:" << cproc_data->name << std::endl;
    } else if (tagname == CALIB_SENSOR_CPROC_SATURATION_TAG) {
      int no = ParseFloatArray(tag.Value(), &cproc_data->cproc_saturation, 1);
      DCT_ASSERT((no == tag.Size()));
      redirectOut << __func__ << " saturation:" << cproc_data->cproc_saturation << std::endl;
    } else if (tagname == CALIB_SENSOR_CPROC_CONTRAST_TAG) {
      int no = ParseFloatArray(tag.Value(), &cproc_data->cproc_contrast, 1);
      DCT_ASSERT((no == tag.Size()));
      redirectOut << __func__ << " contrast:" << cproc_data->cproc_contrast << std::endl;
    } else if (tagname == CALIB_SENSOR_CPROC_BRIGHTNESS_TAG) {
      float value = 0;
      int no = ParseFloatArray(tag.Value(), &value, 1);
      DCT_ASSERT((no == tag.Size()));
      cproc_data->cproc_brightness = (char)(value);
      redirectOut << __func__ << " brightness:" << (int)(cproc_data->cproc_brightness) << std::endl;
    } else if (tagname == CALIB_SENSOR_CPROC_HUE_TAG) {
      int no = ParseFloatArray(tag.Value(), &cproc_data->cproc_hue, 1);
      DCT_ASSERT((no == tag.Size()));
      redirectOut << __func__ << " hue:" << cproc_data->cproc_hue << std::endl;
    }

    pchild = pchild->NextSibling();
  }

  RESULT result = RET_SUCCESS;
  DCT_ASSERT(result == RET_SUCCESS);

  redirectOut << __func__ << " (exit)" << std::endl;

  return (true);
}



