#ifndef _RANGE_CHECK_H_
#define _RANGE_CHECK_H_

#include "xcam_log.h"

#define ENABLE_RANGE_CHECK_ATTRIB

#define ENABLE_RANGE_CHECK_PARAM

// define the log function when range check fail
#define CHECK_LOGE(format) LOGE(format)

// use empty define if do not want range check
//#define CHECK_NUM_IN_RANGE(val, min, max)

// if value out of range, return false
#define CHECK_NUM_IN_RANGE(val, min, max) \
    if (val < min || val > max) { \
        CHECK_LOGE(#val " out of range!"); \
        return false; \
    }

#endif
