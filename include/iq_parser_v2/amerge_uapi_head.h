

#ifndef __AMERGE_UAPI_HEAD_H__
#define __AMERGE_UAPI_HEAD_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct uapiMergeCurrCtlData_s
{
  // M4_NUMBER_DESC("Envlv", "f32", M4_RANGE(0,8), "0", M4_DIGIT(6))
  float Envlv;
  // M4_NUMBER_DESC("MoveCoef", "f32", M4_RANGE(0,8), "0", M4_DIGIT(6))
  float MoveCoef;
} uapiMergeCurrCtlData_t;

#ifdef __cplusplus
}
#endif

#endif  /*__AMERGE_UAPI_HEAD_H__*/
