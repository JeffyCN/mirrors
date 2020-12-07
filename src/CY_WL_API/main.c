#include "wifi.h"

static void usage(void)
{
	printf("usage:\n"
	       "	keepalive [-i <ifname>] -s <SSID> [-k <PASSWORD>] -a <Server IP ADDRESS> -p <Server TCP Port>\n");
}

static int network_up(char* ifname)
{
	int ret;
	char cmd[256];

	pr_info("%s, enter\n", __FUNCTION__);

	sprintf(cmd, "ifconfig %s up", ifname);
	pr_info("%s, CMD: %s\n", cmd, __FUNCTION__);

	ret = system(cmd);

	pr_info("%s, ret = %d\n", __FUNCTION__, ret);

	sleep(1);

	return ret;
}

static int connect_ap(char* ssid_name, char* password)
{
	int ret;
	wl_ssid_t ssid;
	uint32_t i;
	uint32_t j;
	uint32_t ap_security_list[MAX_SAME_AP_NUM];
	uint32_t ap_count = 0;

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
						pr_info("get AP %d security = 0x%08x\n", ap_count, ap_info[i].security);
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

static int ping_server(char* ip)
{
	int ret;
	char cmd[256];

	pr_info("%s, enter\n", __FUNCTION__);

	sprintf(cmd, "ping %s -c 5", ip);

	pr_info("%s, CMD: %s\n", __FUNCTION__, cmd);

	ret = system(cmd);

	return ret;
}

static int connect_server(char *ip, int port)
{
	int sock;
	int ret;
	int i;
	struct sockaddr_in addr = { 0 };

	printf("%s, enter\n", __FUNCTION__);

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if(sock < 0) {
		printf("%s, create sock err = %d\n", __FUNCTION__, sock);
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(port);

	for (i = 0; i < TCP_CONNECT_RETRIES; i++)
	{
		ret = connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

		if (ret < 0)
		{
			printf("%s, connect server err = %d\n", __FUNCTION__, ret);
		}
		else
		{
			break;
		}
	}

	if (ret < 0)
	{
		close(sock);
		return ret;
	}
	else
	{
		return sock;
	}
}

static bool wifi_get_wakeup(void)
{
	int ret = 0;
	uint32_t wakeind = 0;

	pr_info("%s, enter\n", __FUNCTION__);

	wakeind = WIFI_GetWakupReason();
	//ret = wl_wowl_get_wakeind(&wakeind);
	if (wakeind == 0)
	{
		pr_info("%s, wl_wowl_get_wakeind, ret = %d\n", __FUNCTION__, ret);
	}
	else
	{
		if (wakeind & WL_WOWL_NET)
		{
			pr_info("%s, wakeup by APP\n", __FUNCTION__);
		}

		if (wakeind & WL_WOWL_DIS)
		{
			pr_info("%s, wakeup by disassocation\n", __FUNCTION__);			
		}

		if (wakeind & WL_WOWL_BCN)
		{
			pr_info("%s, wakeup by network lost\n", __FUNCTION__);			
		}

		if (wakeind & WL_WOWL_TCPKEEP_TIME)
		{
			pr_info("%s, wakeup by tcp connection lost\n", __FUNCTION__);			
		}
	}

	return (wakeind != 0);
}

extern int print_scan(uint32_t time_per_channel);

int main(int argc, char **argv)
{
	int ret;
	char *ifname = DEFAULT_IFNAME; 
	char *ssid = NULL; 
	char *password = NULL; 
	char *ip = NULL;
	int port = 0;
    int sock = -1;
	int scan_time = 300;

    if (argc < 2)
	{
		usage();
    }

	while (*argv)
	{
		if (!strcmp(*argv, "-h"))
		{
			usage();
		}
		else if (!strcmp(*argv, "-i"))
		{
			argv++;
			ifname = *argv;
		}
		else if (!strcmp(*argv, "-s"))
		{
			argv++;
			ssid = *argv;
		}
		else if (!strcmp(*argv, "-k"))
		{
			argv++;
			password = *argv;
		}
		else if (!strcmp(*argv, "-a"))
		{
			argv++;
			ip = *argv;
		}
		else if (!strcmp(*argv, "-p"))
		{
			argv++;
			port = atoi(*argv);
		}
		else if (!strcmp(*argv, "scan"))
		{
			argv++;

			WIFI_Init();
			WIFI_ClientScan(*argv);
			WIFI_Deinit();
			return 0;
		}
		else if (!strcmp(*argv, "scan_r"))
		{
			WIFI_Init();
			argv++;
			scan_time = atoi(*argv);
			print_scan(scan_time);
			//WIFI_GetClientScanResults(NULL, 100);
			WIFI_Deinit();
			return 0;
		}
		else if (!strcmp(*argv, "status"))
		{
			WIFI_Init();
			int ret;
			ret = WIFI_GetStatus();
			pr_info("status: %d\n", ret);
			WIFI_Deinit();
			return 0;
		}
		else if (!strcmp(*argv, "channel"))
		{
			WIFI_Init();
			int ret;
			ret = WIFI_GetChannel();
			pr_info("Channel: %d\n", ret);
			WIFI_Deinit();
			return 0;
		}			
		else if (!strcmp(*argv, "signal"))
		{
			WIFI_Init();
			int ret;
			ret = WIFI_GetSignal();
			pr_info("Signal: %d\n", ret);
			WIFI_Deinit();
			return 0;
		}
		else if (!strcmp(*argv, "ver"))
		{
			pr_info("v1.0\n");
		}

		argv++;
	}

	if ((ssid == NULL) || (ip == NULL) || (port == 0))
	{
		usage();
		return 0;
	}

	ret = WIFI_Init();
	if (ret < 0)
	{
		pr_info("WIFI_Init, err = %d\n", ret);	
		return 0;
	}

	wifi_get_wakeup();

	ret = WIFI_Resume();
	if (ret)
	{
		pr_info("resume_enter, err = %d\n", ret);	
		goto exit;
	}

	if (WIFI_GetStatus()) //if (wl_is_associated())
	{
		pr_info("Already joined AP.\n");
		rk_obtain_ip_from_vendor(ifname);
	}
	else
	{
		ret = WIFI_Connect(ssid, password, 0);

		if (ret < 0)
		{
			goto exit;
		}

		ret = rk_obtain_ip_from_udhcpc(ifname);
		if (ret)
		{
			pr_info("obtain_ip, err = %d\n", ret);	
			goto exit;
		}
	}

	ret = ping_server(ip);
	if (ret)
	{
		pr_info("ping_server, err = %d\n", ret);	
		goto exit;
	}	

	sock = connect_server(ip, port);
	if (sock < 0)
	{
		goto exit;		
	}

	ret = send(sock, tcpka_payload, strlen(tcpka_payload), 0);
    if (ret < 0)
	{
        pr_info("send err = %d\n", ret);
		goto exit;
    }

	sleep(2);

	ret = WIFI_Suspend(sock);

	sleep(12000);

#if (0)
	while (1)
	{
		sleep(1);
		if (wifi_get_wakeup())
		{
			break;
		}
	}
#endif

exit:
	if (sock < 0)
	{
//		close(sock);
	}

	wl_deinit();
	
	return 0;
}
