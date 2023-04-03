/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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
#ifndef RK_AIQ_CORE_CONFIG_H
#define RK_AIQ_CORE_CONFIG_H

#include <unordered_map>

#include "MessageBus.h"
#include "rk_aiq_algo_des.h"
#include "rk_aiq_types_priv.h"

#define grp_array_size(x)       (sizeof(x) / sizeof(x[0]))
#define grp_conds_array_info(x) x, grp_array_size(x)

namespace RkCam {

typedef struct RkAiqGrpCondition_s {
    XCamMessageType cond;
    int8_t delay;
} RkAiqGrpCondition_t;

typedef struct RkAiqGrpConditions_s {
    RkAiqGrpCondition_t* conds;
    u32 size;
} RkAiqGrpConditions_t;

struct RkAiqAlgoDesCommExt {
    RkAiqAlgoDesComm* des;
    enum rk_aiq_core_analyze_type_e group;
    uint8_t algo_ver;
    uint8_t module_hw_ver;
    uint8_t handle_ver;
    RkAiqGrpConditions_t grpConds;
};

inline std::string AlgoTypeToString(RkAiqAlgoType_t type) {
    static std::unordered_map<uint32_t, std::string> str_map = {
        // clang-format off
        { RK_AIQ_ALGO_TYPE_AE,          "Ae"        },
        { RK_AIQ_ALGO_TYPE_AWB,         "Awb"       },
        { RK_AIQ_ALGO_TYPE_AF,          "Af"        },
        { RK_AIQ_ALGO_TYPE_ABLC,        "Ablc"      },
        { RK_AIQ_ALGO_TYPE_ADPCC,       "Adpcc"     },
        { RK_AIQ_ALGO_TYPE_AMERGE,      "Amerge"    },
        { RK_AIQ_ALGO_TYPE_ATMO,        "Atmo"      },
        { RK_AIQ_ALGO_TYPE_ANR,         "Anr"       },
        { RK_AIQ_ALGO_TYPE_ALSC,        "Alsc"      },
        { RK_AIQ_ALGO_TYPE_AGIC,        "Agic"      },
        { RK_AIQ_ALGO_TYPE_ADEBAYER,    "Adebayer"  },
        { RK_AIQ_ALGO_TYPE_ACCM,        "Accm"      },
        { RK_AIQ_ALGO_TYPE_AGAMMA,      "Agamma"    },
        { RK_AIQ_ALGO_TYPE_AWDR,        "Awdr"      },
        { RK_AIQ_ALGO_TYPE_ADHAZ,       "Adehaze"   },
        { RK_AIQ_ALGO_TYPE_A3DLUT,      "A3dlut"    },
        { RK_AIQ_ALGO_TYPE_ALDCH,       "Aldch"     },
        { RK_AIQ_ALGO_TYPE_ACSM,        "Acsm"      },
        { RK_AIQ_ALGO_TYPE_ACP,         "Acp"       },
        { RK_AIQ_ALGO_TYPE_AIE,         "Aie"       },
        { RK_AIQ_ALGO_TYPE_ASHARP,      "Asharp"    },
        { RK_AIQ_ALGO_TYPE_AORB,        "Aorb"      },
        { RK_AIQ_ALGO_TYPE_ACGC,        "Acgc"      },
        { RK_AIQ_ALGO_TYPE_ASD,         "Asd"       },
        { RK_AIQ_ALGO_TYPE_ADRC,        "Adrc"      },
        { RK_AIQ_ALGO_TYPE_ADEGAMMA,    "Adegamma"  },
#if defined(ISP_HW_V30) || defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
        { RK_AIQ_ALGO_TYPE_ARAWNR,      "Abayer2dnr"},
        { RK_AIQ_ALGO_TYPE_AMFNR,       "Abayertnr" },
#else
        { RK_AIQ_ALGO_TYPE_ARAWNR,      "Arawnr"    },
        { RK_AIQ_ALGO_TYPE_AMFNR,       "Amfnr"     },
#endif
        { RK_AIQ_ALGO_TYPE_AYNR,        "Aynr"      },
        { RK_AIQ_ALGO_TYPE_ACNR,        "Acnr"      },
        { RK_AIQ_ALGO_TYPE_AEIS,        "Aeis"      },
        { RK_AIQ_ALGO_TYPE_AFEC,        "Afec"      },
        { RK_AIQ_ALGO_TYPE_AMD,         "Amd"       },
        { RK_AIQ_ALGO_TYPE_AGAIN,       "Again"     },
        { RK_AIQ_ALGO_TYPE_ACAC,        "Acac"      },
        // clang-format oon
    };

    return str_map[uint32_t(type)];
}


}  // namespace RkCam

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#ifdef ISP_HW_V20
#include "RkAiqCoreConfigV20.h"
#endif

#ifdef ISP_HW_V21
#include "RkAiqCoreConfigV21.h"
#endif

#ifdef ISP_HW_V30
#include "RkAiqCoreConfigV30.h"
#endif

#if defined(ISP_HW_V32) || defined(ISP_HW_V32_LITE)
#include "RkAiqCoreConfigV32.h"
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif  // RK_AIQ_CORE_CONFIG_H
