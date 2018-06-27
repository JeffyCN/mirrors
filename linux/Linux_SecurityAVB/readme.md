# **RK3308 AVB Reference**

发布版本：1.0

作者邮箱：jason.zhu@rock-chips.com

日期：2018.05.28

文件密级：公开资料

------

**前言**
​	适用于RK3308。

**概述**

**产品版本**

| **芯片名称** | **内核版本** |
| -------- | -------- |
| RK3308   | 所有内核版本   |

**读者对象**

本文档（本指南）主要适用于以下工程师：

技术支持工程师

软件开发工程师

**修订记录**

| **日期**     | **版本** | **作者**    | **修改说明**                     |
| ---------- | ------ | --------- | ---------------------------- |
| 2018.05.28 | V1.0   | jason.zhu |                              |
| 2018.06.12 | V1.1   | jason.zhu | 增加 Authenticated Unlock及一些说明 |
| 2018.06.27 | V1.2   | Zain.Wong | 增加 key生成描述 |

---

[TOC]

## 1 . 注意事项

1.1. boot.img不能超过9MB
1.2. 关于device lock & unlock

​        当设备处于unlock状态，程序还是会校验整个boot.img，如果固件有错误，程序会报具体是什么错误，**正常启动设备**。而如果设备处于lock状态，程序会校验整个boot.img，如果固件有误，则不会启动下一级固件。所以调试阶段设置device处于unlock状态，方便调试。

​	一旦设备处理lock状态，就需要Authenticated Unlock，具体流程参见2. avb lock & unlock。

## 2 . 固件配置

2.1. trust

   进入rkbin/RKTRUST，找到RK3308TRUST.ini，修改
```
[BL32_OPTION]
SEC=0
PATH=bin/rk33/rk3308_bl32_v1.00.bin //v1.00版本或以上版本
改为
[BL32_OPTION]
SEC=1
PATH=bin/rk33/rk3308_bl32_v1.00.bin //v1.00版本或以上版本
```

2.2. uboot

   uboot需要fastboot和optee支持。
   uboot下打上avb.patch。
   使用./make.sh evb-rk3308，生成uboot.img, trust.img, rk3308_loader_v1.21.103.bin
   说明：

```
CONFIG_OPTEE_CLIENT=y
CONFIG_OPTEE_V2=y
```

​	avb开启需要在config文件中配置

```
CONFIG_AVB_LIBAVB=y
CONFIG_AVB_LIBAVB_AB=y
CONFIG_AVB_LIBAVB_ATX=y
CONFIG_AVB_LIBAVB_USER=y
CONFIG_RK_AVB_LIBAVB_USER=y
上面几个为必选
CONFIG_ANDROID_AB=y //这个支持a/b
CONFIG_ANDROID_AVB=y //这个支持avb
```

​	固件，ATX及hash需要通过fastboot烧写，所以需要在config文件中配置

```
CONFIG_FASTBOOT=y
CONFIG_FASTBOOT_BUF_ADDR=0x800800
CONFIG_FASTBOOT_BUF_SIZE=0x04000000
CONFIG_FASTBOOT_FLASH=y
CONFIG_FASTBOOT_FLASH_MMC_DEV=0
```

2.3. parameter

parameter可以使用该目录下的gpt-emmc-security.txt,或gpt-emmc-nand-security.txt.
如果不满足需求，可以自行修改。

使用AVB需要在parameter中新增vbmeta分区，用来存放avb信息，分区大小为1M，位置无关。
如果存储介质为nand，还需要额外添加security分区，大小4M，位置无关。

AVB分区中，必须有system分区，如果使用buildroot，请将rootfs分区改为system分区，
下载的时候，工具上的名称要同步修改，修改后，重载parameter。

## 3 . Key

~~~
该目录下已经有一套测试的证书和key，如果需要新的key和证书，可以按下面步骤自行生成。
请妥善保管生成的文件，否则加锁之后将无法解锁，机子将无法刷机。
~~~

谷歌定义的avb需要用到的keys定义如下：
1. Product RootKey (PRK)：所有的key校验或派生都依据此key，谷歌提供保管。如果只用到avb，而没有用到需
要谷歌认证的服务，可以由厂商自己生成。
2. ProductIntermediate Key (PIK)：中间key，中介作用，谷歌提供保管。如果只用到avb，而没有用到需要谷歌
认证的服务，可以由厂商自己生成。
3. ProductSigning Key (PSK)：用于签固件的key，谷歌提供保管。如果只用到avb，而没有用到需要谷歌认证的服
务，可以由厂商自己生成。
4. ProductUnlock Key (PUK)：用于解锁设备，谷歌提供保管。如果只用到avb，而没有用到需要谷歌认证的服
务，可以由厂商自己生成。

ATX：Android Things Extension for validating public key metadata。
ATX permanent attributes保存设备的device id及Product RootKey (PRK)的公钥。
vbmeta保存ProductIntermediate Key (PIK)证书，ProductSigning Key (PSK)证书。
三把keys及证书生成。

~~~
    openssl genpkey ‐algorithm RSA ‐pkeyopt rsa_keygen_bits:4096 ‐outform PEM ‐out
testkey_atx_prk.pem

    openssl genpkey ‐algorithm RSA ‐pkeyopt rsa_keygen_bits:4096 ‐outform PEM ‐out
testkey_atx_psk.pem

    openssl genpkey ‐algorithm RSA ‐pkeyopt rsa_keygen_bits:4096 ‐outform PEM ‐out
testkey_atx_pik.pem

    python avbtool make_atx_certificate ‐‐output=atx_pik_certificate.bin ‐‐subject=temp.bin ‐‐
subject_key=testkey_atx_pik.pem ‐‐subject_is_intermediate_authority ‐‐subject_key_version 42 ‐‐
authority_key=testkey_atx_prk.pem

    python avbtool make_atx_certificate ‐‐output=atx_psk_certificate.bin ‐‐
subject=atx_product_id.bin ‐‐subject_key=testkey_atx_psk.pem ‐‐subject_key_version 42 ‐‐
authority_key=testkey_atx_pik.pem

    python avbtool make_atx_metadata ‐‐output=atx_metadata.bin ‐‐
intermediate_key_certificate=atx_pik_certificate.bin ‐‐
product_key_certificate=atx_psk_certificate.bin
~~~

其中temp.bin需要自己创建的临时文件，新建temp.bin即可，无需填写数据。
atx_permanent_attributes.bin生成：

~~~
    python avbtool make_atx_permanent_attributes ‐‐output=atx_permanent_attributes.bin ‐‐
product_id=atx_product_id.bin ‐‐root_authority_key=testkey_atx_prk.pem
~~~

其中atx_product_id.bin需要自己定义，占16字节，可作为产品ID定义。

PUK生成：
~~~
  openssl genpkey ‐algorithm RSA ‐pkeyopt rsa_keygen_bits:4096 ‐outform PEM ‐out
testkey_atx_puk.pem
~~~

atx_unlock_credential.bin为需要下载到设备解锁的证书，其生成过程如下：
~~~
  python avbtool make_atx_certificate ‐‐output=atx_puk_certificate.bin ‐‐
subject=atx_product_id.bin ‐‐subject_key=testkey_atx_puk.pem ‐‐
usage=com.google.android.things.vboot.unlock ‐‐subject_key_version 42 ‐‐
authority_key=testkey_atx_pik.pem

  python avbtool make_atx_unlock_credential ‐‐output=atx_unlock_credential.bin ‐‐
intermediate_key_certificate=atx_pik_certificate.bin ‐‐
unlock_key_certificate=atx_puk_certificate.bin ‐‐challenge=atx_unlock_challenge.bin ‐‐
unlock_key=testkey_atx_puk.pem
~~~

## 4 . 操作流程

1. 把boot.img放到这个目录下
2. 运行make_vbmeta.sh,生成vbmeta.bin和加密过的boot.img
3. 替换固件:
   uboot.img, trust.img, MiniloaderAll.bin替换成上一个步骤中，uboot生成的3个固件。
   boot.img使用该目录下生成的加密固件。
   vbmeta.bin提取出来。
   parameter.txt使用该目录下的2种带security的。或自行在原来parameter中添加分区（详见2-3）
4. 使用工具烧录。
   如果使用的windows工具，请在工具中添加vbmeta分区（如果是nand，还需额外添加security分区），地址不填。
   然后重新加载parameter，工具会自行更新地址。
5. 使能atx校验：
   启动系统，在console里面输入reboot fastboot，进入fastboot模式 （重启使用fastboot reboot）
   电脑端输入（可能需要管理员权限）

~~~
fastboot stage atx_permanent_attributes.bin
fastboot oem fuse at-perm-attr
~~~

​	atx_permanent_attributes.bin也在该文件夹下。

## 5 . avb lock & unlock

​	锁定设备：

~~~
fastboot oem at-lock-vboot
~~~

​	现在uboot这里做了Authenticated Unlock可以用于解锁后级boot.img的校验。

​	解锁设备步骤：

1. 设备进入fastboot模式，电脑端输入

```
fastboot oem at-get-vboot-unlock-challenge
fastboot get-staged raw_atx_unlock_challenge.bin
```

2. raw_atx_unlock_challenge.bin放进本文件夹内，运行

```
./make_unlock.sh
```

​	生成atx_unlock_challenge.bin和atx_unlock_credential.bin。

3. 电脑端输入

```
fastboot stage atx_unlock_credential.bin
fastboot oem at-unlock-vboot
```

​	**注意**：此时设备状态一直处于第一次进入fastboot模式状态，在此期间不能断电、关机、重启。因为步骤1.做完后，设备也存储着生成的随机数，如果断电、关机、重启，会导致随机数丢失，后续校验challenge signature会因为随机数不匹配失败。

4. 设备进入解锁状态，开始解锁。

## 6 . 最终打印

~~~
<debug_uart> 
U-Boot TPL 2017.09-01346-g0068ab3 (May 11 2018 - 17:46:30)
00000000In
589MHz
DDR3
 Col=10 Bank=8 Row=15 Size=512MB
msch:1
00015a08Returning to boot ROM...
Boot1 Release Time: Mar 29 2018 14:20:07, version: 1.01
chip_id:000,0
ChipType = 0x13, 35791055
DPLL = 1300 MHz
NeedKHz=200KHz,clock=12000KHz
NeedKHz=200KHz,clock=12000KHz
NeedKHz=200KHz,clock=12000KHz
DPLL = 1300 MHz
NeedKHz=18000KHz,clock=650000KHz
DPLL = 1300 MHz
NeedKHz=48000KHz,clock=650000KHz
mmc2:cmd19,256
SdmmcInit=2 0
BootCapSize=2000
UserCapSize=7456MB
FwPartOffset=2000 , 2000
SdmmcInit=0 NOT PRESENT
StorageInit ok = 35752692
SecureMode = 0
Secure read PBA: 0x4
Secure read PBA: 0x404
Secure read PBA: 0x804
Secure read PBA: 0xc04
Secure read PBA: 0x1004
SecureInit ret = 0, SecureMode = 0
LoadTrustBL
No find bl30.bin
Load uboot, ReadLba = 2000
Load OK, addr=0x200000, size=0xc2fb8
RunBL31 0x10000
NOTICE:  BL31: v1.3(debug):490c474
NOTICE:  BL31: Built : 14:21:24, May 11 2018
NOTICE:  BL31:Rockchip release version: v1.0
INFO:    ARM GICv2 driver initialized
INFO:    Using opteed sec cpu_context!
INFO:    boot cpu mask: 1
INFO:    plat_rockchip_pmu_init: pd status 0xe
INFO:    BL31: Initializing runtime services
INFO:    BL31: Initializing BL32
INFO:    TEE-CORE: 
INFO:    TEE-CORE: Start rockchip platform init
INFO:    TEE-CORE: Rockchip release version: 1.0
INFO:    TEE-CORE: OP-TEE version: 2.6.0-91-g916f2a2-dev #37 Mon May 28 11:46:39 UTC 2018 aarch64
INFO:    TEE-CORE: Initialized
INFO:    BL31: Preparing for EL3 exit to normal world
INFO:    Entry point address = 0x200000
INFO:    SPSR = 0x3c9


U-Boot 2017.09-01565-g46a684d-dirty (May 28 2018 - 21:18:50 +0800)

Model: Rockchip RK3308 EVB
DRAM:  480 MiB
Relocation Offset is: 1fcf4000
MMC:   dwmmc@ff490000: 0
Using default environment

In:    serial@ff0c0000
Out:   serial@ff0c0000
Err:   serial@ff0c0000
Model: Rockchip RK3308 EVB
MMC Device 1 not found
no mmc device at slot 1
switch to partitions #0, OK
mmc0(part 0) is current device
boot mode: normal
Net:   Net Initialization Skipped
No ethernet found.
Hit any key to stop autoboot:  0 
ca head not found
ANDROID: reboot reason: "(none)"
Could not find security partition
rkss_read_section fail ! ret: -65536.read_is_device_unlocked() ops returned that device is LOCKED
Could not find security partition
rkss_read_section fail ! ret: -65536.INFO:    USER-TA: Hello Rockchip Keymaster! rpmb :1
INFO:    USER-TA: TEE_ReadObjectData success !
INFO:    USER-TA: Goodbye Rockchip Keymaster!
Could not find security partition
rkss_read_section fail ! ret: -65536.Could not find security partition
rkss_read_section fail ! ret: -65536.Could not find security partition
rkss_read_section fail ! ret: -65536.Could not find security partition
rkss_read_section fail ! ret: -65536.Could not find security partition
rkss_read_section fail ! ret: -65536.Could not find security partition
rkss_read_section fail ! ret: -65536.Could not find security partition
rkss_read_section fail ! ret: -65536.Could not find security partition
rkss_read_section fail ! ret: -65536.INFO:    USER-TA: Hello Rockchip Keymaster! rpmb :1
INFO:    USER-TA: TEE_ReadObjectData success !
INFO:    USER-TA: Goodbye Rockchip Keymaster!
FDT load addr 0x10f00000 size 229 KiB
Booting kernel at 0x207f800 with fdt at 27ec800...


## Booting Android Image at 0x0207f800 ...
Kernel load addr 0x02080000 size 7599 KiB
## Flattened Device Tree blob at 027ec800
   Booting using the fdt blob at 0x27ec800
   XIP Kernel Image ... OK
   Loading Device Tree to 00000000081f2000, end 00000000081ff4a7 ... OK
Adding bank: start=0x00200000, size=0x08200000
Adding bank: start=0x0a200000, size=0x15e00000

Starting kernel ...

[    0.000000] Booting Linux on physical CPU 0x0
[    0.000000] Linux version 4.4.126 (yhx@srv165) (gcc version 6.3.1 20170404 (Linaro GCC 6.3-2017.05) ) #1 SMP PREEMPT Thu May 10 20:55:18 CST 2018
