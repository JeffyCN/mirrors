#pragma once
// gcc -E rk-isp35-config.h -I.. -I../..

#include <linux/types.h>
#include <linux/v4l2-controls.h>

#define BIT_ULL(nr) (1ULL << (nr))

/****************ISP SUBDEV IOCTL*****************************/
#define RKISP_CMD_TRIGGER_READ_BACK \
    _IOW('V', BASE_VIDIOC_PRIVATE + 0, struct isp2x_csi_trigger)

#define RKISP_CMD_GET_ISP_INFO \
    _IOR('V', BASE_VIDIOC_PRIVATE + 1, struct rkisp_isp_info)

#define RKISP_CMD_GET_SHARED_BUF \
    _IOR('V', BASE_VIDIOC_PRIVATE + 2, struct rkisp_thunderboot_resmem)

#define RKISP_CMD_FREE_SHARED_BUF \
    _IO('V', BASE_VIDIOC_PRIVATE + 3)

#define RKISP_CMD_GET_LDCHBUF_INFO \
    _IOR('V', BASE_VIDIOC_PRIVATE + 4, struct rkisp_ldchbuf_info)

#define RKISP_CMD_SET_LDCHBUF_SIZE \
    _IOW('V', BASE_VIDIOC_PRIVATE + 5, struct rkisp_ldchbuf_size)

#define RKISP_CMD_GET_SHM_BUFFD \
    _IOWR('V', BASE_VIDIOC_PRIVATE + 6, struct rkisp_thunderboot_shmem)

#define RKISP_CMD_GET_FBCBUF_FD \
    _IOR('V', BASE_VIDIOC_PRIVATE + 7, struct isp2x_buf_idxfd)

#define RKISP_CMD_GET_MESHBUF_INFO \
    _IOWR('V', BASE_VIDIOC_PRIVATE + 8, struct rkisp_meshbuf_info)

#define RKISP_CMD_SET_MESHBUF_SIZE \
    _IOW('V', BASE_VIDIOC_PRIVATE + 9, struct rkisp_meshbuf_size)

#define RKISP_CMD_INFO2DDR \
    _IOWR('V', BASE_VIDIOC_PRIVATE + 10, struct rkisp_info2ddr)

#define RKISP_CMD_MESHBUF_FREE \
    _IOW('V', BASE_VIDIOC_PRIVATE + 11, long long)

/* BASE_VIDIOC_PRIVATE + 12 for RKISP_CMD_GET_TB_HEAD_V32 */
/* BASE_VIDIOC_PRIVATE + 14 for RKISP_CMD_SET_TB_HEAD_V32 */

/* for all isp device stop and no power off but resolution change */
#define RKISP_CMD_MULTI_DEV_FORCE_ENUM \
    _IO('V', BASE_VIDIOC_PRIVATE + 13)

#define RKISP_CMD_GET_BAY3D_BUFFD \
    _IOR('V', BASE_VIDIOC_PRIVATE + 15, struct rkisp_bay3dbuf_info)

#define RKISP_CMD_SET_AIISP_LINECNT \
    _IOW('V', BASE_VIDIOC_PRIVATE + 16, struct rkisp_aiisp_cfg)

#define RKISP_CMD_GET_AIISP_LINECNT \
    _IOR('V', BASE_VIDIOC_PRIVATE + 17, struct rkisp_aiisp_cfg)

#define RKISP_CMD_AIISP_RD_START \
    _IOW('V', BASE_VIDIOC_PRIVATE + 18, struct rkisp_aiisp_st)

/* BASE_VIDIOC_PRIVATE + 19 for RKISP_CMD_GET_TB_HEAD_V33 */
/* BASE_VIDIOC_PRIVATE + 20 for RKISP_CMD_SET_TB_HEAD_V33 */

#define RKISP_CMD_SET_OFFLINE_RAW_BUFCNT \
	_IOW('V', BASE_VIDIOC_PRIVATE + 21, int)

#define RKISP_CMD_GET_OFFLINE_RAW_BUFCNT \
	_IOR('V', BASE_VIDIOC_PRIVATE + 22, int)

#define RKISP_CMD_SET_ONLINE_HDR_WRAP_LINE \
	_IOW('V', BASE_VIDIOC_PRIVATE + 23, int)

#define RKISP_CMD_GET_ONLINE_HDR_WRAP_LINE \
	_IOR('V', BASE_VIDIOC_PRIVATE + 24, int)

#define RKISP_CMD_SET_FPN \
	_IOW('V', BASE_VIDIOC_PRIVATE + 25, struct rkisp_fpn_cfg)

#define RKISP_CMD_INIT_BNR_BUF \
       _IOWR('V', BASE_VIDIOC_PRIVATE + 26, struct rkisp_bnr_buf_info)

#define RKISP_CMD_GET_TB_HEAD \
       _IOR('V', BASE_VIDIOC_PRIVATE + 27, struct rkisp_thunderboot_resmem_head)
#define RKISP_CMD_SET_TB_HEAD \
       _IOW('V', BASE_VIDIOC_PRIVATE + 28, struct rkisp_thunderboot_resmem_head)

#define RKISP_CMD_AIAWB_BUF \
	_IOWR('V', BASE_VIDIOC_PRIVATE + 29, struct rkisp_aiawb_buffd)
/****************ISP VIDEO IOCTL******************************/

#define RKISP_CMD_GET_CSI_MEMORY_MODE \
    _IOR('V', BASE_VIDIOC_PRIVATE + 100, int)

#define RKISP_CMD_SET_CSI_MEMORY_MODE \
    _IOW('V', BASE_VIDIOC_PRIVATE + 101, int)

#define RKISP_CMD_GET_CMSK \
    _IOR('V', BASE_VIDIOC_PRIVATE + 102, struct rkisp_cmsk_cfg)

#define RKISP_CMD_SET_CMSK \
    _IOW('V', BASE_VIDIOC_PRIVATE + 103, struct rkisp_cmsk_cfg)

#define RKISP_CMD_GET_STREAM_INFO \
    _IOR('V', BASE_VIDIOC_PRIVATE + 104, struct rkisp_stream_info)

#define RKISP_CMD_GET_MIRROR_FLIP \
    _IOR('V', BASE_VIDIOC_PRIVATE + 105, struct rkisp_mirror_flip)

#define RKISP_CMD_SET_MIRROR_FLIP \
    _IOW('V', BASE_VIDIOC_PRIVATE + 106, struct rkisp_mirror_flip)

#define RKISP_CMD_GET_WRAP_LINE \
    _IOR('V', BASE_VIDIOC_PRIVATE + 107, struct rkisp_wrap_info)
/* set wrap line before VIDIOC_S_FMT */
#define RKISP_CMD_SET_WRAP_LINE \
    _IOW('V', BASE_VIDIOC_PRIVATE + 108, struct rkisp_wrap_info)

#define RKISP_CMD_SET_FPS \
    _IOW('V', BASE_VIDIOC_PRIVATE + 109, int)

#define RKISP_CMD_GET_FPS \
    _IOR('V', BASE_VIDIOC_PRIVATE + 110, int)

#define RKISP_CMD_GET_TB_STREAM_INFO \
    _IOR('V', BASE_VIDIOC_PRIVATE + 111, struct rkisp_tb_stream_info)

#define RKISP_CMD_FREE_TB_STREAM_BUF \
    _IO('V', BASE_VIDIOC_PRIVATE + 112)

/* Private v4l2 event */
#define CIFISP_V4L2_EVENT_STREAM_START \
    (V4L2_EVENT_PRIVATE_START + 1)

#define CIFISP_V4L2_EVENT_STREAM_STOP  \
    (V4L2_EVENT_PRIVATE_START + 2)

#define RKISP_CMD_SET_IQTOOL_CONN_ID \
    _IOW('V', BASE_VIDIOC_PRIVATE + 113, int)

#define RKISP_CMD_SET_EXPANDER \
    _IOW('V', BASE_VIDIOC_PRIVATE + 114, struct rkmodule_hdr_cfg)

#define RKISP_CMD_GET_PARAMS_V33 \
	_IOR('V', BASE_VIDIOC_PRIVATE + 116, struct isp33_isp_params_cfg)

/* frame information attach to image tail, see struct rkisp_frame_info
 * set this before VIDIOC_REQBUFS then VIDIOC_QUERYBUF to get buf size
 */
#define RKISP_CMD_STREAM_ATTACH_INFO \
	_IOW('V', BASE_VIDIOC_PRIVATE + 118, int)

/**********************EVENT_PRIVATE***************************/
#define RKISP_V4L2_EVENT_AIISP_LINECNT (V4L2_EVENT_PRIVATE_START + 1)

/*************************************************************/

#define ISP2X_ID_DPCC           (0)
#define ISP2X_ID_BLS            (1)
#define ISP2X_ID_SDG            (2)
#define ISP2X_ID_SIHST          (3)
#define ISP2X_ID_LSC            (4)
#define ISP2X_ID_AWB_GAIN       (5)
#define ISP2X_ID_BDM            (7)
#define ISP2X_ID_CCM            (8)
#define ISP2X_ID_GOC            (9)
#define ISP2X_ID_CPROC          (10)
#define ISP2X_ID_SIAF           (11)
#define ISP2X_ID_SIAWB          (12)
#define ISP2X_ID_IE         (13)
#define ISP2X_ID_YUVAE          (14)
#define ISP2X_ID_WDR            (15)
#define ISP2X_ID_RK_IESHARP     (16)
#define ISP2X_ID_RAWAF          (17)
#define ISP2X_ID_RAWAE0         (18)
#define ISP2X_ID_RAWAE1         (19)
#define ISP2X_ID_RAWAE2         (20)
#define ISP2X_ID_RAWAE3         (21)
#define ISP2X_ID_RAWAWB         (22)
#define ISP2X_ID_RAWHIST0       (23)
#define ISP2X_ID_RAWHIST1       (24)
#define ISP2X_ID_RAWHIST2       (25)
#define ISP2X_ID_RAWHIST3       (26)
#define ISP2X_ID_HDRMGE         (27)
#define ISP2X_ID_RAWNR          (28)
#define ISP2X_ID_HDRTMO         (29)
#define ISP2X_ID_GIC            (30)
#define ISP2X_ID_DHAZ           (31)
#define ISP2X_ID_3DLUT          (32)
#define ISP2X_ID_LDCH           (33)
#define ISP2X_ID_GAIN           (34)
#define ISP2X_ID_DEBAYER        (35)
#define ISP2X_ID_MAX            (63)

#define ISP2X_MODULE_DPCC       BIT_ULL(ISP2X_ID_DPCC)
#define ISP2X_MODULE_BLS        BIT_ULL(ISP2X_ID_BLS)
#define ISP2X_MODULE_SDG        BIT_ULL(ISP2X_ID_SDG)
#define ISP2X_MODULE_SIHST      BIT_ULL(ISP2X_ID_SIHST)
#define ISP2X_MODULE_LSC        BIT_ULL(ISP2X_ID_LSC)
#define ISP2X_MODULE_AWB_GAIN       BIT_ULL(ISP2X_ID_AWB_GAIN)
#define ISP2X_MODULE_BDM        BIT_ULL(ISP2X_ID_BDM)
#define ISP2X_MODULE_CCM        BIT_ULL(ISP2X_ID_CCM)
#define ISP2X_MODULE_GOC        BIT_ULL(ISP2X_ID_GOC)
#define ISP2X_MODULE_CPROC      BIT_ULL(ISP2X_ID_CPROC)
#define ISP2X_MODULE_SIAF       BIT_ULL(ISP2X_ID_SIAF)
#define ISP2X_MODULE_SIAWB      BIT_ULL(ISP2X_ID_SIAWB)
#define ISP2X_MODULE_IE         BIT_ULL(ISP2X_ID_IE)
#define ISP2X_MODULE_YUVAE      BIT_ULL(ISP2X_ID_YUVAE)
#define ISP2X_MODULE_WDR        BIT_ULL(ISP2X_ID_WDR)
#define ISP2X_MODULE_RK_IESHARP     BIT_ULL(ISP2X_ID_RK_IESHARP)
#define ISP2X_MODULE_RAWAF      BIT_ULL(ISP2X_ID_RAWAF)
#define ISP2X_MODULE_RAWAE0     BIT_ULL(ISP2X_ID_RAWAE0)
#define ISP2X_MODULE_RAWAE1     BIT_ULL(ISP2X_ID_RAWAE1)
#define ISP2X_MODULE_RAWAE2     BIT_ULL(ISP2X_ID_RAWAE2)
#define ISP2X_MODULE_RAWAE3     BIT_ULL(ISP2X_ID_RAWAE3)
#define ISP2X_MODULE_RAWAWB     BIT_ULL(ISP2X_ID_RAWAWB)
#define ISP2X_MODULE_RAWHIST0       BIT_ULL(ISP2X_ID_RAWHIST0)
#define ISP2X_MODULE_RAWHIST1       BIT_ULL(ISP2X_ID_RAWHIST1)
#define ISP2X_MODULE_RAWHIST2       BIT_ULL(ISP2X_ID_RAWHIST2)
#define ISP2X_MODULE_RAWHIST3       BIT_ULL(ISP2X_ID_RAWHIST3)
#define ISP2X_MODULE_HDRMGE     BIT_ULL(ISP2X_ID_HDRMGE)
#define ISP2X_MODULE_RAWNR      BIT_ULL(ISP2X_ID_RAWNR)
#define ISP2X_MODULE_HDRTMO     BIT_ULL(ISP2X_ID_HDRTMO)
#define ISP2X_MODULE_GIC        BIT_ULL(ISP2X_ID_GIC)
#define ISP2X_MODULE_DHAZ       BIT_ULL(ISP2X_ID_DHAZ)
#define ISP2X_MODULE_3DLUT      BIT_ULL(ISP2X_ID_3DLUT)
#define ISP2X_MODULE_LDCH       BIT_ULL(ISP2X_ID_LDCH)
#define ISP2X_MODULE_GAIN       BIT_ULL(ISP2X_ID_GAIN)
#define ISP2X_MODULE_DEBAYER        BIT_ULL(ISP2X_ID_DEBAYER)

#define ISP2X_MODULE_BAYNR			BIT_ULL(36)
#define ISP2X_MODULE_BAY3D			BIT_ULL(37)
#define ISP2X_MODULE_YNR			BIT_ULL(38)
#define ISP2X_MODULE_CNR			BIT_ULL(39)
#define ISP2X_MODULE_SHARP			BIT_ULL(40)
#define ISP2X_MODULE_DRC			BIT_ULL(41)
#define ISP2X_MODULE_CSM			BIT_ULL(43)
#define ISP2X_MODULE_CGC			BIT_ULL(44)

#define ISP3X_MODULE_DPCC		ISP2X_MODULE_DPCC
#define ISP3X_MODULE_BLS		ISP2X_MODULE_BLS
#define ISP3X_MODULE_SDG		ISP2X_MODULE_SDG
#define ISP3X_MODULE_LSC		ISP2X_MODULE_LSC
#define ISP3X_MODULE_AWB_GAIN		ISP2X_MODULE_AWB_GAIN
#define ISP3X_MODULE_BDM		ISP2X_MODULE_BDM
#define ISP3X_MODULE_CCM		ISP2X_MODULE_CCM
#define ISP3X_MODULE_GOC		ISP2X_MODULE_GOC
#define ISP3X_MODULE_CPROC		ISP2X_MODULE_CPROC
#define ISP3X_MODULE_IE			ISP2X_MODULE_IE
#define ISP3X_MODULE_RAWAF		ISP2X_MODULE_RAWAF
#define ISP3X_MODULE_RAWAE0		ISP2X_MODULE_RAWAE0
#define ISP3X_MODULE_RAWAE1		ISP2X_MODULE_RAWAE1
#define ISP3X_MODULE_RAWAE2		ISP2X_MODULE_RAWAE2
#define ISP3X_MODULE_RAWAE3		ISP2X_MODULE_RAWAE3
#define ISP3X_MODULE_RAWAWB		ISP2X_MODULE_RAWAWB
#define ISP3X_MODULE_RAWHIST0		ISP2X_MODULE_RAWHIST0
#define ISP3X_MODULE_RAWHIST1		ISP2X_MODULE_RAWHIST1
#define ISP3X_MODULE_RAWHIST2		ISP2X_MODULE_RAWHIST2
#define ISP3X_MODULE_RAWHIST3		ISP2X_MODULE_RAWHIST3
#define ISP3X_MODULE_HDRMGE		ISP2X_MODULE_HDRMGE
#define ISP3X_MODULE_RAWNR		ISP2X_MODULE_RAWNR
#define ISP3X_MODULE_GIC		ISP2X_MODULE_GIC
#define ISP3X_MODULE_DHAZ		ISP2X_MODULE_DHAZ
#define ISP3X_MODULE_3DLUT		ISP2X_MODULE_3DLUT
#define ISP3X_MODULE_LDCH		ISP2X_MODULE_LDCH
#define ISP3X_MODULE_GAIN		ISP2X_MODULE_GAIN
#define ISP3X_MODULE_DEBAYER		ISP2X_MODULE_DEBAYER
#define ISP3X_MODULE_BAYNR		ISP2X_MODULE_BAYNR
#define ISP3X_MODULE_BAY3D		ISP2X_MODULE_BAY3D
#define ISP3X_MODULE_YNR		ISP2X_MODULE_YNR
#define ISP3X_MODULE_CNR		ISP2X_MODULE_CNR
#define ISP3X_MODULE_SHARP		ISP2X_MODULE_SHARP
#define ISP3X_MODULE_DRC		ISP2X_MODULE_DRC
#define ISP3X_MODULE_CAC		BIT_ULL(42)
#define ISP3X_MODULE_CSM		ISP2X_MODULE_CSM
#define ISP3X_MODULE_CGC		ISP2X_MODULE_CGC

#define ISP32_MODULE_DPCC		ISP3X_MODULE_DPCC
#define ISP32_MODULE_BLS		ISP3X_MODULE_BLS
#define ISP32_MODULE_SDG		ISP3X_MODULE_SDG
#define ISP32_MODULE_LSC		ISP3X_MODULE_LSC
#define ISP32_MODULE_AWB_GAIN		ISP3X_MODULE_AWB_GAIN
#define ISP32_MODULE_BDM		ISP3X_MODULE_BDM
#define ISP32_MODULE_CCM		ISP3X_MODULE_CCM
#define ISP32_MODULE_GOC		ISP3X_MODULE_GOC
#define ISP32_MODULE_CPROC		ISP3X_MODULE_CPROC
#define ISP32_MODULE_IE			ISP3X_MODULE_IE
#define ISP32_MODULE_RAWAF		ISP3X_MODULE_RAWAF
#define ISP32_MODULE_RAWAE0		ISP3X_MODULE_RAWAE0
#define ISP32_MODULE_RAWAE1		ISP3X_MODULE_RAWAE1
#define ISP32_MODULE_RAWAE2		ISP3X_MODULE_RAWAE2
#define ISP32_MODULE_RAWAE3		ISP3X_MODULE_RAWAE3
#define ISP32_MODULE_RAWAWB		ISP3X_MODULE_RAWAWB
#define ISP32_MODULE_RAWHIST0		ISP3X_MODULE_RAWHIST0
#define ISP32_MODULE_RAWHIST1		ISP3X_MODULE_RAWHIST1
#define ISP32_MODULE_RAWHIST2		ISP3X_MODULE_RAWHIST2
#define ISP32_MODULE_RAWHIST3		ISP3X_MODULE_RAWHIST3
#define ISP32_MODULE_HDRMGE		ISP3X_MODULE_HDRMGE
#define ISP32_MODULE_RAWNR		ISP3X_MODULE_RAWNR
#define ISP32_MODULE_GIC		ISP3X_MODULE_GIC
#define ISP32_MODULE_DHAZ		ISP3X_MODULE_DHAZ
#define ISP32_MODULE_3DLUT		ISP3X_MODULE_3DLUT
#define ISP32_MODULE_LDCH		ISP3X_MODULE_LDCH
#define ISP32_MODULE_GAIN		ISP3X_MODULE_GAIN
#define ISP32_MODULE_DEBAYER		ISP3X_MODULE_DEBAYER
#define ISP32_MODULE_BAYNR		ISP3X_MODULE_BAYNR
#define ISP32_MODULE_BAY3D		ISP3X_MODULE_BAY3D
#define ISP32_MODULE_YNR		ISP3X_MODULE_YNR
#define ISP32_MODULE_CNR		ISP3X_MODULE_CNR
#define ISP32_MODULE_SHARP		ISP3X_MODULE_SHARP
#define ISP32_MODULE_DRC		ISP3X_MODULE_DRC
#define ISP32_MODULE_CAC		ISP3X_MODULE_CAC
#define ISP32_MODULE_CSM		ISP3X_MODULE_CSM
#define ISP32_MODULE_CGC		ISP3X_MODULE_CGC
#define ISP32_MODULE_VSM		BIT_ULL(45)
#define ISP32_MODULE_RTT_FST		BIT_ULL(62)

#define ISP33_MODULE_DPCC		ISP3X_MODULE_DPCC
#define ISP33_MODULE_BLS		ISP3X_MODULE_BLS
#define ISP33_MODULE_LSC		ISP3X_MODULE_LSC
#define ISP33_MODULE_AWB_GAIN		ISP3X_MODULE_AWB_GAIN
#define ISP33_MODULE_CCM		ISP3X_MODULE_CCM
#define ISP33_MODULE_GOC		ISP3X_MODULE_GOC
#define ISP33_MODULE_CPROC		ISP3X_MODULE_CPROC
#define ISP33_MODULE_IE			ISP3X_MODULE_IE
#define ISP33_MODULE_RAWAE0		ISP3X_MODULE_RAWAE0
#define ISP33_MODULE_RAWAE3		ISP3X_MODULE_RAWAE3
#define ISP33_MODULE_RAWAWB		ISP3X_MODULE_RAWAWB
#define ISP33_MODULE_RAWHIST0		ISP3X_MODULE_RAWHIST0
#define ISP33_MODULE_RAWHIST3		ISP3X_MODULE_RAWHIST3
#define ISP33_MODULE_HDRMGE		ISP3X_MODULE_HDRMGE
#define ISP33_MODULE_GIC		ISP3X_MODULE_GIC
#define ISP33_MODULE_LDCH		ISP3X_MODULE_LDCH
#define ISP33_MODULE_GAIN		ISP3X_MODULE_GAIN
#define ISP33_MODULE_DEBAYER		ISP3X_MODULE_DEBAYER
#define ISP33_MODULE_BAY3D		ISP3X_MODULE_BAY3D
#define ISP33_MODULE_YNR		ISP3X_MODULE_YNR
#define ISP33_MODULE_CNR		ISP3X_MODULE_CNR
#define ISP33_MODULE_SHARP		ISP3X_MODULE_SHARP
#define ISP33_MODULE_DRC		ISP3X_MODULE_DRC
#define ISP33_MODULE_CAC		ISP3X_MODULE_CAC
#define ISP33_MODULE_CSM		ISP3X_MODULE_CSM
#define ISP33_MODULE_CGC		ISP3X_MODULE_CGC
#define ISP33_MODULE_HSV		BIT_ULL(48)
#define ISP33_MODULE_ENH		BIT_ULL(49)
#define ISP33_MODULE_HIST		BIT_ULL(50)

#define ISP35_MODULE_DPCC		ISP3X_MODULE_DPCC
#define ISP35_MODULE_BLS		ISP3X_MODULE_BLS
#define ISP35_MODULE_SDG		ISP3X_MODULE_SDG
#define ISP35_MODULE_LSC		ISP3X_MODULE_LSC
#define ISP35_MODULE_AWB_GAIN		ISP3X_MODULE_AWB_GAIN
#define ISP35_MODULE_BDM		ISP3X_MODULE_BDM
#define ISP35_MODULE_CCM		ISP3X_MODULE_CCM
#define ISP35_MODULE_GOC		ISP3X_MODULE_GOC
#define ISP35_MODULE_CPROC		ISP3X_MODULE_CPROC
#define ISP35_MODULE_IE			ISP3X_MODULE_IE
#define ISP35_MODULE_RAWAF		ISP3X_MODULE_RAWAF
#define ISP35_MODULE_RAWAE0		ISP3X_MODULE_RAWAE0
#define ISP35_MODULE_RAWAE1		ISP3X_MODULE_RAWAE1
#define ISP35_MODULE_RAWAE2		ISP3X_MODULE_RAWAE2
#define ISP35_MODULE_RAWAE3		ISP3X_MODULE_RAWAE3
#define ISP35_MODULE_RAWAWB		ISP3X_MODULE_RAWAWB
#define ISP35_MODULE_RAWHIST0		ISP3X_MODULE_RAWHIST0
#define ISP35_MODULE_RAWHIST1		ISP3X_MODULE_RAWHIST1
#define ISP35_MODULE_RAWHIST2		ISP3X_MODULE_RAWHIST2
#define ISP35_MODULE_RAWHIST3		ISP3X_MODULE_RAWHIST3
#define ISP35_MODULE_HDRMGE		ISP3X_MODULE_HDRMGE
#define ISP35_MODULE_RAWNR		ISP3X_MODULE_RAWNR
#define ISP35_MODULE_GIC		ISP3X_MODULE_GIC
#define ISP35_MODULE_DHAZ		ISP3X_MODULE_DHAZ
#define ISP35_MODULE_3DLUT		ISP3X_MODULE_3DLUT
#define ISP35_MODULE_LDCH		ISP3X_MODULE_LDCH
#define ISP35_MODULE_GAIN		ISP3X_MODULE_GAIN
#define ISP35_MODULE_DEBAYER		ISP3X_MODULE_DEBAYER
#define ISP35_MODULE_BAYNR		ISP3X_MODULE_BAYNR
#define ISP35_MODULE_BAY3D		ISP3X_MODULE_BAY3D
#define ISP35_MODULE_YNR		ISP3X_MODULE_YNR
#define ISP35_MODULE_CNR		ISP3X_MODULE_CNR
#define ISP35_MODULE_SHARP		ISP3X_MODULE_SHARP
#define ISP35_MODULE_DRC		ISP3X_MODULE_DRC
#define ISP35_MODULE_CAC		ISP3X_MODULE_CAC
#define ISP35_MODULE_CSM		ISP3X_MODULE_CSM
#define ISP35_MODULE_CGC		ISP3X_MODULE_CGC
#define ISP35_MODULE_RGBIR		ISP39_MODULE_RGBIR
#define ISP35_MODULE_HSV		ISP33_MODULE_HSV
#define ISP35_MODULE_ENH		ISP33_MODULE_ENH
#define ISP35_MODULE_HIST		ISP33_MODULE_HIST
#define ISP35_MODULE_AI			BIT_ULL(51)
#define ISP35_MODULE_AIAWB		BIT_ULL(52)
#define ISP35_MODULE_AWBSYNC		BIT_ULL(53)
#define ISP35_MODULE_BAY3D_L2		BIT_ULL(54)


typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

__extension__ typedef __signed__ long long __s64;
__extension__ typedef unsigned long long __u64;

struct rkmodule_reg_struct {
 __u32 reg_addr;
 __u32 reg_val;
};

struct rkmodule_i2cdev_info {
 __u8 slave_addr;
} __attribute__ ((packed));

struct rkmodule_dev_info {
 union {
  struct rkmodule_i2cdev_info i2c_dev;
  __u32 reserved[8];
 };
} __attribute__ ((packed));


enum rkmodule_phy_mode {
 PHY_FULL_MODE,
 PHY_SPLIT_01,
 PHY_SPLIT_23,
};

struct rkmodule_mipi_lvds_bus {
 __u32 bus_type;
 __u32 lanes;
 __u32 phy_mode;
};

struct rkmodule_bus_config {
 union {
  struct rkmodule_mipi_lvds_bus bus;
  __u32 reserved[32];
 };
} __attribute__ ((packed));

struct rkmodule_reg {
 __u64 num_regs;
 __u64 preg_addr;
 __u64 preg_value;
 __u64 preg_addr_bytes;
 __u64 preg_value_bytes;
} __attribute__ ((packed));





struct rkmodule_base_inf {
 char sensor[32];
 char module[32];
 char lens[32];
} __attribute__ ((packed));





struct rkmodule_fac_inf {
 __u32 flag;

 char module[32];
 char lens[32];
 __u32 year;
 __u32 month;
 __u32 day;
} __attribute__ ((packed));





struct rkmodule_awb_inf {
 __u32 flag;

 __u32 r_value;
 __u32 b_value;
 __u32 gr_value;
 __u32 gb_value;

 __u32 golden_r_value;
 __u32 golden_b_value;
 __u32 golden_gr_value;
 __u32 golden_gb_value;
} __attribute__ ((packed));





struct rkmodule_lsc_inf {
 __u32 flag;

 __u16 lsc_w;
 __u16 lsc_h;
 __u16 decimal_bits;

 __u16 lsc_r[289];
 __u16 lsc_b[289];
 __u16 lsc_gr[289];
 __u16 lsc_gb[289];

 __u16 width;
 __u16 height;
 __u16 table_size;
} __attribute__ ((packed));




enum rkmodele_af_otp_dir {
 AF_OTP_DIR_HORIZONTAL = 0,
 AF_OTP_DIR_UP = 1,
 AF_OTP_DIR_DOWN = 2,
};




struct rkmodule_af_otp {
 __u32 vcm_start;
 __u32 vcm_end;
 __u32 vcm_dir;
};





struct rkmodule_af_inf {
 __u32 flag;
 __u32 dir_cnt;
 struct rkmodule_af_otp af_otp[3];
} __attribute__ ((packed));





struct rkmodule_pdaf_inf {
 __u32 flag;

 __u32 gainmap_width;
 __u32 gainmap_height;
 __u32 dccmap_width;
 __u32 dccmap_height;
 __u32 dcc_mode;
 __u32 dcc_dir;
 __u32 pd_offset;
 __u16 gainmap[1024];
 __u16 dccmap[256];
} __attribute__ ((packed));





struct rkmodule_otp_module_inf {
 __u32 flag;
 __u8 vendor[8];
 __u32 module_id;
 __u16 version;
 __u16 full_width;
 __u16 full_height;
 __u8 supplier_id;
 __u8 year;
 __u8 mouth;
 __u8 day;
 __u8 sensor_id;
 __u8 lens_id;
 __u8 vcm_id;
 __u8 drv_id;
 __u8 flip;
} __attribute__ ((packed));





struct rkmodule_inf {
 struct rkmodule_base_inf base;
 struct rkmodule_fac_inf fac;
 struct rkmodule_awb_inf awb;
 struct rkmodule_lsc_inf lsc;
 struct rkmodule_af_inf af;
 struct rkmodule_pdaf_inf pdaf;
 struct rkmodule_otp_module_inf module_inf;
} __attribute__ ((packed));





struct rkmodule_awb_cfg {
 __u32 enable;
 __u32 golden_r_value;
 __u32 golden_b_value;
 __u32 golden_gr_value;
 __u32 golden_gb_value;
} __attribute__ ((packed));





struct rkmodule_af_cfg {
 __u32 enable;
 __u32 vcm_start;
 __u32 vcm_end;
 __u32 vcm_dir;
} __attribute__ ((packed));





struct rkmodule_lsc_cfg {
 __u32 enable;
} __attribute__ ((packed));







enum rkmodule_hdr_mode {
 NO_HDR = 0,
 HDR_X2 = 5,
 HDR_X3 = 6,
 HDR_COMPR,
};

struct rkmodule_hdr_compr {
 __u8 point;
 __u8 src_bit;
 __u8 k_shift;
 __u16 data_compr[32];
 __u32 data_src[32];
 __u32 slope_k[32];
};






enum hdr_esp_mode {
 HDR_NORMAL_VC = 0,
 HDR_LINE_CNT,
 HDR_ID_CODE,
};




enum rkmodule_csi_dsi_seq {
 RKMODULE_CSI_INPUT = 0,
 RKMODULE_DSI_INPUT,
};




enum rkmodule_dsi_mode {
 RKMODULE_DSI_VIDEO = 0,
 RKMODULE_DSI_COMMAND,
};

struct rkmodule_hdr_esp {
 enum hdr_esp_mode mode;
 union {
  struct {
   __u32 padnum;
   __u32 padpix;
  } lcnt;
  struct {
   __u32 efpix;
   __u32 obpix;
  } idcd;
 } val;
};

enum exp_mode_e {
 EXP_NORMAL = 0,
 EXP_HDR2_STA,
 EXP_HDR2_DCG,
 EXP_HDR3_DCG_VS,
 EXP_HDR3_DCG_SPD,
 EXP_HDR3_STA,
 EXP_HDR3_DCG_LOFIC,
 EXP_HDR3_LCG_LOFIC_VS,
};

struct rkmodule_hdr_cfg {
 __u32 hdr_mode;
 struct rkmodule_hdr_esp esp;
 struct rkmodule_hdr_compr compr;
} __attribute__ ((packed));





struct rkmodule_sync_code {
 __u16 sav;
 __u16 eav;
};

enum rkmodule_lvds_mode {
 LS_FIRST = 0,
 FS_FIRST,
 SONY_DOL_HDR_1,
 SONY_DOL_HDR_2
};





struct rkmodule_lvds_frm_sync_code {
 struct rkmodule_sync_code act;
 struct rkmodule_sync_code blk;
};





struct rkmodule_lvds_frame_sync_code {
 struct rkmodule_lvds_frm_sync_code odd_sync_code;
 struct rkmodule_lvds_frm_sync_code even_sync_code;
};


enum rkmodule_lvds_sync_code_group {
 LVDS_CODE_GRP_LINEAR = 0x0,
 LVDS_CODE_GRP_LONG,
 LVDS_CODE_GRP_MEDIUM,
 LVDS_CODE_GRP_SHORT,
 LVDS_CODE_GRP_MAX
};

struct rkmodule_lvds_cfg {
 enum rkmodule_lvds_mode mode;
 struct rkmodule_lvds_frame_sync_code frm_sync_code[LVDS_CODE_GRP_MAX];
} __attribute__ ((packed));

struct rkmodule_dpcc_cfg {
 __u32 enable;
 __u32 cur_single_dpcc;
 __u32 cur_multiple_dpcc;
 __u32 total_dpcc;
} __attribute__ ((packed));

struct rkmodule_nr_switch_threshold {
 __u32 direct;
 __u32 up_thres;
 __u32 down_thres;
 __u32 div_coeff;
} __attribute__ ((packed));





enum rkmodule_bt656_intf_type {
 BT656_STD_RAW = 0,
 BT656_SONY_RAW,
};





struct rkmodule_vc_fmt_info {
 __u32 width[4];
 __u32 height[4];
 __u32 fps[4];
} __attribute__ ((packed));






struct rkmodule_vc_hotplug_info {
 __u8 detect_status;
} __attribute__ ((packed));







enum rkmodule_start_stream_seq {
 RKMODULE_START_STREAM_DEFAULT = 0,
 RKMODULE_START_STREAM_BEHIND,
 RKMODULE_START_STREAM_FRONT,
};




enum rkmodule_hdmiin_mode_seq {
 RKMODULE_HDMIIN_DEFAULT = 0,
 RKMODULE_HDMIIN_MODE,
};



enum rkmodule_reset_src {
 RKCIF_RESET_SRC_NON = 0x0,
 RKCIF_RESET_SRC_ERR_CSI2,
 RKCIF_RESET_SRC_ERR_LVDS,
 RKICF_RESET_SRC_ERR_CUTOFF,
 RKCIF_RESET_SRC_ERR_HOTPLUG,
 RKCIF_RESET_SRC_ERR_APP,
 RKCIF_RESET_SRC_ERR_ISP,
};

struct rkmodule_vicap_reset_info {
 __u32 is_reset;
 enum rkmodule_reset_src src;
} __attribute__ ((packed));

struct rkmodule_bt656_mbus_info {
 __u32 flags;
 __u32 id_en_bits;
} __attribute__ ((packed));


struct rkmodule_dcg_ratio {
 __u32 integer;
 __u32 decimal;
 __u32 div_coeff;
};

struct rkmodule_channel_info {
 __u32 index;
 __u32 vc;
 __u32 width;
 __u32 height;
 __u32 bus_fmt;
 __u32 data_type;
 __u32 data_bit;
 __u32 field;
} __attribute__ ((packed));

enum rkmodule_max_pad {
 PAD0,
 PAD1,
 PAD2,
 PAD3,
 PAD_MAX,
};




enum rkmodule_sync_mode {
 NO_SYNC_MODE = 0,
 EXTERNAL_MASTER_MODE,
 INTERNAL_MASTER_MODE,
 SLAVE_MODE,
 SOFT_SYNC_MODE,
};

struct rkmodule_mclk_data {
 __u32 enable;
 __u32 mclk_index;
 __u32 mclk_rate;
 __u32 reserved[8];
};

enum csi2_dphy_vendor {
 PHY_VENDOR_INNO = 0x0,
 PHY_VENDOR_SAMSUNG = 0x01,
};

struct rkmodule_csi_dphy_param {
 __u32 vendor;
 __u32 lp_vol_ref;
 __u32 lp_hys_sw[4];
 __u32 lp_escclk_pol_sel[4];
 __u32 skew_data_cal_clk[4];
 __u32 clk_hs_term_sel;
 __u32 data_hs_term_sel[4];
 __u32 reserved[32];
};

struct rkmodule_sensor_fmt {
 __u32 sensor_index;
 __u32 sensor_width;
 __u32 sensor_height;
};

struct rkmodule_sensor_infos {
 struct rkmodule_sensor_fmt sensor_fmt[8];
};

enum rkmodule_capture_mode {
 RKMODULE_CAPTURE_MODE_NONE = 0,
 RKMODULE_MULTI_DEV_COMBINE_ONE,
 RKMODULE_ONE_CH_TO_MULTI_ISP,
 RKMODULE_MULTI_CH_TO_MULTI_ISP,
 RKMODULE_MULTI_CH_COMBINE_SQUARE,
};

struct rkmodule_multi_dev_info {
 __u32 dev_idx[4];
 __u32 combine_idx[4];
 __u32 pixel_offset;
 __u32 dev_num;
 __u32 reserved[8];
};

struct rkmodule_one_to_multi_info {
 __u32 isp_num;
 __u32 frame_pattern[4];
};

struct rkmodule_multi_combine_info {
 __u32 combine_num;
 __u32 combine_index[4];
};

struct rkmodule_capture_info {
 __u32 mode;
 union {
  struct rkmodule_multi_dev_info multi_dev;
  struct rkmodule_one_to_multi_info one_to_multi;
  struct rkmodule_multi_combine_info multi_combine_info;
 };
};

enum rk_light_type {
 LIGHT_PWM,
 LIGHT_GPIO,
};

struct rk_light_param {
 __u8 light_type;
 __u8 light_enable;
 __u64 duty_cycle;
 __u64 period;
 __u32 polarity;
} __attribute__ ((packed));

struct rk_sensor_setting {
 __u32 width;
 __u32 height;
 __u32 fps;
 __u32 fmt;
 __u32 mode;
} __attribute__ ((packed));

struct rkmodule_exp_delay {
 __u32 exp_delay;
 __u32 gain_delay;
 __u32 vts_delay;
 __u32 dcg_delay;
 __u32 reserved[2];
} __attribute__ ((packed));

enum rkmodule_gain_mode_e {
 RKMODULE_GAIN_MODE_LINEAR,
 RKMODULE_GAIN_MODE_DB,
};

struct rkmodule_gain_mode {
 __u32 gain_mode;
 __u32 factor;
} __attribute__ ((packed));

struct rkmodule_exp_info {
 __u32 exp[3];
 __u32 gain[3];
 __u32 exp_reg[3];
 __u32 gain_reg[3];
 __u32 hts;
 __u32 vts;
 __u32 pclk;
 __u32 dcg_used;
 __u32 dcg_val[3];
 struct rkmodule_dcg_ratio dcg_ratio;
 struct rkmodule_gain_mode gain_mode;
 __u32 reserved[6];
} __attribute__ ((packed));



enum rkmodule_wb_type {
 RKMODULE_HCG_WB_GAIN,
 RKMODULE_LCG_WB_GAIN,
 RKMODULE_SPD_WB_GAIN,
 RKMODULE_VS_WB_GAIN,
};

struct rkmodule_wb_gain {
 __u32 b_gain;
 __u32 gb_gain;
 __u32 gr_gain;
 __u32 r_gain;
};

struct rkmodule_wb_gain_group {
 __u32 group_num;
 enum rkmodule_wb_type wb_gain_type[(4)];
 struct rkmodule_wb_gain wb_gain[(4)];
};



enum rkmodule_blc_type {
 RKMODULE_HCG_BLC,
 RKMODULE_LCG_BLC,
 RKMODULE_SPD_BLC,
 RKMODULE_VS_BLC,
};

struct rkmodule_blc_group {
 __u32 enable;
 __u32 group_num;
 enum rkmodule_blc_type blc_type[(4)];
 __u32 blc[(4)];
 __u32 bkdg_sw_en;
 __u32 dgbk2bkdg_thred;
 __u32 bkdg2dgbk_thred;
 __u32 reg_num;
 struct rkmodule_reg_struct reg_list[(16)];
};

enum rkmodule_bayer_mode {
 RKMODULE_NORMAL_BAYER,
 RKMODULE_QUARD_BAYER,
};

struct rkmodule_wb_gain_info {
 __u32 coarse_bit;
 __u32 fine_bit;
 __u32 reserved[8];
};

struct rkmodule_blc_info {
 __u32 bit_width;
 __u32 reserved[8];
};

enum rkmodule_cmps_mode {
 CMPS_LOW_BIT_WIDTH_MODE,
 CMPS_HIGH_BIT_WIDTH_MODE,
};

struct rkmodule_error_info {
 __u32 err_code;
 __u8 detail[256];
};

enum rkmodule_expand_single_mode {
 EXPAND_SINGLE_LCG,
 EXPAND_SINGLE_HCG,
 EXPAND_SINGLE_VS,
 EXPAND_SINGLE_SPD,
 EXPAND_SINGLE_LOFIC,
};



struct rkmodule_lenc_gain {
 __u32 g[289];
 __u32 b[289];
 __u32 r[289];
};

struct rkmodule_lenc_data {
 __u16 rgain;
 __u16 bgain;
 struct rkmodule_lenc_gain lenc_gain;
};

struct rkmodule_lenc_inf {
 __u32 flag;
 __u32 group_num;
 __u32 lenc_gain_len;
 struct rkmodule_lenc_data lenc_data[(4)];
};

struct rkmodule_lenc_info {
 __u32 bit_width;
 __u32 grid_num;
 __u32 reserved[8];
};

enum rkmodule_binning_mode {
 BAYER_BINNING_2X2,
 BAYER_SKIP_2X2,
 QBC_BINNING_2X2,
};

struct rkmodule_reg_setting {
 __u32 setting_id;
 __u32 binning_mode;
 __u32 reg_num;
 struct rkmodule_reg_struct reg_list[(16)];
};

struct rkmodule_bayer_param {
 __u32 bayer_mode;
 __u32 reg_num;
 struct rkmodule_reg_struct reg_list[(16)];
};

struct rkmodule_hdr_compr_single_frame_info {
 __u32 single_bitwidth;
 __u32 reserved[8];
};

struct rkmodule_channel_power {
 __u32 channel;
 __u32 enable;
};

struct rkmodule_channel_stream {
 __u32 channel;
 __u32 enable;
};


struct rkisp_buf_info {
 int buf_cnt;
 int buf_size;
 int buf_stride;
 int buf_fd[8];
} __attribute__ ((packed));

enum rkisp_aiawb_ds {
 RKISP_AIAWB_DS_4X4,
 RKISP_AIAWB_DS_8X4,
 RKISP_AIAWB_DS_8X8,
 RKISP_AIAWB_DS_16X16,
};




struct rkisp_aiawb_buffd {
 enum rkisp_aiawb_ds ds;
 struct rkisp_buf_info info;
} __attribute__ ((packed));

enum rkisp_isp_mode {

 RKISP_ISP_NORMAL = (((1UL)) << (0)),
 RKISP_ISP_HDR2 = (((1UL)) << (1)),
 RKISP_ISP_HDR3 = (((1UL)) << (2)),
 RKISP_ISP_COMPR = (((1UL)) << (3)),


 RKISP_ISP_BIGMODE = (((1UL)) << (28)),
};

struct rkisp_isp_info {
 enum rkisp_isp_mode mode;
 __u32 act_width;
 __u32 act_height;
 __u8 compr_bit;
} __attribute__ ((packed));

enum isp2x_mesh_buf_stat {
 MESH_BUF_INIT = 0,
 MESH_BUF_WAIT2CHIP,
 MESH_BUF_CHIPINUSE,
};

struct rkisp_meshbuf_info {
 __u64 module_id;
 __u32 unite_isp_id;
 __s32 buf_fd[3];
 __u32 buf_size[3];
} __attribute__ ((packed));

struct rkisp_meshbuf_size {
 __u64 module_id;
 __u32 unite_isp_id;
 __u32 meas_width;
 __u32 meas_height;
 int buf_cnt;
} __attribute__ ((packed));

struct isp2x_mesh_head {
 enum isp2x_mesh_buf_stat stat;
 __u32 data_oft;
 __u32 data1_oft;
} __attribute__ ((packed));

enum {
 RKISP_FPN_DATA_SHIFT_0 = 0,
 RKISP_FPN_DATA_SHIFT_1,
 RKISP_FPN_DATA_SHIFT_2,
 RKISP_FPN_DATA_SHIFT_3,
};

struct rkisp_fpn_cfg {
 char en;
 char row_en;
 char data_shift;
 char reserved;
 int buf_size;
 void *buf;
} __attribute__ ((packed));



struct rkisp_aiisp_ev_info {
 unsigned long long timestamp;
 int sequence;
 int height;

 int iir_index;
 int gain_index;
 int aipre_gain_index;
 int vpsl_index;

 int aiisp_index;

 int y_src_index;
 int y_dest_index;
} __attribute__ ((packed));

struct rkisp_aiisp_st {
 unsigned long long timestamp;
 int sequence;

 int iir_index;
 int gain_index;

 int aiisp_index;

 int aipre_gain_index;
 int vpsl_index;

 int y_src_index;
 int y_dest_index;
} __attribute__ ((packed));







struct rkisp_aiisp_cfg {
 int mode;
 int wr_linecnt;
 int rd_linecnt;
 int wr_mode;
} __attribute__ ((packed));




struct rkisp_bnr_buf_info {
 struct rkisp_buf_info iir;
 union {
  struct {
   struct rkisp_buf_info aiisp;
   struct rkisp_buf_info gain;
   __u8 iirsparse_en;
  } v39;
  struct {
   struct rkisp_buf_info ds;
   struct rkisp_buf_info wgt;

   struct rkisp_buf_info aiisp;
   struct rkisp_buf_info gain;
   struct rkisp_buf_info aipre_gain;
   struct rkisp_buf_info vpsl;
   struct rkisp_buf_info y_src;
   __u8 iir_rw_fmt;
   __u8 gain_mode;
   __u8 yraw_sel;
   __u8 aibnr_l2;

   __u32 vpsl_yraw_offs[6];
   __u32 vpsl_yraw_stride[6];

   __u32 vpsl_sig_offs[5];
   __u32 vpsl_sig_stride[5];
  } v35;
 } u;
} __attribute__ ((packed));

struct rkisp_bay3dbuf_info {
 int iir_fd;
 int iir_size;
 union {
  struct {
   int cur_fd;
   int cur_size;
   int ds_fd;
   int ds_size;
  } v30;
  struct {
   int ds_fd;
   int ds_size;
  } v32;
  struct {
   int ds_fd;
   int ds_size;
   int gain_fd;
   int gain_size;
  } v33;
 } u;
} __attribute__ ((packed));

struct rkisp_cmsk_win {
 unsigned short mode;
 unsigned short win_en;

 unsigned char cover_color_y;
 unsigned char cover_color_u;
 unsigned char cover_color_v;

 unsigned short h_offs;
 unsigned short v_offs;
 unsigned short h_size;
 unsigned short v_size;
} __attribute__ ((packed));







struct rkisp_cmsk_cfg {
 struct rkisp_cmsk_win win[12];
 unsigned int mosaic_block;
 unsigned int width_ro;
 unsigned int height_ro;
} __attribute__ ((packed));







struct rkisp_stream_info {
 unsigned int cur_frame_id;
 unsigned int input_frame_loss;
 unsigned int output_frame_loss;
 unsigned char stream_on;
 unsigned char stream_id;
} __attribute__ ((packed));





struct rkisp_mirror_flip {
 unsigned char mirror;
 unsigned char flip;
} __attribute__ ((packed));

struct rkisp_wrap_info {
 int width;
 int height;
};


struct rkisp_tb_stream_buf {
 unsigned int dma_addr;
 unsigned int sequence;
 long long timestamp;
} __attribute__ ((packed));






struct rkisp_tb_stream_info {
 unsigned int width;
 unsigned int height;
 unsigned int bytesperline;
 unsigned int frame_size;
 unsigned int buf_max;
 unsigned int buf_cnt;
 struct rkisp_tb_stream_buf buf[5];
} __attribute__ ((packed));

enum isp2x_trigger_mode {
 T_TRY = (((1UL)) << (0)),
 T_TRY_YES = (((1UL)) << (1)),
 T_TRY_NO = (((1UL)) << (2)),

 T_START_X1 = (((1UL)) << (4)),
 T_START_X2 = (((1UL)) << (5)),
 T_START_X3 = (((1UL)) << (6)),
 T_START_C = (((1UL)) << (7)),
};

struct isp2x_csi_trigger {

 __u64 sof_timestamp;
 __u64 frame_timestamp;
 __u32 frame_id;
 int times;
 enum isp2x_trigger_mode mode;
} __attribute__ ((packed));

enum isp_csi_memory {
 CSI_MEM_COMPACT = 0,
 CSI_MEM_WORD_BIG_END = 1,
 CSI_MEM_WORD_LITTLE_ALIGN = 1,
 CSI_MEM_WORD_BIG_ALIGN = 2,
};





enum rkisp_info2ddr_owner {
 RKISP_INFO2DRR_OWNER_NULL,
 RKISP_INFO2DRR_OWNER_GAIN,
 RKISP_INFO2DRR_OWNER_AWB,
};

struct rkisp_info2ddr {
 enum rkisp_info2ddr_owner owner;

 union {
  struct {
   __u8 gain2ddr_mode;
  } gain;

  struct {
   __u8 awb2ddr_sel;
  } awb;
 } u;

 __u8 buf_cnt;
 __s32 buf_fd[4];

 __u32 wsize;
 __u32 vsize;
} __attribute__ ((packed));

struct isp2x_ispgain_buf {
 __u32 gain_dmaidx;
 __u32 mfbc_dmaidx;
 __u32 gain_size;
 __u32 mfbc_size;
 __u32 frame_id;
} __attribute__ ((packed));

struct isp2x_buf_idxfd {
 __u32 buf_num;
 __u32 index[64];
 __s32 dmafd[64];
} __attribute__ ((packed));

struct isp2x_window {
 __u16 h_offs;
 __u16 v_offs;
 __u16 h_size;
 __u16 v_size;
} __attribute__ ((packed));

struct isp2x_bls_fixed_val {
 __s16 r;
 __s16 gr;
 __s16 gb;
 __s16 b;
} __attribute__ ((packed));

struct isp2x_bls_cfg {
 __u8 enable_auto;
 __u8 en_windows;
 struct isp2x_window bls_window1;
 struct isp2x_window bls_window2;
 __u8 bls_samples;
 struct isp2x_bls_fixed_val fixed_val;
} __attribute__ ((packed));

struct isp2x_bls_stat {
 __u16 meas_r;
 __u16 meas_gr;
 __u16 meas_gb;
 __u16 meas_b;
} __attribute__ ((packed));

struct isp2x_dpcc_pdaf_point {
 __u8 y;
 __u8 x;
} __attribute__ ((packed));

struct isp2x_dpcc_cfg {

 __u8 stage1_enable;
 __u8 grayscale_mode;


 __u8 sw_rk_out_sel;
 __u8 sw_dpcc_output_sel;
 __u8 stage1_rb_3x3;
 __u8 stage1_g_3x3;
 __u8 stage1_incl_rb_center;
 __u8 stage1_incl_green_center;


 __u8 stage1_use_fix_set;
 __u8 stage1_use_set_3;
 __u8 stage1_use_set_2;
 __u8 stage1_use_set_1;


 __u8 sw_rk_red_blue1_en;
 __u8 rg_red_blue1_enable;
 __u8 rnd_red_blue1_enable;
 __u8 ro_red_blue1_enable;
 __u8 lc_red_blue1_enable;
 __u8 pg_red_blue1_enable;
 __u8 sw_rk_green1_en;
 __u8 rg_green1_enable;
 __u8 rnd_green1_enable;
 __u8 ro_green1_enable;
 __u8 lc_green1_enable;
 __u8 pg_green1_enable;


 __u8 sw_rk_red_blue2_en;
 __u8 rg_red_blue2_enable;
 __u8 rnd_red_blue2_enable;
 __u8 ro_red_blue2_enable;
 __u8 lc_red_blue2_enable;
 __u8 pg_red_blue2_enable;
 __u8 sw_rk_green2_en;
 __u8 rg_green2_enable;
 __u8 rnd_green2_enable;
 __u8 ro_green2_enable;
 __u8 lc_green2_enable;
 __u8 pg_green2_enable;


 __u8 sw_rk_red_blue3_en;
 __u8 rg_red_blue3_enable;
 __u8 rnd_red_blue3_enable;
 __u8 ro_red_blue3_enable;
 __u8 lc_red_blue3_enable;
 __u8 pg_red_blue3_enable;
 __u8 sw_rk_green3_en;
 __u8 rg_green3_enable;
 __u8 rnd_green3_enable;
 __u8 ro_green3_enable;
 __u8 lc_green3_enable;
 __u8 pg_green3_enable;


 __u8 sw_mindis1_rb;
 __u8 sw_mindis1_g;
 __u8 line_thr_1_rb;
 __u8 line_thr_1_g;


 __u8 sw_dis_scale_min1;
 __u8 sw_dis_scale_max1;
 __u8 line_mad_fac_1_rb;
 __u8 line_mad_fac_1_g;


 __u8 pg_fac_1_rb;
 __u8 pg_fac_1_g;


 __u8 rnd_thr_1_rb;
 __u8 rnd_thr_1_g;


 __u8 rg_fac_1_rb;
 __u8 rg_fac_1_g;


 __u8 sw_mindis2_rb;
 __u8 sw_mindis2_g;
 __u8 line_thr_2_rb;
 __u8 line_thr_2_g;


 __u8 sw_dis_scale_min2;
 __u8 sw_dis_scale_max2;
 __u8 line_mad_fac_2_rb;
 __u8 line_mad_fac_2_g;


 __u8 pg_fac_2_rb;
 __u8 pg_fac_2_g;


 __u8 rnd_thr_2_rb;
 __u8 rnd_thr_2_g;


 __u8 rg_fac_2_rb;
 __u8 rg_fac_2_g;


 __u8 sw_mindis3_rb;
 __u8 sw_mindis3_g;
 __u8 line_thr_3_rb;
 __u8 line_thr_3_g;


 __u8 sw_dis_scale_min3;
 __u8 sw_dis_scale_max3;
 __u8 line_mad_fac_3_rb;
 __u8 line_mad_fac_3_g;


 __u8 pg_fac_3_rb;
 __u8 pg_fac_3_g;


 __u8 rnd_thr_3_rb;
 __u8 rnd_thr_3_g;


 __u8 rg_fac_3_rb;
 __u8 rg_fac_3_g;


 __u8 ro_lim_3_rb;
 __u8 ro_lim_3_g;
 __u8 ro_lim_2_rb;
 __u8 ro_lim_2_g;
 __u8 ro_lim_1_rb;
 __u8 ro_lim_1_g;


 __u8 rnd_offs_3_rb;
 __u8 rnd_offs_3_g;
 __u8 rnd_offs_2_rb;
 __u8 rnd_offs_2_g;
 __u8 rnd_offs_1_rb;
 __u8 rnd_offs_1_g;


 __u8 bpt_rb_3x3;
 __u8 bpt_g_3x3;
 __u8 bpt_incl_rb_center;
 __u8 bpt_incl_green_center;
 __u8 bpt_use_fix_set;
 __u8 bpt_use_set_3;
 __u8 bpt_use_set_2;
 __u8 bpt_use_set_1;
 __u8 bpt_cor_en;
 __u8 bpt_det_en;


 __u16 bp_number;


 __u16 bp_table_addr;


 __u16 bpt_v_addr;
 __u16 bpt_h_addr;


 __u32 bp_cnt;


 __u8 sw_pdaf_en;


 __u8 pdaf_point_en[16];


 __u16 pdaf_offsety;
 __u16 pdaf_offsetx;


 __u16 pdaf_wrapy;
 __u16 pdaf_wrapx;


 __u16 pdaf_wrapy_num;
 __u16 pdaf_wrapx_num;


 struct isp2x_dpcc_pdaf_point point[16];


 __u8 pdaf_forward_med;
} __attribute__ ((packed));

struct isp2x_hdrmge_curve {
 __u16 curve_1[17];
 __u16 curve_0[17];
} __attribute__ ((packed));

struct isp2x_hdrmge_cfg {
 __u8 mode;

 __u16 gain0_inv;
 __u16 gain0;

 __u16 gain1_inv;
 __u16 gain1;

 __u8 gain2;

 __u8 lm_dif_0p15;
 __u8 lm_dif_0p9;
 __u8 ms_diff_0p15;
 __u8 ms_dif_0p8;

 struct isp2x_hdrmge_curve curve;
 __u16 e_y[17];
} __attribute__ ((packed));

struct isp2x_rawnr_cfg {
 __u8 gauss_en;
 __u8 log_bypass;

 __u16 filtpar0;
 __u16 filtpar1;
 __u16 filtpar2;

 __u32 dgain0;
 __u32 dgain1;
 __u32 dgain2;

 __u16 luration[8];
 __u16 lulevel[8];

 __u32 gauss;
 __u16 sigma;
 __u16 pix_diff;

 __u32 thld_diff;

 __u8 gas_weig_scl2;
 __u8 gas_weig_scl1;
 __u16 thld_chanelw;

 __u16 lamda;

 __u16 fixw0;
 __u16 fixw1;
 __u16 fixw2;
 __u16 fixw3;

 __u32 wlamda0;
 __u32 wlamda1;
 __u32 wlamda2;

 __u16 rgain_filp;
 __u16 bgain_filp;
} __attribute__ ((packed));

struct isp2x_lsc_cfg {
 __u16 r_data_tbl[290];
 __u16 gr_data_tbl[290];
 __u16 gb_data_tbl[290];
 __u16 b_data_tbl[290];

 __u16 x_grad_tbl[8];
 __u16 y_grad_tbl[8];

 __u16 x_size_tbl[8];
 __u16 y_size_tbl[8];
} __attribute__ ((packed));

enum isp2x_goc_mode {
 ISP2X_GOC_MODE_LOGARITHMIC,
 ISP2X_GOC_MODE_EQUIDISTANT
};

struct isp2x_goc_cfg {
 enum isp2x_goc_mode mode;
 __u8 gamma_y[17];
} __attribute__ ((packed));

struct isp2x_hdrtmo_predict {
 __u8 global_tmo;
 __s32 iir_max;
 __s32 global_tmo_strength;

 __u8 scene_stable;
 __s32 k_rolgmean;
 __s32 iir;
} __attribute__ ((packed));

struct isp2x_hdrtmo_cfg {
 __u16 cnt_vsize;
 __u8 gain_ld_off2;
 __u8 gain_ld_off1;
 __u8 big_en;
 __u8 nobig_en;
 __u8 newhst_en;
 __u8 cnt_mode;

 __u16 expl_lgratio;
 __u8 lgscl_ratio;
 __u8 cfg_alpha;

 __u16 set_gainoff;
 __u16 set_palpha;

 __u16 set_lgmax;
 __u16 set_lgmin;

 __u8 set_weightkey;
 __u16 set_lgmean;

 __u16 set_lgrange1;
 __u16 set_lgrange0;

 __u16 set_lgavgmax;

 __u8 clipgap1_i;
 __u8 clipgap0_i;
 __u8 clipratio1;
 __u8 clipratio0;
 __u8 ratiol;

 __u16 lgscl_inv;
 __u16 lgscl;

 __u16 lgmax;

 __u16 hist_low;
 __u16 hist_min;

 __u8 hist_shift;
 __u16 hist_0p3;
 __u16 hist_high;

 __u16 palpha_lwscl;
 __u16 palpha_lw0p5;
 __u16 palpha_0p18;

 __u16 maxgain;
 __u16 maxpalpha;

 struct isp2x_hdrtmo_predict predict;
} __attribute__ ((packed));

struct isp2x_hdrtmo_stat {
 __u16 lglow;
 __u16 lgmin;
 __u16 lghigh;
 __u16 lgmax;
 __u16 weightkey;
 __u16 lgmean;
 __u16 lgrange1;
 __u16 lgrange0;
 __u16 palpha;
 __u16 lgavgmax;
 __u16 linecnt;
 __u32 min_max[32];
} __attribute__ ((packed));

struct isp2x_gic_cfg {
 __u8 edge_open;

 __u16 regmingradthrdark2;
 __u16 regmingradthrdark1;
 __u16 regminbusythre;

 __u16 regdarkthre;
 __u16 regmaxcorvboth;
 __u16 regdarktthrehi;

 __u8 regkgrad2dark;
 __u8 regkgrad1dark;
 __u8 regstrengthglobal_fix;
 __u8 regdarkthrestep;
 __u8 regkgrad2;
 __u8 regkgrad1;
 __u8 reggbthre;

 __u16 regmaxcorv;
 __u16 regmingradthr2;
 __u16 regmingradthr1;

 __u8 gr_ratio;
 __u16 dnloscale;
 __u16 dnhiscale;
 __u8 reglumapointsstep;

 __u16 gvaluelimitlo;
 __u16 gvaluelimithi;
 __u8 fusionratiohilimt1;

 __u8 regstrength_fix;

 __u16 sigma_y[15];

 __u8 noise_cut_en;
 __u16 noise_coe_a;

 __u16 noise_coe_b;
 __u16 diff_clip;
} __attribute__ ((packed));

struct isp2x_debayer_cfg {
 __u8 filter_c_en;
 __u8 filter_g_en;

 __u8 thed1;
 __u8 thed0;
 __u8 dist_scale;
 __u8 max_ratio;
 __u8 clip_en;

 __s8 filter1_coe5;
 __s8 filter1_coe4;
 __s8 filter1_coe3;
 __s8 filter1_coe2;
 __s8 filter1_coe1;

 __s8 filter2_coe5;
 __s8 filter2_coe4;
 __s8 filter2_coe3;
 __s8 filter2_coe2;
 __s8 filter2_coe1;

 __u16 hf_offset;
 __u8 gain_offset;
 __u8 offset;

 __u8 shift_num;
 __u8 order_max;
 __u8 order_min;
} __attribute__ ((packed));

struct isp2x_ccm_cfg {
 __s16 coeff0_r;
 __s16 coeff1_r;
 __s16 coeff2_r;
 __s16 offset_r;

 __s16 coeff0_g;
 __s16 coeff1_g;
 __s16 coeff2_g;
 __s16 offset_g;

 __s16 coeff0_b;
 __s16 coeff1_b;
 __s16 coeff2_b;
 __s16 offset_b;

 __u16 coeff0_y;
 __u16 coeff1_y;
 __u16 coeff2_y;

 __u16 alp_y[17];

 __u8 bound_bit;
} __attribute__ ((packed));

struct isp2x_gammaout_cfg {
 __u8 equ_segm;
 __u16 offset;
 __u16 gamma_y[45];
} __attribute__ ((packed));

enum isp2x_wdr_mode {
 ISP2X_WDR_MODE_BLOCK,
 ISP2X_WDR_MODE_GLOBAL
};

struct isp2x_wdr_cfg {
 enum isp2x_wdr_mode mode;
 unsigned int c_wdr[48];
} __attribute__ ((packed));

struct isp2x_dhaz_cfg {
 __u8 enhance_en;
 __u8 hist_chn;
 __u8 hpara_en;
 __u8 hist_en;
 __u8 dc_en;
 __u8 big_en;
 __u8 nobig_en;

 __u8 yblk_th;
 __u8 yhist_th;
 __u8 dc_max_th;
 __u8 dc_min_th;

 __u16 wt_max;
 __u8 bright_max;
 __u8 bright_min;

 __u8 tmax_base;
 __u8 dark_th;
 __u8 air_max;
 __u8 air_min;

 __u16 tmax_max;
 __u16 tmax_off;

 __u8 hist_th_off;
 __u8 hist_gratio;

 __u16 hist_min;
 __u16 hist_k;

 __u16 enhance_value;
 __u16 hist_scale;

 __u16 iir_wt_sigma;
 __u16 iir_sigma;
 __u16 stab_fnum;

 __u16 iir_tmax_sigma;
 __u16 iir_air_sigma;

 __u16 cfg_wt;
 __u16 cfg_air;
 __u16 cfg_alpha;

 __u16 cfg_gratio;
 __u16 cfg_tmax;

 __u16 dc_weitcur;
 __u16 dc_thed;

 __u8 sw_dhaz_dc_bf_h3;
 __u8 sw_dhaz_dc_bf_h2;
 __u8 sw_dhaz_dc_bf_h1;
 __u8 sw_dhaz_dc_bf_h0;

 __u8 sw_dhaz_dc_bf_h5;
 __u8 sw_dhaz_dc_bf_h4;

 __u16 air_weitcur;
 __u16 air_thed;

 __u8 air_bf_h2;
 __u8 air_bf_h1;
 __u8 air_bf_h0;

 __u8 gaus_h2;
 __u8 gaus_h1;
 __u8 gaus_h0;

 __u8 conv_t0[6];
 __u8 conv_t1[6];
 __u8 conv_t2[6];
} __attribute__ ((packed));

struct isp2x_dhaz_stat {
 __u16 dhaz_adp_air_base;
 __u16 dhaz_adp_wt;

 __u16 dhaz_adp_gratio;
 __u16 dhaz_adp_tmax;

 __u16 h_r_iir[64];
 __u16 h_g_iir[64];
 __u16 h_b_iir[64];
} __attribute__ ((packed));

struct isp2x_cproc_cfg {
 __u8 c_out_range;
 __u8 y_in_range;
 __u8 y_out_range;
 __u8 contrast;
 __u8 brightness;
 __u8 sat;
 __u8 hue;
} __attribute__ ((packed));

struct isp2x_ie_cfg {
 __u16 effect;
 __u16 color_sel;
 __u16 eff_mat_1;
 __u16 eff_mat_2;
 __u16 eff_mat_3;
 __u16 eff_mat_4;
 __u16 eff_mat_5;
 __u16 eff_tint;
} __attribute__ ((packed));

struct isp2x_rkiesharp_cfg {
 __u8 coring_thr;
 __u8 full_range;
 __u8 switch_avg;
 __u8 yavg_thr[4];
 __u8 delta1[5];
 __u8 delta2[5];
 __u8 maxnumber[5];
 __u8 minnumber[5];
 __u8 gauss_flat_coe[9];
 __u8 gauss_noise_coe[9];
 __u8 gauss_other_coe[9];
 __u8 line1_filter_coe[6];
 __u8 line2_filter_coe[9];
 __u8 line3_filter_coe[6];
 __u16 grad_seq[4];
 __u8 sharp_factor[5];
 __u8 uv_gauss_flat_coe[15];
 __u8 uv_gauss_noise_coe[15];
 __u8 uv_gauss_other_coe[15];
 __u8 lap_mat_coe[9];
} __attribute__ ((packed));

struct isp2x_superimp_cfg {
 __u8 transparency_mode;
 __u8 ref_image;

 __u16 offset_x;
 __u16 offset_y;

 __u8 y_comp;
 __u8 cb_comp;
 __u8 cr_comp;
} __attribute__ ((packed));

struct isp2x_gamma_corr_curve {
 __u16 gamma_y[17];
} __attribute__ ((packed));

struct isp2x_gamma_curve_x_axis_pnts {
 __u32 gamma_dx0;
 __u32 gamma_dx1;
} __attribute__ ((packed));

struct isp2x_sdg_cfg {
 struct isp2x_gamma_corr_curve curve_r;
 struct isp2x_gamma_corr_curve curve_g;
 struct isp2x_gamma_corr_curve curve_b;
 struct isp2x_gamma_curve_x_axis_pnts xa_pnts;
} __attribute__ ((packed));

struct isp2x_bdm_config {
 unsigned char demosaic_th;
} __attribute__ ((packed));

struct isp2x_gain_cfg {
 __u8 dhaz_en;
 __u8 wdr_en;
 __u8 tmo_en;
 __u8 lsc_en;
 __u8 mge_en;

 __u32 mge_gain[3];
 __u16 idx[15];
 __u16 lut[17];
} __attribute__ ((packed));

struct isp2x_3dlut_cfg {
 __u8 bypass_en;
 __u32 actual_size;
 __u16 lut_r[729];
 __u16 lut_g[729];
 __u16 lut_b[729];
} __attribute__ ((packed));

enum isp2x_ldch_buf_stat {
 LDCH_BUF_INIT = 0,
 LDCH_BUF_WAIT2CHIP,
 LDCH_BUF_CHIPINUSE,
};

struct rkisp_ldchbuf_info {
 __s32 buf_fd[2];
 __u32 buf_size[2];
} __attribute__ ((packed));

struct rkisp_ldchbuf_size {
 __u32 meas_width;
 __u32 meas_height;
} __attribute__ ((packed));

struct isp2x_ldch_head {
 enum isp2x_ldch_buf_stat stat;
 __u32 data_oft;
} __attribute__ ((packed));

struct isp2x_ldch_cfg {
 __u32 hsize;
 __u32 vsize;
 __s32 buf_fd;
} __attribute__ ((packed));

struct isp2x_awb_gain_cfg {
 __u16 gain_red;
 __u16 gain_green_r;
 __u16 gain_blue;
 __u16 gain_green_b;
} __attribute__ ((packed));

struct isp2x_siawb_meas_cfg {
 struct isp2x_window awb_wnd;
 __u8 awb_mode;
 __u8 max_y;
 __u8 min_y;
 __u8 max_csum;
 __u8 min_c;
 __u8 frames;
 __u8 awb_ref_cr;
 __u8 awb_ref_cb;
 __u8 enable_ymax_cmp;
} __attribute__ ((packed));

struct isp2x_rawawb_meas_cfg {
 __u8 rawawb_sel;
 __u8 sw_rawawb_light_num;
 __u8 sw_rawawb_wind_size;
 __u8 sw_rawawb_c_range;
 __u8 sw_rawawb_y_range;
 __u8 sw_rawawb_3dyuv_ls_idx3;
 __u8 sw_rawawb_3dyuv_ls_idx2;
 __u8 sw_rawawb_3dyuv_ls_idx1;
 __u8 sw_rawawb_3dyuv_ls_idx0;
 __u8 sw_rawawb_xy_en;
 __u8 sw_rawawb_uv_en;
 __u8 sw_rawlsc_bypass_en;
 __u8 sw_rawawb_blk_measure_mode;
 __u8 sw_rawawb_store_wp_flag_ls_idx2;
 __u8 sw_rawawb_store_wp_flag_ls_idx1;
 __u8 sw_rawawb_store_wp_flag_ls_idx0;
 __u16 sw_rawawb_store_wp_th0;
 __u16 sw_rawawb_store_wp_th1;
 __u16 sw_rawawb_store_wp_th2;
 __u16 sw_rawawb_v_offs;
 __u16 sw_rawawb_h_offs;
 __u16 sw_rawawb_v_size;
 __u16 sw_rawawb_h_size;
 __u16 sw_rawawb_g_max;
 __u16 sw_rawawb_r_max;
 __u16 sw_rawawb_y_max;
 __u16 sw_rawawb_b_max;
 __u16 sw_rawawb_g_min;
 __u16 sw_rawawb_r_min;
 __u16 sw_rawawb_y_min;
 __u16 sw_rawawb_b_min;
 __u16 sw_rawawb_coeff_y_g;
 __u16 sw_rawawb_coeff_y_r;
 __u16 sw_rawawb_coeff_y_b;
 __u16 sw_rawawb_coeff_u_g;
 __u16 sw_rawawb_coeff_u_r;
 __u16 sw_rawawb_coeff_u_b;
 __u16 sw_rawawb_coeff_v_g;
 __u16 sw_rawawb_coeff_v_r;
 __u16 sw_rawawb_coeff_v_b;
 __u16 sw_rawawb_vertex0_v_0;
 __u16 sw_rawawb_vertex0_u_0;
 __u16 sw_rawawb_vertex1_v_0;
 __u16 sw_rawawb_vertex1_u_0;
 __u16 sw_rawawb_vertex2_v_0;
 __u16 sw_rawawb_vertex2_u_0;
 __u16 sw_rawawb_vertex3_v_0;
 __u16 sw_rawawb_vertex3_u_0;
 __u32 sw_rawawb_islope01_0;
 __u32 sw_rawawb_islope12_0;
 __u32 sw_rawawb_islope23_0;
 __u32 sw_rawawb_islope30_0;
 __u16 sw_rawawb_vertex0_v_1;
 __u16 sw_rawawb_vertex0_u_1;
 __u16 sw_rawawb_vertex1_v_1;
 __u16 sw_rawawb_vertex1_u_1;
 __u16 sw_rawawb_vertex2_v_1;
 __u16 sw_rawawb_vertex2_u_1;
 __u16 sw_rawawb_vertex3_v_1;
 __u16 sw_rawawb_vertex3_u_1;
 __u32 sw_rawawb_islope01_1;
 __u32 sw_rawawb_islope12_1;
 __u32 sw_rawawb_islope23_1;
 __u32 sw_rawawb_islope30_1;
 __u16 sw_rawawb_vertex0_v_2;
 __u16 sw_rawawb_vertex0_u_2;
 __u16 sw_rawawb_vertex1_v_2;
 __u16 sw_rawawb_vertex1_u_2;
 __u16 sw_rawawb_vertex2_v_2;
 __u16 sw_rawawb_vertex2_u_2;
 __u16 sw_rawawb_vertex3_v_2;
 __u16 sw_rawawb_vertex3_u_2;
 __u32 sw_rawawb_islope01_2;
 __u32 sw_rawawb_islope12_2;
 __u32 sw_rawawb_islope23_2;
 __u32 sw_rawawb_islope30_2;
 __u16 sw_rawawb_vertex0_v_3;
 __u16 sw_rawawb_vertex0_u_3;
 __u16 sw_rawawb_vertex1_v_3;
 __u16 sw_rawawb_vertex1_u_3;
 __u16 sw_rawawb_vertex2_v_3;
 __u16 sw_rawawb_vertex2_u_3;
 __u16 sw_rawawb_vertex3_v_3;
 __u16 sw_rawawb_vertex3_u_3;
 __u32 sw_rawawb_islope01_3;
 __u32 sw_rawawb_islope12_3;
 __u32 sw_rawawb_islope23_3;
 __u32 sw_rawawb_islope30_3;
 __u16 sw_rawawb_vertex0_v_4;
 __u16 sw_rawawb_vertex0_u_4;
 __u16 sw_rawawb_vertex1_v_4;
 __u16 sw_rawawb_vertex1_u_4;
 __u16 sw_rawawb_vertex2_v_4;
 __u16 sw_rawawb_vertex2_u_4;
 __u16 sw_rawawb_vertex3_v_4;
 __u16 sw_rawawb_vertex3_u_4;
 __u32 sw_rawawb_islope01_4;
 __u32 sw_rawawb_islope12_4;
 __u32 sw_rawawb_islope23_4;
 __u32 sw_rawawb_islope30_4;
 __u16 sw_rawawb_vertex0_v_5;
 __u16 sw_rawawb_vertex0_u_5;
 __u16 sw_rawawb_vertex1_v_5;
 __u16 sw_rawawb_vertex1_u_5;
 __u16 sw_rawawb_vertex2_v_5;
 __u16 sw_rawawb_vertex2_u_5;
 __u16 sw_rawawb_vertex3_v_5;
 __u16 sw_rawawb_vertex3_u_5;
 __u32 sw_rawawb_islope01_5;
 __u32 sw_rawawb_islope12_5;
 __u32 sw_rawawb_islope23_5;
 __u32 sw_rawawb_islope30_5;
 __u16 sw_rawawb_vertex0_v_6;
 __u16 sw_rawawb_vertex0_u_6;
 __u16 sw_rawawb_vertex1_v_6;
 __u16 sw_rawawb_vertex1_u_6;
 __u16 sw_rawawb_vertex2_v_6;
 __u16 sw_rawawb_vertex2_u_6;
 __u16 sw_rawawb_vertex3_v_6;
 __u16 sw_rawawb_vertex3_u_6;
 __u32 sw_rawawb_islope01_6;
 __u32 sw_rawawb_islope12_6;
 __u32 sw_rawawb_islope23_6;
 __u32 sw_rawawb_islope30_6;
 __u32 sw_rawawb_b_uv_0;
 __u32 sw_rawawb_slope_vtcuv_0;
 __u32 sw_rawawb_inv_dslope_0;
 __u32 sw_rawawb_slope_ydis_0;
 __u32 sw_rawawb_b_ydis_0;
 __u32 sw_rawawb_b_uv_1;
 __u32 sw_rawawb_slope_vtcuv_1;
 __u32 sw_rawawb_inv_dslope_1;
 __u32 sw_rawawb_slope_ydis_1;
 __u32 sw_rawawb_b_ydis_1;
 __u32 sw_rawawb_b_uv_2;
 __u32 sw_rawawb_slope_vtcuv_2;
 __u32 sw_rawawb_inv_dslope_2;
 __u32 sw_rawawb_slope_ydis_2;
 __u32 sw_rawawb_b_ydis_2;
 __u32 sw_rawawb_b_uv_3;
 __u32 sw_rawawb_slope_vtcuv_3;
 __u32 sw_rawawb_inv_dslope_3;
 __u32 sw_rawawb_slope_ydis_3;
 __u32 sw_rawawb_b_ydis_3;
 __u32 sw_rawawb_ref_u;
 __u8 sw_rawawb_ref_v_3;
 __u8 sw_rawawb_ref_v_2;
 __u8 sw_rawawb_ref_v_1;
 __u8 sw_rawawb_ref_v_0;
 __u16 sw_rawawb_dis1_0;
 __u16 sw_rawawb_dis0_0;
 __u16 sw_rawawb_dis3_0;
 __u16 sw_rawawb_dis2_0;
 __u16 sw_rawawb_dis5_0;
 __u16 sw_rawawb_dis4_0;
 __u8 sw_rawawb_th3_0;
 __u8 sw_rawawb_th2_0;
 __u8 sw_rawawb_th1_0;
 __u8 sw_rawawb_th0_0;
 __u8 sw_rawawb_th5_0;
 __u8 sw_rawawb_th4_0;
 __u16 sw_rawawb_dis1_1;
 __u16 sw_rawawb_dis0_1;
 __u16 sw_rawawb_dis3_1;
 __u16 sw_rawawb_dis2_1;
 __u16 sw_rawawb_dis5_1;
 __u16 sw_rawawb_dis4_1;
 __u8 sw_rawawb_th3_1;
 __u8 sw_rawawb_th2_1;
 __u8 sw_rawawb_th1_1;
 __u8 sw_rawawb_th0_1;
 __u8 sw_rawawb_th5_1;
 __u8 sw_rawawb_th4_1;
 __u16 sw_rawawb_dis1_2;
 __u16 sw_rawawb_dis0_2;
 __u16 sw_rawawb_dis3_2;
 __u16 sw_rawawb_dis2_2;
 __u16 sw_rawawb_dis5_2;
 __u16 sw_rawawb_dis4_2;
 __u8 sw_rawawb_th3_2;
 __u8 sw_rawawb_th2_2;
 __u8 sw_rawawb_th1_2;
 __u8 sw_rawawb_th0_2;
 __u8 sw_rawawb_th5_2;
 __u8 sw_rawawb_th4_2;
 __u16 sw_rawawb_dis1_3;
 __u16 sw_rawawb_dis0_3;
 __u16 sw_rawawb_dis3_3;
 __u16 sw_rawawb_dis2_3;
 __u16 sw_rawawb_dis5_3;
 __u16 sw_rawawb_dis4_3;
 __u8 sw_rawawb_th3_3;
 __u8 sw_rawawb_th2_3;
 __u8 sw_rawawb_th1_3;
 __u8 sw_rawawb_th0_3;
 __u8 sw_rawawb_th5_3;
 __u8 sw_rawawb_th4_3;
 __u16 sw_rawawb_wt1;
 __u16 sw_rawawb_wt0;
 __u16 sw_rawawb_wt2;
 __u16 sw_rawawb_mat0_y;
 __u16 sw_rawawb_mat0_x;
 __u16 sw_rawawb_mat1_y;
 __u16 sw_rawawb_mat1_x;
 __u16 sw_rawawb_mat2_y;
 __u16 sw_rawawb_mat2_x;
 __u16 sw_rawawb_nor_x1_0;
 __u16 sw_rawawb_nor_x0_0;
 __u16 sw_rawawb_nor_y1_0;
 __u16 sw_rawawb_nor_y0_0;
 __u16 sw_rawawb_big_x1_0;
 __u16 sw_rawawb_big_x0_0;
 __u16 sw_rawawb_big_y1_0;
 __u16 sw_rawawb_big_y0_0;
 __u16 sw_rawawb_sma_x1_0;
 __u16 sw_rawawb_sma_x0_0;
 __u16 sw_rawawb_sma_y1_0;
 __u16 sw_rawawb_sma_y0_0;
 __u16 sw_rawawb_nor_x1_1;
 __u16 sw_rawawb_nor_x0_1;
 __u16 sw_rawawb_nor_y1_1;
 __u16 sw_rawawb_nor_y0_1;
 __u16 sw_rawawb_big_x1_1;
 __u16 sw_rawawb_big_x0_1;
 __u16 sw_rawawb_big_y1_1;
 __u16 sw_rawawb_big_y0_1;
 __u16 sw_rawawb_sma_x1_1;
 __u16 sw_rawawb_sma_x0_1;
 __u16 sw_rawawb_sma_y1_1;
 __u16 sw_rawawb_sma_y0_1;
 __u16 sw_rawawb_nor_x1_2;
 __u16 sw_rawawb_nor_x0_2;
 __u16 sw_rawawb_nor_y1_2;
 __u16 sw_rawawb_nor_y0_2;
 __u16 sw_rawawb_big_x1_2;
 __u16 sw_rawawb_big_x0_2;
 __u16 sw_rawawb_big_y1_2;
 __u16 sw_rawawb_big_y0_2;
 __u16 sw_rawawb_sma_x1_2;
 __u16 sw_rawawb_sma_x0_2;
 __u16 sw_rawawb_sma_y1_2;
 __u16 sw_rawawb_sma_y0_2;
 __u16 sw_rawawb_nor_x1_3;
 __u16 sw_rawawb_nor_x0_3;
 __u16 sw_rawawb_nor_y1_3;
 __u16 sw_rawawb_nor_y0_3;
 __u16 sw_rawawb_big_x1_3;
 __u16 sw_rawawb_big_x0_3;
 __u16 sw_rawawb_big_y1_3;
 __u16 sw_rawawb_big_y0_3;
 __u16 sw_rawawb_sma_x1_3;
 __u16 sw_rawawb_sma_x0_3;
 __u16 sw_rawawb_sma_y1_3;
 __u16 sw_rawawb_sma_y0_3;
 __u16 sw_rawawb_nor_x1_4;
 __u16 sw_rawawb_nor_x0_4;
 __u16 sw_rawawb_nor_y1_4;
 __u16 sw_rawawb_nor_y0_4;
 __u16 sw_rawawb_big_x1_4;
 __u16 sw_rawawb_big_x0_4;
 __u16 sw_rawawb_big_y1_4;
 __u16 sw_rawawb_big_y0_4;
 __u16 sw_rawawb_sma_x1_4;
 __u16 sw_rawawb_sma_x0_4;
 __u16 sw_rawawb_sma_y1_4;
 __u16 sw_rawawb_sma_y0_4;
 __u16 sw_rawawb_nor_x1_5;
 __u16 sw_rawawb_nor_x0_5;
 __u16 sw_rawawb_nor_y1_5;
 __u16 sw_rawawb_nor_y0_5;
 __u16 sw_rawawb_big_x1_5;
 __u16 sw_rawawb_big_x0_5;
 __u16 sw_rawawb_big_y1_5;
 __u16 sw_rawawb_big_y0_5;
 __u16 sw_rawawb_sma_x1_5;
 __u16 sw_rawawb_sma_x0_5;
 __u16 sw_rawawb_sma_y1_5;
 __u16 sw_rawawb_sma_y0_5;
 __u16 sw_rawawb_nor_x1_6;
 __u16 sw_rawawb_nor_x0_6;
 __u16 sw_rawawb_nor_y1_6;
 __u16 sw_rawawb_nor_y0_6;
 __u16 sw_rawawb_big_x1_6;
 __u16 sw_rawawb_big_x0_6;
 __u16 sw_rawawb_big_y1_6;
 __u16 sw_rawawb_big_y0_6;
 __u16 sw_rawawb_sma_x1_6;
 __u16 sw_rawawb_sma_x0_6;
 __u16 sw_rawawb_sma_y1_6;
 __u16 sw_rawawb_sma_y0_6;
 __u8 sw_rawawb_multiwindow_en;
 __u8 sw_rawawb_exc_wp_region6_domain;
 __u8 sw_rawawb_exc_wp_region6_measen;
 __u8 sw_rawawb_exc_wp_region6_excen;
 __u8 sw_rawawb_exc_wp_region5_domain;
 __u8 sw_rawawb_exc_wp_region5_measen;
 __u8 sw_rawawb_exc_wp_region5_excen;
 __u8 sw_rawawb_exc_wp_region4_domain;
 __u8 sw_rawawb_exc_wp_region4_measen;
 __u8 sw_rawawb_exc_wp_region4_excen;
 __u8 sw_rawawb_exc_wp_region3_domain;
 __u8 sw_rawawb_exc_wp_region3_measen;
 __u8 sw_rawawb_exc_wp_region3_excen;
 __u8 sw_rawawb_exc_wp_region2_domain;
 __u8 sw_rawawb_exc_wp_region2_measen;
 __u8 sw_rawawb_exc_wp_region2_excen;
 __u8 sw_rawawb_exc_wp_region1_domain;
 __u8 sw_rawawb_exc_wp_region1_measen;
 __u8 sw_rawawb_exc_wp_region1_excen;
 __u8 sw_rawawb_exc_wp_region0_domain;
 __u8 sw_rawawb_exc_wp_region0_measen;
 __u8 sw_rawawb_exc_wp_region0_excen;
 __u16 sw_rawawb_multiwindow0_v_offs;
 __u16 sw_rawawb_multiwindow0_h_offs;
 __u16 sw_rawawb_multiwindow0_v_size;
 __u16 sw_rawawb_multiwindow0_h_size;
 __u16 sw_rawawb_multiwindow1_v_offs;
 __u16 sw_rawawb_multiwindow1_h_offs;
 __u16 sw_rawawb_multiwindow1_v_size;
 __u16 sw_rawawb_multiwindow1_h_size;
 __u16 sw_rawawb_multiwindow2_v_offs;
 __u16 sw_rawawb_multiwindow2_h_offs;
 __u16 sw_rawawb_multiwindow2_v_size;
 __u16 sw_rawawb_multiwindow2_h_size;
 __u16 sw_rawawb_multiwindow3_v_offs;
 __u16 sw_rawawb_multiwindow3_h_offs;
 __u16 sw_rawawb_multiwindow3_v_size;
 __u16 sw_rawawb_multiwindow3_h_size;
 __u16 sw_rawawb_multiwindow4_v_offs;
 __u16 sw_rawawb_multiwindow4_h_offs;
 __u16 sw_rawawb_multiwindow4_v_size;
 __u16 sw_rawawb_multiwindow4_h_size;
 __u16 sw_rawawb_multiwindow5_v_offs;
 __u16 sw_rawawb_multiwindow5_h_offs;
 __u16 sw_rawawb_multiwindow5_v_size;
 __u16 sw_rawawb_multiwindow5_h_size;
 __u16 sw_rawawb_multiwindow6_v_offs;
 __u16 sw_rawawb_multiwindow6_h_offs;
 __u16 sw_rawawb_multiwindow6_v_size;
 __u16 sw_rawawb_multiwindow6_h_size;
 __u16 sw_rawawb_multiwindow7_v_offs;
 __u16 sw_rawawb_multiwindow7_h_offs;
 __u16 sw_rawawb_multiwindow7_v_size;
 __u16 sw_rawawb_multiwindow7_h_size;
 __u16 sw_rawawb_exc_wp_region0_xu1;
 __u16 sw_rawawb_exc_wp_region0_xu0;
 __u16 sw_rawawb_exc_wp_region0_yv1;
 __u16 sw_rawawb_exc_wp_region0_yv0;
 __u16 sw_rawawb_exc_wp_region1_xu1;
 __u16 sw_rawawb_exc_wp_region1_xu0;
 __u16 sw_rawawb_exc_wp_region1_yv1;
 __u16 sw_rawawb_exc_wp_region1_yv0;
 __u16 sw_rawawb_exc_wp_region2_xu1;
 __u16 sw_rawawb_exc_wp_region2_xu0;
 __u16 sw_rawawb_exc_wp_region2_yv1;
 __u16 sw_rawawb_exc_wp_region2_yv0;
 __u16 sw_rawawb_exc_wp_region3_xu1;
 __u16 sw_rawawb_exc_wp_region3_xu0;
 __u16 sw_rawawb_exc_wp_region3_yv1;
 __u16 sw_rawawb_exc_wp_region3_yv0;
 __u16 sw_rawawb_exc_wp_region4_xu1;
 __u16 sw_rawawb_exc_wp_region4_xu0;
 __u16 sw_rawawb_exc_wp_region4_yv1;
 __u16 sw_rawawb_exc_wp_region4_yv0;
 __u16 sw_rawawb_exc_wp_region5_xu1;
 __u16 sw_rawawb_exc_wp_region5_xu0;
 __u16 sw_rawawb_exc_wp_region5_yv1;
 __u16 sw_rawawb_exc_wp_region5_yv0;
 __u16 sw_rawawb_exc_wp_region6_xu1;
 __u16 sw_rawawb_exc_wp_region6_xu0;
 __u16 sw_rawawb_exc_wp_region6_yv1;
 __u16 sw_rawawb_exc_wp_region6_yv0;
} __attribute__ ((packed));

struct isp2x_rawaebig_meas_cfg {
 __u8 rawae_sel;
 __u8 wnd_num;
 __u8 subwin_en[4];
 struct isp2x_window win;
 struct isp2x_window subwin[4];
} __attribute__ ((packed));

struct isp2x_rawaelite_meas_cfg {
 __u8 rawae_sel;
 __u8 wnd_num;
 struct isp2x_window win;
} __attribute__ ((packed));

struct isp2x_yuvae_meas_cfg {
 __u8 ysel;
 __u8 wnd_num;
 __u8 subwin_en[4];
 struct isp2x_window win;
 struct isp2x_window subwin[4];
} __attribute__ ((packed));

struct isp2x_rawaf_meas_cfg {
 __u8 rawaf_sel;
 __u8 num_afm_win;
 __u8 gaus_en;
 __u8 gamma_en;
 struct isp2x_window win[2];
 __u8 line_en[5];
 __u8 line_num[5];
 __u8 gaus_coe_h2;
 __u8 gaus_coe_h1;
 __u8 gaus_coe_h0;
 __u16 afm_thres;
 __u8 lum_var_shift[2];
 __u8 afm_var_shift[2];
 __u16 gamma_y[17];
} __attribute__ ((packed));

struct isp2x_siaf_win_cfg {
 __u8 sum_shift;
 __u8 lum_shift;
 struct isp2x_window win;
} __attribute__ ((packed));

struct isp2x_siaf_cfg {
 __u8 num_afm_win;
 __u32 thres;
 struct isp2x_siaf_win_cfg afm_win[3];
} __attribute__ ((packed));

struct isp2x_rawhistbig_cfg {
 __u8 wnd_num;
 __u8 data_sel;
 __u8 waterline;
 __u8 mode;
 __u8 stepsize;
 __u8 off;
 __u8 bcc;
 __u8 gcc;
 __u8 rcc;
 struct isp2x_window win;
 __u8 weight[225];
} __attribute__ ((packed));

struct isp2x_rawhistlite_cfg {
 __u8 data_sel;
 __u8 waterline;
 __u8 mode;
 __u8 stepsize;
 __u8 off;
 __u8 bcc;
 __u8 gcc;
 __u8 rcc;
 struct isp2x_window win;
 __u8 weight[25];
} __attribute__ ((packed));

struct isp2x_sihst_win_cfg {
 __u8 data_sel;
 __u8 waterline;
 __u8 auto_stop;
 __u8 mode;
 __u8 stepsize;
 struct isp2x_window win;
} __attribute__ ((packed));

struct isp2x_sihst_cfg {
 __u8 wnd_num;
 struct isp2x_sihst_win_cfg win_cfg[1];
 __u8 hist_weight[225];
} __attribute__ ((packed));

struct isp2x_isp_other_cfg {
 struct isp2x_bls_cfg bls_cfg;
 struct isp2x_dpcc_cfg dpcc_cfg;
 struct isp2x_hdrmge_cfg hdrmge_cfg;
 struct isp2x_rawnr_cfg rawnr_cfg;
 struct isp2x_lsc_cfg lsc_cfg;
 struct isp2x_awb_gain_cfg awb_gain_cfg;

 struct isp2x_gic_cfg gic_cfg;
 struct isp2x_debayer_cfg debayer_cfg;
 struct isp2x_ccm_cfg ccm_cfg;
 struct isp2x_gammaout_cfg gammaout_cfg;
 struct isp2x_wdr_cfg wdr_cfg;
 struct isp2x_cproc_cfg cproc_cfg;
 struct isp2x_ie_cfg ie_cfg;
 struct isp2x_rkiesharp_cfg rkiesharp_cfg;
 struct isp2x_superimp_cfg superimp_cfg;
 struct isp2x_sdg_cfg sdg_cfg;
 struct isp2x_bdm_config bdm_cfg;
 struct isp2x_hdrtmo_cfg hdrtmo_cfg;
 struct isp2x_dhaz_cfg dhaz_cfg;
 struct isp2x_gain_cfg gain_cfg;
 struct isp2x_3dlut_cfg isp3dlut_cfg;
 struct isp2x_ldch_cfg ldch_cfg;
} __attribute__ ((packed));

struct isp2x_isp_meas_cfg {
 struct isp2x_siawb_meas_cfg siawb;
 struct isp2x_rawawb_meas_cfg rawawb;
 struct isp2x_rawaelite_meas_cfg rawae0;
 struct isp2x_rawaebig_meas_cfg rawae1;
 struct isp2x_rawaebig_meas_cfg rawae2;
 struct isp2x_rawaebig_meas_cfg rawae3;
 struct isp2x_yuvae_meas_cfg yuvae;
 struct isp2x_rawaf_meas_cfg rawaf;
 struct isp2x_siaf_cfg siaf;
 struct isp2x_rawhistlite_cfg rawhist0;
 struct isp2x_rawhistbig_cfg rawhist1;
 struct isp2x_rawhistbig_cfg rawhist2;
 struct isp2x_rawhistbig_cfg rawhist3;
 struct isp2x_sihst_cfg sihst;
} __attribute__ ((packed));

struct sensor_exposure_s {
 __u32 fine_integration_time;
 __u32 coarse_integration_time;
 __u32 analog_gain_code_global;
 __u32 digital_gain_global;
 __u32 isp_digital_gain;
 __u32 rolling_shutter_skew;
} __attribute__ ((packed));

struct sensor_exposure_cfg {
 struct sensor_exposure_s linear_exp;
 struct sensor_exposure_s hdr_exp[3];
} __attribute__ ((packed));

struct isp2x_isp_params_cfg {
 __u64 module_en_update;
 __u64 module_ens;
 __u64 module_cfg_update;

 __u32 frame_id;
 struct isp2x_isp_meas_cfg meas;
 struct isp2x_isp_other_cfg others;
 struct sensor_exposure_cfg exposure;
} __attribute__ ((packed));

struct isp2x_siawb_meas {
 __u32 cnt;
 __u8 mean_y_or_g;
 __u8 mean_cb_or_b;
 __u8 mean_cr_or_r;
} __attribute__ ((packed));

struct isp2x_siawb_stat {
 struct isp2x_siawb_meas awb_mean[1];
} __attribute__ ((packed));

struct isp2x_rawawb_ramdata {
 __u32 wp;
 __u32 r;
 __u32 g;
 __u32 b;
};

struct isp2x_rawawb_meas_stat {
 __u32 ro_rawawb_sum_r_nor[7];
 __u32 ro_rawawb_sum_g_nor[7];
 __u32 ro_rawawb_sum_b_nor[7];
 __u32 ro_rawawb_wp_num_nor[7];
 __u32 ro_rawawb_sum_r_big[7];
 __u32 ro_rawawb_sum_g_big[7];
 __u32 ro_rawawb_sum_b_big[7];
 __u32 ro_rawawb_wp_num_big[7];
 __u32 ro_rawawb_sum_r_sma[7];
 __u32 ro_rawawb_sum_g_sma[7];
 __u32 ro_rawawb_sum_b_sma[7];
 __u32 ro_rawawb_wp_num_sma[7];
 __u32 ro_sum_r_nor_multiwindow[8];
 __u32 ro_sum_g_nor_multiwindow[8];
 __u32 ro_sum_b_nor_multiwindow[8];
 __u32 ro_wp_nm_nor_multiwindow[8];
 __u32 ro_sum_r_big_multiwindow[8];
 __u32 ro_sum_g_big_multiwindow[8];
 __u32 ro_sum_b_big_multiwindow[8];
 __u32 ro_wp_nm_big_multiwindow[8];
 __u32 ro_sum_r_sma_multiwindow[8];
 __u32 ro_sum_g_sma_multiwindow[8];
 __u32 ro_sum_b_sma_multiwindow[8];
 __u32 ro_wp_nm_sma_multiwindow[8];
 __u32 ro_sum_r_exc[7];
 __u32 ro_sum_g_exc[7];
 __u32 ro_sum_b_exc[7];
 __u32 ro_wp_nm_exc[7];
 struct isp2x_rawawb_ramdata ramdata[225];
} __attribute__ ((packed));

struct isp2x_rawae_meas_data {
 __u16 channelr_xy;
 __u16 channelb_xy;
 __u16 channelg_xy;
};

struct isp2x_rawaebig_stat {
 __u32 sumr[4];
 __u32 sumg[4];
 __u32 sumb[4];
 struct isp2x_rawae_meas_data data[225];
} __attribute__ ((packed));

struct isp2x_rawaelite_stat {
 struct isp2x_rawae_meas_data data[25];
} __attribute__ ((packed));

struct isp2x_yuvae_stat {
 __u32 ro_yuvae_sumy[4];
 __u8 mean[225];
} __attribute__ ((packed));

struct isp2x_rawaf_stat {
 __u32 int_state;
 __u32 afm_sum[2];
 __u32 afm_lum[2];
 __u32 ramdata[225];
} __attribute__ ((packed));

struct isp2x_siaf_meas_val {
 __u32 sum;
 __u32 lum;
} __attribute__ ((packed));

struct isp2x_siaf_stat {
 struct isp2x_siaf_meas_val win[3];
} __attribute__ ((packed));

struct isp2x_rawhistbig_stat {
 __u32 hist_bin[256];
} __attribute__ ((packed));

struct isp2x_rawhistlite_stat {
 __u32 hist_bin[256];
} __attribute__ ((packed));

struct isp2x_sihst_win_stat {
 __u32 hist_bins[32];
} __attribute__ ((packed));

struct isp2x_sihst_stat {
 struct isp2x_sihst_win_stat win_stat[1];
} __attribute__ ((packed));

struct isp2x_stat {
 struct isp2x_siawb_stat siawb;
 struct isp2x_rawawb_meas_stat rawawb;
 struct isp2x_rawaelite_stat rawae0;
 struct isp2x_rawaebig_stat rawae1;
 struct isp2x_rawaebig_stat rawae2;
 struct isp2x_rawaebig_stat rawae3;
 struct isp2x_yuvae_stat yuvae;
 struct isp2x_rawaf_stat rawaf;
 struct isp2x_siaf_stat siaf;
 struct isp2x_rawhistlite_stat rawhist0;
 struct isp2x_rawhistbig_stat rawhist1;
 struct isp2x_rawhistbig_stat rawhist2;
 struct isp2x_rawhistbig_stat rawhist3;
 struct isp2x_sihst_stat sihst;

 struct isp2x_bls_stat bls;
 struct isp2x_hdrtmo_stat hdrtmo;
 struct isp2x_dhaz_stat dhaz;
} __attribute__ ((packed));

struct rkisp_isp2x_stat_buffer {
 unsigned int meas_type;
 unsigned int frame_id;
 struct isp2x_stat params;
} __attribute__ ((packed));

struct rkisp_mipi_luma {
 unsigned int exp_mean[16];
} __attribute__ ((packed));

struct rkisp_isp2x_luma_buffer {
 unsigned int meas_type;
 unsigned int frame_id;
 struct rkisp_mipi_luma luma[3];
} __attribute__ ((packed));

enum {
 RKISP_RTT_MODE_NORMAL = 0,
 RKISP_RTT_MODE_MULTI_FRAME,
 RKISP_RTT_MODE_ONE_FRAME,
};






struct rkisp_thunderboot_resmem_head {
 __u16 enable;
 __u16 complete;
 __u16 frm_total;
 __u16 hdr_mode;
 __u16 rtt_mode;
 __u16 width;
 __u16 height;
 __u16 camera_num;
 __u16 camera_index;
 __u16 md_flag;

 __u32 exp_time[3];
 __u32 exp_gain[3];
 __u32 exp_time_reg[3];
 __u32 exp_gain_reg[3];
 __u32 exp_isp_dgain[3];
 __u32 dcg_mode[3];
 __u32 nr_buf_size;
 __u32 share_mem_size;
 __u32 pre_buf_num;
 __u32 pre_buf_addr[(4)];
 __u32 pre_buf_timestamp[(4)];
} __attribute__ ((packed));




struct rkisp_thunderboot_resmem {
 __u32 resmem_padr;
 __u32 resmem_size;
} __attribute__ ((packed));




struct rkisp_thunderboot_shmem {
 __u32 shm_start;
 __u32 shm_size;
 __s32 shm_fd;
} __attribute__ ((packed));

struct rkisp_frame_info {
 __u64 timestamp;
 __u32 seq;
 __u32 hdr;
 __u32 rolling_shutter_skew;

 __u32 sensor_exposure_time;
 __u32 sensor_analog_gain;
 __u32 sensor_digital_gain;
 __u32 isp_digital_gain;

 __u32 sensor_exposure_time_m;
 __u32 sensor_analog_gain_m;
 __u32 sensor_digital_gain_m;
 __u32 isp_digital_gain_m;

 __u32 sensor_exposure_time_l;
 __u32 sensor_analog_gain_l;
 __u32 sensor_digital_gain_l;
 __u32 isp_digital_gain_l;

 __u32 isp_reg[6144];
} __attribute__ ((packed));


struct isp21_cgc_cfg {
 __u8 yuv_limit;
 __u8 ratio_en;
} __attribute__ ((packed));

struct isp21_csm_cfg {
 __u8 csm_full_range;
 __u16 csm_y_offset;
 __u16 csm_c_offset;

 __u32 csm_coeff[9];
} __attribute__ ((packed));

struct isp21_bls_cfg {
 __u8 enable_auto;
 __u8 en_windows;
 __u8 bls1_en;
 struct isp2x_window bls_window1;
 struct isp2x_window bls_window2;
 __u8 bls_samples;
 struct isp2x_bls_fixed_val fixed_val;
 struct isp2x_bls_fixed_val bls1_val;
} __attribute__ ((packed));

struct isp21_awb_gain_cfg {
 __u16 gain0_red;
 __u16 gain0_green_r;
 __u16 gain0_blue;
 __u16 gain0_green_b;
 __u16 gain1_red;
 __u16 gain1_green_r;
 __u16 gain1_blue;
 __u16 gain1_green_b;
 __u16 gain2_red;
 __u16 gain2_green_r;
 __u16 gain2_blue;
 __u16 gain2_green_b;
} __attribute__ ((packed));

struct isp21_gic_cfg {
 __u16 regmingradthrdark2;
 __u16 regmingradthrdark1;
 __u16 regminbusythre;

 __u16 regdarkthre;
 __u16 regmaxcorvboth;
 __u16 regdarktthrehi;

 __u8 regkgrad2dark;
 __u8 regkgrad1dark;
 __u8 regstrengthglobal_fix;
 __u8 regdarkthrestep;
 __u8 regkgrad2;
 __u8 regkgrad1;
 __u8 reggbthre;

 __u16 regmaxcorv;
 __u16 regmingradthr2;
 __u16 regmingradthr1;

 __u8 gr_ratio;
 __u8 noise_scale;
 __u16 noise_base;
 __u16 diff_clip;

 __u16 sigma_y[15];
} __attribute__ ((packed));

struct isp21_baynr_cfg {
 __u8 sw_baynr_gauss_en;
 __u8 sw_baynr_log_bypass;
 __u16 sw_baynr_dgain1;
 __u16 sw_baynr_dgain0;
 __u16 sw_baynr_dgain2;
 __u16 sw_baynr_pix_diff;
 __u16 sw_baynr_diff_thld;
 __u16 sw_baynr_softthld;
 __u16 sw_bltflt_streng;
 __u16 sw_baynr_reg_w1;
 __u16 sw_sigma_x[16];
 __u16 sw_sigma_y[16];
 __u16 weit_d2;
 __u16 weit_d1;
 __u16 weit_d0;
} __attribute__ ((packed));

struct isp21_bay3d_cfg {
 __u8 sw_bay3d_exp_sel;
 __u8 sw_bay3d_bypass_en;
 __u8 sw_bay3d_pk_en;
 __u16 sw_bay3d_softwgt;
 __u16 sw_bay3d_sigratio;
 __u32 sw_bay3d_glbpk2;
 __u16 sw_bay3d_exp_str;
 __u16 sw_bay3d_str;
 __u16 sw_bay3d_wgtlmt_h;
 __u16 sw_bay3d_wgtlmt_l;
 __u16 sw_bay3d_sig_x[16];
 __u16 sw_bay3d_sig_y[16];
} __attribute__ ((packed));

struct isp21_ynr_cfg {
 __u8 sw_ynr_thumb_mix_cur_en;
 __u8 sw_ynr_global_gain_alpha;
 __u8 sw_ynr_global_gain;
 __u8 sw_ynr_flt1x1_bypass_sel;
 __u8 sw_ynr_sft5x5_bypass;
 __u8 sw_ynr_flt1x1_bypass;
 __u8 sw_ynr_lgft3x3_bypass;
 __u8 sw_ynr_lbft5x5_bypass;
 __u8 sw_ynr_bft3x3_bypass;

 __u16 sw_ynr_rnr_max_r;

 __u16 sw_ynr_low_bf_inv1;
 __u16 sw_ynr_low_bf_inv0;

 __u16 sw_ynr_low_peak_supress;
 __u16 sw_ynr_low_thred_adj;

 __u16 sw_ynr_low_dist_adj;
 __u16 sw_ynr_low_edge_adj_thresh;

 __u16 sw_ynr_low_bi_weight;
 __u16 sw_ynr_low_weight;
 __u16 sw_ynr_low_center_weight;
 __u16 sw_ynr_hi_min_adj;
 __u16 sw_ynr_high_thred_adj;
 __u8 sw_ynr_high_retain_weight;
 __u8 sw_ynr_hi_edge_thed;
 __u8 sw_ynr_base_filter_weight2;
 __u8 sw_ynr_base_filter_weight1;
 __u8 sw_ynr_base_filter_weight0;
 __u16 sw_ynr_low_gauss1_coeff2;
 __u16 sw_ynr_low_gauss1_coeff1;
 __u16 sw_ynr_low_gauss1_coeff0;
 __u16 sw_ynr_low_gauss2_coeff2;
 __u16 sw_ynr_low_gauss2_coeff1;
 __u16 sw_ynr_low_gauss2_coeff0;
 __u8 sw_ynr_direction_weight3;
 __u8 sw_ynr_direction_weight2;
 __u8 sw_ynr_direction_weight1;
 __u8 sw_ynr_direction_weight0;
 __u8 sw_ynr_direction_weight7;
 __u8 sw_ynr_direction_weight6;
 __u8 sw_ynr_direction_weight5;
 __u8 sw_ynr_direction_weight4;
 __u16 sw_ynr_luma_points_x[17];
 __u16 sw_ynr_lsgm_y[17];
 __u16 sw_ynr_hsgm_y[17];
 __u8 sw_ynr_rnr_strength3[17];
} __attribute__ ((packed));

struct isp21_cnr_cfg {
 __u8 sw_cnr_thumb_mix_cur_en;
 __u8 sw_cnr_lq_bila_bypass;
 __u8 sw_cnr_hq_bila_bypass;
 __u8 sw_cnr_exgain_bypass;
 __u8 sw_cnr_exgain_mux;
 __u8 sw_cnr_gain_iso;
 __u8 sw_cnr_gain_offset;
 __u8 sw_cnr_gain_1sigma;
 __u8 sw_cnr_gain_uvgain1;
 __u8 sw_cnr_gain_uvgain0;
 __u8 sw_cnr_lmed3_alpha;
 __u8 sw_cnr_lbf5_gain_y;
 __u8 sw_cnr_lbf5_gain_c;
 __u8 sw_cnr_lbf5_weit_d3;
 __u8 sw_cnr_lbf5_weit_d2;
 __u8 sw_cnr_lbf5_weit_d1;
 __u8 sw_cnr_lbf5_weit_d0;
 __u8 sw_cnr_lbf5_weit_d4;
 __u8 sw_cnr_hmed3_alpha;
 __u16 sw_cnr_hbf5_weit_src;
 __u16 sw_cnr_hbf5_min_wgt;
 __u16 sw_cnr_hbf5_sigma;
 __u16 sw_cnr_lbf5_weit_src;
 __u16 sw_cnr_lbf3_sigma;
} __attribute__ ((packed));

struct isp21_sharp_cfg {
 __u8 sw_sharp_bypass;
 __u8 sw_sharp_sharp_ratio;
 __u8 sw_sharp_bf_ratio;
 __u8 sw_sharp_gaus_ratio;
 __u8 sw_sharp_pbf_ratio;
 __u8 sw_sharp_luma_dx[7];
 __u16 sw_sharp_pbf_sigma_inv[8];
 __u16 sw_sharp_bf_sigma_inv[8];
 __u8 sw_sharp_bf_sigma_shift;
 __u8 sw_sharp_pbf_sigma_shift;
 __u16 sw_sharp_ehf_th[8];
 __u16 sw_sharp_clip_hf[8];
 __u8 sw_sharp_pbf_coef_2;
 __u8 sw_sharp_pbf_coef_1;
 __u8 sw_sharp_pbf_coef_0;
 __u8 sw_sharp_bf_coef_2;
 __u8 sw_sharp_bf_coef_1;
 __u8 sw_sharp_bf_coef_0;
 __u8 sw_sharp_gaus_coef_2;
 __u8 sw_sharp_gaus_coef_1;
 __u8 sw_sharp_gaus_coef_0;
} __attribute__ ((packed));

struct isp21_ccm_cfg {
 __u8 highy_adjust_dis;
 __u8 bound_bit;

 __s16 coeff0_r;
 __s16 coeff1_r;
 __s16 coeff2_r;
 __s16 offset_r;

 __s16 coeff0_g;
 __s16 coeff1_g;
 __s16 coeff2_g;
 __s16 offset_g;

 __s16 coeff0_b;
 __s16 coeff1_b;
 __s16 coeff2_b;
 __s16 offset_b;

 __u16 coeff0_y;
 __u16 coeff1_y;
 __u16 coeff2_y;

 __u16 alp_y[17];
} __attribute__ ((packed));

struct isp21_dhaz_cfg {
 __u8 enhance_en;
 __u8 air_lc_en;
 __u8 hpara_en;
 __u8 hist_en;
 __u8 dc_en;

 __u8 yblk_th;
 __u8 yhist_th;
 __u8 dc_max_th;
 __u8 dc_min_th;

 __u16 wt_max;
 __u8 bright_max;
 __u8 bright_min;

 __u8 tmax_base;
 __u8 dark_th;
 __u8 air_max;
 __u8 air_min;

 __u16 tmax_max;
 __u16 tmax_off;

 __u8 hist_k;
 __u8 hist_th_off;
 __u16 hist_min;

 __u16 hist_gratio;
 __u16 hist_scale;

 __u16 enhance_value;
 __u16 enhance_chroma;

 __u16 iir_wt_sigma;
 __u16 iir_sigma;
 __u16 stab_fnum;

 __u16 iir_tmax_sigma;
 __u16 iir_air_sigma;
 __u8 iir_pre_wet;

 __u16 cfg_wt;
 __u16 cfg_air;
 __u16 cfg_alpha;

 __u16 cfg_gratio;
 __u16 cfg_tmax;

 __u16 range_sima;
 __u8 space_sigma_pre;
 __u8 space_sigma_cur;

 __u16 dc_weitcur;
 __u16 bf_weight;

 __u16 enh_curve[17];

 __u8 gaus_h2;
 __u8 gaus_h1;
 __u8 gaus_h0;
} __attribute__ ((packed));

struct isp21_dhaz_stat {
 __u16 dhaz_adp_air_base;
 __u16 dhaz_adp_wt;

 __u16 dhaz_adp_gratio;
 __u16 dhaz_adp_tmax;

 __u16 h_rgb_iir[64];
} __attribute__ ((packed));

struct isp21_drc_cfg {
 __u8 sw_drc_offset_pow2;
 __u16 sw_drc_compres_scl;
 __u16 sw_drc_position;
 __u16 sw_drc_delta_scalein;
 __u16 sw_drc_hpdetail_ratio;
 __u16 sw_drc_lpdetail_ratio;
 __u8 sw_drc_weicur_pix;
 __u8 sw_drc_weipre_frame;
 __u16 sw_drc_force_sgm_inv0;
 __u8 sw_drc_motion_scl;
 __u8 sw_drc_edge_scl;
 __u16 sw_drc_space_sgm_inv1;
 __u16 sw_drc_space_sgm_inv0;
 __u16 sw_drc_range_sgm_inv1;
 __u16 sw_drc_range_sgm_inv0;
 __u8 sw_drc_weig_maxl;
 __u8 sw_drc_weig_bilat;
 __u16 sw_drc_gain_y[17];
 __u16 sw_drc_compres_y[17];
 __u16 sw_drc_scale_y[17];
 __u16 sw_drc_iir_weight;
 __u16 sw_drc_min_ogain;
} __attribute__ ((packed));

struct isp21_rawawb_meas_cfg {
 __u8 rawawb_sel;
 __u8 sw_rawawb_xy_en0;
 __u8 sw_rawawb_uv_en0;
 __u8 sw_rawawb_xy_en1;
 __u8 sw_rawawb_uv_en1;
 __u8 sw_rawawb_3dyuv_en0;
 __u8 sw_rawawb_3dyuv_en1;
 __u8 sw_rawawb_wp_blk_wei_en0;
 __u8 sw_rawawb_wp_blk_wei_en1;
 __u8 sw_rawawb_wp_luma_wei_en0;
 __u8 sw_rawawb_wp_luma_wei_en1;
 __u8 sw_rawlsc_bypass_en;
 __u8 sw_rawawb_blk_measure_enable;
 __u8 sw_rawawb_blk_measure_mode;
 __u8 sw_rawawb_blk_measure_xytype;
 __u8 sw_rawawb_blk_measure_illu_idx;
 __u8 sw_rawawb_wp_hist_xytype;
 __u8 sw_rawawb_light_num;
 __u8 sw_rawawb_wind_size;
 __u8 sw_rawawb_r_max;
 __u8 sw_rawawb_g_max;
 __u8 sw_rawawb_b_max;
 __u8 sw_rawawb_y_max;
 __u8 sw_rawawb_r_min;
 __u8 sw_rawawb_g_min;
 __u8 sw_rawawb_b_min;
 __u8 sw_rawawb_y_min;
 __u8 sw_rawawb_3dyuv_ls_idx0;
 __u8 sw_rawawb_3dyuv_ls_idx1;
 __u8 sw_rawawb_3dyuv_ls_idx2;
 __u8 sw_rawawb_3dyuv_ls_idx3;
 __u8 sw_rawawb_exc_wp_region0_excen0;
 __u8 sw_rawawb_exc_wp_region0_excen1;
 __u8 sw_rawawb_exc_wp_region0_domain;
 __u8 sw_rawawb_exc_wp_region1_excen0;
 __u8 sw_rawawb_exc_wp_region1_excen1;
 __u8 sw_rawawb_exc_wp_region1_domain;
 __u8 sw_rawawb_exc_wp_region2_excen0;
 __u8 sw_rawawb_exc_wp_region2_excen1;
 __u8 sw_rawawb_exc_wp_region2_domain;
 __u8 sw_rawawb_exc_wp_region3_excen0;
 __u8 sw_rawawb_exc_wp_region3_excen1;
 __u8 sw_rawawb_exc_wp_region3_domain;
 __u8 sw_rawawb_exc_wp_region4_excen0;
 __u8 sw_rawawb_exc_wp_region4_excen1;
 __u8 sw_rawawb_exc_wp_region4_domain;
 __u8 sw_rawawb_exc_wp_region5_excen0;
 __u8 sw_rawawb_exc_wp_region5_excen1;
 __u8 sw_rawawb_exc_wp_region5_domain;
 __u8 sw_rawawb_exc_wp_region6_excen0;
 __u8 sw_rawawb_exc_wp_region6_excen1;
 __u8 sw_rawawb_exc_wp_region6_domain;
 __u8 sw_rawawb_wp_luma_weicurve_y0;
 __u8 sw_rawawb_wp_luma_weicurve_y1;
 __u8 sw_rawawb_wp_luma_weicurve_y2;
 __u8 sw_rawawb_wp_luma_weicurve_y3;
 __u8 sw_rawawb_wp_luma_weicurve_y4;
 __u8 sw_rawawb_wp_luma_weicurve_y5;
 __u8 sw_rawawb_wp_luma_weicurve_y6;
 __u8 sw_rawawb_wp_luma_weicurve_y7;
 __u8 sw_rawawb_wp_luma_weicurve_y8;
 __u8 sw_rawawb_wp_luma_weicurve_w0;
 __u8 sw_rawawb_wp_luma_weicurve_w1;
 __u8 sw_rawawb_wp_luma_weicurve_w2;
 __u8 sw_rawawb_wp_luma_weicurve_w3;
 __u8 sw_rawawb_wp_luma_weicurve_w4;
 __u8 sw_rawawb_wp_luma_weicurve_w5;
 __u8 sw_rawawb_wp_luma_weicurve_w6;
 __u8 sw_rawawb_wp_luma_weicurve_w7;
 __u8 sw_rawawb_wp_luma_weicurve_w8;
 __u8 sw_rawawb_rotu0_ls0;
 __u8 sw_rawawb_rotu1_ls0;
 __u8 sw_rawawb_rotu2_ls0;
 __u8 sw_rawawb_rotu3_ls0;
 __u8 sw_rawawb_rotu4_ls0;
 __u8 sw_rawawb_rotu5_ls0;
 __u8 sw_rawawb_dis_x1x2_ls0;
 __u8 sw_rawawb_rotu0_ls1;
 __u8 sw_rawawb_rotu1_ls1;
 __u8 sw_rawawb_rotu2_ls1;
 __u8 sw_rawawb_rotu3_ls1;
 __u8 sw_rawawb_rotu4_ls1;
 __u8 sw_rawawb_rotu5_ls1;
 __u8 sw_rawawb_dis_x1x2_ls1;
 __u8 sw_rawawb_rotu0_ls2;
 __u8 sw_rawawb_rotu1_ls2;
 __u8 sw_rawawb_rotu2_ls2;
 __u8 sw_rawawb_rotu3_ls2;
 __u8 sw_rawawb_rotu4_ls2;
 __u8 sw_rawawb_rotu5_ls2;
 __u8 sw_rawawb_dis_x1x2_ls2;
 __u8 sw_rawawb_rotu0_ls3;
 __u8 sw_rawawb_rotu1_ls3;
 __u8 sw_rawawb_rotu2_ls3;
 __u8 sw_rawawb_rotu3_ls3;
 __u8 sw_rawawb_rotu4_ls3;
 __u8 sw_rawawb_rotu5_ls3;
 __u8 sw_rawawb_dis_x1x2_ls3;
 __u8 sw_rawawb_blk_rtdw_measure_en;
 __u8 sw_rawawb_blk_with_luma_wei_en;
 __u8 sw_rawawb_wp_blk_wei_w[225];

 __u16 sw_rawawb_h_offs;
 __u16 sw_rawawb_v_offs;
 __u16 sw_rawawb_h_size;
 __u16 sw_rawawb_v_size;
 __u16 sw_rawawb_vertex0_u_0;
 __u16 sw_rawawb_vertex0_v_0;
 __u16 sw_rawawb_vertex1_u_0;
 __u16 sw_rawawb_vertex1_v_0;
 __u16 sw_rawawb_vertex2_u_0;
 __u16 sw_rawawb_vertex2_v_0;
 __u16 sw_rawawb_vertex3_u_0;
 __u16 sw_rawawb_vertex3_v_0;
 __u16 sw_rawawb_vertex0_u_1;
 __u16 sw_rawawb_vertex0_v_1;
 __u16 sw_rawawb_vertex1_u_1;
 __u16 sw_rawawb_vertex1_v_1;
 __u16 sw_rawawb_vertex2_u_1;
 __u16 sw_rawawb_vertex2_v_1;
 __u16 sw_rawawb_vertex3_u_1;
 __u16 sw_rawawb_vertex3_v_1;
 __u16 sw_rawawb_vertex0_u_2;
 __u16 sw_rawawb_vertex0_v_2;
 __u16 sw_rawawb_vertex1_u_2;
 __u16 sw_rawawb_vertex1_v_2;
 __u16 sw_rawawb_vertex2_u_2;
 __u16 sw_rawawb_vertex2_v_2;
 __u16 sw_rawawb_vertex3_u_2;
 __u16 sw_rawawb_vertex3_v_2;
 __u16 sw_rawawb_vertex0_u_3;
 __u16 sw_rawawb_vertex0_v_3;
 __u16 sw_rawawb_vertex1_u_3;
 __u16 sw_rawawb_vertex1_v_3;
 __u16 sw_rawawb_vertex2_u_3;
 __u16 sw_rawawb_vertex2_v_3;
 __u16 sw_rawawb_vertex3_u_3;
 __u16 sw_rawawb_vertex3_v_3;
 __u16 sw_rawawb_vertex0_u_4;
 __u16 sw_rawawb_vertex0_v_4;
 __u16 sw_rawawb_vertex1_u_4;
 __u16 sw_rawawb_vertex1_v_4;
 __u16 sw_rawawb_vertex2_u_4;
 __u16 sw_rawawb_vertex2_v_4;
 __u16 sw_rawawb_vertex3_u_4;
 __u16 sw_rawawb_vertex3_v_4;
 __u16 sw_rawawb_vertex0_u_5;
 __u16 sw_rawawb_vertex0_v_5;
 __u16 sw_rawawb_vertex1_u_5;
 __u16 sw_rawawb_vertex1_v_5;
 __u16 sw_rawawb_vertex2_u_5;
 __u16 sw_rawawb_vertex2_v_5;
 __u16 sw_rawawb_vertex3_u_5;
 __u16 sw_rawawb_vertex3_v_5;
 __u16 sw_rawawb_vertex0_u_6;
 __u16 sw_rawawb_vertex0_v_6;
 __u16 sw_rawawb_vertex1_u_6;
 __u16 sw_rawawb_vertex1_v_6;
 __u16 sw_rawawb_vertex2_u_6;
 __u16 sw_rawawb_vertex2_v_6;
 __u16 sw_rawawb_vertex3_u_6;
 __u16 sw_rawawb_vertex3_v_6;

 __u16 sw_rawawb_wt0;
 __u16 sw_rawawb_wt1;
 __u16 sw_rawawb_wt2;
 __u16 sw_rawawb_mat0_x;
 __u16 sw_rawawb_mat1_x;
 __u16 sw_rawawb_mat2_x;
 __u16 sw_rawawb_mat0_y;
 __u16 sw_rawawb_mat1_y;
 __u16 sw_rawawb_mat2_y;
 __u16 sw_rawawb_nor_x0_0;
 __u16 sw_rawawb_nor_x1_0;
 __u16 sw_rawawb_nor_y0_0;
 __u16 sw_rawawb_nor_y1_0;
 __u16 sw_rawawb_big_x0_0;
 __u16 sw_rawawb_big_x1_0;
 __u16 sw_rawawb_big_y0_0;
 __u16 sw_rawawb_big_y1_0;
 __u16 sw_rawawb_nor_x0_1;
 __u16 sw_rawawb_nor_x1_1;
 __u16 sw_rawawb_nor_y0_1;
 __u16 sw_rawawb_nor_y1_1;
 __u16 sw_rawawb_big_x0_1;
 __u16 sw_rawawb_big_x1_1;
 __u16 sw_rawawb_big_y0_1;
 __u16 sw_rawawb_big_y1_1;
 __u16 sw_rawawb_nor_x0_2;
 __u16 sw_rawawb_nor_x1_2;
 __u16 sw_rawawb_nor_y0_2;
 __u16 sw_rawawb_nor_y1_2;
 __u16 sw_rawawb_big_x0_2;
 __u16 sw_rawawb_big_x1_2;
 __u16 sw_rawawb_big_y0_2;
 __u16 sw_rawawb_big_y1_2;
 __u16 sw_rawawb_nor_x0_3;
 __u16 sw_rawawb_nor_x1_3;
 __u16 sw_rawawb_nor_y0_3;
 __u16 sw_rawawb_nor_y1_3;
 __u16 sw_rawawb_big_x0_3;
 __u16 sw_rawawb_big_x1_3;
 __u16 sw_rawawb_big_y0_3;
 __u16 sw_rawawb_big_y1_3;
 __u16 sw_rawawb_nor_x0_4;
 __u16 sw_rawawb_nor_x1_4;
 __u16 sw_rawawb_nor_y0_4;
 __u16 sw_rawawb_nor_y1_4;
 __u16 sw_rawawb_big_x0_4;
 __u16 sw_rawawb_big_x1_4;
 __u16 sw_rawawb_big_y0_4;
 __u16 sw_rawawb_big_y1_4;
 __u16 sw_rawawb_nor_x0_5;
 __u16 sw_rawawb_nor_x1_5;
 __u16 sw_rawawb_nor_y0_5;
 __u16 sw_rawawb_nor_y1_5;
 __u16 sw_rawawb_big_x0_5;
 __u16 sw_rawawb_big_x1_5;
 __u16 sw_rawawb_big_y0_5;
 __u16 sw_rawawb_big_y1_5;
 __u16 sw_rawawb_nor_x0_6;
 __u16 sw_rawawb_nor_x1_6;
 __u16 sw_rawawb_nor_y0_6;
 __u16 sw_rawawb_nor_y1_6;
 __u16 sw_rawawb_big_x0_6;
 __u16 sw_rawawb_big_x1_6;
 __u16 sw_rawawb_big_y0_6;
 __u16 sw_rawawb_big_y1_6;
 __u16 sw_rawawb_pre_wbgain_inv_r;
 __u16 sw_rawawb_pre_wbgain_inv_g;
 __u16 sw_rawawb_pre_wbgain_inv_b;
 __u16 sw_rawawb_exc_wp_region0_xu0;
 __u16 sw_rawawb_exc_wp_region0_xu1;
 __u16 sw_rawawb_exc_wp_region0_yv0;
 __u16 sw_rawawb_exc_wp_region0_yv1;
 __u16 sw_rawawb_exc_wp_region1_xu0;
 __u16 sw_rawawb_exc_wp_region1_xu1;
 __u16 sw_rawawb_exc_wp_region1_yv0;
 __u16 sw_rawawb_exc_wp_region1_yv1;
 __u16 sw_rawawb_exc_wp_region2_xu0;
 __u16 sw_rawawb_exc_wp_region2_xu1;
 __u16 sw_rawawb_exc_wp_region2_yv0;
 __u16 sw_rawawb_exc_wp_region2_yv1;
 __u16 sw_rawawb_exc_wp_region3_xu0;
 __u16 sw_rawawb_exc_wp_region3_xu1;
 __u16 sw_rawawb_exc_wp_region3_yv0;
 __u16 sw_rawawb_exc_wp_region3_yv1;
 __u16 sw_rawawb_exc_wp_region4_xu0;
 __u16 sw_rawawb_exc_wp_region4_xu1;
 __u16 sw_rawawb_exc_wp_region4_yv0;
 __u16 sw_rawawb_exc_wp_region4_yv1;
 __u16 sw_rawawb_exc_wp_region5_xu0;
 __u16 sw_rawawb_exc_wp_region5_xu1;
 __u16 sw_rawawb_exc_wp_region5_yv0;
 __u16 sw_rawawb_exc_wp_region5_yv1;
 __u16 sw_rawawb_exc_wp_region6_xu0;
 __u16 sw_rawawb_exc_wp_region6_xu1;
 __u16 sw_rawawb_exc_wp_region6_yv0;
 __u16 sw_rawawb_exc_wp_region6_yv1;
 __u16 sw_rawawb_rgb2ryuvmat0_u;
 __u16 sw_rawawb_rgb2ryuvmat1_u;
 __u16 sw_rawawb_rgb2ryuvmat2_u;
 __u16 sw_rawawb_rgb2ryuvofs_u;
 __u16 sw_rawawb_rgb2ryuvmat0_v;
 __u16 sw_rawawb_rgb2ryuvmat1_v;
 __u16 sw_rawawb_rgb2ryuvmat2_v;
 __u16 sw_rawawb_rgb2ryuvofs_v;
 __u16 sw_rawawb_rgb2ryuvmat0_y;
 __u16 sw_rawawb_rgb2ryuvmat1_y;
 __u16 sw_rawawb_rgb2ryuvmat2_y;
 __u16 sw_rawawb_rgb2ryuvofs_y;
 __u16 sw_rawawb_th0_ls0;
 __u16 sw_rawawb_th1_ls0;
 __u16 sw_rawawb_th2_ls0;
 __u16 sw_rawawb_th3_ls0;
 __u16 sw_rawawb_th4_ls0;
 __u16 sw_rawawb_th5_ls0;
 __u16 sw_rawawb_coor_x1_ls0_u;
 __u16 sw_rawawb_coor_x1_ls0_v;
 __u16 sw_rawawb_coor_x1_ls0_y;
 __u16 sw_rawawb_vec_x21_ls0_u;
 __u16 sw_rawawb_vec_x21_ls0_v;
 __u16 sw_rawawb_vec_x21_ls0_y;
 __u16 sw_rawawb_th0_ls1;
 __u16 sw_rawawb_th1_ls1;
 __u16 sw_rawawb_th2_ls1;
 __u16 sw_rawawb_th3_ls1;
 __u16 sw_rawawb_th4_ls1;
 __u16 sw_rawawb_th5_ls1;
 __u16 sw_rawawb_coor_x1_ls1_u;
 __u16 sw_rawawb_coor_x1_ls1_v;
 __u16 sw_rawawb_coor_x1_ls1_y;
 __u16 sw_rawawb_vec_x21_ls1_u;
 __u16 sw_rawawb_vec_x21_ls1_v;
 __u16 sw_rawawb_vec_x21_ls1_y;
 __u16 sw_rawawb_th0_ls2;
 __u16 sw_rawawb_th1_ls2;
 __u16 sw_rawawb_th2_ls2;
 __u16 sw_rawawb_th3_ls2;
 __u16 sw_rawawb_th4_ls2;
 __u16 sw_rawawb_th5_ls2;
 __u16 sw_rawawb_coor_x1_ls2_u;
 __u16 sw_rawawb_coor_x1_ls2_v;
 __u16 sw_rawawb_coor_x1_ls2_y;
 __u16 sw_rawawb_vec_x21_ls2_u;
 __u16 sw_rawawb_vec_x21_ls2_v;
 __u16 sw_rawawb_vec_x21_ls2_y;
 __u16 sw_rawawb_th0_ls3;
 __u16 sw_rawawb_th1_ls3;
 __u16 sw_rawawb_th2_ls3;
 __u16 sw_rawawb_th3_ls3;
 __u16 sw_rawawb_th4_ls3;
 __u16 sw_rawawb_th5_ls3;
 __u16 sw_rawawb_coor_x1_ls3_u;
 __u16 sw_rawawb_coor_x1_ls3_v;
 __u16 sw_rawawb_coor_x1_ls3_y;
 __u16 sw_rawawb_vec_x21_ls3_u;
 __u16 sw_rawawb_vec_x21_ls3_v;
 __u16 sw_rawawb_vec_x21_ls3_y;

 __u32 sw_rawawb_islope01_0;
 __u32 sw_rawawb_islope12_0;
 __u32 sw_rawawb_islope23_0;
 __u32 sw_rawawb_islope30_0;
 __u32 sw_rawawb_islope01_1;
 __u32 sw_rawawb_islope12_1;
 __u32 sw_rawawb_islope23_1;
 __u32 sw_rawawb_islope30_1;
 __u32 sw_rawawb_islope01_2;
 __u32 sw_rawawb_islope12_2;
 __u32 sw_rawawb_islope23_2;
 __u32 sw_rawawb_islope30_2;
 __u32 sw_rawawb_islope01_3;
 __u32 sw_rawawb_islope12_3;
 __u32 sw_rawawb_islope23_3;
 __u32 sw_rawawb_islope30_3;
 __u32 sw_rawawb_islope01_4;
 __u32 sw_rawawb_islope12_4;
 __u32 sw_rawawb_islope23_4;
 __u32 sw_rawawb_islope30_4;
 __u32 sw_rawawb_islope01_5;
 __u32 sw_rawawb_islope12_5;
 __u32 sw_rawawb_islope23_5;
 __u32 sw_rawawb_islope30_5;
 __u32 sw_rawawb_islope01_6;
 __u32 sw_rawawb_islope12_6;
 __u32 sw_rawawb_islope23_6;
 __u32 sw_rawawb_islope30_6;
} __attribute__ ((packed));

struct isp21_isp_other_cfg {
 struct isp21_bls_cfg bls_cfg;
 struct isp2x_dpcc_cfg dpcc_cfg;
 struct isp2x_lsc_cfg lsc_cfg;
 struct isp21_awb_gain_cfg awb_gain_cfg;
 struct isp21_gic_cfg gic_cfg;
 struct isp2x_debayer_cfg debayer_cfg;
 struct isp21_ccm_cfg ccm_cfg;
 struct isp2x_gammaout_cfg gammaout_cfg;
 struct isp2x_cproc_cfg cproc_cfg;
 struct isp2x_ie_cfg ie_cfg;
 struct isp2x_sdg_cfg sdg_cfg;
 struct isp21_drc_cfg drc_cfg;
 struct isp2x_hdrmge_cfg hdrmge_cfg;
 struct isp21_dhaz_cfg dhaz_cfg;
 struct isp2x_3dlut_cfg isp3dlut_cfg;
 struct isp2x_ldch_cfg ldch_cfg;
 struct isp21_baynr_cfg baynr_cfg;
 struct isp21_bay3d_cfg bay3d_cfg;
 struct isp21_ynr_cfg ynr_cfg;
 struct isp21_cnr_cfg cnr_cfg;
 struct isp21_sharp_cfg sharp_cfg;
 struct isp21_csm_cfg csm_cfg;
 struct isp21_cgc_cfg cgc_cfg;
} __attribute__ ((packed));

struct isp21_isp_meas_cfg {
 struct isp2x_siawb_meas_cfg siawb;
 struct isp21_rawawb_meas_cfg rawawb;
 struct isp2x_rawaelite_meas_cfg rawae0;
 struct isp2x_rawaebig_meas_cfg rawae1;
 struct isp2x_rawaebig_meas_cfg rawae2;
 struct isp2x_rawaebig_meas_cfg rawae3;
 struct isp2x_yuvae_meas_cfg yuvae;
 struct isp2x_rawaf_meas_cfg rawaf;
 struct isp2x_siaf_cfg siaf;
 struct isp2x_rawhistlite_cfg rawhist0;
 struct isp2x_rawhistbig_cfg rawhist1;
 struct isp2x_rawhistbig_cfg rawhist2;
 struct isp2x_rawhistbig_cfg rawhist3;
 struct isp2x_sihst_cfg sihst;
} __attribute__ ((packed));

struct isp21_isp_params_cfg {
 __u64 module_en_update;
 __u64 module_ens;
 __u64 module_cfg_update;

 __u32 frame_id;
 struct isp21_isp_meas_cfg meas;
 struct isp21_isp_other_cfg others;
} __attribute__ ((packed));

struct isp21_rawawb_meas_stat {
 __u16 ro_yhist_bin[8];
 __u32 ro_rawawb_sum_rgain_nor[7];
 __u32 ro_rawawb_sum_bgain_nor[7];
 __u32 ro_rawawb_wp_num_nor[7];
 __u32 ro_rawawb_sum_rgain_big[7];
 __u32 ro_rawawb_sum_bgain_big[7];
 __u32 ro_rawawb_wp_num_big[7];
 struct isp2x_rawawb_ramdata ramdata[225];
} __attribute__ ((packed));

struct isp21_stat {
 struct isp2x_siawb_stat siawb;
 struct isp21_rawawb_meas_stat rawawb;
 struct isp2x_rawaelite_stat rawae0;
 struct isp2x_rawaebig_stat rawae1;
 struct isp2x_rawaebig_stat rawae2;
 struct isp2x_rawaebig_stat rawae3;
 struct isp2x_yuvae_stat yuvae;
 struct isp2x_rawaf_stat rawaf;
 struct isp2x_siaf_stat siaf;
 struct isp2x_rawhistlite_stat rawhist0;
 struct isp2x_rawhistbig_stat rawhist1;
 struct isp2x_rawhistbig_stat rawhist2;
 struct isp2x_rawhistbig_stat rawhist3;
 struct isp2x_sihst_stat sihst;

 struct isp2x_bls_stat bls;
 struct isp21_dhaz_stat dhaz;
} __attribute__ ((packed));

struct rkisp_isp21_stat_buffer {
 unsigned int meas_type;
 unsigned int frame_id;
 unsigned int params_id;
 struct isp21_stat params;
} __attribute__ ((packed));


enum isp3x_unite_id {
 ISP3_LEFT = 0,
 ISP3_RIGHT,
 ISP3_UNITE_MAX,
};

struct isp3x_gammaout_cfg {
 __u8 equ_segm;
 __u8 finalx4_dense_en;
 __u16 offset;
 __u16 gamma_y[49];
} __attribute__ ((packed));

struct isp3x_lsc_cfg {
 __u8 sector_16x16;

 __u16 r_data_tbl[290];
 __u16 gr_data_tbl[290];
 __u16 gb_data_tbl[290];
 __u16 b_data_tbl[290];

 __u16 x_grad_tbl[16];
 __u16 y_grad_tbl[16];

 __u16 x_size_tbl[16];
 __u16 y_size_tbl[16];
} __attribute__ ((packed));

struct isp3x_baynr_cfg {
 __u8 lg2_mode;
 __u8 gauss_en;
 __u8 log_bypass;

 __u16 dgain1;
 __u16 dgain0;
 __u16 dgain2;

 __u16 pix_diff;

 __u16 diff_thld;
 __u16 softthld;

 __u16 bltflt_streng;
 __u16 reg_w1;

 __u16 sigma_x[16];
 __u16 sigma_y[16];

 __u16 weit_d2;
 __u16 weit_d1;
 __u16 weit_d0;

 __u16 lg2_lgoff;
 __u16 lg2_off;

 __u32 dat_max;
} __attribute__ ((packed));

struct isp3x_bay3d_cfg {
 __u8 bypass_en;
 __u8 hibypass_en;
 __u8 lobypass_en;
 __u8 himed_bypass_en;
 __u8 higaus_bypass_en;
 __u8 hiabs_possel;
 __u8 hichnsplit_en;
 __u8 lomed_bypass_en;
 __u8 logaus5_bypass_en;
 __u8 logaus3_bypass_en;
 __u8 glbpk_en;
 __u8 loswitch_protect;

 __u16 softwgt;
 __u16 hidif_th;

 __u32 glbpk2;

 __u16 wgtlmt;
 __u16 wgtratio;

 __u16 sig0_x[16];
 __u16 sig0_y[16];
 __u16 sig1_x[16];
 __u16 sig1_y[16];
 __u16 sig2_x[16];
 __u16 sig2_y[16];
} __attribute__ ((packed));

struct isp3x_ynr_cfg {
 __u8 rnr_en;
 __u8 thumb_mix_cur_en;
 __u8 global_gain_alpha;
 __u8 flt1x1_bypass_sel;
 __u8 sft5x5_bypass;
 __u8 flt1x1_bypass;
 __u8 lgft3x3_bypass;
 __u8 lbft5x5_bypass;
 __u8 bft3x3_bypass;
 __u16 global_gain;

 __u16 rnr_max_r;
 __u16 local_gainscale;

 __u16 rnr_center_coorh;
 __u16 rnr_center_coorv;

 __u16 loclagain_adj_thresh;
 __u16 localgain_adj;

 __u16 low_bf_inv1;
 __u16 low_bf_inv0;

 __u16 low_peak_supress;
 __u16 low_thred_adj;

 __u16 low_dist_adj;
 __u16 low_edge_adj_thresh;

 __u16 low_bi_weight;
 __u16 low_weight;
 __u16 low_center_weight;
 __u16 hi_min_adj;
 __u16 high_thred_adj;
 __u8 high_retain_weight;
 __u8 hi_edge_thed;
 __u8 base_filter_weight2;
 __u8 base_filter_weight1;
 __u8 base_filter_weight0;
 __u16 frame_full_size;
 __u16 lbf_weight_thres;
 __u16 low_gauss1_coeff2;
 __u16 low_gauss1_coeff1;
 __u16 low_gauss1_coeff0;
 __u16 low_gauss2_coeff2;
 __u16 low_gauss2_coeff1;
 __u16 low_gauss2_coeff0;
 __u8 direction_weight3;
 __u8 direction_weight2;
 __u8 direction_weight1;
 __u8 direction_weight0;
 __u8 direction_weight7;
 __u8 direction_weight6;
 __u8 direction_weight5;
 __u8 direction_weight4;
 __u16 luma_points_x[17];
 __u16 lsgm_y[17];
 __u16 hsgm_y[17];
 __u8 rnr_strength3[17];
} __attribute__ ((packed));

struct isp3x_cnr_cfg {
 __u8 thumb_mix_cur_en;
 __u8 lq_bila_bypass;
 __u8 hq_bila_bypass;
 __u8 exgain_bypass;

 __u8 global_gain_alpha;
 __u16 global_gain;

 __u8 gain_iso;
 __u8 gain_offset;
 __u8 gain_1sigma;

 __u8 gain_uvgain1;
 __u8 gain_uvgain0;

 __u8 lmed3_alpha;

 __u8 lbf5_gain_y;
 __u8 lbf5_gain_c;

 __u8 lbf5_weit_d3;
 __u8 lbf5_weit_d2;
 __u8 lbf5_weit_d1;
 __u8 lbf5_weit_d0;

 __u8 lbf5_weit_d4;

 __u8 hmed3_alpha;

 __u16 hbf5_weit_src;
 __u16 hbf5_min_wgt;
 __u16 hbf5_sigma;

 __u16 lbf5_weit_src;
 __u16 lbf3_sigma;

 __u8 sigma_y[13];
} __attribute__ ((packed));

struct isp3x_sharp_cfg {
 __u8 bypass;
 __u8 center_mode;
 __u8 exgain_bypass;

 __u8 sharp_ratio;
 __u8 bf_ratio;
 __u8 gaus_ratio;
 __u8 pbf_ratio;

 __u8 luma_dx[7];

 __u16 pbf_sigma_inv[8];

 __u16 bf_sigma_inv[8];

 __u8 bf_sigma_shift;
 __u8 pbf_sigma_shift;

 __u16 ehf_th[8];

 __u16 clip_hf[8];

 __u8 pbf_coef2;
 __u8 pbf_coef1;
 __u8 pbf_coef0;

 __u8 bf_coef2;
 __u8 bf_coef1;
 __u8 bf_coef0;

 __u8 gaus_coef[6];
} __attribute__ ((packed));

struct isp3x_dhaz_cfg {
 __u8 round_en;
 __u8 soft_wr_en;
 __u8 enhance_en;
 __u8 air_lc_en;
 __u8 hpara_en;
 __u8 hist_en;
 __u8 dc_en;

 __u8 yblk_th;
 __u8 yhist_th;
 __u8 dc_max_th;
 __u8 dc_min_th;

 __u16 wt_max;
 __u8 bright_max;
 __u8 bright_min;

 __u8 tmax_base;
 __u8 dark_th;
 __u8 air_max;
 __u8 air_min;

 __u16 tmax_max;
 __u16 tmax_off;

 __u8 hist_k;
 __u8 hist_th_off;
 __u16 hist_min;

 __u16 hist_gratio;
 __u16 hist_scale;

 __u16 enhance_value;
 __u16 enhance_chroma;

 __u16 iir_wt_sigma;
 __u16 iir_sigma;
 __u16 stab_fnum;

 __u16 iir_tmax_sigma;
 __u16 iir_air_sigma;
 __u8 iir_pre_wet;

 __u16 cfg_wt;
 __u16 cfg_air;
 __u16 cfg_alpha;

 __u16 cfg_gratio;
 __u16 cfg_tmax;

 __u16 range_sima;
 __u8 space_sigma_pre;
 __u8 space_sigma_cur;

 __u16 dc_weitcur;
 __u16 bf_weight;

 __u16 enh_curve[17];

 __u8 gaus_h2;
 __u8 gaus_h1;
 __u8 gaus_h0;

 __u8 sigma_idx[15];
 __u16 sigma_lut[17];

 __u16 adp_wt_wr;
 __u16 adp_air_wr;

 __u16 adp_tmax_wr;
 __u16 adp_gratio_wr;

 __u16 hist_wr[64];
} __attribute__ ((packed));

struct isp3x_dhaz_stat {
 __u32 dhaz_pic_sumh;

 __u16 dhaz_adp_air_base;
 __u16 dhaz_adp_wt;

 __u16 dhaz_adp_gratio;
 __u16 dhaz_adp_tmax;

 __u16 h_rgb_iir[64];
} __attribute__ ((packed));

struct isp3x_drc_cfg {
 __u8 bypass_en;
 __u8 offset_pow2;
 __u16 compres_scl;
 __u16 position;
 __u16 delta_scalein;
 __u16 hpdetail_ratio;
 __u16 lpdetail_ratio;
 __u8 weicur_pix;
 __u8 weipre_frame;
 __u8 bilat_wt_off;
 __u16 force_sgm_inv0;
 __u8 motion_scl;
 __u8 edge_scl;
 __u16 space_sgm_inv1;
 __u16 space_sgm_inv0;
 __u16 range_sgm_inv1;
 __u16 range_sgm_inv0;
 __u8 weig_maxl;
 __u8 weig_bilat;
 __u8 enable_soft_thd;
 __u16 bilat_soft_thd;
 __u16 gain_y[17];
 __u16 compres_y[17];
 __u16 scale_y[17];
 __u16 wr_cycle;
 __u16 iir_weight;
 __u16 min_ogain;
} __attribute__ ((packed));

struct isp3x_hdrmge_cfg {
 __u8 s_base;
 __u8 mode;

 __u16 gain0_inv;
 __u16 gain0;
 __u16 gain1_inv;
 __u16 gain1;
 __u8 gain2;

 __u8 lm_dif_0p15;
 __u8 lm_dif_0p9;
 __u8 ms_diff_0p15;
 __u8 ms_dif_0p8;

 __u16 ms_thd1;
 __u16 ms_thd0;
 __u16 ms_scl;
 __u16 lm_thd1;
 __u16 lm_thd0;
 __u16 lm_scl;
 struct isp2x_hdrmge_curve curve;
 __u16 e_y[17];
} __attribute__ ((packed));

struct isp3x_rawawb_meas_cfg {
 __u8 rawawb_sel;
 __u8 sw_rawawb_xy_en0;
 __u8 sw_rawawb_uv_en0;
 __u8 sw_rawawb_xy_en1;
 __u8 sw_rawawb_uv_en1;
 __u8 sw_rawawb_3dyuv_en0;
 __u8 sw_rawawb_3dyuv_en1;
 __u8 sw_rawawb_wp_blk_wei_en0;
 __u8 sw_rawawb_wp_blk_wei_en1;
 __u8 sw_rawawb_wp_luma_wei_en0;
 __u8 sw_rawawb_wp_luma_wei_en1;
 __u8 sw_rawlsc_bypass_en;
 __u8 sw_rawawb_blk_measure_enable;
 __u8 sw_rawawb_blk_measure_mode;
 __u8 sw_rawawb_blk_measure_xytype;
 __u8 sw_rawawb_blk_measure_illu_idx;
 __u8 sw_rawawb_wp_hist_xytype;
 __u8 sw_rawawb_light_num;
 __u8 sw_rawawb_wind_size;
 __u8 sw_rawawb_r_max;
 __u8 sw_rawawb_g_max;
 __u8 sw_rawawb_b_max;
 __u8 sw_rawawb_y_max;
 __u8 sw_rawawb_r_min;
 __u8 sw_rawawb_g_min;
 __u8 sw_rawawb_b_min;
 __u8 sw_rawawb_y_min;
 __u8 sw_rawawb_3dyuv_ls_idx0;
 __u8 sw_rawawb_3dyuv_ls_idx1;
 __u8 sw_rawawb_3dyuv_ls_idx2;
 __u8 sw_rawawb_3dyuv_ls_idx3;
 __u8 sw_rawawb_multiwindow_en;
 __u8 sw_rawawb_exc_wp_region0_excen0;
 __u8 sw_rawawb_exc_wp_region0_excen1;
 __u8 sw_rawawb_exc_wp_region0_measen;
 __u8 sw_rawawb_exc_wp_region0_domain;
 __u8 sw_rawawb_exc_wp_region1_excen0;
 __u8 sw_rawawb_exc_wp_region1_excen1;
 __u8 sw_rawawb_exc_wp_region1_measen;
 __u8 sw_rawawb_exc_wp_region1_domain;
 __u8 sw_rawawb_exc_wp_region2_excen0;
 __u8 sw_rawawb_exc_wp_region2_excen1;
 __u8 sw_rawawb_exc_wp_region2_measen;
 __u8 sw_rawawb_exc_wp_region2_domain;
 __u8 sw_rawawb_exc_wp_region3_excen0;
 __u8 sw_rawawb_exc_wp_region3_excen1;
 __u8 sw_rawawb_exc_wp_region3_measen;
 __u8 sw_rawawb_exc_wp_region3_domain;
 __u8 sw_rawawb_exc_wp_region4_excen0;
 __u8 sw_rawawb_exc_wp_region4_excen1;
 __u8 sw_rawawb_exc_wp_region4_domain;
 __u8 sw_rawawb_exc_wp_region5_excen0;
 __u8 sw_rawawb_exc_wp_region5_excen1;
 __u8 sw_rawawb_exc_wp_region5_domain;
 __u8 sw_rawawb_exc_wp_region6_excen0;
 __u8 sw_rawawb_exc_wp_region6_excen1;
 __u8 sw_rawawb_exc_wp_region6_domain;
 __u8 sw_rawawb_wp_luma_weicurve_y0;
 __u8 sw_rawawb_wp_luma_weicurve_y1;
 __u8 sw_rawawb_wp_luma_weicurve_y2;
 __u8 sw_rawawb_wp_luma_weicurve_y3;
 __u8 sw_rawawb_wp_luma_weicurve_y4;
 __u8 sw_rawawb_wp_luma_weicurve_y5;
 __u8 sw_rawawb_wp_luma_weicurve_y6;
 __u8 sw_rawawb_wp_luma_weicurve_y7;
 __u8 sw_rawawb_wp_luma_weicurve_y8;
 __u8 sw_rawawb_wp_luma_weicurve_w0;
 __u8 sw_rawawb_wp_luma_weicurve_w1;
 __u8 sw_rawawb_wp_luma_weicurve_w2;
 __u8 sw_rawawb_wp_luma_weicurve_w3;
 __u8 sw_rawawb_wp_luma_weicurve_w4;
 __u8 sw_rawawb_wp_luma_weicurve_w5;
 __u8 sw_rawawb_wp_luma_weicurve_w6;
 __u8 sw_rawawb_wp_luma_weicurve_w7;
 __u8 sw_rawawb_wp_luma_weicurve_w8;
 __u8 sw_rawawb_rotu0_ls0;
 __u8 sw_rawawb_rotu1_ls0;
 __u8 sw_rawawb_rotu2_ls0;
 __u8 sw_rawawb_rotu3_ls0;
 __u8 sw_rawawb_rotu4_ls0;
 __u8 sw_rawawb_rotu5_ls0;
 __u8 sw_rawawb_dis_x1x2_ls0;
 __u8 sw_rawawb_rotu0_ls1;
 __u8 sw_rawawb_rotu1_ls1;
 __u8 sw_rawawb_rotu2_ls1;
 __u8 sw_rawawb_rotu3_ls1;
 __u8 sw_rawawb_rotu4_ls1;
 __u8 sw_rawawb_rotu5_ls1;
 __u8 sw_rawawb_dis_x1x2_ls1;
 __u8 sw_rawawb_rotu0_ls2;
 __u8 sw_rawawb_rotu1_ls2;
 __u8 sw_rawawb_rotu2_ls2;
 __u8 sw_rawawb_rotu3_ls2;
 __u8 sw_rawawb_rotu4_ls2;
 __u8 sw_rawawb_rotu5_ls2;
 __u8 sw_rawawb_dis_x1x2_ls2;
 __u8 sw_rawawb_rotu0_ls3;
 __u8 sw_rawawb_rotu1_ls3;
 __u8 sw_rawawb_rotu2_ls3;
 __u8 sw_rawawb_rotu3_ls3;
 __u8 sw_rawawb_rotu4_ls3;
 __u8 sw_rawawb_rotu5_ls3;
 __u8 sw_rawawb_dis_x1x2_ls3;
 __u8 sw_rawawb_blk_rtdw_measure_en;
 __u8 sw_rawawb_blk_with_luma_wei_en;
 __u8 sw_rawawb_wp_blk_wei_w[225];

 __u16 sw_rawawb_h_offs;
 __u16 sw_rawawb_v_offs;
 __u16 sw_rawawb_h_size;
 __u16 sw_rawawb_v_size;
 __u16 sw_rawawb_vertex0_u_0;
 __u16 sw_rawawb_vertex0_v_0;
 __u16 sw_rawawb_vertex1_u_0;
 __u16 sw_rawawb_vertex1_v_0;
 __u16 sw_rawawb_vertex2_u_0;
 __u16 sw_rawawb_vertex2_v_0;
 __u16 sw_rawawb_vertex3_u_0;
 __u16 sw_rawawb_vertex3_v_0;
 __u16 sw_rawawb_vertex0_u_1;
 __u16 sw_rawawb_vertex0_v_1;
 __u16 sw_rawawb_vertex1_u_1;
 __u16 sw_rawawb_vertex1_v_1;
 __u16 sw_rawawb_vertex2_u_1;
 __u16 sw_rawawb_vertex2_v_1;
 __u16 sw_rawawb_vertex3_u_1;
 __u16 sw_rawawb_vertex3_v_1;
 __u16 sw_rawawb_vertex0_u_2;
 __u16 sw_rawawb_vertex0_v_2;
 __u16 sw_rawawb_vertex1_u_2;
 __u16 sw_rawawb_vertex1_v_2;
 __u16 sw_rawawb_vertex2_u_2;
 __u16 sw_rawawb_vertex2_v_2;
 __u16 sw_rawawb_vertex3_u_2;
 __u16 sw_rawawb_vertex3_v_2;
 __u16 sw_rawawb_vertex0_u_3;
 __u16 sw_rawawb_vertex0_v_3;
 __u16 sw_rawawb_vertex1_u_3;
 __u16 sw_rawawb_vertex1_v_3;
 __u16 sw_rawawb_vertex2_u_3;
 __u16 sw_rawawb_vertex2_v_3;
 __u16 sw_rawawb_vertex3_u_3;
 __u16 sw_rawawb_vertex3_v_3;
 __u16 sw_rawawb_vertex0_u_4;
 __u16 sw_rawawb_vertex0_v_4;
 __u16 sw_rawawb_vertex1_u_4;
 __u16 sw_rawawb_vertex1_v_4;
 __u16 sw_rawawb_vertex2_u_4;
 __u16 sw_rawawb_vertex2_v_4;
 __u16 sw_rawawb_vertex3_u_4;
 __u16 sw_rawawb_vertex3_v_4;
 __u16 sw_rawawb_vertex0_u_5;
 __u16 sw_rawawb_vertex0_v_5;
 __u16 sw_rawawb_vertex1_u_5;
 __u16 sw_rawawb_vertex1_v_5;
 __u16 sw_rawawb_vertex2_u_5;
 __u16 sw_rawawb_vertex2_v_5;
 __u16 sw_rawawb_vertex3_u_5;
 __u16 sw_rawawb_vertex3_v_5;
 __u16 sw_rawawb_vertex0_u_6;
 __u16 sw_rawawb_vertex0_v_6;
 __u16 sw_rawawb_vertex1_u_6;
 __u16 sw_rawawb_vertex1_v_6;
 __u16 sw_rawawb_vertex2_u_6;
 __u16 sw_rawawb_vertex2_v_6;
 __u16 sw_rawawb_vertex3_u_6;
 __u16 sw_rawawb_vertex3_v_6;

 __u16 sw_rawawb_wt0;
 __u16 sw_rawawb_wt1;
 __u16 sw_rawawb_wt2;
 __u16 sw_rawawb_mat0_x;
 __u16 sw_rawawb_mat1_x;
 __u16 sw_rawawb_mat2_x;
 __u16 sw_rawawb_mat0_y;
 __u16 sw_rawawb_mat1_y;
 __u16 sw_rawawb_mat2_y;
 __u16 sw_rawawb_nor_x0_0;
 __u16 sw_rawawb_nor_x1_0;
 __u16 sw_rawawb_nor_y0_0;
 __u16 sw_rawawb_nor_y1_0;
 __u16 sw_rawawb_big_x0_0;
 __u16 sw_rawawb_big_x1_0;
 __u16 sw_rawawb_big_y0_0;
 __u16 sw_rawawb_big_y1_0;
 __u16 sw_rawawb_nor_x0_1;
 __u16 sw_rawawb_nor_x1_1;
 __u16 sw_rawawb_nor_y0_1;
 __u16 sw_rawawb_nor_y1_1;
 __u16 sw_rawawb_big_x0_1;
 __u16 sw_rawawb_big_x1_1;
 __u16 sw_rawawb_big_y0_1;
 __u16 sw_rawawb_big_y1_1;
 __u16 sw_rawawb_nor_x0_2;
 __u16 sw_rawawb_nor_x1_2;
 __u16 sw_rawawb_nor_y0_2;
 __u16 sw_rawawb_nor_y1_2;
 __u16 sw_rawawb_big_x0_2;
 __u16 sw_rawawb_big_x1_2;
 __u16 sw_rawawb_big_y0_2;
 __u16 sw_rawawb_big_y1_2;
 __u16 sw_rawawb_nor_x0_3;
 __u16 sw_rawawb_nor_x1_3;
 __u16 sw_rawawb_nor_y0_3;
 __u16 sw_rawawb_nor_y1_3;
 __u16 sw_rawawb_big_x0_3;
 __u16 sw_rawawb_big_x1_3;
 __u16 sw_rawawb_big_y0_3;
 __u16 sw_rawawb_big_y1_3;
 __u16 sw_rawawb_nor_x0_4;
 __u16 sw_rawawb_nor_x1_4;
 __u16 sw_rawawb_nor_y0_4;
 __u16 sw_rawawb_nor_y1_4;
 __u16 sw_rawawb_big_x0_4;
 __u16 sw_rawawb_big_x1_4;
 __u16 sw_rawawb_big_y0_4;
 __u16 sw_rawawb_big_y1_4;
 __u16 sw_rawawb_nor_x0_5;
 __u16 sw_rawawb_nor_x1_5;
 __u16 sw_rawawb_nor_y0_5;
 __u16 sw_rawawb_nor_y1_5;
 __u16 sw_rawawb_big_x0_5;
 __u16 sw_rawawb_big_x1_5;
 __u16 sw_rawawb_big_y0_5;
 __u16 sw_rawawb_big_y1_5;
 __u16 sw_rawawb_nor_x0_6;
 __u16 sw_rawawb_nor_x1_6;
 __u16 sw_rawawb_nor_y0_6;
 __u16 sw_rawawb_nor_y1_6;
 __u16 sw_rawawb_big_x0_6;
 __u16 sw_rawawb_big_x1_6;
 __u16 sw_rawawb_big_y0_6;
 __u16 sw_rawawb_big_y1_6;
 __u16 sw_rawawb_pre_wbgain_inv_r;
 __u16 sw_rawawb_pre_wbgain_inv_g;
 __u16 sw_rawawb_pre_wbgain_inv_b;
 __u16 sw_rawawb_multiwindow0_v_offs;
 __u16 sw_rawawb_multiwindow0_h_offs;
 __u16 sw_rawawb_multiwindow0_v_size;
 __u16 sw_rawawb_multiwindow0_h_size;
 __u16 sw_rawawb_multiwindow1_v_offs;
 __u16 sw_rawawb_multiwindow1_h_offs;
 __u16 sw_rawawb_multiwindow1_v_size;
 __u16 sw_rawawb_multiwindow1_h_size;
 __u16 sw_rawawb_multiwindow2_v_offs;
 __u16 sw_rawawb_multiwindow2_h_offs;
 __u16 sw_rawawb_multiwindow2_v_size;
 __u16 sw_rawawb_multiwindow2_h_size;
 __u16 sw_rawawb_multiwindow3_v_offs;
 __u16 sw_rawawb_multiwindow3_h_offs;
 __u16 sw_rawawb_multiwindow3_v_size;
 __u16 sw_rawawb_multiwindow3_h_size;
 __u16 sw_rawawb_exc_wp_region0_xu0;
 __u16 sw_rawawb_exc_wp_region0_xu1;
 __u16 sw_rawawb_exc_wp_region0_yv0;
 __u16 sw_rawawb_exc_wp_region0_yv1;
 __u16 sw_rawawb_exc_wp_region1_xu0;
 __u16 sw_rawawb_exc_wp_region1_xu1;
 __u16 sw_rawawb_exc_wp_region1_yv0;
 __u16 sw_rawawb_exc_wp_region1_yv1;
 __u16 sw_rawawb_exc_wp_region2_xu0;
 __u16 sw_rawawb_exc_wp_region2_xu1;
 __u16 sw_rawawb_exc_wp_region2_yv0;
 __u16 sw_rawawb_exc_wp_region2_yv1;
 __u16 sw_rawawb_exc_wp_region3_xu0;
 __u16 sw_rawawb_exc_wp_region3_xu1;
 __u16 sw_rawawb_exc_wp_region3_yv0;
 __u16 sw_rawawb_exc_wp_region3_yv1;
 __u16 sw_rawawb_exc_wp_region4_xu0;
 __u16 sw_rawawb_exc_wp_region4_xu1;
 __u16 sw_rawawb_exc_wp_region4_yv0;
 __u16 sw_rawawb_exc_wp_region4_yv1;
 __u16 sw_rawawb_exc_wp_region5_xu0;
 __u16 sw_rawawb_exc_wp_region5_xu1;
 __u16 sw_rawawb_exc_wp_region5_yv0;
 __u16 sw_rawawb_exc_wp_region5_yv1;
 __u16 sw_rawawb_exc_wp_region6_xu0;
 __u16 sw_rawawb_exc_wp_region6_xu1;
 __u16 sw_rawawb_exc_wp_region6_yv0;
 __u16 sw_rawawb_exc_wp_region6_yv1;
 __u16 sw_rawawb_rgb2ryuvmat0_u;
 __u16 sw_rawawb_rgb2ryuvmat1_u;
 __u16 sw_rawawb_rgb2ryuvmat2_u;
 __u16 sw_rawawb_rgb2ryuvofs_u;
 __u16 sw_rawawb_rgb2ryuvmat0_v;
 __u16 sw_rawawb_rgb2ryuvmat1_v;
 __u16 sw_rawawb_rgb2ryuvmat2_v;
 __u16 sw_rawawb_rgb2ryuvofs_v;
 __u16 sw_rawawb_rgb2ryuvmat0_y;
 __u16 sw_rawawb_rgb2ryuvmat1_y;
 __u16 sw_rawawb_rgb2ryuvmat2_y;
 __u16 sw_rawawb_rgb2ryuvofs_y;
 __u16 sw_rawawb_th0_ls0;
 __u16 sw_rawawb_th1_ls0;
 __u16 sw_rawawb_th2_ls0;
 __u16 sw_rawawb_th3_ls0;
 __u16 sw_rawawb_th4_ls0;
 __u16 sw_rawawb_th5_ls0;
 __u16 sw_rawawb_coor_x1_ls0_u;
 __u16 sw_rawawb_coor_x1_ls0_v;
 __u16 sw_rawawb_coor_x1_ls0_y;
 __u16 sw_rawawb_vec_x21_ls0_u;
 __u16 sw_rawawb_vec_x21_ls0_v;
 __u16 sw_rawawb_vec_x21_ls0_y;
 __u16 sw_rawawb_th0_ls1;
 __u16 sw_rawawb_th1_ls1;
 __u16 sw_rawawb_th2_ls1;
 __u16 sw_rawawb_th3_ls1;
 __u16 sw_rawawb_th4_ls1;
 __u16 sw_rawawb_th5_ls1;
 __u16 sw_rawawb_coor_x1_ls1_u;
 __u16 sw_rawawb_coor_x1_ls1_v;
 __u16 sw_rawawb_coor_x1_ls1_y;
 __u16 sw_rawawb_vec_x21_ls1_u;
 __u16 sw_rawawb_vec_x21_ls1_v;
 __u16 sw_rawawb_vec_x21_ls1_y;
 __u16 sw_rawawb_th0_ls2;
 __u16 sw_rawawb_th1_ls2;
 __u16 sw_rawawb_th2_ls2;
 __u16 sw_rawawb_th3_ls2;
 __u16 sw_rawawb_th4_ls2;
 __u16 sw_rawawb_th5_ls2;
 __u16 sw_rawawb_coor_x1_ls2_u;
 __u16 sw_rawawb_coor_x1_ls2_v;
 __u16 sw_rawawb_coor_x1_ls2_y;
 __u16 sw_rawawb_vec_x21_ls2_u;
 __u16 sw_rawawb_vec_x21_ls2_v;
 __u16 sw_rawawb_vec_x21_ls2_y;
 __u16 sw_rawawb_th0_ls3;
 __u16 sw_rawawb_th1_ls3;
 __u16 sw_rawawb_th2_ls3;
 __u16 sw_rawawb_th3_ls3;
 __u16 sw_rawawb_th4_ls3;
 __u16 sw_rawawb_th5_ls3;
 __u16 sw_rawawb_coor_x1_ls3_u;
 __u16 sw_rawawb_coor_x1_ls3_v;
 __u16 sw_rawawb_coor_x1_ls3_y;
 __u16 sw_rawawb_vec_x21_ls3_u;
 __u16 sw_rawawb_vec_x21_ls3_v;
 __u16 sw_rawawb_vec_x21_ls3_y;

 __u32 sw_rawawb_islope01_0;
 __u32 sw_rawawb_islope12_0;
 __u32 sw_rawawb_islope23_0;
 __u32 sw_rawawb_islope30_0;
 __u32 sw_rawawb_islope01_1;
 __u32 sw_rawawb_islope12_1;
 __u32 sw_rawawb_islope23_1;
 __u32 sw_rawawb_islope30_1;
 __u32 sw_rawawb_islope01_2;
 __u32 sw_rawawb_islope12_2;
 __u32 sw_rawawb_islope23_2;
 __u32 sw_rawawb_islope30_2;
 __u32 sw_rawawb_islope01_3;
 __u32 sw_rawawb_islope12_3;
 __u32 sw_rawawb_islope23_3;
 __u32 sw_rawawb_islope30_3;
 __u32 sw_rawawb_islope01_4;
 __u32 sw_rawawb_islope12_4;
 __u32 sw_rawawb_islope23_4;
 __u32 sw_rawawb_islope30_4;
 __u32 sw_rawawb_islope01_5;
 __u32 sw_rawawb_islope12_5;
 __u32 sw_rawawb_islope23_5;
 __u32 sw_rawawb_islope30_5;
 __u32 sw_rawawb_islope01_6;
 __u32 sw_rawawb_islope12_6;
 __u32 sw_rawawb_islope23_6;
 __u32 sw_rawawb_islope30_6;
} __attribute__ ((packed));

struct isp3x_rawawb_meas_stat {
 __u16 ro_yhist_bin[8];
 __u32 ro_rawawb_sum_rgain_nor[7];
 __u32 ro_rawawb_sum_bgain_nor[7];
 __u32 ro_rawawb_wp_num_nor[7];
 __u32 ro_rawawb_sum_rgain_big[7];
 __u32 ro_rawawb_sum_bgain_big[7];
 __u32 ro_rawawb_wp_num_big[7];
 __u32 ro_wp_num2[7];
 __u32 ro_sum_r_nor_multiwindow[4];
 __u32 ro_sum_b_nor_multiwindow[4];
 __u32 ro_wp_nm_nor_multiwindow[4];
 __u32 ro_sum_r_big_multiwindow[4];
 __u32 ro_sum_b_big_multiwindow[4];
 __u32 ro_wp_nm_big_multiwindow[4];
 __u32 ro_sum_r_exc[4];
 __u32 ro_sum_b_exc[4];
 __u32 ro_wp_nm_exc[4];
 struct isp2x_rawawb_ramdata ramdata[225];
} __attribute__ ((packed));

struct isp3x_rawaf_curve {
 __u8 ldg_lumth;
 __u8 ldg_gain;
 __u16 ldg_gslp;
} __attribute__ ((packed));

struct isp3x_rawaf_meas_cfg {
 __u8 rawaf_sel;
 __u8 num_afm_win;

 __u8 gamma_en;
 __u8 gaus_en;
 __u8 v1_fir_sel;
 __u8 hiir_en;
 __u8 viir_en;
 __u8 accu_8bit_mode;
 __u8 ldg_en;
 __u8 h1_fv_mode;
 __u8 h2_fv_mode;
 __u8 v1_fv_mode;
 __u8 v2_fv_mode;
 __u8 ae_mode;
 __u8 y_mode;

 struct isp2x_window win[2];

 __u8 line_num[5];
 __u8 line_en[5];

 __u16 afm_thres;

 __u8 afm_var_shift[2];
 __u8 lum_var_shift[2];

 __u8 h1iir_var_shift;
 __u8 h2iir_var_shift;
 __u8 v1iir_var_shift;
 __u8 v2iir_var_shift;

 __u16 gamma_y[17];

 __u16 h_fv_thresh;
 __u16 v_fv_thresh;
 struct isp3x_rawaf_curve curve_h[2];
 struct isp3x_rawaf_curve curve_v[2];
 __s16 h1iir1_coe[6];
 __s16 h1iir2_coe[6];
 __s16 h2iir1_coe[6];
 __s16 h2iir2_coe[6];
 __s16 v1iir_coe[9];
 __s16 v2iir_coe[3];
 __s16 v1fir_coe[3];
 __s16 v2fir_coe[3];
 __u16 highlit_thresh;
} __attribute__ ((packed));

struct isp3x_rawaf_ramdata {
 __u32 v1;
 __u32 v2;
 __u32 h1;
 __u32 h2;
} __attribute__ ((packed));

struct isp3x_rawaf_stat {
 __u32 int_state;
 __u32 afm_sum_b;
 __u32 afm_lum_b;
 __u32 highlit_cnt_winb;
 struct isp3x_rawaf_ramdata ramdata[225];
} __attribute__ ((packed));

struct isp3x_cac_cfg {
 __u8 bypass_en;
 __u8 center_en;

 __u8 psf_sft_bit;
 __u16 cfg_num;

 __u16 center_width;
 __u16 center_height;

 __u16 strength[22];

 __u32 hsize;
 __u32 vsize;
 __s32 buf_fd;
} __attribute__ ((packed));

struct isp3x_gain_cfg {
 __u32 g0;
 __u16 g1;
 __u16 g2;
} __attribute__ ((packed));

struct isp3x_isp_other_cfg {
 struct isp21_bls_cfg bls_cfg;
 struct isp2x_dpcc_cfg dpcc_cfg;
 struct isp3x_lsc_cfg lsc_cfg;
 struct isp21_awb_gain_cfg awb_gain_cfg;
 struct isp21_gic_cfg gic_cfg;
 struct isp2x_debayer_cfg debayer_cfg;
 struct isp21_ccm_cfg ccm_cfg;
 struct isp3x_gammaout_cfg gammaout_cfg;
 struct isp2x_cproc_cfg cproc_cfg;
 struct isp2x_ie_cfg ie_cfg;
 struct isp2x_sdg_cfg sdg_cfg;
 struct isp3x_drc_cfg drc_cfg;
 struct isp3x_hdrmge_cfg hdrmge_cfg;
 struct isp3x_dhaz_cfg dhaz_cfg;
 struct isp2x_3dlut_cfg isp3dlut_cfg;
 struct isp2x_ldch_cfg ldch_cfg;
 struct isp3x_baynr_cfg baynr_cfg;
 struct isp3x_bay3d_cfg bay3d_cfg;
 struct isp3x_ynr_cfg ynr_cfg;
 struct isp3x_cnr_cfg cnr_cfg;
 struct isp3x_sharp_cfg sharp_cfg;
 struct isp3x_cac_cfg cac_cfg;
 struct isp3x_gain_cfg gain_cfg;
 struct isp21_csm_cfg csm_cfg;
 struct isp21_cgc_cfg cgc_cfg;
} __attribute__ ((packed));

struct isp3x_isp_meas_cfg {
 struct isp3x_rawaf_meas_cfg rawaf;
 struct isp3x_rawawb_meas_cfg rawawb;
 struct isp2x_rawaelite_meas_cfg rawae0;
 struct isp2x_rawaebig_meas_cfg rawae1;
 struct isp2x_rawaebig_meas_cfg rawae2;
 struct isp2x_rawaebig_meas_cfg rawae3;
 struct isp2x_rawhistlite_cfg rawhist0;
 struct isp2x_rawhistbig_cfg rawhist1;
 struct isp2x_rawhistbig_cfg rawhist2;
 struct isp2x_rawhistbig_cfg rawhist3;
} __attribute__ ((packed));

struct isp3x_isp_params_cfg {
 __u64 module_en_update;
 __u64 module_ens;
 __u64 module_cfg_update;

 __u32 frame_id;
 struct isp3x_isp_meas_cfg meas;
 struct isp3x_isp_other_cfg others;
 struct sensor_exposure_cfg exposure;
} __attribute__ ((packed));

struct isp3x_stat {
 struct isp2x_rawaebig_stat rawae3;
 struct isp2x_rawaebig_stat rawae1;
 struct isp2x_rawaebig_stat rawae2;
 struct isp2x_rawaelite_stat rawae0;
 struct isp2x_rawhistbig_stat rawhist3;
 struct isp2x_rawhistlite_stat rawhist0;
 struct isp2x_rawhistbig_stat rawhist1;
 struct isp2x_rawhistbig_stat rawhist2;
 struct isp3x_rawaf_stat rawaf;
 struct isp3x_rawawb_meas_stat rawawb;
 struct isp3x_dhaz_stat dhaz;
 struct isp2x_bls_stat bls;
} __attribute__ ((packed));

struct rkisp3x_isp_stat_buffer {
 __u32 meas_type;
 __u32 frame_id;
 __u32 params_id;
 struct isp3x_stat params;
} __attribute__ ((packed));


struct isp32_ldch_cfg {
 __u8 frm_end_dis;
 __u8 zero_interp_en;
 __u8 sample_avr_en;
 __u8 bic_mode_en;
 __u8 force_map_en;
 __u8 map13p3_en;

 __u8 bicubic[36];

 __u32 hsize;
 __u32 vsize;
 __s32 buf_fd;
} __attribute__ ((packed));

struct isp32_awb_gain_cfg {

 __u16 awb1_gain_gb;
 __u16 awb1_gain_gr;

 __u16 awb1_gain_b;
 __u16 awb1_gain_r;

 __u16 gain0_green_b;
 __u16 gain0_green_r;

 __u16 gain0_blue;
 __u16 gain0_red;

 __u16 gain1_green_b;
 __u16 gain1_green_r;

 __u16 gain1_blue;
 __u16 gain1_red;

 __u16 gain2_green_b;
 __u16 gain2_green_r;

 __u16 gain2_blue;
 __u16 gain2_red;
} __attribute__ ((packed));

struct isp32_bls_cfg {
 __u8 enable_auto;
 __u8 en_windows;
 __u8 bls1_en;

 __u8 bls_samples;

 struct isp2x_window bls_window1;
 struct isp2x_window bls_window2;
 struct isp2x_bls_fixed_val fixed_val;
 struct isp2x_bls_fixed_val bls1_val;

 __u16 isp_ob_offset;
 __u16 isp_ob_predgain;
 __u32 isp_ob_max;
} __attribute__ ((packed));

struct isp32_ccm_cfg {

 __u8 highy_adjust_dis;
 __u8 enh_adj_en;
 __u8 asym_adj_en;

 __u8 bound_bit;
 __u8 right_bit;

 __s16 coeff0_r;
 __s16 coeff1_r;

 __s16 coeff2_r;
 __s16 offset_r;

 __s16 coeff0_g;
 __s16 coeff1_g;

 __s16 coeff2_g;
 __s16 offset_g;

 __s16 coeff0_b;
 __s16 coeff1_b;

 __s16 coeff2_b;
 __s16 offset_b;

 __u16 coeff0_y;
 __u16 coeff1_y;

 __u16 coeff2_y;

 __u16 alp_y[18];

 __u16 color_coef0_r2y;
 __u16 color_coef1_g2y;

 __u16 color_coef2_b2y;
 __u16 color_enh_rat_max;
} __attribute__ ((packed));

struct isp32_debayer_cfg {

 __u8 filter_g_en;
 __u8 filter_c_en;

 __u8 clip_en;
 __u8 dist_scale;
 __u8 thed0;
 __u8 thed1;
 __u8 select_thed;
 __u8 max_ratio;

 __s8 filter1_coe1;
 __s8 filter1_coe2;
 __s8 filter1_coe3;
 __s8 filter1_coe4;

 __s8 filter2_coe1;
 __s8 filter2_coe2;
 __s8 filter2_coe3;
 __s8 filter2_coe4;

 __s8 guid_gaus_coe0;
 __s8 guid_gaus_coe1;
 __s8 guid_gaus_coe2;

 __s8 ce_gaus_coe0;
 __s8 ce_gaus_coe1;
 __s8 ce_gaus_coe2;

 __s8 alpha_gaus_coe0;
 __s8 alpha_gaus_coe1;
 __s8 alpha_gaus_coe2;

 __u8 ce_sgm;
 __u8 exp_shift;

 __u8 wet_clip;
 __u8 wet_ghost;

 __u8 bf_clip;
 __u8 bf_curwgt;
 __u16 bf_sgm;

 __u16 hf_offset;
 __u16 gain_offset;

 __u16 offset;

 __u16 loghf_offset;
 __u16 loggd_offset;

 __u16 wgtslope;

 __u16 alpha_offset;

 __u16 edge_offset;
 __u32 edge_scale;

 __u32 alpha_scale;
} __attribute__ ((packed));

struct isp32_baynr_cfg {

 __u8 bay3d_gain_en;
 __u8 lg2_mode;
 __u8 gauss_en;
 __u8 log_bypass;

 __u16 dgain1;
 __u16 dgain0;
 __u16 dgain2;

 __u16 pix_diff;

 __u16 diff_thld;
 __u16 softthld;

 __u16 bltflt_streng;
 __u16 reg_w1;

 __u16 sigma_x[16];
 __u16 sigma_y[16];

 __u16 weit_d2;
 __u16 weit_d1;
 __u16 weit_d0;

 __u16 lg2_lgoff;
 __u16 lg2_off;

 __u32 dat_max;

 __u16 rgain_off;
 __u16 bgain_off;

 __u8 gain_x[16];
 __u16 gain_y[16];
} __attribute__ ((packed));

struct isp32_bay3d_cfg {

 __u8 bypass_en;
 __u8 hibypass_en;
 __u8 lobypass_en;
 __u8 himed_bypass_en;
 __u8 higaus_bypass_en;
 __u8 hiabs_possel;
 __u8 hichnsplit_en;
 __u8 lomed_bypass_en;
 __u8 logaus5_bypass_en;
 __u8 logaus3_bypass_en;
 __u8 glbpk_en;
 __u8 loswitch_protect;
 __u8 bwsaving_en;

 __u8 hiwgt_opt_en;
 __u8 hichncor_en;
 __u8 bwopt_gain_dis;
 __u8 lo4x8_en;
 __u8 lo4x4_en;
 __u8 hisig_ind_sel;
 __u8 pksig_ind_sel;
 __u8 iirwr_rnd_en;
 __u8 curds_high_en;
 __u8 higaus3_mode;
 __u8 higaus5x5_en;
 __u8 wgtmix_opt_en;


 __u8 wgtmm_opt_en;
 __u8 wgtmm_sel_en;


 __u8 siggaus0;
 __u8 siggaus1;
 __u8 siggaus2;
 __u8 siggaus3;

 __u16 softwgt;
 __u16 hidif_th;

 __u16 wgtlmt;
 __u16 wgtratio;

 __u16 sig0_x[16];
 __u16 sig0_y[16];
 __u16 sig1_x[16];
 __u16 sig1_y[16];
 __u16 sig2_x[16];
 __u16 sig2_y[16];


 __u16 wgtmin;


 __u16 hisigrat0;
 __u16 hisigrat1;

 __u16 hisigoff0;
 __u16 hisigoff1;

 __u16 losigoff;
 __u16 losigrat;

 __u16 rgain_off;
 __u16 bgain_off;

 __u32 glbpk2;
} __attribute__ ((packed));

struct isp32_ynr_cfg {

 __u8 rnr_en;
 __u8 thumb_mix_cur_en;
 __u8 global_gain_alpha;
 __u8 flt1x1_bypass_sel;
 __u8 nlm11x11_bypass;
 __u8 flt1x1_bypass;
 __u8 lgft3x3_bypass;
 __u8 lbft5x5_bypass;
 __u8 bft3x3_bypass;

 __u8 rnr_strength3[17];

 __u8 nlm_hi_gain_alpha;

 __u8 nlm_coe[6];


 __u8 frame_add4line;

 __u16 global_gain;


 __u16 rnr_max_r;
 __u16 local_gainscale;

 __u16 rnr_center_coorh;
 __u16 rnr_center_coorv;

 __u16 loclagain_adj_thresh;
 __u16 localgain_adj;

 __u16 low_bf_inv1;
 __u16 low_bf_inv0;

 __u16 low_peak_supress;
 __u16 low_thred_adj;

 __u16 low_dist_adj;
 __u16 low_edge_adj_thresh;

 __u16 low_bi_weight;
 __u16 low_weight;
 __u16 low_center_weight;

 __u16 frame_full_size;
 __u16 lbf_weight_thres;

 __u16 low_gauss1_coeff2;
 __u16 low_gauss1_coeff1;
 __u16 low_gauss1_coeff0;

 __u16 low_gauss2_coeff2;
 __u16 low_gauss2_coeff1;
 __u16 low_gauss2_coeff0;

 __u16 luma_points_x[17];

 __u16 lsgm_y[17];

 __u16 nlm_min_sigma;
 __u16 nlm_hi_bf_scale;

 __u16 nlm_nr_weight;
 __u16 nlm_weight_offset;

 __u32 nlm_center_weight;
} __attribute__ ((packed));

struct isp32_cnr_cfg {

 __u8 exgain_bypass;
 __u8 yuv422_mode;
 __u8 thumb_mode;
 __u8 bf3x3_wgt0_sel;

 __u8 lbf1x7_weit_d0;
 __u8 lbf1x7_weit_d1;
 __u8 lbf1x7_weit_d2;
 __u8 lbf1x7_weit_d3;

 __u8 iir_uvgain;
 __u8 iir_strength;
 __u8 exp_shift;

 __u8 chroma_ghost;
 __u8 iir_uv_clip;

 __u8 gaus_coe[6];

 __u8 bf_wgt_clip;

 __u8 uv_gain;
 __u8 bf_ratio;

 __u8 sigma_y[13];

 __u8 iir_gain_alpha;
 __u8 iir_global_gain;

 __u8 gain_iso;
 __u8 global_gain_alpha;
 __u16 global_gain;

 __u16 thumb_sigma_c;
 __u16 thumb_sigma_y;

 __u16 thumb_bf_ratio;

 __u16 wgt_slope;

 __u16 gaus_ratio;
 __u16 global_alpha;

 __u16 sigma_r;

 __u16 adj_offset;
 __u16 adj_ratio;
} __attribute__ ((packed));

struct isp32_sharp_cfg {

 __u8 bypass;
 __u8 center_mode;
 __u8 exgain_bypass;
 __u8 radius_ds_mode;
 __u8 noiseclip_mode;


 __u8 clip_hf_mode;
 __u8 add_mode;


 __u8 sharp_ratio;
 __u8 bf_ratio;
 __u8 gaus_ratio;
 __u8 pbf_ratio;

 __u8 luma_dx[7];

 __u8 bf_sigma_shift;
 __u8 pbf_sigma_shift;

 __u8 pbf_coef2;
 __u8 pbf_coef1;
 __u8 pbf_coef0;

 __u8 bf_coef2;
 __u8 bf_coef1;
 __u8 bf_coef0;

 __u8 gaus_coef[6];

 __u8 global_gain_alpha;
 __u8 local_gainscale;

 __u8 strength[22];

 __u8 enhance_bit;

 __u16 pbf_sigma_inv[8];

 __u16 bf_sigma_inv[8];

 __u16 clip_hf[8];

 __u16 global_gain;

 __u16 gain_adj[14];

 __u16 center_wid;
 __u16 center_het;

 __u16 noise_sigma;
 __u16 noise_strength;


 __u16 ehf_th[8];

 __u16 clip_neg[8];
} __attribute__ ((packed));

struct isp32_dhaz_cfg {

 __u8 enh_luma_en;
 __u8 color_deviate_en;
 __u8 round_en;
 __u8 soft_wr_en;
 __u8 enhance_en;
 __u8 air_lc_en;
 __u8 hpara_en;
 __u8 hist_en;
 __u8 dc_en;

 __u8 yblk_th;
 __u8 yhist_th;
 __u8 dc_max_th;
 __u8 dc_min_th;

 __u8 tmax_base;
 __u8 dark_th;
 __u8 air_max;
 __u8 air_min;

 __u8 gaus_h2;
 __u8 gaus_h1;
 __u8 gaus_h0;

 __u8 sigma_idx[15];

 __u8 hist_gratio;
 __u16 hist_scale;

 __u8 bright_max;
 __u8 bright_min;
 __u16 wt_max;

 __u16 tmax_max;
 __u16 tmax_off;

 __u8 hist_k;
 __u8 hist_th_off;
 __u16 hist_min;

 __u16 enhance_value;
 __u16 enhance_chroma;

 __u16 iir_wt_sigma;
 __u8 iir_sigma;
 __u8 stab_fnum;

 __u16 iir_tmax_sigma;
 __u8 iir_air_sigma;
 __u8 iir_pre_wet;

 __u16 cfg_wt;
 __u8 cfg_air;
 __u8 cfg_alpha;

 __u16 cfg_gratio;
 __u16 cfg_tmax;

 __u16 range_sima;
 __u8 space_sigma_pre;
 __u8 space_sigma_cur;

 __u16 dc_weitcur;
 __u16 bf_weight;

 __u16 enh_curve[17];

 __u16 sigma_lut[17];

 __u16 hist_wr[64];

 __u16 enh_luma[17];
} __attribute__ ((packed));

struct isp32_drc_cfg {
 __u8 bypass_en;

 __u8 offset_pow2;
 __u16 compres_scl;
 __u16 position;

 __u16 hpdetail_ratio;
 __u16 lpdetail_ratio;
 __u8 delta_scalein;

 __u8 weicur_pix;
 __u8 weipre_frame;
 __u8 bilat_wt_off;

 __u8 edge_scl;
 __u8 motion_scl;
 __u16 force_sgm_inv0;

 __u16 space_sgm_inv1;
 __u16 space_sgm_inv0;

 __u16 range_sgm_inv1;
 __u16 range_sgm_inv0;

 __u16 bilat_soft_thd;
 __u8 weig_maxl;
 __u8 weig_bilat;
 __u8 enable_soft_thd;

 __u8 iir_weight;
 __u16 min_ogain;

 __u16 gas_t;

 __u8 gas_l0;
 __u8 gas_l1;
 __u8 gas_l2;
 __u8 gas_l3;

 __u16 gain_y[17];
 __u16 compres_y[17];
 __u16 scale_y[17];
} __attribute__ ((packed));

struct isp32_hdrmge_cfg {
 __u8 s_base;
 __u8 mode;
 __u8 dbg_mode;
 __u8 each_raw_en;

 __u8 gain2;

 __u8 lm_dif_0p15;
 __u8 lm_dif_0p9;
 __u8 ms_diff_0p15;
 __u8 ms_dif_0p8;

 __u16 gain0_inv;
 __u16 gain0;
 __u16 gain1_inv;
 __u16 gain1;

 __u16 ms_thd1;
 __u16 ms_thd0;
 __u16 ms_scl;
 __u16 lm_thd1;
 __u16 lm_thd0;
 __u16 lm_scl;
 struct isp2x_hdrmge_curve curve;
 __u16 e_y[17];
 __u16 l_raw0[17];
 __u16 l_raw1[17];
 __u16 each_raw_gain0;
 __u16 each_raw_gain1;
} __attribute__ ((packed));

struct isp32_rawawb_meas_cfg {
 __u8 bls2_en;

 __u8 rawawb_sel;
 __u8 bnr2awb_sel;
 __u8 drc2awb_sel;

 __u8 uv_en0;
 __u8 xy_en0;
 __u8 yuv3d_en0;
 __u8 yuv3d_ls_idx0;
 __u8 yuv3d_ls_idx1;
 __u8 yuv3d_ls_idx2;
 __u8 yuv3d_ls_idx3;
 __u8 in_rshift_to_12bit_en;
 __u8 in_overexposure_check_en;
 __u8 wind_size;
 __u8 rawlsc_bypass_en;
 __u8 light_num;
 __u8 uv_en1;
 __u8 xy_en1;
 __u8 yuv3d_en1;
 __u8 low12bit_val;

 __u8 wp_luma_wei_en0;
 __u8 wp_luma_wei_en1;
 __u8 wp_blk_wei_en0;
 __u8 wp_blk_wei_en1;
 __u8 wp_hist_xytype;

 __u8 exc_wp_region0_excen;
 __u8 exc_wp_region0_measen;
 __u8 exc_wp_region0_domain;
 __u8 exc_wp_region1_excen;
 __u8 exc_wp_region1_measen;
 __u8 exc_wp_region1_domain;
 __u8 exc_wp_region2_excen;
 __u8 exc_wp_region2_measen;
 __u8 exc_wp_region2_domain;
 __u8 exc_wp_region3_excen;
 __u8 exc_wp_region3_measen;
 __u8 exc_wp_region3_domain;
 __u8 exc_wp_region4_excen;
 __u8 exc_wp_region4_domain;
 __u8 exc_wp_region5_excen;
 __u8 exc_wp_region5_domain;
 __u8 exc_wp_region6_excen;
 __u8 exc_wp_region6_domain;
 __u8 multiwindow_en;

 __u8 wp_luma_weicurve_y0;
 __u8 wp_luma_weicurve_y1;
 __u8 wp_luma_weicurve_y2;
 __u8 wp_luma_weicurve_y3;

 __u8 wp_luma_weicurve_y4;
 __u8 wp_luma_weicurve_y5;
 __u8 wp_luma_weicurve_y6;
 __u8 wp_luma_weicurve_y7;

 __u8 wp_luma_weicurve_y8;

 __u8 wp_luma_weicurve_w0;
 __u8 wp_luma_weicurve_w1;
 __u8 wp_luma_weicurve_w2;
 __u8 wp_luma_weicurve_w3;

 __u8 wp_luma_weicurve_w4;
 __u8 wp_luma_weicurve_w5;
 __u8 wp_luma_weicurve_w6;
 __u8 wp_luma_weicurve_w7;

 __u8 wp_luma_weicurve_w8;

 __u8 dis_x1x2_ls0;
 __u8 rotu0_ls0;
 __u8 rotu1_ls0;

 __u8 rotu2_ls0;
 __u8 rotu3_ls0;
 __u8 rotu4_ls0;
 __u8 rotu5_ls0;

 __u8 dis_x1x2_ls1;
 __u8 rotu0_ls1;
 __u8 rotu1_ls1;

 __u8 rotu2_ls1;
 __u8 rotu3_ls1;
 __u8 rotu4_ls1;
 __u8 rotu5_ls1;

 __u8 dis_x1x2_ls2;
 __u8 rotu0_ls2;
 __u8 rotu1_ls2;

 __u8 rotu2_ls2;
 __u8 rotu3_ls2;
 __u8 rotu4_ls2;
 __u8 rotu5_ls2;

 __u8 dis_x1x2_ls3;
 __u8 rotu0_ls3;
 __u8 rotu1_ls3;
 __u8 rotu2_ls3;
 __u8 rotu3_ls3;
 __u8 rotu4_ls3;
 __u8 rotu5_ls3;

 __u8 exc_wp_region0_weight;
 __u8 exc_wp_region1_weight;
 __u8 exc_wp_region2_weight;
 __u8 exc_wp_region3_weight;
 __u8 exc_wp_region4_weight;
 __u8 exc_wp_region5_weight;
 __u8 exc_wp_region6_weight;

 __u8 wp_blk_wei_w[225];

 __u8 blk_measure_enable;
 __u8 blk_measure_mode;
 __u8 blk_measure_xytype;
 __u8 blk_rtdw_measure_en;
 __u8 blk_measure_illu_idx;


 __u8 ds16x8_mode_en;

 __u8 blk_with_luma_wei_en;
 __u16 in_overexposure_threshold;

 __u16 r_max;
 __u16 g_max;

 __u16 b_max;
 __u16 y_max;

 __u16 r_min;
 __u16 g_min;

 __u16 b_min;
 __u16 y_min;

 __u16 h_offs;
 __u16 v_offs;

 __u16 h_size;
 __u16 v_size;

 __u16 pre_wbgain_inv_r;

 __u16 pre_wbgain_inv_g;
 __u16 pre_wbgain_inv_b;

 __u16 vertex0_u_0;
 __u16 vertex0_v_0;

 __u16 vertex1_u_0;
 __u16 vertex1_v_0;

 __u16 vertex2_u_0;
 __u16 vertex2_v_0;

 __u16 vertex3_u_0;
 __u16 vertex3_v_0;

 __u16 vertex0_u_1;
 __u16 vertex0_v_1;

 __u16 vertex1_u_1;
 __u16 vertex1_v_1;

 __u16 vertex2_u_1;
 __u16 vertex2_v_1;

 __u16 vertex3_u_1;
 __u16 vertex3_v_1;

 __u16 vertex0_u_2;
 __u16 vertex0_v_2;

 __u16 vertex1_u_2;
 __u16 vertex1_v_2;

 __u16 vertex2_u_2;
 __u16 vertex2_v_2;

 __u16 vertex3_u_2;
 __u16 vertex3_v_2;

 __u16 vertex0_u_3;
 __u16 vertex0_v_3;

 __u16 vertex1_u_3;
 __u16 vertex1_v_3;

 __u16 vertex2_u_3;
 __u16 vertex2_v_3;

 __u16 vertex3_u_3;
 __u16 vertex3_v_3;

 __u16 wt0;
 __u16 wt1;
 __u16 wt2;

 __u16 mat0_x;
 __u16 mat0_y;

 __u16 mat1_x;
 __u16 mat1_y;

 __u16 mat2_x;
 __u16 mat2_y;

 __u16 nor_x0_0;
 __u16 nor_x1_0;
 __u16 nor_y0_0;
 __u16 nor_y1_0;

 __u16 nor_x0_1;
 __u16 nor_x1_1;
 __u16 nor_y0_1;
 __u16 nor_y1_1;

 __u16 nor_x0_2;
 __u16 nor_x1_2;
 __u16 nor_y0_2;
 __u16 nor_y1_2;

 __u16 nor_x0_3;
 __u16 nor_x1_3;
 __u16 nor_y0_3;
 __u16 nor_y1_3;

 __u16 big_x0_0;
 __u16 big_x1_0;
 __u16 big_y0_0;
 __u16 big_y1_0;

 __u16 big_x0_1;
 __u16 big_x1_1;
 __u16 big_y0_1;
 __u16 big_y1_1;

 __u16 big_x0_2;
 __u16 big_x1_2;
 __u16 big_y0_2;
 __u16 big_y1_2;

 __u16 big_x0_3;
 __u16 big_x1_3;
 __u16 big_y0_3;
 __u16 big_y1_3;

 __u16 multiwindow0_v_offs;
 __u16 multiwindow0_h_offs;
 __u16 multiwindow0_v_size;
 __u16 multiwindow0_h_size;

 __u16 multiwindow1_v_offs;
 __u16 multiwindow1_h_offs;
 __u16 multiwindow1_v_size;
 __u16 multiwindow1_h_size;

 __u16 multiwindow2_v_offs;
 __u16 multiwindow2_h_offs;
 __u16 multiwindow2_v_size;
 __u16 multiwindow2_h_size;

 __u16 multiwindow3_v_offs;
 __u16 multiwindow3_h_offs;
 __u16 multiwindow3_v_size;
 __u16 multiwindow3_h_size;

 __u16 exc_wp_region0_xu0;
 __u16 exc_wp_region0_xu1;

 __u16 exc_wp_region0_yv0;
 __u16 exc_wp_region0_yv1;

 __u16 exc_wp_region1_xu0;
 __u16 exc_wp_region1_xu1;

 __u16 exc_wp_region1_yv0;
 __u16 exc_wp_region1_yv1;

 __u16 exc_wp_region2_xu0;
 __u16 exc_wp_region2_xu1;

 __u16 exc_wp_region2_yv0;
 __u16 exc_wp_region2_yv1;

 __u16 exc_wp_region3_xu0;
 __u16 exc_wp_region3_xu1;

 __u16 exc_wp_region3_yv0;
 __u16 exc_wp_region3_yv1;

 __u16 exc_wp_region4_xu0;
 __u16 exc_wp_region4_xu1;

 __u16 exc_wp_region4_yv0;
 __u16 exc_wp_region4_yv1;

 __u16 exc_wp_region5_xu0;
 __u16 exc_wp_region5_xu1;

 __u16 exc_wp_region5_yv0;
 __u16 exc_wp_region5_yv1;

 __u16 exc_wp_region6_xu0;
 __u16 exc_wp_region6_xu1;

 __u16 exc_wp_region6_yv0;
 __u16 exc_wp_region6_yv1;

 __u16 rgb2ryuvmat0_y;
 __u16 rgb2ryuvmat1_y;
 __u16 rgb2ryuvmat2_y;
 __u16 rgb2ryuvofs_y;

 __u16 rgb2ryuvmat0_u;
 __u16 rgb2ryuvmat1_u;
 __u16 rgb2ryuvmat2_u;
 __u16 rgb2ryuvofs_u;

 __u16 rgb2ryuvmat0_v;
 __u16 rgb2ryuvmat1_v;
 __u16 rgb2ryuvmat2_v;
 __u16 rgb2ryuvofs_v;

 __u16 coor_x1_ls0_y;
 __u16 vec_x21_ls0_y;
 __u16 coor_x1_ls0_u;
 __u16 vec_x21_ls0_u;
 __u16 coor_x1_ls0_v;
 __u16 vec_x21_ls0_v;

 __u16 coor_x1_ls1_y;
 __u16 vec_x21_ls1_y;
 __u16 coor_x1_ls1_u;
 __u16 vec_x21_ls1_u;
 __u16 coor_x1_ls1_v;
 __u16 vec_x21_ls1_v;

 __u16 coor_x1_ls2_y;
 __u16 vec_x21_ls2_y;
 __u16 coor_x1_ls2_u;
 __u16 vec_x21_ls2_v;
 __u16 coor_x1_ls2_v;
 __u16 vec_x21_ls2_u;

 __u16 coor_x1_ls3_y;
 __u16 vec_x21_ls3_y;
 __u16 coor_x1_ls3_u;
 __u16 vec_x21_ls3_u;
 __u16 coor_x1_ls3_v;
 __u16 vec_x21_ls3_v;

 __u16 th0_ls0;
 __u16 th1_ls0;
 __u16 th2_ls0;
 __u16 th3_ls0;
 __u16 th4_ls0;
 __u16 th5_ls0;

 __u16 th0_ls1;
 __u16 th1_ls1;
 __u16 th2_ls1;
 __u16 th3_ls1;
 __u16 th4_ls1;
 __u16 th5_ls1;

 __u16 th0_ls2;
 __u16 th1_ls2;
 __u16 th2_ls2;
 __u16 th3_ls2;
 __u16 th4_ls2;
 __u16 th5_ls2;

 __u16 th0_ls3;
 __u16 th1_ls3;
 __u16 th2_ls3;
 __u16 th3_ls3;
 __u16 th4_ls3;
 __u16 th5_ls3;

 __u32 islope01_0;
 __u32 islope12_0;
 __u32 islope23_0;
 __u32 islope30_0;
 __u32 islope01_1;
 __u32 islope12_1;
 __u32 islope23_1;
 __u32 islope30_1;
 __u32 islope01_2;
 __u32 islope12_2;
 __u32 islope23_2;
 __u32 islope30_2;
 __u32 islope01_3;
 __u32 islope12_3;
 __u32 islope23_3;
 __u32 islope30_3;


 __u32 win_weight[5];
 struct isp2x_bls_fixed_val bls2_val;
} __attribute__ ((packed));

struct isp32_rawaf_meas_cfg {
 __u8 rawaf_sel;
 __u8 num_afm_win;

 __u8 bnr2af_sel;


 __u8 gamma_en;
 __u8 gaus_en;
 __u8 v1_fir_sel;
 __u8 hiir_en;
 __u8 viir_en;
 __u8 accu_8bit_mode;
 __u8 ldg_en;
 __u8 h1_fv_mode;
 __u8 h2_fv_mode;
 __u8 v1_fv_mode;
 __u8 v2_fv_mode;
 __u8 ae_mode;
 __u8 y_mode;
 __u8 vldg_sel;
 __u8 sobel_sel;
 __u8 v_dnscl_mode;
 __u8 from_awb;
 __u8 from_ynr;
 __u8 ae_config_use;

 __u8 ae_sel;


 __u8 hiir_left_border_mode;
 __u8 avg_ds_en;
 __u8 avg_ds_mode;
 __u8 h1_acc_mode;
 __u8 h2_acc_mode;
 __u8 v1_acc_mode;
 __u8 v2_acc_mode;


 __s16 bls_offset;
 __u8 bls_en;
 __u8 hldg_dilate_num;


 struct isp2x_window win[2];

 __u8 line_num[5];
 __u8 line_en[5];

 __u16 afm_thres;

 __u8 afm_var_shift[2];
 __u8 lum_var_shift[2];

 __u8 tnrin_shift;


 __u8 h1iir_var_shift;
 __u8 h2iir_var_shift;
 __u8 v1iir_var_shift;
 __u8 v2iir_var_shift;

 __s8 gaus_coe[9];


 __u16 gamma_y[17];

 __u16 h_fv_thresh;
 __u16 v_fv_thresh;
 struct isp3x_rawaf_curve curve_h[2];
 struct isp3x_rawaf_curve curve_v[2];
 __s16 h1iir1_coe[6];
 __s16 h1iir2_coe[6];
 __s16 h2iir1_coe[6];
 __s16 h2iir2_coe[6];
 __s16 v1iir_coe[3];
 __s16 v2iir_coe[3];
 __s16 v1fir_coe[3];
 __s16 v2fir_coe[3];
 __u16 highlit_thresh;


 __u16 h_fv_limit;
 __u16 h_fv_slope;

 __u16 v_fv_limit;
 __u16 v_fv_slope;
} __attribute__ ((packed));

struct isp32_cac_cfg {
 __u8 bypass_en;
 __u8 center_en;
 __u8 clip_g_mode;
 __u8 edge_detect_en;
 __u8 neg_clip0_en;

 __u8 flat_thed_b;
 __u8 flat_thed_r;

 __u8 psf_sft_bit;
 __u16 cfg_num;

 __u16 center_width;
 __u16 center_height;

 __u16 strength[22];

 __u16 offset_b;
 __u16 offset_r;

 __u32 expo_thed_b;
 __u32 expo_thed_r;
 __u32 expo_adj_b;
 __u32 expo_adj_r;

 __u32 hsize;
 __u32 vsize;
 __s32 buf_fd;
} __attribute__ ((packed));

struct isp32_vsm_cfg {
 __u8 h_segments;
 __u8 v_segments;
 __u16 h_offs;
 __u16 v_offs;
 __u16 h_size;
 __u16 v_size;
} __attribute__ ((packed));

struct isp32_isp_other_cfg {
 struct isp32_bls_cfg bls_cfg;
 struct isp2x_dpcc_cfg dpcc_cfg;
 struct isp3x_lsc_cfg lsc_cfg;
 struct isp32_awb_gain_cfg awb_gain_cfg;
 struct isp21_gic_cfg gic_cfg;
 struct isp32_debayer_cfg debayer_cfg;
 struct isp32_ccm_cfg ccm_cfg;
 struct isp3x_gammaout_cfg gammaout_cfg;
 struct isp2x_cproc_cfg cproc_cfg;
 struct isp2x_ie_cfg ie_cfg;
 struct isp2x_sdg_cfg sdg_cfg;
 struct isp32_drc_cfg drc_cfg;
 struct isp32_hdrmge_cfg hdrmge_cfg;
 struct isp32_dhaz_cfg dhaz_cfg;
 struct isp2x_3dlut_cfg isp3dlut_cfg;
 struct isp32_ldch_cfg ldch_cfg;
 struct isp32_baynr_cfg baynr_cfg;
 struct isp32_bay3d_cfg bay3d_cfg;
 struct isp32_ynr_cfg ynr_cfg;
 struct isp32_cnr_cfg cnr_cfg;
 struct isp32_sharp_cfg sharp_cfg;
 struct isp32_cac_cfg cac_cfg;
 struct isp3x_gain_cfg gain_cfg;
 struct isp21_csm_cfg csm_cfg;
 struct isp21_cgc_cfg cgc_cfg;
 struct isp32_vsm_cfg vsm_cfg;
} __attribute__ ((packed));

struct isp32_isp_meas_cfg {
 struct isp32_rawaf_meas_cfg rawaf;
 struct isp32_rawawb_meas_cfg rawawb;
 struct isp2x_rawaelite_meas_cfg rawae0;
 struct isp2x_rawaebig_meas_cfg rawae1;
 struct isp2x_rawaebig_meas_cfg rawae2;
 struct isp2x_rawaebig_meas_cfg rawae3;
 struct isp2x_rawhistlite_cfg rawhist0;
 struct isp2x_rawhistbig_cfg rawhist1;
 struct isp2x_rawhistbig_cfg rawhist2;
 struct isp2x_rawhistbig_cfg rawhist3;
} __attribute__ ((packed));

struct isp32_rawae_meas_data {
 __u32 channelg_xy:12;
 __u32 channelb_xy:10;
 __u32 channelr_xy:10;
} __attribute__ ((packed));

struct isp32_rawaebig_stat0 {
 struct isp32_rawae_meas_data data[225];
 __u32 reserved[3];
} __attribute__ ((packed));

struct isp32_rawaebig_stat1 {
 __u32 sumr[4];
 __u32 sumg[4];
 __u32 sumb[4];
} __attribute__ ((packed));

struct isp32_rawaelite_stat {
 struct isp32_rawae_meas_data data[25];
 __u32 reserved[21];
} __attribute__ ((packed));

struct isp32_rawaf_stat {
 struct isp3x_rawaf_ramdata ramdata[225];
 __u32 int_state;
 __u32 afm_sum_b;
 __u32 afm_lum_b;
 __u32 highlit_cnt_winb;
 __u32 reserved[18];
} __attribute__ ((packed));

struct isp32_rawawb_ramdata {
 __u64 b:18;
 __u64 g:18;
 __u64 r:18;
 __u64 wp:10;
} __attribute__ ((packed));

struct isp32_rawawb_sum {
 __u32 rgain_nor;
 __u32 bgain_nor;
 __u32 wp_num_nor;
 __u32 wp_num2;

 __u32 rgain_big;
 __u32 bgain_big;
 __u32 wp_num_big;
 __u32 reserved;
} __attribute__ ((packed));

struct isp32_rawawb_sum_exc {
 __u32 rgain_exc;
 __u32 bgain_exc;
 __u32 wp_num_exc;
 __u32 reserved;
} __attribute__ ((packed));

struct isp32_rawawb_meas_stat {
 struct isp32_rawawb_ramdata ramdata[225];
 __u64 reserved;
 struct isp32_rawawb_sum sum[4];
 __u16 yhist_bin[8];
 struct isp32_rawawb_sum_exc sum_exc[4];
} __attribute__ ((packed));

struct isp32_vsm_stat {
 __u16 delta_h;
 __u16 delta_v;
} __attribute__ ((packed));

struct isp32_info2ddr_stat {
 __u32 owner;
 __s32 buf_fd;
} __attribute__ ((packed));

struct isp32_isp_params_cfg {
 __u64 module_en_update;
 __u64 module_ens;
 __u64 module_cfg_update;

 __u32 frame_id;
 struct isp32_isp_meas_cfg meas;
 struct isp32_isp_other_cfg others;
} __attribute__ ((packed));

struct isp32_stat {
 struct isp32_rawaebig_stat0 rawae3_0;
 struct isp32_rawaebig_stat0 rawae1_0;
 struct isp32_rawaebig_stat0 rawae2_0;
 struct isp32_rawaelite_stat rawae0;
 struct isp32_rawaebig_stat1 rawae3_1;
 struct isp32_rawaebig_stat1 rawae1_1;
 struct isp32_rawaebig_stat1 rawae2_1;
 struct isp2x_bls_stat bls;
 struct isp2x_rawhistbig_stat rawhist3;
 struct isp2x_rawhistlite_stat rawhist0;
 struct isp2x_rawhistbig_stat rawhist1;
 struct isp2x_rawhistbig_stat rawhist2;
 struct isp32_rawaf_stat rawaf;
 struct isp3x_dhaz_stat dhaz;
 struct isp32_vsm_stat vsm;
 struct isp32_info2ddr_stat info2ddr;
 struct isp32_rawawb_meas_stat rawawb;
} __attribute__ ((packed));

struct rkisp32_isp_stat_buffer {
 struct isp32_stat params;
 __u32 meas_type;
 __u32 frame_id;
 __u32 params_id;
} __attribute__ ((packed));

struct rkisp32_thunderboot_resmem_head {
 struct rkisp_thunderboot_resmem_head head;
 struct isp32_isp_params_cfg cfg;
} __attribute__ ((packed));



struct isp32_lite_rawaebig_stat {
 __u32 sumr;
 __u32 sumg;
 __u32 sumb;
 struct isp2x_rawae_meas_data data[225];
} __attribute__ ((packed));

struct isp32_lite_rawawb_meas_stat {
 __u32 ramdata_r[25];
 __u32 ramdata_g[25];
 __u32 ramdata_b[25];
 __u32 ramdata_wpnum0[13];
 __u32 ramdata_wpnum1[13];
 struct isp32_rawawb_sum sum[4];
 __u16 yhist_bin[8];
 struct isp32_rawawb_sum_exc sum_exc[4];
} __attribute__ ((packed));

struct isp32_lite_rawaf_ramdata {
 __u32 hiir_wnd_data[25];
 __u32 viir_wnd_data[25];
} __attribute__ ((packed));

struct isp32_lite_rawaf_stat {
 struct isp32_lite_rawaf_ramdata ramdata;
 __u32 int_state;
 __u32 afm_sum_b;
 __u32 afm_lum_b;
 __u32 highlit_cnt_winb;
} __attribute__ ((packed));

struct isp32_lite_rawhistlite_stat {
 __u32 hist_bin[64];
} __attribute__ ((packed));

struct isp32_lite_stat {
 struct isp2x_bls_stat bls;
 struct isp3x_dhaz_stat dhaz;
 struct isp32_info2ddr_stat info2ddr;
 struct isp2x_rawaelite_stat rawae0;
 struct isp32_lite_rawaebig_stat rawae3;
 struct isp32_lite_rawhistlite_stat rawhist0;
 struct isp2x_rawhistbig_stat rawhist3;
 struct isp32_lite_rawaf_stat rawaf;
 struct isp32_lite_rawawb_meas_stat rawawb;
} __attribute__ ((packed));

struct rkisp32_lite_stat_buffer {
 struct isp32_lite_stat params;
 __u32 meas_type;
 __u32 frame_id;
 __u32 params_id;
} __attribute__ ((packed));


struct isp39_gic_cfg {

 __u8 bypass_en;

 __u16 regmingradthrdark2;
 __u16 regmingradthrdark1;
 __u16 regminbusythre;

 __u16 regdarkthre;
 __u16 regmaxcorvboth;
 __u16 regdarktthrehi;

 __u8 regkgrad2dark;
 __u8 regkgrad1dark;
 __u8 regstrengthglobal_fix;
 __u8 regdarkthrestep;
 __u8 regkgrad2;
 __u8 regkgrad1;
 __u8 reggbthre;

 __u16 regmaxcorv;
 __u16 regmingradthr2;
 __u16 regmingradthr1;

 __u8 gr_ratio;
 __u8 noise_scale;
 __u16 noise_base;

 __u16 diff_clip;

 __u16 sigma_y[15];
} __attribute__ ((packed));

struct isp39_dpcc_cfg {

 __u8 stage1_enable;
 __u8 grayscale_mode;

 __u8 border_bypass_mode;
 __u8 sw_rk_out_sel;
 __u8 sw_dpcc_output_sel;
 __u8 stage1_rb_3x3;
 __u8 stage1_g_3x3;
 __u8 stage1_incl_rb_center;
 __u8 stage1_incl_green_center;

 __u8 stage1_use_fix_set;
 __u8 stage1_use_set_3;
 __u8 stage1_use_set_2;
 __u8 stage1_use_set_1;

 __u8 sw_rk_red_blue1_en;
 __u8 rg_red_blue1_enable;
 __u8 rnd_red_blue1_enable;
 __u8 ro_red_blue1_enable;
 __u8 lc_red_blue1_enable;
 __u8 pg_red_blue1_enable;
 __u8 sw_rk_green1_en;
 __u8 rg_green1_enable;
 __u8 rnd_green1_enable;
 __u8 ro_green1_enable;
 __u8 lc_green1_enable;
 __u8 pg_green1_enable;

 __u8 sw_rk_red_blue2_en;
 __u8 rg_red_blue2_enable;
 __u8 rnd_red_blue2_enable;
 __u8 ro_red_blue2_enable;
 __u8 lc_red_blue2_enable;
 __u8 pg_red_blue2_enable;
 __u8 sw_rk_green2_en;
 __u8 rg_green2_enable;
 __u8 rnd_green2_enable;
 __u8 ro_green2_enable;
 __u8 lc_green2_enable;
 __u8 pg_green2_enable;

 __u8 sw_rk_red_blue3_en;
 __u8 rg_red_blue3_enable;
 __u8 rnd_red_blue3_enable;
 __u8 ro_red_blue3_enable;
 __u8 lc_red_blue3_enable;
 __u8 pg_red_blue3_enable;
 __u8 sw_rk_green3_en;
 __u8 rg_green3_enable;
 __u8 rnd_green3_enable;
 __u8 ro_green3_enable;
 __u8 lc_green3_enable;
 __u8 pg_green3_enable;

 __u8 sw_mindis1_rb;
 __u8 sw_mindis1_g;
 __u8 line_thr_1_rb;
 __u8 line_thr_1_g;

 __u8 sw_dis_scale_min1;
 __u8 sw_dis_scale_max1;
 __u8 line_mad_fac_1_rb;
 __u8 line_mad_fac_1_g;

 __u8 pg_fac_1_rb;
 __u8 pg_fac_1_g;

 __u8 rnd_thr_1_rb;
 __u8 rnd_thr_1_g;

 __u8 rg_fac_1_rb;
 __u8 rg_fac_1_g;

 __u8 sw_mindis2_rb;
 __u8 sw_mindis2_g;
 __u8 line_thr_2_rb;
 __u8 line_thr_2_g;

 __u8 sw_dis_scale_min2;
 __u8 sw_dis_scale_max2;
 __u8 line_mad_fac_2_rb;
 __u8 line_mad_fac_2_g;

 __u8 pg_fac_2_rb;
 __u8 pg_fac_2_g;

 __u8 rnd_thr_2_rb;
 __u8 rnd_thr_2_g;

 __u8 rg_fac_2_rb;
 __u8 rg_fac_2_g;

 __u8 sw_mindis3_rb;
 __u8 sw_mindis3_g;
 __u8 line_thr_3_rb;
 __u8 line_thr_3_g;

 __u8 sw_dis_scale_min3;
 __u8 sw_dis_scale_max3;
 __u8 line_mad_fac_3_rb;
 __u8 line_mad_fac_3_g;

 __u8 pg_fac_3_rb;
 __u8 pg_fac_3_g;

 __u8 rnd_thr_3_rb;
 __u8 rnd_thr_3_g;

 __u8 rg_fac_3_rb;
 __u8 rg_fac_3_g;

 __u8 ro_lim_3_rb;
 __u8 ro_lim_3_g;
 __u8 ro_lim_2_rb;
 __u8 ro_lim_2_g;
 __u8 ro_lim_1_rb;
 __u8 ro_lim_1_g;

 __u8 rnd_offs_3_rb;
 __u8 rnd_offs_3_g;
 __u8 rnd_offs_2_rb;
 __u8 rnd_offs_2_g;
 __u8 rnd_offs_1_rb;
 __u8 rnd_offs_1_g;

 __u8 bpt_rb_3x3;
 __u8 bpt_g_3x3;
 __u8 bpt_incl_rb_center;
 __u8 bpt_incl_green_center;
 __u8 bpt_use_fix_set;
 __u8 bpt_use_set_3;
 __u8 bpt_use_set_2;
 __u8 bpt_use_set_1;
 __u8 bpt_cor_en;
 __u8 bpt_det_en;

 __u16 bp_number;

 __u16 bp_table_addr;

 __u16 bpt_v_addr;
 __u16 bpt_h_addr;

 __u32 bp_cnt;

 __u8 sw_pdaf_en;

 __u8 pdaf_point_en[16];

 __u16 pdaf_offsety;
 __u16 pdaf_offsetx;

 __u16 pdaf_wrapy;
 __u16 pdaf_wrapx;

 __u16 pdaf_wrapy_num;
 __u16 pdaf_wrapx_num;

 struct isp2x_dpcc_pdaf_point point[16];

 __u8 pdaf_forward_med;
} __attribute__ ((packed));

struct isp39_yuvme_cfg {

 __u8 bypass;
 __u8 tnr_wgt0_en;

 __u8 global_nr_strg;
 __u8 wgt_fact3;
 __u8 search_range_mode;
 __u8 static_detect_thred;

 __u8 time_relevance_offset;
 __u8 space_relevance_offset;
 __u8 nr_diff_scale;
 __u16 nr_fusion_limit;

 __u8 nr_static_scale;
 __u16 nr_motion_scale;
 __u8 nr_fusion_mode;
 __u16 cur_weight_limit;

 __u16 nr_luma2sigma_val[16];
} __attribute__ ((packed));

struct isp39_rgbir_cfg {

 __u16 coe_theta;

 __u16 coe_delta;

 __u16 scale[4];

 __u16 luma_point[17];

 __u16 scale_map[17];
} __attribute__ ((packed));

struct isp39_ldcv_cfg {

 __u8 thumb_mode;
 __u8 dth_bypass;
 __u8 force_map_en;
 __u8 map13p3_en;


 __u8 bicubic[36];


 __u16 last_offset;

 __u32 out_vsize;

 __u32 hsize;
 __u32 vsize;
 __s32 buf_fd;
} __attribute__ ((packed));

struct isp39_ldch_cfg {
 __u8 frm_end_dis;
 __u8 sample_avr_en;
 __u8 bic_mode_en;
 __u8 force_map_en;
 __u8 map13p3_en;

 __u8 bicubic[36];

 __u16 out_hsize;

 __u32 hsize;
 __u32 vsize;
 __s32 buf_fd;
} __attribute__ ((packed));

struct isp39_ccm_cfg {

 __u8 highy_adjust_dis;
 __u8 enh_adj_en;
 __u8 asym_adj_en;
 __u8 sat_decay_en;

 __s16 coeff0_r;
 __s16 coeff1_r;

 __s16 coeff2_r;
 __s16 offset_r;

 __s16 coeff0_g;
 __s16 coeff1_g;

 __s16 coeff2_g;
 __s16 offset_g;

 __s16 coeff0_b;
 __s16 coeff1_b;

 __s16 coeff2_b;
 __s16 offset_b;

 __u16 coeff0_y;
 __u16 coeff1_y;

 __u16 coeff2_y;

 __u16 alp_y[18];

 __u8 bound_bit;
 __u8 right_bit;

 __u16 color_coef0_r2y;
 __u16 color_coef1_g2y;

 __u16 color_coef2_b2y;
 __u16 color_enh_rat_max;

 __u8 hf_low;
 __u8 hf_up;
 __u16 hf_scale;

 __u16 hf_factor[17];

} __attribute__ ((packed));

struct isp39_debayer_cfg {

 __u8 filter_g_en;
 __u8 filter_c_en;

 __u8 luma_dx[7];

 __u8 clip_en;
 __u8 dist_scale;
 __u8 thed0;
 __u8 thed1;
 __u8 select_thed;
 __u8 max_ratio;

 __s8 filter1_coe1;
 __s8 filter1_coe2;
 __s8 filter1_coe3;
 __s8 filter1_coe4;

 __s8 filter2_coe1;
 __s8 filter2_coe2;
 __s8 filter2_coe3;
 __s8 filter2_coe4;

 __u16 gain_offset;
 __u8 gradloflt_alpha;
 __u8 wgt_alpha;

 __u16 drct_offset[8];

 __u8 gfilter_mode;
 __u16 bf_ratio;
 __u16 offset;

 __s8 filter_coe0;
 __s8 filter_coe1;
 __s8 filter_coe2;

 __u16 vsigma[8];

 __s8 guid_gaus_coe0;
 __s8 guid_gaus_coe1;
 __s8 guid_gaus_coe2;

 __s8 ce_gaus_coe0;
 __s8 ce_gaus_coe1;
 __s8 ce_gaus_coe2;

 __s8 alpha_gaus_coe0;
 __s8 alpha_gaus_coe1;
 __s8 alpha_gaus_coe2;

 __u16 loghf_offset;
 __u16 loggd_offset;
 __u8 log_en;

 __u16 alpha_offset;
 __u32 alpha_scale;

 __u16 edge_offset;
 __u32 edge_scale;

 __u8 ce_sgm;
 __u8 exp_shift;
 __u16 wgtslope;

 __u8 wet_clip;
 __u8 wet_ghost;

 __u16 bf_sgm;
 __u8 bf_clip;
 __u8 bf_curwgt;
} __attribute__ ((packed));

struct isp39_bay3d_cfg {

 __u8 bypass_en;
 __u8 iirsparse_en;

 __u8 transf_bypass_en;
 __u8 sigma_curve_double_en;
 __u8 cur_spnr_bypass_en;
 __u8 cur_spnr_sigma_idxfilt_bypass_en;
 __u8 cur_spnr_sigma_curve_double_en;
 __u8 pre_spnr_bypass_en;
 __u8 pre_spnr_sigma_idxfilt_bypass_en;
 __u8 pre_spnr_sigma_curve_double_en;
 __u8 lpf_hi_bypass_en;
 __u8 lo_diff_vfilt_bypass_en;
 __u8 lpf_lo_bypass_en;
 __u8 lo_wgt_hfilt_en;
 __u8 lo_diff_hfilt_en;
 __u8 sig_hfilt_en;
 __u8 spnr_pre_sigma_use_en;
 __u8 lo_detection_mode;
 __u8 md_wgt_out_en;
 __u8 md_bypass_en;
 __u8 iirspnr_out_en;
 __u8 lomdwgt_dbg_en;
 __u8 curdbg_out_en;
 __u8 noisebal_mode;

 __u8 transf_mode;
 __u8 wgt_cal_mode;
 __u8 mge_wgt_ds_mode;
 __u8 kalman_wgt_ds_mode;
 __u8 mge_wgt_hdr_sht_thred;
 __u8 sigma_calc_mge_wgt_hdr_sht_thred;

 __u16 transf_mode_offset;
 __u8 transf_mode_scale;
 __u16 itransf_mode_offset;

 __u32 transf_data_max_limit;

 __u16 cur_spnr_sigma_hdr_sht_scale;
 __u16 cur_spnr_sigma_scale;

 __u16 cur_spnr_luma_sigma_x[16];

 __u16 cur_spnr_luma_sigma_y[16];

 __u16 cur_spnr_sigma_rgain_offset;
 __u16 cur_spnr_sigma_bgain_offset;

 __u16 cur_spnr_sigma_hdr_sht_offset;
 __u16 cur_spnr_sigma_offset;

 __u16 cur_spnr_pix_diff_max_limit;
 __u16 cur_spnr_wgt_cal_offset;

 __u16 cur_spnr_wgt;
 __u16 pre_spnr_wgt;

 __u16 cur_spnr_space_rb_wgt[9];

 __u16 cur_spnr_space_gg_wgt[9];

 __u16 pre_spnr_sigma_hdr_sht_scale;
 __u16 pre_spnr_sigma_scale;

 __u16 pre_spnr_luma_sigma_x[16];

 __u16 pre_spnr_luma_sigma_y[16];

 __u16 pre_spnr_sigma_rgain_offset;
 __u16 pre_spnr_sigma_bgain_offset;

 __u16 pre_spnr_sigma_hdr_sht_offset;
 __u16 pre_spnr_sigma_offset;

 __u16 pre_spnr_pix_diff_max_limit;
 __u16 pre_spnr_wgt_cal_offset;

 __u16 pre_spnr_space_rb_wgt[9];

 __u16 pre_spnr_space_gg_wgt[9];

 __u16 cur_spnr_wgt_cal_scale;
 __u16 pre_spnr_wgt_cal_scale;


 __u16 tnr_luma_sigma_x[20];

 __u16 tnr_luma_sigma_y[20];

 __u16 tnr_lpf_hi_coeff[9];

 __u16 tnr_lpf_lo_coeff[9];

 __u16 tnr_wgt_filt_coeff0;
 __u16 tnr_wgt_filt_coeff1;
 __u16 tnr_wgt_filt_coeff2;

 __u16 tnr_sigma_scale;
 __u16 tnr_sigma_hdr_sht_scale;

 __u8 tnr_sig_vfilt_wgt;
 __u8 tnr_lo_diff_vfilt_wgt;
 __u8 tnr_lo_wgt_vfilt_wgt;
 __u8 tnr_sig_first_line_scale;
 __u8 tnr_lo_diff_first_line_scale;

 __u16 tnr_lo_wgt_cal_offset;
 __u16 tnr_lo_wgt_cal_scale;

 __u16 tnr_low_wgt_cal_max_limit;
 __u16 tnr_mode0_base_ratio;

 __u16 tnr_lo_diff_wgt_cal_offset;
 __u16 tnr_lo_diff_wgt_cal_scale;

 __u16 tnr_lo_mge_pre_wgt_offset;
 __u16 tnr_lo_mge_pre_wgt_scale;

 __u16 tnr_mode0_lo_wgt_scale;
 __u16 tnr_mode0_lo_wgt_hdr_sht_scale;

 __u16 tnr_mode1_lo_wgt_scale;
 __u16 tnr_mode1_lo_wgt_hdr_sht_scale;

 __u16 tnr_mode1_wgt_scale;
 __u16 tnr_mode1_wgt_hdr_sht_scale;

 __u16 tnr_mode1_lo_wgt_offset;
 __u16 tnr_mode1_lo_wgt_hdr_sht_offset;

 __u16 tnr_auto_sigma_count_wgt_thred;
 __u16 tnr_mode1_wgt_min_limit;
 __u16 tnr_mode1_wgt_offset;

 __u32 tnr_out_sigma_sq;

 __u16 tnr_lo_wgt_clip_min_limit;
 __u16 tnr_lo_wgt_clip_hdr_sht_min_limit;

 __u16 tnr_lo_wgt_clip_max_limit;
 __u16 tnr_lo_wgt_clip_hdr_sht_max_limit;

 __u16 tnr_hi_wgt_clip_min_limit;
 __u16 tnr_hi_wgt_clip_hdr_sht_min_limit;

 __u16 tnr_hi_wgt_clip_max_limit;
 __u16 tnr_hi_wgt_clip_hdr_sht_max_limit;

 __u8 tnr_cur_spnr_hi_wgt_min_limit;
 __u8 tnr_pre_spnr_hi_wgt_min_limit;

 __u16 tnr_pix_max;
 __u8 lowgt_ctrl;
 __u16 lowgt_offint;

 __u32 tnr_auto_sigma_count_th;

 __u16 tnr_motion_nr_strg;
 __u8 tnr_gain_max;
} __attribute__ ((packed));

struct isp39_ynr_cfg {

 __u8 lospnr_bypass;
 __u8 hispnr_bypass;
 __u8 exgain_bypass;
 __u16 global_set_gain;
 __u8 gain_merge_alpha;
 __u8 rnr_en;

 __u16 rnr_max_radius;
 __u16 local_gain_scale;

 __u16 rnr_center_coorh;
 __u16 rnr_center_coorv;

 __u16 ds_filt_soft_thred_scale;
 __u8 ds_img_edge_scale;
 __u16 ds_filt_wgt_thred_scale;

 __u8 ds_filt_local_gain_alpha;
 __u8 ds_iir_init_wgt_scale;
 __u16 ds_filt_center_wgt;

 __u16 ds_filt_inv_strg;
 __u16 lospnr_wgt;

 __u16 lospnr_center_wgt;
 __u16 lospnr_strg;

 __u16 lospnr_dist_vstrg_scale;
 __u16 lospnr_dist_hstrg_scale;

 __u8 pre_filt_coeff0;
 __u8 pre_filt_coeff1;
 __u8 pre_filt_coeff2;

 __u8 lospnr_gain2strg_val[9];

 __u16 luma2sima_idx[17];

 __u16 luma2sima_val[17];

 __u8 radius2strg_val[17];

 __u8 hispnr_strong_edge;

 __u16 hispnr_sigma_min_limit;
 __u8 hispnr_local_gain_alpha;
 __u16 hispnr_strg;

 __u8 hispnr_filt_coeff[6];

 __u16 hispnr_filt_wgt_offset;
 __u32 hispnr_filt_center_wgt;

 __u16 hispnr_filt_wgt;
 __u16 hispnr_gain_thred;
} __attribute__ ((packed));

struct isp39_cnr_cfg {

 __u8 exgain_bypass;
 __u8 yuv422_mode;
 __u8 thumb_mode;
 __u8 hiflt_wgt0_mode;
 __u8 loflt_coeff;

 __u16 global_gain;
 __u8 global_gain_alpha;
 __u8 local_gain_scale;

 __u16 lobfflt_vsigma_uv;
 __u16 lobfflt_vsigma_y;

 __u16 lobfflt_alpha;

 __u8 thumb_bf_coeff0;
 __u8 thumb_bf_coeff1;
 __u8 thumb_bf_coeff2;
 __u8 thumb_bf_coeff3;

 __u8 loflt_uv_gain;
 __u8 loflt_vsigma;
 __u8 exp_x_shift_bit;
 __u16 loflt_wgt_slope;

 __u8 loflt_wgt_min_thred;
 __u8 loflt_wgt_max_limit;

 __u8 gaus_flt_coeff[6];

 __u16 gaus_flt_alpha;
 __u8 hiflt_wgt_min_limit;
 __u16 hiflt_alpha;

 __u8 hiflt_uv_gain;
 __u16 hiflt_global_vsigma;
 __u8 hiflt_cur_wgt;

 __u16 adj_offset;
 __u16 adj_scale;

 __u8 sgm_ratio[13];

 __u8 loflt_global_sgm_ratio;
 __u8 loflt_global_sgm_ratio_alpha;

 __u8 cur_wgt[13];

 __u16 hiflt_vsigma_idx[8];

 __u16 hiflt_vsigma[8];
} __attribute__ ((packed));

struct isp39_sharp_cfg {

 __u8 bypass;
 __u8 center_mode;
 __u8 local_gain_bypass;
 __u8 radius_step_mode;
 __u8 noise_clip_mode;
 __u8 clipldx_sel;
 __u8 baselmg_sel;
 __u8 noise_filt_sel;
 __u8 tex2wgt_en;


 __u8 pre_bifilt_alpha;
 __u8 guide_filt_alpha;
 __u8 detail_bifilt_alpha;
 __u8 global_sharp_strg;


 __u8 luma2table_idx[7];


 __u16 pbf_sigma_inv[8];


 __u16 bf_sigma_inv[8];


 __u8 pbf_sigma_shift;
 __u8 bf_sigma_shift;


 __u16 luma2strg_val[8];


 __u16 luma2posclip_val[8];


 __u8 pbf_coef0;
 __u8 pbf_coef1;
 __u8 pbf_coef2;


 __u8 bf_coef0;
 __u8 bf_coef1;
 __u8 bf_coef2;


 __u8 img_lpf_coeff[6];


 __u16 global_gain;
 __u8 gain_merge_alpha;
 __u8 local_gain_scale;


 __u16 gain2strg_val[14];


 __u16 center_x;
 __u16 center_y;


 __u8 distance2strg_val[22];


 __u16 luma2neg_clip_val[8];


 __u16 noise_max_limit;
 __u8 tex_reserve_level;

 __u16 tex_wet_scale;
 __u8 noise_norm_bit;
 __u8 tex_wgt_mode;

 __u16 tex2wgt_val[17];

 __u32 noise_strg;

 __u16 detail2strg_val[17];
} __attribute__ ((packed));

struct isp39_dhaz_cfg {

 __u8 dc_en;
 __u8 hist_en;
 __u8 map_mode;
 __u8 mem_mode;
 __u8 mem_force;
 __u8 air_lc_en;
 __u8 enhance_en;
 __u8 soft_wr_en;
 __u8 round_en;
 __u8 color_deviate_en;
 __u8 enh_luma_en;

 __u8 dc_min_th;
 __u8 dc_max_th;
 __u8 yhist_th;
 __u8 yblk_th;

 __u8 bright_min;
 __u8 bright_max;
 __u16 wt_max;

 __u8 air_min;
 __u8 air_max;
 __u8 dark_th;
 __u8 tmax_base;

 __u16 tmax_off;
 __u16 tmax_max;

 __u16 enhance_chroma;
 __u16 enhance_value;

 __u8 stab_fnum;
 __u8 iir_sigma;
 __u16 iir_wt_sigma;

 __u8 iir_air_sigma;
 __u16 iir_tmax_sigma;
 __u8 iir_pre_wet;

 __u8 cfg_alpha;
 __u8 cfg_air;
 __u16 cfg_wt;

 __u16 cfg_tmax;

 __u8 space_sigma_cur;
 __u8 space_sigma_pre;
 __u16 range_sima;

 __u16 bf_weight;
 __u16 dc_weitcur;

 __u16 enh_curve[17];

 __u8 gaus_h0;
 __u8 gaus_h1;
 __u8 gaus_h2;

 __u16 enh_luma[17];

 __u16 adp_wt_wr;
 __u8 adp_air_wr;

 __u16 adp_tmax_wr;

 __u8 sigma_idx[15];

 __u16 sigma_lut[17];

 __u16 gain_fuse_alpha;

 __u8 hist_k;
 __u8 hist_th_off;
 __u8 hist_min;

 __u16 blk_het;
 __u16 blk_wid;

 __u8 thumb_row;
 __u8 thumb_col;

 __u16 cfg_k_alpha;
 __u16 cfg_k;

 __u16 k_gain;

 __u16 blend_wet[17];

 __u16 hist_iir_wr;
 __u16 hist_iir[(8 * 10)][16];
} __attribute__ ((packed));

struct isp39_drc_cfg {

 __u8 bypass_en;
 __u8 cmps_byp_en;
 __u8 gainx32_en;
 __u8 raw_dly_dis;


 __u16 position;
 __u16 compres_scl;
 __u8 offset_pow2;


 __u16 lpdetail_ratio;
 __u16 hpdetail_ratio;
 __u8 delta_scalein;


 __u8 bilat_wt_off;
 __u16 thumb_thd_neg;
 __u8 thumb_thd_enable;
 __u8 weicur_pix;


 __u8 cmps_offset_bits_int;
 __u8 cmps_fixbit_mode;
 __u16 drc_gas_t;


 __u32 thumb_clip;
 __u8 thumb_scale;


 __u32 range_sgm_inv0;
 __u16 range_sgm_inv1;


 __u8 weig_bilat;
 __u8 weight_8x8thumb;
 __u16 bilat_soft_thd;
 __u8 enable_soft_thd;


 __u16 gain_y[17];

 __u16 compres_y[17];

 __u16 scale_y[17];


 __u16 min_ogain;

 __u16 sfthd_y[17];
} __attribute__ ((packed));

struct isp39_rawawb_meas_cfg {
 __u8 bls2_en;

 __u8 rawawb_sel;
 __u8 bnr2awb_sel;
 __u8 drc2awb_sel;

 __u8 uv_en0;
 __u8 xy_en0;
 __u8 yuv3d_en0;
 __u8 yuv3d_ls_idx0;
 __u8 yuv3d_ls_idx1;
 __u8 yuv3d_ls_idx2;
 __u8 yuv3d_ls_idx3;
 __u8 in_rshift_to_12bit_en;
 __u8 in_overexposure_check_en;
 __u8 wind_size;
 __u8 rawlsc_bypass_en;
 __u8 light_num;
 __u8 uv_en1;
 __u8 xy_en1;
 __u8 yuv3d_en1;
 __u8 low12bit_val;

 __u8 blk_measure_enable;
 __u8 blk_measure_mode;
 __u8 blk_measure_xytype;
 __u8 blk_rtdw_measure_en;
 __u8 blk_measure_illu_idx;
 __u8 ds16x8_mode_en;
 __u8 blk_with_luma_wei_en;
 __u16 in_overexposure_threshold;

 __u16 h_offs;
 __u16 v_offs;

 __u16 h_size;
 __u16 v_size;

 __u16 r_max;
 __u16 g_max;

 __u16 b_max;
 __u16 y_max;

 __u16 r_min;
 __u16 g_min;

 __u16 b_min;
 __u16 y_min;

 __u8 wp_luma_wei_en0;
 __u8 wp_luma_wei_en1;
 __u8 wp_blk_wei_en0;
 __u8 wp_blk_wei_en1;
 __u8 wp_hist_xytype;

 __u8 wp_luma_weicurve_y0;
 __u8 wp_luma_weicurve_y1;
 __u8 wp_luma_weicurve_y2;
 __u8 wp_luma_weicurve_y3;

 __u8 wp_luma_weicurve_y4;
 __u8 wp_luma_weicurve_y5;
 __u8 wp_luma_weicurve_y6;
 __u8 wp_luma_weicurve_y7;

 __u8 wp_luma_weicurve_y8;

 __u8 wp_luma_weicurve_w0;
 __u8 wp_luma_weicurve_w1;
 __u8 wp_luma_weicurve_w2;
 __u8 wp_luma_weicurve_w3;

 __u8 wp_luma_weicurve_w4;
 __u8 wp_luma_weicurve_w5;
 __u8 wp_luma_weicurve_w6;
 __u8 wp_luma_weicurve_w7;

 __u8 wp_luma_weicurve_w8;
 __u16 pre_wbgain_inv_r;

 __u16 pre_wbgain_inv_g;
 __u16 pre_wbgain_inv_b;

 __u16 vertex0_u_0;
 __u16 vertex0_v_0;

 __u16 vertex1_u_0;
 __u16 vertex1_v_0;

 __u16 vertex2_u_0;
 __u16 vertex2_v_0;

 __u16 vertex3_u_0;
 __u16 vertex3_v_0;

 __u32 islope01_0;

 __u32 islope12_0;

 __u32 islope23_0;

 __u32 islope30_0;

 __u16 vertex0_u_1;
 __u16 vertex0_v_1;

 __u16 vertex1_u_1;
 __u16 vertex1_v_1;

 __u16 vertex2_u_1;
 __u16 vertex2_v_1;

 __u16 vertex3_u_1;
 __u16 vertex3_v_1;

 __u32 islope01_1;

 __u32 islope12_1;

 __u32 islope23_1;

 __u32 islope30_1;

 __u16 vertex0_u_2;
 __u16 vertex0_v_2;

 __u16 vertex1_u_2;
 __u16 vertex1_v_2;

 __u16 vertex2_u_2;
 __u16 vertex2_v_2;

 __u16 vertex3_u_2;
 __u16 vertex3_v_2;

 __u32 islope01_2;

 __u32 islope12_2;

 __u32 islope23_2;

 __u32 islope30_2;

 __u16 vertex0_u_3;
 __u16 vertex0_v_3;

 __u16 vertex1_u_3;
 __u16 vertex1_v_3;

 __u16 vertex2_u_3;
 __u16 vertex2_v_3;

 __u16 vertex3_u_3;
 __u16 vertex3_v_3;

 __u32 islope01_3;

 __u32 islope12_3;

 __u32 islope23_3;

 __u32 islope30_3;

 __u16 rgb2ryuvmat0_y;
 __u16 rgb2ryuvmat1_y;

 __u16 rgb2ryuvmat2_y;
 __u16 rgb2ryuvofs_y;

 __u16 rgb2ryuvmat0_u;
 __u16 rgb2ryuvmat1_u;

 __u16 rgb2ryuvmat2_u;
 __u16 rgb2ryuvofs_u;

 __u16 rgb2ryuvmat0_v;
 __u16 rgb2ryuvmat1_v;

 __u16 rgb2ryuvmat2_v;
 __u16 rgb2ryuvofs_v;

 __u16 coor_x1_ls0_y;
 __u16 vec_x21_ls0_y;

 __u16 coor_x1_ls0_u;
 __u16 vec_x21_ls0_u;

 __u16 coor_x1_ls0_v;
 __u16 vec_x21_ls0_v;

 __u8 dis_x1x2_ls0;
 __u8 rotu0_ls0;
 __u8 rotu1_ls0;

 __u8 rotu2_ls0;
 __u8 rotu3_ls0;
 __u8 rotu4_ls0;
 __u8 rotu5_ls0;

 __u16 th0_ls0;
 __u16 th1_ls0;

 __u16 th2_ls0;
 __u16 th3_ls0;

 __u16 th4_ls0;
 __u16 th5_ls0;

 __u16 coor_x1_ls1_y;
 __u16 vec_x21_ls1_y;

 __u16 coor_x1_ls1_u;
 __u16 vec_x21_ls1_u;

 __u16 coor_x1_ls1_v;
 __u16 vec_x21_ls1_v;

 __u8 dis_x1x2_ls1;
 __u8 rotu0_ls1;
 __u8 rotu1_ls1;

 __u8 rotu2_ls1;
 __u8 rotu3_ls1;
 __u8 rotu4_ls1;
 __u8 rotu5_ls1;

 __u16 th0_ls1;
 __u16 th1_ls1;

 __u16 th2_ls1;
 __u16 th3_ls1;

 __u16 th4_ls1;
 __u16 th5_ls1;

 __u16 coor_x1_ls2_y;
 __u16 vec_x21_ls2_y;

 __u16 coor_x1_ls2_u;
 __u16 vec_x21_ls2_u;

 __u16 coor_x1_ls2_v;
 __u16 vec_x21_ls2_v;

 __u8 dis_x1x2_ls2;
 __u8 rotu0_ls2;
 __u8 rotu1_ls2;

 __u8 rotu2_ls2;
 __u8 rotu3_ls2;
 __u8 rotu4_ls2;
 __u8 rotu5_ls2;

 __u16 th0_ls2;
 __u16 th1_ls2;

 __u16 th2_ls2;
 __u16 th3_ls2;

 __u16 th4_ls2;
 __u16 th5_ls2;

 __u16 coor_x1_ls3_y;
 __u16 vec_x21_ls3_y;

 __u16 coor_x1_ls3_u;
 __u16 vec_x21_ls3_u;

 __u16 coor_x1_ls3_v;
 __u16 vec_x21_ls3_v;

 __u8 dis_x1x2_ls3;
 __u8 rotu0_ls3;
 __u8 rotu1_ls3;

 __u8 rotu2_ls3;
 __u8 rotu3_ls3;
 __u8 rotu4_ls3;
 __u8 rotu5_ls3;

 __u16 th0_ls3;
 __u16 th1_ls3;

 __u16 th2_ls3;
 __u16 th3_ls3;

 __u16 th4_ls3;
 __u16 th5_ls3;

 __u16 wt0;
 __u16 wt1;

 __u16 wt2;

 __u16 mat0_x;
 __u16 mat0_y;

 __u16 mat1_x;
 __u16 mat1_y;

 __u16 mat2_x;
 __u16 mat2_y;

 __u16 nor_x0_0;
 __u16 nor_x1_0;

 __u16 nor_y0_0;
 __u16 nor_y1_0;

 __u16 big_x0_0;
 __u16 big_x1_0;

 __u16 big_y0_0;
 __u16 big_y1_0;

 __u16 nor_x0_1;
 __u16 nor_x1_1;

 __u16 nor_y0_1;
 __u16 nor_y1_1;

 __u16 big_x0_1;
 __u16 big_x1_1;

 __u16 big_y0_1;
 __u16 big_y1_1;

 __u16 nor_x0_2;
 __u16 nor_x1_2;

 __u16 nor_y0_2;
 __u16 nor_y1_2;

 __u16 big_x0_2;
 __u16 big_x1_2;

 __u16 big_y0_2;
 __u16 big_y1_2;

 __u16 nor_x0_3;
 __u16 nor_x1_3;

 __u16 nor_y0_3;
 __u16 nor_y1_3;

 __u16 big_x0_3;
 __u16 big_x1_3;

 __u16 big_y0_3;
 __u16 big_y1_3;

 __u8 exc_wp_region0_excen;
 __u8 exc_wp_region0_measen;
 __u8 exc_wp_region0_domain;
 __u8 exc_wp_region1_excen;
 __u8 exc_wp_region1_measen;
 __u8 exc_wp_region1_domain;
 __u8 exc_wp_region2_excen;
 __u8 exc_wp_region2_measen;
 __u8 exc_wp_region2_domain;
 __u8 exc_wp_region3_excen;
 __u8 exc_wp_region3_measen;
 __u8 exc_wp_region3_domain;
 __u8 exc_wp_region4_excen;
 __u8 exc_wp_region4_domain;
 __u8 exc_wp_region5_excen;
 __u8 exc_wp_region5_domain;
 __u8 exc_wp_region6_excen;
 __u8 exc_wp_region6_domain;
 __u8 multiwindow_en;

 __u16 multiwindow0_h_offs;
 __u16 multiwindow0_v_offs;

 __u16 multiwindow0_h_size;
 __u16 multiwindow0_v_size;

 __u16 multiwindow1_h_offs;
 __u16 multiwindow1_v_offs;

 __u16 multiwindow1_h_size;
 __u16 multiwindow1_v_size;

 __u16 multiwindow2_h_offs;
 __u16 multiwindow2_v_offs;

 __u16 multiwindow2_h_size;
 __u16 multiwindow2_v_size;

 __u16 multiwindow3_h_offs;
 __u16 multiwindow3_v_offs;

 __u16 multiwindow3_h_size;
 __u16 multiwindow3_v_size;

 __u16 exc_wp_region0_xu0;
 __u16 exc_wp_region0_xu1;

 __u16 exc_wp_region0_yv0;
 __u16 exc_wp_region0_yv1;

 __u16 exc_wp_region1_xu0;
 __u16 exc_wp_region1_xu1;

 __u16 exc_wp_region1_yv0;
 __u16 exc_wp_region1_yv1;

 __u16 exc_wp_region2_xu0;
 __u16 exc_wp_region2_xu1;

 __u16 exc_wp_region2_yv0;
 __u16 exc_wp_region2_yv1;

 __u16 exc_wp_region3_xu0;
 __u16 exc_wp_region3_xu1;

 __u16 exc_wp_region3_yv0;
 __u16 exc_wp_region3_yv1;

 __u16 exc_wp_region4_xu0;
 __u16 exc_wp_region4_xu1;

 __u16 exc_wp_region4_yv0;
 __u16 exc_wp_region4_yv1;

 __u16 exc_wp_region5_xu0;
 __u16 exc_wp_region5_xu1;

 __u16 exc_wp_region5_yv0;
 __u16 exc_wp_region5_yv1;

 __u16 exc_wp_region6_xu0;
 __u16 exc_wp_region6_xu1;

 __u16 exc_wp_region6_yv0;
 __u16 exc_wp_region6_yv1;

 __u8 exc_wp_region0_weight;
 __u8 exc_wp_region1_weight;
 __u8 exc_wp_region2_weight;
 __u8 exc_wp_region3_weight;

 __u8 exc_wp_region4_weight;
 __u8 exc_wp_region5_weight;
 __u8 exc_wp_region6_weight;

 __u8 wp_blk_wei_w[225];

 struct isp2x_bls_fixed_val bls2_val;
} __attribute__ ((packed));

struct isp39_rawaf_meas_cfg {
 __u8 rawaf_sel;
 __u8 num_afm_win;
 __u8 bnr2af_sel;


 __u8 gamma_en;
 __u8 gaus_en;
 __u8 hiir_en;
 __u8 viir_en;
 __u8 ldg_en;
 __u8 h1_fv_mode;
 __u8 h2_fv_mode;
 __u8 v1_fv_mode;
 __u8 v2_fv_mode;
 __u8 ae_mode;
 __u8 y_mode;
 __u8 vldg_sel;
 __u8 v_dnscl_mode;
 __u8 from_ynr;
 __u8 ae_config_use;
 __u8 hiir_left_border_mode;
 __u8 avg_ds_en;
 __u8 avg_ds_mode;
 __u8 h1_acc_mode;
 __u8 h2_acc_mode;
 __u8 v1_acc_mode;
 __u8 v2_acc_mode;


 struct isp2x_window win[2];


 __s16 bls_offset;
 __u8 bls_en;
 __u8 aehgl_en;
 __u8 hldg_dilate_num;
 __u8 tnrin_shift;


 __u8 h1iir_shift_wina;
 __u8 h2iir_shift_wina;
 __u8 v1iir_shift_wina;
 __u8 v2iir_shift_wina;
 __u8 h1iir_shift_winb;
 __u8 h2iir_shift_winb;
 __u8 v1iir_shift_winb;
 __u8 v2iir_shift_winb;


 __s8 gaus_coe[9];


 __u16 gamma_y[17];

 __u16 h_fv_thresh;
 __u16 v_fv_thresh;
 struct isp3x_rawaf_curve curve_h[2];
 struct isp3x_rawaf_curve curve_v[2];
 __s16 h1iir1_coe[6];
 __s16 h1iir2_coe[6];
 __s16 h2iir1_coe[6];
 __s16 h2iir2_coe[6];
 __s16 v1iir_coe[3];
 __s16 v2iir_coe[3];
 __s16 v1fir_coe[3];
 __s16 v2fir_coe[3];

 __u16 highlit_thresh;


 __u16 h_fv_limit;
 __u16 h_fv_slope;

 __u16 v_fv_limit;
 __u16 v_fv_slope;
} __attribute__ ((packed));

struct isp39_isp_other_cfg {
 struct isp32_bls_cfg bls_cfg;
 struct isp39_dpcc_cfg dpcc_cfg;
 struct isp3x_lsc_cfg lsc_cfg;
 struct isp32_awb_gain_cfg awb_gain_cfg;
 struct isp39_gic_cfg gic_cfg;
 struct isp39_debayer_cfg debayer_cfg;
 struct isp39_ccm_cfg ccm_cfg;
 struct isp3x_gammaout_cfg gammaout_cfg;
 struct isp2x_cproc_cfg cproc_cfg;
 struct isp2x_sdg_cfg sdg_cfg;
 struct isp39_drc_cfg drc_cfg;
 struct isp32_hdrmge_cfg hdrmge_cfg;
 struct isp39_dhaz_cfg dhaz_cfg;
 struct isp2x_3dlut_cfg isp3dlut_cfg;
 struct isp39_ldch_cfg ldch_cfg;
 struct isp39_bay3d_cfg bay3d_cfg;
 struct isp39_ynr_cfg ynr_cfg;
 struct isp39_cnr_cfg cnr_cfg;
 struct isp39_sharp_cfg sharp_cfg;
 struct isp32_cac_cfg cac_cfg;
 struct isp3x_gain_cfg gain_cfg;
 struct isp21_csm_cfg csm_cfg;
 struct isp21_cgc_cfg cgc_cfg;
 struct isp39_yuvme_cfg yuvme_cfg;
 struct isp39_ldcv_cfg ldcv_cfg;
 struct isp39_rgbir_cfg rgbir_cfg;
} __attribute__ ((packed));

struct isp39_isp_meas_cfg {
 struct isp39_rawaf_meas_cfg rawaf;
 struct isp39_rawawb_meas_cfg rawawb;
 struct isp2x_rawaebig_meas_cfg rawae0;
 struct isp2x_rawaebig_meas_cfg rawae3;
 struct isp2x_rawhistbig_cfg rawhist0;
 struct isp2x_rawhistbig_cfg rawhist3;
} __attribute__ ((packed));

struct isp39_isp_params_cfg {
 __u64 module_en_update;
 __u64 module_ens;
 __u64 module_cfg_update;

 __u32 frame_id;
 struct isp39_isp_meas_cfg meas;
 struct isp39_isp_other_cfg others;
 struct sensor_exposure_cfg exposure;
} __attribute__ ((packed));

struct isp39_dhaz_stat {
 __u16 adp_wt;
 __u16 adp_air_base;
 __u16 adp_tmax;

 __u16 hist_iir[(8 * 10)][16];
} __attribute__ ((packed));

struct isp39_bay3d_stat {
 __u32 tnr_auto_sigma_count;
 __u16 tnr_auto_sigma_calc[20];
} __attribute__ ((packed));

struct isp39_rawae_mean_lum {
 __u32 g:12;
 __u32 b:10;
 __u32 r:10;
} __attribute__ ((packed));

struct isp39_rawae_mean_line {

 struct isp39_rawae_mean_lum blk_x[15];
 __u32 reserved;
} __attribute__ ((packed));

struct isp39_rawae_stat {
 struct isp39_rawae_mean_line blk_y[15];
} __attribute__ ((packed));

struct isp39_rawhist_stat {
 __u32 bin[256];
} __attribute__ ((packed));

struct isp39_rawaf_mean_hiir {
 __u32 h1;
 __u32 h2;
} __attribute__ ((packed));

struct isp39_rawaf_mean_hiir_line {

 struct isp39_rawaf_mean_hiir hiir_blk_x[15];
 __u32 reserved[2];
} __attribute__ ((packed));

struct isp39_rawaf_mean_viir {
 __u32 v1;
 __u32 v2;
} __attribute__ ((packed));

struct isp39_rawaf_mean_viir_line {

 struct isp39_rawaf_mean_viir viir_blk_x[15];
 __u32 reserved[2];
} __attribute__ ((packed));

struct isp39_rawaf_mean_aehgl {
 __u32 average:12;
 __u32 highlight:20;
} __attribute__ ((packed));

struct isp39_rawaf_mean_aehgl_line {

 struct isp39_rawaf_mean_aehgl aehgl_blk_x[15];
 __u32 reserved;
} __attribute__ ((packed));

struct isp39_rawaf_stat {
 struct isp39_rawaf_mean_hiir_line hiir_blk_y[15];
 struct isp39_rawaf_mean_viir_line viir_blk_y[15];
 struct isp39_rawaf_mean_aehgl_line aehgl_blk_y[15];
 __u32 h1iir_sumb;
 __u32 h2iir_sumb;
 __u32 v1iir_sumb;
 __u32 v2iir_sumb;
 __u32 sumy_winb;
 __u32 highlit_cnt_winb;
 __u32 reserved[2];
} __attribute__ ((packed));

struct isp39_rawawb_mean_ramdata {
 __u64 b:18;
 __u64 g:18;
 __u64 r:18;
 __u64 wp:10;
} __attribute__ ((packed));

struct isp39_rawawb_mean_ramdata_line {
 struct isp39_rawawb_mean_ramdata ramdata_blk_x[15];
 __u32 reserved[2];
} __attribute__ ((packed));

struct isp39_rawawb_mean_sum {
 __u32 rgain_nor;
 __u32 bgain_nor;
 __u32 wp_num_nor;
 __u32 wp_num2;

 __u32 rgain_big;
 __u32 bgain_big;
 __u32 wp_num_big;
 __u32 reserved;
} __attribute__ ((packed));

struct isp39_rawawb_mean_sum_exc {
 __u32 rgain_exc;
 __u32 bgain_exc;
 __u32 wp_num_exc;
 __u32 reserved;
} __attribute__ ((packed));

struct isp39_rawawb_stat {
 struct isp39_rawawb_mean_ramdata_line ramdata_blk_y[15];
 struct isp39_rawawb_mean_sum sum[4];
 __u16 yhist[8];
 struct isp39_rawawb_mean_sum_exc sum_exc[4];
} __attribute__ ((packed));

struct isp39_stat {

 struct isp39_rawae_stat rawae3;
 struct isp39_rawhist_stat rawhist3;
 struct isp39_rawae_stat rawae0;
 struct isp39_rawhist_stat rawhist0;
 struct isp39_rawaf_stat rawaf;
 struct isp39_rawawb_stat rawawb;

 struct isp39_dhaz_stat dhaz;
 struct isp39_bay3d_stat bay3d;
 struct isp32_info2ddr_stat info2ddr;
 int buf_bay3d_iir_index;
} __attribute__ ((packed));

struct rkisp39_stat_buffer {
 struct isp39_stat stat;
 __u32 meas_type;
 __u32 frame_id;
 __u32 params_id;
} __attribute__ ((packed));


struct isp33_hsv_cfg {
 __u8 hsv_1dlut0_en;
 __u8 hsv_1dlut1_en;
 __u8 hsv_2dlut_en;
 __u8 hsv_1dlut0_idx_mode;
 __u8 hsv_1dlut1_idx_mode;
 __u8 hsv_2dlut_idx_mode;
 __u8 hsv_1dlut0_item_mode;
 __u8 hsv_1dlut1_item_mode;
 __u8 hsv_2dlut_item_mode;

 __u16 lut0_1d[65];
 __u16 lut1_1d[65];
 __u16 lut_2d[17][17];
} __attribute__ ((packed));

struct isp33_gic_cfg {

 __u8 bypass_en;
 __u8 pro_mode;
 __u8 manualnoisecurve_en;
 __u8 manualnoisethred_en;
 __u8 gain_bypass_en;

 __u8 medflt_minthred;
 __u8 medflt_maxthred;
 __u8 medflt_ratio;

 __u8 medfltuv_minthred;
 __u8 medfltuv_maxthred;
 __u8 medfltuv_ratio;

 __u16 noisecurve_scale;

 __u16 bffltwgt_offset;
 __u8 bffltwgt_scale;

 __u8 bfflt_ratio;

 __u8 bfflt_coeff0;
 __u8 bfflt_coeff1;
 __u8 bfflt_coeff2;

 __u16 bfflt_vsigma_y[17];

 __u8 luma_dx[7];

 __u16 thred_y[8];

 __u16 minthred_y[8];

 __u16 autonoisethred_scale;

 __u8 lofltgr_coeff0;
 __u8 lofltgr_coeff1;
 __u8 lofltgr_coeff2;
 __u8 lofltgr_coeff3;

 __u8 lofltgb_coeff0;
 __u8 lofltgb_coeff1;

 __u16 sumlofltcoeff_inv;

 __u8 lofltthred_coeff0;
 __u8 lofltthred_coeff1;

 __u8 globalgain_alpha;
 __u8 globalgain_scale;
 __u16 global_gain;

 __u16 gain_offset;
 __u16 gain_scale;

 __u16 gainadjflt_minthred;
 __u16 gainadjflt_maxthred;
} __attribute__ ((packed));

struct isp33_cac_cfg {

 __u8 bypass_en;
 __u8 edge_detect_en;
 __u8 neg_clip0_en;
 __u8 wgt_color_en;

 __u8 psf_table_fix_bit;

 __u16 hi_drct_ratio;

 __u32 over_expo_thred;

 __u32 over_expo_adj;

 __u8 flat_thred;
 __u16 flat_offset;

 __u8 chroma_lo_flt_coeff0;
 __u8 chroma_lo_flt_coeff1;
 __u8 color_lo_flt_coeff0;
 __u8 color_lo_flt_coeff1;

 __u16 search_range_ratio;
 __u16 residual_chroma_ratio;

 __u32 wgt_color_b_min_thred;

 __u32 wgt_color_r_min_thred;

 __u32 wgt_color_b_slope;

 __u32 wgt_color_r_slope;

 __u32 wgt_color_min_luma;

 __u32 wgt_color_luma_slope;

 __u32 wgt_over_expo_min_thred;

 __u32 wgt_over_expo_slope;

 __u32 wgt_contrast_min_thred;

 __u32 wgt_contrast_slope;

 __u32 wgt_contrast_offset;

 __u32 wgt_dark_thed;

 __u32 wgt_dark_slope;

 __u8 psf_b_ker[11];

 __u8 psf_r_ker[11];
} __attribute__ ((packed));

struct isp33_ccm_cfg {

 __u8 highy_adjust_dis;
 __u8 enh_adj_en;
 __u8 asym_adj_en;
 __u8 sat_decay_en;

 __s16 coeff0_r;
 __s16 coeff1_r;

 __s16 coeff2_r;
 __s16 offset_r;

 __s16 coeff0_g;
 __s16 coeff1_g;

 __s16 coeff2_g;
 __s16 offset_g;

 __s16 coeff0_b;
 __s16 coeff1_b;

 __s16 coeff2_b;
 __s16 offset_b;

 __u16 coeff0_y;
 __u16 coeff1_y;

 __u16 coeff2_y;

 __u16 alp_y[18];

 __u8 bound_bit;
 __u8 right_bit;

 __u16 color_coef0_r2y;
 __u16 color_coef1_g2y;

 __u16 color_coef2_b2y;
 __u16 color_enh_rat_max;

 __u8 hf_low;
 __u8 hf_up;
 __u16 hf_scale;

 __u16 hf_factor[17];
} __attribute__ ((packed));

struct isp33_debayer_cfg {

 __u8 bypass;
 __u8 g_out_flt_en;

 __u8 luma_dx[7];

 __u8 g_interp_clip_en;
 __u8 hi_texture_thred;
 __u8 hi_drct_thred;
 __u8 lo_drct_thred;
 __u8 drct_method_thred;
 __u8 g_interp_sharp_strg_max_limit;

 __s8 lo_drct_flt_coeff1;
 __s8 lo_drct_flt_coeff2;
 __s8 lo_drct_flt_coeff3;
 __s8 lo_drct_flt_coeff4;

 __s8 hi_drct_flt_coeff1;
 __s8 hi_drct_flt_coeff2;
 __s8 hi_drct_flt_coeff3;
 __s8 hi_drct_flt_coeff4;

 __u16 g_interp_sharp_strg_offset;
 __u8 grad_lo_flt_alpha;

 __u16 drct_offset[8];

 __u8 gflt_mode;
 __u16 gflt_ratio;
 __u16 gflt_offset;

 __s8 gflt_coe0;
 __s8 gflt_coe1;
 __s8 gflt_coe2;

 __u16 gflt_vsigma[8];
} __attribute__ ((packed));

struct isp33_bay3d_cfg {

 __u8 bypass_en;
 __u8 iirsparse_en;
 __u8 out_use_pre_mode;
 __u8 motion_est_en;

 __u8 transf_bypass_en;
 __u8 tnrsigma_curve_double_en;
 __u8 md_large_lo_use_mode;
 __u8 md_large_lo_min_filter_bypass_en;
 __u8 md_large_lo_gauss_filter_bypass_en;
 __u8 md_large_lo_md_wgt_bypass_en;
 __u8 pre_pix_out_mode;
 __u8 motion_detect_bypass_en;
 __u8 lpf_hi_bypass_en;
 __u8 lo_diff_vfilt_bypass_en;
 __u8 lpf_lo_bypass_en;
 __u8 lo_wgt_hfilt_en;
 __u8 lo_diff_hfilt_en;
 __u8 sig_hfilt_en;
 __u8 lo_detection_bypass_en;
 __u8 lo_mge_wgt_mode;
 __u8 pre_spnr_out_en;
 __u8 md_only_lo_en;
 __u8 cur_spnr_out_en;
 __u8 md_wgt_out_en;

 __u8 cur_spnr_filter_bypass_en;
 __u8 pre_spnr_hi_filter_gic_en;
 __u8 pre_spnr_hi_filter_gic_enhance_en;
 __u8 spnr_presigma_use_en;
 __u8 pre_spnr_lo_filter_bypass_en;
 __u8 pre_spnr_hi_filter_bypass_en;
 __u8 pre_spnr_sigma_curve_double_en;
 __u8 pre_spnr_hi_guide_filter_bypass_en;
 __u8 pre_spnr_sigma_idx_filt_bypass_en;
 __u8 pre_spnr_sigma_idx_filt_mode;
 __u8 pre_spnr_hi_noise_ctrl_en;
 __u8 pre_spnr_hi_filter_wgt_mode;
 __u8 pre_spnr_lo_filter_wgt_mode;
 __u8 pre_spnr_hi_filter_rb_wgt_mode;
 __u8 pre_spnr_lo_filter_rb_wgt_mode;
 __u8 pre_hi_gic_lp_en;
 __u8 pre_hi_bf_lp_en;
 __u8 pre_lo_avg_lp_en;

 __u8 transf_mode;
 __u8 wgt_cal_mode;
 __u8 ww_mode;
 __u8 wgt_last_mode;
 __u8 mge_wgt_hdr_sht_thred;
 __u8 sigma_calc_mge_wgt_hdr_sht_thred;

 __u8 transf_mode_scale;
 __u16 transf_mode_offset;
 __u16 itransf_mode_offset;

 __u32 transf_data_max_limit;

 __u16 pre_sig_ctrl_scl;

 __u8 pre_hi_guide_out_wgt;

 __u8 cur_spnr_filter_coeff[6];

 __u16 pre_spnr_luma2sigma_x[16];

 __u16 pre_spnr_luma2sigma_y[16];

 __u16 pre_spnr_hi_sigma_scale;

 __u8 pre_spnr_hi_wgt_calc_scale;

 __u8 pre_spnr_hi_filter_wgt_min_limit;

 __u8 pre_spnr_hi_filter_out_wgt;
 __u8 pre_spnr_sigma_offset;
 __u8 pre_spnr_sigma_hdr_sht_offset;

 __u16 pre_spnr_sigma_scale;
 __u16 pre_spnr_sigma_hdr_sht_scale;

 __u8 pre_spnr_hi_filter_coeff[6];

 __u16 pre_spnr_lo_sigma_scale;

 __u8 pre_spnr_lo_wgt_calc_scale;

 __u16 pre_spnr_hi_noise_ctrl_scale;
 __u8 pre_spnr_hi_noise_ctrl_offset;

 __u16 tnr_luma2sigma_x[20];

 __u16 tnr_luma2sigma_y[20];

 __u16 lpf_hi_coeff[9];

 __u16 lpf_lo_coeff[9];

 __u8 sigma_idx_filt_coeff[6];
 __u16 lo_wgt_cal_first_line_sigma_scale;

 __u8 lo_diff_vfilt_wgt;
 __u8 lo_wgt_vfilt_wgt;
 __u8 sig_first_line_scale;
 __u8 lo_diff_first_line_scale;

 __u16 lo_wgt_cal_offset;
 __u16 lo_wgt_cal_scale;

 __u16 lo_wgt_cal_max_limit;
 __u16 mode0_base_ratio;

 __u16 lo_diff_wgt_cal_offset;
 __u16 lo_diff_wgt_cal_scale;

 __u16 lo_mge_pre_wgt_offset;
 __u16 lo_mge_pre_wgt_scale;

 __u16 mode0_lo_wgt_scale;
 __u16 mode0_lo_wgt_hdr_sht_scale;

 __u16 mode1_lo_wgt_scale;
 __u16 mode1_lo_wgt_hdr_sht_scale;

 __u16 mode1_wgt_scale;
 __u16 mode1_wgt_hdr_sht_scale;

 __u16 mode1_lo_wgt_offset;
 __u16 mode1_lo_wgt_hdr_sht_offset;

 __u16 auto_sigma_count_wgt_thred;
 __u16 mode1_wgt_min_limit;
 __u16 mode1_wgt_offset;

 __u32 tnr_out_sigma_sq;

 __u16 lo_wgt_clip_min_limit;
 __u16 lo_wgt_clip_hdr_sht_min_limit;

 __u16 lo_wgt_clip_max_limit;
 __u16 lo_wgt_clip_hdr_sht_max_limit;

 __u16 lo_pre_gg_soft_thresh_scale;
 __u16 lo_pre_rb_soft_thresh_scale;

 __u16 lo_pre_soft_thresh_max_limit;
 __u16 lo_pre_soft_thresh_min_limit;

 __u8 cur_spnr_hi_wgt_min_limit;
 __u8 pre_spnr_hi_wgt_min_limit;
 __u16 motion_est_lo_wgt_thred;

 __u16 pix_max_limit;

 __u32 sigma_num_th;

 __u32 out_use_hi_noise_bal_nr_strg;
 __u8 gain_out_max_limit;

 __u16 sigma_scale;
 __u16 sigma_hdr_sht_scale;

 __u16 lo_wgt_vfilt_offset;
 __u16 lo_diff_vfilt_offset;
 __u8 lo_wgt_cal_first_line_vfilt_wgt;

 __u8 lo_wgt_vfilt_scale;
 __u8 lo_diff_vfilt_scale_bit;
 __u8 lo_diff_vfilt_scale;
 __u8 lo_diff_first_line_vfilt_wgt;

 __u16 motion_est_up_mvx_cost_offset;
 __u16 motion_est_up_mvx_cost_scale;
 __u8 motion_est_sad_vert_wgt0;

 __u16 motion_est_up_left_mvx_cost_offset;
 __u16 motion_est_up_left_mvx_cost_scale;
 __u8 motion_est_sad_vert_wgt1;

 __u16 motion_est_up_right_mvx_cost_offset;
 __u16 motion_est_up_right_mvx_cost_scale;
 __u8 motion_est_sad_vert_wgt2;

 __u16 lo_wgt_clip_motion_max_limit;

 __u16 mode1_wgt_max_limit;

 __u16 mode0_wgt_out_max_limit;
 __u16 mode0_wgt_out_offset;

 __u8 pre_spnr_lo_val_wgt_out_wgt;
 __u8 pre_spnr_lo_filter_out_wgt;
 __u8 pre_spnr_lo_filter_wgt_min;

 __u8 md_large_lo_md_wgt_offset;
 __u16 md_large_lo_md_wgt_scale;

 __u16 md_large_lo_wgt_cut_offset;
 __u16 md_large_lo_wgt_add_offset;

 __u16 md_large_lo_wgt_scale;
} __attribute__ ((packed));

struct isp33_ynr_cfg {

 __u8 hi_spnr_bypass;
 __u8 mi_spnr_bypass;
 __u8 lo_spnr_bypass;
 __u8 rnr_en;
 __u8 tex2lo_strg_en;
 __u8 hi_lp_en;

 __u16 global_set_gain;
 __u8 gain_merge_alpha;
 __u8 local_gain_scale;

 __u16 lo_spnr_gain2strg[9];

 __u16 rnr_max_radius;

 __u16 rnr_center_h;
 __u16 rnr_center_v;

 __u8 radius2strg[17];

 __u16 luma2sima_x[17];

 __u16 luma2sima_y[17];

 __u16 hi_spnr_sigma_min_limit;
 __u16 hi_spnr_strg;
 __u8 hi_spnr_local_gain_alpha;

 __u8 hi_spnr_filt_coeff[5];

 __u16 hi_spnr_filt_wgt_offset;
 __u16 hi_spnr_filt_center_wgt;

 __u16 hi_spnr_filt1_coeff[6];

 __u16 hi_spnr_filt1_tex_thred;
 __u16 hi_spnr_filt1_tex_scale;
 __u16 hi_spnr_filt1_wgt_alpha;

 __u16 mi_spnr_filt_coeff0;
 __u16 mi_spnr_filt_coeff1;
 __u16 mi_spnr_filt_coeff2;

 __u16 mi_spnr_strg;
 __u16 mi_spnr_soft_thred_scale;

 __u8 mi_spnr_wgt;
 __u8 mi_ehance_scale_en;
 __u8 mi_ehance_scale;
 __u16 mi_spnr_filt_center_wgt;

 __u16 lo_spnr_strg;
 __u16 lo_spnr_soft_thred_scale;

 __u16 lo_spnr_thumb_thred_scale;
 __u16 tex2lo_strg_mantissa;
 __u8 tex2lo_strg_exponent;

 __u8 lo_spnr_wgt;
 __u16 lo_spnr_filt_center_wgt;

 __u16 tex2lo_strg_upper_thred;
 __u16 tex2lo_strg_lower_thred;

 __u8 lo_gain2wgt[9];
} __attribute__ ((packed));

struct isp33_cnr_cfg {

 __u8 exgain_bypass;
 __u8 yuv422_mode;
 __u8 thumb_mode;
 __u8 hiflt_wgt0_mode;
 __u8 local_alpha_dis;
 __u8 loflt_coeff;

 __u16 global_gain;
 __u8 global_gain_alpha;
 __u8 local_gain_scale;

 __u16 lobfflt_vsigma_uv;
 __u16 lobfflt_vsigma_y;

 __u16 lobfflt_alpha;

 __u8 thumb_bf_coeff0;
 __u8 thumb_bf_coeff1;
 __u8 thumb_bf_coeff2;
 __u8 thumb_bf_coeff3;

 __u8 loflt_uv_gain;
 __u8 loflt_vsigma;
 __u8 exp_x_shift_bit;
 __u16 loflt_wgt_slope;

 __u8 loflt_wgt_min_thred;
 __u8 loflt_wgt_max_limit;

 __u8 gaus_flt_coeff[6];

 __u8 hiflt_wgt_min_limit;
 __u16 gaus_flt_alpha;
 __u16 hiflt_alpha;

 __u8 hiflt_uv_gain;
 __u8 hiflt_cur_wgt;
 __u16 hiflt_global_vsigma;

 __u16 adj_offset;
 __u16 adj_scale;

 __u8 sgm_ratio[13];
 __u16 bf_merge_max_limit;

 __u8 loflt_global_sgm_ratio;
 __u8 loflt_global_sgm_ratio_alpha;
 __u16 bf_alpha_max_limit;

 __u8 cur_wgt[13];

 __u16 hiflt_vsigma_idx[8];

 __u16 hiflt_vsigma[8];
} __attribute__ ((packed));

struct isp33_sharp_cfg {

 __u8 bypass;
 __u8 local_gain_bypass;
 __u8 tex_est_mode;
 __u8 max_min_flt_mode;
 __u8 detail_fusion_wgt_mode;
 __u8 noise_calc_mode;
 __u8 radius_step_mode;
 __u8 noise_curve_mode;
 __u8 gain_wgt_mode;
 __u8 detail_lp_en;
 __u8 debug_mode;

 __u16 fst_noise_scale;
 __u16 fst_sigma_scale;

 __u16 fst_sigma_offset;
 __u16 fst_wgt_scale;

 __u8 tex_wgt_mode;
 __u8 noise_est_alpha;

 __u16 sec_noise_scale;
 __u16 sec_sigma_scale;

 __u16 sec_sigma_offset;
 __u16 sec_wgt_scale;

 __u8 img_hpf_coeff[6];

 __u8 texWgt_flt_coeff0;
 __u8 texWgt_flt_coeff1;
 __u8 texWgt_flt_coeff2;

 __u8 detail_in_alpha;
 __u8 pre_bifilt_alpha;
 __u8 fusion_wgt_min_limit;
 __u8 fusion_wgt_max_limit;
 __u16 pre_bifilt_slope_fix;

 __u32 detail_fusion_slope_fix;

 __u8 luma_dx[7];

 __u16 pre_bifilt_vsigma_inv[8];

 __u8 pre_bifilt_coeff0;
 __u8 pre_bifilt_coeff1;
 __u8 pre_bifilt_coeff2;

 __u8 hi_detail_lpf_coeff[6];
 __u8 mi_detail_lpf_coeff[6];

 __u16 global_gain;
 __u8 gain_merge_alpha;
 __u8 local_gain_scale;

 __u8 edge_gain_max_limit;
 __u8 edge_gain_min_limit;
 __u8 detail_gain_max_limit;
 __u8 detail_gain_min_limit;

 __u8 hitex_gain_max_limit;
 __u8 hitex_gain_min_limit;

 __u8 edge_gain_slope;
 __u8 detail_gain_slope;
 __u8 hitex_gain_slope;

 __u16 edge_gain_offset;
 __u16 detail_gain_offset;
 __u16 hitex_gain_offset;

 __u16 edge_gain_sigma;
 __u16 detail_gain_sigma;

 __u16 pos_edge_wgt_scale;
 __u16 neg_edge_wgt_scale;

 __u8 pos_edge_strg;
 __u8 neg_edge_strg;
 __u8 overshoot_alpha;
 __u8 undershoot_alpha;

 __u8 edge_lpf_coeff[10];

 __u16 edge_wgt_val[17];

 __u8 luma2strg[8];

 __u16 center_x;
 __u16 center_y;

 __u16 flat_max_limit;
 __u16 edge_min_limit;

 __u32 tex_x_inv_fix0;
 __u32 tex_x_inv_fix1;
 __u32 tex_x_inv_fix2;

 __u16 tex2detail_strg[4];
 __u16 tex2loss_tex_in_hinr_strg[4];

 __u8 contrast2pos_strg[9];
 __u8 contrast2neg_strg[9];
 __u8 pos_detail_strg;
 __u8 neg_detail_strg;

 __u16 tex2detail_pos_clip[9];
 __u16 tex2detail_neg_clip[9];

 __u16 tex2grain_pos_clip[9];
 __u16 tex2grain_neg_clip[9];

 __u16 luma2detail_pos_clip[8];
 __u16 luma2detail_neg_clip[8];

 __u8 grain_strg;

 __u16 hue2strg[9];

 __u8 distance2strg[11];

 __u16 hi_tex_threshold[9];

 __u8 loss_tex_in_hinr_strg;

 __u16 noise_curve_ext[17];
 __u8 noise_count_thred_ratio;
 __u8 noise_clip_scale;

 __u16 noise_clip_min_limit;
 __u16 noise_clip_max_limit;
} __attribute__ ((packed));

struct isp33_enh_cfg {

 __u8 bypass;
 __u8 blf3_bypass;

 __u16 iir_inv_sigma;
 __u8 iir_soft_thed;
 __u8 iir_cur_wgt;

 __u16 blf3_inv_sigma;
 __u16 blf3_cur_wgt;
 __u8 blf3_thumb_cur_wgt;

 __u8 blf5_cur_wgt;
 __u16 blf5_inv_sigma;

 __u16 global_strg;

 __u16 lum2strg[17];

 __u16 detail2strg_idx[8];

 __u8 detail2strg_power0;
 __u8 detail2strg_power1;
 __u8 detail2strg_power2;
 __u8 detail2strg_power3;
 __u8 detail2strg_power4;
 __u8 detail2strg_power5;
 __u8 detail2strg_power6;

 __u16 detail2strg_val[8];

 __u8 pre_wet_frame_cnt0;
 __u8 pre_wet_frame_cnt1;

 __u8 iir_wr;
 __u8 iir[18][24];
} __attribute__ ((packed));

struct isp33_hist_cfg {

 __u8 bypass;
 __u8 mem_mode;

 __u8 count_scale;
 __u8 count_offset;
 __u16 count_min_limit;

 __u16 blk_het;
 __u16 blk_wid;

 __u8 thumb_row;
 __u8 thumb_col;

 __u16 merge_alpha;
 __u16 user_set;

 __u16 map_count_scale;
 __u8 gain_ref_wgt;

 __u8 flt_cur_wgt;
 __u16 flt_inv_sigma;

 __u8 pos_alpha[17];

 __u8 neg_alpha[17];

 __u8 stab_frame_cnt0;
 __u8 stab_frame_cnt1;

 __u8 saturate_scale;

 __u16 iir_wr;
 __u16 iir[(8 * 10)][16];
} __attribute__ ((packed));

struct isp33_drc_cfg {

 __u8 bypass_en;
 __u8 cmps_byp_en;
 __u8 gainx32_en;
 __u8 bf_lp_en;
 __u8 raw_dly_dis;

 __u16 position;
 __u16 compres_scl;
 __u8 offset_pow2;

 __u16 lpdetail_ratio;
 __u16 hpdetail_ratio;
 __u8 delta_scalein;

 __u8 bilat_wt_off;
 __u16 thumb_thd_neg;
 __u8 thumb_thd_enable;
 __u8 weicur_pix;

 __u8 cmps_offset_bits_int;
 __u8 cmps_fixbit_mode;
 __u16 drc_gas_t;

 __u16 thumb_clip;
 __u8 thumb_scale;

 __u16 range_sgm_inv0;
 __u16 range_sgm_inv1;

 __u8 weig_bilat;
 __u8 weight_8x8thumb;
 __u8 enable_soft_thd;
 __u16 bilat_soft_thd;

 __u16 gain_y[17];

 __u16 compres_y[17];

 __u16 scale_y[17];

 __u16 min_ogain;

 __u16 sfthd_y[17];
} __attribute__ ((packed));

struct isp33_rawawb_meas_cfg {
 __u8 bls2_en;

 __u8 rawawb_sel;
 __u8 bnr2awb_sel;
 __u8 drc2awb_sel;

 __u8 uv_en0;
 __u8 xy_en0;
 __u8 yuv3d_en0;
 __u8 yuv3d_ls_idx0;
 __u8 yuv3d_ls_idx1;
 __u8 yuv3d_ls_idx2;
 __u8 yuv3d_ls_idx3;
 __u8 in_rshift_to_12bit_en;
 __u8 in_overexposure_check_en;
 __u8 wind_size;
 __u8 rawlsc_bypass_en;
 __u8 light_num;
 __u8 uv_en1;
 __u8 xy_en1;
 __u8 yuv3d_en1;
 __u8 low12bit_val;

 __u8 blk_measure_enable;
 __u8 blk_measure_mode;
 __u8 blk_measure_xytype;
 __u8 blk_rtdw_measure_en;
 __u8 blk_measure_illu_idx;
 __u8 ds16x8_mode_en;
 __u8 blk_with_luma_wei_en;
 __u8 ovexp_2ddr_dis;
 __u16 in_overexposure_threshold;

 __u16 h_offs;
 __u16 v_offs;

 __u16 h_size;
 __u16 v_size;

 __u16 r_max;
 __u16 g_max;

 __u16 b_max;
 __u16 y_max;

 __u16 r_min;
 __u16 g_min;

 __u16 b_min;
 __u16 y_min;

 __u8 wp_luma_wei_en0;
 __u8 wp_luma_wei_en1;
 __u8 wp_blk_wei_en0;
 __u8 wp_blk_wei_en1;
 __u8 wp_hist_xytype;

 __u8 wp_luma_weicurve_y0;
 __u8 wp_luma_weicurve_y1;
 __u8 wp_luma_weicurve_y2;
 __u8 wp_luma_weicurve_y3;

 __u8 wp_luma_weicurve_y4;
 __u8 wp_luma_weicurve_y5;
 __u8 wp_luma_weicurve_y6;
 __u8 wp_luma_weicurve_y7;

 __u8 wp_luma_weicurve_y8;

 __u8 wp_luma_weicurve_w0;
 __u8 wp_luma_weicurve_w1;
 __u8 wp_luma_weicurve_w2;
 __u8 wp_luma_weicurve_w3;

 __u8 wp_luma_weicurve_w4;
 __u8 wp_luma_weicurve_w5;
 __u8 wp_luma_weicurve_w6;
 __u8 wp_luma_weicurve_w7;

 __u8 wp_luma_weicurve_w8;
 __u16 pre_wbgain_inv_r;

 __u16 pre_wbgain_inv_g;
 __u16 pre_wbgain_inv_b;

 __u16 vertex0_u_0;
 __u16 vertex0_v_0;

 __u16 vertex1_u_0;
 __u16 vertex1_v_0;

 __u16 vertex2_u_0;
 __u16 vertex2_v_0;

 __u16 vertex3_u_0;
 __u16 vertex3_v_0;

 __u32 islope01_0;

 __u32 islope12_0;

 __u32 islope23_0;

 __u32 islope30_0;

 __u16 vertex0_u_1;
 __u16 vertex0_v_1;

 __u16 vertex1_u_1;
 __u16 vertex1_v_1;

 __u16 vertex2_u_1;
 __u16 vertex2_v_1;

 __u16 vertex3_u_1;
 __u16 vertex3_v_1;

 __u32 islope01_1;

 __u32 islope12_1;

 __u32 islope23_1;

 __u32 islope30_1;

 __u16 vertex0_u_2;
 __u16 vertex0_v_2;

 __u16 vertex1_u_2;
 __u16 vertex1_v_2;

 __u16 vertex2_u_2;
 __u16 vertex2_v_2;

 __u16 vertex3_u_2;
 __u16 vertex3_v_2;

 __u32 islope01_2;

 __u32 islope12_2;

 __u32 islope23_2;

 __u32 islope30_2;

 __u16 vertex0_u_3;
 __u16 vertex0_v_3;

 __u16 vertex1_u_3;
 __u16 vertex1_v_3;

 __u16 vertex2_u_3;
 __u16 vertex2_v_3;

 __u16 vertex3_u_3;
 __u16 vertex3_v_3;

 __u32 islope01_3;

 __u32 islope12_3;

 __u32 islope23_3;

 __u32 islope30_3;

 __u16 ccm_coeff0_r;
 __u16 ccm_coeff1_r;

 __u16 ccm_coeff2_r;

 __u16 ccm_coeff0_g;
 __u16 ccm_coeff1_g;

 __u16 ccm_coeff2_g;

 __u16 ccm_coeff0_b;
 __u16 ccm_coeff1_b;

 __u16 ccm_coeff2_b;

 __u16 wt0;
 __u16 wt1;

 __u16 wt2;

 __u16 mat0_x;
 __u16 mat0_y;

 __u16 mat1_x;
 __u16 mat1_y;

 __u16 mat2_x;
 __u16 mat2_y;

 __u16 nor_x0_0;
 __u16 nor_x1_0;

 __u16 nor_y0_0;
 __u16 nor_y1_0;

 __u16 big_x0_0;
 __u16 big_x1_0;

 __u16 big_y0_0;
 __u16 big_y1_0;

 __u16 nor_x0_1;
 __u16 nor_x1_1;

 __u16 nor_y0_1;
 __u16 nor_y1_1;

 __u16 big_x0_1;
 __u16 big_x1_1;

 __u16 big_y0_1;
 __u16 big_y1_1;

 __u16 nor_x0_2;
 __u16 nor_x1_2;

 __u16 nor_y0_2;
 __u16 nor_y1_2;

 __u16 big_x0_2;
 __u16 big_x1_2;

 __u16 big_y0_2;
 __u16 big_y1_2;

 __u16 nor_x0_3;
 __u16 nor_x1_3;

 __u16 nor_y0_3;
 __u16 nor_y1_3;

 __u16 big_x0_3;
 __u16 big_x1_3;

 __u16 big_y0_3;
 __u16 big_y1_3;

 __u8 exc_wp_region0_excen;
 __u8 exc_wp_region0_measen;
 __u8 exc_wp_region0_domain;
 __u8 exc_wp_region1_excen;
 __u8 exc_wp_region1_measen;
 __u8 exc_wp_region1_domain;
 __u8 exc_wp_region2_excen;
 __u8 exc_wp_region2_measen;
 __u8 exc_wp_region2_domain;
 __u8 exc_wp_region3_excen;
 __u8 exc_wp_region3_measen;
 __u8 exc_wp_region3_domain;
 __u8 exc_wp_region4_excen;
 __u8 exc_wp_region4_domain;
 __u8 exc_wp_region5_excen;
 __u8 exc_wp_region5_domain;
 __u8 exc_wp_region6_excen;
 __u8 exc_wp_region6_domain;
 __u8 multiwindow_en;

 __u16 multiwindow0_h_offs;
 __u16 multiwindow0_v_offs;

 __u16 multiwindow0_h_size;
 __u16 multiwindow0_v_size;

 __u16 multiwindow1_h_offs;
 __u16 multiwindow1_v_offs;

 __u16 multiwindow1_h_size;
 __u16 multiwindow1_v_size;

 __u16 multiwindow2_h_offs;
 __u16 multiwindow2_v_offs;

 __u16 multiwindow2_h_size;
 __u16 multiwindow2_v_size;

 __u16 multiwindow3_h_offs;
 __u16 multiwindow3_v_offs;

 __u16 multiwindow3_h_size;
 __u16 multiwindow3_v_size;

 __u16 exc_wp_region0_xu0;
 __u16 exc_wp_region0_xu1;

 __u16 exc_wp_region0_yv0;
 __u16 exc_wp_region0_yv1;

 __u16 exc_wp_region1_xu0;
 __u16 exc_wp_region1_xu1;

 __u16 exc_wp_region1_yv0;
 __u16 exc_wp_region1_yv1;

 __u16 exc_wp_region2_xu0;
 __u16 exc_wp_region2_xu1;

 __u16 exc_wp_region2_yv0;
 __u16 exc_wp_region2_yv1;

 __u16 exc_wp_region3_xu0;
 __u16 exc_wp_region3_xu1;

 __u16 exc_wp_region3_yv0;
 __u16 exc_wp_region3_yv1;

 __u16 exc_wp_region4_xu0;
 __u16 exc_wp_region4_xu1;

 __u16 exc_wp_region4_yv0;
 __u16 exc_wp_region4_yv1;

 __u16 exc_wp_region5_xu0;
 __u16 exc_wp_region5_xu1;

 __u16 exc_wp_region5_yv0;
 __u16 exc_wp_region5_yv1;

 __u16 exc_wp_region6_xu0;
 __u16 exc_wp_region6_xu1;

 __u16 exc_wp_region6_yv0;
 __u16 exc_wp_region6_yv1;

 __u8 exc_wp_region0_weight;
 __u8 exc_wp_region1_weight;
 __u8 exc_wp_region2_weight;
 __u8 exc_wp_region3_weight;

 __u8 exc_wp_region4_weight;
 __u8 exc_wp_region5_weight;
 __u8 exc_wp_region6_weight;

 __u8 wp_blk_wei_w[225];

 struct isp2x_bls_fixed_val bls2_val;
} __attribute__ ((packed));

struct isp33_isp_other_cfg {
 struct isp32_bls_cfg bls_cfg;
 struct isp39_dpcc_cfg dpcc_cfg;
 struct isp3x_lsc_cfg lsc_cfg;
 struct isp32_awb_gain_cfg awb_gain_cfg;
 struct isp33_gic_cfg gic_cfg;
 struct isp33_debayer_cfg debayer_cfg;
 struct isp33_ccm_cfg ccm_cfg;
 struct isp3x_gammaout_cfg gammaout_cfg;
 struct isp2x_cproc_cfg cproc_cfg;
 struct isp33_drc_cfg drc_cfg;
 struct isp32_hdrmge_cfg hdrmge_cfg;
 struct isp33_enh_cfg enh_cfg;
 struct isp33_hist_cfg hist_cfg;
 struct isp33_hsv_cfg hsv_cfg;
 struct isp32_ldch_cfg ldch_cfg;
 struct isp33_bay3d_cfg bay3d_cfg;
 struct isp33_ynr_cfg ynr_cfg;
 struct isp33_cnr_cfg cnr_cfg;
 struct isp33_sharp_cfg sharp_cfg;
 struct isp33_cac_cfg cac_cfg;
 struct isp3x_gain_cfg gain_cfg;
 struct isp21_csm_cfg csm_cfg;
 struct isp21_cgc_cfg cgc_cfg;
} __attribute__ ((packed));

struct isp33_isp_meas_cfg {
 struct isp33_rawawb_meas_cfg rawawb;
 struct isp2x_rawaebig_meas_cfg rawae0;
 struct isp2x_rawaebig_meas_cfg rawae3;
 struct isp2x_rawhistbig_cfg rawhist0;
 struct isp2x_rawhistbig_cfg rawhist3;
} __attribute__ ((packed));

struct isp33_isp_params_cfg {
 __u64 module_en_update;
 __u64 module_ens;
 __u64 module_cfg_update;

 __u32 frame_id;
 struct isp33_isp_meas_cfg meas;
 struct isp33_isp_other_cfg others;
} __attribute__ ((packed));

struct rkisp33_thunderboot_resmem_head {
 struct rkisp_thunderboot_resmem_head head;
 struct isp33_isp_params_cfg cfg;
} __attribute__ ((packed));

struct isp33_sharp_stat {
 __u16 noise_curve[17];
} __attribute__ ((packed));

struct isp33_bay3d_stat {
 __u32 sigma_num;
 __u16 sigma_y[20];
} __attribute__ ((packed));

struct isp33_enh_stat {
 __u8 iir[18][24];
} __attribute__ ((packed));

struct isp33_hist_stat {
 __u16 iir[(8 * 10)][16];
} __attribute__ ((packed));

struct isp33_rawae_mean_lum {
 __u32 g:12;
 __u32 b:10;
 __u32 r:10;
} __attribute__ ((packed));

struct isp33_rawae_mean_line {

 struct isp33_rawae_mean_lum blk_x[15];
 __u32 reserved;
} __attribute__ ((packed));

struct isp33_rawae_stat {
 struct isp33_rawae_mean_line blk_y[15];
 __u32 wnd1_sumg;
 __u32 wnd1_sumb;
 __u32 wnd1_sumr;
 __u32 reserved0;
 __u32 wnd2_sumg;
 __u32 wnd2_sumb;
 __u32 wnd2_sumr;
 __u32 reserved1;
} __attribute__ ((packed));

struct isp33_rawhist_stat {
 __u32 bin[256];
} __attribute__ ((packed));

struct isp33_rawawb_mean_ramdata {
 __u64 b:18;
 __u64 g:18;
 __u64 r:18;
 __u64 wp:10;
} __attribute__ ((packed));

struct isp33_rawawb_mean_ramdata_line {
 struct isp33_rawawb_mean_ramdata ramdata_blk_x[15];
 __u32 reserved[2];
} __attribute__ ((packed));

struct isp33_rawawb_mean_sum {
 __u32 rgain_nor;
 __u32 bgain_nor;
 __u32 wp_num_nor;
 __u32 wp_num2;

 __u32 rgain_big;
 __u32 bgain_big;
 __u32 wp_num_big;
 __u32 reserved;
} __attribute__ ((packed));

struct isp33_rawawb_mean_sum_exc {
 __u32 rgain_exc;
 __u32 bgain_exc;
 __u32 wp_num_exc;
 __u32 reserved;
} __attribute__ ((packed));

struct isp33_rawawb_stat {
 struct isp33_rawawb_mean_ramdata_line ramdata_blk_y[15];
 struct isp33_rawawb_mean_sum sum[4];
 __u16 yhist[8];
 struct isp33_rawawb_mean_sum_exc sum_exc[4];
} __attribute__ ((packed));

struct isp33_stat {

 struct isp33_rawae_stat rawae3;
 struct isp33_rawhist_stat rawhist3;
 struct isp33_rawae_stat rawae0;
 struct isp33_rawhist_stat rawhist0;
 struct isp33_rawawb_stat rawawb;

 struct isp33_bay3d_stat bay3d;
 struct isp33_sharp_stat sharp;
 struct isp33_enh_stat enh;
 struct isp33_hist_stat hist;
 struct isp32_info2ddr_stat info2ddr;
} __attribute__ ((packed));

struct rkisp33_stat_buffer {
 struct isp33_stat stat;
 __u32 meas_type;
 __u32 frame_id;
 __u32 params_id;
} __attribute__ ((packed));


struct isp35_bls_cfg {
 __u8 enable_auto;
 __u8 en_windows;
 __u8 bls1_en;

 __u8 bls_samples;

 struct isp2x_window bls_window1;
 struct isp2x_window bls_window2;
 struct isp2x_bls_fixed_val fixed_val;
 struct isp2x_bls_fixed_val bls1_val;

 __u16 isp_ob_offset;
 __u16 isp_ob_offset1;
 __u16 isp_ob_predgain;
 __u32 isp_ob_max;
} __attribute__ ((packed));

struct isp35_hdrmge_cfg {

 __u8 short_base_en;
 __u8 frame_mode;
 __u8 dbg_mode;
 __u8 channel_detection_en;
 __u8 s_base_mode;

 __u16 short_gain;
 __u16 short_inv_gain;

 __u16 medium_gain;
 __u16 medium_inv_gain;

 __u8 long_gain;

 __u8 ms_diff_scale;
 __u8 ms_diff_offset;
 __u8 lm_diff_scale;
 __u8 lm_diff_offset;

 __u16 ms_abs_diff_scale;
 __u16 ms_abs_diff_thred_min_limit;
 __u16 ms_adb_diff_thred_max_limit;

 __u16 lm_abs_diff_scale;
 __u16 lm_abs_diff_thred_min_limit;
 __u16 lm_abs_diff_thred_max_limit;

 __u16 ms_luma_diff2wgt[17];
 __u16 lm_luma_diff2wgt[17];

 __u16 luma2wgt[17];
 __u16 ms_raw_diff2wgt[17];
 __u16 lm_raw_diff2wgt[17];

 __u16 channel_detn_short_gain;
 __u16 channel_detn_medium_gain;

 __u16 mid_luma_scale;

 __u16 mid_luma_thred_max_limit;
 __u16 mid_luma_thred_min_limit;
} __attribute__ ((packed));

struct isp35_hsv_cfg {
 __u8 hsv_1dlut0_en;
 __u8 hsv_1dlut1_en;
 __u8 hsv_2dlut0_en;
 __u8 hsv_2dlut1_en;
 __u8 hsv_2dlut2_en;
 __u8 hsv_2dlut12_cfg;

 __u8 hsv_1dlut0_idx_mode;
 __u8 hsv_1dlut1_idx_mode;
 __u8 hsv_2dlut0_idx_mode;
 __u8 hsv_2dlut1_idx_mode;
 __u8 hsv_2dlut2_idx_mode;
 __u8 hsv_1dlut0_item_mode;
 __u8 hsv_1dlut1_item_mode;
 __u8 hsv_2dlut0_item_mode;
 __u8 hsv_2dlut1_item_mode;
 __u8 hsv_2dlut2_item_mode;

 __u16 lut0_1d[65];
 __u16 lut1_1d[65];
 __u16 lut0_2d[17][17];
 __u16 lut1_2d[17][17];
 __u16 lut2_2d[17][17];
} __attribute__ ((packed));

struct isp35_debayer_cfg {

 __u8 bypass;
 __u8 g_out_flt_en;
 __u8 cnt_flt_en;

 __u8 luma_dx[7];

 __u8 g_interp_clip_en;
 __u8 hi_texture_thred;
 __u8 hi_drct_thred;
 __u8 lo_drct_thred;
 __u8 drct_method_thred;
 __u8 g_interp_sharp_strg_max_limit;

 __s8 lo_drct_flt_coeff1;
 __s8 lo_drct_flt_coeff2;
 __s8 lo_drct_flt_coeff3;
 __s8 lo_drct_flt_coeff4;

 __s8 hi_drct_flt_coeff1;
 __s8 hi_drct_flt_coeff2;
 __s8 hi_drct_flt_coeff3;
 __s8 hi_drct_flt_coeff4;

 __u16 g_interp_sharp_strg_offset;
 __u8 grad_lo_flt_alpha;

 __u16 drct_offset[8];

 __u8 gflt_mode;
 __u16 gflt_ratio;
 __u16 gflt_offset;

 __s8 gflt_coe0;
 __s8 gflt_coe1;
 __s8 gflt_coe2;

 __u16 gflt_vsigma[8];

 __u8 cnr_lo_guide_lpf_coe0;
 __u8 cnr_lo_guide_lpf_coe1;
 __u8 cnr_lo_guide_lpf_coe2;

 __u8 cnr_pre_flt_coe0;
 __u8 cnr_pre_flt_coe1;
 __u8 cnr_pre_flt_coe2;

 __u8 cnr_alpha_lpf_coe0;
 __u8 cnr_alpha_lpf_coe1;
 __u8 cnr_alpha_lpf_coe2;

 __u16 cnr_log_grad_offset;
 __u16 cnr_log_guide_offset;
 __u8 cnr_trans_en;

 __u16 cnr_moire_alpha_offset;
 __u32 cnr_moire_alpha_scale;

 __u16 cnr_edge_alpha_offset;
 __u32 cnr_edge_alpha_scale;

 __u8 cnr_lo_flt_strg_inv;
 __u8 cnr_lo_flt_strg_shift;
 __u16 cnr_lo_flt_wgt_slope;

 __u8 cnr_lo_flt_wgt_max_limit;
 __u8 cnr_lo_flt_wgt_min_thred;

 __u16 cnr_hi_flt_vsigma;
 __u8 cnr_hi_flt_wgt_min_limit;
 __u8 cnr_hi_flt_cur_wgt;
} __attribute__ ((packed));

struct isp35_bay3d_cfg {

 __u8 bypass_en;
 __u8 iir_wr_src;
 __u8 out_use_pre_mode;
 __u8 motion_est_en;
 __u8 iir_rw_fmt;

 __u8 transf_bypass_en;
 __u8 tnrsigma_curve_double_en;
 __u8 md_large_lo_use_mode;
 __u8 md_large_lo_min_filter_bypass_en;
 __u8 md_large_lo_gauss_filter_bypass_en;
 __u8 md_large_lo_md_wgt_bypass_en;
 __u8 pre_pix_out_mode;
 __u8 motion_detect_bypass_en;
 __u8 lpf_hi_bypass_en;
 __u8 lo_diff_vfilt_bypass_en;
 __u8 lpf_lo_bypass_en;
 __u8 lo_wgt_hfilt_en;
 __u8 lo_diff_hfilt_en;
 __u8 sig_hfilt_en;
 __u8 lo_detection_bypass_en;
 __u8 lo_mge_wgt_mode;
 __u8 pre_spnr_out_en;
 __u8 md_only_lo_en;
 __u8 cur_spnr_out_en;
 __u8 md_wgt_out_en;

 __u8 cur_spnr_filter_bypass_en;
 __u8 pre_spnr_hi_filter_gic_en;
 __u8 pre_spnr_hi_filter_gic_enhance_en;
 __u8 spnr_presigma_use_en;
 __u8 pre_spnr_lo_filter_bypass_en;
 __u8 pre_spnr_hi_filter_bypass_en;
 __u8 pre_spnr_sigma_curve_double_en;
 __u8 pre_spnr_hi_guide_filter_bypass_en;
 __u8 pre_spnr_sigma_idx_filt_bypass_en;
 __u8 pre_spnr_sigma_idx_filt_mode;
 __u8 pre_spnr_hi_noise_ctrl_en;
 __u8 pre_spnr_hi_filter_wgt_mode;
 __u8 pre_spnr_lo_filter_wgt_mode;
 __u8 pre_spnr_hi_filter_rb_wgt_mode;
 __u8 pre_spnr_lo_filter_rb_wgt_mode;
 __u8 pre_hi_gic_lp_en;
 __u8 pre_hi_bf_lp_en;
 __u8 pre_lo_avg_lp_en;
 __u8 pre_spnr_dpc_flt_en;
 __u8 pre_spnr_dpc_nr_bal_mode;
 __u8 pre_spnr_dpc_flt_mode;
 __u8 pre_spnr_dpc_flt_prewgt_en;

 __u8 transf_mode;
 __u8 wgt_cal_mode;
 __u8 mge_wgt_ds_mode;
 __u8 kalman_wgt_ds_mode;
 __u8 mge_wgt_hdr_sht_thred;
 __u8 sigma_calc_mge_wgt_hdr_sht_thred;

 __u16 transf_mode_offset;
 __u8 transf_mode_scale;
 __u16 itransf_mode_offset;

 __u32 transf_data_max_limit;

 __u16 pre_spnr_sigma_ctrl_scale;

 __u8 pre_spnr_hi_guide_out_wgt;

 __u8 cur_spnr_filter_coeff[6];

 __u16 pre_spnr_luma2sigma_x[16];

 __u16 pre_spnr_luma2sigma_y[16];

 __u16 pre_spnr_hi_sigma_scale;

 __u8 pre_spnr_hi_wgt_calc_scale;

 __u8 pre_spnr_hi_filter_wgt_min_limit;
 __u8 pre_spnr_hi_wgt_calc_offset;

 __u8 pre_spnr_hi_filter_out_wgt;
 __u8 pre_spnr_sigma_offset;
 __u8 pre_spnr_sigma_hdr_sht_offset;

 __u16 pre_spnr_sigma_scale;
 __u16 pre_spnr_sigma_hdr_sht_scale;

 __u8 pre_spnr_hi_filter_coeff[6];

 __u16 pre_spnr_lo_sigma_scale;

 __u8 pre_spnr_lo_wgt_calc_offset;
 __u8 pre_spnr_lo_wgt_calc_scale;

 __u16 pre_spnr_hi_noise_ctrl_scale;
 __u8 pre_spnr_hi_noise_ctrl_offset;

 __u16 tnr_luma2sigma_x[20];

 __u16 tnr_luma2sigma_y[20];

 __u16 lpf_hi_coeff[9];

 __u16 lpf_lo_coeff[9];

 __u8 sigma_idx_filt_coeff[6];
 __u16 lo_wgt_cal_first_line_sigma_scale;

 __u8 lo_diff_vfilt_wgt;
 __u8 lo_wgt_vfilt_wgt;
 __u8 sig_first_line_scale;
 __u8 lo_diff_first_line_scale;

 __u16 lo_wgt_cal_offset;
 __u16 lo_wgt_cal_scale;

 __u16 lo_wgt_cal_max_limit;
 __u16 mode0_base_ratio;

 __u16 lo_diff_wgt_cal_offset;
 __u16 lo_diff_wgt_cal_scale;

 __u16 lo_mge_pre_wgt_offset;
 __u16 lo_mge_pre_wgt_scale;

 __u16 mode0_lo_wgt_scale;
 __u16 mode0_lo_wgt_hdr_sht_scale;

 __u16 mode1_lo_wgt_scale;
 __u16 mode1_lo_wgt_hdr_sht_scale;

 __u16 mode1_wgt_scale;
 __u16 mode1_wgt_hdr_sht_scale;

 __u16 mode1_lo_wgt_offset;
 __u16 mode1_lo_wgt_hdr_sht_offset;

 __u16 auto_sigma_count_wgt_thred;
 __u16 mode1_wgt_min_limit;
 __u16 mode1_wgt_offset;

 __u32 tnr_out_sigma_sq;

 __u16 lo_wgt_clip_min_limit;
 __u16 lo_wgt_clip_hdr_sht_min_limit;

 __u16 lo_wgt_clip_max_limit;
 __u16 lo_wgt_clip_hdr_sht_max_limit;

 __u16 lo_pre_gg_soft_thresh_scale;
 __u16 lo_pre_rb_soft_thresh_scale;

 __u16 lo_pre_soft_thresh_max_limit;
 __u16 lo_pre_soft_thresh_min_limit;

 __u8 cur_spnr_hi_wgt_min_limit;
 __u8 pre_spnr_hi_wgt_min_limit;
 __u16 motion_est_lo_wgt_thred;

 __u16 pix_max_limit;

 __u32 sigma_num_th;

 __u16 out_use_hi_noise_bal_nr_strg;
 __u16 out_use_md_noise_bal_nr_strg;
 __u8 gain_out_max_limit;

 __u16 sigma_scale;
 __u16 sigma_hdr_sht_scale;

 __u16 lo_wgt_vfilt_offset;
 __u16 lo_diff_vfilt_offset;
 __u8 lo_wgt_cal_first_line_vfilt_wgt;

 __u8 lo_wgt_vfilt_scale;
 __u8 lo_diff_vfilt_scale_bit;
 __u8 lo_diff_vfilt_scale;
 __u8 lo_diff_first_line_vfilt_wgt;

 __u16 motion_est_up_mvx_cost_offset;
 __u16 motion_est_up_mvx_cost_scale;
 __u8 motion_est_sad_vert_wgt0;

 __u16 motion_est_up_left_mvx_cost_offset;
 __u16 motion_est_up_left_mvx_cost_scale;
 __u8 motion_est_sad_vert_wgt1;

 __u16 motion_est_up_right_mvx_cost_offset;
 __u16 motion_est_up_right_mvx_cost_scale;
 __u8 motion_est_sad_vert_wgt2;

 __u16 lo_wgt_clip_motion_max_limit;

 __u16 mode1_wgt_max_limit;

 __u16 mode0_wgt_out_max_limit;
 __u16 mode0_wgt_out_offset;

 __u8 lo_wgt_hflt_coeff2;
 __u8 lo_wgt_hflt_coeff1;
 __u8 lo_wgt_hflt_coeff0;
 __u8 sig_hflt_coeff2;
 __u8 sig_hflt_coeff1;
 __u8 sig_hflt_coeff0;

 __u8 lo_dif_hflt_coeff2;
 __u8 lo_dif_hflt_coeff1;
 __u8 lo_dif_hflt_coeff0;

 __u8 pre_spnr_dpc_bright_str;
 __u8 pre_spnr_dpc_dark_str;
 __u8 pre_spnr_dpc_str;
 __u8 pre_spnr_dpc_wk_scale;
 __u8 pre_spnr_dpc_wk_offset;

 __u16 pre_spnr_dpc_nr_bal_str;
 __u16 pre_spnr_dpc_soft_thr_scale;

 __u8 pre_spnr_lo_val_wgt_out_wgt;
 __u8 pre_spnr_lo_filter_out_wgt;
 __u8 pre_spnr_lo_filter_wgt_min;

 __u8 md_large_lo_md_wgt_offset;
 __u16 md_large_lo_md_wgt_scale;

 __u16 md_large_lo_wgt_cut_offset;
 __u16 md_large_lo_wgt_add_offset;

 __u16 md_large_lo_wgt_scale;

 __u16 out_use_hi_noise_bal_nr_off;
 __u16 out_use_md_noise_bal_nr_off;


 __u8 btnr_ldc_en;
 __u8 b3dldcv_map13p3_en;
 __u8 b3dldcv_force_map_en;

 __u8 b3dldch_en;
 __u8 b3dldch_map13p3_en;
 __u8 b3dldch_force_map_en;

 __u8 btnr_ldcltp_mode;
 __u16 btnr_ldc_wrap_ext_bound_offset;

 __u16 b3dldc_last;

 __s32 lut_buf_fd;
} __attribute__ ((packed));

struct isp35_ai_cfg {

 __u8 aiisp_raw12_msb;
 __u8 aiisp_gain_mode;
 __u8 aiisp_curve_en;
 __u8 aipre_iir_en;
 __u8 aipre_iir2ddr_en;
 __u8 aipre_gain_en;
 __u8 aipre_gain2ddr_en;
 __u8 aipre_luma2gain_dis;
 __u8 aipre_nl_ddr_mode;
 __u8 aipre_yraw_sel;
 __u8 aipre_gain_bypass;
 __u8 aipre_gain_mode;
 __u8 aipre_narmap_inv;

 __u16 aiisp_sigma_y[33];

 __u8 aipre_scale;
 __s8 aipre_zp;
 __u16 aipre_black_lvl;

 __u8 aipre_gain_alpha;
 __u8 aipre_global_gain;
 __u16 aipre_gain_ratio;

 __u16 aipre_sigma_y[33];

 __u8 aipre_noise_mot_offset;
 __s8 aipre_noise_mot_gain;
 __u16 aipre_noise_luma_offset;

 __u16 aipre_noise_luma_gain;
 __u16 aipre_noise_luma_clip;
 __u8 aipre_noise_luma_static;

 __u8 aipre_nar_manual;
 __u8 aipre_nar_manual_alpha;


 __u8 pyr_yraw_mode;
 __u8 pyr_sigma_en;
 __u8 pyr_yraw_sel;
 __u8 pyr_gain_leftshift;
 __u8 pyr_blacklvl_sig;

 __u8 pyr_sigma_y[81];
} __attribute__ ((packed));

struct isp35_ynr_cfg {

 __u8 hi_spnr_bypass;
 __u8 mi_spnr_bypass;
 __u8 lo_spnr_bypass;
 __u8 rnr_en;
 __u8 tex2lo_strg_en;
 __u8 hi_lp_en;
 __u8 dsfilt_bypass;
 __u8 tex2wgt_en;

 __u16 global_set_gain;
 __u8 gain_merge_alpha;
 __u8 local_gain_scale;

 __u16 lo_spnr_gain2strg[9];

 __u16 rnr_max_radius;

 __u16 rnr_center_h;
 __u16 rnr_center_v;

 __u8 radius2strg[17];

 __u16 luma2sima_x[17];

 __u16 luma2sima_y[17];

 __u8 mi_spnr_tex2wgt_scale[9];

 __u8 lo_spnr_tex2wgt_scale[9];

 __u16 hi_spnr_sigma_min_limit;
 __u8 hi_spnr_local_gain_alpha;
 __u16 hi_spnr_strg;

 __u8 hi_spnr_filt_coeff[4];

 __u16 hi_spnr_filt_wgt_offset;
 __u16 hi_spnr_filt_center_wgt;

 __u16 hi_spnr_filt1_coeff[6];

 __u16 hi_spnr_filt1_tex_thred;
 __u16 hi_spnr_filt1_tex_scale;
 __u16 hi_spnr_filt1_wgt_alpha;

 __u8 mi_spnr_filt_coeff0;
 __u8 mi_spnr_filt_coeff1;
 __u8 mi_spnr_filt_coeff2;
 __u8 mi_spnr_filt_coeff3;
 __u8 mi_spnr_filt_coeff4;

 __u16 mi_spnr_strg;
 __u16 mi_spnr_soft_thred_scale;

 __u8 mi_spnr_wgt;
 __u8 mi_ehance_scale_en;
 __u8 mi_ehance_scale;
 __u16 mi_spnr_filt_center_wgt;

 __u16 dsfilt_diff_offset;
 __u16 dsfilt_center_wgt;
 __u16 dsfilt_strg;

 __u16 lo_spnr_strg;
 __u16 lo_spnr_soft_thred_scale;

 __u16 lo_spnr_thumb_thred_scale;
 __u16 tex2lo_strg_mantissa;
 __u8 tex2lo_strg_exponent;

 __u8 lo_spnr_wgt;
 __u16 lo_spnr_filt_center_wgt;
 __u8 lo_enhance_scale;

 __u16 tex2lo_strg_upper_thred;
 __u16 tex2lo_strg_lower_thred;

 __u8 lo_gain2wgt[9];
} __attribute__ ((packed));

struct isp35_cnr_cfg {

 __u8 exgain_bypass;
 __u8 yuv422_mode;
 __u8 thumb_mode;
 __u8 uv_dis;
 __u8 hiflt_wgt0_mode;
 __u8 local_alpha_dis;
 __u8 loflt_coeff;
 __u8 hsv_alpha_en;

 __u16 global_gain;
 __u8 global_gain_alpha;
 __u8 local_gain_scale;

 __u16 lobfflt_vsigma_uv;
 __u16 lobfflt_vsigma_y;

 __u16 lobfflt_alpha;

 __u8 thumb_bf_coeff0;
 __u8 thumb_bf_coeff1;
 __u8 thumb_bf_coeff2;
 __u8 thumb_bf_coeff3;

 __u8 loflt_uv_gain;
 __u8 loflt_vsigma;
 __u8 exp_x_shift_bit;
 __u16 loflt_wgt_slope;

 __u8 loflt_wgt_min_thred;
 __u8 loflt_wgt_max_limit;

 __u8 gaus_flt_coeff[6];

 __u16 gaus_flt_alpha;
 __u8 hiflt_wgt_min_limit;
 __u16 hiflt_alpha;

 __u8 hiflt_uv_gain;
 __u16 hiflt_global_vsigma;
 __u8 hiflt_cur_wgt;

 __u16 adj_offset;
 __u16 adj_scale;

 __u8 sgm_ratio[13];
 __u16 bf_merge_max_limit;

 __u8 loflt_global_sgm_ratio;
 __u8 loflt_global_sgm_ratio_alpha;
 __u16 bf_alpha_max_limit;

 __u8 cur_wgt[13];

 __u16 hiflt_vsigma_idx[8];

 __u16 hiflt_vsigma[8];

 __u8 lo_flt_vsigma[13];

 __u8 hsv_adj_alpha_table[10];

 __u8 sat_adj_alpha_table[10];

 __u8 gain_adj_alpha_table[10];
} __attribute__ ((packed));

struct isp35_sharp_cfg {

 __u8 bypass;
 __u8 local_gain_bypass;
 __u8 tex_est_mode;
 __u8 max_min_flt_mode;
 __u8 detail_fusion_wgt_mode;
 __u8 noise_calc_mode;
 __u8 radius_step_mode;
 __u8 noise_curve_mode;
 __u8 gain_wgt_mode;
 __u8 detail_lp_en;
 __u8 debug_mode;

 __u16 fst_noise_scale;
 __u16 fst_sigma_scale;

 __u16 fst_sigma_offset;
 __u16 fst_wgt_scale;

 __u8 tex_wgt_mode;
 __u8 noise_est_alpha;

 __u16 sec_noise_scale;
 __u16 sec_sigma_scale;

 __u16 sec_sigma_offset;
 __u16 sec_wgt_scale;

 __u8 img_hpf_coeff[6];

 __u8 texWgt_flt_coeff0;
 __u8 texWgt_flt_coeff1;
 __u8 texWgt_flt_coeff2;

 __u8 detail_in_alpha;
 __u8 pre_bifilt_alpha;
 __u8 fusion_wgt_min_limit;
 __u8 fusion_wgt_max_limit;
 __u16 pre_bifilt_slope_fix;

 __u8 luma_dx[7];

 __u16 pre_bifilt_vsigma_inv[8];

 __u8 pre_bifilt_coeff0;
 __u8 pre_bifilt_coeff1;
 __u8 pre_bifilt_coeff2;

 __u8 hi_detail_lpf_coeff[6];
 __u8 mi_detail_lpf_coeff[6];

 __u16 global_gain;
 __u8 gain_merge_alpha;
 __u8 local_gain_scale;

 __u8 edge_gain_max_limit;
 __u8 edge_gain_min_limit;
 __u8 detail_gain_max_limit;
 __u8 detail_gain_min_limit;

 __u8 hitex_gain_max_limit;
 __u8 hitex_gain_min_limit;

 __u8 edge_gain_slope;
 __u8 detail_gain_slope;
 __u8 hitex_gain_slope;

 __u16 edge_gain_offset;
 __u16 detail_gain_offset;
 __u16 hitex_gain_offset;

 __u16 edge_gain_sigma;
 __u16 detail_gain_sigma;

 __u16 pos_edge_wgt_scale;
 __u16 neg_edge_wgt_scale;

 __u8 pos_edge_strg;
 __u8 neg_edge_strg;
 __u8 overshoot_alpha;
 __u8 undershoot_alpha;

 __u8 edge_bpf_coeff[10];

 __u16 edge_wgt_val[17];

 __u8 luma2strg[8];

 __u16 center_x;
 __u16 center_y;

 __u16 flat_max_limit;
 __u16 edge_min_limit;

 __u32 tex_x_inv_fix0;
 __u32 tex_x_inv_fix1;
 __u32 tex_x_inv_fix2;

 __u16 tex2loss_tex_in_hinr_strg[4];

 __u8 contrast2pos_strg[9];
 __u8 contrast2neg_strg[9];
 __u8 pos_detail_strg;
 __u8 neg_detail_strg;

 __u16 tex2detail_pos_clip[9];
 __u16 tex2detail_neg_clip[9];

 __u16 tex2grain_pos_clip[9];
 __u16 tex2grain_neg_clip[9];

 __u16 luma2detail_pos_clip[8];
 __u16 luma2detail_neg_clip[8];

 __u8 grain_strg;

 __u16 hue2strg[9];

 __u8 distance2strg[11];

 __u16 tex2detail_strg[9];

 __u16 hi_tex_threshold[9];

 __u16 tex2mf_detail_strg[9];

 __u8 loss_tex_in_hinr_strg;

 __u16 noise_curve_ext[17];
 __u8 noise_count_thred_ratio;
 __u8 noise_clip_scale;

 __u16 noise_clip_min_limit;
 __u16 noise_clip_max_limit;

 __u8 edge_wgt_flt_coeff0;
 __u8 edge_wgt_flt_coeff1;
 __u8 edge_wgt_flt_coeff2;

 __u16 edge_glb_clip_thred;
 __u16 pos_edge_clip;
 __u16 neg_edge_clip;

 __u8 mf_detail_data_alpha;
 __u8 pos_mf_detail_strg;
 __u8 neg_mf_detail_strg;

 __u16 mf_detail_pos_clip;
 __u16 sharp_mf_detail_neg_clip;

 __u8 staturation2strg[9];
 __u16 lo_saturation_strg;
} __attribute__ ((packed));

struct isp35_enh_cfg {

 __u8 bypass;
 __u8 blf3_bypass;

 __u16 iir_inv_sigma;
 __u8 iir_soft_thed;
 __u8 iir_cur_wgt;

 __u16 blf3_inv_sigma;
 __u16 blf3_cur_wgt;
 __u8 blf3_thumb_cur_wgt;

 __u8 blf5_cur_wgt;
 __u16 blf5_inv_sigma;

 __u16 global_strg;

 __u16 lum2strg[17];

 __u16 detail2strg_idx[8];

 __u8 detail2strg_power0;
 __u8 detail2strg_power1;
 __u8 detail2strg_power2;
 __u8 detail2strg_power3;
 __u8 detail2strg_power4;
 __u8 detail2strg_power5;
 __u8 detail2strg_power6;

 __u16 detail2strg_val[8];

 __u8 pre_wet_frame_cnt0;
 __u8 pre_wet_frame_cnt1;

 __u8 iir_wr;
 __u8 iir[24][32];
} __attribute__ ((packed));

struct isp35_drc_cfg {

 __u8 bypass_en;
 __u8 cmps_byp_en;
 __u8 gainx32_en;

 __u16 gain_idx_luma_scale;
 __u16 comps_idx_luma_scale;
 __u8 log_transform_offset_bits;

 __u16 lo_detail_ratio;
 __u16 hi_detail_ratio;
 __u8 adj_gain_idx_luma_scale;

 __u8 bifilt_wgt_offset;
 __u16 thumb_thred_neg;
 __u8 thumb_thred_en;
 __u8 bifilt_cur_pixel_wgt;

 __u8 cmps_offset_bits;
 __u8 cmps_mode;
 __u16 filt_luma_soft_thred;

 __u16 thumb_max_limit;
 __u8 thumb_scale;

 __u16 hi_range_inv_sigma;
 __u16 lo_range_inv_sigma;

 __u8 bifilt_wgt;
 __u8 bifilt_hi_wgt;
 __u16 bifilt_soft_thred;
 __u8 bifilt_soft_thred_en;

 __u16 gain_y[17];

 __u16 compres_y[17];

 __u16 scale_y[17];

 __u16 comps_gain_min_limit;

 __u16 sfthd_y[17];

 __u8 max_luma_wgt;
 __u8 mid_luma_wgt;
 __u8 min_luma_wgt;
} __attribute__ ((packed));

struct isp35_rawawb_meas_cfg {
 __u8 bls2_en;

 __u8 rawawb_sel;
 __u8 bnr2awb_sel;
 __u8 drc2awb_sel;

 __u8 uv_en0;
 __u8 xy_en0;
 __u8 yuv3d_en0;
 __u8 yuv3d_ls_idx0;
 __u8 yuv3d_ls_idx1;
 __u8 yuv3d_ls_idx2;
 __u8 yuv3d_ls_idx3;
 __u8 in_rshift_to_12bit_en;
 __u8 in_overexposure_check_en;
 __u8 wind_size;
 __u8 rawlsc_bypass_en;
 __u8 light_num;
 __u8 uv_en1;
 __u8 xy_en1;
 __u8 yuv3d_en1;
 __u8 low12bit_val;

 __u8 blk_measure_enable;
 __u8 blk_measure_mode;
 __u8 blk_measure_xytype;
 __u8 blk_rtdw_measure_en;
 __u8 blk_measure_illu_idx;
 __u8 ds16x8_mode_en;
 __u8 blk_with_luma_wei_en;
 __u8 ovexp_2ddr_dis;
 __u8 bnr_be_sel;
 __u16 in_overexposure_threshold;

 __u16 h_offs;
 __u16 v_offs;

 __u16 h_size;
 __u16 v_size;

 __u16 r_max;
 __u16 g_max;

 __u16 b_max;
 __u16 y_max;

 __u16 r_min;
 __u16 g_min;

 __u16 b_min;
 __u16 y_min;

 __u8 wp_luma_wei_en0;
 __u8 wp_luma_wei_en1;
 __u8 wp_blk_wei_en0;
 __u8 wp_blk_wei_en1;
 __u8 wp_hist_xytype;

 __u8 wp_luma_weicurve_y0;
 __u8 wp_luma_weicurve_y1;
 __u8 wp_luma_weicurve_y2;
 __u8 wp_luma_weicurve_y3;

 __u8 wp_luma_weicurve_y4;
 __u8 wp_luma_weicurve_y5;
 __u8 wp_luma_weicurve_y6;
 __u8 wp_luma_weicurve_y7;

 __u8 wp_luma_weicurve_y8;

 __u8 wp_luma_weicurve_w0;
 __u8 wp_luma_weicurve_w1;
 __u8 wp_luma_weicurve_w2;
 __u8 wp_luma_weicurve_w3;

 __u8 wp_luma_weicurve_w4;
 __u8 wp_luma_weicurve_w5;
 __u8 wp_luma_weicurve_w6;
 __u8 wp_luma_weicurve_w7;

 __u8 wp_luma_weicurve_w8;
 __u16 pre_wbgain_inv_r;

 __u16 pre_wbgain_inv_g;
 __u16 pre_wbgain_inv_b;

 __u16 vertex0_u_0;
 __u16 vertex0_v_0;

 __u16 vertex1_u_0;
 __u16 vertex1_v_0;

 __u16 vertex2_u_0;
 __u16 vertex2_v_0;

 __u16 vertex3_u_0;
 __u16 vertex3_v_0;

 __u32 islope01_0;

 __u32 islope12_0;

 __u32 islope23_0;

 __u32 islope30_0;

 __u16 vertex0_u_1;
 __u16 vertex0_v_1;

 __u16 vertex1_u_1;
 __u16 vertex1_v_1;

 __u16 vertex2_u_1;
 __u16 vertex2_v_1;

 __u16 vertex3_u_1;
 __u16 vertex3_v_1;

 __u32 islope01_1;

 __u32 islope12_1;

 __u32 islope23_1;

 __u32 islope30_1;

 __u16 vertex0_u_2;
 __u16 vertex0_v_2;

 __u16 vertex1_u_2;
 __u16 vertex1_v_2;

 __u16 vertex2_u_2;
 __u16 vertex2_v_2;

 __u16 vertex3_u_2;
 __u16 vertex3_v_2;

 __u32 islope01_2;

 __u32 islope12_2;

 __u32 islope23_2;

 __u32 islope30_2;

 __u16 vertex0_u_3;
 __u16 vertex0_v_3;

 __u16 vertex1_u_3;
 __u16 vertex1_v_3;

 __u16 vertex2_u_3;
 __u16 vertex2_v_3;

 __u16 vertex3_u_3;
 __u16 vertex3_v_3;

 __u32 islope01_3;

 __u32 islope12_3;

 __u32 islope23_3;

 __u32 islope30_3;

 __u16 rgb2ryuvmat0_y;
 __u16 rgb2ryuvmat1_y;

 __u16 rgb2ryuvmat2_y;
 __u16 rgb2ryuvofs_y;

 __u16 rgb2ryuvmat0_u;
 __u16 rgb2ryuvmat1_u;

 __u16 rgb2ryuvmat2_u;
 __u16 rgb2ryuvofs_u;

 __u16 rgb2ryuvmat0_v;
 __u16 rgb2ryuvmat1_v;

 __u16 rgb2ryuvmat2_v;
 __u16 rgb2ryuvofs_v;

 __u16 coor_x1_ls0_y;
 __u16 vec_x21_ls0_y;

 __u16 coor_x1_ls0_u;
 __u16 vec_x21_ls0_u;

 __u16 coor_x1_ls0_v;
 __u16 vec_x21_ls0_v;

 __u8 dis_x1x2_ls0;
 __u8 rotu0_ls0;
 __u8 rotu1_ls0;

 __u8 rotu2_ls0;
 __u8 rotu3_ls0;
 __u8 rotu4_ls0;
 __u8 rotu5_ls0;

 __u16 th0_ls0;
 __u16 th1_ls0;

 __u16 th2_ls0;
 __u16 th3_ls0;

 __u16 th4_ls0;
 __u16 th5_ls0;

 __u16 coor_x1_ls1_y;
 __u16 vec_x21_ls1_y;

 __u16 coor_x1_ls1_u;
 __u16 vec_x21_ls1_u;

 __u16 coor_x1_ls1_v;
 __u16 vec_x21_ls1_v;

 __u8 dis_x1x2_ls1;
 __u8 rotu0_ls1;
 __u8 rotu1_ls1;

 __u8 rotu2_ls1;
 __u8 rotu3_ls1;
 __u8 rotu4_ls1;
 __u8 rotu5_ls1;

 __u16 th0_ls1;
 __u16 th1_ls1;

 __u16 th2_ls1;
 __u16 th3_ls1;

 __u16 th4_ls1;
 __u16 th5_ls1;

 __u16 coor_x1_ls2_y;
 __u16 vec_x21_ls2_y;

 __u16 coor_x1_ls2_u;
 __u16 vec_x21_ls2_u;

 __u16 coor_x1_ls2_v;
 __u16 vec_x21_ls2_v;

 __u8 dis_x1x2_ls2;
 __u8 rotu0_ls2;
 __u8 rotu1_ls2;

 __u8 rotu2_ls2;
 __u8 rotu3_ls2;
 __u8 rotu4_ls2;
 __u8 rotu5_ls2;

 __u16 th0_ls2;
 __u16 th1_ls2;

 __u16 th2_ls2;
 __u16 th3_ls2;

 __u16 th4_ls2;
 __u16 th5_ls2;

 __u16 coor_x1_ls3_y;
 __u16 vec_x21_ls3_y;

 __u16 coor_x1_ls3_u;
 __u16 vec_x21_ls3_u;

 __u16 coor_x1_ls3_v;
 __u16 vec_x21_ls3_v;

 __u8 dis_x1x2_ls3;
 __u8 rotu0_ls3;
 __u8 rotu1_ls3;

 __u8 rotu2_ls3;
 __u8 rotu3_ls3;
 __u8 rotu4_ls3;
 __u8 rotu5_ls3;

 __u16 th0_ls3;
 __u16 th1_ls3;

 __u16 th2_ls3;
 __u16 th3_ls3;

 __u16 th4_ls3;
 __u16 th5_ls3;

 __u16 ccm_coeff0_r;
 __u16 ccm_coeff1_r;

 __u16 ccm_coeff2_r;

 __u16 ccm_coeff0_g;
 __u16 ccm_coeff1_g;

 __u16 ccm_coeff2_g;

 __u16 ccm_coeff0_b;
 __u16 ccm_coeff1_b;

 __u16 ccm_coeff2_b;

 __u16 wt0;
 __u16 wt1;

 __u16 wt2;

 __u16 mat0_x;
 __u16 mat0_y;

 __u16 mat1_x;
 __u16 mat1_y;

 __u16 mat2_x;
 __u16 mat2_y;

 __u16 nor_x0_0;
 __u16 nor_x1_0;

 __u16 nor_y0_0;
 __u16 nor_y1_0;

 __u16 big_x0_0;
 __u16 big_x1_0;

 __u16 big_y0_0;
 __u16 big_y1_0;

 __u16 nor_x0_1;
 __u16 nor_x1_1;

 __u16 nor_y0_1;
 __u16 nor_y1_1;

 __u16 big_x0_1;
 __u16 big_x1_1;

 __u16 big_y0_1;
 __u16 big_y1_1;

 __u16 nor_x0_2;
 __u16 nor_x1_2;

 __u16 nor_y0_2;
 __u16 nor_y1_2;

 __u16 big_x0_2;
 __u16 big_x1_2;

 __u16 big_y0_2;
 __u16 big_y1_2;

 __u16 nor_x0_3;
 __u16 nor_x1_3;

 __u16 nor_y0_3;
 __u16 nor_y1_3;

 __u16 big_x0_3;
 __u16 big_x1_3;

 __u16 big_y0_3;
 __u16 big_y1_3;

 __u8 exc_wp_region0_excen;
 __u8 exc_wp_region0_measen;
 __u8 exc_wp_region0_domain;
 __u8 exc_wp_region1_excen;
 __u8 exc_wp_region1_measen;
 __u8 exc_wp_region1_domain;
 __u8 exc_wp_region2_excen;
 __u8 exc_wp_region2_measen;
 __u8 exc_wp_region2_domain;
 __u8 exc_wp_region3_excen;
 __u8 exc_wp_region3_measen;
 __u8 exc_wp_region3_domain;
 __u8 exc_wp_region4_excen;
 __u8 exc_wp_region4_domain;
 __u8 exc_wp_region5_excen;
 __u8 exc_wp_region5_domain;
 __u8 exc_wp_region6_excen;
 __u8 exc_wp_region6_domain;
 __u8 multiwindow_en;

 __u16 multiwindow0_h_offs;
 __u16 multiwindow0_v_offs;

 __u16 multiwindow0_h_size;
 __u16 multiwindow0_v_size;

 __u16 multiwindow1_h_offs;
 __u16 multiwindow1_v_offs;

 __u16 multiwindow1_h_size;
 __u16 multiwindow1_v_size;

 __u16 multiwindow2_h_offs;
 __u16 multiwindow2_v_offs;

 __u16 multiwindow2_h_size;
 __u16 multiwindow2_v_size;

 __u16 multiwindow3_h_offs;
 __u16 multiwindow3_v_offs;

 __u16 multiwindow3_h_size;
 __u16 multiwindow3_v_size;

 __u16 exc_wp_region0_xu0;
 __u16 exc_wp_region0_xu1;

 __u16 exc_wp_region0_yv0;
 __u16 exc_wp_region0_yv1;

 __u16 exc_wp_region1_xu0;
 __u16 exc_wp_region1_xu1;

 __u16 exc_wp_region1_yv0;
 __u16 exc_wp_region1_yv1;

 __u16 exc_wp_region2_xu0;
 __u16 exc_wp_region2_xu1;

 __u16 exc_wp_region2_yv0;
 __u16 exc_wp_region2_yv1;

 __u16 exc_wp_region3_xu0;
 __u16 exc_wp_region3_xu1;

 __u16 exc_wp_region3_yv0;
 __u16 exc_wp_region3_yv1;

 __u16 exc_wp_region4_xu0;
 __u16 exc_wp_region4_xu1;

 __u16 exc_wp_region4_yv0;
 __u16 exc_wp_region4_yv1;

 __u16 exc_wp_region5_xu0;
 __u16 exc_wp_region5_xu1;

 __u16 exc_wp_region5_yv0;
 __u16 exc_wp_region5_yv1;

 __u16 exc_wp_region6_xu0;
 __u16 exc_wp_region6_xu1;

 __u16 exc_wp_region6_yv0;
 __u16 exc_wp_region6_yv1;

 __u8 exc_wp_region0_weight;
 __u8 exc_wp_region1_weight;
 __u8 exc_wp_region2_weight;
 __u8 exc_wp_region3_weight;

 __u8 exc_wp_region4_weight;
 __u8 exc_wp_region5_weight;
 __u8 exc_wp_region6_weight;

 __u8 wp_blk_wei_w[225];

 struct isp2x_bls_fixed_val bls2_val;
} __attribute__ ((packed));

struct isp35_aiawb_meas_cfg {
 __u8 bls3_en;

 __u8 ds_mode_config_en;
 __u8 ds_mode;
 __u8 rgb2w_mode;
 __u8 rawout_sel;
 __u8 path_sel;
 __u8 in_shift;

 __u8 exp1_check_en;
 __u8 exp_thr;
 __u16 saturation_hthr;
 __u16 saturation_lthr;

 __u16 h_offs;
 __u16 v_offs;

 __u16 h_size;
 __u16 v_size;

 __s8 flt_coe[5];

 __u16 wbgain_inv_g;
 __u16 wbgain_inv_b;

 __u16 wbgain_inv_r;
 __u16 expand;

 __u16 ms00;
 __u16 ms01;

 __u16 mr00;
 __u16 mr01;

 __u16 mr10;
 __u16 mr11;

 struct isp2x_bls_fixed_val bls3_val;
} __attribute__ ((packed));

struct isp35_awbsync_meas_cfg {

 __u8 sumval_check_en;
 __u8 sumval_mode;

 __u16 scl_b;
 __u16 scl_g;
 __u16 scl_r;

 __u16 sumval_minb;
 __u16 sumval_ming;
 __u16 sumval_minr;

 __u16 sumval_maxb;
 __u16 sumval_maxg;
 __u16 sumval_maxr;

 __u16 win0_h_offs;
 __u16 win0_v_offs;

 __u16 win0_r_coor;
 __u16 win0_d_coor;

 __u16 win1_h_offs;
 __u16 win1_v_offs;

 __u16 win1_r_coor;
 __u16 win1_d_coor;

 __u16 win2_h_offs;
 __u16 win2_v_offs;

 __u16 win2_r_coor;
 __u16 win2_d_coor;
} __attribute__ ((packed));

struct isp35_rawaf_meas_cfg {
 __u8 rawaf_sel;
 __u8 num_afm_win;
 __u8 bnr2af_sel;


 __u8 gamma_en;
 __u8 gaus_en;
 __u8 hiir_en;
 __u8 viir_en;
 __u8 ldg_en;
 __u8 h1_fv_mode;
 __u8 h2_fv_mode;
 __u8 v1_fv_mode;
 __u8 v2_fv_mode;
 __u8 ae_mode;
 __u8 y_mode;
 __u8 vldg_sel;
 __u8 v_dnscl_mode;
 __u8 from_ynr;
 __u8 bnr_be_sel;
 __u8 hiir_left_border_mode;
 __u8 avg_ds_en;
 __u8 avg_ds_mode;
 __u8 h1_acc_mode;
 __u8 h2_acc_mode;
 __u8 v1_acc_mode;
 __u8 v2_acc_mode;


 struct isp2x_window win[2];


 __s16 bls_offset;
 __u8 bls_en;
 __u8 aehgl_en;
 __u8 hldg_dilate_num;
 __u8 tnrin_shift;


 __u8 h1iir_shift_wina;
 __u8 h2iir_shift_wina;
 __u8 v1iir_shift_wina;
 __u8 v2iir_shift_wina;
 __u8 h1iir_shift_winb;
 __u8 h2iir_shift_winb;
 __u8 v1iir_shift_winb;
 __u8 v2iir_shift_winb;


 __s8 gaus_coe[9];


 __u16 gamma_y[17];

 __u16 h_fv_thresh;
 __u16 v_fv_thresh;
 struct isp3x_rawaf_curve curve_h[2];
 struct isp3x_rawaf_curve curve_v[2];
 __s16 h1iir1_coe[6];
 __s16 h1iir2_coe[6];
 __s16 h2iir1_coe[6];
 __s16 h2iir2_coe[6];
 __s16 v1iir_coe[3];
 __s16 v2iir_coe[3];
 __s16 v1fir_coe[3];
 __s16 v2fir_coe[3];

 __u16 highlit_thresh;


 __u16 h_fv_limit;
 __u16 h_fv_slope;

 __u16 v_fv_limit;
 __u16 v_fv_slope;
} __attribute__ ((packed));

struct isp35_rawae_meas_cfg {
 __u8 rawae_sel;
 __u8 bnr2ae_sel;

 __u8 wnd_num;
 __u8 wnd1_en;
 __u8 debug_en;
 __u8 bnr_be_sel;

 __u16 win0_h_offset;
 __u16 win0_v_offset;
 __u16 win0_h_size;
 __u16 win0_v_size;
 __u16 win1_h_offset;
 __u16 win1_v_offset;
 __u16 win1_h_size;
 __u16 win1_v_size;
} __attribute__ ((packed));

struct isp35_rawhist_meas_cfg {
 __u8 stepsize;
 __u8 debug_en;
 __u8 mode;
 __u8 data_sel;
 __u8 wnd_num;
 __u16 waterline;

 __u8 rcc;
 __u8 gcc;
 __u8 bcc;
 __u8 off;

 __u16 h_offset;
 __u16 v_offset;
 __u16 h_size;
 __u16 v_size;

 __u8 weight[225];
} __attribute__ ((packed));

struct isp35_isp_other_cfg {
 struct isp39_rgbir_cfg rgbir_cfg;
 struct isp35_bls_cfg bls_cfg;
 struct isp32_awb_gain_cfg awb_gain_cfg;
 struct isp39_dpcc_cfg dpcc_cfg;
 struct isp35_hdrmge_cfg hdrmge_cfg;
 struct isp3x_gain_cfg gain_cfg;
 struct isp35_bay3d_cfg bay3d_cfg;
 struct isp35_bay3d_cfg bay3d_l2_cfg;
 struct isp35_ai_cfg ai_cfg;

 struct isp33_cac_cfg cac_cfg;
 struct isp3x_lsc_cfg lsc_cfg;

 struct isp35_debayer_cfg debayer_cfg;
 struct isp35_drc_cfg drc_cfg;
 struct isp33_ccm_cfg ccm_cfg;
 struct isp3x_gammaout_cfg gammaout_cfg;
 struct isp35_hsv_cfg hsv_cfg;
 struct isp21_csm_cfg csm_cfg;
 struct isp33_gic_cfg gic_cfg;
 struct isp35_cnr_cfg cnr_cfg;
 struct isp35_ynr_cfg ynr_cfg;
 struct isp35_sharp_cfg sharp_cfg;
 struct isp35_enh_cfg enh_cfg;
 struct isp33_hist_cfg hist_cfg;
 struct isp32_ldch_cfg ldch_cfg;
 struct isp21_cgc_cfg cgc_cfg;
 struct isp2x_cproc_cfg cproc_cfg;
} __attribute__ ((packed));

struct isp35_isp_meas_cfg {
 struct isp35_rawae_meas_cfg rawae0;
 struct isp35_rawhist_meas_cfg rawhist0;
 struct isp35_rawae_meas_cfg rawae3;
 struct isp35_rawhist_meas_cfg rawhist3;
 struct isp35_rawawb_meas_cfg rawawb;
 struct isp35_rawaf_meas_cfg rawaf;
 struct isp35_aiawb_meas_cfg aiawb;
 struct isp35_awbsync_meas_cfg awbsync;
} __attribute__ ((packed));

struct isp35_isp_params_cfg {
 __u64 module_en_update;
 __u64 module_ens;
 __u64 module_cfg_update;

 __u32 frame_id;
 struct isp35_isp_meas_cfg meas;
 struct isp35_isp_other_cfg others;
 struct sensor_exposure_cfg exposure;
} __attribute__ ((packed));

struct isp35_awbsync_stat {
 __u64 sumr[3];
 __u64 sumg[3];
 __u64 sumb[3];
 __u64 sump[3];
} __attribute__ ((packed));

struct isp35_enh_stat {
 __u8 iir[24][32];
} __attribute__ ((packed));

struct isp35_stat {

 struct isp33_rawae_stat rawae3;
 struct isp33_rawhist_stat rawhist3;
 struct isp33_rawae_stat rawae0;
 struct isp33_rawhist_stat rawhist0;
 struct isp39_rawaf_stat rawaf;
 struct isp33_rawawb_stat rawawb;

 struct isp33_bay3d_stat bay3d;
 struct isp33_sharp_stat sharp;
 struct isp35_enh_stat enh;
 struct isp33_hist_stat hist;
 struct isp35_awbsync_stat awbsync;
 struct isp32_info2ddr_stat info2ddr;

 int buf_aiawb_index;
 int buf_bay3d_iir_index;
 int buf_bay3d_ds_index;
 int buf_bay3d_wgt_index;
 int buf_gain_index;
 int buf_aipre_gain_index;
 int buf_vpsl_index;
} __attribute__ ((packed));

struct rkisp35_stat_buffer {
 struct isp35_stat stat;
 __u32 meas_type;
 __u32 frame_id;
 __u32 params_id;
} __attribute__ ((packed));
