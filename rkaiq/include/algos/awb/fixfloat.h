#ifndef __FIXFLOAT_H__
#define __FIXFLOAT_H__

#include  "rk_aiq_comm.h"
RKAIQ_BEGIN_DECLARE
uint16_t UtlFloatToFix_U0012( float fFloat );
int16_t UtlFloatToFix_S0310( float fFloat );
int16_t UtlFloatToFix_S0312( float fFloat );
int16_t FloatToS16(float fFloat);
int16_t UtlFloatToFix_S0406( float fFloat );
RKAIQ_END_DECLARE
#endif