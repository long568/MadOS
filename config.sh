#!/bin/bash

# export MADOS_WORKING_APP=test_kernel
export MADOS_WORKING_APP=test_module
# export MADOS_WORKING_APP=lesson000
# export MADOS_WORKING_APP=LoArm
# export MADOS_WORKING_APP=LoIoT
# export MADOS_WORKING_APP=LoNode

cp Makefile tmp.mk
sed $"s/^export APP.*/export APP = $MADOS_WORKING_APP/" tmp.mk > Makefile
rm -f tmp.mk
cp .vscode/c_cpp_properties.json tmp.json
sed $"s/\"\${workspaceFolder}\/app\/.*\"/\"\${workspaceFolder}\/app\/$MADOS_WORKING_APP\"/" \
    tmp.json > .vscode/c_cpp_properties.json
rm -f tmp.json

echo "MadOS is ready."
echo "Enjoy yourself!"
