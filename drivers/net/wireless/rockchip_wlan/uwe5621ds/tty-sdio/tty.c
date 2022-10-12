/*
 * Copyright (C) 2015 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/atomic.h>
#include <linux/console.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/tty.h>
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/vt_kern.h>
#ifdef CONFIG_OF
#include <linux/of_device.h>
#endif
#include <linux/compat.h>
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/tty_flip.h>
#include <linux/workqueue.h>

#include "include/debug.h"
#include "include/hci.h"
#include "include/interface.h"
#include "include/lpm.h"
#include "include/rfkill.h"
#include "include/sdio.h"
#include "include/sitm.h"
#include "include/tty.h"
#include <marlin_platform.h>

// #define  UNISOC_AMLOGIC_SUSPEND
#ifdef UNISOC_AMLOGIC_SUSPEND
#include <linux/amlogic/pm.h> //add
static struct early_suspend bt_early_suspend; //add
#endif

#if 0
#define DO_SYSCALL_2(sc, t1, a1, t2, a2) \
    (((asmlinkage long (*)(t1, t2))sys_call_table[__NR_##sc])(a1, a2));
#define USER_SYSCALL_2(sc, t1, a1, t2, a2)                   \
    static inline asmlinkage long syscall_##sc(t1 a1, t2 a2) \
    {                                                        \
        return DO_SYSCALL_2(sc, t1, a1, t2, a2)              \
    }

USER_SYSCALL_2(chmod, const char __user *, filename, mode_t, mode);
#endif

#ifndef USE_DTS
static void __exit uwe5621_bt_tty_exit(void);
#endif
static int mtty_remove(struct platform_device* pdev);
int closeflag = 0;
struct mtty_device* mtty_dev;

struct bt_data_interface_t* bt_data_interface;
static struct bt_data_interface_cb_t bt_data_interface_cb;
extern int list_time;
extern unsigned char dis_flag;
static int mtty_open(struct tty_struct* tty, struct file* filp)
{
    struct mtty_device* mtty = NULL;
    struct tty_driver* driver = NULL;

    // start_marlin(MARLIN_BLUETOOTH);
    bluetooth_set_power(0, 0);

    clear_woble_devices();

    if (tty == NULL) {
        pr_err("mtty open input tty is NULL!\n");
        return -ENOMEM;
    }

    driver = tty->driver;
    mtty = (struct mtty_device*)driver->driver_state;

    if (mtty == NULL) {
        pr_err("mtty open input mtty NULL!\n");
        return -ENOMEM;
    }

    mtty->tty = tty;
    tty->driver_data = (void*)mtty;

    atomic_set(&mtty->state, MTTY_STATE_OPEN);
    sitm_init();
    // pr_info("%s device success! pid %d %d %d sid %d\n", __func__, task_pid_nr(current), current->pid, pid_nr(get_task_pid(current, PIDTYPE_PID)),
    // pid_nr(get_task_pid(current, PIDTYPE_SID)));
    closeflag = 0;
    list_time = 0x11;
    dis_flag = 0;
    return 0;
}

static void mtty_close(struct tty_struct* tty, struct file* filp)
{
    closeflag = 1;
    list_time = 0x11;
    dis_flag = 0;
    pr_info("mtty_close device success _close!\n");

    /*
    struct mtty_device *mtty = NULL;

    if (tty == NULL)
    {
        pr_err("mtty close input tty is NULL!\n");
        return;
    }

    mtty = (struct mtty_device *) tty->driver_data;

    if (mtty == NULL)
    {
        pr_err("mtty close s tty is NULL!\n");
        return;
    }

    atomic_set(&mtty->state, MTTY_STATE_CLOSE);
    sitm_cleanup();
    pr_info("mtty_close device success !\n");

    //stop_marlin(MARLIN_BLUETOOTH);
    bluetooth_set_power(0, 1);
*/
}

static inline int mtty_transfer_upper(const unsigned char* buf, int count)
{
    int ret = tty_insert_flip_string(mtty_dev->port, buf, count);

    if (ret != count) {
        pr_err("%s err ret %d count %d !!!!!!!!!!!\n", __func__, ret, count);
    }

    tty_flip_buffer_push(mtty_dev->port);
    return ret;
}
// add ..... 04 05 04 00 00 02 13
int resume_transfer_upper(const unsigned char* buf, int count)
{
    int ret = tty_insert_flip_string(mtty_dev->port, buf, count);
    pr_err("test_%s\n", __func__); //
    if (ret != count) {
        pr_err("%s err ret %d count %d !!!!!!!!!!!\n", __func__, ret, count);
    }

    tty_flip_buffer_push(mtty_dev->port);
    return ret;
}

static int mtty_recv(const unsigned char* buf, int count)
{
    // pr_err("mtty_recv count:%d\n", count);
    // hex_dump_block(buf, count);
    // return mtty_transfer_upper(buf, count);
    return rx_data_recv(buf, count, mtty_transfer_upper);
}

static int sdio_data_transmit(uint8_t* data, size_t count)
{
    return bt_data_interface->write(data, count);
}

static int mtty_write_plus(struct tty_struct* tty,
    const unsigned char* buf, int count)
{

    return sitm_write(buf, count, sdio_data_transmit);
}

static void mtty_flush_chars(struct tty_struct* tty)
{
}

static int mtty_write_room(struct tty_struct* tty)
{
    return INT_MAX;
}

static const struct tty_operations mtty_ops = {
    .open = mtty_open,
    .close = mtty_close,
    .write = mtty_write_plus,
    .flush_chars = mtty_flush_chars,
    .write_room = mtty_write_room,
};

static struct tty_port* mtty_port_init(void)
{
    struct tty_port* port = NULL;

    port = kzalloc(sizeof(struct tty_port), GFP_KERNEL);

    if (port == NULL) {
        return NULL;
    }

    tty_port_init(port);

    return port;
}

static int mtty_tty_driver_init(struct mtty_device* device)
{
    struct tty_driver* driver;
    int ret = 0;

    device->port = mtty_port_init();

    if (!device->port) {
        return -ENOMEM;
    }

    driver = alloc_tty_driver(MTTY_DEV_MAX_NR);

    if (!driver) {
        return -ENOMEM;
    }

    /*
    * Initialize the tty_driver structure
    * Entries in mtty_driver that are NOT initialized:
    * proc_entry, set_termios, flush_buffer, set_ldisc, write_proc
    */
    driver->owner = THIS_MODULE;
#ifndef USE_DTS
    driver->driver_name = "ttyBT";
    driver->name = "ttyBT";
#else
    driver->driver_name = device->pdata->name;
    driver->name = device->pdata->name;
#endif

#if 1
    driver->major = 0;
#else
    driver->major = TTYAUX_MAJOR;
    driver->minor_start = 2;
#endif

    driver->type = TTY_DRIVER_TYPE_SYSTEM;
    driver->subtype = SYSTEM_TYPE_TTY;
    driver->init_termios = tty_std_termios;
    driver->driver_state = (void*)device;
    device->driver = driver;
    device->driver->flags = TTY_DRIVER_REAL_RAW;
    /* initialize the tty driver */
    tty_set_operations(driver, &mtty_ops);
    tty_port_link_device(device->port, driver, 0);
    ret = tty_register_driver(driver);

    if (ret) {
        put_tty_driver(driver);
        tty_port_destroy(device->port);
        return ret;
    }

    // sys_chmod("/dev/ttyBT0", 0777);
    // syscall_chmod("/dev/ttyBT0", 0777);
    return ret;
}

static void mtty_tty_driver_exit(struct mtty_device* device)
{
    struct tty_driver* driver = device->driver;

    tty_unregister_driver(driver);
    put_tty_driver(driver);
    tty_port_destroy(device->port);
}

static int mtty_parse_dt(struct mtty_init_data** init, struct device* dev)
{
#ifdef CONFIG_OF
    struct device_node* np = dev->of_node;
    struct mtty_init_data* pdata = NULL;
    int ret;

    pdata = kzalloc(sizeof(struct mtty_init_data), GFP_KERNEL);

    if (!pdata) {
        return -ENOMEM;
    }

    ret = of_property_read_string(np,
        "sprd,name",
        (const char**)&pdata->name);

    if (ret) {
        goto error;
    }

    *init = pdata;

    return 0;
error:
    kfree(pdata);
    *init = NULL;
    return ret;
#else
    return -ENODEV;
#endif
}

static inline void mtty_destroy_pdata(struct mtty_init_data** init)
{
#if (defined CONFIG_OF) && (defined USE_DTS)
    struct mtty_init_data* pdata = *init;

    kfree(pdata);

    *init = NULL;
#else
    return;
#endif
}

static ssize_t at_store(struct device* dev,
    struct device_attribute* attr,
    const char* buf, size_t count)
{
    pr_info("%s\n", __func__);
    hci_woble_enable();
    return count;
}

static ssize_t at_show(struct device* dev,
    struct device_attribute* attr,
    char* buf)
{
    pr_info("%s\n", __func__);
    return 0;
}

static ssize_t woble_set_store(struct device* dev, struct device_attribute* attr, const char* buf, size_t count)
{
    unsigned char* p = (unsigned char*)buf;
    unsigned char opcode = 0;
    unsigned short handler = 0;
    STREAM_TO_UINT8(opcode, p);

    // pr_info("%s, len %lu op: %d\n", __func__, count, opcode);

    switch (opcode) {
    case WOBLE_KERNEL_OP_CLEAR:
        clear_woble_devices();
        break;

    case WOBLE_KERNEL_OP_SET_OWN:
        set_random_address(p);
        break;

    case WOBLE_KERNEL_OP_ADD_RMT: {
        unsigned char type = 0;
        STREAM_TO_UINT8(type, p);
        STREAM_TO_UINT16(handler, p);
        pr_info("%s, get type: %d\n", __func__, type);
        update_woble_devices(type, handler, p);
    } break;

    case WOBLE_KERNEL_OP_DEL_RMT: {
        unsigned char type = 0;
        STREAM_TO_UINT8(type, p);
        pr_info("%s, get type: %d\n", __func__, type);
        del_woble_devices(type, p);
    } break;

    case WOBLE_KERNEL_OP_ENABLE_BLE_INDICATE: {
        set_need_indicate_cp2_woble(1);
    } break;

    case WOBLE_KERNEL_OP_DISABLE_BLE_INDICATE: {
        set_need_indicate_cp2_woble(0);
    } break;
    }

    return count;
}

static ssize_t woble_set_show(struct device* dev,
    struct device_attribute* attr,
    char* buf)
{
    pr_info("%s\n", __func__);
    dump_woble_devices();
    return 0;
}

static ssize_t ant_num_show(struct device* dev,
    struct device_attribute* attr,
    char* buf)
{
    int num = 2;

    num = marlin_get_ant_num();
    pr_err("%s: %d", __func__, num);

    return sprintf(buf, "%d", num);
}

static ssize_t chipid_show(struct device* dev,
    struct device_attribute* attr,
    char* buf)
{
    int type = 0;

    type = wcn_get_chip_model();
    pr_err("%s: %d", __func__, type);

    return sprintf(buf, "%d", type);
}

static ssize_t misc_node_store(struct device* dev,
    struct device_attribute* attr,
    const char* buf, size_t count)
{
    unsigned char opcode = 0;
    const char* tbuf = buf;
    pr_info("%s\n", __func__);

    if (memcmp("AT+BTLOG=", buf, strlen("AT+BTLOG=")) == 0 && count >= strlen("AT+BTLOG=") + 1) {
        tbuf += strlen("AT+BTLOG=");
        STREAM_TO_UINT8(opcode, tbuf);
        set_log_level(opcode - '0');
    } else if (memcmp("AT+SLEEP=", buf, strlen("AT+SLEEP=")) == 0 && count >= strlen("AT+SLEEP=") + 1) {
        unsigned char is_resume = 0;
        tbuf += strlen("AT+SLEEP=");
        STREAM_TO_UINT8(is_resume, tbuf);
        is_resume = is_resume - '0';
        pr_err("%s AT+SLEEP=%d\n", __func__, is_resume);
        switch (is_resume) {
        case 0:
            hci_set_scan_parameters();
            hci_set_scan_enable(1);
            break;
        case 1:
            hci_add_device_to_wakeup_list();
            break;
        case 2:
            hci_set_ap_sleep_mode(WOBLE_IS_NOT_SHUTDOWN, WOBLE_IS_NOT_RESUME);
            break;
        case 3:
            hci_set_ap_start_sleep();
            break;
        case 4:
            hci_set_ap_sleep_mode(WOBLE_IS_NOT_SHUTDOWN, WOBLE_IS_RESUME);
            break;
        default:
            break;
        }
    }
    return count;
}

static ssize_t misc_node_show(struct device* dev,
    struct device_attribute* attr,
    char* buf)
{
    pr_info("%s\n", __func__);

    return 0;
}
#define ALL_PER 1

#if ALL_PER
#pragma push_macro("VERIFY_OCTAL_PERMISSIONS")
#ifdef VERIFY_OCTAL_PERMISSIONS
#undef VERIFY_OCTAL_PERMISSIONS
#endif

#define VERIFY_OCTAL_PERMISSIONS(perms) (perms)
#endif
#if ALL_PER
static DEVICE_ATTR(at, 00660, at_show, at_store);
static DEVICE_ATTR(woble_set, 00660, woble_set_show, woble_set_store);
static DEVICE_ATTR(ant_num, 00664, ant_num_show, 0);
static DEVICE_ATTR(chipid, 00664, chipid_show, 0);
static DEVICE_ATTR(misc_node, 00660, misc_node_show, misc_node_store);

#pragma pop_macro("VERIFY_OCTAL_PERMISSIONS")

#else
static DEVICE_ATTR(at, 00660, at_show, at_store);
static DEVICE_ATTR(woble_set, 00660, woble_set_show, woble_set_store);
static DEVICE_ATTR(ant_num, 00660, ant_num_show, 0);
static DEVICE_ATTR(chipid, 00660, chipid_show, 0);
static DEVICE_ATTR(misc_node, 00660, misc_node_show, misc_node_store);
#endif

struct attribute* bluetooth_attrs[] = {
    &dev_attr_at.attr,
    &dev_attr_woble_set.attr,
    &dev_attr_ant_num.attr,
    &dev_attr_chipid.attr,
    &dev_attr_misc_node.attr,
    NULL,
};

static struct attribute_group bluetooth_group = {
    .name = NULL,
    .attrs = bluetooth_attrs,
};

#ifdef UNISOC_AMLOGIC_SUSPEND
extern unsigned char resume_rxmsg[7];
int early_flag = 0x11;
int dis_cmd_flag = 0x11;
extern unsigned char red_bt_flag; //区分蓝牙，红外待机标志；0x11:红外 0x22: bt
//int  uni_sus_flag =0x11;
//int  uni_resu_flag= 0x11;
//int  resume_suspen_flag=0x11;
//int  firs_dis_flag=0x11;
static void bt_earlysuspend(struct early_suspend* h)
{
    dis_cmd_flag = 0x11;
    early_flag = 0x11;
}

static void bt_lateresume(struct early_suspend* h)
{

    if (early_flag == 0x22) {
        resume_transfer_upper(resume_rxmsg, 7);
    }
    early_flag = 0x11;
    dis_cmd_flag = 0x11;
    dis_flag =0 ;// 更新狀態
    pr_err("unisoc_lateresume\n"); //
}
#endif

#ifdef CP2_RESET_SUPPORT
static void bt_cp2_pre_reset(void) {
	pr_err("%s: entry!\n", __func__);
}

static void bt_cp2_post_reset(void) {
#define RESET_BUFSIZE 5

	int ret = 0;
	int block_size = RESET_BUFSIZE;
	unsigned char reset_buf[RESET_BUFSIZE]= {0x04, 0xff, 0x02, 0x57, 0xa5};

	pr_err("%s: reset callback coming\n", __func__);
	if (mtty_dev != NULL) {
		// if (!work_pending(&mtty_dev->bt_rx_work)) {
		if (atomic_read(&mtty_dev->state) == MTTY_STATE_OPEN && (RESET_BUFSIZE > 0)) {
			pr_err("%s tty_insert_flip_string", __func__);
			while(ret < block_size){
				pr_err("%s before tty_insert_flip_string ret: %d, len: %d\n",
						__func__, ret, RESET_BUFSIZE);
				ret = tty_insert_flip_string(mtty_dev->port,
									(unsigned char *)reset_buf,
									RESET_BUFSIZE);   // -BT_SDIO_HEAD_LEN
				pr_err("%s ret: %d, len: %d\n", __func__, ret, RESET_BUFSIZE);
				if (ret)
					tty_flip_buffer_push(mtty_dev->port);
				block_size = block_size - ret;
				ret = 0;
			}
		}
	}
}


int bt_cp2_reset_callback(struct notifier_block *nb, unsigned long event, void *v) {
	if (!v) {
		pr_err("%s: v is NULL\n", __func__);
		return NOTIFY_BAD;
	}

	pr_err("%s: %s %d\n", __func__, (char *)v, (int)event);

	switch(event) {
		case MARLIN_CP2_STS_ASSERTED:
			bt_cp2_pre_reset();
			break;
		case MARLIN_CP2_STS_READY:
			bt_cp2_post_reset();
			break;
		default:
			pr_err("%s: parameter error event: %d\n", __func__, (int)event);
			return NOTIFY_BAD;
	}

	return NOTIFY_OK;
}

static struct notifier_block bt_cp2_reset_notifier = {
	.notifier_call = bt_cp2_reset_callback,
};
#endif  /*CP2_RESET_SUPPORT*/

static int mtty_probe(struct platform_device* pdev)
{
    struct mtty_init_data* pdata = (struct mtty_init_data*)pdev->dev.platform_data;
    struct mtty_device* mtty;
    int rval = 0;

    if (marlin_get_wcn_module_vendor() < 0) {
        pr_err("%s not unisoc soc, exit", __func__);
        // mtty_remove(pdev);
        // uwe5621_bt_tty_exit();
        return -EACCES;
    } else {
        pr_err("%s unisoc soc, continue", __func__);
    }

    if (pdev->dev.of_node && !pdata) {
        rval = mtty_parse_dt(&pdata, &pdev->dev);

        if (rval) {
            pr_err("%s failed to parse mtty device tree, ret=%d\n", __func__,
                rval);
            return rval;
        }
    }

    mtty = kzalloc(sizeof(struct mtty_device), GFP_KERNEL);

    if (mtty == NULL) {
        mtty_destroy_pdata(&pdata);
        pr_err("%s Failed to allocate device!\n", __func__);
        return -ENOMEM;
    }

    mtty->pdata = pdata;
    rval = mtty_tty_driver_init(mtty);

    if (rval) {
        mtty_tty_driver_exit(mtty);
        kfree(mtty->port);
        kfree(mtty);
        mtty_destroy_pdata(&pdata);
        pr_err("%s regitster notifier failed (%d)\n", __func__, rval);
        return rval;
    }

    pr_info("%s init device addr: 0x%p\n", __func__, mtty);
    platform_set_drvdata(pdev, mtty);

    atomic_set(&mtty->state, MTTY_STATE_CLOSE);

    mtty_dev = mtty;

    if (sysfs_create_group(&(pdev->dev.kobj), &bluetooth_group)) {
        pr_err("%s create dispc attr node failed", __func__);
    }

    rfkill_bluetooth_init(pdev);
    // bluesleep_init();

    // uwe5621_bt_tty_exit();
    hci_init();

    bt_data_interface_cb.size = sizeof(struct bt_data_interface_cb_t);
    bt_data_interface_cb.recv = mtty_recv;
    bt_data_interface = get_marlin_sdio_interface();
    bt_data_interface->init(&bt_data_interface_cb);

/********************************************/
#ifdef UNISOC_AMLOGIC_SUSPEND
    bt_early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB;
    bt_early_suspend.suspend = bt_earlysuspend;
    bt_early_suspend.resume = bt_lateresume;
    bt_early_suspend.param = pdev;
    register_early_suspend(&bt_early_suspend);
#endif
/********************************************/

#ifdef CP2_RESET_SUPPORT
		marlin_reset_callback_register(MARLIN_BLUETOOTH, &bt_cp2_reset_notifier);
#endif  /*CP2_RESET_SUPPORT*/

    return 0;
}

static int mtty_remove(struct platform_device* pdev)
{
    struct mtty_device* mtty = platform_get_drvdata(pdev);
    rfkill_bluetooth_remove(pdev);
    pr_err("%s\n", __func__);
    mtty_tty_driver_exit(mtty);
    bt_data_interface->cleanup();
    kfree(mtty->port);
    mtty_destroy_pdata(&mtty->pdata);
    /* tasklet_kill(&mtty->rx_task); */
    kfree(mtty);
    platform_set_drvdata(pdev, NULL);
    // bluesleep_exit();
    hci_destory();
    return 0;
}

static int mtty_suspend(struct platform_device* device, pm_message_t state)
{
    pr_err("%s\n", __func__);
    // at_store(0, 0, 0, 0);
    // stop_marlin(MARLIN_BLUETOOTH);
    // sprdwcn_bus_remove_card();
    return 0;
}

static void mtty_shutdown(struct platform_device* device)
{
    pr_err("%s\n", __func__);
    return;
}
static int mtty_resume(struct platform_device* device)
{
    pr_err("%s\n", __func__);
    return 0;
}

static void mtty_device_release(struct device* device)
{
    pr_err("%s\n", __func__);
    return;
}

static struct platform_device uwe5621_bt_tty_device = {
    .name = "mtty",
    .dev = {
        .release = mtty_device_release,
    },
};
static const struct of_device_id mtty_match_table[] = {
    {
        .compatible = "sprd,mtty",
    },
};
static struct platform_driver uwe5621_bt_tty_driver = {
    .driver = {
        .owner = THIS_MODULE,
        .name = "mtty",
        .of_match_table = mtty_match_table,
    },
    .probe = mtty_probe,
    .remove = mtty_remove,
    .suspend = mtty_suspend,
    .resume = mtty_resume,
    .shutdown = mtty_shutdown,
};

#ifdef USE_DTS
// for dts
module_platform_driver(uwe5621_bt_tty_driver);

#else

static int __init uwe5621_bt_tty_init(void)
{
    pr_err("%s\n", __func__);
    platform_device_register(&uwe5621_bt_tty_device);
    return platform_driver_register(&uwe5621_bt_tty_driver);
}

static void __exit uwe5621_bt_tty_exit(void)
{
    pr_err("%s\n", __func__);
    platform_driver_unregister(&uwe5621_bt_tty_driver);
    platform_device_unregister(&uwe5621_bt_tty_device);
}

// module_init(uwe5621_bt_tty_init);
late_initcall_sync(uwe5621_bt_tty_init);
module_exit(uwe5621_bt_tty_exit);

#endif

MODULE_AUTHOR("Unisoc wcn bt");
MODULE_DESCRIPTION("Unisoc marlin tty driver");
