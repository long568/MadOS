#!/bin/bash

SSTACK=0x20000000
MCU=$MCU_PREFIX

if [ $MCU = 'gd32e23x' ]
then
	ISR_OFS=$[51 * 4]
fi

ESTACK=$[SSTACK + MCU_RAM_SIZE]
RAM0_LEN=$ISR_OFS
RAM_ORG=$[SSTACK + ISR_OFS]
RAM_LEN=$[MCU_RAM_SIZE - ISR_OFS]

echo -e "Generate ldscript ... \c"
mkdir -p $BUILD_DIR
printf "_estack = 0x%X;\n" $ESTACK > $BUILD_DIR/HiMadOS.ld
printf "MEMORY\n{\n" >> $BUILD_DIR/HiMadOS.ld
printf "RAM0 (xrw) : ORIGIN = 0x%X, LENGTH = 0x%X\n" $SSTACK $RAM0_LEN >> $BUILD_DIR/HiMadOS.ld
printf "RAM (xrw)  : ORIGIN = 0x%X, LENGTH = 0x%X\n" $RAM_ORG $RAM_LEN >> $BUILD_DIR/HiMadOS.ld
printf "FLASH (rx) : ORIGIN = 0x8000000,  LENGTH = %s\n" $MCU_FLS_SIZE >> $BUILD_DIR/HiMadOS.ld
printf "}\n\n" >> $BUILD_DIR/HiMadOS.ld
cat $ROOT/arch/$MCU_PREFIX/$MCU_PREFIX.ld >> $BUILD_DIR/HiMadOS.ld
echo 'Done'
