# OBD-based Auto Dynamics System
> This repository contains resources, documentation, and project files for
ECE492 OBDADS1 capstone project.

A vehicle monitoring system for rally racing vehicles utilizing On-Board
Diagnostics (OBD-II) data and machine learning to aid in monitoring
vehicle performance and health.

## Setup
1. Boot up
    - Device tree is automatically selected from `/boot/mmc0_extlinux/extlinux.conf`
2. Turn on access point
    - Used for Grafana server
3. Start IPC
    - Start the firmware on the MCU
    - Firmware is found at `/usr/local/projects/OBDADS_DK2_CM4/lib/firmware/OBDADS_DK2_CM4.elf`
    - Firmware control script is found at `/usr/local/projects/OBDADS_DK2_CM4/fw_cortex_m4.sh`
        - This script is used to start and stop the firmware
        - Alternatively:
            - The firmware can be loaded into RemoteProc using `echo /usr/local/projects/OBDADS_DK2_CM4/lib/firmware/OBDADS_DK2_CM4.elf > /sys/class/remoteproc/remoteproc0/firmware`
            - And started using `echo start > /sys/class/remoteproc/remoteproc0/state`
            - And stopped using `echo stop > /sys/class/remoteproc/remoteproc0/state`
    - Status of the firmware is checked with `cat /sys/class/remoteproc/remoteproc0/state`
    - Uses `/dev/ttyRPMSG0` which is only created after the MCU firmware is started
4. Start ML training
5. Start Grafana

## Deployment
There are a few things that needs to happen when deploying our OBDADS for the first time. This assumes the usage of a factory development STM32MP157F-DK2 board, meaning the OpenSTLinux operating system has not been modified to reflect the hardware changes required by the firmware.

### Rebuild OpenSTLinux
OpenSTLinux needs to be rebuilt to create a new image to support the device tree changes required by the firmware running on the MCU. There are two approaches:
1. Manual build using script
    - Obtain the Device Tree Source files from STM32CubeIDE or STM32CubeMX
    - Download the STM32MP1 SDK sources (the one labeled x86) [Link](https://www.st.com/en/embedded-software/stm32mp1dev.html#get-software)
    - Download the STM32MP1 flash sources [Link](https://www.st.com/en/embedded-software/stm32mp1starter.html)
    - Download the STM32MP1 kernel sources (the one labeled SRC) [Link](https://www.st.com/en/embedded-software/stm32mp1dev.html#get-software)
    - Update the `DTBNAME` in the script if necessary
    - Run the script `./build.sh tree.tar flash-sources.tar kernel-sources.tar sdk-sources.tar`
    - Wait long time and pray for no errors `:)`

2. Yocto Project Bitbake (WORK IN PROGRESS)

### Flash the SD Card
Now that the OpenSTLinux operating system was rebuilt, we need to populate the SD card. We also need to update some files to make sure our device tree is automatically selected on boot up.

On host PC (Worked using Ubuntu 18.04 LTS on WSL2):
- Connect SD card to a reader and connect to the host PC
- list the partitions on the SD card using `ls -l /dev/disk/by-partlabel`
- Flash the TF-A source to `fsbl1` and `fsbl2` (First stage boot loader), make sure to change the `DTB_FILE_NAME` and `SD_CARD_DEV`:
    - `sudo dd if=build/outputs/DTB_FILE_NAME-optee-sdcard.stm32 of=/dev/SD_CARD_DEV_fsbl1 bs=1M conv=fdatasync status=progress`
    - `sudo dd if=build/outputs/DTB_FILE_NAME-optee-sdcard.stm32 of=/dev/SD_CARD_DEV_fsbl2 bs=1M conv=fdatasync status=progress`
- Flash the FIP sources to `fip-a`
    - `sudo dd if=build/outputs/fip-DTB_FILE_NAME-mx-optee-sdcard.bin of=/dev/SD_CARD_DEV_fip-a bs=1M conv=fdatasync status=progress`
- Unmount the SD card then put it in the MPU and boot it up
- Transfer the `build/outputs/DTB_FILE_NAME_uImage` file to `root@192.168.7.1:/boot`
- Transfer the `build/outputs/*.dtb` file to `root@192.168.7.1:/boot`
- Transfer the `build/outputs/lib/modules` folder to `root@192.168.7.1:/lib/modules`

On the MPU:
- Sync the new modules `cd /lib/modules` then `depmod -a` then `sync`
- Now we need to edit the `/boot/mmc0_extlinux/extlinux.conf` file to auto select our device tree on boot up:
    - `cd /boot/mmc0_extlinux`
    - Use `vi extlinx.conf` and `vi stm32mp157f-dk2_extlinx.conf` to add the following, make sure to change the `CHANGE_TO_DEVICE_TREE_NAME` with the name of the `build/outputs/*.dtb`:
        ```
        LABEL CHANGE_TO_DEVICE_TREE_NAME
        KERNEL /CHANGE_TO_DEVICE_TREE_NAME_uImage
        FDT /CHANGE_TO_DEVICE_TREE_NAME.dtb
        APPEND root=PARTUUID=e91c4e10-16e6-4c0e-bd0e-77becf4a3582 rootwait rw console=${console},${baudrate}
        ```
    - `sync` then `reboot`

### Installing Applications
Now that the operating system is setup, the applications need to be transferred and installed.

On the host PC:
1. Transfer IPC application to `/usr/local/projects`
2. Transfer Grafana application to `/usr/local/projects`
3. Transfer ML application to `/usr/local/projects`

On the MPU:
1. Install UV for Python, we are using `Python 3.12.9` ([See here](https://docs.astral.sh/uv/getting-started/installation/))
2. Install Grafana ([See here](https://grafana.com/grafana/download?platform=arm))
3. Navigate to `/usr/local/projects` and go into each application and `uv sync` to install dependencies for each application

### Configuring Services
Now that the applications have been installed, we need to use `systemctl` to automatically start our applications on bootup.

### Configuring Access Point
We also need to configure the access point for the first time.

