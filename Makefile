PWD=$(shell pwd)

TOP_DIR ?= $(PWD)/../../..
ifeq ("$(HOST)","")
#target build
##include $(TOP_DIR)/include/target-build.mk
else
#host build
endif



#################################
# App config
#################################
app=wificmd
CFLAGS+= -Wall
LDFLAGS+=


#################################
# Framework for dependency check
#################################
sources=$(wildcard *.c)
CFLAGS+= -MMD -MP
LDFLAGS+=

CFLAGS += -I$(TARGET_BUILD_DIR)/include
##LDFLAGS += -L$(TARGET_BUILD_DIR)/lib
##LDLIBS += -lcms -lcms_helper -laes_pbkdf2 -lssl -lcrypto -ljson-c -lm

app_objs = wifi.o common.o config.o openwrt.o cms.o cms_helper.o
##           ralink.o ralink_wps.o ralink_logger.o \
##           ralink_ap_scan.o \
##           ralink_sta_list.o ralink_driver.o dhcp_lease.o \

all: $(app)
### install

${app}: $(app_objs)
	$(CC) $(LDFLAGS) $^ -o $(app) $(LDLIBS)


install:
	cp $(app) $(ROOTFS_DIR)/bin
	$(STRIP) -R .comment -R .note $(ROOTFS_DIR)/bin/$(app)
	ln -sf wifi $(ROOTFS_DIR)/bin/wifi_wps
	ln -sf wifi $(ROOTFS_DIR)/bin/wifi_logger
	ln -sf wifi $(ROOTFS_DIR)/bin/wifi_ap_scan
	ln -sf wifi $(ROOTFS_DIR)/bin/wifi_sta_list
	# wifi.sh
	cp wifi.sh $(ROOTFS_DIR)/bin

clean:
	rm -rf *.o *.d ${app} *.orig

########################
target:
	make clean all
	cp ralink_conf /mnt/tftp/
	telnetcmd 'cd /mnt/jffs2/ && tftp -gr ralink_conf 192.168.15.2;'

host:
	CC=gcc HOST=1 make clean all

-include $(sources:.c=.d)

astyle:
	find ./ -type f -name '*.[ch]' | xargs -n 1 astyle --style=linux --indent=tab --pad-oper --unpad-paren
