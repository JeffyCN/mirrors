/*
 * Copyright 2021 Rockchip Electronics Co. LTD
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
 *
 */

#ifndef DUMP_CLIENT_H_
#define DUMP_CLIENT_H_

#include <sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <mutex>
#include "DumpLog.h"

const unsigned STOP_LISTENING_MSG_ID = 0;
const unsigned MESSAGE_BUFF_SIZE = 1024 * 1024* 1;
#define FFMIN(a, b) ((a) > (b) ? (b) : (a))

class DumpClient {
 public:
    DumpClient();
    virtual ~DumpClient();

 public:
    bool sockClientOpen(const char *uri);
    bool receiveDump(char* buf, uint32_t size);
    bool receiveDumpMessage(uint32_t &id, uint32_t &size);
    bool sendDump(uint32_t id, const char *data, uint32_t size);

 private:
    bool sendMessage(uint32_t id, const char *buf, uint32_t size);
    bool sendBuffer(const char *buf, uint32_t size);
    bool receiveMessage(int server_socket_fd, uint32_t &id, uint32_t &size);
    bool receiveBuffer(int server_socket_fd, char* buf, uint32_t size);
    bool init(const char *hostname);
    bool init(const char *hostname, int port);
    void urlSplit(char *proto, int protoSize,
                     char *authorization, int authorizationSize,
                     char *hostname, int hostnameSize,
                     int *portPtr, char *path, int pathSize, const char *url);
    int strlcpy(char *dst, const char *src, int size);

 private:
    std::string socket_filename;
    std::mutex mtx;
    int server_socket_fd = 0;
    char *message_buf = NULL;
};


#endif /* DUMP_CLIENT_H_ */
