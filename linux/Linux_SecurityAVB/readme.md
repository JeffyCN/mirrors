# AVB Reference

Version: 2.5
Author: jason.zhu@rock-chips.com; zain.wang@rock-chips.com
Date: 2019.09.02
Confidentiality level: Disclosure

------

## Introduction

This document applies to RK3308/RK3326/PX30/RK3399/RK3328/RK3288/RK1808

## Summarize

### Version
| **Chip** | **Security Storage** | **Kernel** |   **U-Boot**   |   **RKBIN**    |
| -------- | -------------------- | ---------- | -------------- | -------------- |
| RK3308   | OTP                  | ALL        | commit d3a731e | commit e00dab6 |
| RK3326   | OTP                  | 4.4        | commit d3a731e | commit 20af526 |
| PX30     | OTP                  | 4.4        | commit d3a731e | commit 20af526 |
| RK3328   | OTP                  | 4.4        | commit d3a731e | commit 158ccc6 |
| RK3399   | eFuse                | 4.4        | commit d3a731e | commit 158ccc6 |
| RK3288   | eFuse                | 4.4        | commit d3a731e | commit c021945 |
| RK1808   | eFuse                | 4.4        | commit d3a731e | commit 890556f |

### Reader
This document is mainly suitable for below engineers:
- Technical Support Engineer
- Software Engineer

### Changes
|  **Date**  | **Version** | **Author** |                                                       **Changes**                                                        |
| ---------- | ----------- | ---------- | ------------------------------------------------------------------------------------------------------------------------ |
| 2018.05.28 | V1.0        | Jason Zhu  | Initial                                                                                                                  |
| 2018.06.12 | V1.1        | Jason Zhu  | Add Authenticated Unlock and some others descripation                                                                    |
| 2018.06.27 | V1.2        | Zain Wong  | Add descripation generation AVB keys                                                                                     |
| 2018.11.16 | V1.3        | Zain Wong  | Add u-boot config descripation <br> Add rk3326 support                                                                   |
| 2019.01.28 | v2.0        | Zain Wong  | Support lastest uboot <br> Add rk3399 support                                                                            |
| 2019.05.22 | v2.1        | Zain Wong  | Add new command to write avb keys to efuse/OTP <br> Add rk3328 support                                                   |
| 2019.06.03 | v2.2        | Zain Wong  | Fix error descripation                                                                                                   |
| 2019.07.13 | v2.3        | Zain Wong  | Add chapter "Notice" <br> Add parameter.txt descripation <br> Support lastest uboot <br> Add chapter "Verified Platform" |
| 2019.08.20 | v2.4        | Zain Wong  | Add descripation for u-boot CONFIG_RK_AVB_LIBAVB_ENABLE_ATH_UNLOCK                                                       |
| 2019.09.02 | v2.5        | Zain Wong  | Add platform defconfig descripation                                                                                      |
| 2019.09.10 | v2.6        | Zain Wong  | Add En Version <br> Fixed some descripations                                                                             |

[TOC]

## 1. Verified Platform

1.    commit d3a731eb0a985e5d1c850e7e2fb3aa348ca27935
      rockchip: rk1808: add CONFIG_SUPPORT_EMMC_RPMB
2.    chip:
      rk3308/rk3288/rk3328/rk3399/rk1808/rk3326/px30
3.    defconfig:
      evb-xxxx_defconfig / xxxx_defconfig  (xxxx: chip name, such as evb-rk3308_defconfig / rk3308_defconfig)

## 2. Notice

1.    When the device is in **unlock state**, AVB will still verify the boot.img. AVB will show the error If boot.img is unvalid, but the device **boot normally**. When the device is in lock state, AVB will **stop booting** if boot.img is unvalid and show the error as well. Therefore, setting the device to unlock state is convenient for debugging.

2.    AVB is not support compressed kernel images.
3.    Chips used efuse must enable Base SecureBoot (Rockchip_Developer_Guide_Linux4.4_SecureBoot.pdf **chapter 2 Base SecureBoot**)

## 3. Configuration

### 3.1. trust

Make sure the trust.img has enable secure option.

Take rk3308 as an example.
Enter rkbin/RKTRUST, find **RK3308TRUST.ini**(This file selected by make.sh in u-boot) and change:
```changes
diff --git a/RKTRUST/RK3308TRUST.ini b/RKTRUST/RK3308TRUST.ini
index 0b2839d..51ec627 100644
--- a/RKTRUST/RK3308TRUST.ini
+++ b/RKTRUST/RK3308TRUST.ini
@@ -8,7 +8,7 @@ SEC=1
 PATH=bin/rk33/rk3308_bl31_v2.21.elf
 ADDR=0x00010000
 [BL32_OPTION]
-SEC=0
+SEC=1
 PATH=bin/rk33/rk3308_bl32_v1.12.bin
 ADDR=0x00200000
 [BL33_OPTION]

```
**Secure option is enabled as default for TOS format, like RK3288TOS.ini**

<span id="3.2"></span>
### 3.2. U-Boot

AVB needs these configs:
```config
# OPTEE support
CONFIG_OPTEE_CLIENT=y
CONFIG_OPTEE_V1=y»      # For rk3288/rk3328/rk3399 and NOT work with V2
CONFIG_OPTEE_V2=y»      # For rk3308/rk3326/px30/rk1808 and NOT work with V1

# CRYPTO support
CONFIG_DM_CRYPTO=y # Only For efuse
CONFIG_ROCKCHIP_CRYPTO_V1=y # For efuse chips, like rk3399/rk3288
CONFIG_ROCKCHIP_CRYPTO_V2=y # For efuse chips, like rk1808

# AVB support
CONFIG_AVB_LIBAVB=y
CONFIG_AVB_LIBAVB_AB=y
CONFIG_AVB_LIBAVB_ATX=y
CONFIG_AVB_LIBAVB_USER=y
CONFIG_RK_AVB_LIBAVB_USER=y
CONFIG_AVB_VBMETA_PUBLIC_KEY_VALIDATE=y
CONFIG_ANDROID_AVB=y
CONFIG_ANDROID_AB=y    # Not necessary
CONFIG_OPTEE_ALWAYS_USE_SECURITY_PARTITION=y    # Enable it if no RPMB
CONFIG_ROCKCHIP_PRELOADER_PUB_KEY=y    # Only for efuse
CONFIG_RK_AVB_LIBAVB_ENABLE_ATH_UNLOCK=y # See below

#fastboot support
CONFIG_FASTBOOT=y
CONFIG_FASTBOOT_BUF_ADDR=0x2000000»     # Not fixed, change it if necessary
CONFIG_FASTBOOT_BUF_SIZE=0x08000000»    # Not fixed, change it if necessary
CONFIG_FASTBOOT_FLASH=y
CONFIG_FASTBOOT_FLASH_MMC_DEV=0
```

CONFIG_RK_AVB_LIBAVB_ENABLE_ATH_UNLOCK is newer config for U-Boot, if no this config in U-Boot, try to fixed it manually.
The important is try to run rk_auth_unlock() to ensure unlock operation in valid.
Note: changes base on U-Boot commit 46a8a26905fc68e6683b93c97adae0dd9a4e37ba

```C
diff --git a/drivers/usb/gadget/f_fastboot.c b/drivers/usb/gadget/f_fastboot.c
index 99d62a6..fec465e 100755
--- a/drivers/usb/gadget/f_fastboot.c
+++ b/drivers/usb/gadget/f_fastboot.c
@@ -1370,7 +1370,7 @@ static void cb_oem(struct usb_ep *ep, struct usb_request *req)
                        fastboot_tx_write_str("FAILThe vboot is disable!");
                } else {
                        lock_state = 1;
-#ifdef CONFIG_RK_AVB_LIBAVB_ENABLE_ATH_UNLOCK
+#if 1
                        if (rk_auth_unlock((void *)CONFIG_FASTBOOT_BUF_ADDR,
                                           &out_is_trusted)) {
                                printf("rk_auth_unlock ops error!\n");
```

If ramdisk is required when used A/B Partition, U-Boot has to modify.
(This change unabled to merge to master since conflict with android)
~~~C
diff --git a/common/android_bootloader.c b/common/android_bootloader.c
index 230bf0e..e5c81e6 100644
--- a/common/android_bootloader.c
+++ b/common/android_bootloader.c
@@ -1061,9 +1061,6 @@ int android_bootloader_boot_flow(struct blk_desc *dev_desc,
                 * "skip_initramfs" to the cmdline to make it ignore the
                 * recovery initramfs in the boot partition.
                 */
-#ifdef CONFIG_ANDROID_AB
-               mode_cmdline = "skip_initramfs";
-#endif
                break;
        case ANDROID_BOOT_MODE_RECOVERY:
                /* In recovery mode we still boot the kernel from "boot" but
~~~

Now, run ./make.sh xxx generating uboot.img, trust.img and loader.bin

<span id="3.3"></span>
### 3.3. Parameter

Partition "vbmeta" and "system" are required.
"vbmeta" use to store the signature information, size 1M, no matter where vbmeta is.
"system" has different name in some unique platform, like buildroot calls "system" to "rootfs".
Rename "rootfs" to "system". If used uuid, changes uuid partition name as well.

"security" partition is required if storage medium is not EMMC (no RPMB partition), it used to storage AVB information instead of RPMB.
AVB content is encrypted, the size is 4M and the location is optional.

Here are examples of AVB parameter：
~~~
0x00002000@0x00004000(uboot),0x00002000@0x00006000(trust),0x00002000@0x00008000(misc),0x00010000@0x0000a000(boot),0x00010000@0x0001a000(recovery),0x00010000@0x0002a000(backup),0x00020000@0x0003a000(oem),0x00300000@0x0005a000(system),0x00000800@0x0035a000(vbmeta),0x00002000@0x0035a800(security),-@0x0035c800(userdata:grow)
uuid:system=614e0000-0000-4b53-8000-1d28000054a9
~~~

AVB and A/B parameter:
~~~
0x00002000@0x00004000(uboot),0x00002000@0x00006000(trust),0x00004000@0x00008000(misc),0x00010000@0x0000c000(boot_a),0x00010000@0x0001c000(boot_b),0x00010000@0x0002c000(backup),0x00020000@0x0003c000(oem),0x00300000@0x0005c000(system_a),0x00300000@0x0035c000(system_b),0x00000800@0x0065c000(vbmeta_a),0x00000800@0x0065c800(vbmeta_b),0x00002000@0x0065d000(security),-@0x0065f00(userdata:grow)
~~~

When downloading, the partition names on the AndroidTools (windows PC）should be modified synchronously. After modification, reload parameter.

## 4. AVB Keys

AVB contains the following four Keys:
Product RootKey (PRK): root Key of AVB, in eFuse devices, related information is verified by Base SecureBoot Key. In OTP devices, PRK-Hash information pre-stored in OTP is directly read and verified;
ProductIntermediate Key (PIK): intermediate Key;
ProductSigning Key (PSK): used to sign a firmware;
ProductUnlock Key (PUK): used to unlock a device.

There is already a set of test certificates and keys in this directory. If you need new Keys and certificates, you can generate them by the following steps:
~~~
    openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:4096 -outform PEM -out testkey_prk.pem
    openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:4096 -outform PEM -out testkey_psk.pem
    openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:4096 -outform PEM -out testkey_pik.pem
    touch temp.bin
    python avbtool make_atx_certificate --output=pik_certificate.bin --subject=temp.bin --subject_key=testkey_pik.pem --subject_is_intermediate_authority --subject_key_version 4
    echo "RKXXXX_nnnnnnnn" > product_id.bin
    python avbtool make_atx_certificate --output=psk_certificate.bin --subject=product_id.bin --subject_key=testkey_psk.pem --subject_key_version 42 --authority_key=testkey_pik.
    python avbtool make_atx_metadata --output=metadata.bin --intermediate_key_certificate=pik_certificate.bin --product_key_certificate=psk_certificate.bin
~~~

temp.bin is a temporary file created manually, just touch temp.bin and blank it.
Product_id.bin created manually is used for product ID, the size is 16 bytes.

Generate permanent_attributes.bin：
~~~
    python avbtool make_atx_permanent_attributes --output=permanent_attributes.bin --product_id=product_id.bin --root_authority_key=testkey_prk.pem
~~~

Generate PUK：
~~~
    openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:4096 -outform PEM -out testkey_puk.pem
~~~

Generate permanent_attributes.bin
~~~
    python avbtool make_atx_certificate --output=puk_certificate.bin --subject=product_id.bin --subject_key=testkey_puk.pem --usage=com.google.android.things.vboot.unlock --subj
~~~

For eFuse devices, you need to generate additional permanent_attributes_cer.bin (OTP devices can skip this step)
And the PrivateKey.pem which is the key of Base SecureBoot is requried.
~~~
    openssl dgst -sha256 -out permanent_attributes_cer.bin -sign PrivateKey.pem permanent_attributes.bin
~~~

**Please keep the generated files properly, otherwise you will not be able to unlock after locking, and the machine will not be able to upgrade anymore.**

## 5. Download Process

1.    run `make_vbmeta.sh -b <path-to-boot.img> -r <path-to-recovery.img>`, and generated vbmeta.img in "out" folder. (Remove -r option if no recovery.img)
2.    used these firmware about AVB:
- uboot.img, trust.img, MiniloaderAll.bin (`<chip_name>_loader_<version_lable>.bin>`) generated by U-Boot with [3.2. U-Boot](#3.2) configuration.
- vbmeta.img generated with step 1.
- parameter.txt generated with [3.3. Parameter](#3.3)
3.    If use windows download tool (AndroidTool), try to add "vbmeta" partition to the tool and blank the address. Then reload parameter.txt, the tool will update "vbmata" address by itself.
4.    After downloading, the device is in Unlock state defaultly.

## 6. AVB Lock and Unlock

AVB support Lock and Unlock states:
- Lock: verify the image next stage to u-boot like boot.img and recovery.img, and stop booting unvalid image, report error if image is unvalid
- Unlock: verify the image next stage to u-boot like boot.img and recovery.img, report error if image is unvalid, but still boot.

Therefore, setting the device to unlock state is convenient for debugging.

All of AVB user operation is used fastboot.
There are some ways to enter device fastboot mode:

1. press "fastboot" key at bootimg if there is "fastboot" in board
2. run "reboot fastboot" in system
3. run "fastboot usb 0" in uboot console. (set CONFIG_BOOTDELAY in uboot, it support specific delay to wait ctrl-c to enter uboot console)

Now, we can run fastboot command in PC to commuicate with device.

Download AVB root information (must done before "Lock" and "Unlock"):
~~~
sudo ./fastboot stage permanent_attributes.bin
sudo ./fastboot oem fuse at-perm-attr

# EFUSE only， skip this step if used OTP
sudo ./fastboot stage permanent_attributes_cer.bin
sudo ./fastboot oem fuse at-rsa-perm-attr
~~~

Lock the device:
~~~
sudo ./fastboot oem at-lock-vboot
sudo ./fastboot reboot
~~~

Unlock the device:
```
sudo ./fastboot oem at-get-vboot-unlock-challenge
sudo ./fastboot get_staged raw_unlock_challenge.bin # get raw_unlock_challenge.bin
./make_unlock.sh # generate unlock_credential.bin with raw_unlock_challenge.bin
sudo ./fastboot stage unlock_credential.bin
sudo ./fastboot oem at-unlock-vboot
```

## 7. Verity

If everything goes well, the log below will be shown if the device is LOCKED.
~~~
ANDROID: reboot reason: "(none)"
Could not find security partition
read_is_device_unlocked() ops returned that device is LOCKED
~~~
