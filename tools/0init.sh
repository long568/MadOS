#!/bin/bash

source config.sh

sed $"s/^export APP.*/export APP = $MADOS_WORKING_APP/" \
    tools/temp.Makefile > Makefile
sed $"s/\"\${workspaceFolder}\/app\/.*\"/\"\${workspaceFolder}\/app\/$MADOS_WORKING_APP\"/" \
    tools/temp.c_cpp_properties.json > .vscode/c_cpp_properties.json
echo "MadOS is ready... Enjoy yourself!"
