/*
 * V4L2 video capture example
 * AUTHOT : Jacob Chen
 * DATA : 2018-02-25
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h> /* getopt_long() */
#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <dlfcn.h>
#include <signal.h>
#include <dirent.h>
#include <inttypes.h>
#if ISPDEMO_ENABLE_DRM
#include "drmDsp.h"
#endif
#include "uAPI2/rk_aiq_user_api2_sysctl.h"
#include "uAPI2/rk_aiq_user_api2_debug.h"
#include "sample_image_process.h"
#include "rkisp_demo.h"
#include <termios.h>

#include "uAPI2/rk_aiq_user_api2_ae.h"

#include "ae_algo_demo/third_party_ae_algo.h"
//#include "awb_algo_demo/third_party_awb_algo.h"  //for rk3588
//#include "awb_algo_demo/third_party_awbV32_algo.h" //for rv1106
#include "awb_algo_demo/third_party_awbV39_algo.h" //for rk3576

#include "af_algo_demo/third_party_af_algo.h"
#if USE_NEWSTRUCT
#include "uAPI2/rk_aiq_user_api2_stats.h"
#endif
#if ISPDEMO_ENABLE_RGA && ISPDEMO_ENABLE_DRM
#include "display.h"
#include "rga.h"
#endif
//#include <list>
//#include <vector>
//#include <string>
//#include <algorithm>

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define FMT_NUM_PLANES 1

#define BUFFER_COUNT 3

#ifdef ANDROID
#define CAPTURE_RAW_PATH "/data"
#define DEFAULT_CAPTURE_RAW_PATH "/data/capture_image"
#else
#define CAPTURE_RAW_PATH "/tmp"
#define DEFAULT_CAPTURE_RAW_PATH "/tmp/capture_image"
#endif
#define CAPTURE_CNT_FILENAME ".capture_cnt"
// #define ENABLE_UAPI_TEST
#define IQFILE_PATH_MAX_LEN 256
// #define CUSTOM_AE_DEMO_TEST
// #define CUSTOM_GROUP_AE_DEMO_TEST
//#define CUSTOM_AWB_DEMO_TEST
// #define TEST_MEMS_SENSOR_INTF
// #define CUSTOM_AF_DEMO_TEST
// #define CUSTOM_PDAF_DEMO_TEST
// #define CUSTOM_GROUP_AWB_DEMO_TEST
// #define OTP_API_TEST
//#define COLOR_CONSISTENCY_TEST
#define LOOP_QUEUE_ONE_RAW_IMAGE
#define DEMO_DISPLAY_TO_HDMI       1
#define DEMO_DISPLAY_TO_DSI        2
#define SOC_SLEEP_STR "mem"
#define SOC_SLEEP_PATH "/sys/power/state"
#define SUSPEND_TIME_REG 0xff300048

#ifdef OTP_API_TEST
static u16 lsc_r_table0[]={
    6167, 5236, 4644, 4030, 3399, 2928, 2638, 2484, 2433, 2482, 2642, 2912, 3391, 4021, 4676, 5324, 6154,
    5718, 4991, 4314, 3578, 2968, 2583, 2336, 2192, 2143, 2193, 2336, 2571, 2967, 3575, 4334, 4989, 5817,
    5156, 4635, 3819, 3058, 2542, 2215, 1985, 1844, 1797, 1844, 1983, 2211, 2540, 3050, 3834, 4650, 5224,
    4774, 4283, 3366, 2681, 2235, 1919, 1697, 1569, 1529, 1569, 1696, 1916, 2227, 2678, 3379, 4317, 4875,
    4578, 3914, 3015, 2416, 1994, 1684, 1482, 1368, 1331, 1368, 1483, 1681, 1993, 2417, 3038, 3967, 4683,
    4334, 3630, 2771, 2226, 1809, 1517, 1328, 1215, 1178, 1217, 1328, 1516, 1813, 2230, 2795, 3679, 4473,
    4157, 3409, 2605, 2087, 1680, 1406, 1218, 1110, 1078, 1109, 1217, 1403, 1684, 2096, 2638, 3467, 4288,
    4053, 3278, 2515, 2002, 1606, 1341, 1155, 1057, 1031, 1056, 1152, 1335, 1609, 2013, 2547, 3333, 4174,
    3998, 3229, 2480, 1973, 1579, 1317, 1133, 1039, 1024, 1040, 1130, 1314, 1586, 1983, 2515, 3290, 4121,
    4037, 3267, 2509, 1996, 1602, 1336, 1151, 1051, 1031, 1054, 1149, 1336, 1612, 2011, 2543, 3333, 4171,
    4143, 3392, 2597, 2079, 1675, 1401, 1213, 1106, 1075, 1105, 1214, 1393, 1670, 2093, 2635, 3469, 4290,
    4301, 3602, 2757, 2215, 1801, 1511, 1323, 1210, 1173, 1211, 1324, 1505, 1789, 2230, 2791, 3673, 4501,
    4538, 3874, 2989, 2402, 1979, 1673, 1474, 1360, 1325, 1364, 1479, 1680, 1997, 2422, 3031, 3956, 4700,
    4719, 4218, 3315, 2653, 2216, 1902, 1682, 1559, 1520, 1561, 1687, 1913, 2236, 2678, 3363, 4320, 4855,
    5007, 4560, 3754, 3008, 2516, 2197, 1961, 1824, 1779, 1827, 1968, 2206, 2537, 3034, 3803, 4635, 5155,
    5501, 4864, 4236, 3504, 2916, 2546, 2305, 2167, 2121, 2169, 2316, 2557, 2937, 3535, 4305, 4943, 5748,
    5954, 5144, 4550, 3930, 3308, 2871, 2598, 2451, 2406, 2462, 2610, 2886, 3337, 3971, 4646, 5325, 5905};

static u16 lsc_b_table0[]={
    5949, 5298, 4628, 4005, 3356, 2878, 2593, 2444, 2393, 2436, 2581, 2834, 3301, 3943, 4587, 5301, 5912,
    5590, 4940, 4298, 3547, 2939, 2545, 2301, 2166, 2120, 2162, 2291, 2512, 2896, 3491, 4245, 4892, 5677,
    5111, 4584, 3783, 3030, 2519, 2193, 1971, 1840, 1793, 1835, 1958, 2174, 2489, 2981, 3744, 4523, 5093,
    4657, 4224, 3336, 2655, 2220, 1912, 1699, 1575, 1536, 1573, 1687, 1894, 2189, 2619, 3290, 4178, 4705,
    4485, 3852, 2984, 2397, 1990, 1689, 1494, 1383, 1344, 1380, 1485, 1671, 1965, 2367, 2954, 3824, 4507,
    4232, 3570, 2743, 2212, 1811, 1529, 1344, 1230, 1193, 1229, 1337, 1515, 1796, 2189, 2721, 3542, 4262,
    4044, 3367, 2583, 2081, 1689, 1421, 1233, 1122, 1087, 1120, 1227, 1408, 1678, 2066, 2575, 3353, 4104,
    3943, 3243, 2501, 2005, 1619, 1356, 1167, 1064, 1035, 1063, 1161, 1343, 1609, 1994, 2498, 3233, 4004,
    3915, 3196, 2469, 1977, 1593, 1332, 1145, 1044, 1024, 1044, 1138, 1323, 1589, 1967, 2468, 3196, 3958,
    3938, 3229, 2492, 2000, 1617, 1352, 1163, 1058, 1033, 1059, 1159, 1344, 1612, 1991, 2493, 3229, 3999,
    4041, 3349, 2572, 2075, 1686, 1415, 1227, 1116, 1080, 1114, 1222, 1399, 1664, 2069, 2578, 3355, 4106,
    4217, 3550, 2729, 2202, 1804, 1523, 1336, 1222, 1183, 1221, 1332, 1504, 1777, 2192, 2723, 3545, 4282,
    4464, 3811, 2955, 2381, 1975, 1678, 1485, 1373, 1335, 1373, 1482, 1670, 1971, 2376, 2949, 3811, 4513,
    4621, 4169, 3283, 2631, 2207, 1898, 1685, 1567, 1530, 1567, 1684, 1893, 2205, 2626, 3273, 4159, 4681,
    4970, 4500, 3715, 2986, 2498, 2176, 1954, 1824, 1782, 1824, 1951, 2169, 2488, 2967, 3704, 4495, 5032,
    5506, 4850, 4228, 3481, 2889, 2516, 2281, 2150, 2110, 2152, 2281, 2508, 2877, 3451, 4200, 4835, 5585,
    5891, 5222, 4541, 3908, 3270, 2828, 2566, 2430, 2388, 2429, 2565, 2819, 3259, 3870, 4536, 5274, 5746};

static u16 lsc_gr_table0[]={
    6101, 5408, 4698, 4130, 3467, 2989, 2699, 2556, 2504, 2551, 2692, 2969, 3446, 4120, 4735, 5415, 6134,
    5591, 5123, 4414, 3671, 3048, 2636, 2387, 2252, 2204, 2248, 2380, 2623, 3027, 3647, 4427, 5130, 5659,
    5321, 4695, 3915, 3138, 2611, 2264, 2023, 1887, 1840, 1883, 2018, 2252, 2598, 3121, 3909, 4694, 5397,
    4845, 4335, 3453, 2757, 2295, 1958, 1723, 1596, 1556, 1594, 1719, 1947, 2275, 2742, 3450, 4351, 4926,
    4558, 4002, 3100, 2487, 2045, 1713, 1504, 1387, 1349, 1385, 1499, 1703, 2031, 2478, 3098, 4013, 4644,
    4403, 3703, 2857, 2298, 1855, 1544, 1346, 1229, 1191, 1228, 1341, 1533, 1845, 2286, 2858, 3722, 4466,
    4250, 3505, 2702, 2158, 1723, 1430, 1233, 1119, 1084, 1116, 1226, 1418, 1716, 2152, 2710, 3518, 4314,
    4139, 3394, 2618, 2079, 1650, 1364, 1167, 1064, 1034, 1059, 1160, 1350, 1645, 2079, 2631, 3412, 4198,
    4100, 3351, 2586, 2052, 1626, 1341, 1145, 1044, 1024, 1042, 1137, 1330, 1622, 2049, 2599, 3371, 4154,
    4132, 3388, 2611, 2075, 1648, 1361, 1163, 1058, 1034, 1058, 1158, 1353, 1648, 2074, 2626, 3406, 4198,
    4231, 3493, 2695, 2152, 1720, 1427, 1229, 1117, 1081, 1113, 1223, 1412, 1704, 2154, 2713, 3517, 4305,
    4405, 3683, 2846, 2290, 1849, 1540, 1343, 1226, 1187, 1223, 1338, 1524, 1826, 2288, 2860, 3717, 4456,
    4550, 3964, 3077, 2477, 2033, 1708, 1499, 1384, 1346, 1383, 1498, 1704, 2037, 2481, 3097, 4002, 4654,
    4769, 4285, 3407, 2735, 2282, 1946, 1714, 1591, 1554, 1592, 1716, 1949, 2292, 2743, 3428, 4331, 4911,
    5196, 4611, 3841, 3095, 2588, 2248, 2007, 1871, 1830, 1873, 2013, 2249, 2594, 3100, 3869, 4678, 5294,
    5548, 4992, 4331, 3583, 2987, 2604, 2360, 2227, 2185, 2228, 2362, 2603, 2991, 3592, 4366, 5060, 5567,
    5963, 5327, 4617, 4016, 3366, 2926, 2658, 2525, 2483, 2525, 2660, 2926, 3377, 4032, 4668, 5422, 5923};

static u16 lsc_gb_table0[]={
    6092, 5336, 4693, 4135, 3480, 3007, 2722, 2576, 2529, 2571, 2715, 2993, 3463, 4119, 4731, 5377, 6112,
    5545, 5071, 4390, 3668, 3051, 2650, 2403, 2265, 2217, 2262, 2396, 2634, 3035, 3644, 4401, 5106, 5640,
    5217, 4641, 3892, 3130, 2611, 2270, 2031, 1893, 1845, 1889, 2025, 2254, 2595, 3108, 3881, 4662, 5352,
    4792, 4289, 3422, 2742, 2293, 1960, 1728, 1598, 1557, 1596, 1721, 1944, 2267, 2722, 3417, 4312, 4896,
    4490, 3955, 3069, 2470, 2037, 1712, 1505, 1387, 1348, 1384, 1499, 1698, 2019, 2455, 3069, 3976, 4611,
    4351, 3662, 2828, 2279, 1847, 1543, 1346, 1229, 1190, 1226, 1338, 1527, 1832, 2263, 2830, 3685, 4440,
    4205, 3461, 2678, 2143, 1717, 1428, 1233, 1118, 1082, 1114, 1224, 1413, 1703, 2131, 2680, 3489, 4287,
    4099, 3356, 2594, 2066, 1648, 1365, 1168, 1064, 1033, 1058, 1158, 1346, 1634, 2058, 2602, 3382, 4169,
    4051, 3322, 2567, 2042, 1625, 1343, 1147, 1045, 1024, 1041, 1136, 1326, 1612, 2029, 2572, 3346, 4135,
    4092, 3353, 2592, 2065, 1645, 1363, 1166, 1058, 1033, 1057, 1156, 1350, 1637, 2056, 2599, 3382, 4168,
    4190, 3450, 2672, 2139, 1716, 1427, 1230, 1117, 1081, 1111, 1222, 1407, 1692, 2133, 2683, 3484, 4275,
    4338, 3639, 2816, 2272, 1842, 1539, 1343, 1225, 1184, 1221, 1335, 1518, 1813, 2266, 2826, 3677, 4437,
    4472, 3917, 3044, 2459, 2026, 1705, 1500, 1383, 1344, 1381, 1496, 1699, 2024, 2457, 3059, 3957, 4595,
    4727, 4233, 3374, 2719, 2275, 1948, 1718, 1594, 1555, 1593, 1717, 1946, 2279, 2720, 3394, 4287, 4851,
    5116, 4566, 3811, 3079, 2586, 2255, 2016, 1879, 1835, 1879, 2017, 2253, 2589, 3083, 3835, 4627, 5229,
    5480, 4943, 4289, 3571, 2989, 2615, 2376, 2241, 2198, 2243, 2378, 2614, 2994, 3583, 4337, 5020, 5555,
    5921, 5252, 4589, 4005, 3376, 2943, 2675, 2541, 2502, 2545, 2677, 2948, 3388, 4035, 4654, 5390, 5904};

static u16 lsc_table1[]={
    5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120,
    5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120,
    5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120,
    5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120,
    5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120,
    5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120,
    5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120,
    5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120,
    5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120,
    5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120,
    5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120,
    5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120,
    5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120,
    5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120,
    5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120,
    5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120,
    5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120, 5120};

#endif

#ifdef ISPFEC_API
#include "IspFec/rk_ispfec_api.h"
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libdrm/drm_mode.h>
#include <drm_fourcc.h>

struct drm_buf {
    int fb_id;
    uint32_t handle;
    uint32_t size;
    uint32_t pitch;
    char *map;
    int dmabuf_fd;
};

static rk_ispfec_ctx_t* g_ispfec_ctx = NULL;
static rk_ispfec_cfg_t g_ispfec_cfg;
struct drm_buf g_drm_buf_pic_out;
struct drm_buf g_drm_buf_xint;
struct drm_buf g_drm_buf_xfra;
struct drm_buf g_drm_buf_yint;
struct drm_buf g_drm_buf_yfra;
#endif

struct buffer {
    void *start;
    size_t length;
    int export_fd;
    int sequence;
};

enum TEST_CTL_TYPE {
    TEST_CTL_TYPE_DEFAULT,
    TEST_CTL_TYPE_REPEAT_INIT_PREPARE_START_STOP_DEINIT,
    TEST_CTL_TYPE_REPEAT_START_STOP,
    TEST_CTL_TYPE_REPEAT_PREPARE_START_STOP,
    TEST_CTL_TYPE_REPEAT_SWITCHSCENE_PREPARE_START_STOP,
};

static struct termios oldt;
static int silent;
static demo_context_t *g_main_ctx = NULL,  *g_second_ctx = NULL, *g_third_ctx = NULL, *g_fourth_ctx = NULL, *g_fifth_ctx = NULL;
static bool _if_quit = false;

#ifdef ISPFEC_API
int alloc_drm_buffer(int fd, int width, int height,
                     int bpp, struct drm_buf *buf)
{
    struct drm_mode_create_dumb alloc_arg;
    struct drm_mode_map_dumb mmap_arg;
    struct drm_mode_destroy_dumb destory_arg;
    void *map;
    int ret;

    memset(&alloc_arg, 0, sizeof(alloc_arg));
    alloc_arg.bpp = bpp;
    alloc_arg.width = width;
    alloc_arg.height = height;

    ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &alloc_arg);
    if (ret) {
        printf("failed to create dumb buffer\n");
        return ret;
    }

    memset(&mmap_arg, 0, sizeof(mmap_arg));
    mmap_arg.handle = alloc_arg.handle;
    ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mmap_arg);
    if (ret) {
        printf("failed to create map dumb\n");
        ret = -EINVAL;
        goto destory_dumb;
    }
    map = mmap(0, alloc_arg.size,
               PROT_READ | PROT_WRITE, MAP_SHARED,
               fd, mmap_arg.offset);
    if (map == MAP_FAILED) {
        printf("failed to mmap buffer\n");
        ret = -EINVAL;
        goto destory_dumb;
    }
    ret = drmPrimeHandleToFD(fd, alloc_arg.handle, 0,
                             &buf->dmabuf_fd);
    if (ret) {
        printf("failed to get dmabuf fd\n");
        munmap(map, alloc_arg.size);
        ret = -EINVAL;
        goto destory_dumb;
    }
    buf->size = alloc_arg.size;
    buf->map = (char*)map;

destory_dumb:
    memset(&destory_arg, 0, sizeof(destory_arg));
    destory_arg.handle = alloc_arg.handle;
    drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destory_arg);
    return ret;
}

int free_drm_buffer(int fd, struct drm_buf *buf)
{
    if (buf) {
        close(buf->dmabuf_fd);
        return munmap(buf->map, buf->size);
    }
    return -EINVAL;
}

int init_ispfec_bufs(rk_ispfec_cfg_t* cfg)
{
    int ret = 0;
    int drm_fd = drmOpen("rockchip", NULL);
    if (drm_fd < 0) {
        printf("failed to open rockchip drm\n");
        return -1;
    }

    int mesh_size = rk_ispfec_api_calFecMeshsize(cfg->in_width, cfg->in_height);

    printf("\nmesh_size:%d\n", mesh_size);
    ret = alloc_drm_buffer(drm_fd, mesh_size * 2, 1, 8, &g_drm_buf_xint);
    if (ret)
        goto close_drm_fd;
    printf("xint fd:%d size:%d\n", g_drm_buf_xint.dmabuf_fd, g_drm_buf_xint.size);

    ret = alloc_drm_buffer(drm_fd, mesh_size, 1, 8, &g_drm_buf_xfra);
    if (ret)
        goto free_drm_buf_xint;
    printf("xfra fd:%d size:%d\n", g_drm_buf_xfra.dmabuf_fd, g_drm_buf_xfra.size);

    ret = alloc_drm_buffer(drm_fd, mesh_size * 2, 1, 8, &g_drm_buf_yint);
    if (ret)
        goto free_drm_buf_xfra;
    printf("yint fd:%d size:%d\n", g_drm_buf_yint.dmabuf_fd, g_drm_buf_yint.size);

    ret = alloc_drm_buffer(drm_fd, mesh_size, 1, 8, &g_drm_buf_yfra);
    if (ret)
        goto free_drm_buf_yint;
    printf("yfra fd:%d size:%d\n", g_drm_buf_yfra.dmabuf_fd, g_drm_buf_yfra.size);

    ret = alloc_drm_buffer(drm_fd, cfg->out_width, cfg->out_height * 3 / 2, 8, &g_drm_buf_pic_out);
    if (ret)
        goto free_drm_buf_yfra;
    printf("out pic fd:%d size:%d\n", g_drm_buf_pic_out.dmabuf_fd, g_drm_buf_pic_out.size);

    cfg->mesh_xint.dmaFd = g_drm_buf_xint.dmabuf_fd;
    cfg->mesh_xint.size = g_drm_buf_xint.size;
    cfg->mesh_xint.vir_addr = g_drm_buf_xint.map;

    cfg->mesh_xfra.dmaFd = g_drm_buf_xfra.dmabuf_fd;
    cfg->mesh_xfra.size = g_drm_buf_xfra.size;
    cfg->mesh_xfra.vir_addr = g_drm_buf_xfra.map;

    cfg->mesh_yint.dmaFd = g_drm_buf_yint.dmabuf_fd;
    cfg->mesh_yint.size = g_drm_buf_yint.size;
    cfg->mesh_yint.vir_addr = g_drm_buf_yint.map;

    cfg->mesh_yfra.dmaFd = g_drm_buf_yfra.dmabuf_fd;
    cfg->mesh_yfra.size = g_drm_buf_yfra.size;
    cfg->mesh_yfra.vir_addr = g_drm_buf_yfra.map;

    goto close_drm_fd;

free_drm_buf_pic_out:
    free_drm_buffer(drm_fd, &g_drm_buf_pic_out);
free_drm_buf_yfra:
    free_drm_buffer(drm_fd, &g_drm_buf_yfra);
free_drm_buf_yint:
    free_drm_buffer(drm_fd, &g_drm_buf_yfra);
free_drm_buf_xfra:
    free_drm_buffer(drm_fd, &g_drm_buf_xfra);
free_drm_buf_xint:
    free_drm_buffer(drm_fd, &g_drm_buf_xint);
close_drm_fd:
    close(drm_fd);

    return ret;
}

void deinit_ispfec_bufs()
{
    free_drm_buffer(-1, &g_drm_buf_pic_out);
    free_drm_buffer(-1, &g_drm_buf_yfra);
    free_drm_buffer(-1, &g_drm_buf_yfra);
    free_drm_buffer(-1, &g_drm_buf_xfra);
    free_drm_buffer(-1, &g_drm_buf_xint);
}
#endif

//restore terminal settings
void restore_terminal_settings(void)
{
    // Apply saved settings
    tcsetattr(0, TCSANOW, &oldt);
}

//make terminal read 1 char at a time
void disable_terminal_return(void)
{
    struct termios newt;

    //save terminal settings
    tcgetattr(0, &oldt);
    //init new settings
    newt = oldt;
    //change settings
    newt.c_lflag &= ~(ICANON | ECHO);
    //apply settings
    tcsetattr(0, TCSANOW, &newt);

    //make sure settings will be restored when program ends
    atexit(restore_terminal_settings);
}

int getsubopt(char** opt, char* const* keys, char** val) {
    char* s = *opt;
    int i;

    *val = NULL;
    *opt = strchr(s, ',');
    if (*opt)
        *(*opt)++ = 0;
    else
        *opt = s + strlen(s);

    for (i = 0; keys[i]; i++) {
        size_t l = strlen(keys[i]);
        if (strncmp(keys[i], s, l)) continue;
        if (s[l] == '=')
            *val = s + l + 1;
        else if (s[l])
            continue;
        return i;
    }
    return -1;
}

int parse_subopt(char** subs, const char* const* subopts, char** value) {
    int opt = getsubopt(subs, (char* const*)(subopts), value);

    if (opt == -1) {
        fprintf(stderr, "Invalid suboptions specified\n");
        return -1;
    }
    if (*value == NULL) {
        fprintf(stderr, "No value given to suboption <%s>\n", subopts[opt]);
        return -1;
    }
    return opt;
}

int parse_iq_scene_val(char* optarg, char main_scene[32], char sub_scene[32]) {
    char *value, *subs;

    subs = optarg;
    while (*subs != '\0') {
        static const char* subopts[] = {"main_scene", "sub_scene", NULL};
        size_t len;

        switch (parse_subopt(&subs, subopts, &value)) {
            case 0:
                len = strlen(value);
                if (len == 0 || len > 32) {
                    ERR("Invalid main_scene value: %s", value);
                    return -1;
                }
                memcpy(main_scene, value, len);
                main_scene[len] = '\0';
                break;
            case 1:
                len = strlen(value);
                if (len == 0 || len > 32) {
                    ERR("Invalid sub_scene value: %s", value);
                    return -1;
                }
                memcpy(sub_scene, value, len);
                sub_scene[len] = '\0';
                break;
            default:
                ERR("Invalid scene option: %s", value);
                return -1;
        }
    }
    return 0;
}

int parse_switch_scene_val(char* optarg, char main_scene[32], char sub_scene[32],
                           char main_scene1[32], char sub_scene1[32]) {
    char *value, *subs;

    subs = optarg;
    while (*subs != '\0') {
        static const char* subopts[] = {"main_scene", "sub_scene", "main_scene1", "sub_scene1", NULL};
        size_t len;

        switch (parse_subopt(&subs, subopts, &value)) {
            case 0:
                len = strlen(value);
                if (len == 0 || len > 32) {
                    ERR("Invalid main_scene value: %s", value);
                    return -1;
                }
                memcpy(main_scene, value, len);
                main_scene[len] = '\0';
                break;
            case 1:
                len = strlen(value);
                if (len == 0 || len > 32) {
                    ERR("Invalid sub_scene value: %s", value);
                    return -1;
                }
                memcpy(sub_scene, value, len);
                sub_scene[len] = '\0';
                break;
            case 2:
                len = strlen(value);
                if (len == 0 || len > 32) {
                    ERR("Invalid main_scene1 value: %s", value);
                    return -1;
                }
                memcpy(main_scene1, value, len);
                main_scene1[len] = '\0';
                break;
            case 3:
                len = strlen(value);
                if (len == 0 || len > 32) {
                    ERR("Invalid sub_scene1 value: %s", value);
                    return -1;
                }
                memcpy(sub_scene1, value, len);
                sub_scene1[len] = '\0';
                break;
            default:
                ERR("Invalid scene option: %s", value);
                return -1;
        }
    }
    return 0;
}

char* get_dev_name(demo_context_t* ctx)
{
    if (ctx->dev_using == 1)
        return ctx->dev_name;
    else if (ctx->dev_using == 2)
        return ctx->dev_name2;
    else if (ctx->dev_using == 3)
        return ctx->dev_name3;
    else if (ctx->dev_using == 4)
        return ctx->dev_name4;
    else if (ctx->dev_using == 5)
        return ctx->dev_name5;
    else {
        ERR("!!!dev_using is not supported!!!");
        return NULL;
    }
}

char* get_sensor_name(demo_context_t* ctx)
{
    return ctx->sns_name;
}

void test_update_iqfile(const demo_context_t* demo_ctx)
{
    char iqfile[IQFILE_PATH_MAX_LEN] = {0};

    printf("\nspecial an new iqfile:\n");
    strcat(iqfile, demo_ctx->iqpath);
    strcat(iqfile, "/");
    if (fgets(iqfile + strlen(iqfile), IQFILE_PATH_MAX_LEN, stdin) != NULL) {
        char* json = strstr(iqfile, "json");

        if (!json) {
            printf("[AIQ]input is not an valide json:%s\n", iqfile);
            return;
        }

        /* fgets may add '\n' in the end of input, delete it */
        json += strlen("json");
        *json = '\0';

        printf("[AIQ] appling new iq file:%s\n", iqfile);

        rk_aiq_uapi2_sysctl_updateIq(demo_ctx->aiq_ctx, iqfile);
    }
}

#if 0
static int set_ae_onoff(const rk_aiq_sys_ctx_t* ctx, bool onoff);
void test_imgproc(const demo_context_t* demo_ctx) {

    if (demo_ctx == NULL) {
        return;
    }

    const rk_aiq_sys_ctx_t* ctx = (const rk_aiq_sys_ctx_t*)(demo_ctx->aiq_ctx);

    /*TODO: when rkaiq_3A_server & rkisp_demo run in two different shell, rk_aiq_sys_ctx_t would be null?*/
    if (ctx == NULL) {
        printf("ERROR : rk_aiq_sys_ctx_t is null.\n");
        _if_quit = true;
        return;
    }

    int key = getchar();
    printf("press key=[%c]\n", key);

    opMode_t mode;
    paRange_t range;
    expPwrLineFreq_t freq;
    rk_aiq_wb_scene_t scene;
    rk_aiq_wb_gain_t gain;
    rk_aiq_wb_cct_t ct;
    antiFlickerMode_t flicker;
    switch (key)
    {
    case '0':
        rk_aiq_uapi_setExpMode(ctx, OP_MANUAL);
        printf("set exp manual\n");
        break;
    case '.':
        rk_aiq_uapi_setExpMode(ctx, OP_AUTO);
        printf("set exp auto\n");
        break;
    case '1':
        rk_aiq_uapi_getExpMode(ctx, &mode);
        printf("exp mode=%d\n", mode);
        break;
    case '2':
        range.min = 5.0f;
        range.max = 8.0f;
        rk_aiq_uapi_setExpGainRange(ctx, &range);
        printf("set gain range\n");
        break;
    case '3':
        rk_aiq_uapi_getExpGainRange(ctx, &range);
        printf("get gain range[%f,%f]\n", range.min, range.max);
        break;
    case '4':
        range.min = 10.0f;
        range.max = 30.0f;
        rk_aiq_uapi_setExpTimeRange(ctx, &range);
        printf("set time range\n");
        break;
    case '5':
        rk_aiq_uapi_getExpTimeRange(ctx, &range);
        printf("get time range[%f,%f]\n", range.min, range.max);
        break;
    case '6':
        rk_aiq_uapi_setExpPwrLineFreqMode(ctx, EXP_PWR_LINE_FREQ_50HZ);
        printf("setExpPwrLineFreqMode 50hz\n");
        break;
    case ',':
        rk_aiq_uapi_setExpPwrLineFreqMode(ctx, EXP_PWR_LINE_FREQ_60HZ);
        printf("setExpPwrLineFreqMode 60hz\n");
        break;
    case '7':
        rk_aiq_uapi_getExpPwrLineFreqMode(ctx, &freq);
        printf("getExpPwrLineFreqMode=%d\n", freq);
        break;
    case '8':
        rk_aiq_uapi_setWBMode(ctx, OP_MANUAL);
        printf("setWBMode manual\n");
        break;
    case '/':
        rk_aiq_uapi_setWBMode(ctx, OP_AUTO);
        printf("setWBMode auto\n");
        break;
    case '9':
        rk_aiq_uapi_getWBMode(ctx, &mode);
        printf("getWBMode=%d\n", mode);
        break;
    case 'a':
        rk_aiq_uapi_lockAWB(ctx);
        printf("lockAWB\n");
        break;
    case 'b':
        rk_aiq_uapi_unlockAWB(ctx);
        printf("unlockAWB\n");
        break;
    case 'c':
        rk_aiq_uapi_setMWBScene(ctx, RK_AIQ_WBCT_TWILIGHT);
        printf("setMWBScene\n");
        break;
    case 'd':
        rk_aiq_uapi_getMWBScene(ctx, &scene);
        printf("getMWBScene=%d\n", scene);
        break;
    case 'e':
        gain.rgain = 0.5f;
        gain.grgain = 0.5f;
        gain.gbgain = 0.5f;
        gain.bgain = 0.5f;
        rk_aiq_uapi_setMWBGain(ctx, &gain);
        printf("setMWBGain\n");
        break;
    case 'f':
        rk_aiq_uapi_getMWBGain(ctx, &gain);
        printf("getMWBGain=[%f %f %f %f]\n", gain.rgain, gain.grgain, gain.gbgain, gain.bgain);
        break;
    case 'g':
        break;
    case 'h':
        break;
    case 'i':
        rk_aiq_uapi_setAntiFlickerMode(ctx, ANTIFLICKER_NORMAL_MODE);
        printf("setAntiFlickerMode normal\n");
        break;
    case 'j':
        rk_aiq_uapi_setAntiFlickerMode(ctx, ANTIFLICKER_AUTO_MODE);
        printf("setAntiFlickerMode auto\n");
        break;
    case 'k':
        rk_aiq_uapi_getAntiFlickerMode(ctx, &flicker);
        printf("getAntiFlickerMode=%d\n", flicker);
        break;
    case 'l':
        rk_aiq_uapi_setSaturation(ctx, 50);
        printf("setSaturation\n");
        break;
    case 'm':
        unsigned int level1;
        rk_aiq_uapi_getSaturation(ctx, &level1);
        printf("getSaturation=%d\n", level1);
        break;
    case 'n':
        rk_aiq_uapi_setCrSuppsn(ctx, 50);
        printf("setCrSuppsn\n");
        break;
    case 'o':
        unsigned int level2;
        rk_aiq_uapi_getCrSuppsn(ctx, &level2);
        printf("getCrSuppsn=%d\n", level2);
        break;
    case 'p':
        //rk_aiq_uapi_setHDRMode(ctx, OP_AUTO);
        printf("setHDRMode\n");
        break;
    case 'q':
        //rk_aiq_uapi_setHDRMode(ctx, OP_MANUAL);
        printf("setHDRMode\n");
        break;
    case 'r':
        //rk_aiq_uapi_getHDRMode(ctx, &mode);
        printf("getHDRMode=%d\n", mode);
        break;
    case 's': {
        unsigned int set_anr_strength = 80;
        unsigned int get_anr_strength = 0;
        rk_aiq_uapi_setANRStrth(ctx, set_anr_strength);
        printf("setANRStrth %u \n", set_anr_strength);
        sleep(1);
        rk_aiq_uapi_getANRStrth(ctx, &get_anr_strength);
        printf("getANRStrth %u \n", get_anr_strength);
        break;
    }
    case 't': {
        unsigned int set_strength = 80;
        unsigned int get_space_strength = 0;
        unsigned int get_mfnr_strength = 0;
        bool state;
        rk_aiq_uapi_setMSpaNRStrth(ctx, true, set_strength);
        rk_aiq_uapi_setMTNRStrth(ctx, true, set_strength);
        printf("setMSpaNRStrth and setMTNRStrth :%u \n", set_strength);
        rk_aiq_uapi_getMSpaNRStrth(ctx, &state, &get_space_strength);
        rk_aiq_uapi_getMTNRStrth(ctx, &state, &get_mfnr_strength);
        printf("setMSpaNRStrth and setMTNRStrth :%u %u\n", get_space_strength, get_mfnr_strength);
        break;
    }
    case 'u':
        //rk_aiq_uapi_setDhzMode(ctx, OP_MANUAL);
        //printf("setDhzMode\n");
        break;
    case 'v':
        //rk_aiq_uapi_getDhzMode(ctx, &mode);
        // printf("getDhzMode=%d\n", mode);
        break;
    case 'w':
    {
        bool stat = false;
        //unsigned int level4 = 0;
        //rk_aiq_uapi_getMHDRStrth(ctx, &stat, &level4);
        //printf("getMHDRStrth: status:%d, level=%d\n", stat, level4);
    }
    break;
    case 'x':
        //rk_aiq_uapi_setMHDRStrth(ctx, true, 8);
        //printf("setMHDRStrth true\n");
        break;
    case 'y':
    {
        bool mod_en;
        rk_aiq_uapi2_sysctl_getModuleCtl(ctx, RK_MODULE_TNR, &mod_en);
        printf("getModuleCtl=%d\n", mod_en);
        if (mod_en) {
            rk_aiq_uapi2_sysctl_setModuleCtl(ctx, RK_MODULE_TNR, false);
        } else {
            rk_aiq_uapi2_sysctl_setModuleCtl(ctx, RK_MODULE_TNR, true);
        }
    }
    break;
    case 'z':
        rk_aiq_uapi_setFocusMode(ctx, OP_AUTO);
        printf("setFocusMode OP_AUTO\n");
        break;
    case 'A':
        rk_aiq_uapi_setFocusMode(ctx, OP_SEMI_AUTO);
        printf("setFocusMode OP_SEMI_AUTO\n");
        break;
    case 'B':
        rk_aiq_uapi_setFocusMode(ctx, OP_MANUAL);
        printf("setFocusMode OP_MANUAL\n");
        break;
    case 'C':
        rk_aiq_uapi_manualTrigerFocus(ctx);
        printf("manualTrigerFocus\n");
        break;
    case 'D': {
        rk_aiq_af_zoomrange range;
        int code;

        rk_aiq_uapi_getZoomRange(ctx, &range);
        printf("zoom.min_pos %d, zoom.max_pos %d\n", range.min_pos, range.max_pos);

        rk_aiq_uapi_getOpZoomPosition(ctx, &code);
        printf("getOpZoomPosition code %d\n", code);

        code += 20;
        if (code > range.max_pos)
            code = range.max_pos;
        rk_aiq_uapi_setOpZoomPosition(ctx, code);
        rk_aiq_uapi_endOpZoomChange(ctx);
        printf("setOpZoomPosition %d\n", code);
    }
    break;
    case 'E': {
        rk_aiq_af_zoomrange range;
        int code;

        rk_aiq_uapi_getZoomRange(ctx, &range);
        printf("zoom.min_pos %d, zoom.max_pos %d\n", range.min_pos, range.max_pos);

        rk_aiq_uapi_getOpZoomPosition(ctx, &code);
        printf("getOpZoomPosition code %d\n", code);

        code -= 20;
        if (code < range.min_pos)
            code = range.min_pos;
        rk_aiq_uapi_setOpZoomPosition(ctx, code);
        rk_aiq_uapi_endOpZoomChange(ctx);
        printf("setOpZoomPosition %d\n", code);
    }
    break;
    case 'F': {
        rk_aiq_af_focusrange range;
        short code;

        rk_aiq_uapi_getFocusRange(ctx, &range);
        printf("focus.min_pos %d, focus.max_pos %d\n", range.min_pos, range.max_pos);

        rk_aiq_uapi_getFixedModeCode(ctx, &code);

        code++;
        if (code > range.max_pos)
            code = range.max_pos;
        rk_aiq_uapi_setFixedModeCode(ctx, code);
        printf("setFixedModeCode %d\n", code);
    }
    break;
    case 'G': {
        rk_aiq_af_focusrange range;
        short code;

        rk_aiq_uapi_getFocusRange(ctx, &range);
        printf("focus.min_pos %d, focus.max_pos %d\n", range.min_pos, range.max_pos);

        rk_aiq_uapi_getFixedModeCode(ctx, &code);

        code--;
        if (code < range.min_pos)
            code = range.min_pos;
        rk_aiq_uapi_setFixedModeCode(ctx, code);
        printf("setFixedModeCode %d\n", code);
    }
    break;
    case 'H': {
        rk_aiq_af_attrib_t attr;
        uint16_t gamma_y[RKAIQ_RAWAF_GAMMA_NUM] =
        {0, 45, 108, 179, 245, 344, 409, 459, 500, 567, 622, 676, 759, 833, 896, 962, 1023};

        rk_aiq_user_api_af_GetAttrib(ctx, &attr);
        attr.manual_meascfg.contrast_af_en = 1;
        attr.manual_meascfg.rawaf_sel = 0; // normal = 0; hdr = 1

        attr.manual_meascfg.window_num = 2;
        attr.manual_meascfg.wina_h_offs = 2;
        attr.manual_meascfg.wina_v_offs = 2;
        attr.manual_meascfg.wina_h_size = 2580;
        attr.manual_meascfg.wina_v_size = 1935;

        attr.manual_meascfg.winb_h_offs = 500;
        attr.manual_meascfg.winb_v_offs = 600;
        attr.manual_meascfg.winb_h_size = 300;
        attr.manual_meascfg.winb_v_size = 300;

        attr.manual_meascfg.gamma_flt_en = 1;
        memcpy(attr.manual_meascfg.gamma_y, gamma_y, RKAIQ_RAWAF_GAMMA_NUM * sizeof(uint16_t));

        attr.manual_meascfg.gaus_flt_en = 1;
        attr.manual_meascfg.gaus_h0 = 0x20;
        attr.manual_meascfg.gaus_h1 = 0x10;
        attr.manual_meascfg.gaus_h2 = 0x08;

        attr.manual_meascfg.afm_thres = 4;

        attr.manual_meascfg.lum_var_shift[0] = 0;
        attr.manual_meascfg.afm_var_shift[0] = 0;
        attr.manual_meascfg.lum_var_shift[1] = 4;
        attr.manual_meascfg.afm_var_shift[1] = 4;

        attr.manual_meascfg.sp_meas.enable = true;
        attr.manual_meascfg.sp_meas.ldg_xl = 10;
        attr.manual_meascfg.sp_meas.ldg_yl = 28;
        attr.manual_meascfg.sp_meas.ldg_kl = (255 - 28) * 256 / 45;
        attr.manual_meascfg.sp_meas.ldg_xh = 118;
        attr.manual_meascfg.sp_meas.ldg_yh = 8;
        attr.manual_meascfg.sp_meas.ldg_kh = (255 - 8) * 256 / 15;
        attr.manual_meascfg.sp_meas.highlight_th = 245;
        attr.manual_meascfg.sp_meas.highlight2_th = 200;
        rk_aiq_user_api_af_SetAttrib(ctx, &attr);
    }
    break;
    case 'I':
        if (CHECK_ISP_HW_V20()) {
            rk_aiq_nr_IQPara_t stNRIQPara;
            rk_aiq_nr_IQPara_t stGetNRIQPara;
            stNRIQPara.module_bits = (1 << ANR_MODULE_BAYERNR) | (1 << ANR_MODULE_MFNR) | (1 << ANR_MODULE_UVNR) | (1 << ANR_MODULE_YNR);
            stGetNRIQPara.module_bits = (1 << ANR_MODULE_BAYERNR) | (1 << ANR_MODULE_MFNR) | (1 << ANR_MODULE_UVNR) | (1 << ANR_MODULE_YNR);
            rk_aiq_user_api_anr_GetIQPara(ctx, &stNRIQPara);
            for(int m = 0; m < 3; m++) {
                for(int k = 0; k < 2; k++) {
                    for(int i = 0; i < CALIBDB_NR_SHARP_MAX_ISO_LEVEL; i++ ) {
                        //bayernr
                        stNRIQPara.stBayernrPara.mode_cell[m].setting[k].filtPara[i] = 0.1;
                        stNRIQPara.stBayernrPara.mode_cell[m].setting[k].lamda = 500;
                        stNRIQPara.stBayernrPara.mode_cell[m].setting[k].fixW[0][i] = 0.1;
                        stNRIQPara.stBayernrPara.mode_cell[m].setting[k].fixW[1][i] = 0.1;
                        stNRIQPara.stBayernrPara.mode_cell[m].setting[k].fixW[2][i] = 0.1;
                        stNRIQPara.stBayernrPara.mode_cell[m].setting[k].fixW[3][i] = 0.1;

                        //mfnr
                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].weight_limit_y[0] = 2;
                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].weight_limit_y[1] = 2;
                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].weight_limit_y[2] = 2;
                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].weight_limit_y[3] = 2;

                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].weight_limit_uv[0] = 2;
                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].weight_limit_uv[1] = 2;
                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].weight_limit_uv[2] = 2;

                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].y_lo_bfscale[0] = 0.4;
                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].y_lo_bfscale[1] = 0.6;
                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].y_lo_bfscale[2] = 0.8;
                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].y_lo_bfscale[3] = 1.0;

                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].y_hi_bfscale[0] = 0.4;
                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].y_hi_bfscale[1] = 0.6;
                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].y_hi_bfscale[2] = 0.8;
                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].y_hi_bfscale[3] = 1.0;

                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].uv_lo_bfscale[0] = 0.1;
                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].uv_lo_bfscale[1] = 0.2;
                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].uv_lo_bfscale[2] = 0.3;

                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].uv_hi_bfscale[0] = 0.1;
                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].uv_hi_bfscale[1] = 0.2;
                        stNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].uv_hi_bfscale[2] = 0.3;

                        //ynr
                        stNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].lo_bfScale[0] = 0.4;
                        stNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].lo_bfScale[1] = 0.6;
                        stNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].lo_bfScale[2] = 0.8;
                        stNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].lo_bfScale[3] = 1.0;

                        stNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].hi_bfScale[0] = 0.4;
                        stNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].hi_bfScale[1] = 0.6;
                        stNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].hi_bfScale[2] = 0.8;
                        stNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].hi_bfScale[3] = 1.0;

                        stNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].hi_denoiseStrength = 1.0;

                        stNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].hi_denoiseWeight[0] = 1.0;
                        stNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].hi_denoiseWeight[1] = 1.0;
                        stNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].hi_denoiseWeight[2] = 1.0;
                        stNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].hi_denoiseWeight[3] = 1.0;

                        stNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].denoise_weight[0] = 1.0;
                        stNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].denoise_weight[1] = 1.0;
                        stNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].denoise_weight[2] = 1.0;
                        stNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].denoise_weight[3] = 1.0;

                        //uvnr
                        stNRIQPara.stUvnrPara.mode_cell[m].setting[k].step0_uvgrad_ratio[i] = 100;
                        stNRIQPara.stUvnrPara.mode_cell[m].setting[k].step1_median_ratio[i] = 0.5;
                        stNRIQPara.stUvnrPara.mode_cell[m].setting[k].step2_median_ratio[i] = 0.5;
                        stNRIQPara.stUvnrPara.mode_cell[m].setting[k].step1_bf_sigmaR[i] = 20;
                        stNRIQPara.stUvnrPara.mode_cell[m].setting[k].step2_bf_sigmaR[i] = 16;
                        stNRIQPara.stUvnrPara.mode_cell[m].setting[k].step3_bf_sigmaR[i] = 8;

                    }
                }
            }

            rk_aiq_user_api_anr_SetIQPara(ctx, &stNRIQPara);
            sleep(5);
            //printf all the para
            rk_aiq_user_api_anr_GetIQPara(ctx, &stGetNRIQPara);

            for(int m = 0; m < 1; m++) {
                for(int k = 0; k < 1; k++) {
                    for(int i = 0; i < CALIBDB_NR_SHARP_MAX_ISO_LEVEL; i++ ) {
                        printf("\n\n!!!!!!!!!!set:%d cell:%d !!!!!!!!!!\n", k, i);
                        printf("oyyf222 bayernr: fiter:%f lamda:%f fixw:%f %f %f %f\n",
                               stGetNRIQPara.stBayernrPara.mode_cell[m].setting[k].filtPara[i],
                               stGetNRIQPara.stBayernrPara.mode_cell[m].setting[k].lamda,
                               stGetNRIQPara.stBayernrPara.mode_cell[m].setting[k].fixW[0][i],
                               stGetNRIQPara.stBayernrPara.mode_cell[m].setting[k].fixW[1][i],
                               stGetNRIQPara.stBayernrPara.mode_cell[m].setting[k].fixW[2][i],
                               stGetNRIQPara.stBayernrPara.mode_cell[m].setting[k].fixW[3][i]);

                        printf("oyyf222 mfnr: limiy:%f %f %f %f uv: %f %f %f, y_lo:%f %f %f %f y_hi:%f %f %f %f uv_lo:%f %f %f uv_hi:%f %f %f\n",
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].weight_limit_y[0],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].weight_limit_y[1],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].weight_limit_y[2],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].weight_limit_y[3],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].weight_limit_uv[0],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].weight_limit_uv[1],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].weight_limit_uv[2],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].y_lo_bfscale[0],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].y_lo_bfscale[1],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].y_lo_bfscale[2],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].y_lo_bfscale[3],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].y_hi_bfscale[0],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].y_hi_bfscale[1],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].y_hi_bfscale[2],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].y_hi_bfscale[3],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].uv_lo_bfscale[0],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].uv_lo_bfscale[1],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].uv_lo_bfscale[2],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].uv_hi_bfscale[0],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].uv_hi_bfscale[1],
                               stGetNRIQPara.stMfnrPara.mode_cell[m].setting[k].mfnr_iso[i].uv_hi_bfscale[2]);

                        printf("oyyf222 ynr: lo_bf:%f %f %f %f  lo_do:%f %f %f %f  hi_bf:%f %f %f %f stre:%f hi_do:%f %f %f %f\n",
                               stGetNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].lo_bfScale[0],
                               stGetNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].lo_bfScale[1],
                               stGetNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].lo_bfScale[2],
                               stGetNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].lo_bfScale[3],
                               stGetNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].denoise_weight[0],
                               stGetNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].denoise_weight[1],
                               stGetNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].denoise_weight[2],
                               stGetNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].denoise_weight[3],
                               stGetNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].hi_bfScale[0],
                               stGetNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].hi_bfScale[1],
                               stGetNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].hi_bfScale[2],
                               stGetNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].hi_bfScale[3],
                               stGetNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].hi_denoiseStrength,
                               stGetNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].hi_denoiseWeight[0],
                               stGetNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].hi_denoiseWeight[1],
                               stGetNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].hi_denoiseWeight[2],
                               stGetNRIQPara.stYnrPara.mode_cell[m].setting[k].ynr_iso[i].hi_denoiseWeight[3]
                              );

                        printf("oyyf222 uvnr: uv:%f  med:%f %f sigmaR:%f %f %f\n",
                               stGetNRIQPara.stUvnrPara.mode_cell[m].setting[k].step0_uvgrad_ratio[i],
                               stGetNRIQPara.stUvnrPara.mode_cell[m].setting[k].step1_median_ratio[i],
                               stGetNRIQPara.stUvnrPara.mode_cell[m].setting[k].step2_median_ratio[i],
                               stGetNRIQPara.stUvnrPara.mode_cell[m].setting[k].step1_bf_sigmaR[i],
                               stGetNRIQPara.stUvnrPara.mode_cell[m].setting[k].step2_bf_sigmaR[i],
                               stGetNRIQPara.stUvnrPara.mode_cell[m].setting[k].step3_bf_sigmaR[i]);

                        printf("!!!!!!!!!!set:%d cell:%d  end !!!!!!!!!!\n\n", k, i);
                    }
                }
            }
        }
        break;
    case 'J':
        if (CHECK_ISP_HW_V20()) {
            rk_aiq_sharp_IQpara_t stSharpIQpara;
            rk_aiq_sharp_IQpara_t stGetSharpIQpara;
            stSharpIQpara.module_bits = (1 << ASHARP_MODULE_SHARP) | (1 << ASHARP_MODULE_EDGEFILTER) ;
            rk_aiq_user_api_asharp_GetIQPara(ctx, &stSharpIQpara);
            for(int m = 0; m < 3; m++) {
                for(int k = 0; k < 2; k++) {
                    for(int i = 0; i < CALIBDB_NR_SHARP_MAX_ISO_LEVEL; i++ ) {
                        stSharpIQpara.stSharpPara.mode_cell[m].setting[k].sharp_iso[i].hratio = 1.9;
                        stSharpIQpara.stSharpPara.mode_cell[m].setting[k].sharp_iso[i].lratio = 0.4;
                        stSharpIQpara.stSharpPara.mode_cell[m].setting[k].sharp_iso[i].mf_sharp_ratio = 5.0;
                        stSharpIQpara.stSharpPara.mode_cell[m].setting[k].sharp_iso[i].hf_sharp_ratio = 6.0;

                        stSharpIQpara.stEdgeFltPara.mode_cell[m].setting[k].edgeFilter_iso[i].edge_thed = 33.0;
                        stSharpIQpara.stEdgeFltPara.mode_cell[m].setting[k].edgeFilter_iso[i].local_alpha = 0.5;
                    }
                }
            }
            rk_aiq_user_api_asharp_SetIQPara(ctx, &stSharpIQpara);
            sleep(5);
            rk_aiq_user_api_asharp_GetIQPara(ctx, &stGetSharpIQpara);
            for(int m = 0; m < 1; m++) {
                for(int k = 0; k < 1; k++) {
                    for(int i = 0; i < CALIBDB_NR_SHARP_MAX_ISO_LEVEL; i++ ) {
                        printf("\n\n!!!!!!!!!!set:%d cell:%d !!!!!!!!!!\n", k, i);
                        printf("oyyf222 sharp:%f %f ratio:%f %f\n",
                               stGetSharpIQpara.stSharpPara.mode_cell[m].setting[k].sharp_iso[i].lratio,
                               stGetSharpIQpara.stSharpPara.mode_cell[m].setting[k].sharp_iso[i].hratio,
                               stGetSharpIQpara.stSharpPara.mode_cell[m].setting[k].sharp_iso[i].mf_sharp_ratio,
                               stGetSharpIQpara.stSharpPara.mode_cell[m].setting[k].sharp_iso[i].hf_sharp_ratio);

                        printf("oyyf222 edgefilter:%f %f\n",
                               stGetSharpIQpara.stEdgeFltPara.mode_cell[m].setting[k].edgeFilter_iso[i].edge_thed,
                               stGetSharpIQpara.stEdgeFltPara.mode_cell[m].setting[k].edgeFilter_iso[i].local_alpha);

                        printf("!!!!!!!!!!set:%d cell:%d  end !!!!!!!!!!\n", k, i);
                    }
                }
            }
        }
        break;
    case 'K':
        printf("test mirro, flip\n");
        bool mirror, flip;
        rk_aiq_uapi_getMirrorFlip(ctx, &mirror, &flip);
        printf("oringinal mir %d, flip %d \n", mirror, flip);
        mirror = true;
        flip = true;
        printf("set mir %d, flip %d \n", mirror, flip);
        rk_aiq_uapi_setMirroFlip(ctx, true, true, 3);
        rk_aiq_uapi_getMirrorFlip(ctx, &mirror, &flip);
        printf("after set mir %d, flip %d \n", mirror, flip);
        break;
    case 'L':
        printf("test fec correct level100\n");
        rk_aiq_uapi_setFecCorrectLevel(ctx, 100);
        break;
    case 'M':
        printf("test fec correct level255\n");
        rk_aiq_uapi_setFecCorrectLevel(ctx, 255);
        break;
    case 'N':
    {
        rk_aiq_dpcc_attrib_V20_t attr;
        rk_aiq_user_api2_adpcc_GetAttrib(ctx, &attr);
        rk_aiq_user_api2_adpcc_SetAttrib(ctx, &attr);
        adebayer_attrib_t attr2;
        rk_aiq_user_api2_adebayer_SetAttrib(ctx, attr2);
    }
    break;
    case 'O':
        printf("test not bypass fec\n");
        rk_aiq_uapi_setFecBypass(ctx, false);
        break;
    case 'P':
    {
        int work_mode = demo_ctx->hdrmode;
        rk_aiq_working_mode_t new_mode;
        if (work_mode == RK_AIQ_WORKING_MODE_NORMAL)
            new_mode = RK_AIQ_WORKING_MODE_ISP_HDR3;
        else
            new_mode = RK_AIQ_WORKING_MODE_NORMAL;
        printf("switch work mode from %d to %d\n", work_mode, new_mode);
        *const_cast<int*>(&demo_ctx->hdrmode) = work_mode = new_mode;
        rk_aiq_uapi_sysctl_swWorkingModeDyn(ctx, new_mode);
    }
    break;
    case 'Q':
    {
        rk_aiq_rotation_t rot = RK_AIQ_ROTATION_90;
        rk_aiq_mems_sensor_intf_t intf = {0};
        const char* main_scene = "good";
        const char* sub_scene = "bad";
        rk_aiq_uapi2_sysctl_setSharpFbcRotation(ctx, rot);
        rk_aiq_uapi2_sysctl_setMulCamConc(ctx, true);
        rk_aiq_uapi2_sysctl_regMemsSensorIntf(ctx, &intf);
        rk_aiq_uapi2_sysctl_switch_scene(ctx, main_scene, sub_scene);
    }
    break;
    case 'R':
    {
        rk_aiq_cpsl_info_t cpsl_info;
        rk_aiq_cpsl_cap_t cpsl_cap;
        rk_aiq_uapi2_sysctl_getCpsLtInfo(ctx, &cpsl_info);
        rk_aiq_uapi2_sysctl_queryCpsLtCap(ctx, &cpsl_cap);
        printf("sensitivity: %f, cap sensitivity: %f:%f:%f\n", cpsl_info.sensitivity,
               cpsl_cap.sensitivity.min, cpsl_cap.sensitivity.step, cpsl_cap.sensitivity.max);
        rk_aiq_cpsl_cfg_t cpsl_cfg;
        rk_aiq_uapi2_sysctl_setCpsLtCfg(ctx, &cpsl_cfg);
    }
    break;
    case 'S':
        printf("test ldch correct level100\n");
        rk_aiq_uapi_setLdchCorrectLevel(ctx, 100);
        break;
    case 'T':
    {
        rk_aiq_rect_t info;
        rk_aiq_uapi2_sysctl_getCrop(ctx, &info);
        printf("left:%d, top:%d, width:%d, height:%d\n", info.left, info.top, info.width, info.height);
        info.left += 64;
        info.top += 64;
        info.width = 640;
        info.height = 480;
        //rk_aiq_uapi2_sysctl_setCrop(ctx, info);
    }
    break;
    case 'U':
    {
        char output_dir[64] = {0};
        printf("test to capture raw sync\n");
        rk_aiq_uapi_debug_captureRawSync(ctx, CAPTURE_RAW_SYNC, 5, "/tmp", output_dir);
        printf("Raw's storage directory is (%s)\n", output_dir);
    }
    break;
    case 'V':
    {
        test_update_iqfile(demo_ctx);
    }
    break;
    case 'W':
    {
        rk_aiq_ver_info_t vers;
        rk_aiq_uapi2_get_version_info(&vers);
        printf("aiq ver %s, parser ver %s, magic code %d, awb ver %s\n"
               "ae ver %s, af ver %s, ahdr ver %s\n", vers.aiq_ver,
               vers.iq_parser_ver, vers.iq_parser_magic_code,
               vers.awb_algo_ver, vers.ae_algo_ver,
               vers.af_algo_ver, vers.ahdr_algo_ver);
    }
    break;
    case 'X':
    {
        for (int type = RK_AIQ_ALGO_TYPE_AE; type < RK_AIQ_ALGO_TYPE_MAX; type++)
        {
            bool ret = rk_aiq_uapi2_sysctl_getAxlibStatus(ctx, type, 0);
            printf("%d is %s, \n", type, (ret ? "enabled" : "disabled or unregistered"));

            const RkAiqAlgoContext* algo_ptr = rk_aiq_uapi2_sysctl_getEnabledAxlibCtx(ctx, type);
            if (algo_ptr) {
                printf("type: %d ==> algo_ptr: %p\n", type, algo_ptr);
            }
            if (ret == false) {
                bool ret2 = rk_aiq_uapi2_sysctl_enableAxlib(ctx, type, 0, !ret);
            }
        }
    }
    break;
    case 'Y':
    {
        rk_aiq_lsc_querry_info_t info;
        rk_aiq_user_api2_alsc_QueryLscInfo(ctx, &info);
        printf("LSC: enable: %s, \n", (info.lsc_en ? "true" : "false"));
        printf("r: %d, %d, %d \n", info.r_data_tbl[0], info.r_data_tbl[1], info.r_data_tbl[2]);
        printf("gr: %d, %d, %d \n", info.gr_data_tbl[0], info.gr_data_tbl[1], info.gr_data_tbl[2]);
        printf("gb: %d, %d, %d \n", info.gb_data_tbl[0], info.gb_data_tbl[1], info.gb_data_tbl[2]);
        printf("b: %d, %d, %d \n", info.b_data_tbl[0], info.b_data_tbl[1], info.b_data_tbl[2]);
    }
    break;
    case 'Z':
    {
        rk_aiq_static_info_t info;
        rk_aiq_uapi2_sysctl_enumStaticMetas(0, &info);
        printf("isp version: %d, sensor name: %s\n", info.isp_hw_ver, info.sensor_info.sensor_name);
    }
    case '[':
        set_ae_onoff(ctx, true);
        printf("set ae on\n");
        break;
    case ']':
        set_ae_onoff(ctx, false);
        printf("set ae off\n");
        break;
    default:
        break;
    }
}
#endif

static void errno_exit(demo_context_t *ctx, const char *s)
{
    ERR("%s: %s error %d, %s\n", get_sensor_name(ctx), s, errno, strerror(errno));
    //exit(EXIT_FAILURE);
}

static int xioctl(int fh, int request, void *arg)
{
    int r;
    do {
        r = ioctl(fh, request, arg);
    } while (-1 == r && EINTR == errno);
    return r;
}

static bool get_value_from_file(const char* path, int* value, int* frameId)
{
    const char *delim = " ";
    char buffer[16] = {0};
    int fp;

    fp = open(path, O_RDONLY | O_SYNC);
    if (fp) {
        if (read(fp, buffer, sizeof(buffer)) > 0) {
            char *p = NULL;

            p = strtok(buffer, delim);
            if (p != NULL) {
                *value = atoi(p);
                p = strtok(NULL, delim);
                if (p != NULL)
                    *frameId = atoi(p);
            }
        }
        close(fp);
        return true;
    }

    return false;
}

static int write_yuv_to_file(const void *p,
                             int size, int sequence, demo_context_t *ctx)
{
    char file_name[128] = {0};

    snprintf(file_name, sizeof(file_name),
            "%s/frame%d.yuv",
            ctx->yuv_dir_path,sequence);
    ctx->fp = fopen(file_name, "wb+");
    if (ctx->fp == NULL) {
        ERR("fopen yuv file %s failed!\n", file_name);
        return -1;
    }

    fwrite(p, size, 1, ctx->fp);
    fflush(ctx->fp);

    if (ctx->fp) {
        fclose(ctx->fp);
        ctx->fp = NULL;
    }

    for (int i = 0; i < ctx->capture_yuv_num; i++)
        printf("<");

    printf("\n");
    // printf("write frame%d yuv\n", sequence);

    return 0;
}

static int creat_yuv_dir(const char* path, demo_context_t *ctx)
{
    time_t now;
    struct tm* timenow;

    if (!path)
        return -1;

    time(&now);
    timenow = localtime(&now);
    snprintf(ctx->yuv_dir_path, sizeof(ctx->yuv_dir_path),
             "%s/yuv_%04d-%02d-%02d_%02d-%02d-%02d",
             path,
             timenow->tm_year + 1900,
             timenow->tm_mon + 1,
             timenow->tm_mday,
             timenow->tm_hour,
             timenow->tm_min,
             timenow->tm_sec);

    // printf("mkdir %s for capturing yuv!\n", yuv_dir_path);

    if(mkdir(ctx->yuv_dir_path, 0755) < 0) {
        printf("mkdir %s error!!!\n", ctx->yuv_dir_path);
        return -1;
    }

    ctx->_is_yuv_dir_exist = true;

    return 0;
}

static void process_image(const void *p, int sequence, int size, demo_context_t *ctx)
{
    if (ctx->fp && sequence >= ctx->skipCnt && ctx->outputCnt-- > 0) {
        printf(">\n");
        fwrite(p, size, 1, ctx->fp);
        fflush(ctx->fp);
        fsync(fileno(ctx->fp));
    } else if (ctx->fp && sequence >= ctx->skipCnt && ctx->outputCnt-- == 0) {
        fclose(ctx->fp);
        ctx->fp = NULL;
    } else if (ctx->writeFileSync) {
        if (!ctx->is_capture_yuv) {
            char file_name[32] = {0};
            int rawFrameId = 0;

            snprintf(file_name, sizeof(file_name), "%s/%s",
                     CAPTURE_RAW_PATH, CAPTURE_CNT_FILENAME);
            get_value_from_file(file_name, &ctx->capture_yuv_num, &rawFrameId);

            /*
             * printf("%s: rawFrameId: %d, sequence: %d\n", __FUNCTION__,
             *        rawFrameId, sequence);
             */

            sequence += 1;
            if (ctx->capture_yuv_num > 0 && \
                    ((sequence >= rawFrameId && rawFrameId > 0) || sequence < 2))
                ctx->is_capture_yuv = true;
        }

        if (ctx->is_capture_yuv) {
            if (!ctx->_is_yuv_dir_exist) {
                creat_yuv_dir(DEFAULT_CAPTURE_RAW_PATH, ctx);
            }

            if (ctx->_is_yuv_dir_exist) {
                write_yuv_to_file(p, size, sequence, ctx);
                rk_aiq_uapi2_debug_captureRawNotify(ctx->aiq_ctx);
            }

            if (ctx->capture_yuv_num-- == 0) {
                ctx->is_capture_yuv = false;
                ctx->_is_yuv_dir_exist = false;
            }
        }
    }
}

static int read_frame(demo_context_t *ctx)
{
    struct v4l2_buffer buf;
    int i, bytesused;

    while (ctx->use_poll) {
        fd_set fds;
        struct timeval tv = { ctx->use_poll ? 2 : 0, 0 };
        int r = 0;

        FD_ZERO(&fds);
        FD_SET(ctx->fd, &fds);

        r = select(ctx->fd + 1, &fds, NULL, NULL, &tv);

        if (r == -1) {
            if (EINTR == errno)
                continue;
            ERR("select error: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (r == 0) {
            ERR("select timeout\n");
            if (ctx->isAovMode) {
                return 0;
            }
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(ctx->fd, &fds))
            break;
    }

    CLEAR(buf);

    buf.type = ctx->buf_type;
    buf.memory = V4L2_MEMORY_MMAP;

    struct v4l2_plane planes[FMT_NUM_PLANES];
    memset(planes, 0, sizeof(struct v4l2_plane)*FMT_NUM_PLANES);
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == ctx->buf_type) {
        buf.m.planes = planes;
        buf.length = FMT_NUM_PLANES;
    }

    if (-1 == xioctl(ctx->fd, VIDIOC_DQBUF, &buf))
        errno_exit(ctx, "VIDIOC_DQBUF");

    i = buf.index;

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == ctx->buf_type)
        bytesused = buf.m.planes[0].bytesused;
    else
        bytesused = buf.bytesused;

#if ISPDEMO_ENABLE_DRM
#ifdef ISPFEC_API
    int buf_fd = -1;
    void* buf_addr = NULL;

    buf_fd = ctx->buffers[i].export_fd;
    buf_addr = ctx->buffers[i].start;

    int dstFd = g_drm_buf_pic_out.dmabuf_fd;
    buf_fd = dstFd;
    buf_addr = g_drm_buf_pic_out.map;
    rk_ispfec_api_process(g_ispfec_ctx, ctx->buffers[i].export_fd, dstFd);
#endif

    if (ctx->vop) {
        int dispWidth, dispHeight;

        if (ctx->width > 1920)
            dispWidth = 1920;
        else
            dispWidth = ctx->width;

        if (ctx->height > 1080)
            dispHeight = 1080;
        else
            dispHeight = ctx->height;

#if ISPDEMO_ENABLE_RGA
        if (strlen(ctx->dev_name) && strlen(ctx->dev_name2)) {
            if (ctx->dev_using == 1) {
#ifdef ISPFEC_API
                display_win1(buf_addr, buf_fd,  RK_FORMAT_YCbCr_420_SP, dispWidth, dispHeight, 0);
#else
                display_win1(ctx->buffers[i].start, ctx->buffers[i].export_fd,  RK_FORMAT_YCbCr_420_SP, dispWidth, dispHeight, 0);
#endif
            } else {
#ifdef ISPFEC_API
                display_win2(buf_addr, buf_fd,  RK_FORMAT_YCbCr_420_SP, dispWidth, dispHeight, 0);
#else
                display_win2(ctx->buffers[i].start, ctx->buffers[i].export_fd,  RK_FORMAT_YCbCr_420_SP, dispWidth, dispHeight, 0);
#endif
            }
        } else {
#else
        {
#endif
            drmDspFrame(ctx->width, ctx->height, dispWidth, dispHeight, ctx->buffers[i].export_fd,
                        ctx->buffers[i].start, DRM_FORMAT_NV12);
        }
    }
#endif

#ifdef ISPFEC_API
    process_image(buf_addr,  buf.sequence, bytesused, ctx);
#else
    process_image(ctx->buffers[i].start,  buf.sequence, bytesused, ctx);
#endif

#ifdef COLOR_CONSISTENCY_TEST
    rk_aiq_wb_querry_info_t wb_querry_info;
    rk_aiq_user_api2_awb_QueryWBInfo(ctx->aiq_ctx, &wb_querry_info);//ctx->aiq_ctx is main camera
    rk_aiq_uapiV2_awb_Slave2Main_Cfg_t slave2Main;
    slave2Main.enable = true;
    slave2Main.camM.wbgain = wb_querry_info.gain;
    slave2Main.camM.fLV = wb_querry_info.LVValue;
    slave2Main.camM.fLV_valid = true;
    char filename[] = "/etc/iqfiles/wbgain_convert2.bin";
    rk_aiq_user_api2_awb_loadConvertLut(&slave2Main.cct_lut_cfg, filename);
    rk_aiq_user_api2_awb_IqMap2Main(ctx->aiq_ctx, slave2Main); //ctx->aiq_ctx is slave camera
    rk_aiq_user_api2_awb_freeConvertLut(&slave2Main.cct_lut_cfg);
    rk_aiq_color_info_t aColor_sw_info;
    rk_aiq_user_api2_GetAcolorSwInfo(ctx->aiq_ctx, &aColor_sw_info); //ctx->aiq_ctx is main camera
    printf("sensor gain = %f ,wbgain=[%f,%f]\n", aColor_sw_info.sensorGain, aColor_sw_info.awbGain[0], aColor_sw_info.awbGain[1]);
    rk_aiq_uapi2_setAcolorSwInfo(ctx->aiq_ctx, aColor_sw_info); //ctx->aiq_ctx is slave camera
#endif

    if (-1 == xioctl(ctx->fd, VIDIOC_QBUF, &buf))
        errno_exit(ctx, "VIDIOC_QBUF");

    return 1;
}

static int read_frame_pp_oneframe(demo_context_t *ctx)
{
    struct v4l2_buffer buf;
    struct v4l2_buffer buf_pp;
    int i, ii = 0, bytesused;
    static int first_time = 1;

    CLEAR(buf);
    // dq one buf from isp mp
    DBG("------ dq 1 from isp mp --------------\n");
    buf.type = ctx->buf_type;
    buf.memory = V4L2_MEMORY_MMAP;

    struct v4l2_plane planes[FMT_NUM_PLANES];
    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == ctx->buf_type) {
        buf.m.planes = planes;
        buf.length = FMT_NUM_PLANES;
    }

    if (-1 == xioctl(ctx->fd_isp_mp, VIDIOC_DQBUF, &buf))
        errno_exit(ctx, "VIDIOC_DQBUF");

    i = buf.index;

    if (first_time ) {
        DBG("------ dq 2 from isp mp --------------\n");
        if (-1 == xioctl(ctx->fd_isp_mp, VIDIOC_DQBUF, &buf))
            errno_exit(ctx, "VIDIOC_DQBUF");

        ii = buf.index;
    }

    if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == ctx->buf_type)
        bytesused = buf.m.planes[0].bytesused;
    else
        bytesused = buf.bytesused;

    // queue to ispp input
    DBG("------ queue 1 index %d to ispp input --------------\n", i);
    CLEAR(buf_pp);
    buf_pp.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    buf_pp.memory = V4L2_MEMORY_DMABUF;
    buf_pp.index = i;

    struct v4l2_plane planes_pp[FMT_NUM_PLANES];
    memset(planes_pp, 0, sizeof(planes_pp));
    buf_pp.m.planes = planes_pp;
    buf_pp.length = FMT_NUM_PLANES;
    buf_pp.m.planes[0].m.fd = ctx->buffers_mp[i].export_fd;

    if (-1 == xioctl(ctx->fd_pp_input, VIDIOC_QBUF, &buf_pp))
        errno_exit(ctx, "VIDIOC_QBUF");

    if (first_time ) {
        DBG("------ queue 2 index %d to ispp input --------------\n", ii);
        buf_pp.index = ii;
        buf_pp.m.planes[0].m.fd = ctx->buffers_mp[ii].export_fd;
        if (-1 == xioctl(ctx->fd_pp_input, VIDIOC_QBUF, &buf_pp))
            errno_exit(ctx, "VIDIOC_QBUF");
    }
    // read frame from ispp sharp/scale
    DBG("------ readframe from output --------------\n");
    read_frame(ctx);
    // dq from pp input
    DBG("------ dq 1 from ispp input--------------\n");
    if (-1 == xioctl(ctx->fd_pp_input, VIDIOC_DQBUF, &buf_pp))
        errno_exit(ctx, "VIDIOC_DQBUF");
    // queue back to mp
    DBG("------ queue 1 index %d back to isp mp--------------\n", buf_pp.index);
    buf.index = buf_pp.index;
    if (-1 == xioctl(ctx->fd_isp_mp, VIDIOC_QBUF, &buf))
        errno_exit(ctx, "VIDIOC_QBUF");

    first_time = 0;
    return 1;
}

static bool g_allow_enter_sleep = false;
static int aov_read_frame(demo_context_t *ctx) {
    int ret = read_frame(ctx);
    if (ret == 1) {
        if (!ctx->aovPauseAiq) {
            ctx->aovContinueRunCnt++;
            if (ctx->aovContinueRunCnt == ctx->aovContinueCnt) {
                rk_aiq_uapi2_sysctl_pause(ctx->aiq_ctx, true);
                ctx->aovPauseAiq = true;
                ctx->aovLoopRunCnt = 0;
            }
        } else {
            if (ctx->aovLoopRunCnt < ctx->aovLoopCnt) {
                if (g_allow_enter_sleep) {
                    ctx->aovLoopRunCnt++;
                    ctx->aovEnterSleep = true;
                }
            } else {
                ctx->aovContinueRunCnt = 0;
                rk_aiq_uapi2_sysctl_resume(ctx->aiq_ctx);
                ctx->aovPauseAiq = false;
                g_allow_enter_sleep = false;
            }
        }
    } else if (ctx->aovPauseAiq) {
        g_allow_enter_sleep = true;
        ctx->aovEnterSleep = true;
    }
    return ret;
}

static void mainloop(demo_context_t *ctx)
{
    while ((ctx->frame_count == -1) || (ctx->frame_count-- > 0)) {
        if (ctx->pponeframe) {
            read_frame_pp_oneframe(ctx);
        } else if (ctx->isAovMode) {
            aov_read_frame(ctx);
        } else {
            read_frame(ctx);
            XCAM_STATIC_FPS_CALCULATION(rkisp_demo, 30);
        }
    }
}

static void stop_capturing(demo_context_t *ctx)
{
    enum v4l2_buf_type type;

    type = ctx->buf_type;
    if (-1 == xioctl(ctx->fd, VIDIOC_STREAMOFF, &type))
        errno_exit(ctx, "VIDIOC_STREAMOFF");
}

static void stop_capturing_pp_oneframe(demo_context_t *ctx)
{
    enum v4l2_buf_type type;

    type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    if (-1 == xioctl(ctx->fd_pp_input, VIDIOC_STREAMOFF, &type))
        errno_exit(ctx, "VIDIOC_STREAMOFF ppinput");
    type = ctx->buf_type;
    if (-1 == xioctl(ctx->fd_isp_mp, VIDIOC_STREAMOFF, &type))
        errno_exit(ctx, "VIDIOC_STREAMOFF ispmp");
}

static void start_capturing(demo_context_t *ctx)
{
    unsigned int i;
    enum v4l2_buf_type type;

    for (i = 0; i < ctx->n_buffers; ++i) {
        struct v4l2_buffer buf;

        CLEAR(buf);
        buf.type = ctx->buf_type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == ctx->buf_type) {
            struct v4l2_plane planes[FMT_NUM_PLANES];

            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }
        if (-1 == xioctl(ctx->fd, VIDIOC_QBUF, &buf))
            errno_exit(ctx, "VIDIOC_QBUF");
    }
    type = ctx->buf_type;
    DBG("%s:-------- stream on output -------------\n", get_sensor_name(ctx));

    if (-1 == xioctl(ctx->fd, VIDIOC_STREAMON, &type))
        errno_exit(ctx, "VIDIOC_STREAMON");
}

static void start_capturing_pp_oneframe(demo_context_t *ctx)
{
    unsigned int i;
    enum v4l2_buf_type type;

    type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    DBG("%s:-------- stream on pp input -------------\n", get_sensor_name(ctx));
    if (-1 == xioctl(ctx->fd_pp_input, VIDIOC_STREAMON, &type))
        errno_exit(ctx, "VIDIOC_STREAMON pp input");

    type = ctx->buf_type;
    for (i = 0; i < ctx->n_buffers; ++i) {
        struct v4l2_buffer buf;

        CLEAR(buf);
        buf.type = ctx->buf_type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == ctx->buf_type) {
            struct v4l2_plane planes[FMT_NUM_PLANES];

            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }
        if (-1 == xioctl(ctx->fd_isp_mp, VIDIOC_QBUF, &buf))
            errno_exit(ctx, "VIDIOC_QBUF");
    }
    DBG("%s:-------- stream on isp mp -------------\n", get_sensor_name(ctx));
    if (-1 == xioctl(ctx->fd_isp_mp, VIDIOC_STREAMON, &type))
        errno_exit(ctx, "VIDIOC_STREAMON ispmp");
}


static void uninit_device(demo_context_t *ctx)
{
    unsigned int i;
    if (ctx->n_buffers == 0)
        return;

    for (i = 0; i < ctx->n_buffers; ++i) {
        if (-1 == munmap(ctx->buffers[i].start, ctx->buffers[i].length))
            errno_exit(ctx, "munmap");

        close(ctx->buffers[i].export_fd);
    }

    free(ctx->buffers);
    ctx->n_buffers = 0;
}

static void uninit_device_pp_oneframe(demo_context_t *ctx)
{
    unsigned int i;

    for (i = 0; i < ctx->n_buffers; ++i) {
        if (-1 == munmap(ctx->buffers_mp[i].start, ctx->buffers_mp[i].length))
            errno_exit(ctx, "munmap");

        close(ctx->buffers_mp[i].export_fd);
    }

    free(ctx->buffers_mp);
}

static void init_mmap(int pp_onframe, demo_context_t *ctx)
{
    struct v4l2_requestbuffers req;
    int fd_tmp = -1;

    CLEAR(req);

    if (pp_onframe)
        fd_tmp = ctx->fd_isp_mp ;
    else
        fd_tmp = ctx->fd;

    req.count = BUFFER_COUNT;
    req.type = ctx->buf_type;
    req.memory = V4L2_MEMORY_MMAP;

    struct buffer *tmp_buffers = NULL;

    if (-1 == xioctl(fd_tmp, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            ERR("%s: %s does not support "
                "memory mapping\n", get_sensor_name(ctx), get_dev_name(ctx));
            //exit(EXIT_FAILURE);
        } else {
            errno_exit(ctx, "VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2) {
        ERR("%s: Insufficient buffer memory on %s\n", get_sensor_name(ctx),
            get_dev_name(ctx));
        //exit(EXIT_FAILURE);
    }

    tmp_buffers = (struct buffer*)calloc(req.count, sizeof(struct buffer));

    if (!tmp_buffers) {
        ERR("%s: Out of memory\n", get_sensor_name(ctx));
        //exit(EXIT_FAILURE);
    }

    if (pp_onframe)
        ctx->buffers_mp = tmp_buffers;
    else
        ctx->buffers = tmp_buffers;

    for (ctx->n_buffers = 0; ctx->n_buffers < req.count; ++ctx->n_buffers) {
        struct v4l2_buffer buf;
        struct v4l2_plane planes[FMT_NUM_PLANES];
        CLEAR(buf);
        CLEAR(planes);

        buf.type = ctx->buf_type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = ctx->n_buffers;

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == ctx->buf_type) {
            buf.m.planes = planes;
            buf.length = FMT_NUM_PLANES;
        }

        if (-1 == xioctl(fd_tmp, VIDIOC_QUERYBUF, &buf))
            errno_exit(ctx, "VIDIOC_QUERYBUF");

        if (V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE == ctx->buf_type) {
            tmp_buffers[ctx->n_buffers].length = buf.m.planes[0].length;
            tmp_buffers[ctx->n_buffers].start =
                mmap(NULL /* start anywhere */,
                     buf.m.planes[0].length,
                     PROT_READ | PROT_WRITE /* required */,
                     MAP_SHARED /* recommended */,
                     fd_tmp, buf.m.planes[0].m.mem_offset);
        } else {
            tmp_buffers[ctx->n_buffers].length = buf.length;
            tmp_buffers[ctx->n_buffers].start =
                mmap(NULL /* start anywhere */,
                     buf.length,
                     PROT_READ | PROT_WRITE /* required */,
                     MAP_SHARED /* recommended */,
                     fd_tmp, buf.m.offset);
        }

        if (MAP_FAILED == tmp_buffers[ctx->n_buffers].start)
            errno_exit(ctx, "mmap");

        // export buf dma fd
        struct v4l2_exportbuffer expbuf;
        xcam_mem_clear (expbuf);
        expbuf.type = ctx->buf_type;
        expbuf.index = ctx->n_buffers;
        expbuf.flags = O_CLOEXEC;
        if (xioctl(fd_tmp, VIDIOC_EXPBUF, &expbuf) < 0) {
            errno_exit(ctx, "get dma buf failed\n");
        } else {
            DBG("%s: get dma buf(%u)-fd: %d\n", get_sensor_name(ctx), ctx->n_buffers, expbuf.fd);
        }
        tmp_buffers[ctx->n_buffers].export_fd = expbuf.fd;
    }
}

static void init_input_dmabuf_oneframe(demo_context_t *ctx) {
    struct v4l2_requestbuffers req;

    CLEAR(req);

    printf("%s:-------- request pp input buffer -------------\n", get_sensor_name(ctx));
    req.count = BUFFER_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
    req.memory = V4L2_MEMORY_DMABUF;

    if (-1 == xioctl(ctx->fd_pp_input, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            ERR("does not support "
                "DMABUF\n");
            exit(EXIT_FAILURE);
        } else {
            errno_exit(ctx, "VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2) {
        ERR("Insufficient buffer memory on %s\n",
            get_dev_name(ctx));
        exit(EXIT_FAILURE);
    }
    printf("%s:-------- request isp mp buffer -------------\n", get_sensor_name(ctx));
    init_mmap(true, ctx);
}

static void init_device(demo_context_t *ctx)
{
    struct v4l2_capability cap;
    struct v4l2_format fmt;

    if (-1 == xioctl(ctx->fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            ERR("%s: %s is no V4L2 device\n", get_sensor_name(ctx),
                get_dev_name(ctx));
            //exit(EXIT_FAILURE);
        } else {
            errno_exit(ctx, "VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) &&
            !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
        ERR("%s: %s is not a video capture device, capabilities: %x\n",
            get_sensor_name(ctx), get_dev_name(ctx), cap.capabilities);
        //exit(EXIT_FAILURE);
    }

    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
        ERR("%s: %s does not support streaming i/o\n", get_sensor_name(ctx),
            get_dev_name(ctx));
        //exit(EXIT_FAILURE);
    }

    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
        ctx->buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        CLEAR(fmt);
        fmt.type = ctx->buf_type;
        fmt.fmt.pix.width = ctx->width;
        fmt.fmt.pix.height = ctx->height;
        fmt.fmt.pix.pixelformat = ctx->format;
        fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
        if (ctx->limit_range)
            fmt.fmt.pix.quantization = V4L2_QUANTIZATION_LIM_RANGE;
        else
            fmt.fmt.pix.quantization = V4L2_QUANTIZATION_FULL_RANGE;
    } else if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
        ctx->buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        CLEAR(fmt);
        fmt.type = ctx->buf_type;
        fmt.fmt.pix_mp.width = ctx->width;
        fmt.fmt.pix_mp.height = ctx->height;
        fmt.fmt.pix_mp.pixelformat = ctx->format;
        fmt.fmt.pix_mp.field = V4L2_FIELD_INTERLACED;
        if (ctx->limit_range)
            fmt.fmt.pix_mp.quantization = V4L2_QUANTIZATION_LIM_RANGE;
        else
            fmt.fmt.pix_mp.quantization = V4L2_QUANTIZATION_FULL_RANGE;
    }

    if (-1 == xioctl(ctx->fd, VIDIOC_S_FMT, &fmt))
        errno_exit(ctx, "VIDIOC_S_FMT");

    init_mmap(false, ctx);
}

static void init_device_pp_oneframe(demo_context_t *ctx)
{
    // TODO, set format and link, now do with setup_link.sh
    init_input_dmabuf_oneframe(ctx);
}

static void close_device(demo_context_t *ctx)
{
    if (-1 == close(ctx->fd))
        errno_exit(ctx, "close");

    ctx->fd = -1;
}

static void open_device(demo_context_t *ctx)
{
    printf("-------- open output dev_name:%s -------------\n", get_dev_name(ctx));

    if (!ctx->use_poll)
        ctx->fd = open(get_dev_name(ctx), O_RDWR /* required */ /*| O_NONBLOCK*/, 0);
    else
        ctx->fd = open(get_dev_name(ctx), O_RDWR | O_CLOEXEC | O_NONBLOCK, 0);

    if (-1 == ctx->fd) {
        ERR("Cannot open '%s': %d, %s\n",
            get_dev_name(ctx), errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static void close_device_pp_oneframe(demo_context_t *ctx)
{
    if (-1 == close(ctx->fd_pp_input))
        errno_exit(ctx, "close");

    ctx->fd_pp_input = -1;

    if (-1 == close(ctx->fd_isp_mp))
        errno_exit(ctx, "close");

    ctx->fd_isp_mp = -1;
}

static void open_device_pp_oneframe(demo_context_t *ctx)
{
    printf("-------- open pp input(video13) -------------\n");
    ctx->fd_pp_input = open("/dev/video13", O_RDWR /* required */ /*| O_NONBLOCK*/, 0);

    if (-1 == ctx->fd_pp_input) {
        ERR("Cannot open '%s': %d, %s\n",
            get_dev_name(ctx), errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("-------- open isp mp(video0) -------------\n");
    ctx->fd_isp_mp = open("/dev/video0", O_RDWR /* required */ /*| O_NONBLOCK*/, 0);

    if (-1 == ctx->fd_isp_mp ) {
        ERR("Cannot open '%s': %d, %s\n",
            get_dev_name(ctx), errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static void uninit_device_pp_onframe(demo_context_t *ctx)
{
    unsigned int i;

    for (i = 0; i < ctx->n_buffers; ++i) {
        if (-1 == munmap(ctx->buffers_mp[i].start, ctx->buffers_mp[i].length))
            errno_exit(ctx, "munmap");
        close(ctx->buffers_mp[i].export_fd);
    }

    free(ctx->buffers_mp);
}

static void parse_args(int argc, char **argv, demo_context_t *ctx)
{
    int c;
    int digit_optind = 0;
    optind = 0;
    while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"width",    required_argument, 0, 'w' },
            {"height",   required_argument, 0, 'h' },
            {"format",   required_argument, 0, 'f' },
            {"device",   required_argument, 0, 'd' },
            {"device2",   required_argument, 0, 'i' },
            {"device3",   required_argument, 0, 'g' },
            {"device4",   required_argument, 0, 'j' },
            {"device5",   required_argument, 0, 'y' },
            {"stream-to",   required_argument, 0, 'o' },
            {"stream-count",   required_argument, 0, 'n' },
            {"stream-skip",   required_argument, 0, 'k' },
            {"count",    required_argument, 0, 'c' },
            {"help",     no_argument,       0, 'p' },
            {"silent",   no_argument,       0, 's' },
            {"vop",     optional_argument, 0, 'v' },
            {"rkaiq",   no_argument,       0, 'r' },
            {"pponeframe",   no_argument,       0, 'm' },
            {"hdr",   required_argument,       0, 'a' },
            {"sync-to-raw", no_argument, 0, 'e' },
            {"limit", no_argument, 0, 'l' },
            {"ctl", required_argument, 0, 't' },
            {"iqpath", required_argument, 0, '1' },
            {"orp", required_argument, 0, '2' },
            //{"sensor",   required_argument,       0, 'b' },
            {"camgroup",   no_argument,       0, '3' },
            {"stream-poll",   no_argument,       0, '4' },
            {"aov",   no_argument,       0, '5' },
            {"aov-loop",   required_argument, 0, '6' },
            {"aov-continue",   required_argument, 0, '7' },
            {"set-scene",   required_argument, 0, '8' },
            {"switch-scene-loop", required_argument, 0, '9' },
            {0,          0,                 0,  0  }
        };

        //c = getopt_long(argc, argv, "w:h:f:i:d:o:c:ps",
        c = getopt_long(argc, argv, "w:h:f:i:g:j:y:d:o:c:n:k:a:t:1:2:v::3456:7:8:9:mpserl",
                        long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 'c':
            ctx->frame_count = atoi(optarg);
            break;
        case 'w':
            ctx->width = atoi(optarg);
            break;
        case 'h':
            ctx->height = atoi(optarg);
            break;
        case 'f':
            ctx->format = v4l2_fourcc(optarg[0], optarg[1], optarg[2], optarg[3]);
            break;
        case 'd':
            strcpy(ctx->dev_name, optarg);
            break;
        case 'i':
            strcpy(ctx->dev_name2, optarg);
            break;
        case 'g':
            strcpy(ctx->dev_name3, optarg);
            break;
        case 'j':
            strcpy(ctx->dev_name4, optarg);
            break;
        case 'y':
            strcpy(ctx->dev_name5, optarg);
            break;
        case 'o':
            strcpy(ctx->out_file, optarg);
            ctx->writeFile = 1;
            break;
        case 'n':
            ctx->outputCnt = atoi(optarg);
            break;
        case 'k':
            ctx->skipCnt = atoi(optarg);
            break;
        case 's':
            silent = 1;
            break;
        case 'v':
            if (optarg != NULL) {
                if (atoi(optarg) == DEMO_DISPLAY_TO_DSI) {
                    ctx->vop = DEMO_DISPLAY_TO_DSI;
                    DBG("Display to DSI\n");
                } else {
                    ctx->vop = DEMO_DISPLAY_TO_HDMI;
                    DBG("Display to HDMI\n");
                }
            } else {
                ctx->vop = DEMO_DISPLAY_TO_HDMI;
                DBG("Display to HDMI\n");
            }
            break;
        case 'r':
            ctx->rkaiq = 1;
            break;
        case 'm':
            ctx->pponeframe = 1;
            break;
        case 'a':
            ctx->hdrmode = atoi(optarg);
            break;
        case 'e':
            ctx->writeFileSync = 1;
            break;
        case 'l':
            ctx->limit_range = 1;
            break;
        case '1':
            strcpy(ctx->iqpath, optarg);
            break;
        case '2':
        {
            // parse raw fmt
            char* raw_dir = strstr(optarg, ",");
            if (!raw_dir) {
                printf("orp dir error ! \n");
                exit(-1);
            }
            size_t raw_dir_str_len = raw_dir - optarg;
            strncpy(ctx->orppath, optarg, raw_dir_str_len);

            char* raw_fmt_w_start = raw_dir + 1;
            int raw_width = atoi(raw_fmt_w_start);
            if (raw_width == 0) {
                printf("orp raw_width error ! \n");
                exit(-1);
            }
            ctx->orpRawW = raw_width;
            char* raw_fmt_h_start = strstr(raw_fmt_w_start, ":") + 1;
            if (!raw_fmt_h_start) {
                printf("orp raw_h error ! \n");
                exit(-1);
            }
            int raw_height = atoi(raw_fmt_h_start);
            ctx->orpRawH = raw_height;

            char* raw_fmt_pix_start = strstr(raw_fmt_h_start, ":") + 1;
            char* raw_fmt_pix_end = strstr(raw_fmt_pix_start, ",");
            char* rawbuf_type = NULL;
            if (raw_fmt_pix_end) {
                size_t fmt_len = raw_fmt_pix_end - raw_fmt_pix_start;
                size_t max_fmt_len = sizeof(ctx->orpRawFmt) - 1;
                if (fmt_len > max_fmt_len) {
                    printf("WARNING: orpRawFmt truncated from %zu to %zu\n", fmt_len, max_fmt_len);
                    fmt_len = max_fmt_len;
                }
                strncpy(ctx->orpRawFmt, raw_fmt_pix_start, fmt_len);
                ctx->orpRawFmt[fmt_len] = '\0';
                rawbuf_type = raw_fmt_pix_end + 1;
            }  else {
                strncpy(ctx->orpRawFmt, raw_fmt_pix_start, sizeof(ctx->orpRawFmt)-1);
                ctx->orpRawFmt[sizeof(ctx->orpRawFmt)-1] = '\0';
                rawbuf_type = NULL;
            }

            if (rawbuf_type) {
                if (!strcmp(rawbuf_type, "RAW_ADDR"))
                    ctx->orpRawBufType = RK_AIQ_RAW_ADDR;
                else if (!strcmp(rawbuf_type, "RAW_FD"))
                    ctx->orpRawBufType = RK_AIQ_RAW_FD;
                else if (!strcmp(rawbuf_type, "RAW_DATA"))
                    ctx->orpRawBufType = RK_AIQ_RAW_DATA;
                else
                    ctx->orpRawBufType = RK_AIQ_RAW_FILE;
            }

            printf("orp_path:%s, w:%d,h:%d, pix:%s, rawbuf_type:%s\n",
                   ctx->orppath, raw_width, raw_height, ctx->orpRawFmt, rawbuf_type);
            ctx->isOrp = true;
        }
        break;
        case 't':
            ctx->ctl_type = atoi(optarg);
            break;
        case '3':
            ctx->camGroup = true;
            break;
        case '4':
            ctx->use_poll = true;
            break;
        case '5':
            ctx->use_poll = true;
            ctx->isAovMode = true;
            break;
        case '6':
            ctx->aovLoopCnt = atoi(optarg);
            break;
        case '7':
            ctx->aovContinueCnt = atoi(optarg);
            break;
        case '8':
            if (parse_iq_scene_val(optarg, ctx->main_scene[0], ctx->sub_scene[0]) < 0) {
                ERR("Invalid iq scene value: %s\n", optarg);
                exit(-1);
            } else {
                DBG("Set iq scene, main: %s, sub: %s\n", ctx->main_scene[0], ctx->sub_scene[0]);
            }
            break;
        case '9':
            if (parse_switch_scene_val(optarg, ctx->main_scene[0], ctx->sub_scene[0],
                                   ctx->main_scene[1], ctx->sub_scene[1]) < 0) {
                ERR("Invalid iq scene value: %s\n", optarg);
                exit(-1);
            } else {
                DBG("Set switch scene %s:%s to %s:%s\n", ctx->main_scene[0], ctx->sub_scene[0],
                    ctx->main_scene[1], ctx->sub_scene[1]);
                ctx->ctl_type = TEST_CTL_TYPE_REPEAT_SWITCHSCENE_PREPARE_START_STOP;
            }
            break;
        case '?':
        case 'p':
            ERR("Usage: %s to capture rkisp1 frames\n"
                "         --width,  default 640,             optional, width of image\n"
                "         --height, default 480,             optional, height of image\n"
                "         --format, default NV12,            optional, fourcc of format\n"
                "         --count,  default 1000,            optional, how many frames to capture\n"
                "         --device,                          required, path of video device1\n"
                "         --device2,                         required, path of video device2\n"
                "         --device3,                         required, path of video device3\n"
                "         --device4,                         required, path of video device4\n"
                "         --stream-to,                       optional, output file path, if <file> is '-', then the data is written to stdout\n"
                "         --stream-count, default 3          optional, how many frames to write files\n"
                "         --stream-skip, default 30          optional, how many frames to skip befor writing file\n"
                "         --vop,                             optional, drm display\n"
                "                                            vop=2 -> display to DSI, others -> display to HDMI\n"
                "         --rkaiq,                           optional, auto image quality\n"
                "         --silent,                          optional, subpress debug log\n"
                "         --pponeframe,                      optional, pp oneframe readback mode\n"
                "         --hdr <val>,                       optional, hdr mode, val 2 means hdrx2, 3 means hdrx3 \n"
                "         --sync-to-raw,                     optional, write yuv files in sync with raw\n"
                "         --limit,                           optional, yuv limit range\n"
                "         --ctl <val>,                       optional, sysctl procedure test \n"
                "         --iqpath <val>,                    optional, absolute path of iq file dir \n"
                "         --orp <raw_dir,w:h:raw_fmt>,       optional, absolute path of raw files dir \n"
                "                                            raw_fmt: BG10 -> SBGGR10, GB10 -> SGBRG10 \n"
                "                                                     BA10 -> SGRBG10, RG10 -> SRGGB10 \n"
                "                                            rawbuf_type: RAW_ADDR, RAW_FD, RAW_DATA, RAW_FILE \n"
                "         --stream-poll                      use non-blocking mode and select() to stream.\n"
                "         --aov                              optional, use aov mode.\n"
                "         --aov-continue, default 30         optional, sequential frame mode run count\n"
                "         --aov-loop, default 30             optional, one frame mode run count\n"
                "         --set-scene                        optional, main_scene=<val>,sub_scene=<val>\n"
                "                                            main_scene and sub_scene is the name of scene in iq file\n"
                "         --switch-scene-loop                optional, main_scene=<val>,sub_scene=<val>,main_scene1=<val>,sub_scene1=<val>"
                "                                            main_scene and sub_scene is the name of scene in iq file\n"
                "                                            loop switch between scene and scene1 every --count frames\n"
                "         --sensor,  default os04a10,        optional, optional, sensor names\n",
                argv[0]);
            exit(-1);

        default:
            ERR("?? getopt returned character code 0%o ??\n", c);
        }
    }

    if (strlen(ctx->dev_name) == 0) {
        ERR("%s: arguments --output and --device are required\n", get_sensor_name(ctx));
        //exit(-1);
    }

}

static void deinit(demo_context_t *ctx)
{
    _if_quit = true;
    //if (!ctx->camgroup_ctx)
    stop_capturing(ctx);

    if (ctx->pponeframe)
        stop_capturing_pp_oneframe(ctx);
    if (ctx->aiq_ctx) {
        printf("%s:-------- stop aiq -------------\n", get_sensor_name(ctx));
        rk_aiq_uapi2_sysctl_stop(ctx->aiq_ctx, false);
    } else if (ctx->camgroup_ctx) {
        if (ctx->dev_using == 1) {
            printf("%s:-------- stop aiq camgroup -------------\n", get_sensor_name(ctx));
            rk_aiq_uapi2_camgroup_stop(ctx->camgroup_ctx);
#ifdef CUSTOM_GROUP_AE_DEMO_TEST
            rk_aiq_uapi2_ae_unRegister((const rk_aiq_sys_ctx_t*)(ctx->camgroup_ctx));
#endif
#ifdef CUSTOM_GROUP_AWB_DEMO_TEST
            rk_aiq_uapi2_customAWB_unRegister((const rk_aiq_sys_ctx_t*)(ctx->camgroup_ctx));
#endif

        }
    }

    if (ctx->aiq_ctx) {
        printf("%s:-------- deinit aiq -------------\n", get_sensor_name(ctx));
#ifdef CUSTOM_AE_DEMO_TEST
        //rk_aiq_AELibunRegCallBack(ctx->aiq_ctx, 0);
        rk_aiq_uapi2_ae_unRegister(ctx->aiq_ctx);
#endif
#ifdef CUSTOM_AWB_DEMO_TEST
        //rk_aiq_AELibunRegCallBack(ctx->aiq_ctx, 0);
        rk_aiq_uapi2_customAWB_unRegister(ctx->aiq_ctx);
#endif
        rk_aiq_uapi2_sysctl_deinit(ctx->aiq_ctx);
        printf("%s:-------- deinit aiq end -------------\n", get_sensor_name(ctx));
    } else if (ctx->camgroup_ctx) {
        if (ctx->dev_using == 1) {
            printf("%s:-------- deinit aiq camgroup -------------\n", get_sensor_name(ctx));
            rk_aiq_uapi2_camgroup_destroy(ctx->camgroup_ctx);
            ctx->camgroup_ctx = NULL;
            printf("%s:-------- deinit aiq camgroup end -------------\n", get_sensor_name(ctx));
        }
    }

#ifdef ISPFEC_API
    rk_ispfec_api_deinit(g_ispfec_ctx);
    g_ispfec_ctx = NULL;
#endif

    uninit_device(ctx);
    if (ctx->pponeframe)
        uninit_device_pp_oneframe(ctx);
    close_device(ctx);
    if (ctx->pponeframe)
        close_device_pp_oneframe(ctx);

    while (!list_empty(&ctx->queue)) {
        fakecam_rawbuffer* buf = list_first_entry(&ctx->queue, fakecam_rawbuffer, list);
        list_del(&buf->list);
    }

    if (ctx->fp)
        fclose(ctx->fp);
}
static void signal_handle(int signo)
{
    printf("force exit signo %d !!!\n", signo);

    if (g_main_ctx) {
        g_main_ctx->frame_count = 0;
        stop_capturing(g_main_ctx);
        if (g_main_ctx->camGroup && g_second_ctx)
            stop_capturing(g_second_ctx);
        deinit(g_main_ctx);
        g_main_ctx = NULL;
    }
    if (g_second_ctx) {
        g_second_ctx->frame_count = 0;
        deinit(g_second_ctx);
        g_second_ctx = NULL;
    }
    exit(0);
}

static void* test_thread(void* args) {
    pthread_detach (pthread_self());
    disable_terminal_return();
    printf("begin test imgproc\n");
    while(!_if_quit) {
        // test_imgproc((demo_context_t*) args);
        sample_main(args);
    }
    printf("end test imgproc\n");
    restore_terminal_settings();
    return 0;
}

#if 0
static long long findDigitsByKeyword(std::string & str)
{
    std::string searchString = "frame";
    size_t found = str.find(searchString);
    if (found != std::string::npos) {
        std::string numberPart = str.substr(found + searchString.size());

        std::string::size_type sz;
        return std::stoll(numberPart, &sz);
    } else {
        printf("`frame` not found in the string.\n");
    }

    return 0;
}

static long long findLastDigits(std::string & str)
{
    int32_t lastDigitPos = str.size() - 1;
    while (lastDigitPos >= 0 && !std::isdigit(str[lastDigitPos])) {
        --lastDigitPos;
    }

    if (lastDigitPos >= 0) {
        int lastNonDigitPos = lastDigitPos - 1;
        while (lastNonDigitPos >= 0 && std::isdigit(str[lastNonDigitPos])) {
            --lastNonDigitPos;
        }

        std::string lastDigits = str.substr(lastNonDigitPos + 1, lastDigitPos - lastNonDigitPos);

        std::string::size_type sz;
        return std::stoll(lastDigits, &sz);
    }

    return 0;
}
#endif

#if defined(ANDROID)
#include <ctype.h>

int strverscmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        int diff = 0;
        while (*s1 && *s2 && !diff) {
            char c1 = tolower((unsigned char)*s1);
            char c2 = tolower((unsigned char)*s2);
            if (isdigit(c1) && isdigit(c2)) {
                int num1 = 0;
                int num2 = 0;
                while (c1 && isdigit(c1)) {
                    num1 = num1 * 10 + (c1 - '0');
                    c1 = tolower((unsigned char)*++s1);
                }
                while (c2 && isdigit(c2)) {
                    num2 = num2 * 10 + (c2 - '0');
                    c2 = tolower((unsigned char)*++s2);
                }
                if (num1 != num2) {
                    return num1 - num2;
                }
            } else {
                diff = c1 - c2;
                s1++;
                s2++;
            }
        }
        if (diff) {
            return diff;
        }
    }
    return *s1 - *s2;
}
#endif

int compare_versions(const void *a, const void *b) {
    const char *version1 = *(const char **)a;
    const char *version2 = *(const char **)b;
    int result = strverscmp(version1, version2);
    return result;
}

// functions versionsort
static void getVersionFiles(char* dir, char** raw_files, int *filenum) {
    FILE *fp;
    char path[1024];
    int raw_flies_num = 0;

    char cmd[512] = { "ls "};
    strcat(cmd, dir);
    fp = popen(cmd, "r");
    if (fp == NULL) {
        printf("Failed to run command\n" );
                exit(1);
    }

    while (fgets(path, sizeof(path)-1, fp) != NULL) {
        if (strstr(path, ".raw") || strstr(path, ".rkraw"))
            raw_files[raw_flies_num++] = strdup(path);
    }

    qsort(raw_files, raw_flies_num, sizeof(char*), compare_versions);

    pclose(fp);
    *filenum =  raw_flies_num;
}

static int enqueueRkRawBuf(demo_context_t* demo_ctx, const char* full_name) {
    FILE* ofp = fopen(full_name, "rb");
    if (!ofp) {
        ERR("Failed to open file %s\n", full_name);
        return -1;
    }

    struct stat file_stat;
    if (fstat(fileno(ofp), &file_stat) == -1) {
        ERR("Failed to get file size\n");
        goto clean_up;
    }

    if (!demo_ctx->rawBufs[0].vaddr) {
        for (int i = 0; i < RAWBUF_MAX_FRAME; i++) {
            void* buf = malloc(file_stat.st_size);
            if (!buf) {
                ERR("Failed to malloc size %" PRId64 "\n", file_stat.st_size);
                goto clean_up;
            }

            demo_ctx->rawBufs[i].index = i;
            demo_ctx->rawBufs[i].vaddr = buf;

            list_add_tail(&demo_ctx->rawBufs[i].list, &demo_ctx->queue);
        }
    }

#ifdef LOOP_QUEUE_ONE_RAW_IMAGE
    size_t bytesRead = fread(demo_ctx->rawBufs[0].vaddr, 1, file_stat.st_size, ofp);
    if (bytesRead <= 0) {
        ERR("Failed to read file %s\n", full_name);
        goto clean_up;
    }

    for (int i = 1; i < RAWBUF_MAX_FRAME; i++)
        memcpy(demo_ctx->rawBufs[i].vaddr, demo_ctx->rawBufs[0].vaddr, bytesRead);

    while (!demo_ctx->orpStop) {
        pthread_mutex_lock(&demo_ctx->mutex);
        if (!list_empty(&demo_ctx->queue)) {
            fakecam_rawbuffer* buf =
                list_first_entry(&demo_ctx->queue,
                        fakecam_rawbuffer,
                        list);
            list_del(&buf->list);

            pthread_mutex_unlock(&demo_ctx->mutex);

            if (rk_aiq_uapi2_sysctl_enqueueRkRawBuf(demo_ctx->aiq_ctx, buf->vaddr, false) < 0) {
                ERR("Failed to get enqueueRkRawBuf\n");
                goto clean_up;
            }
        } else {
            pthread_mutex_unlock(&demo_ctx->mutex);
            usleep(5 * 1000);
        }
    }
#else
    pthread_mutex_lock(&demo_ctx->mutex);
    if (!list_empty(&demo_ctx->queue)) {
        fakecam_rawbuffer* buf =
            list_first_entry(&demo_ctx->queue,
                    fakecam_rawbuffer,
                    list);
        list_del(&buf->list);
        pthread_mutex_unlock(&demo_ctx->mutex);

        size_t bytesRead = fread(buf->vaddr, 1, file_stat.st_size, ofp);
        if (bytesRead <= 0) {
            ERR("Failed to read file %s\n", full_name);
            goto clean_up;
        }

        if (rk_aiq_uapi2_sysctl_enqueueRkRawBuf(demo_ctx->aiq_ctx, buf->vaddr, false) < 0) {
            ERR("Failed to get enqueueRkRawBuf\n");
        }
    } else {
        pthread_mutex_unlock(&demo_ctx->mutex);
        usleep(5 * 1000);
    }
#endif

    if (ofp)
        fclose(ofp);

    return 0;

clean_up:
    if (ofp)
        fclose(ofp);

    return -1;
}

static void* test_offline_thread(void* args) {
    pthread_detach (pthread_self());
    demo_context_t* demo_ctx = (demo_context_t*) args;
    char* raw_files[512];
    int raw_flies_num = 0;
    if (strlen(demo_ctx->orppath)) {
        getVersionFiles(demo_ctx->orppath, raw_files, &raw_flies_num);
    }

    while (!demo_ctx->orpStop) {
        for (int i = 0; i < raw_flies_num && !demo_ctx->orpStop; i++) {
            char full_name[512];
            strcpy(full_name, demo_ctx->orppath);
            strcat(full_name, raw_files[i]);
            int length = strlen(full_name);
            if (length > 0 && full_name[length - 1] == '\n') {
                full_name[length - 1] = '\0';
            }
            DBG("process raw : %s \n", full_name);

            if (demo_ctx->orpRawBufType == RK_AIQ_RAW_FILE) {
                if (!demo_ctx->camGroup)
                    rk_aiq_uapi2_sysctl_enqueueRkRawFile(demo_ctx->aiq_ctx, full_name);
                else
                    rk_aiq_uapi2_sysctl_enqueueRkRawFile((const rk_aiq_sys_ctx_t*)demo_ctx->camgroup_ctx, full_name);
                usleep(500000);
            } else if (demo_ctx->orpRawBufType == RK_AIQ_RAW_DATA) {
                enqueueRkRawBuf(demo_ctx, full_name);
            }
        }
        usleep(500000);
    }

    for (int32_t i = 0; i < RAWBUF_MAX_FRAME; i++) {
        if (demo_ctx->rawBufs[i].vaddr)
            free(demo_ctx->rawBufs[i].vaddr);
        demo_ctx->rawBufs[i].vaddr = NULL;
    }

    for (int i = 0; i < raw_flies_num;i++) {
       if (raw_files[i]) {
           free(raw_files[i]);
           raw_files[i] = NULL;
        }
    }
    demo_ctx->orpStopped = true;
    return 0;
}

#if 0
static int set_ae_onoff(const rk_aiq_sys_ctx_t* ctx, bool onoff)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    Uapi_ExpSwAttr_t expSwAttr;

    ret = rk_aiq_user_api_ae_getExpSwAttr(ctx, &expSwAttr);
    expSwAttr.enable = onoff;
    ret = rk_aiq_user_api_ae_setExpSwAttr(ctx, expSwAttr);

    return 0;
}
#endif

static int query_ae_state(const rk_aiq_sys_ctx_t* ctx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V39) || defined(ISP_HW_V33) || defined(ISP_HW_V35)
    ae_api_queryInfo_t queryInfo;
    ret = rk_aiq_user_api2_ae_queryExpResInfo(ctx, &queryInfo);
    printf("ae IsConverged: %d\n", queryInfo.isConverged);
#else
    Uapi_ExpQueryInfo_t queryInfo;
    ret =  rk_aiq_user_api2_ae_queryExpResInfo(ctx, &queryInfo);
    printf("ae IsConverged: %d\n", queryInfo.IsConverged);
#endif

    return 0;
}

static void* stats_thread(void* args) {
    demo_context_t* ctx =  (demo_context_t*)args;
    XCamReturn ret;
    pthread_detach (pthread_self());
    printf("begin stats thread\n");

    while(!_if_quit) {
#if (USE_NEWSTRUCT == 0)
        rk_aiq_isp_stats_t *stats_ref = NULL;
        ret = rk_aiq_uapi2_sysctl_get3AStatsBlk(ctx->aiq_ctx, &stats_ref, -1);
        if (ret == XCAM_RETURN_NO_ERROR && stats_ref != NULL) {
            printf("get one stats frame id %d \n", stats_ref->frame_id);
            query_ae_state(ctx->aiq_ctx);
            rk_aiq_uapi2_sysctl_release3AStatsRef(ctx->aiq_ctx, stats_ref);
#else
        rk_aiq_isp_statistics_t stats_ref;
        stats_ref.bValid_aec_stats = false;
        stats_ref.bValid_awb_stats = false;

        ret = rk_aiq_uapi2_stats_getIspStats(ctx->aiq_ctx, &stats_ref, -1);
        if (ret == XCAM_RETURN_NO_ERROR) {
            // do nothing
            printf("stats fid:%d, valid:%d,%d\n", stats_ref.frame_id,
                  stats_ref.bValid_aec_stats,
                  stats_ref.bValid_awb_stats);
            query_ae_state(ctx->aiq_ctx);
#endif
        } else {
            if (ret == XCAM_RETURN_NO_ERROR) {
                printf("aiq has stopped !\n");
                break;
            } else if (ret == XCAM_RETURN_ERROR_TIMEOUT) {
                printf("aiq timeout!\n");
                continue;
            } else if (ret == XCAM_RETURN_ERROR_FAILED) {
                printf("aiq failed!\n");
                break;
            }
        }
    }
    printf("end stats thread\n");
    return 0;
}

void release_buffer(void *addr) {
    // DBG("release buffer called: addr=%p\n", addr);
    if (addr) {
        pthread_mutex_lock(&g_main_ctx->mutex);
        for (int32_t i = 0; i < RAWBUF_MAX_FRAME; i++) {
            if (g_main_ctx->rawBufs[i].vaddr == addr) {
                list_add_tail(&g_main_ctx->rawBufs[i].list, &g_main_ctx->queue);
                break;
            }
        }
        pthread_mutex_unlock(&g_main_ctx->mutex);
    }
}

static void test_tuning_api(demo_context_t *ctx)
{
    /*   Sample code of API rk_aiq_uapi2_sysctl_tuning
     * This API replaces current iq params through JsonPatch.
     * 1. Must be called after Api rk_aiq_uapi2_sysctl_init, prefer to be
     * called before rk_aiq_uapi2_sysctl_prepare
     * 2. Notice that this API only replaces the current iq params in use, and
     * the iq file would not be modified.
     *    Supported path including:
     * path：/sensor_calib
     *       /module_calib
     *       And submodules in node 'scene_isp30' , such as
     *       /wb_v21  notice that not /main_scene/0/sbu_scene/0/wb_v21, cause
     *                there is no scene contex for in-use iq params.
     * 'path' could refer to iq json file
     * 3.
     *  3.1 Prefer to replace the all params of same iq node in one JsonPatch
     *  3.2 prefer to replace the params of diffrent iq node in separate JsonPatch
     * 4. JsonPatch string format:
     *    [// '[' start syntax
     *      {  // each 'op' embraced by '{}'
              "op": "replace", // Fixed syntax
              "path":          // node path in Json iq file
              "value":         // Note: All chidren's valuse should be specified for the 'path', otherwise those unspecified ones
                               //       may be uninitialzed.
     *      }, { //  ',' for next 'op'

     *      } // Note: no ',' for last 'op'
     *    ]// ']' end syntax
     */
    // example 1: replace sensor info
    // 1. Show how to repalce the non-leaf node "sensor_calib/resolution"
    //    values should be embraced by '{}' for non-leaf node, and all children should be set.
    // 2. Show how to replace the values for multiple sub nodes of same parent in one JsonPatch
    const char* json_sensor_str = " \n\
        [{ \n\
            \"op\":\"replace\", \n\
            \"path\": \"/sensor_calib/resolution\", \n\
            \"value\": \n\
            { \"width\": 2222, \"height\": 2160} \n\
        }, { \n\
            \"op\":\"replace\", \n\
            \"path\": \"/sensor_calib/CISFlip\", \n\
            \"value\": 6\n\
        }]";

    printf("%s\n", json_sensor_str);
    rk_aiq_uapi2_sysctl_tuning(ctx->aiq_ctx, (char*)json_sensor_str);

    // example 2: replace awb info
    // 1. Show how to replace the values for type array
    //    value shoulde be embraced by '[]' for type array, and the target
    //    length of the array should be unchanged.
    // 2. Show how to replace the value for one array item
    //    the path for array item is: array name/array index/
    const char* json_awb_str = " \n\
        [{ \n\
            \"op\":\"replace\", \n\
            \"path\": \"/wb_v21/autoExtPara/wbGainClip/cct\", \n\
            \"value\": \n\
            [100,200,300,40,50,60] \n\
        },{ \n\
            \"op\":\"replace\", \n\
            \"path\": \"/wb_v21/autoPara/lightSources/0/name\", \n\
            \"value\": \"aaaaaaaaa\" \n\
        }]";
    printf("%s\n", json_awb_str);
    rk_aiq_uapi2_sysctl_tuning(ctx->aiq_ctx, (char*)json_awb_str);

    printf("%s done ..\n", __func__);
}

static void rkisp_routine(demo_context_t *ctx)
{
    char sns_entity_name[64];
    rk_aiq_working_mode_t work_mode = RK_AIQ_WORKING_MODE_NORMAL;

    if (ctx->hdrmode == 2)
        work_mode = RK_AIQ_WORKING_MODE_ISP_HDR2;
    else if (ctx->hdrmode == 3)
        work_mode = RK_AIQ_WORKING_MODE_ISP_HDR3;

    printf("work_mode %d\n", work_mode);

    if (ctx->rkaiq) {
        strcpy(sns_entity_name, rk_aiq_uapi2_sysctl_getBindedSnsEntNmByVd(get_dev_name(ctx)));
        printf("sns_entity_name:%s\n", sns_entity_name);
        sscanf(&sns_entity_name[6], "%s", ctx->sns_name);
        printf("sns_name:%s\n", ctx->sns_name);
        rk_aiq_static_info_t s_info;
        rk_aiq_uapi2_sysctl_getStaticMetas(sns_entity_name, &s_info);
        // check if hdr mode is supported
        if (!ctx->isOrp && work_mode != 0) {
            bool b_work_mode_supported = false;
            rk_aiq_sensor_info_t* sns_info = &s_info.sensor_info;
            for (int i = 0; i < SUPPORT_FMT_MAX; i++)
                // TODO, should decide the resolution firstly,
                // then check if the mode is supported on this
                // resolution
                if (((sns_info->support_fmt[i].hdr_mode == 5/*HDR_X2*/ ||
                      sns_info->support_fmt[i].hdr_mode == 7/*HDR_COMPR*/) &&
                      work_mode == RK_AIQ_WORKING_MODE_ISP_HDR2) ||
                    ((sns_info->support_fmt[i].hdr_mode == 6/*HDR_X3*/ ||
                      sns_info->support_fmt[i].hdr_mode == 7/*HDR_COMPR*/) &&
                      work_mode == RK_AIQ_WORKING_MODE_ISP_HDR3)) {
                    b_work_mode_supported = true;
                    break;
                }

            if (!b_work_mode_supported) {
                printf("\nWARNING !!!"
                    "work mode %d is not supported, changed to normal !!!\n\n",
                    work_mode);
                work_mode = RK_AIQ_WORKING_MODE_NORMAL;
            }
        }
    }

    printf("%s:-------- open output dev -------------\n", get_sensor_name(ctx));
    open_device(ctx);
    if (ctx->pponeframe)
        open_device_pp_oneframe(ctx);

    if (ctx->writeFile) {
        ctx->fp = fopen(ctx->out_file, "w+");
        if (ctx->fp == NULL) {
            ERR("%s: fopen output file %s failed!\n", get_sensor_name(ctx), ctx->out_file);
        }
    }

    if (ctx->rkaiq) {
        XCamReturn ret = XCAM_RETURN_NO_ERROR;
        if (strlen(ctx->main_scene[0]) > 0 && strlen(ctx->sub_scene[0]) > 0) {
            DBG("%s: set scene %s, %s\n", get_sensor_name(ctx), ctx->main_scene[0],
                ctx->sub_scene[0]);
            ret = rk_aiq_uapi2_sysctl_preInit_scene(sns_entity_name, ctx->main_scene[0],
                                               ctx->sub_scene[0]);
        } else {
            if (work_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                ret = rk_aiq_uapi2_sysctl_preInit_scene(sns_entity_name, "normal", "day");
            } else {
                ret = rk_aiq_uapi2_sysctl_preInit_scene(sns_entity_name, "hdr", "day");
            }
        }
        if (ret < 0)
            ERR("%s: failed to set %s scene\n",
                get_sensor_name(ctx),
                work_mode == RK_AIQ_WORKING_MODE_NORMAL ? "normal" : "hdr");

        if (strlen(ctx->iqpath))
        {
            if (!ctx->camGroup)
                ctx->aiq_ctx = rk_aiq_uapi2_sysctl_init(sns_entity_name, ctx->iqpath, NULL, NULL);
            else {
                // create once for mulitple cams
                if (ctx->dev_using == 1) {
                    char sns_entity_name2[64] = {'\0'};
                    char sns_entity_name3[64] = {'\0'};
                    char sns_entity_name4[64] = {'\0'};
                    char sns_entity_name5[64] = {'\0'};
                    bool has_dev2 = false;
                    bool has_dev3 = false;
                    bool has_dev4 = false;
                    bool has_dev5 = false;
                    if (strlen(ctx->dev_name2)) {
                        strcpy(sns_entity_name2, rk_aiq_uapi2_sysctl_getBindedSnsEntNmByVd(ctx->dev_name2));
                        printf("sns_entity_name2:%s\n", sns_entity_name2);
                        //sscanf(&sns_entity_name2[6], "%s", ctx->sns_name);
                        //printf("sns_name2:%s\n", ctx->sns_name);
                        has_dev2 = true;
                    }
                    if (strlen(ctx->dev_name3)) {
                        strcpy(sns_entity_name3, rk_aiq_uapi2_sysctl_getBindedSnsEntNmByVd(ctx->dev_name3));
                        printf("sns_entity_name3:%s\n", sns_entity_name3);
                        //sscanf(&sns_entity_name2[6], "%s", ctx->sns_name);
                        //printf("sns_name2:%s\n", ctx->sns_name);
                        has_dev3 = true;
                    }
                    if (strlen(ctx->dev_name4)) {
                        strcpy(sns_entity_name4, rk_aiq_uapi2_sysctl_getBindedSnsEntNmByVd(ctx->dev_name4));
                        printf("sns_entity_name4:%s\n", sns_entity_name3);
                        //sscanf(&sns_entity_name2[6], "%s", ctx->sns_name);
                        //printf("sns_name2:%s\n", ctx->sns_name);
                        has_dev4 = true;
                    }
                    if (strlen(ctx->dev_name5)) {
                        strcpy(sns_entity_name5, rk_aiq_uapi2_sysctl_getBindedSnsEntNmByVd(ctx->dev_name5));
                        printf("sns_entity_name5:%s\n", sns_entity_name5);
                        //sscanf(&sns_entity_name2[6], "%s", ctx->sns_name);
                        //printf("sns_name2:%s\n", ctx->sns_name);
                        has_dev5 = true;
                    }

                    rk_aiq_camgroup_instance_cfg_t camgroup_cfg;
                    memset(&camgroup_cfg, 0, sizeof(camgroup_cfg));
                    camgroup_cfg.sns_num = 1;
                    if (has_dev2)
                        camgroup_cfg.sns_num++;
                    if (has_dev3)
                        camgroup_cfg.sns_num++;
                    if (has_dev4)
                        camgroup_cfg.sns_num++;
                    if (has_dev5)
                        camgroup_cfg.sns_num++;
                    camgroup_cfg.sns_ent_nm_array[0] = sns_entity_name;
                    if (has_dev2)
                        camgroup_cfg.sns_ent_nm_array[1] = sns_entity_name2;
                    if (has_dev3)
                        camgroup_cfg.sns_ent_nm_array[2] = sns_entity_name3;
                    if (has_dev4)
                        camgroup_cfg.sns_ent_nm_array[3] = sns_entity_name4;
                    if (has_dev5)
                        camgroup_cfg.sns_ent_nm_array[4] = sns_entity_name5;
                    camgroup_cfg.config_file_dir = ctx->iqpath;
                    camgroup_cfg.overlap_map_file = "srcOverlapMap.bin";
                    ctx->camgroup_ctx = rk_aiq_uapi2_camgroup_create(&camgroup_cfg);
                    if (!ctx->camgroup_ctx) {
                        printf("create camgroup ctx error !\n");
                        exit(1);
                    }

#ifdef CUSTOM_GROUP_AE_DEMO_TEST
                    rk_aiq_pfnAe_t cbs = {
                        .pfn_ae_init = custom_ae_init,
                        .pfn_ae_run = custom_ae_run,
                        .pfn_ae_ctrl = custom_ae_ctrl,
                        .pfn_ae_exit = custom_ae_exit,
                    };
                    rk_aiq_uapi2_ae_register((const rk_aiq_sys_ctx_t*)(ctx->camgroup_ctx), &cbs);
                    //rk_aiq_uapi2_customAE_enable((const rk_aiq_sys_ctx_t*)(ctx->camgroup_ctx), true);
#endif
#ifdef CUSTOM_GROUP_AWB_DEMO_TEST
                    rk_aiq_customeAwb_cbs_t awb_cbs = {
                        .pfn_awb_init = custom_awb_init,
                        .pfn_awb_run = custom_awb_run,
                        .pfn_awb_ctrl = custom_awb_ctrl,
                        .pfn_awb_exit = custom_awb_exit,
                    };
                    rk_aiq_uapi2_customAWB_register((const rk_aiq_sys_ctx_t*)(ctx->camgroup_ctx), &awb_cbs);
                    rk_aiq_uapi2_customAWB_enable((const rk_aiq_sys_ctx_t*)(ctx->camgroup_ctx), true);
#endif

                }
            }
        } else {
            if (ctx->camGroup) {
                printf("error! should specify iq path !\n");
                exit(1);
            }
#ifndef ANDROID
            rk_aiq_uapi2_sysctl_preInit(sns_entity_name, RK_AIQ_WORKING_MODE_NORMAL, "ov5695_TongJu_CHT842-MD.xml");
            ctx->aiq_ctx = rk_aiq_uapi2_sysctl_init(sns_entity_name, "/oem/etc/iqfiles", NULL, NULL);
#else
            ctx->aiq_ctx = rk_aiq_uapi2_sysctl_init(sns_entity_name, "/data/etc/iqfiles", NULL, NULL);
#endif

        }
        if (ctx->aiq_ctx) {
            printf("%s:-------- init mipi tx/rx -------------\n", get_sensor_name(ctx));
            if (ctx->writeFileSync)
                rk_aiq_uapi2_debug_captureRawYuvSync(ctx->aiq_ctx, CAPTURE_RAW_AND_YUV_SYNC);
#ifdef CUSTOM_AE_DEMO_TEST
            //ae_reg.stAeExpFunc.pfn_ae_init = ae_init;
            //ae_reg.stAeExpFunc.pfn_ae_run = ae_run;
            //ae_reg.stAeExpFunc.pfn_ae_ctrl = ae_ctrl;
            //ae_reg.stAeExpFunc.pfn_ae_exit = ae_exit;
            //rk_aiq_AELibRegCallBack(ctx->aiq_ctx, &ae_reg, 0);
            rk_aiq_pfnAe_t cbs = {
                .pfn_ae_init = custom_ae_init,
                .pfn_ae_run = custom_ae_run,
                .pfn_ae_ctrl = custom_ae_ctrl,
                .pfn_ae_exit = custom_ae_exit,
            };
            rk_aiq_uapi2_ae_register(ctx->aiq_ctx, &cbs);
            //rk_aiq_uapi2_ae_enable(ctx->aiq_ctx, true);
#endif
#ifdef CUSTOM_AWB_DEMO_TEST
            rk_aiq_customeAwb_cbs_t awb_cbs = {
                .pfn_awb_init = custom_awb_init,
                .pfn_awb_run = custom_awb_run,
                .pfn_awb_ctrl = custom_awb_ctrl,
                .pfn_awb_exit = custom_awb_exit,
            };
            rk_aiq_uapi2_customAWB_register(ctx->aiq_ctx, &awb_cbs);
            rk_aiq_uapi2_customAWB_enable(ctx->aiq_ctx, true);
#endif
            if (ctx->isOrp) {
                rk_aiq_raw_prop_t prop;
                if (strcmp(ctx->orpRawFmt, "BA81") == 0)
                    prop.format = RK_PIX_FMT_SBGGR8;
                else if (strcmp(ctx->orpRawFmt, "GBRG") == 0)
                    prop.format = RK_PIX_FMT_SGBRG8;
                else if (strcmp(ctx->orpRawFmt, "RGGB") == 0)
                    prop.format = RK_PIX_FMT_SRGGB8;
                else if (strcmp(ctx->orpRawFmt, "GRBG") == 0)
                    prop.format = RK_PIX_FMT_SGRBG8;
                else if (strcmp(ctx->orpRawFmt, "BG10") == 0)
                    prop.format = RK_PIX_FMT_SBGGR10;
                else if (strcmp(ctx->orpRawFmt, "GB10") == 0)
                    prop.format = RK_PIX_FMT_SGBRG10;
                else if (strcmp(ctx->orpRawFmt, "RG10") == 0)
                    prop.format = RK_PIX_FMT_SRGGB10;
                else if (strcmp(ctx->orpRawFmt, "BA10") == 0)
                    prop.format = RK_PIX_FMT_SGRBG10;
                else if (strcmp(ctx->orpRawFmt, "BG12") == 0)
                    prop.format = RK_PIX_FMT_SBGGR12;
                else if (strcmp(ctx->orpRawFmt, "GB12") == 0)
                    prop.format = RK_PIX_FMT_SGBRG12;
                else if (strcmp(ctx->orpRawFmt, "RG12") == 0)
                    prop.format = RK_PIX_FMT_SRGGB12;
                else if (strcmp(ctx->orpRawFmt, "BA12") == 0)
                    prop.format = RK_PIX_FMT_SGRBG12;
                else if (strcmp(ctx->orpRawFmt, "BG14") == 0)
                    prop.format = RK_PIX_FMT_SBGGR14;
                else if (strcmp(ctx->orpRawFmt, "GB14") == 0)
                    prop.format = RK_PIX_FMT_SGBRG14;
                else if (strcmp(ctx->orpRawFmt, "RG14") == 0)
                    prop.format = RK_PIX_FMT_SRGGB14;
                else if (strcmp(ctx->orpRawFmt, "BA14") == 0)
                    prop.format = RK_PIX_FMT_SGRBG14;
                else if (strcmp(ctx->orpRawFmt, "BYR2") == 0)
                    prop.format = RK_PIX_FMT_SBGGR16;
                else if (strcmp(ctx->orpRawFmt, "GB16") == 0)
                    prop.format = RK_PIX_FMT_SGBRG16;
                else if (strcmp(ctx->orpRawFmt, "GR16") == 0)
                    prop.format = RK_PIX_FMT_SGRBG16;
                else if (strcmp(ctx->orpRawFmt, "RG16") == 0)
                    prop.format = RK_PIX_FMT_SRGGB16;
                else
                    prop.format = RK_PIX_FMT_SBGGR10;
                prop.frame_width = ctx->orpRawW;
                prop.frame_height = ctx->orpRawH;
                prop.rawbuf_type = ctx->orpRawBufType;
                rk_aiq_uapi2_sysctl_prepareRkRaw(ctx->aiq_ctx, prop);
            }
            /*
             * rk_aiq_uapi_setFecEn(ctx->aiq_ctx, true);
             * rk_aiq_uapi_setFecCorrectDirection(ctx->aiq_ctx, FEC_CORRECT_DIRECTION_Y);
             */
#ifdef TEST_MEMS_SENSOR_INTF
            rk_aiq_mems_sensor_intf_t g_rkiio_aiq_api;
            rk_aiq_uapi2_sysctl_regMemsSensorIntf(ctx->aiq_ctx, &g_rkiio_aiq_api);
#endif

#if 0
            test_tuning_api(ctx);
#endif

#ifdef OTP_API_TEST
            rk_aiq_user_otp_info_t otp_info = {};
            otp_info.otp_awb.flag = true;
            otp_info.otp_awb.r_value = 548;
            otp_info.otp_awb.b_value = 521;
            otp_info.otp_awb.gr_value = 1021;
            otp_info.otp_awb.gb_value = -1;
            otp_info.otp_awb.golden_r_value = 532;
            otp_info.otp_awb.golden_b_value = 529;
            otp_info.otp_awb.golden_gr_value = 1020;
            otp_info.otp_awb.golden_gb_value = -1;

            otp_info.otp_lsc.flag = true;
            otp_info.otp_lsc.decimal_bits = 10;
            otp_info.otp_lsc.width = 4080;
            otp_info.otp_lsc.height = 3072;
            otp_info.otp_lsc.table_size = 2312;
            otp_info.otp_lsc.lsc_h = 0;
            otp_info.otp_lsc.lsc_w = 0;
            memcpy(otp_info.otp_lsc.lsc_r, lsc_r_table0, sizeof(lsc_r_table0));
            memcpy(otp_info.otp_lsc.lsc_gr, lsc_gr_table0, sizeof(lsc_gr_table0));
            memcpy(otp_info.otp_lsc.lsc_gb, lsc_gb_table0, sizeof(lsc_gb_table0));
            memcpy(otp_info.otp_lsc.lsc_b, lsc_b_table0, sizeof(lsc_b_table0));

            if (rk_aiq_uapi2_sysctl_setUserOtpInfo(ctx->aiq_ctx, otp_info) != 0) {
                ERR("Failed to set User Otp\n");
            } else {
                printf("set User Otp: flag = %d, value = [%d, %d, %d, %d], golden = [%d, %d, %d, %d];\
                        lsc flag: %d, r[0]= %d, gr[0] = %d, gb[0] = %d, b[0] = %d\n",
                    otp_info.otp_awb.flag, otp_info.otp_awb.r_value, otp_info.otp_awb.b_value,
                    otp_info.otp_awb.gr_value, otp_info.otp_awb.gb_value,
                    otp_info.otp_awb.golden_r_value, otp_info.otp_awb.golden_b_value,
                    otp_info.otp_awb.golden_gr_value, otp_info.otp_awb.golden_gb_value,
                    otp_info.otp_lsc.flag, otp_info.otp_lsc.lsc_r[0],
                    otp_info.otp_lsc.lsc_gr[0], otp_info.otp_lsc.lsc_gb[0],
                    otp_info.otp_lsc.lsc_b[0]);
            }
#endif
#ifdef COLOR_CONSISTENCY_TEST
            rk_aiq_uapiV2_awb_Slave2Main_Cfg_t slave2Main;
            slave2Main.enable = true;
            slave2Main.camM.wbgain.rgain = 1.6480  ;
            slave2Main.camM.wbgain.grgain = 1 ;
            slave2Main.camM.wbgain.gbgain = 1 ;
            slave2Main.camM.wbgain.bgain = 1.84 ;
            char filename[] = "/etc/iqfiles/wbgain_convert2.bin";
            rk_aiq_user_api2_awb_loadConvertLut(&slave2Main.cct_lut_cfg, filename);
            rk_aiq_user_api2_awb_IqMap2Main(ctx->aiq_ctx, slave2Main); //ctx->aiq_ctx is slave camera
            rk_aiq_user_api2_awb_freeConvertLut(&slave2Main.cct_lut_cfg);

            rk_aiq_color_info_t aColor_sw_info;//about main camera
            aColor_sw_info.sensorGain = 5;
            aColor_sw_info.awbGain[0] = slave2Main.camM.wbgain.rgain / slave2Main.camM.wbgain.grgain;
            aColor_sw_info.awbGain[1] = slave2Main.camM.wbgain.bgain / slave2Main.camM.wbgain.gbgain;
            ret = rk_aiq_uapi2_setAcolorSwInfo(ctx->aiq_ctx, aColor_sw_info); //ctx->aiq_ctx is slave camera
#endif
            // rk_aiq_uapi2_sysctl_initAiisp(ctx->aiq_ctx, NULL, NULL);
            XCamReturn ret = rk_aiq_uapi2_sysctl_prepare(ctx->aiq_ctx, ctx->width, ctx->height, work_mode);

            if (ret != XCAM_RETURN_NO_ERROR)
                ERR("%s:rk_aiq_uapi2_sysctl_prepare failed: %d\n", get_sensor_name(ctx), ret);
            else {
                ret = rk_aiq_uapi2_setMirrorFlip(ctx->aiq_ctx, false, false, 3);
                // Ignore failure

                if (ctx->isOrp) {
                    rk_aiq_uapi2_sysctl_registRkRawCb(ctx->aiq_ctx, release_buffer);
                }
                ret = rk_aiq_uapi2_sysctl_start(ctx->aiq_ctx );
                init_device(ctx);
                if (ctx->pponeframe)
                    init_device_pp_oneframe(ctx);
                if (ctx->ctl_type == TEST_CTL_TYPE_DEFAULT) {
                    start_capturing(ctx);
                }
                if (ctx->pponeframe)
                    start_capturing_pp_oneframe(ctx);
                printf("%s:-------- stream on mipi tx/rx -------------\n", get_sensor_name(ctx));

                if (ctx->ctl_type != TEST_CTL_TYPE_DEFAULT) {
                    static int test_ctl_cnts = 0;
                    int frame_count = 0;
restart:
                    frame_count = ctx->frame_count;
                    start_capturing(ctx);
                    while ((frame_count-- > 0))
                        read_frame(ctx);
                    stop_capturing(ctx);
                    printf("+++++++ TEST SYSCTL COUNTS %d ++++++++++++ \n", test_ctl_cnts++);
                    printf("aiq stop .....\n");
                    rk_aiq_uapi2_sysctl_stop(ctx->aiq_ctx, false);
                    if (ctx->ctl_type == TEST_CTL_TYPE_REPEAT_INIT_PREPARE_START_STOP_DEINIT) {
                        printf("aiq deinit .....\n");
                        rk_aiq_uapi2_sysctl_deinit(ctx->aiq_ctx);
                        printf("aiq init .....\n");
                        if (work_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                            //ret = rk_aiq_uapi2_sysctl_preInit_scene(sns_entity_name, "normal", "day");
                            ret = rk_aiq_uapi2_sysctl_preInit_scene(sns_entity_name, "hdr", "day");
                            if (ctx->hdrmode == 2)
                                work_mode = RK_AIQ_WORKING_MODE_ISP_HDR2;
                            else if (ctx->hdrmode == 3)
                                work_mode = RK_AIQ_WORKING_MODE_ISP_HDR3;
                        } else {
                            //ret = rk_aiq_uapi2_sysctl_preInit_scene(sns_entity_name, "hdr", "day");
                            ret = rk_aiq_uapi2_sysctl_preInit_scene(sns_entity_name, "normal", "day");
                            work_mode = RK_AIQ_WORKING_MODE_NORMAL;
                        }
                        if (ret < 0)
                            ERR("%s: failed to set %s scene\n",
                                get_sensor_name(ctx),
                                work_mode == RK_AIQ_WORKING_MODE_NORMAL? "normal" : "hdr");
                        ctx->aiq_ctx = rk_aiq_uapi2_sysctl_init(sns_entity_name, ctx->iqpath, NULL, NULL);
                        printf("aiq prepare .....\n");
                        XCamReturn ret = rk_aiq_uapi2_sysctl_prepare(ctx->aiq_ctx, ctx->width, ctx->height, work_mode);
                    } else if (ctx->ctl_type == TEST_CTL_TYPE_REPEAT_PREPARE_START_STOP) {
                        printf("aiq prepare .....\n");
                        XCamReturn ret = rk_aiq_uapi2_sysctl_prepare(ctx->aiq_ctx, ctx->width, ctx->height, work_mode);
                    } else if (ctx->ctl_type == TEST_CTL_TYPE_REPEAT_START_STOP) {
                        // do nothing
                    } else if (ctx->ctl_type == TEST_CTL_TYPE_REPEAT_SWITCHSCENE_PREPARE_START_STOP) {
                        const char* main_scene[2] = {"normal", "hdr"};
                        const char* sub_scene[2] = {"day", "day_dcgVs"};
                        static int scene_index = 0;  /* Track current scene index */
                        int custom_scenes_valid;

                        /* Use custom scenes if all are configured */
                        custom_scenes_valid = (strlen(ctx->main_scene[0]) > 0 && strlen(ctx->sub_scene[0]) > 0 &&
                                             strlen(ctx->main_scene[1]) > 0 && strlen(ctx->sub_scene[1]) > 0);
                        if (custom_scenes_valid) {
                            main_scene[0] = ctx->main_scene[0];
                            sub_scene[0] = ctx->sub_scene[0];
                            main_scene[1] = ctx->main_scene[1];
                            sub_scene[1] = ctx->sub_scene[1];
                        }

                        /* Toggle between scenes (0 and 1) */
                        scene_index = (scene_index + 1) % 2;

                        int ret = rk_aiq_uapi2_sysctl_switch_scene(ctx->aiq_ctx, main_scene[scene_index], sub_scene[scene_index]);
                        if (ret < 0) {
                            ERR("Failed to switch scene to %s:%s\n", main_scene[scene_index], sub_scene[scene_index]);
                        } else {
                            DBG("Switch scene to %s:%s (index: %d)\n", main_scene[scene_index], sub_scene[scene_index], scene_index);

                            DBG("aiq prepare .....\n");
                            int ret = rk_aiq_uapi2_sysctl_prepare(ctx->aiq_ctx, ctx->width,
                                                                         ctx->height, work_mode);
                            if (ret < 0)
                                ERR("Failed to prepare after switch scene\n");
                        }
                    }

                    printf("aiq start .....\n");
                    ret = rk_aiq_uapi2_sysctl_start(ctx->aiq_ctx);
                    printf("aiq restart .....\n");
                    goto restart;
                }
            }

        } else if (ctx->camgroup_ctx) {
            // only do once for cam group
            if (ctx->dev_using == 1) {
                rk_aiq_camgroup_camInfos_t camInfos;
                memset(&camInfos, 0, sizeof(camInfos));
                if (rk_aiq_uapi2_camgroup_getCamInfos((rk_aiq_camgroup_ctx_t *)ctx->camgroup_ctx, &camInfos) == XCAM_RETURN_NO_ERROR) {
#ifdef OTP_API_TEST
                    for (int i = 0; i < camInfos.valid_sns_num; i++) {
                        rk_aiq_sys_ctx_t* aiq_ctx = NULL;
                        aiq_ctx = rk_aiq_uapi2_camgroup_getAiqCtxBySnsNm((rk_aiq_camgroup_ctx_t *)ctx->camgroup_ctx, camInfos.sns_ent_nm[i]);
                        if (!aiq_ctx)
                            continue;

                        printf("aiq_ctx sns name: %s, camPhyId %d\n",
                               camInfos.sns_ent_nm[i], camInfos.sns_camPhyId[i]);
                        rk_aiq_user_otp_info_t otp_info = {};
                        otp_info.otp_awb.flag = true;
                        otp_info.otp_lsc.flag = true;
                        otp_info.otp_lsc.decimal_bits = 10;
                        otp_info.otp_lsc.width = 4080;
                        otp_info.otp_lsc.height = 3072;
                        otp_info.otp_lsc.table_size = 2312;
                        otp_info.otp_lsc.lsc_h = 0;
                        otp_info.otp_lsc.lsc_w = 0;
                        if (i == 0) {
                            otp_info.otp_awb.r_value = 548;
                            otp_info.otp_awb.b_value = 521;
                            otp_info.otp_awb.gr_value = 1021;
                            otp_info.otp_awb.gb_value = -1;
                            otp_info.otp_awb.golden_r_value = 532;
                            otp_info.otp_awb.golden_b_value = 529;
                            otp_info.otp_awb.golden_gr_value = 1020;
                            otp_info.otp_awb.golden_gb_value = -1;
                            memcpy(otp_info.otp_lsc.lsc_r, lsc_r_table0, sizeof(lsc_r_table0));
                            memcpy(otp_info.otp_lsc.lsc_gr, lsc_gr_table0, sizeof(lsc_gr_table0));
                            memcpy(otp_info.otp_lsc.lsc_gb, lsc_gb_table0, sizeof(lsc_gb_table0));
                            memcpy(otp_info.otp_lsc.lsc_b, lsc_b_table0, sizeof(lsc_b_table0));
                        } else {
                            otp_info.otp_awb.r_value = 1;
                            otp_info.otp_awb.b_value = 1;
                            otp_info.otp_awb.gr_value = 1;
                            otp_info.otp_awb.gb_value = -1;
                            otp_info.otp_awb.golden_r_value = 1;
                            otp_info.otp_awb.golden_b_value = 1;
                            otp_info.otp_awb.golden_gr_value = 1;
                            otp_info.otp_awb.golden_gb_value = -1;
                            memcpy(otp_info.otp_lsc.lsc_r, lsc_table1, sizeof(lsc_table1));
                            memcpy(otp_info.otp_lsc.lsc_gr, lsc_table1, sizeof(lsc_table1));
                            memcpy(otp_info.otp_lsc.lsc_gb, lsc_table1, sizeof(lsc_table1));
                            memcpy(otp_info.otp_lsc.lsc_b, lsc_table1, sizeof(lsc_table1));
                        }

                        if (rk_aiq_uapi2_sysctl_setUserOtpInfo(aiq_ctx, otp_info) != 0) {
                            printf("Failed to set User Otp\n");
                        }
                    }
#endif

                    if (ctx->isOrp) {
                        rk_aiq_raw_prop_t prop;
                        if (strcmp(ctx->orpRawFmt, "BA81") == 0)
                            prop.format = RK_PIX_FMT_SBGGR8;
                        else if (strcmp(ctx->orpRawFmt, "GBRG") == 0)
                            prop.format = RK_PIX_FMT_SGBRG8;
                        else if (strcmp(ctx->orpRawFmt, "RGGB") == 0)
                            prop.format = RK_PIX_FMT_SRGGB8;
                        else if (strcmp(ctx->orpRawFmt, "GRBG") == 0)
                            prop.format = RK_PIX_FMT_SGRBG8;
                        else if (strcmp(ctx->orpRawFmt, "BG10") == 0)
                            prop.format = RK_PIX_FMT_SBGGR10;
                        else if (strcmp(ctx->orpRawFmt, "GB10") == 0)
                            prop.format = RK_PIX_FMT_SGBRG10;
                        else if (strcmp(ctx->orpRawFmt, "RG10") == 0)
                            prop.format = RK_PIX_FMT_SRGGB10;
                        else if (strcmp(ctx->orpRawFmt, "BA10") == 0)
                            prop.format = RK_PIX_FMT_SGRBG10;
                        else if (strcmp(ctx->orpRawFmt, "BG12") == 0)
                            prop.format = RK_PIX_FMT_SBGGR12;
                        else if (strcmp(ctx->orpRawFmt, "GB12") == 0)
                            prop.format = RK_PIX_FMT_SGBRG12;
                        else if (strcmp(ctx->orpRawFmt, "RG12") == 0)
                            prop.format = RK_PIX_FMT_SRGGB12;
                        else if (strcmp(ctx->orpRawFmt, "BA12") == 0)
                            prop.format = RK_PIX_FMT_SGRBG12;
                        else if (strcmp(ctx->orpRawFmt, "BG14") == 0)
                            prop.format = RK_PIX_FMT_SBGGR14;
                        else if (strcmp(ctx->orpRawFmt, "GB14") == 0)
                            prop.format = RK_PIX_FMT_SGBRG14;
                        else if (strcmp(ctx->orpRawFmt, "RG14") == 0)
                            prop.format = RK_PIX_FMT_SRGGB14;
                        else if (strcmp(ctx->orpRawFmt, "BA14") == 0)
                            prop.format = RK_PIX_FMT_SGRBG14;
                        else if (strcmp(ctx->orpRawFmt, "BYR2") == 0)
                            prop.format = RK_PIX_FMT_SBGGR16;
                        else if (strcmp(ctx->orpRawFmt, "GB16") == 0)
                            prop.format = RK_PIX_FMT_SGBRG16;
                        else if (strcmp(ctx->orpRawFmt, "GR16") == 0)
                            prop.format = RK_PIX_FMT_SGRBG16;
                        else if (strcmp(ctx->orpRawFmt, "RG16") == 0)
                            prop.format = RK_PIX_FMT_SRGGB16;
                        else
                            prop.format = RK_PIX_FMT_SBGGR10;
                        prop.frame_width = ctx->orpRawW;
                        prop.frame_height = ctx->orpRawH;
                        prop.rawbuf_type = ctx->orpRawBufType;
                        rk_aiq_uapi2_sysctl_prepareRkRaw((rk_aiq_sys_ctx_t *)ctx->camgroup_ctx, prop);
                    }
                }

                XCamReturn ret = rk_aiq_uapi2_camgroup_prepare(ctx->camgroup_ctx, work_mode);

                if (ret != XCAM_RETURN_NO_ERROR)
                    ERR("%s:rk_aiq_uapi2_camgroup_prepare failed: %d\n", get_sensor_name(ctx), ret);
                else {

                    ret = rk_aiq_uapi2_camgroup_start(ctx->camgroup_ctx);
                }
            }
            init_device(ctx);
            start_capturing(ctx);
        }
    }
    else {
        init_device(ctx);
        if (ctx->pponeframe)
            init_device_pp_oneframe(ctx);
        start_capturing(ctx);
        if (ctx->pponeframe)
            start_capturing_pp_oneframe(ctx);
    }
}

static int aov_test_writeReg(uintptr_t addr, int value) {
    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd < 0) {
        perror("Error opening /dev/mem");
        return -1;
    }

    // 获取页大小
    size_t page_size = getpagesize();

    // 计算页对齐的地址和偏移量
    off_t page_base = (addr & ~(page_size - 1));
    off_t page_offset = addr - page_base;

    // 映射内存
    void *mem_map =
        mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, page_base);
    if (mem_map == MAP_FAILED) {
        perror("Error mapping memory");
        close(mem_fd);
        return -1;
    }

    // 计算寄存器地址
    int *target_reg = (int *)((char *)mem_map + page_offset);

    // 写入数据
    *target_reg = value;

    // 解除内存映射
    munmap(mem_map, page_size);
    close(mem_fd);

    return 0;
}

static int aov_test_setSuspendTime(int wakeup_suspend_time) {
    int result = aov_test_writeReg(SUSPEND_TIME_REG, wakeup_suspend_time * 32.768);
    if (result != 0) {
        printf("Failed to set suspend time!\n");
        return -1;
    }
    printf("[%s()] wakeup suspend time = %d\n", __func__, wakeup_suspend_time);
    return 0;
}

static int g_wakeup_period_time = 100;

static void adjust_suspend_time() {
    if (g_wakeup_period_time != -1) {
        static struct timespec last_time = {0, 0};
        struct timespec current_time;
        clock_gettime(CLOCK_BOOTTIME, &current_time);
        if (last_time.tv_sec != 0 || last_time.tv_nsec != 0) {
            long diff_ms = ((current_time.tv_sec - last_time.tv_sec) * 1000) +
                           ((current_time.tv_nsec - last_time.tv_nsec) / 1000000);
            static int last_suspend_time = 0;
            if (last_suspend_time == 0)
                last_suspend_time = g_wakeup_period_time;
            int current_suspend_time =
                last_suspend_time - (diff_ms - g_wakeup_period_time);
            if (current_suspend_time > 0) {
                aov_test_setSuspendTime(current_suspend_time);
                last_suspend_time = current_suspend_time;
            } else {
                printf("cpu run time > suspend_time, so not set !!!\n");
            }
        }
        last_time = current_time;
    }
}

static int aov_test_common_echo(const char *file_path, const char *buf, int length) {
    int fd = -1;
    ssize_t ret = -1;

    fd = open(file_path, O_WRONLY | O_TRUNC);
    if (fd == -1) {
        perror("open error\n");
        return -1;
    }

    ret = write(fd, buf, length);
    if (ret == -1) {
        perror("write error\n");
        close(fd);
        return -1;
    }

    printf("echo \"%s\" > %s successfully\n", buf, file_path);

    close(fd);

    return 0;
}


static void aov_test_enter_sleep(void *args) {

    // adjust_suspend_time();
    aov_test_common_echo(SOC_SLEEP_PATH, SOC_SLEEP_STR, strlen(SOC_SLEEP_STR));

}

static void* aov_enter_sleep_thread(void* args) {
    pthread_detach (pthread_self());
    demo_context_t *ctx = (demo_context_t *)args;
    while (1) {
        if (ctx->aovEnterSleep) {
            ctx->aovEnterSleep = false;
            aov_test_enter_sleep(args);
        }
        usleep(1 * 1000);
    }
    return 0;
}

static void* others_thread(void* args) {
    pthread_detach (pthread_self());
    demo_context_t* ctx = (demo_context_t*) args;
    rkisp_routine(ctx);
    mainloop(ctx);
    deinit(ctx);
    return 0;
}

int main(int argc, char **argv)
{
#ifdef _WIN32
    signal (SIGINT, signal_handle);
    signal (SIGQUIT, signal_handle);
    signal (SIGTERM, signal_handle);
#else
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGQUIT);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    struct sigaction new_action, old_action;
    new_action.sa_handler = signal_handle;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction (SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction (SIGINT, &new_action, NULL);
    sigaction (SIGQUIT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction (SIGQUIT, &new_action, NULL);
    sigaction (SIGTERM, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction (SIGTERM, &new_action, NULL);
#endif


    demo_context_t main_ctx = {
        .out_file = {'\0'},
        .dev_name = {'\0'},
        .dev_name2 = {'\0'},
        .dev_name3 = {'\0'},
        .dev_name4 = {'\0'},
        .dev_name5 = {'\0'},
        .sns_name = {'\0'},
        .dev_using = 1,
        .width = 640,
        .height = 480,
        .format = V4L2_PIX_FMT_NV12,
        .fd = -1,
        .buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
        .buffers = NULL,
        .n_buffers = 0,
        .frame_count = -1,
        .fp = NULL,
        .aiq_ctx = NULL,
        .camgroup_ctx = NULL,
        .vop = 0,
        .rkaiq = 0,
        .writeFile = 0,
        .writeFileSync = 0,
        .pponeframe = 0,
        .hdrmode = 0,
        .limit_range = 0,
        .fd_pp_input = -1,
        .fd_isp_mp = -1,
        .buffers_mp = NULL,
        .outputCnt = 3,
        .skipCnt = 30,
        .yuv_dir_path = {'\0'},
        ._is_yuv_dir_exist = false,
        .capture_yuv_num = 0,
        .is_capture_yuv = false,
        .ctl_type = TEST_CTL_TYPE_DEFAULT,
        .iqpath = {'\0'},
        .orppath = {'\0'},
        .orpRawW = 0,
        .orpRawH = 0,
        .orpRawFmt = {'\0'},
        .orpRawBufType = RK_AIQ_RAW_FILE,
        .isOrp = false,
        .orpStop = false,
        .orpStopped = false,
        .camGroup = false,
        .use_poll = false,
        .aovEnterSleep = false,
        .aovPauseAiq = false,
        .isAovMode = false,
        .aovLoopCnt = 30,
        .aovContinueCnt = 30,
        .aovLoopRunCnt = 0,
        .aovContinueRunCnt = 0,
        .main_scene = {{0}, {0}},
        .sub_scene = {{0}, {0}},
    };
    demo_context_t second_ctx;
    demo_context_t third_ctx;
    demo_context_t fourth_ctx;
    demo_context_t fifth_ctx;
    pthread_mutex_init(&main_ctx.mutex, NULL);
    INIT_LIST_HEAD(&main_ctx.queue);
    for (int32_t i = 0; i < RAWBUF_MAX_FRAME; i++)
        main_ctx.rawBufs[i].vaddr = NULL;

    parse_args(argc, argv, &main_ctx);
    pthread_sigmask(SIG_UNBLOCK, &mask, NULL);

#if ISPDEMO_ENABLE_DRM
    if (main_ctx.vop) {

#if ISPDEMO_ENABLE_RGA
        if (strlen(main_ctx.dev_name) && strlen(main_ctx.dev_name2)) {
            if (display_init(720, 1280) < 0) {
                printf("display_init failed\n");
            }
        } else {
#else
        {
#endif
            int crtc_num = 0;
            if (main_ctx.vop == DEMO_DISPLAY_TO_HDMI)
                crtc_num = 0;
            else
                crtc_num = 1;

            if (initDrmDsp(crtc_num) < 0) {
                printf("initDrmDsp failed\n");
            }
        }
    }
#endif

    rkisp_routine(&main_ctx);
    g_main_ctx = &main_ctx;

    if(strlen(main_ctx.dev_name2)) {
        pthread_t sec_tid;
        second_ctx = main_ctx;
        second_ctx.dev_using = 2;
        g_second_ctx = &second_ctx;
        pthread_create(&sec_tid, NULL, others_thread, &second_ctx);
    }

    if(strlen(main_ctx.dev_name3)) {
        pthread_t thr_tid;
        third_ctx = main_ctx;
        third_ctx.dev_using = 3;
        g_third_ctx = &third_ctx;
        pthread_create(&thr_tid, NULL, others_thread, &third_ctx);
    }

    if(strlen(main_ctx.dev_name4)) {
        pthread_t fou_tid;
        fourth_ctx = main_ctx;
        fourth_ctx.dev_using = 4;
        g_fourth_ctx = &fourth_ctx;
        pthread_create(&fou_tid, NULL, others_thread, &fourth_ctx);
    }

    if(strlen(main_ctx.dev_name5)) {
        pthread_t fif_tid;
        fifth_ctx = main_ctx;
        fifth_ctx.dev_using = 5;
        g_fifth_ctx = &fifth_ctx;
        pthread_create(&fif_tid, NULL, others_thread, &fifth_ctx);
    }

#ifdef ENABLE_UAPI_TEST
    pthread_t tid;
    pthread_create(&tid, NULL, test_thread, &main_ctx);
#endif

    if (main_ctx.isOrp) {
        pthread_t tid_offline;
        pthread_create(&tid_offline, NULL, test_offline_thread, &main_ctx);
    }

    if (main_ctx.isAovMode) {
        pthread_t tid_aov;
        pthread_create(&tid_aov, NULL, aov_enter_sleep_thread, &main_ctx);
    }

//#define TEST_BLOCKED_STATS_FUNC
#ifdef TEST_BLOCKED_STATS_FUNC
    pthread_t stats_tid;
    pthread_create(&stats_tid, NULL, stats_thread, &main_ctx);
#endif
#ifdef CUSTOM_AF_DEMO_TEST
    custom_af_run(main_ctx.aiq_ctx);
#endif
#ifdef CUSTOM_PDAF_DEMO_TEST
    custom_pdaf_run(main_ctx.aiq_ctx);
#endif

#ifdef ISPFEC_API
    g_ispfec_cfg.in_width     = main_ctx.width;
    g_ispfec_cfg.in_height    = main_ctx.height;
    g_ispfec_cfg.out_width     = main_ctx.width;
    g_ispfec_cfg.out_height    = main_ctx.height;
    g_ispfec_cfg.in_fourcc = 0;
    g_ispfec_cfg.out_fourcc = 0;
#if 1
    g_ispfec_cfg.mesh_upd_mode = RK_ISPFEC_UPDATE_MESH_FROM_FILE;
    strcpy(g_ispfec_cfg.u.mesh_file_path, "/etc/iqfiles/FEC_mesh_3840_2160_imx415_3.6mm/");
    strcpy(g_ispfec_cfg.mesh_xint.mesh_file, "meshxi_level0.bin");
    strcpy(g_ispfec_cfg.mesh_xfra.mesh_file, "meshxf_level0.bin");
    strcpy(g_ispfec_cfg.mesh_yint.mesh_file, "meshyi_level0.bin");
    strcpy(g_ispfec_cfg.mesh_yfra.mesh_file, "meshyf_level0.bin");
#else
    g_ispfec_cfg.mesh_upd_mode = RK_ISPFEC_UPDATE_MESH_ONLINE;
    g_ispfec_cfg.u.mesh_online.light_center[0] = 1956.3909119999998438;
    g_ispfec_cfg.u.mesh_online.light_center[1] = 1140.6355200000000422;
    g_ispfec_cfg.u.mesh_online.coeff[0] = -2819.4072493821618081;
    g_ispfec_cfg.u.mesh_online.coeff[1] = 0.0000316126581792;
    g_ispfec_cfg.u.mesh_online.coeff[2] = 0.0000000688410142;
    g_ispfec_cfg.u.mesh_online.coeff[3] = -0.0000000000130686;
    g_ispfec_cfg.u.mesh_online.correct_level = 250;
    g_ispfec_cfg.u.mesh_online.direction = RK_ISPFEC_CORRECT_DIRECTION_XY;
    g_ispfec_cfg.u.mesh_online.style = RK_ISPFEC_KEEP_ASPECT_RATIO_REDUCE_FOV;
#endif
    init_ispfec_bufs(&g_ispfec_cfg);
    g_ispfec_ctx = rk_ispfec_api_init(&g_ispfec_cfg);
#endif

    mainloop(&main_ctx);
    if (main_ctx.isOrp) {
        main_ctx.orpStop = true;
        while (!main_ctx.orpStopped) {
            printf("wait orp stopped ... \n");
            usleep(500000);
        }
    }
    deinit(&main_ctx);

#if ISPDEMO_ENABLE_RGA && ISPDEMO_ENABLE_DRM
    if (strlen(main_ctx.dev_name) && strlen(main_ctx.dev_name2)) {
        display_exit();
    }
#endif
#if ISPDEMO_ENABLE_DRM
    deInitDrmDsp();
#endif
    return 0;
}

