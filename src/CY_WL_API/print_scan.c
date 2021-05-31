#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include "wl.h"

#define DEFAULT_IFNAME				"wlan0"
#define MAX_SCAN_NUM				10
#define SCAN_DWELL_TIME_PER_CHANNEL	(200)

static void usage(void)
{
	printf("usage:\n"
	       "	scan [-i <ifname>] [-t <scan_time_per_channel>]\n");
}

int print_scan(uint32_t time_per_channel)
{
	int ret;

	ret = wl_scan(NULL, NULL, NULL, 0, time_per_channel);

	if (ret == WL_E_OK)
	{
		int total = 0;
		printf("==================================================================\n");
		printf("| No.\t| SSID\t\t\t| BSSID\t\t\t\t| Security\t| Channel |\n");
		printf("------------------------------------------------------------------\n");

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
				int i;

				for (i = 0; i < count; i ++)
				{
					printf("| %d\t| %s\t| %02x:%02x:%02x:%02x:%02x:%02x\t| 0x%08x\t| %d\t\t|\n",
						total,
						ap_info[i].ssid.value,
						ap_info[i].bssid.octet[0], ap_info[i].bssid.octet[1], ap_info[i].bssid.octet[2],
						ap_info[i].bssid.octet[3], ap_info[i].bssid.octet[4], ap_info[i].bssid.octet[5],
						ap_info[i].security,
						ap_info[i].channel);
					total++;
				}		
			}

			if (is_completed)
			{
				break;
			}
		}
		printf("==================================================================\n");
	}
	else
	{
		printf("wl_scan err = %d\n", ret);
	}

	return ret;
}


int main(int argc, char **argv)
{
	int ret;
	char *ifname = DEFAULT_IFNAME; 
	uint32_t time_per_channel = WL_SCAN_PER_CH_TIME_DEFAULT;

	argv++;

	while (*argv)
	{
		if (!strcmp(*argv, "-i"))
		{
			argv++;

			if (argv != NULL)
			{
				ifname = *argv;
			}
			else
			{
				usage();
				return 0;
			}
		}
		else if (!strcmp(*argv, "-t"))
		{
			argv++;

			if (argv != NULL)
			{
				time_per_channel = atoi(*argv);
			}
			else
			{
				usage();
				return 0;
			}
		}
		else
		{
			usage();
			return 0;
		}

		argv++;
	}
	
	ret = wl_init(ifname);
	if (ret < 0)
	{
		printf("wl_init failed.\n");	
		return 0;
	}

	ret = print_scan(time_per_channel);

	wl_deinit();
	
	return 0;
}
