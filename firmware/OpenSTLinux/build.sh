#!/bin/bash

set -e
exit_error() {
    exit_error "$1"
    exit 1
}

if [ "$#" -ne 4 ]; then
    echo "Usage: $0 <TREETAR> <FLASHTAR> <SOURCETAR> <SDKTAR>"
    exit 1
fi

export DTBNAME="stm32mp157f-obdads_dk2-mx"
export WORKDIR="$PWD/build"
export TREETAR=$(realpath "$1")
export FLASHTAR=$(realpath "$2")
export SOURCETAR=$(realpath "$3")
export SDKTAR=$(realpath "$4")

# echo "dtb-name: $DTBNAME"
# echo "work-dir: $WORKDIR"
# echo "tree-tar: $TREETAR"
# echo "flash-tar: $FLASHTAR"
# echo "source-tar: $SOURCETAR"
# echo "sdk-tar: $SDKTAR"

mkdir -p $WORKDIR
cd $WORKDIR
echo "Created work directory: $WORKDIR"

mkdir -p $WORKDIR/tree
if [ ! -f "$TREETAR" ]; then
    exit_error "Could not find the device tree sources file"
else
    tar --strip-components=1 -xf $TREETAR -C $WORKDIR/tree/
    echo "Found and extracted device tree sources"
fi

if [ ! -f "$FLASHTAR" ]; then
    exit_error "Could not find the flash sources file"
    exit 1
else
    tar --strip-components=1 -xf $FLASHTAR -C $WORKDIR/
    echo "Found and extracted flash sources"
fi

if [ ! -f "$SOURCETAR" ]; then
    exit_error "Could not find the kernel and drivers sources file"
else
    tar --strip-components=1 -xf $SOURCETAR -C $WORKDIR/
    echo "Found and extracted kernel and drivers sources"
fi

mkdir -p $WORKDIR/sdk/install/
if [ ! -f "$SDKTAR" ]; then
    exit_error "Could not find the SDK sources file"
else
    tar --strip-components=1 -xf $SDKTAR -C $WORKDIR/sdk/install/
    echo "Found and extracted sdk sources"
fi

SDK_INSTALL_SCRIPT=$(find $WORKDIR/sdk/install -name "st-image-weston-openstlinux-weston-stm32mp1.rootfs-*.sh" -type f)
if [ -z "$SDK_INSTALL_SCRIPT" ]; then
    exit_error "Could not find the .sh file in $WORKDIR/sdk/install/"
else
    chmod +x "$SDK_INSTALL_SCRIPT"
    "$SDK_INSTALL_SCRIPT" -d $WORKDIR/sdk/
    source $WORKDIR/sdk/environment-setup
    echo "SDK installation and environment setup completed"
fi

# Unpack and patch kernel sources
echo "Unpacking and patching kernel sources..."
pushd $WORKDIR
mkdir -p kernel || error_exit "Failed to create kernel directory"
echo "Created kernel directory: $WORKDIR/kernel/"
echo "Extracting kernel sources..."
tar xf sources/arm-ostl-linux-gnueabi/linux-stm32mp-*/linux-*.tar.xz -C kernel || error_exit "Failed to extract kernel sources"
mv kernel/linux-* kernel/kernel-sources/ || error_exit "Failed to rename kernel sources"
echo "Extracted kernel sources"
echo "Patching kernel sources..."
pushd kernel/kernel-sources/
for p in $(ls -1 ../../sources/arm-ostl-linux-gnueabi/linux-stm32mp-*/*.patch); do
    patch -p1 < "$p" || error_exit "Failed to apply kernel patch: $p";
done
popd
echo "Patched kernel sources"
cp -r $WORKDIR/tree/kernel/* kernel/kernel-sources/arch/arm/boot/dts/st/ || error_exit "Failed to copy the device tree sources into the kernel sources"
echo "Copied the device tree sources into the kernel sources"
popd
echo "Unpacked and patched kernel sources"

# Regenerate the kernel device tree blobs
echo "Regenerating the kernel device tree blob..."
pushd $WORKDIR/kernel/kernel-sources
export OUTPUT_BUILD_DIR=$PWD/../build
mkdir -p ${OUTPUT_BUILD_DIR} || error_exit "Failed to create kernel device tree blobs output directory"
make ARCH=arm O="${OUTPUT_BUILD_DIR}" multi_v7_defconfig *fragment*.config || error_exit "Failed to build the default kernel configuration"
for f in `ls -1 ../../sources/arm-ostl-linux-gnueabi/linux-stm32mp-*/*fragment*.config`;
    do scripts/kconfig/merge_config.sh -m -r -O ${OUTPUT_BUILD_DIR} ${OUTPUT_BUILD_DIR}/.config $f || error_exit "Failed to merge kernel configuration fragment: $f";
done
(yes "" || true ) | make ARCH=arm oldconfig O="${OUTPUT_BUILD_DIR}" || error_exit "Failed to build kernel configuration"
# make ARCH=arm st/$DTBNAME.dtb LOADADDR=0xC2000040 O="${OUTPUT_BUILD_DIR}" || error_exit "Failed to build the kernel device tree blobs"
[ "${ARCH}" = "arm" ] && imgtarget="uImage" || imgtarget="Image.gz"
export IMAGE_KERNEL=${imgtarget}
make ${IMAGE_KERNEL} st/$DTBNAME.dtb vmlinux dtbs LOADADDR=0xC2000040 O="${OUTPUT_BUILD_DIR}" || error_exit "Failed to build kernel device tree blob"
make modules O="${OUTPUT_BUILD_DIR}" || error_exit "Failed to build kernel modules"
make INSTALL_MOD_PATH="${OUTPUT_BUILD_DIR}/install_artifact" modules_install O="${OUTPUT_BUILD_DIR}" || error_exit "Failed to build kernel modules"
mkdir -p ${OUTPUT_BUILD_DIR}/install_artifact/boot/
cp ${OUTPUT_BUILD_DIR}/arch/${ARCH}/boot/${IMAGE_KERNEL} ${OUTPUT_BUILD_DIR}/install_artifact/boot/ || error_exit "Failed to copy kernel boot directory"
find ${OUTPUT_BUILD_DIR}/arch/${ARCH}/boot/dts/ -name 'st*.dtb' -exec cp '{}' ${OUTPUT_BUILD_DIR}/install_artifact/boot/ \;
popd
if [ -z "$WORKDIR/kernel/build/arch/arm/boot/dts/st/$DTBNAME.dtb" ]; then
    exit_error "Could not generate the device tree blob"
fi
echo "Regenerated the kernel device tree blob"

# Unpack and patch TF-A sources
echo "Unpacking and patching TF-A sources..."
pushd $WORKDIR/sources/arm-ostl-linux-gnueabi/tf-a-stm32mp-v[0-9]*
mkdir -p tf-a-sources
echo "Extracting TF-A sources..."
tar xf tf-a-stm32mp-v[0-9]*.tar.* --one-top-level=tf-a-sources --strip-components=1 || error_exit "Failed to extract TF-A sources"
echo "Extracted TF-A sources"
echo "Patching TF-A sources..."
pushd tf-a-sources
for p in `ls -1 ../*.patch`; do
    patch -p1 < $p || error_exit "Failed to apply TF-A patch: $p";
done 
popd
echo "Patched TF-A sources"
cp -r $WORKDIR/tree/tf-a/* $WORKDIR/sources/arm-ostl-linux-gnueabi/tf-a-stm32mp-v[0-9]*/tf-a-sources/fdts/ || error_exit "Failed to copy the device tree sources into the TF-A sources"
echo "Copied the device tree sources into the TF-A sources"
popd
echo "Unpacked and patched TF-A sources"

# Regenerate the TF-A device tree blobs
echo "Regenerating the TF-A device tree blobs..."
pushd $WORKDIR/sources/arm-ostl-linux-gnueabi/tf-a-stm32mp-v[0-9]*/tf-a-sources
export FIP_DEPLOYDIR_ROOT=$PWD/../../FIP_artifacts
make -f ../Makefile.sdk TF_A_DEVICETREE=$DTBNAME TF_A_CONFIG="optee-sdcard optee-programmer-usb" DEPLOYDIR=$FIP_DEPLOYDIR_ROOT/arm-trusted-firmware stm32 || error_exit "Failed to build the TF-A device tree blobs"
popd
echo "Regenerated the TF-A device tree blobs"

# Unpack and patch OP-TEE sources
echo "Unpacking and patching OP-TEE sources..."
pushd $WORKDIR/sources/arm-ostl-linux-gnueabi/optee-os-stm32mp-[0-9]*
echo "Extracting OP-TEE sources..."
tar xf optee-os-stm32mp-[0-9]*.tar.* --one-top-level=optee-os-sources --strip-components=1 || error_exit "Failed to extract OP-TEE sources"
pushd optee-os-sources
tar xfz ../fonts.tar.gz
echo "Extracted OP-TEE sources"
echo "Patching OP-TEE sources..."
for p in `ls -1 ../*.patch`; do
    patch -p1 < $p || error_exit "Failed to apply OP-TEE patch: $p";
done
echo "Patched OP-TEE sources"
popd
cp -r $WORKDIR/tree/optee-os/* $WORKDIR/sources/arm-ostl-linux-gnueabi/optee-os-stm32mp-*/optee-os-sources/core/arch/arm/dts/ || error_exit "Failed to copy the device tree sources into the OP-TEE sources"
echo "Copied the device tree sources into the OP-TEE sources"
popd
echo "Unpacked and patched OP-TEE sources"

# Regenerate the OP-TEE device tree blobs
echo "Regenerating the OP-TEE device tree blobs..."
pushd $WORKDIR/sources/arm-ostl-linux-gnueabi/optee-os-stm32mp-[0-9]*/optee-os-sources
export FIP_DEPLOYDIR_ROOT=$PWD/../../FIP_artifacts
make -f ../Makefile.sdk OPTEE_CONFIG="optee" CFG_EMBED_DTB_SOURCE_FILE=$DTBNAME  CFG_DRAM_SIZE=0x20000000  DEPLOYDIR=$WORKDIR/sources/arm-ostl-linux-gnueabi/FIP_artifacts/optee optee || error_exit "Failed to build the OP-TEE device tree blobs"
popd
echo "Regenerated the OP-TEE device tree blobs"

# Unpack and patch U-Boot sources
echo "Unpacking and patching U-Boot sources..."
pushd $WORKDIR/sources/arm-ostl-linux-gnueabi/u-boot-stm32mp-v[0-9]*
echo "Extracting U-Boot sources..."
tar xf u-boot-stm32mp-v[0-9]*.tar.* --one-top-level=u-boot-sources --strip-components=1 || error_exit "Failed to extract U-Boot sources"
echo "Extracted U-Boot sources"
pushd u-boot-sources
echo "Patching U-Boot sources..."
for p in `ls -1 ../*.patch`; do
    patch -p1 < $p || error_exit "Failed to apply U-Boot patch: $p";
done
echo "Patched U-Boot sources"
popd
cp -r $WORKDIR/tree/u-boot/* $WORKDIR/sources/arm-ostl-linux-gnueabi/u-boot-stm32mp-v[0-9]*/u-boot-sources/arch/arm/dts/ || error_exit "Failed to copy the device tree sources into the U-Boot sources"
echo "Copied the device tree sources into the U-Boot sources"
popd
echo "Unpacked and patched U-Boot sources"

# Regenerate the U-Boot device tree blobs
echo "Regenerating the U-Boot device tree blobs..."
pushd $WORKDIR/sources/arm-ostl-linux-gnueabi/u-boot-stm32mp-v[0-9]*/u-boot-sources
make -f ../Makefile.sdk  UBOOT_DEFCONFIG=stm32mp15_defconfig UBOOT_BINARY=u-boot.dtb FIP_CONFIG="optee-sdcard" DEVICE_TREE=$DTBNAME all || error_exit "Failed to build the U-Boot device tree blobs"
popd
echo "Regenerated the U-Boot device tree blobs"

# Unpack and regenerate GC-Nano driver
echo "Unpacking GC-Nano driver..."
pushd $WORKDIR/sources/arm-ostl-linux-gnueabi/gcnano-driver-stm32mp-*
echo "Extracting GC-Nano sources..."
tar xf gcnano-driver-stm32mp-*.tar.* --one-top-level=gcnano-driver-sources --strip-components=1 || error_exit "Failed to extract GC-Nano driver sources"
echo "Extracted GC-Nano sources"
echo "Regenerating the GC-Nano driver..."
pushd gcnano-driver-sources
make M="${PWD}" AQROOT="${PWD}" modules -C ${OUTPUT_BUILD_DIR} || error_exit "Failed to build GC-Nano driver modules"
make M="${PWD}" AQROOT="${PWD}" INSTALL_MOD_PATH=${OUTPUT_BUILD_DIR}/install_artifact modules_install -C ${OUTPUT_BUILD_DIR} || error_exit "Failed to build GC-Nano driver modules"
echo "Regenerated the GC-Nano driver"
popd
popd
echo "Unpacked and regenerated GC-Nano driver"

mkdir -p $WORKDIR/outputs
cp -r $WORKDIR/kernel/build/install_artifact/boot/uImage $WORKDIR/outputs/
cp -r $WORKDIR/kernel/build/install_artifact/boot/$DTBNAME.dtb $WORKDIR/outputs/
cp -r $WORKDIR/kernel/build/install_artifact/lib $WORKDIR/outputs/
cp -r $WORKDIR/sources/arm-ostl-linux-gnueabi/FIP_artifacts/arm-trusted-firmware/tf-a-$DTBNAME-optee-sdcard.stm32 $WORKDIR/outputs/
cp -r $WORKDIR/sources/arm-ostl-linux-gnueabi/FIP_artifacts/fip/fip-$DTBNAME-optee-sdcard.bin $WORKDIR/outputs/

# ON THE PC:
# scp uImage root@192.168.7.1:/boot
# scp stm32mp157d-obdads_dk1-mx.dtb root@192.168.7.1:/boot
# rm -rf $WORKDIR/outputs/lib/modules/*/build
# scp $WORKDIR/outputs/lib/modules/ root@192.168.7.1:/lib/modules
# ON THE BOARD:
# cd /lib/modules
# depmod -a
# sync
# cd /boot/mmc0_extlinux
# add below to extlinux.conf and stm32mp157d-dk1_extlinux.conf
# LABEL stm32mp157d-obdads_dk1-mx
#     KERNEL /uImage
#     FDT /stm32mp157d-obdads_dk1-mx.dtb
#     APPEND root=PARTUUID=e91c4e10-16e6-4c0e-bd0e-77becf4a3582 rootwait rw console=${console},${baudrate}
# sync
# reboot

# Connect SD card to a reader and connect to PC
# list the partitions on the SD card using
# ls -l /dev/disk/by-partlabel
# flash tf-a source to fsbl1 and fsbl2
# sudo dd if=tf-a-stm32mp157d-obdads_dk1-mx-optee-sdcard.stm32 of=/dev/sdb1 bs=1M conv=fdatasync status=progress
# sudo dd if=tf-a-stm32mp157d-obdads_dk1-mx-optee-sdcard.stm32 of=/dev/sdb2 bs=1M conv=fdatasync status=progress
# flash fip source to fip-a
# sudo dd if=fip-stm32mp157d-obdads_dk1-mx-optee-sdcard.bin of=/dev/sdb5 bs=1M conv=fdatasync status=progress

# To start firmware
# cd /usr/local/projects/OBDADS_DK1_CM4
# ./fw_cortex_m4.sh start
# Top stop firmware
# ./fw_cortex_m4.sh stop
# Check firmware status
# cat /sys/class/remoteproc/remoteproc0/state
