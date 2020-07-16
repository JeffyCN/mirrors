RGA (Raster Graphic Acceleration Unit)是一个独立的2D硬件加速器，可用于加速点/线绘制，执行图像缩放、旋转、bitBlt、alpha混合等常见的2D图形操作。

### 版本说明

librga 版本: v1.03
librga API版本: v1.00

### 适用芯片平台

Rockchip rk3066 | rk3188 | rk2926 | rk2928 | rk3026 | rk3028 | rk3128 | Sofia3gr | rk3288 | rk3288w | rk3190 | rk1108 | rk3368 | rk3326 | rk3228 | rk3228H | rk3326 | rk1808 | rv1126 | rv1109 | rk3399 | rk3399pro

### 编译说明

* **Android Source Project**

下载librga仓库拷贝至android源码工程 hardware/rockchip目录，执行**mm**进行编译。根据不同的Android版本将自动选择Android.mk或Android.bp作为编译脚本。

* **Android NDK (build for android)**

修改librga源码根目录下的文件**ndk-android.sh**，指定**ANDROID_NDK_HOME**为NDK开发包的路径。执行以下操作完成编译：

```bash
$ mkdir build
$ cd build
$ cp ../cmake-android.sh ./
$ chmod +x ./cmake-android.sh
$ ./cmake-android.sh
$ make
```

* **Cmake (buildroot/debian)**

修改librga源码根目录下的**buildroot.cmake**文件，指定**TOOLCHAIN_HOME**为交叉编译工具的路径。执行以下操作完成编译:

```bash
$ mkdir build
$ cd build
$ cp ../cmake-linux.sh ./
$ chmod +x ./cmake-linux.sh
$ ./cmake-linux.sh
$ make
```

* **Makefile(Buildroot/Debian)**

librga也提供了Makefile文件，可以在开发板上直接执行make命令，或修改Makefile自行定义交叉编译工具后执行make完成编译。

### 使用说明

* **头文件引用**

  RockchipRga.h

  im2d_api/im2d.h

  im2d_api/im2d.hpp

* **库文件**

  librga.so

* librga应用开发接口说明参考以下文件：

  [API document](im2d_api/readme.md)

