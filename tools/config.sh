#!/bin/bash
source $1/app_switcher.sh

sed "s/=[ ]*/=/" $1/app/$MADOS_WORKING_APP/CfgApp.mk | sed "s/[ ]*=/=/" | sed "s/[ ]*+=/+=/" > $1/app/$MADOS_WORKING_APP/_CfgApp.mk
source $1/app/$MADOS_WORKING_APP/_CfgApp.mk
rm -f $1/app/$MADOS_WORKING_APP/_CfgApp.mk

if [ -z $MCU_NAME ];then
export MCU_NAME=${MCU_PREFIX}_${MCU_SUFFIX}
fi
export MCU_NAME=$(echo $MCU_NAME | tr 'a-z' 'A-Z')

sed $"s/^export APP.*/export APP = $MADOS_WORKING_APP/" \
    $1/tools/temp.main.mk > $1/main.mk

cp $1/tools/temp.c_cpp_properties.json $1/.vscode/0.json
sed $"s/\"\${workspaceFolder}\/app\/.*\"/\"\${workspaceFolder}\/app\/$MADOS_WORKING_APP\"/" $1/.vscode/0.json > $1/.vscode/1.json
sed $"s/stm32f10x/$MCU_PREFIX/"  $1/.vscode/1.json > $1/.vscode/0.json
sed $"s/STM32F10X_CL/$MCU_NAME/" $1/.vscode/0.json > $1/.vscode/c_cpp_properties.json
rm -f $1/.vscode/0.json $1/.vscode/1.json

echo "MadOS is ready... Enjoy yourself!"
