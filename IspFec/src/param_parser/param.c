// Copyright 2021 Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <pthread.h>

#include "RkIspFecComm.h"
#include "iniparser.h"

#define MAX_SECTION_KEYS 1024

static char g_ini_path_[256];
static dictionary* g_ini_d_;
static pthread_mutex_t g_param_mutex = PTHREAD_MUTEX_INITIALIZER;

int rk_ispfec_param_dump() {
    const char* section_name;
    const char* keys[MAX_SECTION_KEYS];
    int section_keys;
    int section_num = rk_ispfec_iniparser_getnsec(g_ini_d_);
    rkfec_dbg(4, rkfec_debug, "section_num is %d", section_num);

    for (int i = 0; i < section_num; i++) {
        section_name = rk_ispfec_iniparser_getsecname(g_ini_d_, i);
        section_keys = rk_ispfec_iniparser_getsecnkeys(g_ini_d_, section_name);
        rkfec_dbg(4, rkfec_debug, "section_name is %s, section_keys is %d", section_name,
                  section_keys);
        for (int j = 0; j < section_keys; j++) {
            rk_ispfec_iniparser_getseckeys(g_ini_d_, section_name, keys);
            rkfec_dbg(4, rkfec_debug, "%s = %s", keys[j],
                      rk_ispfec_iniparser_getstring(g_ini_d_, keys[j], ""));
        }
    }

    return 0;
}

int rk_ispfec_param_save() {
    FILE* fp = fopen(g_ini_path_, "w");
    if (fp == NULL) {
        rkfec_err("%s, fopen error!", g_ini_path_);
        rk_ispfec_iniparser_freedict(g_ini_d_);
        g_ini_d_ = NULL;
        return -1;
    }
    rk_ispfec_iniparser_dump_ini(g_ini_d_, fp);

    fflush(fp);
    fclose(fp);

    return 0;
}

int rk_ispfec_param_get_int(const char* entry, int default_val) {
    int ret;
    pthread_mutex_lock(&g_param_mutex);
    ret = rk_ispfec_iniparser_getint(g_ini_d_, entry, default_val);
    pthread_mutex_unlock(&g_param_mutex);

    return ret;
}

double rk_ispfec_param_get_double(const char* entry, double default_val) {
    double ret;
    pthread_mutex_lock(&g_param_mutex);
    ret = rk_ispfec_iniparser_getdouble(g_ini_d_, entry, default_val);
    pthread_mutex_unlock(&g_param_mutex);

    return ret;
}

int rk_ispfec_param_set_int(const char* entry, int val) {
    char tmp[8];
    sprintf(tmp, "%d", val);
    pthread_mutex_lock(&g_param_mutex);
    rk_ispfec_iniparser_set(g_ini_d_, entry, tmp);
    pthread_mutex_unlock(&g_param_mutex);

    return 0;
}

const char* rk_ispfec_param_get_string(const char* entry, const char* default_val) {
    const char* ret;
    pthread_mutex_lock(&g_param_mutex);
    ret = rk_ispfec_iniparser_getstring(g_ini_d_, entry, default_val);
    pthread_mutex_unlock(&g_param_mutex);

    return ret;
}

int rk_ispfec_param_set_string(const char* entry, const char* val) {
    pthread_mutex_lock(&g_param_mutex);
    rk_ispfec_iniparser_set(g_ini_d_, entry, val);
    pthread_mutex_unlock(&g_param_mutex);

    return 0;
}

int rk_ispfec_param_init(char* ini_path) {
    rkfec_dbg(4, rkfec_debug, "%s", __func__);
    char cmd[256];
    pthread_mutex_lock(&g_param_mutex);
    g_ini_d_ = NULL;
    memset(g_ini_path_, 0, sizeof(g_ini_path_));
    if (ini_path)
        memcpy(g_ini_path_, ini_path, strlen(ini_path));
    else
        memcpy(g_ini_path_, "/userdata/rkfec_calib.ini", strlen("/userdata/rkfec_calib.ini"));
    rkfec_info("g_ini_path_ is %s", g_ini_path_);

    g_ini_d_ = rk_ispfec_iniparser_load(g_ini_path_);
    if (g_ini_d_ == NULL) {
        rkfec_err("rk_ispfec_iniparser_load %s error! use /tmp/rkfec-factory-config.ini",
                  g_ini_path_);
        snprintf(cmd, 127, "cp /tmp/rkfec-factory-config.ini %s", g_ini_path_);
        rkfec_info("cmd is %s", cmd);
        int ret = system(cmd);
        if (ret < 0) {
            rkfec_err("system (%s) error!", cmd);
            pthread_mutex_unlock(&g_param_mutex);
            return -1;
        }
        g_ini_d_ = rk_ispfec_iniparser_load(g_ini_path_);
        if (g_ini_d_ == NULL) {
            rkfec_err("rk_ispfec_iniparser_load error again!");
            pthread_mutex_unlock(&g_param_mutex);
            return -1;
        }
    }
    rk_ispfec_param_dump();
    pthread_mutex_unlock(&g_param_mutex);

    return 0;
}

int rk_ispfec_param_deinit() {
    rkfec_info("%s", __func__);
    if (g_ini_d_ == NULL) return 0;
    pthread_mutex_lock(&g_param_mutex);
    // rk_ispfec_param_save();
    if (g_ini_d_) rk_ispfec_iniparser_freedict(g_ini_d_);
    g_ini_d_ = NULL;
    pthread_mutex_unlock(&g_param_mutex);

    return 0;
}

int rk_ispfec_param_reload() {
    rkfec_info("%s", __func__);
    pthread_mutex_lock(&g_param_mutex);
    if (g_ini_d_) rk_ispfec_iniparser_freedict(g_ini_d_);
    g_ini_d_ = rk_ispfec_iniparser_load(g_ini_path_);
    if (g_ini_d_ == NULL) {
        rkfec_err("rk_ispfec_iniparser_load error!");
        pthread_mutex_unlock(&g_param_mutex);
        return -1;
    }
    rk_ispfec_param_dump();
    pthread_mutex_unlock(&g_param_mutex);

    return 0;
}
