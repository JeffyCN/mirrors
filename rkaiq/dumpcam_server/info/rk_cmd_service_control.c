/*
 * Copyright 2020 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "rkaiq_type.h"
#include "argparse.h"
#include <string.h>
#ifdef HAVE_RK_OSAL
#include "rk_osal.h"
#else
#include "c_base/aiq_base.h"
#include "c_base/aiq_thread.h"
#endif
#include <fcntl.h>
#include "rk_info_utils.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "RKRecordServiceControl"


static RK_S32 _gHelp = 0;

static RK_U32 ShellItemExtract1(RK_U8 *pstr, RK_U8 **pItem, const RK_U8 *Space) {
    RK_U32 i = 0;
    RK_U32 ItemSize = 0;

    while (*pstr == *Space)
    {
        pstr++;
    }

    (*pItem) = (RK_U8 *)pstr;

    while ((*(pstr + i) != *Space) && (*(pstr + i)))
    {
        ItemSize++;
        i++;
    }
    return ItemSize;
}

static const char *const usages[] = {
    "./dumpsys cat /tmp/mpi/vlog",
    NULL,
};

static RK_S32 dbg_help_cb(struct argparse *self, const struct argparse_option *option) {
    _gHelp = 1;
    return 0;
}


static void cat_main(int argc, char **argv, st_string *result) {
    int fd = 0;
    char *path;
    int real_size;
    char buf[513];
    path = argv[1];
#ifdef HAVE_RK_OSAL
    fd = rkos_dev_open(path, O_RDWR | O_CLOEXEC);
#else
    fd = open(path, O_RDWR | O_CLOEXEC);
#endif
    if (fd < 0) {
#ifdef HAVE_RK_OSAL
       rkos_sprintf(buf + strlen(buf), "%s open fail fd = %d \n", path, fd);
#endif
       aiq_string_printf(result, buf);
       return;
    }

    do {
#ifdef HAVE_RK_OSAL
        real_size = rkos_dev_read(fd, buf, 512);
#else
        real_size = read(fd, buf, 512);
#endif
        buf[real_size] = 0;
        aiq_string_printf(result, buf);
    } while(real_size);

#ifdef HAVE_RK_OSAL
    rkos_dev_close(fd);
#else
    close(fd);
#endif
    return;
}

void aiq_cmd_service_process(char *cmd, st_string *result) {
    char *argv[10];
    int cnt[10];
    int argc = 0;
    RK_U32 str_cnt;
    RK_U8 *pitem;
    RK_U8 space = ' ';
#ifdef HAVE_RK_OSAL
    rkos_memset(argv, 0, sizeof(argv));
#else
    aiq_memset(argv, 0, sizeof(argv));
#endif

    str_cnt = ShellItemExtract1((RK_U8*)cmd, &pitem, &space);
    while (str_cnt)
    {
        argv[argc] = (char*)pitem;
        cnt[argc] = str_cnt;
        pitem[str_cnt] = 0;
        argc++;
        cmd = (char*)(pitem + str_cnt + 1);
        str_cnt = ShellItemExtract1((RK_U8*)cmd, &pitem, &space);
    }
    cat_main(argc, argv, result);

    for (int i = 1; i < argc - 1; i++)
        argv[i][cnt[i]] = ' ';
    return;
}



