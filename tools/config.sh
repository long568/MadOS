#!/bin/bash
source $1/app_switcher.sh

sed $"s/^export APP.*/export APP = $MADOS_WORKING_APP/" \
    $1/tools/temp.main.mk > $1/main.mk

sed $"s/\"\${workspaceFolder}\/app\/.*\"/\"\${workspaceFolder}\/app\/$MADOS_WORKING_APP\"/" \
    $1/tools/temp.c_cpp_properties.json > $1/.vscode/c_cpp_properties.json

echo "MadOS is ready... Enjoy yourself!"
