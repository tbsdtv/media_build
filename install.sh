#!/bin/bash

# Enable some staging drivers
make stagingconfig

# Disable RC/IR support
sed -i -r 's/(^CONFIG.*_RC.*=)./\1n/g' v4l/.config
sed -i -r 's/(^CONFIG.*_IR.*=)./\1n/g' v4l/.config

echo "V4L drivers building..."
make -j$(nproc)

echo "V4L drivers installing..."
sudo rm -r -f /lib/modules/$(uname -r)/updates/extra

sudo make install

echo "V4L drivers installation done"
echo "You need to reboot..."
