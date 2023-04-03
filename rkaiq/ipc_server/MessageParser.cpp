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

#include "MessageParser.hpp"

#define BUFFER_MAX_SIZE (1024 * 512)

#ifndef LOGE
#define LOGE printf
#endif

// #define AIQ_MSGPARSER_CHECK_HASH

#define CALC_32BIT_LITTLE(array)                                               \
  ((array[0] & 0xff) | ((array[1] & 0xff) << 8) | ((array[2] & 0xff) << 16) |  \
   ((array[3] & 0xff) << 24))

namespace RkMSG {

MessageParser::MessageParser(void* ptr) : pri(ptr), is_running(false) {
}

int MessageParser::stop() {
  is_running = false;
  notify_wakeup();
  return 0;
}

MessageParser::~MessageParser() {
  this->stop();
  if (this->proc_thread && this->proc_thread->joinable()) {
    this->proc_thread->join();
    this->proc_thread.reset();
    this->proc_thread = nullptr;
  }
  if (raw_stream.size()) {
    raw_stream.clear();
  }
}

int MessageParser::notify_wakeup() {
  std::unique_lock<std::mutex> lck(this->proc_mutex);
  this->proc_cond.notify_all();

  return 0;
}

int MessageParser::freePacket(void *packet, MessageType type) {
  if (!packet) {
    return 0;
  }

  if (type == RKAIQ_MESSAGE_NEW) {
    RkAiqSocketPacket_t *newpkt = (RkAiqSocketPacket_t *)packet;
    if (newpkt->data) {
      free(newpkt->data);
    }
    free(newpkt);
  } else if (type == RKAIQ_MESSAGE_OLD) {
    RkAiqSocketPacket *newpkt = (RkAiqSocketPacket *)packet;
    if (newpkt->data) {
      free(newpkt->data);
    }
    free(newpkt);
  }

  return 0;
}

void *MessageParser::clonePacket(void *from, MessageType type) {
  if (type == RKAIQ_MESSAGE_NEW) {
    RkAiqSocketPacket_t *temp = (RkAiqSocketPacket_t *)from;
    RkAiqSocketPacket_t *opkt =
        (RkAiqSocketPacket_t *)malloc(sizeof(RkAiqSocketPacket_t));
    if (!opkt) {
      return nullptr;
    }

    memcpy(opkt, temp, sizeof(RkAiqSocketPacket_t));
    opkt->data = (uint8_t *)malloc(temp->payload_size + 1);

    if (!opkt->data) {
      free(opkt);
      return nullptr;
    }

    memcpy(opkt->data, (uint8_t *)&(temp->data), temp->payload_size);
    opkt->data[temp->payload_size] = '\0';

    return opkt;
  } else if (type == RKAIQ_MESSAGE_OLD) {
    RkAiqSocketPacket *temp = (RkAiqSocketPacket *)from;
    RkAiqSocketPacket *opkt =
        (RkAiqSocketPacket *)malloc(sizeof(RkAiqSocketPacket));
    if (!opkt) {
      return nullptr;
    }

    memcpy(opkt, temp, sizeof(RkAiqSocketPacket));
    opkt->data = (char *)malloc(temp->dataSize + 1);

    if (!opkt->data) {
      free(opkt);
      return nullptr;
    }

    memcpy(opkt->data, temp->data, temp->dataSize);
    opkt->data[temp->dataSize] = '\0';

    return opkt;
  }

  return nullptr;
}

void MessageParser::process() {
  while (is_running) {
    std::unique_lock<std::mutex> lck(proc_mutex);
    while (raw_stream.size() <= 0 && is_running) {
      proc_cond.wait(lck);
    }

    if (!is_running) {
      break;
    }

    // Found full packet: deal and call then erease
    // Found non full packet: wait
    // Found nothing: clear raw stream

    size_t start_index = 0;
    size_t end_index = 0;

    RkAiqSocketPacket *old_pkt = nullptr;
    RkAiqSocketPacket_t *new_pkt = nullptr;

    // Check if a new packet, if so, copy it and erase all ahead data
    new_pkt = findValidSection(&raw_stream[0], raw_stream.size(), &start_index,
                               &end_index);
    // if found full packet
    if (new_pkt && mCallBackFunc) {
      mCallBackFunc(pri, new_pkt, RKAIQ_MESSAGE_NEW);
    }

    if (((ssize_t)start_index >= 0) && (end_index > 0)) {
      raw_stream.erase(raw_stream.begin(), raw_stream.begin() + end_index);
    }

    start_index = 0;
    end_index = 0;

    // Check if a new packet, if so, copy it and erase all ahead data
    old_pkt = findValidSection2(raw_stream.size() > 0 ? &raw_stream[0] : NULL,
                                raw_stream.size(), &start_index, &end_index);
    // if found full packet
    if (old_pkt && mCallBackFunc) {
      mCallBackFunc(pri, old_pkt, RKAIQ_MESSAGE_OLD);
      freePacket(old_pkt, RKAIQ_MESSAGE_OLD);
    }

    if (((ssize_t)start_index >= 0) && (end_index > 0)) {
      raw_stream.erase(raw_stream.begin(), raw_stream.begin() + end_index);
    }

    if (!new_pkt && !old_pkt && is_running) {
      proc_cond.wait(lck);
    }
  }
  LOGE("MessageParser %s loop exit!\n", __func__);
}

int MessageParser::pushRawData(const uint8_t *data, size_t size) {
  {
    int erase_section = 0;
    const std::lock_guard<std::mutex> lock(proc_mutex);

    // reach max buffer size, erase old
    if (size > RKAIQ_RAW_STREAM_MAX_SIZE) {
      erase_section = RKAIQ_RAW_STREAM_MAX_SIZE;
    } else {
      erase_section = size;
    }

    // do need erase
    if (raw_stream.size() >= RKAIQ_RAW_STREAM_MAX_SIZE) {
      raw_stream.erase(raw_stream.begin(), raw_stream.begin() + erase_section);
    }

    raw_stream.insert(raw_stream.end(), data, data + size);
  }

  notify_wakeup();

  return 0;
}

uint8_t *MessageParser::bit_stream_find(uint8_t *data, int size,
                                        const uint8_t *dst, int len) {
  int start_pos = -1;

  if (!data || !size || !dst || !len) {
    return NULL;
  }

  if (size < len) {
    return NULL;
  }

  for (start_pos = 0; start_pos < size - len; start_pos++) {
    if (0 == memcmp(data + start_pos, dst, len)) {
      return data + start_pos;
    }
  }

  return NULL;
}

// 1. find valid erase
// 2. find error packet
// 3. if crashed erase header then search again
// 4. erase ahead of first valid data
// 5. return null with non zero start en,then packet error

RkAiqSocketPacket_t *MessageParser::findValidSection(uint8_t *buffer, int len,
                                                     size_t *start_of,
                                                     size_t *end_of) {
  RkAiqSocketPacket_t *aiq_pkt;
  uint8_t *start_pos = NULL;
  size_t skip_size = 0;
  size_t remain_size = 0;

  *start_of = 0;
  *end_of = 0;

  start_pos = bit_stream_find(buffer, len, RKAIQ_SOCKET_DATA_HEADER,
                              RKAIQ_SOCKET_DATA_HEADER_LEN);

  // Found valid start
  if (NULL == start_pos) {
    return nullptr;
  }

  // Calculate data size
  skip_size = start_pos - buffer;
  remain_size = len - skip_size;

  // Check if contains packet information
  if (remain_size < (int)sizeof(RkAiqSocketPacket_t)) {
    LOGE("Not a complete packet [%d], wait more...\n", len);
    return nullptr;
  }

  /*
   * +---------------------------------------------------------------------+
   * |<--------------------------VALID DATA SIZE-------------------------->|
   * +-------------------+------------------------------+------------------+
   * |<---HEADER-SIZE--->|<--------PAYLOAD-SIZE-------->|<---HASH-SIZE---->|
   * +-------------------+------------------------------+------------------+
   * |   HEADER DATA     |      REAL DATA/CMD           |   VERIFY DATA    |
   * +-------------------+------------------------------+------------------+
   *
   * */

  // Found complete packet header, then parse packet info
  aiq_pkt = (RkAiqSocketPacket_t *)start_pos;

  // Assume Single packet, check if data all present
  if (remain_size < (aiq_pkt->packet_size + RKAIQ_SOCKET_DATA_EXTRA_SIZE)) {
    return nullptr;
  }

  *start_of = start_pos - buffer;
  *end_of = *start_of + aiq_pkt->payload_size + RKAIQ_SOCKET_DATA_EXTRA_SIZE;

#ifdef AIQ_MSGPARSER_CHECK_HASH
#endif

  return (RkAiqSocketPacket_t *)clonePacket(aiq_pkt, RKAIQ_MESSAGE_NEW);
}

RkAiqSocketPacket *MessageParser::findValidSection2(uint8_t *buffer, int len,
                                                    size_t *start_of,
                                                    size_t *end_of) {
  RkAiqSocketPacket *aiq_pkt;
  uint8_t *start_pos = NULL;
  size_t skip_size = 0;
  size_t remain_size = 0;
  size_t pkt_size = 0;

  *start_of = 0;
  *end_of = 0;

  start_pos = bit_stream_find(buffer, len, RKAIQ_SOCKET_OLD_HEADER,
                              RKAIQ_SOCKET_OLD_HEADER_LEN);

  // Found valid start
  if (NULL == start_pos) {
    return nullptr;
  }

  // Calculate data size
  skip_size = start_pos - buffer;
  remain_size = len - skip_size;

  // Check if contains packet information
  if (remain_size < ((int)sizeof(RkAiqSocketPacket) - sizeof(void*))) {
    LOGE("Not a complete packet [%d], wait more...\n", len);
    return nullptr;
  }

  // Found complete packet header, then parse packet info
  aiq_pkt = (RkAiqSocketPacket *)start_pos;
  pkt_size = CALC_32BIT_LITTLE(aiq_pkt->packetSize);

  // Assume Single packet, check if data all present
  if (remain_size < pkt_size) {
    return nullptr;
  }

  char *tmpArray = (char *)start_pos;
  int packetSize = (tmpArray[2] & 0xff) | ((tmpArray[3] & 0xff) << 8) |
                   ((tmpArray[4] & 0xff) << 16) | ((tmpArray[5] & 0xff) << 24);

  if (packetSize >= 102400) {
    LOGE("MessageParser %s: packetSize error!\n", __func__);
    return nullptr;
  }

  char *receivedPacket = (char *)malloc(packetSize);
  memset(receivedPacket, 0, packetSize);
  memcpy(receivedPacket, tmpArray, packetSize);

  // parse data
  RkAiqSocketPacket receivedData;
  int offset = 0;
  offset += 2;

  // packetSize
  memcpy(receivedData.packetSize, receivedPacket + offset, 4);
  offset += 4;
  // command id
  memcpy((void *)&(receivedData.commandID), receivedPacket + offset,
         sizeof(int));
  offset += sizeof(int);
  // command id
  memcpy((void *)&(receivedData.commandResult), receivedPacket + offset,
         sizeof(int));
  offset += sizeof(int);

  // data size
  memcpy((void *)&(receivedData.dataSize), receivedPacket + offset,
         sizeof(unsigned int));

  offset += sizeof(unsigned int);
  // data
  receivedData.data = (char *)start_pos + offset;
  offset += receivedData.dataSize;
  // data hash
  memcpy((void *)&(receivedData.dataHash), receivedPacket + offset,
         sizeof(unsigned int));
  free(receivedPacket);
  receivedPacket = NULL;

  // hash check
  unsigned int dataHash = MurMurHash(receivedData.data, receivedData.dataSize);

  // got wrong, discard these data
  if (dataHash != receivedData.dataHash) {
    *start_of = 0;
    *end_of = 0;
    return nullptr;
  }

  *start_of = start_pos - buffer;
  *end_of = *start_of + pkt_size;

  return (RkAiqSocketPacket *)clonePacket(&receivedData, RKAIQ_MESSAGE_OLD);
}

unsigned int MessageParser::MurMurHash(const void *key, int len) {
  const unsigned int m = 0x5bd1e995;
  const int r = 24;
  const int seed = 97;
  unsigned int h = seed ^ len;
  // Mix 4 bytes at a time into the hash
  const unsigned char *data = (const unsigned char *)key;
  while (len >= 4) {
    unsigned int k = *(unsigned int *)data;
    k *= m;
    k ^= k >> r;
    k *= m;
    h *= m;
    h ^= k;
    data += 4;
    len -= 4;
  }
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif
  // Handle the last few bytes of the input array
  switch (len) {
  case 3:
    h ^= data[2] << 16;
  case 2:
    h ^= data[1] << 8;
  case 1:
    h ^= data[0];
    h *= m;
  };
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
  // Do a few final mixes of the hash to ensure the last few
  // bytes are well-incorporated.
  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;
  return h;
}

int MessageParser::start() {
  const std::lock_guard<std::mutex> lock(proc_mutex);
  if (is_running) {
    return -1;
  }

  is_running = true;
  proc_thread =
      std::make_shared<std::thread>(&RkMSG::MessageParser::process, this);

  return 0;
}

} // namespace RkMSG
