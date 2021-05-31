ifndef CC
CC=gcc
endif

ifndef AR
AR=ar
endif

CONFIG_HOST_CALC_PSK := y
## Platform options: Ingenic, Rockchip #
CONFIG_PLATFORM := Ingenic
## Wi-Fi chip options: CYW43438, CYW43012, default is CYW43438 #
CONFIG_WIFI_CHIP := CYW43438

CFLAGS = -g -Wall

ifeq ($(CONFIG_PLATFORM),Rockchip)
CFLAGS += -D_ARM_ -g -Wall
CFLAGS += -DCONFIG_PLATFORM_ROCKCHIP
else
ifeq ($(CONFIG_PLATFORM),Ingenic)
CFLAGS += -O2 -muclibc -Wall
CFLAGS += -DCONFIG_PLATFORM_INGENIC
endif
endif

ifeq ($(CONFIG_WIFI_CHIP),CYW43012)
CFLAGS += -DWL_CONFIG_WIFI_CHIP=2
else
ifeq ($(CONFIG_WIFI_CHIP),CYW43438)
CFLAGS += -DWL_CONFIG_WIFI_CHIP=1
endif
endif

LDFLAGS = -lpthread
OBJS = wl_api.o wl_common.o wl_utils.o

ifeq ($(CONFIG_HOST_CALC_PSK),y)
CFLAGS += -DWL_CONFIG_HOST_CALC_PSK
OBJS += ./crypto/sha1-pbkdf2.o
OBJS += ./crypto/sha1-internal.o
OBJS += ./crypto/sha1.o
endif

%.o:%.c
	$(CC) -c $(CFLAGS) $(LDFLAGS) $< -o $@

all: libwl

libwl: $(OBJS)
	$(AR) $@.a $(OBJS)

install: libwlapi

.PHONEY: clean distclean

clean:
	rm -f libwl.a
	rm -f ${OBJS}

distclean: clean
