#!/bin/bash
set -e

WORKSPACE="$1"

MESON_OPTIONS_TXT="$WORKSPACE/meson_options.txt"

CROSS_BUILD_TXT_TMP="$WORKSPACE/template/cross-build.txt"
CROSS_BUILD_TXT="$WORKSPACE/cross-build.txt"

# 1. 读取meson_options.txt，找出活跃的mcu
MCU=$(grep -E "^option\('mcu'" $MESON_OPTIONS_TXT | sed -n "s/.*value: '\(.*\)',.*/\1/p")

if [ -z "$MCU" ]; then
  echo "Error: MCU not found in meson_options.txt"
  exit 1
fi

echo "Detected MCU: $MCU"

# 2. 根据mcu名称推断对应架构
ARCH=""

if [[ "$MCU" =~ ^stm32g0 ]]; then
  ARCH="cortex-m0plus"
elif [[ "$MCU" =~ ^stm32f1 ]]; then
  ARCH="cortex-m3"
elif [[ "$MCU" =~ ^stm32f4 ]]; then
  ARCH="cortex-m4"
else
  echo "Error: Unknown MCU type: $MCU"
  exit 1
fi

echo "Resolved Architecture: $ARCH"

# 3. 生成新的pkg_config_path
PKG_CONFIG_PATH="$VCPKG_PKGS/arm-$ARCH-mados/lib/pkgconfig"

echo "Using pkg-config path: $PKG_CONFIG_PATH"

# 4. 替换cross-build.txt中的pkg_config_path
# (这里假设 cross-build.txt 里有原始的一行 pkg_config_path = [ '...' ])
sed "s|pkg_config_path = \[.*\]|pkg_config_path = [ '$PKG_CONFIG_PATH' ]|" $CROSS_BUILD_TXT_TMP > $CROSS_BUILD_TXT

echo "cross-build.txt updated successfully."
