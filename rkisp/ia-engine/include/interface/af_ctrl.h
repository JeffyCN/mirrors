#ifndef __AF_CTRL__
#define __AF_CTRL__

int getCtrl(unsigned int id);
int setCtrl(unsigned int id, unsigned int value);
RESULT rkisp_af_focus_set(void* handle, const uint32_t AbsStep);
RESULT rkisp_af_focus_get(void* handle, uint32_t* pAbsStep);
RESULT rkisp_init_moto_drive(void* handle);
RESULT rkisp_setup_moto_drive(void* handle, uint32_t* pMaxStep);
#endif
