#!/bin/sh

# for now, /dev/ffs-diag must exists before starting qc-diag-router
# maybe fixed later by depending on udev

# Enabling the ffs does not offer diag over usb.
# The ffs must be also part of a configuration
# This can be done by usb-moded.
# Or see README.md for a full example

if [ -z "$G1" ]; then
	G1="/sys/kernel/config/usb_gadget/g1"
fi

# This might fail if not all required kernel modules are present
mkdir -p "$G1"
mkdir "$G1/functions/ffs.diag"
mkdir /dev/ffs-diag
mount -t functionfs diag /dev/ffs-diag

exit 0
