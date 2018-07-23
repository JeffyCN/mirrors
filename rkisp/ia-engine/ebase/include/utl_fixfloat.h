/*****************************************************************************/
/*!
 *  @file        utl_fixfloat.h
 *  @version     1.0
 *  @author      Neugebauer
 *  @brief       Floatingpoint to Fixpoint and vice versa conversion
 *               routines.
 */
/*  This is an unpublished work, the copyright in which vests in Silicon Image
 *  GmbH. The information contained herein is the property of Silicon Image GmbH
 *  and is supplied without liability for errors or omissions. No part may be
 *  reproduced or used expect as authorized by contract or other written
 *  permission. Copyright(c) Silicon Image GmbH, 2009, all rights reserved.
 */
/*****************************************************************************/

#ifndef __UTL_FIXFLOAT_H__
#define __UTL_FIXFLOAT_H__
#ifdef __cplusplus
extern "C"
{
#endif
/*****************************************************************************
 * Prototypes
 *****************************************************************************/
#define UTL_FIX_MAX_U0208          3.998f //exactly this would be < 4 - 0.5/256
#define UTL_FIX_MIN_U0208          0.0f

uint32_t UtlFloatToFix_U0402(float fFloat);

uint32_t UtlFloatToFix_U0107(float fFloat);
float  UtlFixToFloat_U0107(uint32_t ulFix);

uint32_t UtlFloatToFix_U0208(float fFloat);
float  UtlFixToFloat_U0208(uint32_t ulFix);

uint32_t UtlFloatToFix_U0408(float fFloat);
float  UtlFixToFloat_U0408(uint32_t ulFix);

uint32_t UtlFloatToFix_U0800(float fFloat);
float  UtlFixToFloat_U0800(uint32_t ulFix);

uint32_t UtlFloatToFix_U1000(float fFloat);
float  UtlFixToFloat_U1000(uint32_t ulFix);

uint32_t UtlFloatToFix_U1200(float fFloat);
float  UtlFixToFloat_U1200(uint32_t ulFix);

uint32_t UtlFloatToFix_U0010(float fFloat);
float  UtlFixToFloat_U0010(uint32_t ulFix);

uint32_t UtlFloatToFix_S0207(float fFloat);
float  UtlFixToFloat_S0207(uint32_t ulFix);

uint32_t UtlFloatToFix_S0307(float fFloat);
float  UtlFixToFloat_S0307(uint32_t ulFix);

uint32_t UtlFloatToFix_S0407(float fFloat);
float  UtlFixToFloat_S0407(uint32_t ulFix);

uint32_t UtlFloatToFix_S0504(float fFloat);
float  UtlFixToFloat_S0504(uint32_t ulFix);

uint32_t UtlFloatToFix_S0808(float fFloat);
float  UtlFixToFloat_S0808(uint32_t ulFix);

uint32_t UtlFloatToFix_S0800(float fFloat);
float  UtlFixToFloat_S0800(uint32_t ulFix);

uint32_t UtlFloatToFix_S0900(float fFloat);
float  UtlFixToFloat_S0900(uint32_t ulFix);

uint32_t UtlFloatToFix_S1200(float fFloat);
float  UtlFixToFloat_S1200(uint32_t ulFix);

uint32_t UtlFloatToFix_S0109(float fFloat);
float UtlFixToFloat_S0109(uint32_t ulFix);

uint32_t UtlFloatToFix_S0408(float fFloat);
float UtlFixToFloat_S0408(uint32_t ulFix);

uint32_t UtlFloatToFix_S0108(float fFloat);
float UtlFixToFloat_S0108(uint32_t ulFix);

uint32_t UtlFloatToFix_S0110(float fFloat);
float UtlFixToFloat_S0110(uint32_t ulFix);
#ifdef __cplusplus
}
#endif

#endif /* __UTL_FIXfloat_H__ */

