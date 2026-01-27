/*
 * Copyright (C) 2009 The Android Open Source Project
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

#include <cassert>
#include "DumpClient.h"

DumpClient::DumpClient() {
    message_buf = new char[MESSAGE_BUFF_SIZE];
}

DumpClient::~DumpClient() {
    if (server_socket_fd)
        ::close(server_socket_fd);
    unlink(socket_filename.c_str());
    delete[] message_buf;
    message_buf = NULL;
}

bool DumpClient::sockClientOpen(const char *uri) {
    int port;
    char hostname[1024], proto[1024], path[1024];
    urlSplit(proto, sizeof(proto), NULL, 0, hostname, sizeof(hostname),
        &port, path, sizeof(path), uri);
    DEBUG_MSG("proto:%s, hostname:%s port:%d \n", proto, hostname, port);
    if (!strcmp(proto, "tcp")) {
        init(hostname, port);
    } else if (!strcmp(proto, "domain")) {
        init(path);
    }
    return true;
}

bool DumpClient::init(const char *hostname, int port) {
    int ret;
    if (port <= 0 || port >= 65536) {
        DEBUG_MSG_ERROR("Port missing in uri \n");
        return false;
    }
    DEBUG_MSG("binding to host %s, port:%d \n", hostname, port);
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd == -1) {
        DEBUG_MSG_ERROR("%s: socket(AF_UNIX, SOCK_STREAM, 0) failed\n", __FUNCTION__);
        return false;
    }

    struct sockaddr_in remote;
    remote.sin_family = AF_INET;
    remote.sin_port = htons(port);
    remote.sin_addr.s_addr = inet_addr(hostname);

    if (connect(server_socket_fd,
            reinterpret_cast<sockaddr*>(&remote), sizeof(remote)) == -1) {
        DEBUG_MSG_ERROR("connect failed, reason: %s \n", strerror(errno));
        ::close(server_socket_fd);
        return false;
    }

    DEBUG_MSG("sock server initialize succeed \n");
    return true;
}

bool DumpClient::init(const char *hostname) {
    socket_filename = hostname;
    server_socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket_fd == -1) {
        DEBUG_MSG_ERROR("%s: socket(AF_UNIX, SOCK_STREAM, 0) failed \n", __FUNCTION__);
        return false;
    }

    DEBUG_MSG("%s: connecting to listener %s...\n", __FUNCTION__, socket_filename.c_str());
    sockaddr_un remote;
    remote.sun_family = AF_UNIX;
    snprintf(remote.sun_path, sizeof(remote.sun_path), "%s", socket_filename.c_str());
    unsigned length = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(server_socket_fd, reinterpret_cast<sockaddr*>(&remote), length) == -1) {
        DEBUG_MSG_ERROR("%s: connect failed \n", __FUNCTION__);
        ::close(server_socket_fd);
        server_socket_fd = 0;
        return false;
    }
    DEBUG_MSG("%s: done.\n", __FUNCTION__);
    return true;
}

bool DumpClient::sendDump(uint32_t id, const char *data, uint32_t size) {
    if (!server_socket_fd) {
        DEBUG_MSG_ERROR("%s: not initialized \n", __FUNCTION__);
        return false;
    }
    DEBUG_MSG("send: data:%s \n", data);
    DEBUG_MSG("%s: sending message: id:%d, size:%d \n", __FUNCTION__, id, size);
    if (!sendMessage(id, data, size)) {
        DEBUG_MSG_ERROR("%s: send_message failed\n", __FUNCTION__);
        if (errno == EPIPE) {
            DEBUG_MSG("%s: errno: EPIPE (connection broken)\n", __FUNCTION__);
        }
        return false;
    }
    return true;
}

bool DumpClient::sendMessage(uint32_t id, const char *buf, uint32_t size) {
    if (!sendBuffer(reinterpret_cast<char*>(&id), sizeof(id)))
        return false;

    if (!sendBuffer(reinterpret_cast<char*>(&size), sizeof(size)))
        return false;

    if (!sendBuffer(buf, size))
        return false;

    return true;
}

bool DumpClient::sendBuffer(const char *buf, uint32_t size) {
    if (!server_socket_fd) {
        DEBUG_MSG_ERROR("%s: not initialized \n", __FUNCTION__);
        return false;
    }

    auto bytes_left = size;
    int num_bytes_sent = 0;
    while ((bytes_left > 0)) {
        num_bytes_sent = ::send(server_socket_fd, buf, bytes_left, MSG_NOSIGNAL);
        if (num_bytes_sent > 0) {
            bytes_left -= num_bytes_sent;
            buf += num_bytes_sent;
        } else {
            DEBUG_MSG("%s: sent fail %d bytes \n", __FUNCTION__, num_bytes_sent);
            break;
        }
    }
    return (bytes_left == 0);
}

bool DumpClient::receiveMessage(int server_socket_fd, uint32_t &id, uint32_t &size ) {
    if (!receiveBuffer(server_socket_fd, reinterpret_cast<char*>(&id), sizeof(id)))
        return false;

    if (!receiveBuffer(server_socket_fd, reinterpret_cast<char*>(&size), sizeof(size)))
        return false;

    if (size > MESSAGE_BUFF_SIZE) {
        DEBUG_MSG_ERROR("Received message size exceeds reception buffer size! \n");
    }
    return true;
}

bool DumpClient::receiveBuffer(int server_socket_fd, char* buf, uint32_t size) {
    auto num_bytes_left = size;
    int num_bytes_received;
    while ((num_bytes_left > 0) && ((num_bytes_received = recv(server_socket_fd, buf, num_bytes_left, 0)) > 0)) {
        num_bytes_left -= num_bytes_received;
        buf += num_bytes_received;
    }
    return (num_bytes_left == 0);
}

bool DumpClient::receiveDumpMessage(uint32_t &id, uint32_t &size) {
    if (server_socket_fd == -1) {
        DEBUG_MSG_ERROR("%s: socket failed \n", __FUNCTION__);
        return false;
    }
    DEBUG_MSG("receive_dump_message id:%d, size:%d \n", id, size);
    bool result = receiveMessage(server_socket_fd, id, size);
    return result;
}

bool DumpClient::receiveDump(char* buf, uint32_t size) {
    if (server_socket_fd == -1) {
        DEBUG_MSG_ERROR("%s: socket failed \n", __FUNCTION__);
        return false;
    }
    DEBUG_MSG("receive_dump size:%d \n", size);
    bool result = receiveBuffer(server_socket_fd, buf, size);
    return result;
}

void DumpClient::urlSplit(char *proto, int protoSize,
                  char *authorization, int authorizationSize,
                  char *hostname, int hostnameSize,
                  int *portPtr, char *path, int pathSize, const char *url) {
    const char *p, *ls, *ls2, *at, *at2, *col, *brk;

    if (portPtr)
        *portPtr = -1;
    if (protoSize > 0)
        proto[0] = 0;
    if (authorizationSize > 0)
        authorization[0] = 0;
    if (hostnameSize > 0)
        hostname[0] = 0;
    if (pathSize > 0)
        path[0] = 0;

    /* parse protocol */
    if ((p = strchr(url, ':'))) {
        strlcpy(proto, url, FFMIN(protoSize, p + 1 - url));
        p++; /* skip ':' */
        if (*p == '/')
            p++;
        if (*p == '/')
            p++;
    } else {
        /* no protocol means plain filename */
        strlcpy(path, url, pathSize);
        return;
    }

    /* separate path from hostname */
    ls = strchr(p, '/');
    ls2 = strchr(p, '?');
    if (!ls)
        ls = ls2;
    else if (ls && ls2)
        ls = FFMIN(ls, ls2);
    if (ls)
        strlcpy(path, ls, pathSize);
    else
        ls = &p[strlen(p)];  // XXX

    /* the rest is hostname, use that to parse auth/port */
    if (ls != p) {
        /* authorization (user[:pass]@hostname) */
        at2 = p;
        while ((at = strchr(p, '@')) && at < ls) {
            strlcpy(authorization, at2,
                       FFMIN(authorizationSize, at + 1 - at2));
            p = at + 1; /* skip '@' */
        }

        if (*p == '[' && (brk = strchr(p, ']')) && brk < ls) {
            /* [host]:port */
            strlcpy(hostname, p + 1,
                       FFMIN(hostnameSize, brk - p));
            if (brk[1] == ':' && portPtr)
                *portPtr = atoi(brk + 2);
        } else if ((col = strchr(p, ':')) && col < ls) {
            strlcpy(hostname, p,
                       FFMIN(col + 1 - p, hostnameSize));
            if (portPtr)
                *portPtr = atoi(col + 1);
        } else {
            strlcpy(hostname, p, FFMIN(ls + 1 - p, hostnameSize));
        }
    }
}

int DumpClient::strlcpy(char *dst, const char *src, int size) {
    int len = 0;
    while (++len < size && *src)
        *dst++ = *src++;
    if (len <= size)
        *dst = 0;
    return len + strlen(src) - 1;
}
