/*
 * Copyright (c) 2018, Fuzhou Rockchip Electronics Co., Ltd
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

#ifndef __RKCAMERA_VENDOR_TAGS_H__
#define __RKCAMERA_VENDOR_TAGS_H__ 

// Camera dependencies
#include "system/camera_metadata.h"
#include "system/camera_vendor_tags.h"

enum rkcamera3_ext_section {
    RKCAMERA3_PRIVATEDATA = VENDOR_SECTION,
    RKCAMERA3_EXT_SECTION_END
};

enum rkcamera3_ext_section_ranges {
    RKCAMERA3_PRIVATEDATA_START = RKCAMERA3_PRIVATEDATA << 16,
};

enum rkcamera3_ext_tags {
    RKCAMERA3_PRIVATEDATA_EFFECTIVE_DRIVER_FRAME_ID = RKCAMERA3_PRIVATEDATA_START,
    RKCAMERA3_PRIVATEDATA_FRAME_SOF_TIMESTAMP,
    RKCAMERA3_PRIVATEDATA_END,
};

class RkCamera3VendorTags {
    public:
        static void get_vendor_tag_ops(vendor_tag_ops_t* ops);
        static int get_tag_count(const vendor_tag_ops_t *ops);
        static void get_all_tags(const vendor_tag_ops_t *ops, uint32_t *tag_array);
        static const char* get_section_name(const vendor_tag_ops_t *ops, uint32_t tag);
        static const char* get_tag_name(const vendor_tag_ops_t *ops, uint32_t tag);
        static int get_tag_type(const vendor_tag_ops_t *ops, uint32_t tag);
        static const vendor_tag_ops_t *Ops;
};

#endif /* __RKCAMERA_VENDOR_TAGS_H__ */
