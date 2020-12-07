ifndef CC
CC=gcc
endif

TARGET = keepalive
CFLAGS = -D_ARM_ -g -Wall
LDFLAGS = -lpthread
OBJS = main.o wl_api.o wl_common.o wl_utils.o print_scan.o wifi.o

%.o:%.c
	$(CC) -c $(CFLAGS) $(LDFLAGS) $< -o $@

all: ${TARGET}

${TARGET} : $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(CFLAGS)

install: ${TARGET}

.PHONEY: clean distclean

clean:	
	rm -f ${TARGET}
	rm -f ${OBJS}

distclean: clean
