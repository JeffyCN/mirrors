# **AVB Reference**

发布版本：2.2

作者邮箱：jason.zhu@rock-chips.com

日期：2019.06.03

文件密级：公开资料

------

## **前言**

   适用于RK3308 / RK3326 / PX30 / RK3399 / RK3328。

## **概述**

## **产品版本**

| **芯片名称** | **内核版本** |
| -------- | -------- |
| RK3308   | 所有内核版本   |
| RK3326   | Kernel 4.4   |
| RK3399   | Kernel 4.4   |
| RK3328   | Kernel 4.4   |

## **读者对象**

本文档（本指南）主要适用于以下工程师：

- 技术支持工程师
- 软件开发工程师

## **修订记录**

| **日期**     | **版本** | **作者**    | **修改说明**                     |
| ---------- | ------ | --------- | ---------------------------- |
| 2018.05.28 | V1.0   | jason.zhu |                              |
| 2018.06.12 | V1.1   | jason.zhu | 增加 Authenticated Unlock及一些说明 |
| 2018.06.27 | V1.2   | Zain.Wong | 增加 key生成描述 |
| 2018.11.16 | V1.3   | Zain.Wong | 增加 uboot 配置说明， 兼容rk3326 |
| 2019.01.28 | v2.0   | Zain.Wong | 同步 uboot 更改，增加 3399 配置 |
| 2019.05.22 | v2.1   | Zain.Wong | 增加 3328 支持，并区分 OTP/EFUSE avb 烧写命令 |
| 2019.06.03 | v2.2   | Zain.Wong | 修复一些不恰当的说明 |

[TOC]

## 1 . 注意事项

关于device lock & unlock

  当设备处于unlock状态，程序还是会校验整个boot.img，如果固件有错误，程序会报具体是什么错误，**正常启动设备**。而如果设备处于lock状态，程序会校验整个boot.img，如果固件有误，则不会启动下一级固件。所以调试阶段设置device处于unlock状态，方便调试。
  一旦设备处理lock状态，就需要Authenticated Unlock，具体流程参见5. avb lock & unlock。

## 2 . 固件配置

2.1. trust

进入rkbin/RKTRUST，以rk3308为例，找到RK3308TRUST.ini，修改

```
[BL32_OPTION]
SEC=0
改为
[BL32_OPTION]
SEC=1
```

2.2. uboot

 uboot需要fastboot和optee支持。

```
CONFIG_OPTEE_CLIENT=y
CONFIG_OPTEE_V1=y	#rk312x/rk322x/rk3288/rk3228H/rk3368/rk3399 与V2互斥
CONFIG_OPTEE_V2=y	#rk3308/rk3326 与V1互斥
```

​avb开启需要在config文件中配置

```
CONFIG_AVB_LIBAVB=y
CONFIG_AVB_LIBAVB_AB=y
CONFIG_AVB_LIBAVB_ATX=y
CONFIG_AVB_LIBAVB_USER=y
CONFIG_RK_AVB_LIBAVB_USER=y
CONFIG_AVB_VBMETA_PUBLIC_KEY_VALIDATE=y
CONFIG_CRYPTO_ROCKCHIP=y
CONFIG_ANDROID_AVB=y
CONFIG_ANDROID_AB=y	#有需要再开
CONFIG_OPTEE_ALWAYS_USE_SECURITY_PARTITION=y	#rpmb无法使用时打开，默认不开
CONFIG_ROCKCHIP_PRELOADER_PUB_KEY=y #efuse 安全方案需要打开
```

​固件，certificate及hash需要通过fastboot烧写，所以需要在config文件中配置

```
CONFIG_FASTBOOT=y
CONFIG_FASTBOOT_BUF_ADDR=0x800800	#各芯片平台不同
CONFIG_FASTBOOT_BUF_SIZE=0x04000000	#各芯片平台不同
CONFIG_FASTBOOT_FLASH=y
CONFIG_FASTBOOT_FLASH_MMC_DEV=0
```

使用./make.sh xxxx，生成uboot.img, trust.img, loader.bin

2.3. parameter

AVB需要添加vbmeta分区，用来存放固件签名信息。大小1M，位置无关。

AVB需要system分区，在buildroot上，即rootfs分区，需要将rootfs改名为system，如果使用了uuid，同时修改uuid分区名。

如果存储介质使用flash，还需要另外添加security分区，用来存放操作信息。内容加密存放。大小4M，位置无关。emmc无需添加该分区，emmc操作信息存放在物理rpmb分区

以下是avb parameter例子：
~~~
0x00002000@0x00004000(uboot),0x00002000@0x00006000(trust),0x00002000@0x00008000(misc),0x00010000@0x0000a000(boot),0x00010000@0x0001a000(recovery),0x00010000@0x0002a000(backup),0x00020000@0x0003a000(oem),0x00300000@0x0005a000(system),0x00000800@0x0035a000(vbmeta),0x00002000@0x0035a800(security),-@0x0035c800(userdata:grow)
~~~

avb ab parameter:
~~~
0x00002000@0x00004000(uboot),0x00002000@0x00006000(trust_a),0x00002000@0x00008000(trust_b),0x00002000@0x0000a000(misc),0x00010000@0x0000c000(boot_a),0x00010000@0x0001c000(boot_b),0x00010000@0x0002c000(backup),0x00020000@0x0003c000(oem),0x00300000@0x0005c000(system_a),0x00300000@0x0035c000(system_b),0x00000800@0x0065c000(vbmeta_a),0x00000800@0x0065c800(vbmeta_b),0x00002000@0x0065d000(security),-@0x0065f00(userdata:grow)
~~~

下载的时候，工具上的名称要同步修改，修改后，重载parameter。

## 3 . Key

AVB中包含以下4把key：
Product RootKey (PRK)：avb的root key
ProductIntermediate Key (PIK)：中间key，中介作用
ProductSigning Key (PSK)：用于签固件的key
ProductUnlock Key (PUK)：用于解锁设备

**该目录下已经有一套测试的证书和key，如果需要新的key和证书，可以按下面步骤自行生成。
请妥善保管生成的文件，否则加锁之后将无法解锁，机子将无法刷机。**

~~~
    openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:4096 -outform PEM -out testkey_prk.pem
    openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:4096 -outform PEM -out testkey_psk.pem
    openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:4096 -outform PEM -out testkey_pik.pem
    touch temp.bin
    python avbtool make_atx_certificate --output=pik_certificate.bin --subject=temp.bin --subject_key=testkey_pik.pem --subject_is_intermediate_authority --subject_key_version 42 --authority_key=testkey_prk.pem
    echo "RKXXXX_nnnnnnnn" > product_id.bin
    python avbtool make_atx_certificate --output=psk_certificate.bin --subject=product_id.bin --subject_key=testkey_psk.pem --subject_key_version 42 --authority_key=testkey_pik.pem
    python avbtool make_atx_metadata --output=metadata.bin --intermediate_key_certificate=pik_certificate.bin --product_key_certificate=psk_certificate.bin
~~~

其中temp.bin需要自己创建的临时文件，新建temp.bin即可，无需填写数据。
product_id.bin需要自己定义，占16字节，可作为产品ID定义。
permanent_attributes.bin生成：

~~~
    python avbtool make_atx_permanent_attributes --output=permanent_attributes.bin --product_id=product_id.bin --root_authority_key=testkey_prk.pem
~~~

PUK生成：
~~~
    openssl genpkey -algorithm RSA -pkeyopt rsa_keygen_bits:4096 -outform PEM -out testkey_puk.pem
~~~

puk_certificate.bin permanent_attributes.bin为设备解锁的证书，
生成过程需要用到PrivateKey.pem，该key为烧录进efuse/otp的key
(参考Rockchip-Secure-Boot-Application-Note-V1.9), 其生成过程如下：
~~~
    python avbtool make_atx_certificate --output=puk_certificate.bin --subject=product_id.bin --subject_key=testkey_puk.pem --usage=com.google.android.things.vboot.unlock --subject_key_version 42 --authority_key=testkey_pik.pem
~~~

针对efuse设备，还需要另外生成 permanent_attributes_cer.bin（otp设备可以跳过）
~~~
    openssl dgst -sha256 -out permanent_attributes_cer.bin -sign PrivateKey.pem permanent_attributes.bin
~~~

## 4 . 修改脚本

签名脚本为make_vbmeta.sh
给固件签名的格式为：

~~~
python avbtool add_hash_footer --image <IMG> --partition_size <SIZE> --partition_name <PARTITION> --key testkey_psk.pem --algorithm SHA512_RSA4096
~~~

IMG 为签名固件
SIZE 为签名后，固件大小，至少比原文件大64K，且不超过parameter中定义的分区大小，大小必须4K对齐
PARTITION = boot / recovery

签名完成后，用签名过的文件生成vbmeta.img
基本格式：
~~~
python avbtool make_vbmeta_image --public_key_metadata metadata.bin --include_descriptors_from_image <IMG> --algorithm SHA256_RSA4096 --rollback_index 0 --key testkey_psk.pem  --output vbmeta.img
~~~

--include_descriptors_from_image <IMG> 该字段可以多次使用，即有多少个加密过的文件，就添加多少个 --include_descriptors_from_image。
例如：
```
python avbtool make_vbmeta_image --public_key_metadata metadata.bin --include_descriptors_from_image boot.img --include_descriptors_from_image recovery.img --algorithm SHA256_RSA4096 --rollback_index 0 --key testkey_psk.pem  --output vbmeta.img
```

可按照上述规则自行修改make_vbmeta.sh脚本

## 5 . 操作流程

1. 把boot.img/recovery.img放到这个目录下
2. 运行make_vbmeta.sh,生成vbmeta.bin和加密过的boot.img/recovery.img
3. 替换固件:
   uboot.img, trust.img, MiniloaderAll.bin替换成新配置uboot生成的3个固件。
   boot.img使用该目录下生成的加密固件。
   vbmeta.bin提取出来。
   parameter.txt 按 2.3 中规则修改
4. 使用工具烧录。
   如果使用的windows工具，请在工具中添加vbmeta分区（security分区视parameter而定），地址不填。
   然后重新加载parameter，工具会自行更新地址。
   如果security/rpmb中avb数据为空，uboot会直接进fastboot，等待fastboot对应信息写入（跳过5）
   需要使用fastboot成功lock住设备，然后重启才能进boot
5. 下载之后，设备默认处于unlock状态，此时固件还是会校验，但是不会阻拦系统启动，只会报错。

## 5 . avb lock & unlock

AVB 只有在lock状态下，才会真正阻拦非签名固件的启动。
首先，需要将设备进入到fastboot模式，大致有3种途径：

1. 启动到系统中，运行reboot fastboot
2. 进入到uboot 命令行中，输入fastboot usb 0
3. 如果有fastboot按键，通过fastboot按键进入fastboot模式。

   然后PC 通过fastboot命令操作（可能需要管理员权限）

pub_key 烧写
~~~
sudo ./fastboot stage permanent_attributes.bin
sudo ./fastboot oem fuse at-perm-attr

# EFUSE only， skip this step if used OTP
sudo ./fastboot stage permanent_attributes_cer.bin
sudo ./fastboot oem fuse at-rsa-perm-attr
~~~

lock
~~~
sudo ./fastboot oem at-lock-vboot
sudo ./fastboot reboot
~~~

​解锁设备步骤：

1. 设备进入fastboot模式，电脑端输入

```
sudo ./fastboot oem at-get-vboot-unlock-challenge
sudo ./fastboot get_staged raw_unlock_challenge.bin
```

2. raw_unlock_challenge.bin放进本文件夹内，运行

```
./make_unlock.sh
```

​生成unlock_credential.bin。

3. 电脑端输入

```
sudo ./fastboot stage unlock_credential.bin
sudo ./fastboot oem at-unlock-vboot
```

## 6 . 最终打印

~~~
ANDROID: reboot reason: "(none)"
Could not find security partition
read_is_device_unlocked() ops returned that device is LOCKED
~~~
