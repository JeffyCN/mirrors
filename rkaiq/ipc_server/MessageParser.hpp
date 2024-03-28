/*
 * Copyright (c) 2019-2021 Rockchip Eletronics Co., Ltd.
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

#ifndef ___MESSAGE_PARSER_H__
#define ___MESSAGE_PARSER_H__

#include <stdlib.h>
#include <string.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <atomic>

#define RKAIQ_SOCKET_DATA_EXTRA_SIZE 28
#define RKAIQ_SOCKET_DATA_OFFSET 24
#define RKAIQ_SOCKET_OLD_HEADER_LEN 2
#define RKAIQ_SOCKET_DATA_HEADER_LEN 4
#define RKAIQ_RAW_STREAM_MAX_SIZE (1024 * 512)

static const uint8_t RKAIQ_SOCKET_OLD_HEADER[2] = {'R', 'K'};
static const uint8_t RKAIQ_SOCKET_DATA_HEADER[4] = {'R', 0xAA, 0xFF, 'K'};

typedef enum __aiq_ipc_cmd_id {
  AIQ_IPC_CMD_UNKNOWN = -1,
  AIQ_IPC_CMD_WRITE = 0,
  AIQ_IPC_CMD_READ = 1,
} aiq_ipc_cmd_id;

typedef struct __RkAiqSocketPacket_s {
  unsigned char magic[2] = {'R', 'K'};
  unsigned char packetSize[4];
  int commandID;
  int commandResult;
  unsigned int dataSize;
  char *data;
  unsigned int dataHash;
} __attribute__((packed)) RkAiqSocketPacket;

typedef struct __RkAiqSocketPacket {
  uint8_t magic[4] = {'R', 0xAA, 0xFF, 'K'};
  int32_t cmd_id;
  int32_t cmd_ret;
  uint32_t sequence;
  uint32_t payload_size;
  uint32_t packet_size;
  uint8_t *data;
  uint32_t data_hash; // different offset in data stream
} __attribute__((packed)) RkAiqSocketPacket_t;

typedef enum __MessageType {
  RKAIQ_MESSAGE_UNKNOWN = -1,
  RKAIQ_MESSAGE_OLD,
  RKAIQ_MESSAGE_NEW,
} MessageType;

typedef int (*MessageCallBack)(void *, void *, MessageType type);

namespace RkMSG {

class MessageParser {
public:
  explicit MessageParser(void *ptr);
  ~MessageParser();

public:
  int pushRawData(const uint8_t *data, size_t size);
  size_t remainData();
  int reset();
  int start();
  int stop();
  int setMsgCallBack(MessageCallBack cbf) {
    mCallBackFunc = cbf;
    return 0;
  };

  static int freePacket(void *packet, MessageType type);
  static unsigned int MurMurHash(const void *key, int len);

private:
  void *pri;
  std::vector<uint8_t> raw_stream;
  std::mutex proc_mutex;
  std::condition_variable proc_cond;
  std::shared_ptr<std::thread> proc_thread;
  std::atomic<bool> is_running;
  MessageCallBack mCallBackFunc{NULL};

  int notify_wakeup();
  void process();

  uint8_t *bit_stream_find(uint8_t *data, int size, const uint8_t *dst,
                           int len);

  void *clonePacket(void *from, MessageType type);
  RkAiqSocketPacket_t *findValidSection(uint8_t *buffer, int len,
                                        size_t *start_of, size_t *end_of);
  RkAiqSocketPacket *findValidSection2(uint8_t *buffer, int len,
                                       size_t *start_of, size_t *end_of);
};

} // namespace RkMSG

#endif /*___MESSAGE_PARSER_H__*/
