#include "wifi.h"

#define SCAN_DWELL_TIME_PER_CHANNEL	(200)
#define DEBUG 0

const char tcpka_payload[] = "helloworld";
const char wowl_pattern[] = "123";

static char gssid[64] = { 0 };
static char gpsk[64] = { 0 };

static int rk_system_fd_closexec(const char* command)
{
	int wait_val = 0;
	pid_t pid = -1; 

	if (!command)
		return 1;

	if ((pid = vfork()) < 0)
		return -1;

	if (pid == 0) {
		int i = 0;
		int stdin_fd = fileno(stdin);
		int stdout_fd = fileno(stdout);
		int stderr_fd = fileno(stderr);
		long sc_open_max = sysconf(_SC_OPEN_MAX);

		if (sc_open_max < 0) {
			sc_open_max = 20000; /* enough? */
		}

		/* close all descriptors in child sysconf(_SC_OPEN_MAX) */
		for (i = 0; i < sc_open_max; i++) {
			if (i == stdin_fd || i == stdout_fd || i == stderr_fd)
				continue;
			close(i);
		}

		execl(_PATH_BSHELL, "sh", "-c", command, (char*)0);
		_exit(127);
	}

	while (waitpid(pid, &wait_val, 0) < 0) {
		if (errno != EINTR) {
			wait_val = -1;
			break;
		}
	}

	return wait_val;
}

int rk_system(const char *cmd)
{
	pid_t status;

	status = rk_system_fd_closexec(cmd);

	if (-1 == status) {
		pr_err("[system_exec_err] -1\n");
		return -1;
	} else {
		if (WIFEXITED(status)) {
			if (0 == WEXITSTATUS(status)) {
				return 0;
			} else {
				pr_err("[system_exec_err] -2\n");
				return -2;
			}
		} else {
			pr_err("[system_exec_err] -3\n");
			return -3;
		}
	}

	return 0;
}

void rk_system_return(const char cmdline[], char recv_buff[], int len)
{
	if (DEBUG)
		pr_info("[BT_DEBUG] execute: %s\n", cmdline);

	FILE *stream = NULL;
	char *tmp_buff = recv_buff;

	memset(recv_buff, 0, len);

	if ((stream = popen(cmdline, "r")) != NULL) {
		while (fgets(tmp_buff, len, stream)) {
			tmp_buff += strlen(tmp_buff);
			len -= strlen(tmp_buff);
			if (len <= 1)
				break;
		}

		if (DEBUG)
			pr_info("[BT_DEBUG] execute_r: %s \n", recv_buff);
		pclose(stream);
	} else
		pr_err("[popen] error: %s\n", cmdline);
}

int rk_obtain_ip_from_vendor(char * ifname)
{
	char cmd_results1[256];
	char cmd_results2[256];
	char cmd_results3[256];
	char cmd_results4[256];
	char cmd[128];

	pr_info("get dhcp info from vendor\n");

	rk_system_return("vendor_storage -r VENDOR_CUSTOM_ID_1E -t string |  sed -n '4p' | awk '{print $3}' | awk -F, '{print $4}'",
					 cmd_results1, 256);
	if (cmd_results1[strlen(cmd_results1) - 1] == '\n')
		cmd_results1[strlen(cmd_results1) - 1] = 0;
	pr_info("%s: cmd_results1: %s\n", __FUNCTION__, cmd_results1);

	rk_system_return("vendor_storage -r VENDOR_CUSTOM_ID_1E -t string |  sed -n '4p' | awk '{print $3}' | awk -F, '{print $5}'",
					 cmd_results2, 256);
	if (cmd_results2[strlen(cmd_results2) - 1] == '\n')
		cmd_results2[strlen(cmd_results2) - 1] = 0;
	pr_info("%s: cmd_results2: %s\n", __FUNCTION__, cmd_results2);

	rk_system_return("vendor_storage -r VENDOR_CUSTOM_ID_1E -t string |  sed -n '4p' | awk '{print $3}' | awk -F, '{print $6}'",
					 cmd_results3, 256);
	if (cmd_results3[strlen(cmd_results3) - 1] == '\n')
		cmd_results3[strlen(cmd_results3) - 1] = 0;
	pr_info("%s: cmd_results3: %s\n", __FUNCTION__, cmd_results3);

	rk_system_return("vendor_storage -r VENDOR_CUSTOM_ID_1E -t string |  sed -n '4p' | awk '{print $3}' | awk -F, '{print $7}'",
					 cmd_results4, 256);
	if (cmd_results4[strlen(cmd_results4) - 1] == '\n')
		cmd_results4[strlen(cmd_results4) - 1] = 0;
	pr_info("%s: cmd_results4: %s\n", __FUNCTION__, cmd_results4);

	memset(cmd, 0, 128);
	sprintf(cmd, "ifconfig wlan0 %s netmask %s", cmd_results1, cmd_results2);
	pr_info("ifconfig: %s\n", cmd);
	rk_system(cmd);

	memset(cmd, 0, 128);
	sprintf(cmd, "route add default gw %s", cmd_results3);
	pr_info("route : %s\n", cmd);
	rk_system(cmd);

	memset(cmd, 0, 128);
	sprintf(cmd, "echo \"nameserver %s\" > /etc/resolv.conf", cmd_results4);
	pr_info("nameserver : %s\n", cmd);
	rk_system(cmd);

	return 0;
}

static int rk_get_local_ip(const char * ifname, char *local_ip)
{
	int ret;
 	int sock;
	struct ifreq ifr;

	pr_info("%s, enter\n", __FUNCTION__);

	sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
	{
		pr_info("%s, create socket err = %d\n", __FUNCTION__, sock);
		return sock;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

	ret = ioctl(sock, SIOCGIFADDR, &ifr);

    if (ret == 0)
	{
		sprintf(local_ip, "%s", inet_ntoa(((struct sockaddr_in*)(&ifr.ifr_addr))->sin_addr));
    }

	if (sock > 0)
	{
		close(sock);
	}

    return ret;
}

static void rk_setnetinfo_to_vendor(void)
{
	char cmd[256];
	char cmd_results1[256];
	char cmd_results2[256];
	char cmd_results3[256];
	char cmd_results4[256];

	pr_info("%s, enter\n", __FUNCTION__);

	rk_system_return("ifconfig wlan0 | grep inet | awk '{print $2}' | awk -F: '{print $2}'",
					 cmd_results1, 256);
	if (cmd_results1[strlen(cmd_results1) - 1] == '\n')
		cmd_results1[strlen(cmd_results1) - 1] = 0;
	pr_info("%s: cmd_results1: %s\n", __FUNCTION__, cmd_results1);

	rk_system_return("ifconfig wlan0 | grep inet | awk '{print $4}' | awk -F: '{print $2}'",
					 cmd_results2, 256);
	if (cmd_results2[strlen(cmd_results2) - 1] == '\n')
		cmd_results2[strlen(cmd_results2) - 1] = 0;
	pr_info("%s: cmd_results2: %s\n", __FUNCTION__, cmd_results2);

	rk_system_return("route -n | awk '{print $2}' | sed -n '3p'",
					 cmd_results3, 256);
	if (cmd_results3[strlen(cmd_results3) - 1] == '\n')
		cmd_results3[strlen(cmd_results3) - 1] = 0;
	pr_info("%s: cmd_results3: %s\n", __FUNCTION__, cmd_results3);

	rk_system_return("cat /etc/resolv.conf | grep nameserver | grep wlan0 | awk '{print $2}'",
					 cmd_results4, 256);
	if (cmd_results4[strlen(cmd_results4) - 1] == '\n')
		cmd_results4[strlen(cmd_results4) - 1] = 0;
	pr_info("%s: cmd_results4: %s\n", __FUNCTION__, cmd_results4);

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "vendor_storage -w VENDOR_CUSTOM_ID_1E -t string -i 1,%s,%s,%s,%s,%s,%s",
			gssid, gpsk, cmd_results1, cmd_results2, cmd_results3, cmd_results4);
	pr_info("CMD: %s\n", cmd);

	rk_system(cmd);
}

int rk_obtain_ip_from_udhcpc(char* ifname)
{
	int ret;
	int i = 0;
	char cmd[256];

	pr_info("%s, enter\n", __FUNCTION__);

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, " udhcpc -i %s -t 15 -q", ifname);
	pr_info("CMD: %s\n", cmd);

	ret = system(cmd);
	pr_info("%s, ret = %d\n", __FUNCTION__, ret);

	while (i < 50)
	{
		memset(cmd, 0, sizeof(cmd));
		ret = rk_get_local_ip(ifname, cmd);

		if (ret < 0)
		{
			i++;
			sleep(1);
		}
		else
		{
			pr_info("%s, ip = %s\n", __FUNCTION__, cmd);
			rk_setnetinfo_to_vendor();
			break;
		}
	}

	return ret;
}

int rk_build_tcpka_param(
				wl_tcpka_param_t* param,
				char* ifname,
				int sock)
{
	int ret;
	struct sockaddr_in localaddr;
	struct sockaddr_in peeraddr;
    struct arpreq req;
	struct tcp_ext_info tcp_info;
	socklen_t len;

	pr_info("%s, enter\n", __FUNCTION__);

	memset(param, 0, sizeof(wl_tcpka_param_t));
	memset(&tcp_info, 0, sizeof(struct tcp_ext_info));

	param->interval = TCPKA_INTERVAL;
	param->retry_interval = TCPKA_RETRY_INTERVAL;
	param->retry_count = TCPKA_RETRY_COUNT;

	pr_info("%s, interval		: %d\n", __FUNCTION__, param->interval);
	pr_info("%s, retry_interval	: %d\n", __FUNCTION__, param->retry_interval);
	pr_info("%s, retry_count		: %d\n", __FUNCTION__, param->retry_count);

	len = sizeof(struct sockaddr_in);
	ret = getsockname(sock, (struct sockaddr*)&localaddr, &len);
	if (ret < 0)
	{
        pr_info("%s, getsockname err = %d\n", __FUNCTION__, ret);
		return ret;
	}
    memcpy(&param->src_ip, &localaddr.sin_addr, sizeof(wl_ipv4_addr_t));
    param->srcport = ntohs(localaddr.sin_port);

	pr_info("%s, src_ip	: %d.%d.%d.%d\n", __FUNCTION__, 
		param->src_ip.addr[0], 
		param->src_ip.addr[1],
		param->src_ip.addr[2],
		param->src_ip.addr[3]);
	pr_info("%s, srcport	: 0x%04x\n", __FUNCTION__, param->srcport);

	len = sizeof(struct sockaddr_in);
	ret = getpeername(sock, (struct sockaddr*)&peeraddr, &len);
	if (ret < 0)
	{
        pr_info("%s, getsockname err = %d\n", __FUNCTION__, ret);
		return ret;
	}
    memcpy(&param->dst_ip, &peeraddr.sin_addr, sizeof(wl_ipv4_addr_t));
    param->dstport = ntohs(peeraddr.sin_port);

	pr_info("%s, dst_ip	: %d.%d.%d.%d\n", __FUNCTION__, 
		param->dst_ip.addr[0], 
		param->dst_ip.addr[1],
		param->dst_ip.addr[2],
		param->dst_ip.addr[3]);
	pr_info("%s, dstport	: 0x%04x\n", __FUNCTION__, param->dstport);

    memset(&req, 0, sizeof(struct arpreq));  
    memcpy(&req.arp_pa, &peeraddr, sizeof(peeraddr));  
    strcpy(req.arp_dev, ifname);  
    req.arp_pa.sa_family = AF_INET;  
    req.arp_ha.sa_family = AF_UNSPEC;  
	ret = ioctl(sock, SIOCGARP, &req);
	if (ret < 0)
	{
        pr_info("%s, ioctl, SIOCGARP err = %d\n", __FUNCTION__, ret);
		return ret;
	}
	memcpy(&param->dst_mac, req.arp_ha.sa_data, sizeof(wl_ether_addr_t));

	pr_info("%s, dst_mac	: %02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__, 
		param->dst_mac.octet[0], 
		param->dst_mac.octet[1], 
		param->dst_mac.octet[2], 
		param->dst_mac.octet[3], 
		param->dst_mac.octet[4], 
		param->dst_mac.octet[5]);

	len = sizeof(tcp_info);
	ret = getsockopt(sock, SOL_TCP, TCP_EXT_INFO, &tcp_info, &len);
	if (ret < 0)
	{
        pr_info("%s, getsockopt err = %d\n", __FUNCTION__, ret);
		return ret;
	}

    param->ipid = tcp_info.ip_id;
    param->seq = tcp_info.snd_nxt;
    param->ack = tcp_info.rcv_nxt;
    param->tcpwin = tcp_info.window;
    param->tsval = tcp_info.tsval;
    param->tsecr = tcp_info.tsecr;

	pr_info("%s, ipid	: 0x%04x\n", __FUNCTION__, param->ipid);
	pr_info("%s, seq		: 0x%08x\n", __FUNCTION__, param->seq);
	pr_info("%s, ack		: 0x%08x\n", __FUNCTION__, param->ack);
	pr_info("%s, tcpwin	: 0x%08x\n", __FUNCTION__, param->tcpwin);
	pr_info("%s, tsval	: 0x%08x\n", __FUNCTION__, param->tsval);
	pr_info("%s, tsecr	: 0x%08x\n", __FUNCTION__, param->tsecr);

	param->payload_len = strlen(tcpka_payload);
	memcpy(param->payload, tcpka_payload, param->payload_len);

	return ret;
}

int WIFI_Init(void)
{
	int ret = 0;
	int retry = 20;

	/* insmod Wi-Fi Module */
	ret = system("insmod /vendor/lib/modules/cywdhd.ko");
	pr_info("%s, insmod ret = %d\n", __FUNCTION__, ret);

	while (retry--) {
		usleep(200 * 1000);
		ret = system("ifconfig wlan0 up");
		pr_info("%s, up ret = %d\n", __FUNCTION__, ret);
		if (ret == 0)
			break;
	}

	ret = wl_init(DEFAULT_IFNAME);
	pr_info("%s, wl_init ret = %d\n", __FUNCTION__, ret);

	return ret;
}

void WIFI_Deinit(void)
{
	/* rmmod Wi-Fi Module */
	wl_deinit();
}

void WIFI_Disconnect(void)
{
	wl_disassoc();
}

int WIFI_Connect(char* ssid_name, char* password, int useip)
{
	int ret;
	wl_ssid_t ssid;
	uint32_t i;
	uint32_t j;
	uint32_t ap_security_list[MAX_SAME_AP_NUM];
	uint32_t ap_count = 0;

	memset(&gssid, 0, 64);
	memset(&gpsk, 0, 64);
	memcpy(gssid, ssid_name, strlen(ssid_name));
	memcpy(gpsk, password, strlen(password));

	memset(&ssid, 0, sizeof(ssid));
	ssid.len = strlen(ssid_name);
	memcpy(ssid.value, ssid_name, ssid.len);

	pr_info("%s, enter\n", __FUNCTION__);

	if (password == NULL)
	{
		pr_info("Open security type\n");
		ap_security_list[0] = WL_SECURITY_OPEN;
		ap_count = 1;
	}
	else
	{
		pr_info("Scan for the security type\n");
		
		ret = wl_scan(&ssid, NULL, NULL, 0, WL_SCAN_PER_CH_TIME_DEFAULT);

		pr_info("wl_scan result = %d\n", ret);
		if (ret < 0)
		{
			return 0;
		}

		while (1)
		{
			wl_ap_info_t ap_info[MAX_SCAN_NUM];
			uint32_t count = MAX_SCAN_NUM;
			bool is_completed = false;

			usleep(20000);

			ret = wl_get_scan_results(ap_info, &count, &is_completed);

			if (ret < 0)
			{
				pr_info("wl_get_scan_results failed, result = %d\n", ret);
				break;
			}
			else
			{
				for (i = 0; i < count; i++)
				{
					if (ap_count < MAX_SCAN_NUM)
					{
						pr_info("get AP %d security = %d\n", ap_count, ap_info[i].security);
						ap_security_list[ap_count] = ap_info[i].security;
						ap_count++;
					}
				}

				if (is_completed)
				{
					pr_info("Scan Done\n");
					break;
				}
			}
		}
	}

	pr_info("Starting to join AP: %s ..., password: %s\n", ssid_name, password);

	/* Try to join each of the found AP until successfully */
	for (i = 0; i < ap_count; i++)
	{
		uint32_t security = ap_security_list[i];

		/* Optional: Using higher security */
		switch (security)
		{
			case WL_SECURITY_WPA_MIXED_PSK:
				security = WL_SECURITY_WPA_AES_PSK;
				break;

			case WL_SECURITY_WPA2_MIXED_PSK:
				security = WL_SECURITY_WPA2_AES_PSK;
				break;

			default:
				break;
		}				

		for (j = 0; j < JOIN_AP_RETRIES; i++)
		{
			uint8_t key_length = 0;

			if (password != NULL)
			{
				key_length = strlen(password);
			}

			ret = wl_join_ap(
						&ssid,
						security,
						(int8_t*)password,
						key_length);

			if (ret < 0)
			{
				pr_info("Try to join AP %d %d times  failed, ret = %d\n", i, j, ret);
			}
			else
			{
				break;
			}
		}

		if (ret == WL_E_OK)
		{
			break;
		}
	}

	if (ret == WL_E_OK)
	{
		pr_info("Joined AP succcessfully\n");
	}

	return ret;
}

int WIFI_GetStatus(void)
{
	return wl_is_associated();
}

int WIFI_GetChannel(void)
{
	uint32_t channel = 0;

	wl_get_channel(&channel);

	return channel;
}

int WIFI_GetWakupReason(void)
{
	uint32_t reason = 0;

	wl_wowl_get_wakeind(&reason);

	return reason;
}

int WIFI_GetSignal(void)
{
	int32_t rssi = 0;

	wl_get_rssi(&rssi);

	return rssi;
}

int WIFI_SetNetInfo(char *ipAddr, char *netmask, char *gateway, char *dns)
{
	char cmd[128];

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "ifconfig wlan0 %s netmask %s", ipAddr, netmask);
	pr_info("ifconfig: %s\n", cmd);
	rk_system(cmd);

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "route add default gw %s", gateway);
	pr_info("route : %s\n", cmd);
	rk_system(cmd);

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "echo \"nameserver %s\" > /etc/resolv.conf", dns);
	pr_info("nameserver : %s\n", cmd);
	rk_system(cmd);

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "vendor_storage -w VENDOR_CUSTOM_ID_1E -t string -i 1,%s,%s,%s,%s,%s,%s",
			gssid, gpsk, ipAddr, netmask, gateway, dns);
	pr_info("CMD: %s\n", cmd);

	rk_system(cmd);

	return 0;
}

int WIFI_StartUpMode(void)
{
	return 0;
}

int WIFI_SetWakeupBySsid(char *ssid, char channelNum, unsigned short interval)
{
	return 0;
}

int WIFI_ClearWakeupSsid(void)
{
	return 0;
}

int WIFI_SetQuickStartWay(int WakeupFlag)
{
	return 0;
}

int WIFI_ClientScan(char *ssid)
{
	wl_ssid_t ssid_name, *pssid = NULL;

	if (strncmp(ssid, "all", 3)) {
		ssid_name.len = strlen(ssid);
		memcpy(ssid_name.value, ssid, ssid_name.len);
		pssid = &ssid_name;
	}

	return wl_scan(pssid, NULL, NULL, 0, SCAN_DWELL_TIME_PER_CHANNEL);
}

int WIFI_GetClientScanResults(wifi_ap_info_t *pstResults, int num)
{
	int ret;
	bool is_completed;
	wl_ap_info_t *ap_info;

	ap_info = malloc(sizeof(wl_ap_info_t) * num);

	if (ap_info == NULL)
	{
		return -1;
	}

	ret = wl_get_scan_results(ap_info, &num, &is_completed);

	for (int i = 0; i < num; i++) {
		pr_info("ssid: %15s\t channel: %d\t rssi: %d\n", ap_info[i].ssid.value, ap_info[i].channel, ap_info[i].rssi);
	}

	return ret;
}

int WIFI_Suspend(int sock)
{
	int ret;

#if (1)
	wl_tcpka_param_t tcpka_param;
    uint32_t wowl_caps;

	pr_info("%s, enter\n", __FUNCTION__);

	ret = rk_build_tcpka_param(&tcpka_param, DEFAULT_IFNAME, sock);
    if (ret < 0)
	{
        pr_info("%s, build_tcpka_param, err = %d\n", __FUNCTION__, ret);
		return ret;
    }

	ret = wl_tcpka_conn_enable(&tcpka_param);
    if (ret < 0)
	{
        pr_info("%s, wl_tcpka_conn_enable, err = %d\n", __FUNCTION__, ret);
		return ret;
    }

	wowl_caps = WL_WOWL_NET | WL_WOWL_DIS | WL_WOWL_BCN | WL_WOWL_TCPKEEP_TIME;
	ret = wl_wowl_enable(
				wowl_caps,
				WOWL_PATTERN_MATCH_OFFSET,
				(const int8_t*)wowl_pattern,
				strlen(wowl_pattern));
    if (ret < 0)
	{
        pr_info("%s, wl_wowl_enable, err = %d\n", __FUNCTION__, ret);
		return ret;
    }

	ret = wl_set_listen_interval(CUSTOM_LISTEN_INTERVAL);
    if (ret < 0)
	{
        pr_info("%s, wl_set_listen_interval, err = %d\n", __FUNCTION__, ret);
		return ret;
    }
#endif

	ret = wl_set_deepsleep(true);
	if (ret < 0)
	{
		pr_info("%s, wl_set_deepsleep, err = %d\n", __FUNCTION__, ret);
		return ret;
	}

	ret = wl_set_powersave(true);
	if (ret < 0)
	{
		pr_info("%s, wl_set_powersave, err = %d\n", __FUNCTION__, ret);	
		return ret;
	}

#if (1)
	ret = wl_set_hostsleep(true);
	if (ret < 0)
	{
		pr_info("%s, wl_set_hostsleep, err = %d\n", __FUNCTION__, ret);	
	}

	return ret;	
#endif
}

int WIFI_Resume(void)
{
	int ret;

	pr_info("%s, enter\n", __FUNCTION__);

	ret = wl_set_hostsleep(false);
	if (ret < 0)
	{
		pr_info("%s, wl_set_hostsleep, err = %d\n", __FUNCTION__, ret);
		return ret;
	}

	ret = wl_set_deepsleep(false);
	if (ret < 0)
	{
		pr_info("%s, wl_set_deepsleep, err = %d\n", __FUNCTION__, ret);
	}

	ret = wl_set_powersave(false);
	if (ret < 0)
	{
		pr_info("%s, wl_set_powersave, err = %d\n", __FUNCTION__, ret);
		return ret;
	}

	ret = wl_reset_listen_interval();
	if (ret < 0)
	{
		pr_info("%s, wl_reset_listen_interval, err = %d\n", __FUNCTION__, ret);
		return ret;
	}

	ret = wl_wowl_clear_wakeind();
	if (ret < 0)
	{
		pr_info("%s, wl_wowl_clear_wakeind, err = %d\n", __FUNCTION__, ret);
		return ret;
	}

	ret = wl_wowl_disable();
	if (ret < 0)
	{
		pr_info("%s, wl_wowl_disable, err = %d\n", __FUNCTION__, ret);
		return ret;
	}

	ret = wl_tcpka_conn_disable();
	if (ret < 0)
	{
		pr_info("%s, wl_tcpka_conn_disable, err = %d\n", __FUNCTION__, ret);
	}

	return ret;
}
