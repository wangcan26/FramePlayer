#!/bin/bash

script_path=$(cd `dirname $0`; pwd)

platform='unknown'
case $OSTYPE in 
    msys*) platform='windows';;
    darwin*) platform='darwin';;
esac
echo "current platform : ${platform}"

source $script_path/VERSION.sh
adb_command=adb
target_path=/storage/emulated/0/Android/data/com.demo/files
if [ "$platform" = "windows" ]; then 
    adb_command=$ANDROID_SDK_VERSION_WIN/platform-tools/adb
    target_path='//mnt\${target_path}'
fi

echo ${script_path}
$adb_command root
$adb_command push  ${script_path}/../assets/  '//mnt\sdcard\'
echo "push success"