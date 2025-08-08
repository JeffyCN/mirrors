// Copyright 2021 Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef __RK_ISPFEC_PARAM_H__
#define __RK_ISPFEC_PARAM_H__

#include "iniparser.h"

#ifdef __cplusplus
extern "C" {
#endif

int rk_ispfec_param_get_int(const char* entry, int default_val);
int rk_ispfec_param_set_int(const char* entry, int val);
double rk_ispfec_param_get_double(const char* entry, double default_val);
const char* rk_ispfec_param_get_string(const char* entry, const char* default_val);
int rk_ispfec_param_set_string(const char* entry, const char* val);
int rk_ispfec_param_save();
int rk_ispfec_param_init(char* ini_path);
int rk_ispfec_param_deinit();
int rk_ispfec_param_reload();

#ifdef __cplusplus
}
#endif

#endif