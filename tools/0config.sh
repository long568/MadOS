#!/bin/sh
source user_config.sh

sed $"s/^export APP.*/export APP = $MADOS_WORKING_APP/" \
    tools/temp.main.mk > main.mk
sed $"s/\"\${workspaceFolder}\/app\/.*\"/\"\${workspaceFolder}\/app\/$MADOS_WORKING_APP\"/" \
    tools/temp.c_cpp_properties.json > .vscode/c_cpp_properties.json
echo "MadOS is ready... Enjoy yourself!"
