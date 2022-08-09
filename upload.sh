#!/bin/bash
BINARY="build/klik.uf2"
MOUNT_POINT="/Volumes/RPI-RP2"

if [ -d $MOUNT_POINT ]
then
    echo "Copying binary file"
    cp $BINARY $MOUNT_POINT/
    echo "Done"
else
    echo "Mount point does not exist"
fi