#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "message_receiver.h"
#include "socket_client.h"
#include "xcam_log.h"
#include "c_base/aiq_base.h"

#include "rk_aiq.h"
#include "rk_aiq_comm.h"
#include "rk_aiq_user_api2_helper.h"
#include "rk_aiq_user_api2_sysctl.h"

#define MAX_DATA_SIZE 512 * 1024
#define DEFAULT_DATA_SIZE 8192

void HexDump(const void* data, int size) 
{
    char ascii[17];
    int i, j;
    ascii[16] = '\0';
    for (i = 0; i < size; ++i) {
        printf("%02X ", ((unsigned char*)data)[i]);
        if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
            ascii[i % 16] = ((unsigned char*)data)[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i+1) % 8 == 0 || i+1 == size) {
            printf(" ");
            if ((i+1) % 16 == 0) {
                printf("|  %s \n", ascii);
            } else if (i+1 == size) {
                ascii[(i+1) % 16] = '\0';
                if ((i+1) % 16 <= 8) {
                    printf(" ");
                }
                for (j = (i+1) % 16; j < 16; ++j) {
                    printf("   ");
                }
                printf("|  %s \n", ascii);
            }
        }
    }
}

enum Receiver_State_e {
    ReceiverState_SOF = 0,
    ReceiverState_ID,
    ReceiverState_RET,
    ReceiverState_SEQ,
    ReceiverState_SIZE1,
    ReceiverState_SIZE2,
    ReceiverState_DATA,
    ReceiverState_CKSUM
};

void message_receiver_init(receiver_t *rec) {
    memset(rec, 0, sizeof(receiver_t));

    rec->rxi = 0;
    rec->rxbuf = (uint8_t *)malloc(DEFAULT_DATA_SIZE);
    rec->rxbuf_size = DEFAULT_DATA_SIZE;
    rec->rxstate = ReceiverState_SOF;
}

void message_receiver_deinit(receiver_t *rec) {
    if (rec->rxbuf) {
        LOGK_IPC("message_receiver_deinit free\n");
        free(rec->rxbuf);
    }
}

void message_receiver_senddata(receiver_t *rec, char *buf, uint32_t len) {
    uint32_t pos = 0;
    uint32_t cksum = 0;
    uint32_t send_len = 28 + len + 128;

    if (send_len > MAX_DATA_SIZE) {
        LOGE_IPC("AIQ send data to long, abort! %d", send_len);
        return;
    }
    if (send_len > rec->rxbuf_size) {
        LOGK_IPC("SendData realloc rxbuf size %d->%d", rec->rxbuf_size, send_len);
        rec->rxbuf = (uint8_t *) realloc(rec->rxbuf, send_len);
        if (rec->rxbuf == NULL) {
            LOGE_IPC("SendData realloc failed, abort!");
            return;
        }
        rec->rxbuf_size = send_len;
    }
    char *sendbuf = (char *)rec->rxbuf;

    *(uint32_t *) (sendbuf + pos) = rec->magic;
    pos += 4;

    *(uint32_t *) (sendbuf + pos) = rec->cmd_id;
    pos += 4;

    // ret
    *(uint32_t *) (sendbuf + pos) = rec->ret;
    pos += 4;

    // seq
    *(uint32_t *) (sendbuf + pos) = rec->cmd_seq;
    pos += 4;

    *(uint32_t *) (sendbuf + pos) = len;
    pos += 4;

    *(uint32_t *) (sendbuf + pos) = send_len;
    pos += 4;

    memcpy(sendbuf + pos, rec->note, 128);
    pos += 128;

    if (len > 0) {
        memcpy(sendbuf + pos, buf, len);
        pos += len;
    }

    pos += 4;
    LOGI_IPC("Sendbuf len %d, pos %d ret 0x%0x, note %s\n", send_len, pos, rec->ret, rec->note);
    //HexDump(sendbuf, pos);
    send(rec->send_fd, sendbuf, pos, 0);
}

static int rkaiq_is_uapi(const char *cmd_str) {
  if (strstr(cmd_str, "uapi/0/")) {
    return 1;
  } else {
    return 0;
  }
}

void message_receiver_handler(receiver_t *rec) {
    int ret = -1;
    char *out_data = NULL;
    int send_len = 0;

    rec->ret = 0xff00;
    rec->note[0] = 0;

    LOGI_IPC("cmd_id %d, is_uapi %d", rec->cmd_id, rkaiq_is_uapi((char *)rec->rxbuf));


    if (rec->cmd_id == 0) {
        if (rkaiq_is_uapi((char *)rec->rxbuf)) {
            ret = rkaiq_uapi_unified_ctl(rec->aiqctx, (char *)rec->rxbuf, &out_data, 0);
        } else {
            rk_aiq_uapi2_sysctl_tuning(rec->aiqctx, (char *)rec->rxbuf);
        }
        if (out_data) send_len = strlen(out_data);
    } else if (rec->cmd_id == 1) {
        if (rkaiq_is_uapi((char *)rec->rxbuf)) {
            ret = rkaiq_uapi_unified_ctl(rec->aiqctx, (char *)rec->rxbuf, &out_data, 1);
        } else {
            out_data = rk_aiq_uapi2_sysctl_readiq(rec->aiqctx, (char *)rec->rxbuf);
        }
        if (out_data) send_len = strlen(out_data);
    } else if (rec->cmd_id == 0x100) {
        // get 3a stats
        out_data = (char *)socket_client_get_isp_statics(rec->aiqctx);
        send_len = sizeof(rk_aiq_isp_tool_stats_t);
    } else if (rec->cmd_id == 0x101) {
        socket_client_enque_rkraw(rec->aiqctx, (char *)rec->rxbuf);
        out_data = NULL;
    } else if (rec->cmd_id == 0x102) {
        socket_client_writeAwbIn(rec->aiqctx, (char *)rec->rxbuf);
        out_data = NULL;
    } else if (rec->cmd_id == IPC_CMDID_GET_HDRCOMPRCURVE) {
        out_data = (char*)socket_client_get_hdrComprCurve(rec->aiqctx);
        if (out_data) send_len = sizeof(rk_aiq_isp_tool_hdr_compr_curve_t);
    }

    if (out_data) {
        message_receiver_senddata(rec, out_data, send_len);
        aiq_free(out_data);
    } else {
        message_receiver_senddata(rec, NULL, 0);
    }
}

#define when break; case
#define or : case
#define otherwise break; default

#define COLLECT_NUMBER(dest, type) dest = (type)(dest | (c << (rec->rxi*8))); \
                                   if (++rec->rxi == sizeof(type))

void message_receiver_gotchar(receiver_t *rec, uint8_t c)
{
    if (rec->reseti == 0 && c == 'R') {
        rec->reseti ++;
    } else if (rec->reseti == 1 && c == 0xAA) {
        rec->reseti ++;
    } else if (rec->reseti == 2 && c == 0xFF) {
        rec->reseti ++;
    } else if (rec->reseti == 3 && c == 'K') {
        rec->reseti = 0;
        if (rec->rxstate != ReceiverState_SOF) {
            LOGW_IPC("ReceiverState FORCE RESET!!!!\n");
            rec->rxstate = ReceiverState_ID;
            rec->cmd_id = 0;
            rec->rxi = 0;
            return;
        }
    } else {
        rec->reseti = 0;
    }

    switch (rec->rxstate) {
    when ReceiverState_SOF:
        COLLECT_NUMBER(rec->magic, uint32_t) {
            LOGD_IPC("ReceiverState_SOF --> ReceiverState_ID %x", rec->magic);
            if (rec->magic == 0x4bffaa52) {
                rec->rxstate = ReceiverState_ID;
                rec->cmd_id = 0;
                rec->rxi = 0;
            } else {
                LOGW_IPC("ReceiverState_SOF: Bad Magic %x!!", rec->magic);
                rec->cmd_id = 0;
                rec->rxi = 0;
            }
        }
    when ReceiverState_ID:
        COLLECT_NUMBER(rec->cmd_id, uint32_t) {
            LOGD_IPC("ReceiverState_ID %d", rec->cmd_id);
            if (rec->cmd_id > 0x200) {
                LOGE_IPC("ReceiverState_ID Error!! %d", rec->cmd_id);
                rec->rxstate = ReceiverState_SOF;
                rec->rxi = 0;
            }
            rec->rxstate = ReceiverState_RET;
            rec->cmd_ret = 0;
            rec->rxi = 0;
        }
    when ReceiverState_RET:
        COLLECT_NUMBER(rec->cmd_ret, uint32_t) {
            //LOGD_IPC("ReceiverState_RET %d", rec->cmd_ret);
            rec->rxstate = ReceiverState_SEQ;
            rec->cmd_seq = 0;
            rec->rxi = 0;
        }
    when ReceiverState_SEQ:
        COLLECT_NUMBER(rec->cmd_seq, uint32_t) {
            //LOGD_IPC("ReceiverState_SEQ %d", rec->cmd_seq);
            rec->rxstate = ReceiverState_SIZE1;
            rec->size1 = 0;
            rec->size2 = 0;
            rec->rxi = 0;
        }
    when ReceiverState_SIZE1:
        COLLECT_NUMBER(rec->size1, uint32_t) {
            LOGD_IPC("ReceiverState_SIZE1 %d", rec->size1);
            rec->rxstate = ReceiverState_SIZE2;
            rec->rxi = 0;
        }
    when ReceiverState_SIZE2:
        COLLECT_NUMBER(rec->size2, uint32_t) {
            LOGD_IPC("ReceiverState_SIZE2 %d", rec->size2);
            if (rec->size1 == 0) {
                //LOGD_IPC("ReceiverState_SIZE2 ==> ReceiverState_CKSUM");
                rec->rxstate = ReceiverState_CKSUM;
                rec->rxi = 0;
            } else if (rec->size1 > MAX_DATA_SIZE) {
                LOGE_IPC("ReceiverState size too large, abort! %d", rec->size1);
                rec->rxstate = ReceiverState_SOF;
                rec->rxi = 0;
            } else {
                if (rec->size1 > rec->rxbuf_size) {
                    LOGK_IPC("ReceiverState realloc rxbuf size %d->%d", rec->rxbuf_size, rec->size1);
                    rec->rxbuf = (uint8_t *) realloc(rec->rxbuf, rec->size1 + 1);
                    if (rec->rxbuf) {
                        rec->rxbuf_size = rec->size1 + 1;
                        rec->rxstate = ReceiverState_DATA;
                        rec->rxi = 0;
                    } else {
                        LOGE_IPC("ReceiverState rxbuf realloc failed, abort!");
                    }
                } else {
                    //LOGD_IPC("ReceiverState_SIZE2 ==> ReceiverState_DATA");
                    rec->rxstate = ReceiverState_DATA;
                    rec->rxi = 0;
                }
            }
        }
    when ReceiverState_DATA:
        rec->rxbuf[rec->rxi++] = c;
        if (rec->rxi == rec->size1) {
            rec->rxbuf[rec->rxi] = 0;
            LOGD_IPC("ReceiverState_DATA %.*s", rec->size1, rec->rxbuf);
            rec->rxstate = ReceiverState_CKSUM;
            rec->rxi = 0;
        }
    when ReceiverState_CKSUM:
        COLLECT_NUMBER(rec->data_cksum, uint32_t) {
            LOGD_IPC("ReceiverState_CKSUM %x", rec->data_cksum);

            message_receiver_handler(rec);

            rec->rxstate = ReceiverState_SOF;
            rec->rxi = 0;
        }
    otherwise:
        rec->rxstate = ReceiverState_SOF;
        rec->rxpos = 0;
    }
}

void message_receiver_gotdata(receiver_t *rec, uint8_t *buf, int len) {
    //HexDump(buf, len);
    for (int i = 0; i < len; i++) {
        message_receiver_gotchar(rec, buf[i]);
    }
}

