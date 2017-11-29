#!/bin/bash

# Enable some staging drivers
make stagingconfig

echo "V4L drivers building..."
make -j5

echo "V4L drivers installing..."
sudo rm -r -f /lib/modules/$(uname -r)/kernel/drivers/media
sudo rm -r -f /lib/modules/$(uname -r)/kernel/drivers/staging/media
sudo rm -r -f /lib/modules/$(uname -r)/extra


sudo make install

echo "V4L drivers installation done"
echo "You need to reboot..."
