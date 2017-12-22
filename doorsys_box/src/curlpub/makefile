CROSS=mips-openwrt-linux-
NETCARD_HOME=/home/openwrt/pc02/openwrt/liuyuyi/doorsys/doorsys_box

INC     = -I$(NETCARD_HOME)/include -I/usr/local/include -I/home/openwrt/pc02/openwrt/build_dir/target-mips_34kc_uClibc-0.9.33.2/OpenWrt-SDK-ar71xx-generic_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-i686/staging_dir/target-mips_34kc_uClibc-0.9.33.2/usr/include
CFLAGS  = -c -g -fPIC -DLINUX
LIB     = $(NETCARD_HOME)/lib
CC      = $(CROSS)gcc

SRC_OBJ := $(patsubst %.c,%.o,$(wildcard *.c))
SRC_SO  := $(patsubst %.c,%.so,$(wildcard *.c))

.PHONY:all
all: list Obj LibSo

.PHONY:list
list:
	@echo "=========== List Obj ============"
	@echo $(SRC_SO)
	@echo "================================="

.PHONY:Obj
Obj: $(SRC_OBJ)
$(SRC_OBJ): %.o :%.c
	$(CC) $(CFLAGS) $(INC) $< -o $@

.PHONY:LibSo
LibSo: libcurlpub.so
libcurlpub.so: %.so :$(SRC_OBJ)
#	$(CC) -fPIC -shared $(INC) $< -o $@ -L$(LIB) -lcurl  -L/usr/local/ssl/lib -lssl -lcrypto
#	$(CC) -fPIC -shared $(INC) $< -o $@ -L$(LIB) $(NETCARD_HOME)/ulib/libcurl.a -L/usr/local/ssl/lib -lssl -lcrypto
	$(CC) -fPIC -shared $(INC) $< -o $@ -L$(LIB) $(NETCARD_HOME)/ulib/libcurl.a -L/home/openwrt/pc02/openwrt/liuyuyi/doorsys/ssl/lib -lssl -lcrypto
#	$(CC) -fPIC -shared $(INC) $< -o $@ -L$(LIB) /home/gofun/ulib/libcurl.a /home/gofun/ulib/libssl.a /home/gofun/ulib/libcrypto.a
	rm -f $<
	mv $@ $(NETCARD_HOME)/lib

.PHONY:clean
clean:
	-rm -f $(SRC_OBJ)
	-rm -f *.so
