#!/bin/bash
echo "V4L drivers archive..."
make cleanall
make dir DIR=./media
cd linux
make todaytar DIR=.
mv -f linux-media-LATEST.tar.* ..

echo "V4L firmwares archive..."
cd ../v4l/firmware
tar cfj dvb-firmwares.tar.bz2 *.fw
mv -f dvb-firmwares.tar.bz2 ../..
