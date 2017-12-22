CROSS=mips-openwrt-linux-
NETCARD_HOME=/home/openwrt/pc02/openwrt/liuyuyi/doorsys/doorsys_box
CC       = $(CROSS)gcc
CXX	 = $(CROSS)g++
CFLAGS   = -Wall -fPIC
#CFLAGS   = -Wall -g

LIB     = $(NETCARD_HOME)/lib
LIBNAME = ./libidread.so

OBJ	  := $(patsubst %.cpp,%.o, $(wildcard ../../../include/*.cpp ../../../include/alg/*.cpp))

INCLUDE  =  -I../../../include/ -I ../../../include/alg/  -I/usr/local/include -I/usr/local/ssl/include

all: unix

clean:
	rm -f ../../../include/*.o ../../../include/alg/*.o $(OBJ)  $(LIBNAME)

unix: $(OBJ)
	$(CXX) -s -shared $(LDFLAGS) $^ -L$(LIB) -o $(LIBNAME)
	mv $(LIBNAME) $(LIB)

$(OBJ) : %.o : %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@ $(INCLUDE) 
