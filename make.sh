#!/bin/bash

set -e

if [ $# -eq 0 ]; then
    echo "No arguments supplied"
    exit 1
fi

WORK_DIR="/home/computer/workspace/my_openipc/hi3520d_uboot"

OPENIPC_DIR="${WORK_DIR}/../chaos_calmer"
LOADER_SRC_DIR="${OPENIPC_DIR}/build_dir/target-arm_cortex-a9_uClibc-0.9.33.2_eabi/linux-hi35xx_20dv200/u-boot-20dv200/u-boot-2010.06_openipc"
REGISTERS_FILE="${OPENIPC_DIR}/package/boot/uboot-hi35xx/files/uboot_tools/reg_info_hi3520d_duo.bin"

if [ $1 = "h" ] ; then
	echo "SDK build"
	# HiSilicon (original)
	BUILD_DIR="${WORK_DIR}/sdk_build"
	TOOLCHAIN=/opt/hisi-linux-nptl/arm-hisiv100-linux/target/bin
	CROSSRULE=arm-hisiv100nptl-linux-
elif [ $1 = "w" ] ; then
	echo "OpenWrt build"
	# OpenWrt
	BUILD_DIR="${WORK_DIR}/wrt_build"
	STAGINDIR="${OPENIPC_DIR}/staging_dir/toolchain-arm_cortex-a9_gcc-4.8-linaro_uClibc-0.9.33.2_eabi"
	STAGING_DIR="${STAGINDIR}"
	TOOLCHAIN="${STAGINDIR}/bin"
	CROSSRULE=arm-openwrt-linux-uclibcgnueabi-
elif [ $1 = "b" ] ; then
	echo "Buildroot build"
	# Buildroot
	BUILD_DIR="${WORK_DIR}/buildroot_build"
	TOOLCHAIN="${WORK_DIR}/20210108_hi35xx-buildroot/output/host/bin"
	CROSSRULE=arm-buildroot-linux-uclibcgnueabi-
else
	echo "Wrong argument"
	exit 1
fi

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

make_run() {
    STAGING_DIR="${STAGINDIR}" PATH=$PATH:${TOOLCHAIN} make O="${BUILD_DIR}" -C "${LOADER_SRC_DIR}" ARCH=arm CROSS_COMPILE=${CROSSRULE} $@
}

#${TOOLCHAIN}/${CROSSRULE}gcc -v
#exit 0

make_run distclean
make_run hi3520d_config
make_run -j$((`nproc`+1))   #>.build_uboot.log 2>&1

dd if=./u-boot.bin of=./fb1 bs=1 count=64
dd if=${REGISTERS_FILE} of=./fb2 bs=4800 conv=sync
dd if=./u-boot.bin of=./fb3 bs=1 skip=4864
cat fb1 fb2 fb3 > ./u-boot_merged.bin
rm -f fb1 fb2 fb3
