NETCARD_HOME=/home/openwrt/pc02/openwrt/liuyuyi/doorsys/doorsys_box
CROSS=mips-openwrt-linux-
#INC=-I$(NETCARD_HOME)/include -I/usr/local/include
INC=-I$(NETCARD_HOME)/include -I/usr/local/include -I/home/openwrt/pc02/openwrt/build_dir/target-mips_34kc_uClibc-0.9.33.2/OpenWrt-SDK-ar71xx-generic_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-i686/staging_dir/target-mips_34kc_uClibc-0.9.33.2/usr/include
CFLAGS= -c -g -DLINUX
LIB=$(NETCARD_HOME)/lib
LIBFLAG=-L$(LIB)
BIN=$(NETCARD_HOME)/bin
CC=$(CROSS)gcc

SRC_OBJ := $(patsubst %.c,%.o,$(wildcard *.c))
SRC_BIN  := $(patsubst %.c,%,$(wildcard *.c))

.PHONY:all
all: list Obj bin

.PHONY:list
list:
	@echo "=========== List Obj ============"
	@echo $(SRC_BIN)
	@echo "================================="

.PHONY:Obj
Obj: $(SRC_OBJ)
$(SRC_OBJ): %.o :%.c
	$(CC) $(CFLAGS) $(INC) $< -o $@

.PHONY:bin
bin:zmqclt_test apdu_test
zmqclt_test: % :%.o
	$(CC) $(INC)  $< -o $@ -L$(LIB) -lzmqtools -lanalysisconf -llog -ltlv -lnetutil -lzmq -ltcp -lcurlpub -ljsonpub -L/home/openwrt/pc02/openwrt/liuyuyi/doorsys/ssl/lib -lssl -lcrypto
	rm -f $<

apdu_test: % :%.o
	$(CC) $(INC)  $< -o $@ -L$(LIB) -lzmqtools -lanalysisconf -llog -ltlv -lnetutil -lzmq -ltcp -lidread -ludev -lpthread -lrt -lm -lgfreadpub -L/home/openwrt/pc02/openwrt/liuyuyi/doorsys/ssl/lib -lssl -lcrypto
	rm -f $< 


.PHONY:clean
clean:
	-rm -f $(SRC_OBJ)
