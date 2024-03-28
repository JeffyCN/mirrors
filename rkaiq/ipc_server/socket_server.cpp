#include "socket_server.h"
#include "xcam_obj_debug.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>

#ifdef __ANDROID__
#include <cutils/sockets.h>
#endif

#ifdef __ANDROID__
#define UNIX_DOMAIN "/dev/socket/camera_tool"
#else
#define UNIX_DOMAIN "/tmp/UNIX.domain"
#endif

#define RKAIQ_SOCKET_DATA_OFFSET 24
#define RKAIQ_SOCKET_OLD_HEADER_LEN 2
#define RKAIQ_SOCKET_DATA_HEADER_LEN 4
std::mutex SocketServer::send_mutex;

typedef struct aiq_tunning_ctx_s {
  int socketfd;
  rk_aiq_sys_ctx_t *aiq_ctx;
  RkAiqSocketPacket_t *aiq_data;
} aiq_tunning_ctx;

int onPacketHandle(void *pri, void *packet, MessageType type);

SocketServer::SocketServer()
    : tool_mode_on(false), sockfd(-1), client_socket(-1), quit_(0),
      serverAddress{AF_UNIX, ""}, clientAddress{AF_UNIX, ""}, aiq_ctx(nullptr),
      accept_threads_(nullptr), tunning_thread(nullptr),
      callback_(nullptr), _stop_fds{-1, -1} {
  msg_parser =
      std::unique_ptr<RkMSG::MessageParser>(new RkMSG::MessageParser(this));
  msg_parser->setMsgCallBack(onPacketHandle);
  msg_parser->start();
}

SocketServer::~SocketServer() {
}

void SocketServer::SaveEixt() {
  LOGV_IPC("SocketServer::%s enter", __func__);
  quit_ = 1;
  if (_stop_fds[1] != -1) {
    char buf = 0xf; // random value to write to flush fd.
    unsigned int size = write(_stop_fds[1], &buf, sizeof(char));
    if (size != sizeof(char)) {
      LOGW("Flush write not completed");
    }
  }
}

void hexdump2(char *buf, const int num) {
  int i;
  for (i = 0; i < num; i++) {
    LOGD_IPC("%02X ", buf[i]);
    if ((i + 1) % 32 == 0) {
    }
  }
  return;
}

int ProcessText(int client_socket, rk_aiq_sys_ctx_t *ctx,
                RkAiqSocketPacket *receivedData) {
  int ret = -1;
  RkAiqSocketPacket dataReply;
  ret = ProcessCommand(ctx, receivedData, &dataReply);
  if (ret != -1) {
    const std::lock_guard<std::mutex> lock(SocketServer::send_mutex);
    unsigned int packetSize =
        sizeof(RkAiqSocketPacket) + dataReply.dataSize - sizeof(char *);
    memcpy(dataReply.packetSize, &packetSize, 4);
    char *dataToSend = (char *)malloc(packetSize);
    int offset = 0;
    char magic[2] = {'R', 'K'};
    memset(dataToSend, 0, packetSize);
    memcpy(dataToSend, magic, 2);
    offset += 2;
    memcpy(dataToSend + offset, dataReply.packetSize, 4);
    offset += 4;
    memcpy(dataToSend + offset, (void *)&dataReply.commandID,
           sizeof(dataReply.commandID));
    offset += sizeof(dataReply.commandID);
    memcpy(dataToSend + offset, (void *)&dataReply.commandResult,
           sizeof(dataReply.commandResult));
    offset += sizeof(dataReply.commandResult);
    memcpy(dataToSend + offset, (void *)&dataReply.dataSize,
           sizeof(dataReply.dataSize));
    offset += sizeof(dataReply.dataSize);
    memcpy(dataToSend + offset, dataReply.data, dataReply.dataSize);
    offset += dataReply.dataSize;
    // LOGE_IPC("offset is %d,packetsize is %d",offset,packetSize);
    memcpy(dataToSend + offset, (void *)&dataReply.dataHash,
           sizeof(dataReply.dataHash));
    send(client_socket, dataToSend, packetSize, 0);
    if (dataReply.data != NULL) {
      free(dataReply.data);
      dataReply.data = NULL;
    }
    free(dataToSend);
    dataToSend = NULL;
  } else {
    return -1;
  }
  return 0;
}

int SocketServer::Send(int cilent_socket, char *buff, int size) {
  return send(cilent_socket, buff, size, 0);
}

int SocketServer::Recvieve() { return 0; }

uint8_t *bit_stream_find(uint8_t *data, int size, const uint8_t *dst, int len) {
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

int rkaiq_ipc_send(int sockfd, int id, int ack, int seqn, void *data,
                   uint32_t data_len) {
  uint32_t out_len = sizeof(RkAiqSocketPacket_t) - sizeof(char *) + data_len;
  char *out_data = (char *)malloc(out_len);
  RkAiqSocketPacket_t *out_res = (RkAiqSocketPacket_t *)out_data;
  const std::lock_guard<std::mutex> lock(SocketServer::send_mutex);

  out_res->magic[0] = 'R';
  out_res->magic[1] = 0xAA;
  out_res->magic[2] = 0xFF;
  out_res->magic[3] = 'K';

  out_res->cmd_id = id;
  out_res->cmd_ret = ack;
  out_res->payload_size = data_len;
  out_res->sequence = seqn;
  out_res->packet_size = data_len;

  memcpy(&out_res->data, data, data_len);

  send(sockfd, out_data, out_len, 0);

  free(out_data);

  return 0;
}

// return 0 if a sigle packet or payload size
int rkaiq_packet_parse(RkAiqSocketPacket_t *aiq_data, uint8_t *buffer,
                       int len) {
  uint8_t *start_pos = NULL;
  uint32_t packet_size = 0;
  RkAiqSocketPacket_t *aiq_pkt = NULL;

  start_pos = bit_stream_find(buffer, len, RKAIQ_SOCKET_DATA_HEADER,
                              RKAIQ_SOCKET_DATA_HEADER_LEN);

  if (start_pos) {
    if ((len - (start_pos - buffer)) < (int)sizeof(RkAiqSocketPacket_t)) {
      LOGE_IPC("Not a complete packet [%d], discard!\n", len);
      return -1;
    }

    aiq_pkt = (RkAiqSocketPacket_t *)start_pos;
    packet_size = &buffer[len - 1] - start_pos;

    memcpy(aiq_data, aiq_pkt, sizeof(RkAiqSocketPacket_t));

    // refer to the real offset of data
    aiq_data->data = (uint8_t *)(&aiq_pkt->data);

    // sigle packet : HEAD:24byte + PAYLOAD + CRC:1byte
    if (aiq_pkt->payload_size <= (packet_size - 1)) {
      return 0;
    }
    return packet_size;
  } else {
    // may be fragment packet, head already parsed just return full size
    return -1;
  }
}

int rkaiq_is_uapi(const char *cmd_str) {
  if (strstr(cmd_str, "uapi/0/")) {
    return 1;
  } else {
    return 0;
  }
}

void rkaiq_params_tuning(aiq_tunning_ctx *tunning_ctx) {
  LOGV_IPC("SocketServer::%s enter", __func__);
  int sockfd = -1;
  rk_aiq_sys_ctx_t *aiq_ctx = NULL;
  RkAiqSocketPacket_t *aiq_data = NULL;

  if (!tunning_ctx) {
    return;
  }

  sockfd = tunning_ctx->socketfd;
  aiq_ctx = tunning_ctx->aiq_ctx;
  aiq_data = tunning_ctx->aiq_data;

  LOGI_IPC("[TCP]%d,%d,%d--->PC CMD STRING:\n%s\n", sockfd, aiq_data->cmd_id,
         aiq_data->payload_size, aiq_data->data);

  switch (aiq_data->cmd_id) {
  case AIQ_IPC_CMD_WRITE: {
    if (rkaiq_is_uapi((char *)aiq_data->data)) {
      char *ret_str_js = NULL;
      rkaiq_uapi_unified_ctl(aiq_ctx, (char *)aiq_data->data, &ret_str_js, 0);
    } else {
      rk_aiq_uapi_sysctl_tuning(aiq_ctx, (char *)aiq_data->data);
    }
  } break;
  case AIQ_IPC_CMD_READ: {
    char *out_data = NULL;
    if (rkaiq_is_uapi((char *)aiq_data->data)) {
      rkaiq_uapi_unified_ctl(aiq_ctx, (char *)aiq_data->data, &out_data, 1);
    } else {
      out_data = rk_aiq_uapi_sysctl_readiq(aiq_ctx, (char *)aiq_data->data);
    }

    if (!out_data) {
      LOGE_IPC("[Tuning]: aiq return NULL!\n");
      break;
    }
    LOGI_IPC("---> read:\n%s\n", out_data);
    rkaiq_ipc_send(sockfd, AIQ_IPC_CMD_READ, 0, 0, out_data, strlen(out_data));
    if (out_data)
      free(out_data);
  } break;
  default:
    break;
  }

  if (aiq_data) {
    RkMSG::MessageParser::freePacket(aiq_data, RKAIQ_MESSAGE_NEW);
  }

  free(tunning_ctx);
}

int SocketServer::packetHandle(void *packet, MessageType type) {
  LOGV_IPC("SocketServer::%s enter", __func__);
  if (type == RKAIQ_MESSAGE_NEW) {
    RkAiqSocketPacket_t *aiq_data = (RkAiqSocketPacket_t *)packet;

    if (this->tunning_thread && this->tunning_thread->joinable()) {
      this->tunning_thread->join();
      this->tunning_thread.reset();
      this->tunning_thread = nullptr;
    }

    aiq_tunning_ctx *tunning_ctx =
        (aiq_tunning_ctx *)calloc(1, sizeof(aiq_tunning_ctx));
    tunning_ctx->aiq_data = aiq_data;
    tunning_ctx->aiq_ctx = aiq_ctx;
    tunning_ctx->socketfd = client_socket;

    this->tunning_thread = std::make_shared<std::thread>(
        std::thread(rkaiq_params_tuning, tunning_ctx));
  } else {
    RkAiqSocketPacket *aiq_data = (RkAiqSocketPacket *)packet;
    ProcessText(client_socket, aiq_ctx, aiq_data);
  }

  return 0;
}

int onPacketHandle(void *pri, void *packet, MessageType type) {
  LOGV_IPC("SocketServer::%s enter", __func__);
  SocketServer *server = (SocketServer *)pri;
  if (server) {
    server->packetHandle(packet, type);
  }

  return 0;
}

int SocketServer::Recvieve(int sync) {
  LOGV_IPC("SocketServer::%s enter", __func__);
  uint8_t buffer[MAXPACKETSIZE];
  struct timeval interval = {3, 0};

  setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval,
             sizeof(struct timeval));
  while (!quit_) {
    int recv_len = -1;

    memset(&buffer, 0, MAXPACKETSIZE);

    // 1. recv MAX SIZE every timne.
    recv_len = recv(client_socket, buffer, MAXPACKETSIZE, 0);

    LOGK_IPC("cid[%d] SocketServer::%s recv_len %d",camId_, __func__, recv_len);
    if (recv_len == 0) {
      break;
    }

    if (recv_len < 0) {
      continue;
    }

    msg_parser->pushRawData(buffer, recv_len);

    if (sync) {
    }
  }

  return 0;
}

#define POLL_STOP_RET (3)

int SocketServer::poll_event(int timeout_msec, int fds[]) {
  int num_fds = fds[1] == -1 ? 1 : 2;
  struct pollfd poll_fds[num_fds];
  int ret = 0;

  memset(poll_fds, 0, sizeof(poll_fds));
  poll_fds[0].fd = fds[0];
  poll_fds[0].events = (POLLIN | POLLOUT | POLLHUP);

  if (fds[1] != -1) {
    poll_fds[1].fd = fds[1];
    poll_fds[1].events = POLLPRI | POLLIN | POLLOUT;
    poll_fds[1].revents = 0;
  }

  ret = poll(poll_fds, num_fds, timeout_msec);
  if (fds[1] != -1) {
    if ((poll_fds[1].revents & POLLIN) || (poll_fds[1].revents & POLLPRI)) {
      LOGD_IPC("%s: Poll returning from flush", __FUNCTION__);
      return POLL_STOP_RET;
    }
  }

  if (ret > 0 && (poll_fds[0].revents & (POLLERR | POLLNVAL | POLLHUP))) {
    LOGE_IPC("polled error");
    return -1;
  }

  return ret;
}

void SocketServer::Accepted() {
  LOGV_IPC("SocketServer::%s enter", __func__);
  struct timeval interval = {3, 0};
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval,
             sizeof(struct timeval));
  while (!quit_) {
    // int client_socket;
    socklen_t sosize = sizeof(clientAddress);
    int fds[2] = {sockfd, _stop_fds[0]};
    int poll_ret = poll_event(-1, fds);
    if (poll_ret == POLL_STOP_RET) {
      LOG1("poll socket stop success !");
      break;
    } else if (poll_ret <= 0) {
      LOGW("poll socket got error(0x%x) but continue\n");
      ::usleep(10000); // 10ms
      continue;
    }
    client_socket = accept(sockfd, (struct sockaddr *)&clientAddress, &sosize);
    if (client_socket < 0) {
      if (errno != EAGAIN)
        LOGE_IPC("Error socket accept failed %d %d\n", client_socket, errno);
      continue;
    }
    LOGK_IPC("cid[%d] socket accept server:%s, client:%s\n",
             camId_, serverAddress.sun_path, clientAddress.sun_path);
    tool_mode_set(true);

    // std::shared_ptr<std::thread> recv_thread;
    // recv_thread = make_shared<thread>(&SocketServer::Recvieve, this,
    // client_socket); recv_thread->join(); recv_thread = nullptr;
    this->Recvieve(0);
    close(client_socket);
    LOGK_IPC("cid[%d], socket accept close\n", camId_);
    tool_mode_set(false);
  }
  LOGD_IPC("socket accept exit\n");
}

#ifdef __ANDROID__
int SocketServer::getAndroidLocalSocket(int camid) {
  char socketName[16] = {0};
  sprintf(socketName, "camera_tool%d", camid);
  int sock = android_get_control_socket(socketName);

  LOGK_IPC("cid[%d] %s path:%s, sockfd:%d", camId_, __func__, socketName, sockfd);
  if (sock < 0) {
    // TODO(Cody): will always failed with permission denied
    // Should let init to create socket
    sock = socket_local_server(socketName, ANDROID_SOCKET_NAMESPACE_RESERVED,
                               SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK);
  }

  return sock;
}
#endif

int SocketServer::Process(rk_aiq_sys_ctx_t *ctx, int camid) {
  LOGV_IPC("SocketServer::%s enter", __func__);
  aiq_ctx = ctx;
  camId_ = camid;
#ifdef __ANDROID__
  sockfd = getAndroidLocalSocket(camid);
  if (sockfd < 0) {
    LOGE_IPC("cid[%d] Error get socket %s\n", camid, strerror(errno));
    return -1;
  }
  fcntl(sockfd, F_SETFD, FD_CLOEXEC);
#else
  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  memset(&serverAddress, 0, sizeof(serverAddress));

  serverAddress.sun_family = AF_LOCAL;
  snprintf(serverAddress.sun_path, sizeof(serverAddress.sun_path),
           "%s%d", UNIX_DOMAIN, camid > 0 ? camid : 0);
  unlink(serverAddress.sun_path);

  if ((::bind(sockfd, (struct sockaddr *)&serverAddress,
              sizeof(serverAddress))) < 0) {
    LOGE_IPC("Error bind %s\n", strerror(errno));
    return -1;
  }
  LOGK_IPC("cid[%d] %s path:%s, sockfd:%d", camid, __func__,
           serverAddress.sun_path, sockfd);
#endif
  if (listen(sockfd, 5) < 0) {
    LOGE_IPC("Error listen\n");
    return -1;
  }

  if (pipe(_stop_fds) < 0) {
    LOGE_IPC("poll stop pipe error: %s", strerror(errno));
  } else {
    if (fcntl(_stop_fds[0], F_SETFL, O_NONBLOCK)) {
      LOGE_IPC("Fail to set stop pipe flag: %s", strerror(errno));
    }
  }

  this->accept_threads_ = std::unique_ptr<std::thread>(
      new std::thread(&SocketServer::Accepted, this));

  return 0;
}

void SocketServer::Deinit() {
  LOGV_IPC("SocketServer::%s enter", __func__);
  // struct linger so_linger;
  // so_linger.l_onoff = 1;
  // so_linger.l_linger = 0;
  this->SaveEixt();
  // setsockopt(client_socket,SOL_SOCKET,SO_LINGER,&so_linger,sizeof(so_linger));
  // struct timeval interval = {0, 0};
  // setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char
  // *)&interval,sizeof(struct timeval));
  if (this->accept_threads_)
    this->accept_threads_->join();
  if (this->tunning_thread && this->tunning_thread->joinable())
    this->tunning_thread->join();
  // shutdown(client_socket, SHUT_RDWR);
  // close(client_socket);
#ifndef __ANDROID__
  unlink(serverAddress.sun_path);
  close(sockfd);
#endif
  this->accept_threads_ = nullptr;
  this->tunning_thread = nullptr;
  if (_stop_fds[0] != -1)
    close(_stop_fds[0]);
  if (_stop_fds[1] != -1)
    close(_stop_fds[1]);
  if (msg_parser) {
    msg_parser->stop();
  }
  LOGK_IPC("cid[%d] %s success", camId_, __func__);
}
