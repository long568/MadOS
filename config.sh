#!/bin/bash

export MADOS_WORKING_APP=test_kernel
# export MADOS_WORKING_APP=test_module
# export MADOS_WORKING_APP=lesson000
# export MADOS_WORKING_APP=LoArm
# export MADOS_WORKING_APP=LoIoT
# export MADOS_WORKING_APP=LoNode

sed $"s/^export APP.*/export APP = $MADOS_WORKING_APP/" Makefile.temp.mk > Makefile
sed $"s/\"\${workspaceFolder}\/app\/.*\"/\"\${workspaceFolder}\/app\/$MADOS_WORKING_APP\"/" \
    .vscode/c_cpp_properties.temp.json > .vscode/c_cpp_properties.json
echo "MadOS is ready... Enjoy yourself!"
