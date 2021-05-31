#include "wl.h"
#include "wifi.h"

typedef 	unsigned int    	BOOL;
#define SCAN_DWELL_TIME_PER_CHANNEL	(200)
#define DEBUG 0
#define VENDOR_WIFI_INFO_ID 30 // VENDOR_CUSTOM_ID_1E
#define DEFAULT_IFNAME	"wlan0"
const char tcpka_payload[] = "sleep";
const char wowl_pattern[] = "wakeup";

typedef	struct wifi_info
{
	char ssid[64];
	char psk[64];
	char ip_addr[64];
	char netmask[64];
	char gateway[64];
	char dns[64];
} wifi_info_s;

static wifi_info_s g_wifi_info;

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
	int fd;
	struct ifreq ifr; 
    struct sockaddr_in *sin;
    struct rtentry	rt;

	pr_info("get dhcp info from vendor\n");
	//rkvendor_read(VENDOR_WIFI_INFO_ID, (char *)&g_wifi_info, sizeof(g_wifi_info));
	pr_info("%s: ip_addr: %s, netmask: %s, gateway: %s, dns: %s\n",
		__FUNCTION__, g_wifi_info.ip_addr, g_wifi_info.netmask, g_wifi_info.gateway, g_wifi_info.dns);
	// set
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0) {
		pr_info("socket error\n");
		return -1;
	}
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, DEFAULT_IFNAME);
    sin = (struct sockaddr_in*)&ifr.ifr_addr;
    sin->sin_family = AF_INET;
    // ip_addr
    if(inet_aton(g_wifi_info.ip_addr, &(sin->sin_addr)) < 0) {
        pr_info("inet_aton error\n");
        return -2;
    }
    if(ioctl(fd, SIOCSIFADDR, &ifr) < 0) {
        pr_info("ioctl SIOCSIFADDR error\n");
        return -3;
    }
    // netmask
    if(inet_aton(g_wifi_info.netmask, &(sin->sin_addr)) < 0) {
        pr_info("inet_pton error\n");
        return -4;
    }
    if(ioctl(fd, SIOCSIFNETMASK, &ifr) < 0) {
        pr_info("ioctl error\n");
        return -5;
    }
	// dns
	FILE *fp = NULL;
	fp = fopen("/etc/resolv.conf", "w");
	if (!fp) {
		pr_info("fopen /etc/resolv.conf error\n");
		return -1;
	}
	fwrite(g_wifi_info.dns, strlen(g_wifi_info.dns), 1, fp);
    fclose(fp);
    // gateway
    memset(&rt, 0, sizeof(struct rtentry));
    memset(sin, 0, sizeof(struct sockaddr_in));
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    if(inet_aton(g_wifi_info.gateway, &sin->sin_addr) < 0) {
       pr_info ("inet_aton error\n");
    }
    memcpy(&rt.rt_gateway, sin, sizeof(struct sockaddr_in));
    ((struct sockaddr_in *)&rt.rt_dst)->sin_family = AF_INET;
    ((struct sockaddr_in *)&rt.rt_genmask)->sin_family = AF_INET;
    rt.rt_flags = RTF_GATEWAY;
    if (ioctl(fd, SIOCADDRT, &rt) < 0) {
        pr_info("ioctl(SIOCADDRT) error in set_default_route or already set\n");
        close(fd);
        return 0;
    }
    close(fd);

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
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
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
	pr_info("%s, enter\n", __FUNCTION__);

	rk_system_return("ifconfig wlan0 | grep inet | awk '{print $2}' | awk -F: '{print $2}'",
					 g_wifi_info.ip_addr, 64);
	if (g_wifi_info.ip_addr[strlen(g_wifi_info.ip_addr) - 1] == '\n')
		g_wifi_info.ip_addr[strlen(g_wifi_info.ip_addr) - 1] = 0;
	pr_info("%s: g_wifi_info.ip_addr: %s\n", __FUNCTION__, g_wifi_info.ip_addr);

	rk_system_return("ifconfig wlan0 | grep inet | awk '{print $4}' | awk -F: '{print $2}'",
					 g_wifi_info.netmask, 64);
	if (g_wifi_info.netmask[strlen(g_wifi_info.netmask) - 1] == '\n')
		g_wifi_info.netmask[strlen(g_wifi_info.netmask) - 1] = 0;
	pr_info("%s: g_wifi_info.netmask: %s\n", __FUNCTION__, g_wifi_info.netmask);

	rk_system_return("route -n | awk '{print $2}' | sed -n '3p'",
					 g_wifi_info.gateway, 64);
	if (g_wifi_info.gateway[strlen(g_wifi_info.gateway) - 1] == '\n')
		g_wifi_info.gateway[strlen(g_wifi_info.gateway) - 1] = 0;
	pr_info("%s: g_wifi_info.gateway: %s\n", __FUNCTION__, g_wifi_info.gateway);

	rk_system_return("cat /etc/resolv.conf | grep nameserver | grep wlan0 | awk '{print $2}'",
					 g_wifi_info.dns, 64);
	if (g_wifi_info.dns[strlen(g_wifi_info.dns) - 1] == '\n')
		g_wifi_info.dns[strlen(g_wifi_info.dns) - 1] = 0;
	pr_info("%s: g_wifi_info.dns: %s\n", __FUNCTION__, g_wifi_info.dns);

	//rkvendor_write(VENDOR_WIFI_INFO_ID, (const char *)&g_wifi_info, sizeof(g_wifi_info));
	pr_info("%s: ssid:%s, psk:%s, ip_addr:%s, netmask:%s, gateway:%s, dns:%s\n",
			__FUNCTION__, g_wifi_info.ssid, g_wifi_info.psk, g_wifi_info.ip_addr,
			g_wifi_info.netmask, g_wifi_info.gateway, g_wifi_info.dns);
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

	printf("*************TCP KEEPALIVE DEBUG.**************\n");
	len = sizeof(struct sockaddr_in);
	int i = 0;
	for (i = 0; i < 3; i++)
	{
		ret = getsockname(sock, (struct sockaddr*)&localaddr, &len);
		if (ret < 0)
		{
	        pr_info("%s, getsockname err = %d  errno:%d\n", __FUNCTION__, ret, errno);
			//return ret;
		} 
		else
		{
			break;
		}
	}

	if (i == 3)
	{
		printf("getsockname : try max cnt:3 err.\n");	
		return  -1;
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
	for (i = 0; i < 3; i++)
	{
		ret = getpeername(sock, (struct sockaddr*)&peeraddr, &len);
		if (ret < 0)
		{
	        pr_info("%s, getpeername err = %d errno:%d\n", __FUNCTION__, ret, errno);
			//return ret;
			//continue;
		}
		else
		{
			break;
		}
	}

	if (i == 3)
	{
		printf("getpeername : try max cnt:3 err.\n");	
		return  -1;
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
	
	//ret = system("insmod /vendor/lib/modules/cywdhd.ko");
	//pr_info("%s, insmod ret = %d\n", __FUNCTION__, ret);

	while (retry--) {
		ret = system("ifconfig wlan0 up");
		pr_info("%s, up ret = %d\n", __FUNCTION__, ret);
		if (ret == 0)
			break;
		usleep(200 * 1000);
	}

	ret = wl_init(DEFAULT_IFNAME);
	pr_info("%s, wl_init ret = %d\n", __FUNCTION__, ret);

	/* insmod Wi-Fi Module */
	//system("ifconfig usb0 down");

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
	wl_ap_info_t join_list[MAX_SAME_AP_NUM];
	uint32_t ap_count = 0;

	memset(g_wifi_info.ssid, 0, 64);
	memset(g_wifi_info.psk, 0, 64);
	memcpy(g_wifi_info.ssid, ssid_name, strlen(ssid_name));
	memcpy(g_wifi_info.psk, password, strlen(password));

	memset(&ssid, 0, sizeof(ssid));
	ssid.len = strlen(ssid_name);
	memcpy(ssid.value, ssid_name, ssid.len);

	printf("%s, enter\n", __FUNCTION__);

	if (password == NULL)
	{
		printf("Open security type\n");
		join_list[0].security = WL_SECURITY_OPEN;
		join_list[0].channel = 0;
		ap_count = 1;
	}
	else
	{
		printf("Scan for the security type\n");
		
		ret = wl_scan(&ssid, NULL, NULL, 0, WL_SCAN_PER_CH_TIME_DEFAULT);

		printf("wl_scan result = %d\n", ret);
		if (ret < 0)
		{
			return ret;
		}

		while (1)
		{
			wl_ap_info_t ap_info[MAX_SCAN_NUM];
			uint32_t count = MAX_SCAN_NUM;
			bool is_completed = false;

			ret = wl_get_scan_results(ap_info, &count, &is_completed);

			if (ret < 0)
			{
				printf("wl_get_scan_results failed, result = %d\n", ret);
				break;
			}
			else
			{
				for (i = 0; i < count; i++)
				{
					if (ap_count < MAX_SAME_AP_NUM)
					{
						printf("Get AP[%d] security = 0x%08x, channel = %d\n", 
							ap_count, ap_info[i].security, ap_info[i].channel);

						memcpy(&join_list[ap_count], &ap_info[i], sizeof(wl_ap_info_t));
						ap_count++;
					}
				}

				if (is_completed)
				{
					printf("Scan Done\n");
					break;
				}
			}
		}
	}

	if (ap_count > 0)
	{
		/* Try to join each of the found AP until successfully */
		for (i = 0; i < ap_count; i++)
		{
			uint32_t security = join_list[i].security;
		    struct timeval start_time;
			struct timeval end_time;
			double join_time;

			printf("Starting to join AP[%d] : %s, password: %s\n", i, ssid_name, password);

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

			for (j = 0; j < JOIN_AP_RETRIES; j++)
			{
				uint8_t key_length = 0;

				if (password != NULL)
				{
					key_length = strlen(password);
				}

				gettimeofday(&start_time, NULL);

				ret = wl_join_ap_specific(
							&ssid,
							security,
							(int8_t*)password,
							key_length,
							NULL,
							join_list[i].channel);

				if (ret < 0)
				{
					printf("Try to join AP %d %d times  failed, ret = %d\n", i, j, ret);
				}
				else
				{
					gettimeofday(&end_time, NULL);
					join_time = (end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec);
				 	printf("Joined AP in time (ms) = %lf\n", join_time / 1000.0);
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
			printf("Joined AP succcessfully\n");
		}
	}
	else
	{
		printf("Can't find AP: %s\n", ssid_name);
		return WL_E_NO_NETWORK;
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

	memset(&g_wifi_info.ip_addr, 0, 64);
	memset(&g_wifi_info.netmask, 0, 64);
	memset(&g_wifi_info.gateway, 0, 64);
	memset(&g_wifi_info.dns, 0, 64);
	memcpy(g_wifi_info.ip_addr, ipAddr, strlen(ipAddr));
	memcpy(g_wifi_info.netmask, netmask, strlen(netmask));
	memcpy(g_wifi_info.gateway, gateway, strlen(gateway));
	memcpy(g_wifi_info.dns, dns, strlen(dns));
	pr_info("%s: ssid:%s, psk:%s, ip_addr:%s, netmask:%s, gateway:%s, dns:%s\n",
			__FUNCTION__, g_wifi_info.ssid, g_wifi_info.psk, g_wifi_info.ip_addr,
			g_wifi_info.netmask, g_wifi_info.gateway, g_wifi_info.dns);
	//rkvendor_write(VENDOR_WIFI_INFO_ID, (const char *)&g_wifi_info, sizeof(g_wifi_info));

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
 
int WIFI_ClientScan(void)
{
	return wl_scan(NULL, NULL, NULL, 0, WL_SCAN_PER_CH_TIME_DEFAULT);
}

int WIFI_GetClientScanResults(wifi_ap_info_t *pstResults, uint32_t num)
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

int builepass(char *sn, char *pass)
{
	char tmpsn[9];

	memset(tmpsn, 0, 9);
	strcpy(tmpsn, "va");
	memcpy(tmpsn + 2, sn + 6, 2);
	memcpy(tmpsn + 4, sn, 2);
	memcpy(tmpsn + 6, sn + 4, 2);
	strcpy(pass, tmpsn);
	printf("pass:%s\n", pass);
	
	return 0;
}

int WIFI_set_wakeup_ssid(char *ssid)
{
	if (!ssid)	return  -1;
	
	int ret = 0;
	wl_ssid_t wlssid;

	wlssid.len = strlen(ssid);
	
	pr_info("enter sleep mode by ssid[%s]\tlen:%d.\n", ssid, wlssid.len);
	
	if (wlssid.len > WL_MAX_SSID_LEN)
	{
		goto exit;
	}
	
	strncpy((char *)wlssid.value, ssid, WL_MAX_SSID_LEN - 1);
	wlssid.value[WL_MAX_SSID_LEN - 1] = '\0';
	ret = wl_wakeup_by_ssid_start(&wlssid, 60);
	
	if (ret < 0)
	{
		pr_info("wl_wakeup_by_ssid_start, err = %d\n", ret);
		goto exit;
	}

	ret = wl_set_hostsleep(true);
	if (ret < 0)
	{
		pr_info("%s, wl_set_hostsleep, err = %d\n", __FUNCTION__, ret);	
	}

	printf("*********set ssid wakeup ok.*************\n");
	return  ret;
	
exit:
	pr_info("set ssid param err.\n");
	return ret;
}

int WIFI_Suspend(int sock, bool is_connected)
{
	int ret;
	wl_tcpka_param_t tcpka_param;
    uint32_t wowl_caps;

	printf("%s, enter\n", __FUNCTION__);

	if (is_connected)
	{
		ret = rk_build_tcpka_param(&tcpka_param, DEFAULT_IFNAME, sock);
	    if (ret < 0)
		{
	        printf("%s, build_tcpka_param, err = %d\n", __FUNCTION__, ret);
			return ret;
	    }

		ret = wl_tcpka_conn_enable(&tcpka_param);
	    if (ret < 0)
		{
	        printf("%s, wl_tcpka_conn_enable, err = %d\n", __FUNCTION__, ret);
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
	        printf("%s, wl_wowl_enable, err = %d\n", __FUNCTION__, ret);
			return ret;
	    }

		ret = wl_set_listen_interval(CUSTOM_LISTEN_INTERVAL);
	    if (ret < 0)
		{
	        printf("%s, wl_set_listen_interval, err = %d\n", __FUNCTION__, ret);
			return ret;
	    }

		ret = wl_set_deepsleep(true);
		if (ret < 0)
		{
			printf("%s, wl_set_deepsleep, err = %d\n", __FUNCTION__, ret);
		}

		ret = wl_set_powersave(true);
		if (ret < 0)
		{
			printf("%s, wl_set_powersave, err = %d\n", __FUNCTION__, ret);	
			return ret;
		}
	}

	ret = wl_set_hostsleep(true);
	if (ret < 0)
	{
		printf("%s, wl_set_hostsleep, err = %d\n", __FUNCTION__, ret);	
	}

	return ret;	

}
    
int WIFI_Resume(void)
{
	int ret;
	uint32_t wakeind = 0;

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

	ret = wl_wowl_get_wakeind(&wakeind);
	if (ret < 0)
	{
        pr_info("%s, wl_wowl_get_wakeind, err = %d\n", __FUNCTION__, ret);
		return ret;
    }
	pr_info("%s, wakeind = 0x%08x\n", __FUNCTION__, wakeind);

	ret = wl_wowl_clear_wakeind();
	if (ret < 0)
	{
		pr_info("%s, wl_wowl_clear_wakeind, err = %d\n", __FUNCTION__, ret); 
		return ret; 	
	}

	if (wakeind & WL_WOWL_NFYSCAN_WAKE)
	{
        pr_info("%s, wakeup by SSID\n", __FUNCTION__);
		wl_wakeup_by_ssid_stop();
	}
	else
	{
		if (wakeind & WL_WOWL_NET)
		{        
			wl_wowl_tcp_rst();
		}

		ret = wl_wowl_disable();
		if (ret < 0)
		{
			printf("%s, wl_wowl_disable, err = %d\n", __FUNCTION__, ret);	
			return ret;		
		}
	
		ret = wl_tcpka_conn_disable();
		if (ret < 0)
		{
			printf("%s, wl_tcpka_conn_disable, err = %d\n", __FUNCTION__, ret);	
		}
	}
	
	return ret;
}
