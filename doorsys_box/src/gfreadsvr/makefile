CROSS=mips-openwrt-linux-
NETCARD_HOME=/home/openwrt/pc02/openwrt/liuyuyi/doorsys/doorsys_box

INC=-I$(NETCARD_HOME)/include -I/usr/local/include -I/home/openwrt/pc02/openwrt/build_dir/target-mips_34kc_uClibc-0.9.33.2/OpenWrt-SDK-ar71xx-generic_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-i686/staging_dir/target-mips_34kc_uClibc-0.9.33.2/usr/include
CFLAGS= -c -g -DLINUX 
LIB=$(NETCARD_HOME)/lib
LIBFLAG=-L$(LIB) -lgfreadbusi -lgfreadpub -ljsonpub -lidread -lgflog -ludev -ltlv -ltcp -lpthread -lrt -lm -llog -lnetutil -lanalysisconf -lzmqtools -lzmq -ldl -lcurlpub -lsysbase -L/home/openwrt/pc02/openwrt/liuyuyi/doorsys/ssl/lib -lssl -lcrypto
BIN=$(NETCARD_HOME)/bin
CC=$(CROSS)gcc

SRC_OBJ := $(patsubst %.c,%.o,$(wildcard *.c))
SRC_BIN  := $(patsubst %.c,%,$(wildcard *.c))

.PHONY:all
all: list Obj bin

.PHONY:list
list:
	@echo "=========== List Obj ============"
	@echo $(SRC_OBJ)
	@echo "================================="

.PHONY:Obj
Obj: $(SRC_OBJ)
$(SRC_OBJ): %.o :%.c
	$(CC) $(CFLAGS) $(INC) $< -o $@

.PHONY:bin
bin:gfreadsvr
gfreadsvr: % :%.o
	$(CC) $(INC)  $< -o $@ ${LIBFLAG}
	rm -f $<
	mv $@ $(BIN)

.PHONY:clean
clean:
	-rm -f $(SRC_OBJ)
