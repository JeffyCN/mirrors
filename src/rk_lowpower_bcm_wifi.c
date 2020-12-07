#include <stdio.h>
#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <errno.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <syslog.h>
#include <fcntl.h>
#include <stdbool.h>
#include <assert.h>
#include <paths.h>
#include <sys/wait.h>

#define LOG_TAG "RK_BCM_LOG"
#define SYSLOG_DEBUG
#define DEBUG 1

#ifdef SYSLOG_DEBUG
#define pr_debug(fmt, ...)      syslog(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define pr_info(fmt, ...)       syslog(LOG_INFO, fmt, ##__VA_ARGS__)
#define pr_warning(fmt, ...)    syslog(LOG_WARNING, fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...)        syslog(LOG_ERR, fmt, ##__VA_ARGS__)
#else
#define pr_debug    printf
#define pr_info printf
#define pr_warning printf
#define pr_err printf
#endif

static int system_fd_closexec(const char* command)
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

static int rk_command_system(const char *cmd)
{
    pid_t status;

    status = system_fd_closexec(cmd);

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

void rk_command(const char cmdline[], char recv_buff[], int len)
{
    if (DEBUG)
        pr_info("[BT_DEBUG] execute: %s\n", cmdline);

    FILE *stream = NULL;
    char *tmp_buff = recv_buff;

    memset(recv_buff, 0, len);

    if ((stream = popen(cmdline, "r")) != NULL) {
        while (fgets(tmp_buff, len, stream)) {
            //pr_info("tmp_buf[%d]: %s\n", strlen(tmp_buff), tmp_buff);
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

static int rk_get_pid(const char Name[])
{
    int len;
    char name[32] = {0};
    char cmdresult[256] = {0};
    char cmd[64] = {0};
    FILE *pFile = NULL;
    int  pid = 0;

    len = strlen(Name);
    strncpy(name,Name,len);
    name[31] ='\0';

    sprintf(cmd, "pidof %s", name);

    pFile = popen(cmd, "r");
    if (pFile != NULL)  {
        while (fgets(cmdresult, sizeof(cmdresult), pFile)) {
            pid = atoi(cmdresult);
            break;
        }
        pclose(pFile);
    }

    return pid;
}

int rk_kill_task(char *name)
{
    char cmd[128] = {0};
    int exec_cnt = 3, retry_cnt = 10;

    if (!rk_get_pid(name))
        return 0;

    memset(cmd, 0, 128);
    sprintf(cmd, "killall %s", name);

    while(exec_cnt) {
        if(!rk_command_system(cmd))
            break;
        exec_cnt--;
    }

    if(exec_cnt <= 0) {
        pr_info("%s: kill %s failed\n", __func__, name);
        return -1;
    }
    msleep(100);

retry:
    if (rk_get_pid(name) && (retry_cnt--)) {
        msleep(100);
        goto retry;
    }

    pr_info("%s: kill %s, retry_cnt = %d\n", __func__, name, retry_cnt);
    if (rk_get_pid(name))
        return -1;
    else
        return 0;
}

int rk_run_task(char *name, char *cmd)
{
    int exec_cnt = 3, retry_cnt = 6;

    while(exec_cnt) {
        if(!rk_command_system(cmd))
            break;
        exec_cnt--;
    }

    if(exec_cnt <= 0) {
        pr_info("%s: run %s failed\n", __func__, name);
        return -1;
    }
    msleep(100);

retry:
    if (!rk_get_pid(name) && (retry_cnt--)) {
        msleep(100);
        goto retry;
    }

    if (!rk_get_pid(name))
        return -1;
    else
        return 0;
}

/****** wpa_supplicant ******/
#include <poll.h>
#include <wpa_ctrl.h>

#define EVENT_BUF_SIZE 1024
#define PROPERTY_VALUE_MAX 32
#define PROPERTY_KEY_MAX 32

static void wifi_close_sockets();

static const char WPA_EVENT_IGNORE[]    = "CTRL-EVENT-IGNORE ";
static const char IFNAME[]              = "IFNAME=";
static const char IFACE_DIR[]           = "/var/run/wpa_supplicant";

#define IFNAMELEN                       (sizeof(IFNAME) - 1)

static struct wpa_ctrl *ctrl_conn;
static struct wpa_ctrl *monitor_conn;

#define DBG_NETWORK 1

static int exit_sockets[2];

static char primary_iface[32] = "wlan0";
static int dispatch_event(char* event)
{
    if (strstr(event, "CTRL-EVENT-BSS") || strstr(event, "CTRL-EVENT-TERMINATING"))
        return 0;

    pr_info("%s: %s\n", __func__, event);

    if (str_starts_with(event, (char *)WPA_EVENT_DISCONNECTED)) {
        pr_info("%s: wifi is disconnect\n", __FUNCTION__);
        rk_command_system("ip addr flush dev wlan0");
        rk_command_system("wpa_cli -i wlan0 reconnect");
    } else if (str_starts_with(event, (char *)WPA_EVENT_CONNECTED)) {
        pr_info("%s: wifi is connected\n", __func__);
    } else if (str_starts_with(event, (char *)WPA_EVENT_SCAN_RESULTS)) {
        pr_info("%s: wifi event results\n", __func__);
    } else if (strstr(event, "reason=WRONG_KEY")) {
        pr_info("%s: wifi reason=WRONG_KEY \n", __func__);
    } else if (str_starts_with(event, (char *)WPA_EVENT_TERMINATING)) {
        pr_info("%s: wifi is WPA_EVENT_TERMINATING!\n", __func__);
        wifi_close_sockets();
        return -1;
    }

    return 0;
}

static int check_wpa_supplicant_state() {
    int count = 5;
    int wpa_supplicant_pid = 0;
    wpa_supplicant_pid = get_pid("wpa_suppplicant");

    pr_info("%s: wpa_supplicant_pid = %d\n", __FUNCTION__, wpa_supplicant_pid);

    if (wpa_supplicant_pid > 0) {
        return 1;
    }

    return 0;
}

static int wifi_ctrl_recv(char *reply, size_t *reply_len)
{
    int res;
    int ctrlfd = wpa_ctrl_get_fd(monitor_conn);
    struct pollfd rfds[2];

    memset(rfds, 0, 2 * sizeof(struct pollfd));
    rfds[0].fd = ctrlfd;
    rfds[0].events |= POLLIN;
    rfds[1].fd = exit_sockets[1];
    rfds[1].events |= POLLIN;
    do {
        res = TEMP_FAILURE_RETRY(poll(rfds, 2, 30000));
        if (res < 0) {
            pr_info("Error poll = %d\n", res);
            return res;
        } else if (res == 0) {
            /* timed out, check if supplicant is activeor not .. */
            res = check_wpa_supplicant_state();
            if (res < 0)
                return -2;
        }
    } while (res == 0);

    if (rfds[0].revents & POLLIN) {
        return wpa_ctrl_recv(monitor_conn, reply, reply_len);
    }
    return -2;
}

static int wifi_wait_on_socket(char *buf, size_t buflen)
{
    size_t nread = buflen - 1;
    int result;
    char *match, *match2;

    if (monitor_conn == NULL) {
        return snprintf(buf, buflen, "IFNAME=%s %s - connection closed",
            primary_iface, WPA_EVENT_TERMINATING);
    }

    result = wifi_ctrl_recv(buf, &nread);

    /* Terminate reception on exit socket */
    if (result == -2) {
        return snprintf(buf, buflen, "IFNAME=%s %s - connection closed",
            primary_iface, WPA_EVENT_TERMINATING);
    }

    if (result < 0) {
        pr_info("wifi_ctrl_recv failed: %s\n", strerror(errno));
        //return snprintf(buf, buflen, "IFNAME=%s %s - recv error",
        //  primary_iface, WPA_EVENT_TERMINATING);
    }

    buf[nread] = '\0';

    /* Check for EOF on the socket */
    if (result == 0 && nread == 0) {
        /* Fabricate an event to pass up */
        pr_info("Received EOF on supplicant socket\n");
        return snprintf(buf, buflen, "IFNAME=%s %s - signal 0 received",
            primary_iface, WPA_EVENT_TERMINATING);
    }

    if (strncmp(buf, IFNAME, IFNAMELEN) == 0) {
        match = strchr(buf, ' ');
        if (match != NULL) {
            if (match[1] == '<') {
                match2 = strchr(match + 2, '>');
                    if (match2 != NULL) {
                        nread -= (match2 - match);
                        memmove(match + 1, match2 + 1, nread - (match - buf) + 1);
                    }
            }
        } else {
            return snprintf(buf, buflen, "%s", WPA_EVENT_IGNORE);
        }
    } else if (buf[0] == '<') {
        match = strchr(buf, '>');
        if (match != NULL) {
            nread -= (match + 1 - buf);
            memmove(buf, match + 1, nread + 1);
            if (0)
                pr_info("supplicant generated event without interface - %s\n", buf);
        }
    } else {
        if (0)
            pr_info("supplicant generated event without interface and without message level - %s\n", buf);
    }

    return nread;
}

static int wifi_connect_on_socket_path(const char *path)
{
    char supp_status[PROPERTY_VALUE_MAX] = {'\0'};

    if(!check_wpa_supplicant_state()) {
        pr_info("%s: wpa_supplicant is not ready\n",__FUNCTION__);
        return -1;
    }

    ctrl_conn = wpa_ctrl_open(path);
    if (ctrl_conn == NULL) {
        pr_info("Unable to open connection to supplicant on \"%s\": %s\n",
        path, strerror(errno));
        return -1;
    }
    monitor_conn = wpa_ctrl_open(path);
    if (monitor_conn == NULL) {
        wpa_ctrl_close(ctrl_conn);
        ctrl_conn = NULL;
        return -1;
    }
    if (wpa_ctrl_attach(monitor_conn) != 0) {
        wpa_ctrl_close(monitor_conn);
        wpa_ctrl_close(ctrl_conn);
        ctrl_conn = monitor_conn = NULL;
        return -1;
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, exit_sockets) == -1) {
        wpa_ctrl_close(monitor_conn);
        wpa_ctrl_close(ctrl_conn);
        ctrl_conn = monitor_conn = NULL;
        return -1;
    }
    return 0;
}

/* Establishes the control and monitor socket connections on the interface */
static int wifi_connect_to_supplicant()
{
    static char path[1024];
    int count = 10;

    pr_info("%s \n", __FUNCTION__);
    while(count-- > 0) {
        if (access(IFACE_DIR, F_OK) == 0)
            break;
        sleep(1);
    }

    snprintf(path, sizeof(path), "%s/%s", IFACE_DIR, primary_iface);

    return wifi_connect_on_socket_path(path);
}

static void rk_wifi_start_monitor(void *arg)
{
    char eventStr[EVENT_BUF_SIZE];
    int ret;

    prctl(PR_SET_NAME, "rk_wifi_monitor");

    if ((ret = wifi_connect_to_supplicant()) != 0) {
        pr_info("%s, connect to supplicant fail.\n", __FUNCTION__);
        return;
    }

    for (;;) {
        memset(eventStr, 0, EVENT_BUF_SIZE);
        if (!wifi_wait_on_socket(eventStr, EVENT_BUF_SIZE))
            continue;

        if (dispatch_event(eventStr)) {
            pr_info("disconnecting from the supplicant, no more events\n");
            break;
        }
    }
}

/****** wpa_supplicant end ******/

void rk_wifi_module_init()
{
    rk_command_system("insmod /vendor/lib/modules/bcmdhd_indep_power.ko");
}

void rk_wifi_module_exit()
{
    rk_command_system("rmmod bcmdhd_indep_power");
}

void rk_wifi_disconnect_ap()
{
    rk_command_system("killall wpa_supplicant");
}

int rk_get_dhcp()
{
    char cmd[128], cmd_results[128];
    int cnt = 60;

    rk_command_system("udhcpc -i wlan0 -t 15 -q");

retry:
    rk_command("ifconfig wlan0 | grep inet | awk '{print $2}' | awk -F: '{print $2}' | awk -F. '{print $1}'", cmd_results, 128);
    if ((cmd_results[0] == 0) && (cmd_results[0] == 127) && (cmd_results[0] == 169)) {
        sleep(1);
        if (cnt--)
            goto retry;

        if (cnt == 0) {
            pr_err("dhcp fail!!!\n");
            return -1;
        }

    }

    return 1;
}

int rk_wifi_get_status(void)
{
    char cmd_results[128];
    int cnt = 60;

retry:
    rk_command("wl status | grep BSSID: | awk '{print $2}'", cmd_results, 128);
    if ((cmd_results[0] == 0) || (strncmp(cmd_results, "00:00:00:00:00:00", 17))) {
        sleep(1);
        if (cnt--)
            goto retry;

        if (cnt == 0) {
            pr_err("status: unknow\n");
            return -1;
        }
    }

    pr_err("status: connected!\n");

    return 1;
}

int rk_wifi_get_channel(void)
{
    char cmd_results[128];

    rk_command("wl status | grep Channel | awk '{print $17}'", cmd_results, 128);

    return cmd_results[0];
}

char *rk_wifi_get_wakeup_reason(void)
{
    char cmd_results[128];

    rk_command("dhd_priv wl wowl_wakeind | grep wakeind | awk '{print $4}' | sed -n '2p' | awk -F= '{print $2}'", cmd_results, 128);

    pr_err("wakeup_reason: %s\n", cmd_results);

    return cmd_results;
}

int rk_wifi_get_signal(void)
{
    char cmd_results[128];
    int signal = 0;

    rk_command("wl status | grep noise | awk '{print $10}'", cmd_results, 128);

    signal = atoi(cmd_results[0]);

    pr_err("signal: %d\n", signal);

    return signal;
}

int rk_wifi_set_net_info(char *ip, char *mask, char *gw, char *dns)
{
    char cmd[128];

    memset(cmd, 0, 128);
    sprintf(cmd, "ifconfig wlan0 %s netmask %s", ip, mask);
    rk_command_system(cmd);

    memset(cmd, 0, 128);
    sprintf(cmd, "route add default gw %s", gw);
    rk_command_system(cmd);

    memset(cmd, 0, 128);
    sprintf(cmd, "echo \"nameserver %s\" > /etc/resolv.conf", dns);
    rk_command_system(cmd);

    return 1;
}

int rk_wifi_set_ssid_for_wakeup(void)
{
    return 1;
}

int rk_wifi_clr_ssid_for_wakeup(void)
{
    return 1;
}

int rk_wifi_start(void)
{

}

int rk_wifi_restart(void)
{

}

void rk_wifi_scan(void)
{

}

int rk_wifi_scan_results(struct wpa_ap_info *pslt, int num)
{

}

void rk_wifi_connect_ap(char *ssid, char *psk)
{
    char cmd[128], cmd_results[128];
    int cnt = 60;

    rk_command_system("ifconfig wlan0 down");
    usleep(500*100);
    rk_command_system("ifconfig wlan0 up");
    usleep(500*100);
    rk_command_system("cp /etc/wpa_supplicant.conf /tmp/ -rf");

    memset(cmd, 0, 128);
    sprintf(cmd, "sed -i \"s/SSID/%s/g\" /tmp/wpa_supplicant.conf", ssid);
    rk_command_system(cmd);

    memset(cmd, 0, 128);
    sprintf(cmd, "sed -i \"s/PASSWORD/%s/g\" /tmp/wpa_supplicant.conf", psk);
    rk_command_system(cmd);

    if (rk_get_pid("wpa_supplicant") > 0)
        rk_kill_task("wpa_supplicant");

    if (rk_run_task("wpa_supplicant", "wpa_supplicant -B -i wlan0 -c /tmp/wpa_supplicant.conf")) {
        pr_err("wpa_supplicant start fail!!!\n");
        return;
    }

    sleep(1);

    rk_wifi_get_status();
    rk_get_dhcp();
}

void rk_wifi_set_tcp_port(int port)
{

}

static char wowl_pattern[256];
void rk_wifi_set_tcp_wakeup_data(int offset, int num, char *data)
{
    int mask;

    for (int i = 0; i < num; i++)
        mask |= (0x1 < i);

    memset(wowl_pattern, 0, 256);

    sprintf(wowl_pattern, "wl_cy wowl_pattern add %d 0x%x %s", offset, mask, data);
}

void rk_wifi_suspend(void)
{
    int fd, size;
    char buf[200];

    fd = open("/proc/tcp_params", O_RDONLY);

    memset(buf, 0, 200);
    lseek(fd, 0, SEEK_SET);
    size = read(fd, buf, 200);

    rk_command_system("wl_cy tcpka_conn_mode 1");
    usleep(200*200);
    pr_info("buf: %s\n", buf);
    rk_command_system(buf);
    usleep(200*200);
    rk_command_system("wl_cy tcpka_conn_enable 1 1 30 1 8");
    usleep(200*200);
    rk_command_system("wl_cy wowl 131094");
    usleep(200*200);
    //printf("wl_cy wowl_pattern add 54 0x07 0x313233\n");
    //rk_command_system("wl_cy wowl_pattern add 54 0x07 0x313233");
    //usleep(200*200);
    //printf("wl_cy wowl_pattern add 66 0x07 0x313233\n");
    //rk_command_system("wl_cy wowl_pattern add 66 0x07 0x313233");
    pr_info("wowl_pattern: %s\n");
    rk_command_system(wowl_pattern);
    usleep(200*200);
    rk_command_system("wl_cy wowl_activate 1");
    usleep(200*200);
    rk_command_system("wl_cy PM 2");
    usleep(200*200);
    rk_command_system("wl_cy hostsleep 1");
    usleep(200*200);
    rk_command_system("killall wpa_supplicant");
}
