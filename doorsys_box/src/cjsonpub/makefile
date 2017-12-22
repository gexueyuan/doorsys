NETCARD_HOME=/home/openwrt/pc02/openwrt/liuyuyi/doorsys/doorsys_box
CROSS=mips-openwrt-linux-
INC     = -I$(NETCARD_HOME)/include  -I/usr/local/include -I/home/openwrt/pc02/openwrt/build_dir/target-mips_34kc_uClibc-0.9.33.2/OpenWrt-SDK-ar71xx-generic_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-i686/staging_dir/target-mips_34kc_uClibc-0.9.33.2/usr/include
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
	@echo $(SRC_OBJ)
	@echo "================================="

.PHONY:Obj
Obj: $(SRC_OBJ)
$(SRC_OBJ): %.o :%.c
	$(CC) $(CFLAGS) $(INC) $< -o $@

.PHONY:LibSo
LibSo: libjsonpub.so 
libjsonpub.so: %.so :%.o
	$(CC) -fPIC -shared $(INC) libjsonpub.o cJSON.o -o $@ -L$(LIB)
	rm -f *$<
	mv $@ $(LIB)

libjsonbox.so: %.so :%.o
	$(CC) -fPIC -shared $(INC) libjsonbox.o cJSON.o -o $@ -L$(LIB)
	rm -f $< cJSON.o
	mv $@ $(LIB)


.PHONY:clean
clean:
	-rm -f $(SRC_OBJ)
	rm -f cJSON.o
	-rm -f *.so
