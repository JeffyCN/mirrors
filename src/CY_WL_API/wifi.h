#ifndef __WIFI_H__
#define __WIFI_H__

#include <syslog.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <sys/time.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <paths.h>
#include <sys/wait.h>
#include <linux/sockios.h>
#include <net/route.h>

#ifdef SYSLOG_DEBUG
#define pr_debug(fmt, ...)              syslog(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define pr_info(fmt, ...)               syslog(LOG_INFO, fmt, ##__VA_ARGS__)
#define pr_warning(fmt, ...)			syslog(LOG_WARNING, fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...)                syslog(LOG_ERR, fmt, ##__VA_ARGS__)
#else
#define pr_debug        printf
#define pr_info printf
#define pr_warning printf
#define pr_err printf
#endif

#define DEFAULT_IFNAME					"wlan0"
#define JOIN_AP_RETRIES					1
#define MAX_SAME_AP_NUM					5
#define MAX_SCAN_NUM					10
#define TCP_CONNECT_RETRIES				3

#define TCPKA_INTERVAL					60
#define TCPKA_RETRY_INTERVAL			4
#define TCPKA_RETRY_COUNT				15
#define ETHERNET_HEADER_LEN				14
#define IPV4_HEADER_FIXED_LEN			20
#define TCP_HEADER_FIXED_LEN			20
#define TCP_OPTIONS_LEN					12
#define WOWL_PATTERN_MATCH_OFFSET      \
                       (ETHERNET_HEADER_LEN + IPV4_HEADER_FIXED_LEN + TCP_HEADER_FIXED_LEN + TCP_OPTIONS_LEN)
#define CUSTOM_LISTEN_INTERVAL			1000

#define SOL_TCP							6
#define TCP_EXT_INFO					37

struct tcp_ext_info {
       uint16_t ip_id;
       uint16_t dummy;
       uint32_t snd_nxt;
       uint32_t rcv_nxt;
       uint32_t window;
       uint32_t tsval;
       uint32_t tsecr;
};

extern const char tcpka_payload[];
extern const char wowl_pattern[];

int WIFI_Suspend(int sock, bool is_connected);
int WIFI_Resume(void);

int rk_build_tcpka_param(wl_tcpka_param_t* param, char* ifname, int sock);
void rk_system_return(const char cmdline[], char recv_buff[], int len);
int rk_system(const char *cmd);
int rk_obtain_ip_from_vendor(char * ifname);
int rk_obtain_ip_from_udhcpc(char* ifname);


typedef struct
{
	uint32_t ssid_len;
    char ssid[32];
    uint8_t bssid[6];
    uint32_t security;
    uint16_t channel;
    int16_t rssi;
} wifi_ap_info_t;

/*
 * 功能   : wifi模块初始化.
 * 参数   : 无
 * 返回值 : 无
 */
int WIFI_Init(void);

/*
 * 功能   : wifi模块退出.
 * 参数   : 无
 * 返回值 : 无
 */
void WIFI_Deinit(void);

/*
 * 功能   : WIFI断开.
 * 参数   : 无
 * 返回值 : 无
 */
void WIFI_Disconnect(void);

/*
 * 功能   : 连接指定网络[静态/动态/OPEN].
 * 参数   :
 *          ssid : 目标SSID.
            pass : 密码.
			useip: 0/静态  1/动态  2/OPEN
 * 返回值 : 无
 */
int WIFI_Connect(char *ssid, char *pass, int  useip);

/*
 * 功能   : 获取网络连接状态.
 * 参数   : 无
 * 返回值 : 无
 */
int WIFI_GetStatus(void);

/*
 * 功能   : 获取WIFI连接信道.
 * 参数   : 无
 * 返回值 : [0-13]/WIFI连接信道.
 */
int WIFI_GetChannel(void);

/*
 * 功能   : 获取WIFI唤醒方式.
 * 参数   : 无
 * 返回值 : WIFI启动原因码.
 */
int WIFI_GetWakupReason(void);
/*
 * 功能   : 获取WIFI信号强度.
 * 参数   : 无
 * 返回值 : WIFI信号强度.
 */
int WIFI_GetSignal(void);

/*
 * 功能   : 设置WIFI静态IP信息.
 * 参数   :
 			ipAddr      :    IP地址信息.
			netmask     :    子网掩码.
			gateway     :    网关.

 * 返回值 :   0/成功
 *          非0/失败
 */
int WIFI_SetNetInfo(char *ipAddr, char *netmask, char *gateway, char *dns);

/*
 * 功能   : 获取WIFI启动方式.
 * 参数   : 无
 * 返回值 : 0/重启  1/PIR触发唤醒  2/WIFI唤醒  3/其他
 */
int WIFI_StartUpMode(void);

/*
 * 功能   : 网关离线配置SSID唤醒.
 * 参数   :
            ssid         : 目标SSID.
			channel_num  : 扫描通道数,0/全信道扫描 [1-12]/单信道扫描.
			interval     : 扫描周期.
 * 返回值 :   0/成功
 *          非0/失败
 */
int WIFI_SetWakeupBySsid(char *ssid, char channelNum, unsigned short interval);
/*
 * 功能   : 网关上线后清除SSID唤醒.
 * 参数   : 无
 * 返回值 :   0/成功
 *          非0/失败
 */
int WIFI_ClearWakeupSsid(void);

/*
 * 功能   : 设置WIFI启动方式.
 * 参数   :
 *            WakeupFlag : 0/慢启动
 *						   1/快启动
 * 返回值 : 无
 */
int WIFI_SetQuickStartWay(int WakeupFlag);

/*
 * 功能   : 扫描周边网络.
 * 参数   : 无
 * 返回值 : 无
 */
int WIFI_ClientScan(void);

/*
 * 功能   : 获取扫描结果.
 * 参数   :
 *	 		   pstResults : 扫描结果.
 *             num        : AP总数.
 * 返回值 :   0/成功
 *          非0/失败
 */
int WIFI_GetClientScanResults(wifi_ap_info_t *pstResults, uint32_t num);

/*
 * 功能   : 进入休眠保活状态.
 * 参数   : 无
 * 返回值 :   0/成功
 *          非0/失败
 */

#endif /* __WIFI_H__ */
