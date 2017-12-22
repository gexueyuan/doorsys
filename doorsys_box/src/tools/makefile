CROSS=mips-openwrt-linux-
NETCARD_HOME=/home/openwrt/pc02/openwrt/liuyuyi/doorsys/doorsys_box
INC     = -I$(NETCARD_HOME)/include -I/home/openwrt/pc02/openwrt/build_dir/target-mips_34kc_uClibc-0.9.33.2/OpenWrt-SDK-ar71xx-generic_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-i686/staging_dir/target-mips_34kc_uClibc-0.9.33.2/usr/include
CFLAGS  = -c -g -fPIC -DLINUX
LIB     = $(NETCARD_HOME)/lib
LIBFLAG =-L$(LIB)
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
LibSo: liblog.so libtcp.so libtlv.so libanalysisconf.so libzmqtools.so libnetutil.so libbase64.so
liblog.so: %.so :%.o
	$(CC) -fPIC -shared $(INC) $< -o $@
	rm -f $<
	mv $@ $(LIB)

libtcp.so: %.so :%.o
	$(CC) -fPIC -shared $(INC) $< -o $@
	rm -f $<
	mv $@ $(LIB)

libtlv.so: %.so :%.o
	$(CC) -fPIC -shared $(INC) $< -o $@
	rm -f $<
	mv $@ $(LIB)

libanalysisconf.so: %.so :%.o
	$(CC) -fPIC -shared $(INC) $< -o $@
	rm -f $<
	mv $@ $(LIB)

libzmqtools.so: %.so :%.o
	$(CC) -fPIC -shared $(INC) $< -o $@ $(LIBFLAG) -lzmq
	rm -f $<
	mv $@ $(LIB)
	
libnetutil.so: %.so :%.o
	$(CC) -fPIC -shared $(INC) $< -o $@
	rm -f $<
	mv $@ $(LIB)

libbase64.so: %.so :%.o
	$(CC) -fPIC -shared $(INC) $< -o $@
	rm -f $<
	mv $@ $(LIB)

.PHONY:clean
clean:
	-rm -f $(SRC_OBJ)
	-rm -f *.so
