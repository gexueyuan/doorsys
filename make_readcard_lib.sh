#! /bin/sh

#cjsonpub
cd /home/openwrt/pc02/openwrt/liuyuyi/doorsys/doorsys_box/src/cjsonpub
make clean
make

#client
cd ../client
make clean
make

#curlpub
cd ../curlpub
make clean
make

#tools
cd ../tools
make clean
make

#gflogsvr
cd ../gflogsvr
make clean
make

#idreadpub
cd ../idreadpub
make clean
make

#gfreadpub
cd ../gfreadpub
make clean
make

#gfcosupdsvr
cd ../gfcosupdsvr
make clean
make

#public
cd ../public
make clean
make


