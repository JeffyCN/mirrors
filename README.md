## librga

RGA (Raster Graphic Acceleration Unit)是一个独立的2D硬件加速器，可用于加速点/线绘制，执行图像缩放、旋转、bitBlt、alpha混合等常见的2D图形操作。本仓库代码实现了RGA用户空间驱动，并提供了一系列2D图形操作API。

### 版本说明

**RGA API** 版本: 1.2.5

### 适用芯片平台

Rockchip RK3066 | RK3188 | RK2926 | RK2928 | RK3026 | RK3028 | RK3128 | Sofia3gr | RK3288 | RK3288w | RK3190 | RK1108 | RK3368 | RK3326 | RK3228 | RK3228H | RK3326 | RK1808 | RV1126 | RV1109 | RK3399 | RK3399pro | RK3566 | RK3568

### 目录说明

**core:** RGA用户空间驱动实现

**include:** 相关头文件

**im2d_api:** RGA API相关实现及头文件

**samples:** librga使用例程

**samples/sample_file：**示例图片。

### 编译说明

* **Android Source Project**

下载librga仓库拷贝至android源码工程 hardware/rockchip目录，执行**mm**进行编译。根据不同的Android版本将自动选择Android.mk或Android.bp作为编译脚本。

* **Android NDK (build for android)**

修改librga源码根目录下的文件**cmake-android.sh**。执行以下操作完成编译：

```bash
$ mkdir build
$ cd build
$ cp ../cmake-android.sh ./
$ chmod +x ./cmake-android.sh
$ ./cmake-android.sh
$ make
```

**[编译选项]**

1. 指定ANDROID_NDK_HOME为NDK开发包的路径
2. 指定CMAKE_ANDROID为android SDK包中cmake可执行文件的路径
3. 根据需要选择不同架构，设置-DANDROID_ABI等于armeabi-v7a或arm64-v8a
4. 根据需要选择不同的android平台版本，设置-DANDROID_PLATFORM

* **Cmake (buildroot/debian)**

修改librga源码根目录下的**buildroot.cmake**文件。执行以下操作完成编译:

```bash
$ mkdir build
$ cd build
$ cp ../cmake-linux.sh ./
$ chmod +x ./cmake-linux.sh
$ ./cmake-linux.sh
```

**[编译选项]**

1. 指定TOOLCHAIN_HOME为交叉编译工具的路径
2. 指定CMAKE_C_COMPILER为gcc编译命令的路径
3. 指定CMAKE_CXX_COMPILER为g++编译命令的路径

* **Meson(buildroot/debian)**

librga提供了meson.build，最新buildroot支持meson 编译。单独编译可以使用meson.sh 脚本进行config，需要自行修改meson.sh 内指定install 路径，以及PATH等环境变量，cross目录下是交叉编译工具配置文件，也需要自行修改为对应交叉编译工具路径。

执行以下操作完成编译:

```bash
$ ./meson.sh
$ ninja -C build-rga install
```

### 使用说明

* **头文件引用**

  * 调用librga

    include/RockchipRga.h

  * 调用im2d api

    im2d_api/im2d.hpp

  * C_API

    include/RgaApi.h

* **库文件**

  librga.so

* librga应用开发接口说明参考以下文件：

  [API document](docs/README.md)

