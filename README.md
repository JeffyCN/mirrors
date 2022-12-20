# librga

RGA (Raster Graphic Acceleration Unit)是一个独立的2D硬件加速器，可用于加速点/线绘制，执行图像缩放、旋转、bitBlt、alpha混合等常见的2D图形操作。本仓库代码实现了RGA用户空间驱动，并提供了一系列2D图形操作API。

## 版本说明

**RGA API** 版本: 1.9.0

## 适用芯片平台

Rockchip RK3066 | RK3188 | RK2926 | RK2928 | RK3026 | RK3028 | RK3128 | Sofia3gr | RK3288 | RK3288w | RK3190 | RK1108 | RK3368 | RK3326 | RK3228 | RK3228H | RK3326 | RK1808 | RV1126 | RV1109 | RK3399 | RK3399pro | RK3566 | RK3568 | RK3588 | RK3326S | RV1106 | RV1103 | RK3528

## 目录说明

├── **im2d_api**：RGA API相关实现及头文件<br/>
├── **include**：RGA硬件相关头文件<br/>
├── **core**：RGA用户空间驱动实现<br/>
├── **docs**：FAQ以及API说明文档<br/>
├── **samples**：示例代码<br/>
├── **toolchains**：示例工具链配置文件<br/>
└──其余编译相关文件<br/>

## 编译说明

### Android Source Project

​	下载librga仓库拷贝至android源码工程 hardware/rockchip目录，配置好编译环境后，执行**mm**进行编译，根据不同的Android版本将自动选择Android.mk或Android.bp作为编译脚本。

```bash
$ mm -j16
```

### CMAKE

​	本仓库示例代码支持CMAKE编译，可以通过修改toolchain_*.cmake文件以及编译脚本实现快速编译。

#### 工具链修改

- **Android NDK（build for android）**

​	参考librga源码目录下**toolchains/toolchain_android_ndk.cmake**写法，修改NDK路径、Android版本信息等。

| 工具链选项                          | 描述                                         |
| ----------------------------------- | -------------------------------------------- |
| CMAKE_ANDROID_NDK                   | NDK编译包路径                                |
| CMAKE_SYSTEM_NAME                   | 平台名，默认为Android                        |
| CMAKE_SYSTEM_VERSION                | Android版本                                  |
| CMAKE_ANDROID_ARCH_ABI              | 处理器版本                                   |
| CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION | 工具链选择（clang/gcc）                      |
| CMAKE_ANDROID_STL_TYPE              | NDK C++库的链接方式（c++_static/c++_shared） |

- **Linux（buildroot/debian）**

​	参考librga源码目录下**toolchains/toolchain_linux.cmake**写法，修改工具链路径、名称。

| 工具链选项     | 描述       |
| -------------- | ---------- |
| TOOLCHAIN_HOME | 工具链目录 |
| TOOLCHAIN_NAME | 工具链名称 |

#### 编译脚本修改

​	修改samples目录或需要编译的示例代码目录下**cmake_*.sh**，指定toolchain路径。

| 编译选项       | 描述                                                         |
| -------------- | ------------------------------------------------------------ |
| TOOLCHAIN_PATH | toolchain的绝对路径，即《工具链修改》小节中修改后的toolchain_*.cmake文件的绝对路径 |
| LIBRGA_PATH    | 需要链接的librga.so的绝对路径，默认为librga cmake编译时的默认打包路径 |
| BUILD_DIR      | 编译生成文件存放的相对路径                                   |

#### 执行编译脚本

- **Android NDK（build for android）**

```bash
$ chmod +x ./cmake_android.sh
$ ./cmake_android.sh
```

- **Linux（buildroot/debian）**

```bash
$ chmod +x ./cmake_linux.sh
$ ./cmake_linux.sh
```

### Meson

​	本仓库提供了meson.build，buildroot/debian支持meson编译。单独编译可以使用meson.sh 脚本进行config，需要自行修改meson.sh 内指定install 路径，以及PATH等环境变量，cross目录下是交叉编译工具配置文件，也需要自行修改为对应交叉编译工具路径。

​	执行以下操作完成编译:

```bash
$ ./meson.sh
$ ninja -C build-rga install
```

## 使用说明

* **头文件引用**

  * C++调用im2d api

    im2d_api/im2d.hpp

  * C调用im2d api

    im2d_api/im2d.h

* **库文件**

  librga.so

  librga.a

* **librga应用开发接口说明文档**

  [IM2D API说明文档【中文】](docs/Rockchip_Developer_Guide_RGA_CN.md)

  [IM2D API说明文档【英文】](docs/Rockchip_Developer_Guide_RGA_EN.md)

* **RGA模块FAQ文档**

  [RGA_FAQ【中文】](docs/Rockchip_FAQ_RGA_CN.md)

  [RGA_FAQ【英文】](docs/Rockchip_FAQ_RGA_EN.md)

