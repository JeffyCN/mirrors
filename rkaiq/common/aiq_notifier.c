/*
 *   Copyright (c) 2024 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "common/aiq_notifier.h"

#include "dumpcam_server/info/include/rk_info_utils.h"
#include "dumpcam_server/info/include/st_string.h"

static int aiq_notifier_subscriber_valid(struct aiq_notifier_subscriber* sub) {
    switch (sub->match_type) {
        case AIQ_NOTIFIER_MATCH_HWI_BASE:
        case AIQ_NOTIFIER_MATCH_HWI_STREAM_CAP:
        case AIQ_NOTIFIER_MATCH_HWI_STREAM_PROC:
        case AIQ_NOTIFIER_MATCH_HWI_SENSOR:
        case AIQ_NOTIFIER_MATCH_HWI_ISP_PARAMS:
        case AIQ_NOTIFIER_MATCH_HWI_ISP_ACTIVE_PARAMS:
        case AIQ_NOTIFIER_MATCH_CORE:
        case AIQ_NOTIFIER_MATCH_CORE_BUF_MGR:
        case AIQ_NOTIFIER_MATCH_CORE_GRP_ANALYZER:
        case AIQ_NOTIFIER_MATCH_CORE_ISP_PARAMS:
        case AIQ_NOTIFIER_MATCH_AEC:
        case AIQ_NOTIFIER_MATCH_HIST:
        case AIQ_NOTIFIER_MATCH_AWB:
        case AIQ_NOTIFIER_MATCH_AWBGAIN:
        case AIQ_NOTIFIER_MATCH_AF:
        case AIQ_NOTIFIER_MATCH_DPCC:
        case AIQ_NOTIFIER_MATCH_MERGE:
        case AIQ_NOTIFIER_MATCH_CCM:
        case AIQ_NOTIFIER_MATCH_LSC:
        case AIQ_NOTIFIER_MATCH_BLC:
        case AIQ_NOTIFIER_MATCH_RAWNR:
        case AIQ_NOTIFIER_MATCH_GIC:
        case AIQ_NOTIFIER_MATCH_DEBAYER:
        case AIQ_NOTIFIER_MATCH_LUT3D:
        case AIQ_NOTIFIER_MATCH_DEHAZE:
        case AIQ_NOTIFIER_MATCH_AGAMMA:
        case AIQ_NOTIFIER_MATCH_ADEGAMMA:
        case AIQ_NOTIFIER_MATCH_CSM:
        case AIQ_NOTIFIER_MATCH_CGC:
        case AIQ_NOTIFIER_MATCH_GAIN:
        case AIQ_NOTIFIER_MATCH_CP:
        case AIQ_NOTIFIER_MATCH_IE:
        case AIQ_NOTIFIER_MATCH_TNR:
        case AIQ_NOTIFIER_MATCH_YNR:
        case AIQ_NOTIFIER_MATCH_CNR:
        case AIQ_NOTIFIER_MATCH_SHARPEN:
        case AIQ_NOTIFIER_MATCH_DRC:
        case AIQ_NOTIFIER_MATCH_CAC:
        case AIQ_NOTIFIER_MATCH_AFD:
        case AIQ_NOTIFIER_MATCH_RGBIR:
        case AIQ_NOTIFIER_MATCH_LDC:
        case AIQ_NOTIFIER_MATCH_AESTATS:
        case AIQ_NOTIFIER_MATCH_HISTEQ:
        case AIQ_NOTIFIER_MATCH_ENH:
        case AIQ_NOTIFIER_MATCH_TEXEST:
        case AIQ_NOTIFIER_MATCH_HSV:
        case AIQ_NOTIFIER_MATCH_AIBNR:
        case AIQ_NOTIFIER_MATCH_AIYNR:
            break;
        default:
            printf("Invalid match type %u on %s\n", sub->match_type, sub->name);
            return -1;
    }

    return 0;
}

void aiq_notifier_init(struct aiq_notifier* notifier) { INIT_LIST_HEAD(&notifier->sub_list); }

int aiq_notifier_add_subscriber(struct aiq_notifier* notifier,
                                struct aiq_notifier_subscriber* sub) {
    int ret;

    // mutex_lock(&list_lock);

    ret = aiq_notifier_subscriber_valid(sub);
    if (ret) goto unlock;

    list_add_tail(&sub->list, &notifier->sub_list);

unlock:
    // mutex_unlock(&list_lock);
    return ret;
}

int aiq_notifier_remove_subscriber(struct aiq_notifier* notifier, int type) {
    if (!notifier) return -1;

    struct aiq_notifier_subscriber *sub = NULL, *tmp = NULL;
    list_for_each_entry_safe(sub, tmp, &notifier->sub_list, list) {
        if (!sub->dump.dumper || !sub->dump.dump_fn_t) continue;

        if (sub->match_type == (enum aiq_notifier_match_type)type) list_del(&sub->list);
    }

    return 0;
}

int aiq_notifier_notify_dumpinfo(struct aiq_notifier* notifier, int type, st_string* dump_info,
                                 int argc, void* argv[]) {
    struct aiq_notifier_subscriber *sub, *tmp;
    char buffer[MAX_LINE_LENGTH] = {0};

    list_for_each_entry_safe(sub, tmp, &notifier->sub_list, list) {
        if (type != AIQ_NOTIFIER_MATCH_ALL && (enum aiq_notifier_match_type)type != sub->match_type)
            continue;

        if (!sub->dump.dumper || !sub->dump.dump_fn_t) continue;

        if (type != AIQ_NOTIFIER_MATCH_ALL)
            aiq_info_dump_mod_name(dump_info, aiq_notifier_module_name(notifier, type));

        if (sub->match_type < _MODS_NUM_OFFSET) aiq_info_dump_submod_name(dump_info, sub->name);

        sub->dump.dump_fn_t(sub->dump.dumper, dump_info, argc, argv);
        aiq_string_printf(dump_info, "\n");
    }

    return 0;
}

const char* aiq_notifier_notify_subscriber_name(struct aiq_notifier* notifier, int type) {
    struct aiq_notifier_subscriber *sub, *tmp;

    list_for_each_entry_safe(sub, tmp, &notifier->sub_list, list) {
        if ((enum aiq_notifier_match_type)type != sub->match_type) continue;

        if (!sub->dump.dumper || !sub->dump.dump_fn_t) continue;

        return sub->name;
    }

    return NULL;
}

const char* aiq_notifier_module_name(struct aiq_notifier* notifier, int type) {
    enum aiq_notifier_match_type match_type = (enum aiq_notifier_match_type)type;

    if (match_type >= AIQ_NOTIFIER_MATCH_HWI_BASE && match_type < AIQ_NOTIFIER_MATCH_HWI_ALL)
        return "HWI";

    if (match_type >= AIQ_NOTIFIER_MATCH_CORE && match_type < AIQ_NOTIFIER_MATCH_CORE_ALL)
        return "CORE";

    if (match_type >= _MODS_NUM_OFFSET && match_type < AIQ_NOTIFIER_MATCH_MODS_ALL)
        return NotifierMatchType2Str[match_type];

    return NULL;
}
