#ifndef __RKISP1_INTERFACE_H__
#define __RKISP1_INTERFACE_H__

int
rkisp_start(void* &engine, int vidFd, const char* tuningFile);

int
rkisp_stop(void* &engine);

#endif
