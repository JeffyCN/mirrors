版本说明：
此版本为RGA1v2,相对于原来的RGA兼容性更好，并且解决了很多bug。目前检测过以下平台，已确认运行OK

Rockchip rk3188  4.4 / 5.1 / 7.1
Rockchip rk3128  4.4 / 7.1
Rockchip rk3228H 4.4 / 7.1
Rockchip rk3288  4.4 / 5.1 / 6.0 / 7.1
Rockchip rk3368  5.1 / 6.0 / 7.1 / 8.0
Rockchip rk3399  6.0 / 7.1

RGA使用帮助：

1、解压文件到hardware/rockchip/librga或者hardware/rk29/librga

2、在/librga目录下编译，在~/out/target/product/rk312x（sdk版本）/system/lib/生成librga.so链接库，
push librga.so到设备system/lib 目录 （64位系统对应lib64）

3、librga/demo/目录提供若干demo代码，包含拷贝，旋转，合成，格式转换，颜色填充等功能，
到对应librga/demo/rgaXXXXX目录 mm -B -j4 即可编译。生成可执行文件在~/out/target/product/rk312x（sdk版本）/system/bin 目录

4、librga/image目录是测试用的二进制源数据，可作为前期熟悉RGA的测试src_data，按需使用。可用7YUV应用打开。

5、librga/normal目录RGA接口实现源代码，兼容rga各个版本，请按需阅读

6、按照一定规则命名二进制文件
/**********************************************************************************************************
   in%dw%d-h%d-%s.bin解释如下：
--->第一个%d是文件的索引，一般为0，防止所有格式一样，导致无法使用，比如合成的时候，可能两个文件完全一样的属性
--->第二个%d是宽的意思，这里的宽一般指虚宽
--->第三个%d是宽的意思，这里的高一般指虚高
--->第四个%s是格式的名字，目前格式名字如下：
*****************************************************************
*HAL_PIXEL_FORMAT_RGB_565:                       "rgb565";      *
*HAL_PIXEL_FORMAT_RGB_888:                       "rgb888";      *
*HAL_PIXEL_FORMAT_RGBA_8888:                     "rgba8888";    *
*HAL_PIXEL_FORMAT_RGBX_8888:                     "rgbx8888";    *
*HAL_PIXEL_FORMAT_BGRA_8888:                     "bgra8888";    *
*HAL_PIXEL_FORMAT_YCrCb_420_SP:                  "crcb420sp";   *
*HAL_PIXEL_FORMAT_YCrCb_NV12:                    "nv12";        *
*HAL_PIXEL_FORMAT_YCrCb_NV12_VIDEO:              "nv12";        *
*HAL_PIXEL_FORMAT_YCrCb_NV12_10:                 "nv12_10";     *
*****************************************************************
*/