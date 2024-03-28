#ifndef _TOOL_RKAIQ_API_SOCKET_H_
#define _TOOL_RKAIQ_API_SOCKET_H_
#include "tcp_client.h"

unsigned int MurMurHash(const void* key, int len);
int RkAiqSocketClientINETSend(int commandID, void* data, unsigned int dataSize);
int RkAiqSocketClientINETReceive(int commandID, void* data, unsigned int dataSize);

#endif // _TOOL_RKAIQ_API_SOCKET_H_
