/*
 * Copyright 2015 Rockchip Electronics Co. LTD
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
 */

#define MODULE_TAG "mpp_platform"

#include <string.h>

#include "mpp_env.h"
#include "mpp_debug.h"
#include "mpp_common.h"
#include "mpp_platform.h"
#include "mpp_service.h"

static MppKernelVersion check_kernel_version(void)
{
    static const char *kernel_version_path = "/proc/version";
    MppKernelVersion version = KERNEL_UNKNOWN;
    FILE *fp = NULL;
    char buf[32];

    if (access(kernel_version_path, F_OK | R_OK))
        return version;

    fp = fopen(kernel_version_path, "rb");
    if (fp) {
        size_t len = fread(buf, 1, sizeof(buf) - 1, fp);
        char *pos = NULL;

        buf[len] = '\0';
        pos = strstr(buf, "Linux version ");
        if (pos) {
            RK_S32 major = 0;
            RK_S32 minor = 0;
            RK_S32 last = 0;
            RK_S32 count = 0;

            pos += 14;
            count = sscanf(pos, "%d.%d.%d ", &major, &minor, &last);
            if (count >= 2 && major > 0 && minor > 0) {
                switch (major) {
                case 3: {
                    version = KERNEL_3_10;
                } break;
                case 4: {
                    version = KERNEL_4_4;
                    if (minor >= 19)
                        version = KERNEL_4_19;
                } break;
                case 5: {
                    version = KERNEL_5_10;
                } break;
                default: break;
                }
            }
        }
        fclose(fp);
    }
    return version;
}

class MppPlatformService
{
private:
    // avoid any unwanted function
    MppPlatformService();
    ~MppPlatformService() {};
    MppPlatformService(const MppPlatformService &);
    MppPlatformService &operator=(const MppPlatformService &);

    MppIoctlVersion     ioctl_version;
    MppKernelVersion    kernel_version;
    RK_U32              vcodec_type;
    RK_U32              hw_ids[32];
    MppServiceCmdCap    mpp_service_cmd_cap;
    const MppSocInfo    *soc_info;
    const char          *soc_name;

public:
    static MppPlatformService *get_instance() {
        static MppPlatformService instance;
        return &instance;
    }

    MppIoctlVersion     get_ioctl_version(void) { return ioctl_version; };
    MppKernelVersion    get_kernel_version(void) { return kernel_version; };
    const char          *get_soc_name() { return soc_name; };
    MppServiceCmdCap    *get_mpp_service_cmd_cap() { return &mpp_service_cmd_cap; };
    RK_U32              get_hw_id(RK_S32 client_type);
    RK_U32              get_vcodec_type(void) { return vcodec_type; };
};

MppPlatformService::MppPlatformService()
    : ioctl_version(IOCTL_MPP_SERVICE_V1),
      kernel_version(KERNEL_UNKNOWN),
      vcodec_type(0),
      soc_info(NULL),
      soc_name(NULL)
{
    /* judge vdpu support version */
    MppServiceCmdCap *cap = &mpp_service_cmd_cap;

    /* default value */
    cap->support_cmd = 0;
    cap->query_cmd = MPP_CMD_QUERY_BASE + 1;
    cap->init_cmd = MPP_CMD_INIT_BASE + 1;
    cap->send_cmd = MPP_CMD_SEND_BASE + 1;
    cap->poll_cmd = MPP_CMD_POLL_BASE + 1;
    cap->ctrl_cmd = MPP_CMD_CONTROL_BASE + 0;

    mpp_env_get_u32("mpp_debug", &mpp_debug, 0);

    /* read soc name */
    soc_name = mpp_get_soc_name();
    soc_info = mpp_get_soc_info();

    if (soc_info->soc_type == ROCKCHIP_SOC_AUTO)
        mpp_log("can not found match soc name: %s\n", soc_name);

    ioctl_version = IOCTL_VCODEC_SERVICE;
    if (mpp_get_mpp_service_name()) {
        ioctl_version = IOCTL_MPP_SERVICE_V1;
        check_mpp_service_cap(&vcodec_type, hw_ids, cap);
        mpp_dbg_platform("vcodec_type from kernel 0x%08x, vs from soc info 0x%08x\n",
                         vcodec_type, soc_info->vcodec_type);
    }
    kernel_version = check_kernel_version();
    if (!vcodec_type) {
        vcodec_type = soc_info->vcodec_type;
    } else {
        // Compare kernel result with soc infomation.
        RK_U32 diff_type = vcodec_type ^ soc_info->vcodec_type;
        RK_U32 i;

        for (i = 0; i <= VPU_CLIENT_VEPU22; i++) {
            RK_U32 mask = 1 << i;

            if (diff_type & mask) {
                MppClientType client_type = (MppClientType) i;

                mpp_dbg_platform("confliction found at client_type %d\n", client_type);

                if (soc_info->vcodec_type & mask) {
                    mpp_err("client %d driver is not ready!\n", client_type);
                } else {
                    mpp_dbg_platform("client %d driver is ready but not declared!\n", client_type);
                    if (client_type == VPU_CLIENT_VDPU2_PP)
                        vcodec_type &= ~mask;
                }
            }
        }

        mpp_dbg_platform("vcode_type 0x%08x\n", vcodec_type);
    }
}

RK_U32 MppPlatformService::get_hw_id(RK_S32 client_type)
{
    RK_U32 hw_id = 0;

    if (vcodec_type & (1 << client_type))
        hw_id = hw_ids[client_type];

    return hw_id;
}

MppIoctlVersion mpp_get_ioctl_version(void)
{
    return MppPlatformService::get_instance()->get_ioctl_version();
}

MppKernelVersion mpp_get_kernel_version(void)
{
    return MppPlatformService::get_instance()->get_kernel_version();
}

RK_U32 mpp_get_2d_hw_flag(void)
{
    RK_U32 flag = 0;

    if (!access("/dev/rga", F_OK))
        flag |= HAVE_RGA;

    if (!access("/dev/iep", F_OK))
        flag |= HAVE_IEP;

    return flag;
}

const MppServiceCmdCap *mpp_get_mpp_service_cmd_cap(void)
{
    return MppPlatformService::get_instance()->get_mpp_service_cmd_cap();
}

RK_U32 mpp_get_client_hw_id(RK_S32 client_type)
{
    return MppPlatformService::get_instance()->get_hw_id(client_type);
}

RK_U32 mpp_get_vcodec_type(void)
{
    static RK_U32 vcodec_type = 0;

    if (!vcodec_type)
        vcodec_type = MppPlatformService::get_instance()->get_vcodec_type();

    return vcodec_type;
}
