#!/bin/sh
cd /home/gofun/src/idreadpub
make clean
make
cd /home/gofun/src/gfreadpub
make

cd /home/gofun/
tar -czvf test.tar.gz bin lib
mv test.tar.gz /nfsdir
