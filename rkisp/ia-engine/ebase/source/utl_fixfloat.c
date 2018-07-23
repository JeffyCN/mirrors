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

#include <ebase/types.h>
#include <ebase/trace.h>
#include <ebase/builtins.h>
#include <ebase/dct_assert.h>
#include <utl_fixfloat.h>

// The general strategie of FrameFun is to use float during all calculations. Just right
// before writing to registers and directly after reading registers conversion to/from
// fixed point takes place. No arithmetics shall be done in fixed point.
// Consequently unsigned long, representing a register value, has been chosen as a
// container for all formats. Unsigned long is also used for signed fixed point formats.
// For better reading leading 1's of negative values, extending over the bit width of the
// register value, are suppressed. Variable contents and registers contents look alike.
//
// Several conversion routines are available. The postfix of their names show the size
// of the integer and the fractional part of the fixed point format. S0407 for example
// stands for a signed (two's complement) format with 4 bit integer and 7 bit fractional
// part.
// All conversion routines are doing a range check if the DCT_ASSERT macro is defined.
//
// Handling of unsigned formats without fractional part seems to be a bit overkill. But
// they are included here to keep the concept for all multipliers and offsets in the
// ISP chain (thereby providing the range check).
//
// Naive use of floating point arithmetic can lead to many problems. The creation of
// thoroughly robust floating point software is a complicated undertaking, and a good
// understanding of numerical analysis is essential. The fact that floating point can
// not mimic faithfully true arithmetic operations, leads to many surprising results.
// Encapsulating the arithmetics here ensures that maximum precision without errors is
// reached with tiny fully tested routines.


// An arithmetic shift is *usually* equivalent to multiplying the number by a positive or a negative
// integral power of the radix, except for the effect of any *rounding*.
// With two's complement binary number representations, arithmetic right shift is *not* equivalent
// to division by a power of 2. For negative numbers, the equivalence breaks down. The most trivial
// example of this is the arithmetic right shift of the number -1 (which is represented as all ones)
// in a two's complement representation.
// The (1999) ISO standard for the C programming language defines the C language's right shift operator
// in terms of divisions by powers of 2. Because of the aforementioned non-equivalence, the standard
// explicitly excludes from that definition the right shifts of signed numbers that have negative values.
// It doesn't specify the behaviour of the right shift operator in such circumstances, but instead requires
// each individual C compiler to specify the behaviour of shifting negative values right.
// *IN SHORT: DO NOT USE SHIFTING HERE!*


// Be careful when casting negative values. Usually casts within the same family of types
// change interpretation only, storage place and binary representation remain the same
// (for example (UINT8)((INT8)(-1)) gives usually 0xff).
// Casting between different families of types usually involves conversion of the binary
// representation. Also the storage place may change. The behaviour when casting negative
// values is not defined. The behaviour is dependent on the environment/compiler (for example
// (UINT8)(-1.0f) may give 0xff or 0x00).
// *IN SHORT: DO NOT CAST NEGATIVE VALUES TO UNSIGNED HERE!*


// Exactly correct for formats with fractional part would be:
// A range limit (MAX) equal to the value in the comments and a comparison "< MAX"
// instead "<= MAX" in the asserts below. But binary representations of real numbers
// are inaccurate anyway. So we take a slightly lower value to avoid potential problems.
//#define UTL_FIX_MAX_U0107          1.9921875f //exactly this would be < 4 - 0.5/256
#define UTL_FIX_MAX_U0107          1.996f //exactly this would be < 2 - 0.5/127
#define UTL_FIX_MIN_U0107          0.0f
#define UTL_FIX_PRECISION_U0107  128.0f
#define UTL_FIX_MASK_U0107       0x0ff


#define UTL_FIX_PRECISION_U0208  256.0f
#define UTL_FIX_MASK_U0208       0x3ff

#define UTL_FIX_MAX_U0408          15.998f //exactly this would be < 16 - 0.5/256
#define UTL_FIX_MIN_U0408          0.0f
#define UTL_FIX_PRECISION_U0408  256.0f
#define UTL_FIX_MASK_U0408       0xfff

#define UTL_FIX_MAX_U0800        255.499f //exactly this would be < 256 - 0.5
#define UTL_FIX_MIN_U0800          0.0f
#define UTL_FIX_MASK_U0800       0x0ff

#define UTL_FIX_MAX_U1000       1023.499f //exactly this would be < 1024 - 0.5
#define UTL_FIX_MIN_U1000          0.0f
#define UTL_FIX_MASK_U1000       0x3ff

#define UTL_FIX_MAX_U1200       4095.499f //exactly this would be < 4096 - 0.5
#define UTL_FIX_MIN_U1200          0.0f
#define UTL_FIX_MASK_U1200       0xfff

#define UTL_FIX_MAX_U0010       0.9995f //exactly this would be < 1 - 0.5/1024
#define UTL_FIX_MIN_U0010          0.0f
#define UTL_FIX_PRECISION_U0010  1024.0f
#define UTL_FIX_MASK_U0010       0x3ff

#define UTL_FIX_MAX_S0207          1.996f //exactly this would be < 2 - 0.5/128
#define UTL_FIX_MIN_S0207         -2.0f
#define UTL_FIX_PRECISION_S0207  128.0f
#define UTL_FIX_MASK_S0207       0x01ff
#define UTL_FIX_SIGN_S0207       0x0100

#define UTL_FIX_MAX_S0307          3.996f //exactly this would be < 4 - 0.5/128
#define UTL_FIX_MIN_S0307         -4.0f
#define UTL_FIX_PRECISION_S0307  128.0f
#define UTL_FIX_MASK_S0307       0x03ff
#define UTL_FIX_SIGN_S0307       0x0200

#define UTL_FIX_MAX_S0407          7.996f //exactly this would be < 8 - 0.5/128
#define UTL_FIX_MIN_S0407         -8.0f
#define UTL_FIX_PRECISION_S0407  128.0f
#define UTL_FIX_MASK_S0407       0x07ff
#define UTL_FIX_SIGN_S0407       0x0400

#define UTL_FIX_MAX_S0504          15.968f //exactly this would be < 16 - 0.5/16
#define UTL_FIX_MIN_S0504         -16.0f
#define UTL_FIX_PRECISION_S0504    16.0f
#define UTL_FIX_MASK_S0504        0x01ff
#define UTL_FIX_SIGN_S0504        0x0100

#define UTL_FIX_MAX_S0800         127.499f //exactly this would be < 16 - 0.5/16
#define UTL_FIX_MIN_S0800        -128.0f
#define UTL_FIX_PRECISION_S0800     0.0f
#define UTL_FIX_MASK_S0800        0x00ff
#define UTL_FIX_SIGN_S0800        0x0080

#define UTL_FIX_MAX_S0900         255.499f //exactly this would be < 16 - 0.5/16
#define UTL_FIX_MIN_S0900        -256.0f
#define UTL_FIX_PRECISION_S0900     0.0f
#define UTL_FIX_MASK_S0900        0x01ff
#define UTL_FIX_SIGN_S0900        0x0100

#define UTL_FIX_MAX_S0808        127.998f //exactly this would be < 128 - 0.5/256
#define UTL_FIX_MIN_S0808       -128.0f
#define UTL_FIX_PRECISION_S0808  256.0f
#define UTL_FIX_MASK_S0808       0xffff
#define UTL_FIX_SIGN_S0808       0x8000

#define UTL_FIX_MAX_S1200       2047.499f //exactly this would be < 2048 - 0.5
#define UTL_FIX_MIN_S1200      -2048.0f
#define UTL_FIX_MASK_S1200       0x0fff
#define UTL_FIX_SIGN_S1200       0x0800

#define UTL_FIX_MAX_S0109        0.999f //exactly this would be < 1 - 0.5/512
#define UTL_FIX_MIN_S0109       -1.0f
#define UTL_FIX_PRECISION_S0109  512.0f
#define UTL_FIX_MASK_S0109       0x03ff
#define UTL_FIX_SIGN_S0109       0x0200

#define UTL_FIX_MAX_S0408        7.998f //exactly this would be < 8 - 0.5/256
#define UTL_FIX_MIN_S0408       -8.0f
#define UTL_FIX_PRECISION_S0408  256.0f
#define UTL_FIX_MASK_S0408       0x0fff
#define UTL_FIX_SIGN_S0408       0x0800

#define UTL_FIX_MAX_S0108        0.998f //exactly this would be < 1 - 0.5/256
#define UTL_FIX_MIN_S0108       -1.0f
#define UTL_FIX_PRECISION_S0108  256.0f
#define UTL_FIX_MASK_S0108       0x01ff
#define UTL_FIX_SIGN_S0108       0x0100

#define UTL_FIX_MAX_S0110        0.9995f //exactly this would be < 1 - 0.5/1024
#define UTL_FIX_MIN_S0110       -1.0f
#define UTL_FIX_PRECISION_S0110  1024.0f
#define UTL_FIX_MASK_S0110       0x07ff
#define UTL_FIX_SIGN_S0110       0x0400

#define UTL_FIX_MAX_U0402   15.875f //exactly this would be <16 - 0.5/4
#define UTL_FIX_MIN_U0402 0.0f
#define UTL_FIX_PRECISION_U0402  4.0f
#define UTL_FIX_MASK_U0402       0x3f

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_U0402 \n
 *  \RETURNVALUE unsigned fixed point value in uint32_t container \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to unsigned fixed point values with \n
 *               4 bit integer and 2 bit fractional part to program the \n
 *               marvin registers. \n
 */
/*****************************************************************************/

// unsigned fixed point 1 bit integer / 7 bit fractional part
// 0x03f = 0x3f/4= 15.75
// 0x004 = 1
// 0x000 = 0

uint32_t UtlFloatToFix_U0402(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_U0402);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_U0402);

  fFloat *= UTL_FIX_PRECISION_U0402;

  // round
  // no handling of negative values required
  ulFix = (uint32_t)(fFloat + 0.5f);

  //no masking of upper bits required

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_U0107 \n
 *  \RETURNVALUE unsigned fixed point value in uint32_t container \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to unsigned fixed point values with \n
 *               1 bit integer and 7 bit fractional part to program the \n
 *               marvin registers. \n
 */
/*****************************************************************************/

// unsigned fixed point 1 bit integer / 7 bit fractional part
// 0x0ff = 0xFF/0x80 = 1.9921875
// 0x000 = 1
// 0x000 = 0

uint32_t UtlFloatToFix_U0107(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_U0107);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_U0107);

  fFloat *= UTL_FIX_PRECISION_U0107;

  // round
  // no handling of negative values required
  ulFix = (uint32_t)(fFloat + 0.5f);

  //no masking of upper bits required

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_U0107 \n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  unsigned fixed point value in uint32_t container \n
 *  \DESCRIPTION Converts unsigned fixed point values with 1 bit integer and \n
 *               7 bit fractional part (marvin register) to float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_U0107(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0x03ff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_U0107) == 0);

  // precision is not cut away here, so no rounding is necessary
  // no handling of negative values required
  fFloat = (float)ulFix;

  fFloat /= UTL_FIX_PRECISION_U0107;

  return fFloat;
}



/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_U0408 \n
 *  \RETURNVALUE unsigned fixed point value in UINT32 container \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to unsigned fixed point values with \n
 *               4 bit integer and 8 bit fractional part to program the \n
 *               marvin registers. \n
 */
/*****************************************************************************/

// unsigned fixed point 4 bit integer / 8 bit fractional part
// 0x3ff = 4095/256 = 15.99609375
// 0x100 = 1
// 0x000 = 0

uint32_t UtlFloatToFix_U0408(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_U0408);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_U0408);

  fFloat *= UTL_FIX_PRECISION_U0408;

  // round
  // no handling of negative values required
  ulFix = (uint32_t)(fFloat + 0.5f);

  //no masking of upper bits required

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_U0408 \n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  unsigned fixed point value in uint32_t container \n
 *  \DESCRIPTION Converts unsigned fixed point values with 4 bit integer and \n
 *               8 bit fractional part (marvin register) to float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_U0408(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0x0fff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_U0408) == 0);

  // precision is not cut away here, so no rounding is necessary
  // no handling of negative values required
  fFloat = (float)ulFix;

  fFloat /= UTL_FIX_PRECISION_U0408;

  return fFloat;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_U0208 \n
 *  \RETURNVALUE unsigned fixed point value in uint32_t container \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to unsigned fixed point values with \n
 *               2 bit integer and 8 bit fractional part to program the \n
 *               marvin registers. \n
 */
/*****************************************************************************/

// unsigned fixed point 2 bit integer / 8 bit fractional part
// 0x3ff = 1023/256 = 3.99609375
// 0x100 = 1
// 0x000 = 0

uint32_t UtlFloatToFix_U0208(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_U0208);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_U0208);

  fFloat *= UTL_FIX_PRECISION_U0208;

  // round
  // no handling of negative values required
  ulFix = (uint32_t)(fFloat + 0.5f);

  //no masking of upper bits required

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_U0208 \n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  unsigned fixed point value in uint32_t container \n
 *  \DESCRIPTION Converts unsigned fixed point values with 2 bit integer and \n
 *               8 bit fractional part (marvin register) to float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_U0208(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0x03ff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_U0208) == 0);

  // precision is not cut away here, so no rounding is necessary
  // no handling of negative values required
  fFloat = (float)ulFix;

  fFloat /= UTL_FIX_PRECISION_U0208;

  return fFloat;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_U1000 \n
 *  \RETURNVALUE unsigned integer value in uint32_t container \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to unsigned integer values with \n
 *               10 bit integer and no fractional part to program the \n
 *               marvin registers. \n
 */
/*****************************************************************************/

// unsigned 10 bit integer, no fractional part
// 0x3ff = 1023
// 0x001 = 1
// 0x000 = 0

uint32_t UtlFloatToFix_U1000(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_U1000);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_U1000);

  // precision is 1, thus no multiplication is required

  // round
  // no handling of negative values required
  ulFix = (uint32_t)(fFloat + 0.5f);

  //no masking of upper bits required

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_U1000 \n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  unsigned integer value in uint32_t container \n
 *  \DESCRIPTION Converts unsigned integer values with 10 bit integer and \n
 *               no fractional part (marvin register) to float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_U1000(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0x03ff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_U1000) == 0);

  // precision is not cut away here, so no rounding is necessary
  // no handling of negative values required
  fFloat = (float)ulFix;

  // precision is 1, thus no division is required.

  return fFloat;
}
/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_U0800 \n
 *  \RETURNVALUE unsigned integer value in uint32_t container \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to unsigned integer values with \n
 *               8 bit integer and no fractional part to program the \n
 *               marvin registers. \n
 */
/*****************************************************************************/

// unsigned 8 bit integer, no fractional part
// 0x0ff = 255
// 0x001 = 1
// 0x000 = 0

uint32_t UtlFloatToFix_U0800(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_U0800);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_U0800);

  // precision is 1, thus no multiplication is required

  // round
  // no handling of negative values required
  ulFix = (uint32_t)(fFloat + 0.5f);

  //no masking of upper bits required

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_U0800 \n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  unsigned integer value in uint32_t container \n
 *  \DESCRIPTION Converts unsigned integer values with 10 bit integer and \n
 *               no fractional part (marvin register) to float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_U0800(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0x00ff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_U0800) == 0);

  // precision is not cut away here, so no rounding is necessary
  // no handling of negative values required
  fFloat = (float)ulFix;

  // precision is 1, thus no division is required.

  return fFloat;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_U1200 \n
 *  \RETURNVALUE unsigned integer value in uint32_t container \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to unsigned integer values with \n
 *               12 bit integer and no fractional part to program the \n
 *               marvin registers. \n
 */
/*****************************************************************************/

// unsigned 12 bit integer, no fractional part
// 0xfff = 4095
// 0x001 = 1
// 0x000 = 0

uint32_t UtlFloatToFix_U1200(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_U1200);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_U1200);

  // precision is 1, thus no multiplication is required

  // round
  // no handling of negative values required
  ulFix = (uint32_t)(fFloat + 0.5f);

  //no masking of upper bits required

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_U1200 \n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  unsigned integer value in uint32_t container \n
 *  \DESCRIPTION Converts unsigned integer values with 12 bit integer and \n
 *               no fractional part (marvin register) to float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_U1200(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0x0fff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_U1200) == 0);

  // precision is not cut away here, so no rounding is necessary
  // no handling of negative values required
  fFloat = (float)ulFix;

  // precision is 1, thus no division is required.

  return fFloat;
}


/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_U0010 \n
 *  \RETURNVALUE unsigned fixed point value in uint32_t container \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to unsigned fixed point values with \n
 *               0 bit integer and 10 bit fractional part to program the \n
 *               marvin registers. \n
 */
/*****************************************************************************/

// unsigned fixed point 0 bit integer / 10 bit fractional part
// 0x3ff = 1023/1024 = 0.9990234375
// 0x000 = 0

uint32_t UtlFloatToFix_U0010(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_U0010);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_U0010);

  fFloat *= UTL_FIX_PRECISION_U0010;

  // round
  // no handling of negative values required
  ulFix = (uint32_t)(fFloat + 0.5f);

  //no masking of upper bits required

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_U0010 \n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  unsigned fixed point value in uint32_t container \n
 *  \DESCRIPTION Converts unsigned fixed point values with 0 bit integer and \n
 *               10 bit fractional part (marvin register) to float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_U0010(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0x03ff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_U0010) == 0);

  // precision is not cut away here, so no rounding is necessary
  // no handling of negative values required
  fFloat = (float)ulFix;

  fFloat /= UTL_FIX_PRECISION_U0010;

  return fFloat;
}


/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_S0207 \n
 *  \RETURNVALUE signed fixed point value in uint32_t container, leading 1's \n
 *               in the upper bits, not used by the format, are suppressed \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to signed fixed point values (two's
 *               complement) with 2 bit integer and 7 bit fractional part to
 *               program the marvin registers. \n
 */
/*****************************************************************************/

// signed (two's complement) fixed point 2 bit integer / 7 bit fractional part
// 0x0ff = 255/128 = 1.9921875
// 0x080 = 1
// 0x000 = 0
// 0x1ff = -1/128 = -0.0078125
// 0x100 = -256/128 = -2

uint32_t UtlFloatToFix_S0207(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_S0207);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_S0207);

  fFloat *= UTL_FIX_PRECISION_S0207;

  // round, two's complement if negative
  if (fFloat > 0.0f) {
    ulFix = (uint32_t)(fFloat + 0.5f);
  } else {
    fFloat = -fFloat;
    ulFix  = (uint32_t)(fFloat + 0.5f);
    ulFix  = ~ulFix;
    ulFix++;
  }

  // set upper (unused) bits to 0
  ulFix &= UTL_FIX_MASK_S0207;

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_S0207 \n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  signed fixed point value in uint32_t container, leading \n
 *               1's in the upper bits, not used by the format, must be 0 \n
 *  \DESCRIPTION Converts signed fixed point values (two's complement) with \n
 *               2 bit integer and 7 bit fractional part (marvin register) to \n
 *               float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_S0207(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0x01ff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_S0207) == 0);

  // sign extension and two's complement if negative
  // (precision is not cut away here, so no rounding is necessary)
  if ((ulFix & UTL_FIX_SIGN_S0207) == 0) {
    fFloat = (float)ulFix;
  } else {
    ulFix |= ~UTL_FIX_MASK_S0207;
    ulFix--;
    ulFix = ~ulFix;
    fFloat = (float)ulFix;
    fFloat = -fFloat;
  }

  fFloat /= UTL_FIX_PRECISION_S0207;

  return fFloat;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_S0307 \n
 *  \RETURNVALUE signed fixed point value in uint32_t container, leading 1's \n
 *               in the upper bits, not used by the format, are suppressed \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to signed fixed point values (two's
 *               complement) with 3 bit integer and 7 bit fractional part to
 *               program the marvin registers. \n
 */
/*****************************************************************************/

// signed (two's complement) fixed point 3 bit integer / 7 bit fractional part
// 0x1ff = 511/128 = 3.9921875
// 0x080 = 1
// 0x000 = 0
// 0x3ff = -1/128 = -0.0078125
// 0x200 = -512/128 = -4

uint32_t UtlFloatToFix_S0307(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_S0307);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_S0307);

  fFloat *= UTL_FIX_PRECISION_S0307;

  // round, two's complement if negative
  if (fFloat > 0.0f) {
    ulFix = (uint32_t)(fFloat + 0.5f);
  } else {
    fFloat = -fFloat;
    ulFix  = (uint32_t)(fFloat + 0.5f);
    ulFix  = ~ulFix;
    ulFix++;
  }

  // set upper (unused) bits to 0
  ulFix &= UTL_FIX_MASK_S0307;

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_S0307 \n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  signed fixed point value in uint32_t container, leading \n
 *               1's in the upper bits, not used by the format, must be 0 \n
 *  \DESCRIPTION Converts signed fixed point values (two's complement) with \n
 *               3 bit integer and 7 bit fractional part (marvin register) to \n
 *               float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_S0307(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0x03ff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_S0307) == 0);

  // sign extension and two's complement if negative
  // (precision is not cut away here, so no rounding is necessary)
  if ((ulFix & UTL_FIX_SIGN_S0307) == 0) {
    fFloat = (float)ulFix;
  } else {
    ulFix |= ~UTL_FIX_MASK_S0307;
    ulFix--;
    ulFix = ~ulFix;
    fFloat = (float)ulFix;
    fFloat = -fFloat;
  }

  fFloat /= UTL_FIX_PRECISION_S0307;

  return fFloat;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_S0407 \n
 *  \RETURNVALUE signed fixed point value in uint32_t container, leading 1's \n
 *               in the upper bits, not used by the format, are suppressed \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to signed fixed point values (two's
 *               complement) with 4 bit integer and 7 bit fractional part to
 *               program the marvin registers. \n
 */
/*****************************************************************************/

// signed (two's complement) fixed point 4 bit integer / 7 bit fractional part
// 0x3ff = 1023/128 = 7.9921875
// 0x080 = 1
// 0x000 = 0
// 0x7ff = -1/128 = -0.0078125
// 0x400 = -1024/128 = -8

uint32_t UtlFloatToFix_S0407(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_S0407);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_S0407);

  fFloat *= UTL_FIX_PRECISION_S0407;

  // round, two's complement if negative
  if (fFloat > 0.0f) {
    ulFix = (uint32_t)(fFloat + 0.5f);
  } else {
    fFloat = -fFloat;
    ulFix  = (uint32_t)(fFloat + 0.5f);
    ulFix  = ~ulFix;
    ulFix++;
  }

  // set upper (unused) bits to 0
  ulFix &= UTL_FIX_MASK_S0407;

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_S0407 \n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  signed fixed point value in uint32_t container, leading \n
 *               1's in the upper bits, not used by the format, must be 0 \n
 *  \DESCRIPTION Converts signed fixed point values (two's complement) with \n
 *               4 bit integer and 7 bit fractional part (marvin register) to \n
 *               float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_S0407(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0x07ff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_S0407) == 0);

  // sign extension and two's complement if negative
  // (precision is not cut away here, so no rounding is necessary)
  if ((ulFix & UTL_FIX_SIGN_S0407) == 0) {
    fFloat = (float)ulFix;
  } else {
    ulFix |= ~UTL_FIX_MASK_S0407;
    ulFix--;
    ulFix = ~ulFix;
    fFloat = (float)ulFix;
    fFloat = -fFloat;
  }

  fFloat /= UTL_FIX_PRECISION_S0407;

  return fFloat;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_S0504\n
 *  \RETURNVALUE signed fixed point value in uint32_t container, leading 1's \n
 *               in the upper bits, not used by the format, are suppressed \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to signed fixed point values (two's
 *               complement) with 4 bit integer and 7 bit fractional part to
 *               program the marvin registers. \n
 */
/*****************************************************************************/

// signed (two's complement) fixed point 4 bit integer / 7 bit fractional part
// 0x3ff = 1023/128 = 7.9921875
// 0x080 = 1
// 0x000 = 0
// 0x7ff = -1/128 = -0.0078125
// 0x400 = -1024/128 = -8

uint32_t UtlFloatToFix_S0504(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_S0504);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_S0504);

  fFloat *= UTL_FIX_PRECISION_S0504;

  // round, two's complement if negative
  if (fFloat > 0.0f) {
    ulFix = (uint32_t)(fFloat + 0.5f);
  } else {
    fFloat = -fFloat;
    ulFix  = (uint32_t)(fFloat + 0.5f);
    ulFix  = ~ulFix;
    ulFix++;
  }

  // set upper (unused) bits to 0
  ulFix &= UTL_FIX_MASK_S0504;

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_S0504\n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  signed fixed point value in uint32_t container, leading \n
 *               1's in the upper bits, not used by the format, must be 0 \n
 *  \DESCRIPTION Converts signed fixed point values (two's complement) with \n
 *               4 bit integer and 7 bit fractional part (marvin register) to \n
 *               float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_S0504(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0x07ff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_S0504) == 0);

  // sign extension and two's complement if negative
  // (precision is not cut away here, so no rounding is necessary)
  if ((ulFix & UTL_FIX_SIGN_S0504) == 0) {
    fFloat = (float)ulFix;
  } else {
    ulFix |= ~UTL_FIX_MASK_S0504;
    ulFix--;
    ulFix = ~ulFix;
    fFloat = (float)ulFix;
    fFloat = -fFloat;
  }

  fFloat /= UTL_FIX_PRECISION_S0504;

  return fFloat;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_S0808 \n
 *  \RETURNVALUE signed fixed point value in uint32_t container, leading 1's \n
 *               in the upper bits, not used by the format, are suppressed \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to signed fixed point values (two's
 *               complement) with 8 bit integer and 8 bit fractional part to
 *               program the marvin registers. \n
 */
/*****************************************************************************/

// signed (two's complement) fixed point 8 bit integer / 8 bit fractional part
// 0x7fff = 32767/256 = 127.99609375
// 0x0100 = 1
// 0x0000 = 0
// 0xffff = -1/256 = -0.00390625
// 0x8000 = -32768/256 = -128

uint32_t UtlFloatToFix_S0808(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_S0808);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_S0808);

  fFloat *= UTL_FIX_PRECISION_S0808;

  // round, two's complement if negative
  if (fFloat > 0.0f) {
    ulFix = (uint32_t)(fFloat + 0.5f);
  } else {
    fFloat = -fFloat;
    ulFix  = (uint32_t)(fFloat + 0.5f);
    ulFix  = ~ulFix;
    ulFix++;
  }

  // set upper (unused) bits to 0
  ulFix &= UTL_FIX_MASK_S0808;

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_S0808 \n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  signed fixed point value in uint32_t container, leading \n
 *               1's in the upper bits, not used by the format, must be 0 \n
 *  \DESCRIPTION Converts signed fixed point values (two's complement) with \n
 *               8 bit integer and 8 bit fractional part (marvin register) to \n
 *               float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_S0808(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0xffff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_S0808) == 0);

  // sign extension and two's complement if negative
  // (precision is not cut away here, so no rounding is necessary)
  if ((ulFix & UTL_FIX_SIGN_S0808) == 0) {
    fFloat = (float)ulFix;
  } else {
    ulFix |= ~UTL_FIX_MASK_S0808;
    ulFix--;
    ulFix = ~ulFix;
    fFloat = (float)ulFix;
    fFloat = -fFloat;
  }

  fFloat /= UTL_FIX_PRECISION_S0808;

  return fFloat;
}


/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_S0800\n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  signed integer value in uint32_t container, leading \n
 *               1's in the upper bits, not used by the format, must be 0 \n
 *  \DESCRIPTION Converts signed integer values (two's complement) with \n
 *               12 bit integer and no fractional part (marvin register) to \n
 *               float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_S0800(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0x0fff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_S0800) == 0);

  // sign extension and two's complement if negative
  // (precision is not cut away here, so no rounding is necessary)
  if ((ulFix & UTL_FIX_SIGN_S0800) == 0) {
    fFloat = (float)ulFix;
  } else {
    ulFix |= ~UTL_FIX_MASK_S0800;
    ulFix--;
    ulFix = ~ulFix;
    fFloat = (float)ulFix;
    fFloat = -fFloat;
  }

  // precision is 1, thus no division is required.

  return fFloat;
}



/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_S0800\n
 *  \RETURNVALUE signed integer value in uint32_t container, leading 1's \n
 *               in the upper bits, not used by the format, are suppressed \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to signed integer values (two's
 *               complement) with 12 bit integer and no fractional part to
 *               program the marvin registers. \n
 */
/*****************************************************************************/

// signed (two's complement) 12 bit integer, no fractional part
// 0x7ff = 2047
// 0x001 = 1
// 0x000 = 0
// 0xfff = -1
// 0x800 = -2048

uint32_t UtlFloatToFix_S0800(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_S0800);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_S0800);

  // precision is 1, thus no multiplication is required

  // round, two's complement if negative
  if (fFloat > 0.0f) {
    ulFix = (uint32_t)(fFloat + 0.5f);
  } else {
    fFloat = -fFloat;
    ulFix  = (uint32_t)(fFloat + 0.5f);
    ulFix  = ~ulFix;
    ulFix++;
  }

  // set upper (unused) bits to 0
  ulFix &= UTL_FIX_MASK_S0800;

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_S0900\n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  signed integer value in uint32_t container, leading \n
 *               1's in the upper bits, not used by the format, must be 0 \n
 *  \DESCRIPTION Converts signed integer values (two's complement) with \n
 *               12 bit integer and no fractional part (marvin register) to \n
 *               float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_S0900(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0x0fff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_S0900) == 0);

  // sign extension and two's complement if negative
  // (precision is not cut away here, so no rounding is necessary)
  if ((ulFix & UTL_FIX_SIGN_S0900) == 0) {
    fFloat = (float)ulFix;
  } else {
    ulFix |= ~UTL_FIX_MASK_S0900;
    ulFix--;
    ulFix = ~ulFix;
    fFloat = (float)ulFix;
    fFloat = -fFloat;
  }

  // precision is 1, thus no division is required.

  return fFloat;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_S0900\n
 *  \RETURNVALUE signed integer value in uint32_t container, leading 1's \n
 *               in the upper bits, not used by the format, are suppressed \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to signed integer values (two's
 *               complement) with 12 bit integer and no fractional part to
 *               program the marvin registers. \n
 */
/*****************************************************************************/

// signed (two's complement) 12 bit integer, no fractional part
// 0x7ff = 2047
// 0x001 = 1
// 0x000 = 0
// 0xfff = -1
// 0x800 = -2048

uint32_t UtlFloatToFix_S0900(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_S0900);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_S0900);

  // precision is 1, thus no multiplication is required

  // round, two's complement if negative
  if (fFloat > 0.0f) {
    ulFix = (uint32_t)(fFloat + 0.5f);
  } else {
    fFloat = -fFloat;
    ulFix  = (uint32_t)(fFloat + 0.5f);
    ulFix  = ~ulFix;
    ulFix++;
  }

  // set upper (unused) bits to 0
  ulFix &= UTL_FIX_MASK_S0900;

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_S1200 \n
 *  \RETURNVALUE signed integer value in uint32_t container, leading 1's \n
 *               in the upper bits, not used by the format, are suppressed \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to signed integer values (two's
 *               complement) with 12 bit integer and no fractional part to
 *               program the marvin registers. \n
 */
/*****************************************************************************/

// signed (two's complement) 12 bit integer, no fractional part
// 0x7ff = 2047
// 0x001 = 1
// 0x000 = 0
// 0xfff = -1
// 0x800 = -2048

uint32_t UtlFloatToFix_S1200(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_S1200);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_S1200);

  // precision is 1, thus no multiplication is required

  // round, two's complement if negative
  if (fFloat > 0.0f) {
    ulFix = (uint32_t)(fFloat + 0.5f);
  } else {
    fFloat = -fFloat;
    ulFix  = (uint32_t)(fFloat + 0.5f);
    ulFix  = ~ulFix;
    ulFix++;
  }

  // set upper (unused) bits to 0
  ulFix &= UTL_FIX_MASK_S1200;

  return ulFix;
}



/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_S1200 \n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  signed integer value in uint32_t container, leading \n
 *               1's in the upper bits, not used by the format, must be 0 \n
 *  \DESCRIPTION Converts signed integer values (two's complement) with \n
 *               12 bit integer and no fractional part (marvin register) to \n
 *               float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_S1200(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0x0fff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_S1200) == 0);

  // sign extension and two's complement if negative
  // (precision is not cut away here, so no rounding is necessary)
  if ((ulFix & UTL_FIX_SIGN_S1200) == 0) {
    fFloat = (float)ulFix;
  } else {
    ulFix |= ~UTL_FIX_MASK_S1200;
    ulFix--;
    ulFix = ~ulFix;
    fFloat = (float)ulFix;
    fFloat = -fFloat;
  }

  // precision is 1, thus no division is required.

  return fFloat;
}


/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_S0109\n
 *  \RETURNVALUE signed fixed point value in uint32_t container, leading 1's \n
 *               in the upper bits, not used by the format, are suppressed \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to signed fixed point values (two's
 *               complement) with 1 bit integer and 9 bit fractional part to
 *               program the marvin registers. \n
 */
/*****************************************************************************/

// signed (two's complement) fixed point 1 bit integer / 9 bit fractional part
// 0x01ff = 511/512 = 0.998046875
// 0x0000 = 0
// 0x03ff = -1/512 = -0.001953125
// 0x0200 = -1

uint32_t UtlFloatToFix_S0109(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_S0109);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_S0109);

  fFloat *= UTL_FIX_PRECISION_S0109;

  // round, two's complement if negative
  if (fFloat > 0.0f) {
    ulFix = (uint32_t)(fFloat + 0.5f);
  } else {
    fFloat = -fFloat;
    ulFix  = (uint32_t)(fFloat + 0.5f);
    ulFix  = ~ulFix;
    ulFix++;
  }

  // set upper (unused) bits to 0
  ulFix &= UTL_FIX_MASK_S0109;

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_S0109\n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  signed fixed point value in uint32_t container, leading \n
 *               1's in the upper bits, not used by the format, must be 0 \n
 *  \DESCRIPTION Converts signed fixed point values (two's complement) with \n
 *               1 bit integer and 9 bit fractional part (marvin register) to \n
 *               float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_S0109(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0x03ff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_S0109) == 0);

  // sign extension and two's complement if negative
  // (precision is not cut away here, so no rounding is necessary)
  if ((ulFix & UTL_FIX_SIGN_S0109) == 0) {
    fFloat = (float)ulFix;
  } else {
    ulFix |= ~UTL_FIX_MASK_S0109;
    ulFix--;
    ulFix = ~ulFix;
    fFloat = (float)ulFix;
    fFloat = -fFloat;
  }

  fFloat /= UTL_FIX_PRECISION_S0109;

  return fFloat;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_S0408\n
 *  \RETURNVALUE signed fixed point value in uint32_t container, leading 1's \n
 *               in the upper bits, not used by the format, are suppressed \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to signed fixed point values (two's
 *               complement) with 4 bit integer and 8 bit fractional part to
 *               program the marvin registers. \n
 */
/*****************************************************************************/

// signed (two's complement) fixed point 4 bit integer / 8 bit fractional part
// 0x07ff = 2047/256 = 7.99609375
// 0x0100 = 1
// 0x0000 = 0
// 0x0fff = -1/256 = -0.00390625
// 0x0800 = -8

uint32_t UtlFloatToFix_S0408(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_S0408);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_S0408);

  fFloat *= UTL_FIX_PRECISION_S0408;

  // round, two's complement if negative
  if (fFloat > 0.0f) {
    ulFix = (uint32_t)(fFloat + 0.5f);
  } else {
    fFloat = -fFloat;
    ulFix  = (uint32_t)(fFloat + 0.5f);
    ulFix  = ~ulFix;
    ulFix++;
  }

  // set upper (unused) bits to 0
  ulFix &= UTL_FIX_MASK_S0408;

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_S0408\n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  signed fixed point value in uint32_t container, leading \n
 *               1's in the upper bits, not used by the format, must be 0 \n
 *  \DESCRIPTION Converts signed fixed point values (two's complement) with \n
 *               4 bit integer and 8 bit fractional part (marvin register) to \n
 *               float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_S0408(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0x0fff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_S0408) == 0);

  // sign extension and two's complement if negative
  // (precision is not cut away here, so no rounding is necessary)
  if ((ulFix & UTL_FIX_SIGN_S0408) == 0) {
    fFloat = (float)ulFix;
  } else {
    ulFix |= ~UTL_FIX_MASK_S0408;
    ulFix--;
    ulFix = ~ulFix;
    fFloat = (float)ulFix;
    fFloat = -fFloat;
  }

  fFloat /= UTL_FIX_PRECISION_S0408;

  return fFloat;
}


/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_S0108\n
 *  \RETURNVALUE signed fixed point value in uint32_t container, leading 1's \n
 *               in the upper bits, not used by the format, are suppressed \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to signed fixed point values (two's
 *               complement) with 1 bit integer and 8 bit fractional part to
 *               program the marvin registers. \n
 */
/*****************************************************************************/

// signed (two's complement) fixed point 1 bit integer / 8 bit fractional part
// 0x00ff = 255/256 = 0.99609375
// 0x0000 = 0
// 0x01ff = -1/256 = -0.00390625
// 0x0100 = -1

uint32_t UtlFloatToFix_S0108(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_S0108);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_S0108);

  fFloat *= UTL_FIX_PRECISION_S0108;

  // round, two's complement if negative
  if (fFloat > 0.0f) {
    ulFix = (uint32_t)(fFloat + 0.5f);
  } else {
    fFloat = -fFloat;
    ulFix  = (uint32_t)(fFloat + 0.5f);
    ulFix  = ~ulFix;
    ulFix++;
  }

  // set upper (unused) bits to 0
  ulFix &= UTL_FIX_MASK_S0108;

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_S0108\n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  signed fixed point value in uint32_t container, leading \n
 *               1's in the upper bits, not used by the format, must be 0 \n
 *  \DESCRIPTION Converts signed fixed point values (two's complement) with \n
 *               1 bit integer and 8 bit fractional part (marvin register) to \n
 *               float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_S0108(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0x0fff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_S0108) == 0);

  // sign extension and two's complement if negative
  // (precision is not cut away here, so no rounding is necessary)
  if ((ulFix & UTL_FIX_SIGN_S0108) == 0) {
    fFloat = (float)ulFix;
  } else {
    ulFix |= ~UTL_FIX_MASK_S0108;
    ulFix--;
    ulFix = ~ulFix;
    fFloat = (float)ulFix;
    fFloat = -fFloat;
  }

  fFloat /= UTL_FIX_PRECISION_S0108;

  return fFloat;
}


/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFloatToFix_S0110\n
 *  \RETURNVALUE signed fixed point value in uint32_t container, leading 1's \n
 *               in the upper bits, not used by the format, are suppressed \n
 *  \PARAMETERS  float value \n
 *  \DESCRIPTION Converts float values to signed fixed point values (two's
 *               complement) with 1 bit integer and 10 bit fractional part to
 *               program the marvin registers. \n
 */
/*****************************************************************************/

// signed (two's complement) fixed point 1 bit integer / 10 bit fractional part
// 0x03ff = 1023/1024 = 0.9990234375
// 0x0000 = 0
// 0x07ff = -1/1024 = -0.0009765625
// 0x0400 = -1


uint32_t UtlFloatToFix_S0110(float fFloat) {
  uint32_t ulFix = 0;

  DCT_ASSERT(fFloat <= UTL_FIX_MAX_S0110);
  DCT_ASSERT(fFloat >= UTL_FIX_MIN_S0110);

  fFloat *= UTL_FIX_PRECISION_S0110;

  // round, two's complement if negative
  if (fFloat > 0.0f) {
    ulFix = (uint32_t)(fFloat + 0.5f);
  } else {
    fFloat = -fFloat;
    ulFix  = (uint32_t)(fFloat + 0.5f);
    ulFix  = ~ulFix;
    ulFix++;
  }

  // set upper (unused) bits to 0
  ulFix &= UTL_FIX_MASK_S0110;

  return ulFix;
}

/*****************************************************************************/
/*!
 *  \FUNCTION    UtlFixToFloat_S0110\n
 *  \RETURNVALUE float value \n
 *  \PARAMETERS  signed fixed point value in uint32_t container, leading \n
 *               1's in the upper bits, not used by the format, must be 0 \n
 *  \DESCRIPTION Converts signed fixed point values (two's complement) with \n
 *               1 bit integer and 10 bit fractional part (marvin register) to \n
 *               float values. \n
 */
/*****************************************************************************/
float UtlFixToFloat_S0110(uint32_t ulFix) {
  float fFloat = 0;

  // any value from 0x0000 to 0x0fff will do as input
  DCT_ASSERT((ulFix & ~UTL_FIX_MASK_S0110) == 0);

  // sign extension and two's complement if negative
  // (precision is not cut away here, so no rounding is necessary)
  if ((ulFix & UTL_FIX_SIGN_S0110) == 0) {
    fFloat = (float)ulFix;
  } else {
    ulFix |= ~UTL_FIX_MASK_S0110;
    ulFix--;
    ulFix = ~ulFix;
    fFloat = (float)ulFix;
    fFloat = -fFloat;
  }

  fFloat /= UTL_FIX_PRECISION_S0110;

  return fFloat;
}

