CROSS=mips-openwrt-linux-
NETCARD_HOME=/home/openwrt/pc02/openwrt/liuyuyi/doorsys/doorsys_box

INC     = -I$(NETCARD_HOME)/include -I../idreadpub -I../../../include -I/usr/local/include -I./  -I../tdrcps/src/lua/ -I/home/openwrt/pc02/openwrt/build_dir/target-mips_34kc_uClibc-0.9.33.2/OpenWrt-SDK-ar71xx-generic_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-i686/staging_dir/target-mips_34kc_uClibc-0.9.33.2/usr/include
CFLAGS  = -c -fPIC -DLINUX 
LIB     = $(NETCARD_HOME)/lib
CC      = $(CROSS)gcc
CXX      = $(CROSS)g++

SRC_OBJ := $(patsubst %.cpp,%.o,$(wildcard *.cpp))
SRC_SO  := $(patsubst %.cpp,%.so,$(wildcard *.cpp))

.PHONY:all
all: list Obj LibSo

.PHONY:list
list:
	@echo "=========== List Obj ============"
	@echo $(SRC_OBJ)
	@echo "================================="

.PHONY:Obj
Obj: $(SRC_OBJ)
$(SRC_OBJ): %.o :%.cpp
	$(CXX) $(CFLAGS) $(INC) $< -o $@

.PHONY:LibSo
LibSo: libgfreadpub.so
libgfreadpub.so: %.so :%.o
	$(CXX) -fPIC -shared $(INC) -L$(LIB) -lidread -lzmqtools -lzmq -lanalysisconf $< -o $@
	rm -f $<
	mv $@ $(LIB)

.PHONY:clean
clean:
	-rm -f $(SRC_OBJ)
	-rm -f *.so
