ifndef CC
CC=gcc
endif

CONFIG_TLS_KEEP_ALIVE := 0

TARGET = keepalive

CONFIG_HOST_CALC_PSK := y
## Platform options: Ingenic, Rockchip #
CONFIG_PLATFORM := Rockchip
## Wi-Fi chip options: CYW43438, CYW43012, default is CYW43438 #
CONFIG_WIFI_CHIP := CYW43438

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
OBJS = ./libwl/wl_api.o ./libwl/wl_common.o ./libwl/wl_utils.o wifi.o main.o

ifeq ($(CONFIG_HOST_CALC_PSK),y)
CFLAGS += -DWL_CONFIG_HOST_CALC_PSK
OBJS += ./libwl/crypto/sha1-pbkdf2.o
OBJS += ./libwl/crypto/sha1-internal.o
OBJS += ./libwl/crypto/sha1.o
endif

CFLAGS += -g -Wall -Iinclude -Ilibwl

ifeq ($(CONFIG_TLS_KEEP_ALIVE),1)
# Compile option from OpenSSL Library Makefile
CFLAGS += -DOPENSSL_NO_CAMELLIA -DOPENSSL_NO_CAPIENG -DOPENSSL_NO_CAST -DOPENSSL_NO_CMS -DOPENSSL_NO_EC_NISTP_64_GCC_128 -DOPENSSL_NO_ENGINE -DOPENSSL_NO_GMP -DOPENSSL_NO_HEARTBEATS -DOPENSSL_NO_IDEA -DOPENSSL_NO_JPAKE -DOPENSSL_NO_MD2 -DOPENSSL_NO_MDC2 -DOPENSSL_NO_RC5 -DOPENSSL_NO_RFC3779 -DOPENSSL_NO_SCTP -DOPENSSL_NO_SEED -DOPENSSL_NO_STORE
CFLAGS += -DTLS_KEEP_ALIVE
LIBS += libssl.a libcrypto.a
endif

%.o:%.c
	$(CC) -c $(CFLAGS) $(LDFLAGS) $< -o $@

all: ${TARGET}

${TARGET} : $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS) $(CFLAGS)

install: ${TARGET}

.PHONEY: clean distclean

clean:	
	rm -f ${TARGET}
	rm -f ${OBJS}

distclean: clean
