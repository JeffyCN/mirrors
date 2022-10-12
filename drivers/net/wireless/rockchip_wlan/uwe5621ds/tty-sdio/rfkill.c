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

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/rfkill.h>
#include <linux/gpio.h>
#include <linux/ioport.h>
#include <linux/clk.h>
#include <linux/of_gpio.h>
#include <marlin_platform.h>
#include <linux/vmalloc.h>
#include "include/rfkill.h"
#include <linux/semaphore.h>
#include "include/hci.h"

static struct rfkill *bt_rfk;
static const char bt_name[] = "bluetooth";

int bluetooth_set_power(void *data, bool blocked)
{
    static int is_power_on = 0;

    pr_err("%s: start_block=%d\n", __func__, blocked);

    if (!blocked)
    {
        is_power_on = 1;
        start_marlin(MARLIN_BLUETOOTH);
    }
    else
    {
        if (is_power_on)
        {
            //struct HC_BT_HDR *payload = (struct HC_BT_HDR *)vmalloc(sizeof(struct HC_BT_HDR) + 3);

            //payload->len = 3;
            //payload->data[0] = 0;
            //payload->data[1] = 0;
            //payload->data[2] = 0;

            //hci_cmd_send_sync(0xfca1, payload, NULL);
        }

        is_power_on = 0;
        stop_marlin(MARLIN_BLUETOOTH);
    }

    pr_err("%s: end_block=%d\n", __func__, blocked);
    return 0;
}

//static int bluetooth_set_power_dryrun(void *data, bool blocked)
//{
    //pr_err("%s: blocked %d data 0x%p\n", __func__, blocked, data);
    //return 0;
//}

static struct rfkill_ops rfkill_bluetooth_ops =
{
    .set_block = bluetooth_set_power,
};

int rfkill_bluetooth_init(struct platform_device *pdev)
{
    int rc = 0;

    pr_err("%s\n", __func__);
    bt_rfk = rfkill_alloc(bt_name, &pdev->dev, RFKILL_TYPE_BLUETOOTH,
                          &rfkill_bluetooth_ops, NULL);

    if (!bt_rfk)
    {
        rc = -ENOMEM;
        goto err_rfkill_alloc;
    }

    /* userspace cannot take exclusive control */

    rfkill_init_sw_state(bt_rfk, true); //default power off

    rc = rfkill_register(bt_rfk);

    if (rc)
    {
        goto err_rfkill_reg;
    }

    pr_err("%s end\n", __func__);
    return 0;

err_rfkill_reg:
    rfkill_destroy(bt_rfk);
err_rfkill_alloc:
    return rc;
}

int rfkill_bluetooth_remove(struct platform_device *dev)
{
    pr_err("%s\n", __func__);
    rfkill_unregister(bt_rfk);
    rfkill_destroy(bt_rfk);
    return 0;
}


