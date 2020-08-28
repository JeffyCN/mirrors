蓝牙音频AVRCP API是基于BLue5 在封装的API，使用如下：

1、在lib目录中放入libgdbus-internal.a 注意32位系统请使用arm目录下libgdbus-internal.a, 64位系统请使用arm64目录下libgdbus-internal.a

2、编译配置
在Makefile 加入库的引用
 -lglib-2.0  -ldbus-1 -ldbus-glib-1  -lreadline
CFLAGS 加入
 $(shell pkg-config --libs dbus-glib-1) $(shell pkg-config --libs glib-2.0) $(shell pkg-config --cflags dbus-1) 
 
3、添加avrcpctrl.h avrcpctrl.c gdbus.h 到APP工程中