#!/bin/bash

if [ "$#" -ne 5 ]; then
    echo "Usage: $0 <WORKDIR> <TREETAR> <FLASHTAR> <SOURCETAR> <SDKTAR>"
    exit 1
fi

WORKDIR=$(realpath "$1")
TREETAR=$(realpath "$2")
FLASHTAR=$(realpath "$3")
SOURCETAR=$(realpath "$4")
SDKTAR=$(realpath "$5")

cd $HOME
mkdir -p $WORKDIR
cd $WORKDIR
echo "Created work directory at $WORKDIR"

tar --strip-components=1 -xf $FLASHTAR -C $WORKDIR/
echo "Extracted $FLASHTAR"

tar --strip-components=1 -xf $SOURCETAR -C $WORKDIR/
echo "Extracted $SOURCETAR"

mkdir -p $WORKDIR/sdk/install/
tar --strip-components=2 -xf $SDKTAR -C $WORKDIR/sdk/install/
echo "Extracted $SDKTAR"

mkdir -p $WORKDIR/tree
tar --strip-components=1 -xf $TREETAR -C $WORKDIR/tree/
echo "Extracted $TREETAR"

SDK_INSTALL_SCRIPT=$(find $WORKDIR/sdk/install -name "st-image-weston-openstlinux-weston-stm32mp1.rootfs-*.sh" -type f)
if [ -z "$SDK_INSTALL_SCRIPT" ]; then
    echo "Error: Could not find the .sh file in $WORKDIR/sdk/install/"
    exit 1
fi

chmod +x "$SDK_INSTALL_SCRIPT"
"$SDK_INSTALL_SCRIPT" -d $WORKDIR/sdk/

source $WORKDIR/sdk/environment-setup
echo "OpenSTLinux SDK environment configured"



echo "Updating the kernel device tree"
pushd $WORKDIR
mkdir -p kernel
tar xf sources/arm-ostl-linux-gnueabi/linux-stm32mp-*/linux-*.tar.xz -C kernel
mv kernel/linux-* kernel/kernel-sources/
pushd kernel/kernel-sources/
for p in $(ls -1 ../../sources/arm-ostl-linux-gnueabi/linux-stm32mp-*/*.patch);
    do patch -p1 < $p;
done
popd
popd
echo "Unpacked kernel sources and applied patches"

pushd $WORKDIR
cp -r $WORKDIR/tree/kernel/* kernel/kernel-sources/arch/arm/boot/dts/st/
popd
echo "Copied Device Tree Source (DTS) into the Linux kernel source code"

echo "Regenerating the Linux kernel Device Tree Blob (DTB)"
DTB_FILE="stm32mp157d-obdads_dk1-mx"
pushd $WORKDIR/kernel/kernel-sources
export OUTPUT_BUILD_DIR=$PWD/../build
mkdir -p ${OUTPUT_BUILD_DIR}
make ARCH=arm O="${OUTPUT_BUILD_DIR}" multi_v7_defconfig *fragment*.config
for f in `ls -1 ../../sources/arm-ostl-linux-gnueabi/linux-stm32mp-*/*fragment*.config`;
    do scripts/kconfig/merge_config.sh -m -r -O ${OUTPUT_BUILD_DIR} ${OUTPUT_BUILD_DIR}/.config $f;
done
(yes "" || true ) | make ARCH=arm oldconfig O="${OUTPUT_BUILD_DIR}"
[ "${ARCH}" = "arm" ] && imgtarget="uImage" || imgtarget="Image.gz"
export IMAGE_KERNEL=${imgtarget}
make ${IMAGE_KERNEL} st/$DTB_FILE.dtb vmlinux dtbs LOADADDR=0xC2000040 O="${OUTPUT_BUILD_DIR}"
make modules O="${OUTPUT_BUILD_DIR}"
make INSTALL_MOD_PATH="${OUTPUT_BUILD_DIR}/install_artifact" modules_install O="${OUTPUT_BUILD_DIR}"
mkdir -p ${OUTPUT_BUILD_DIR}/install_artifact/boot/
cp ${OUTPUT_BUILD_DIR}/arch/${ARCH}/boot/${IMAGE_KERNEL} ${OUTPUT_BUILD_DIR}/install_artifact/boot/
find ${OUTPUT_BUILD_DIR}/arch/${ARCH}/boot/dts/ -name 'st*.dtb' -exec cp '{}' ${OUTPUT_BUILD_DIR}/install_artifact/boot/ \;
popd
if [ -z "$WORKDIR/kernel/build/arch/arm/boot/dts/st/$DTB_FILE.dtb" ]; then
    echo "Error: Could not generate the Device Tree Blob (DTB)"
    exit 1
fi
echo "Device Tree Blob (DTB) regenerated"



echo "Updating the TF-A device tree"
pushd $WORKDIR/sources/arm-ostl-linux-gnueabi/tf-a-stm32mp-v[0-9]*
mkdir -p tf-a-sources
tar xf tf-a-stm32mp-v[0-9]*.tar.* --one-top-level=tf-a-sources --strip-components=1
pushd tf-a-sources
for p in `ls -1 ../*.patch`; do patch -p1 < $p; done
popd
popd
echo "Unpacked TF-A sources and applied patches"

pushd $WORKDIR
cp -r $WORKDIR/tree/tf-a/* sources/arm-ostl-linux-gnueabi/tf-a-stm32mp-v[0-9]*/tf-a-sources/fdts/
popd
echo "Copied Device Tree Source (DTS) into the TF-A source code"

pushd $WORKDIR/sources/arm-ostl-linux-gnueabi/tf-a-stm32mp-v[0-9]*/tf-a-sources
export FIP_DEPLOYDIR_ROOT=$PWD/../../FIP_artifacts
make -f ../Makefile.sdk TF_A_DEVICETREE=$DTB_FILE TF_A_CONFIG="optee-sdcard optee-programmer-usb" DEPLOYDIR=$FIP_DEPLOYDIR_ROOT/arm-trusted-firmware stm32
popd
echo "TF-A regenerated"



echo "Updating the OP-TEE device tree"
pushd $WORKDIR/sources/arm-ostl-linux-gnueabi/optee-os-stm32mp-[0-9]*
tar xf optee-os-stm32mp-[0-9]*.tar.* --one-top-level=optee-os-sources --strip-components=1
pushd optee-os-sources
tar xfz ../fonts.tar.gz
for p in `ls -1 ../*.patch`; do patch -p1 < $p; done
popd
popd
echo "Unpacked OP-TEE sources and applied patches"

pushd $WORKDIR
cp -r $WORKDIR/tree/optee-os/* sources/arm-ostl-linux-gnueabi/optee-os-stm32mp-*/optee-os-sources/core/arch/arm/dts/
popd
echo "Copied Device Tree Source (DTS) into the OP-TEE source code"

pushd $WORKDIR/sources/arm-ostl-linux-gnueabi/optee-os-stm32mp-[0-9]*/optee-os-sources
export FIP_DEPLOYDIR_ROOT=$PWD/../../FIP_artifacts
make -f ../Makefile.sdk OPTEE_CONFIG="optee" CFG_EMBED_DTB_SOURCE_FILE=$DTB_FILE  CFG_DRAM_SIZE=0x20000000  DEPLOYDIR=$WORKDIR/sources/arm-ostl-linux-gnueabi/FIP_artifacts/optee optee
popd
echo "OP-TEE regenerated"



echo "Updating the U-Boot device tree"
pushd $WORKDIR/sources/arm-ostl-linux-gnueabi/u-boot-stm32mp-v[0-9]*
tar xf u-boot-stm32mp-v[0-9]*.tar.* --one-top-level=u-boot-sources --strip-components=1
pushd u-boot-sources
for p in `ls -1 ../*.patch`; do patch -p1 < $p; done
popd
popd
echo "Unpacked U-Boot sources and applied patches"

pushd $WORKDIR
cp -r tree/u-boot/* sources/arm-ostl-linux-gnueabi/u-boot-stm32mp-v[0-9]*/u-boot-sources/arch/arm/dts/
popd
echo "Copied Device Tree Source (DTS) into the U-Boot source code"

pushd $WORKDIR/sources/arm-ostl-linux-gnueabi/u-boot-stm32mp-v[0-9]*/u-boot-sources
make -f ../Makefile.sdk  UBOOT_DEFCONFIG=stm32mp15_defconfig UBOOT_BINARY=u-boot.dtb FIP_CONFIG="optee-sdcard" DEVICE_TREE=$DTB_FILE all
popd
echo "U-Boot regenerated"


pushd $WORKDIR/sources/arm-ostl-linux-gnueabi/gcnano-driver-stm32mp-*
tar xf gcnano-driver-stm32mp-*.tar.* --one-top-level=gcnano-driver-sources --strip-components=1
pushd gcnano-driver-sources
make M="${PWD}" AQROOT="${PWD}" modules -C ${OUTPUT_BUILD_DIR}
make M="${PWD}" AQROOT="${PWD}" INSTALL_MOD_PATH=${OUTPUT_BUILD_DIR}/install_artifact modules_install -C ${OUTPUT_BUILD_DIR}
popd
popd



mkdir -p $WORKDIR/outputs
cp -r $WORKDIR/kernel/build/install_artifact/boot/uImage $WORKDIR/outputs/
cp -r $WORKDIR/kernel/build/install_artifact/boot/$DTB_FILE.dtb $WORKDIR/outputs/
cp -r $WORKDIR/kernel/build/install_artifact/lib $WORKDIR/outputs/
cp -r $WORKDIR/sources/arm-ostl-linux-gnueabi/FIP_artifacts/arm-trusted-firmware/tf-a-$DTB_FILE-optee-sdcard.stm32 $WORKDIR/outputs/
cp -r $WORKDIR/sources/arm-ostl-linux-gnueabi/FIP_artifacts/fip/fip-$DTB_FILE-optee-sdcard.bin $WORKDIR/outputs/



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
