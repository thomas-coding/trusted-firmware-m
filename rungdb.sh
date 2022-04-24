#!/bin/bash

# shell folder
shell_folder=$(cd "$(dirname "$0")" || exit;pwd)

workspace_dir=${shell_folder}/../../..

# toolchain
toolchains_try_dir1=~/.toolchain;
toolchains_try_dir2=${workspace_dir}/.toolchains;
toolchian=gcc-arm-none-eabi-10.3-2021.10

tfm_home=${shell_folder}

# get toolchain and export
function get_and_export_toolchain()
{
    echo "get_toolchain"
    if [[ -d ${toolchains_try_dir1} ]]; then
        if [[ -d ${toolchains_try_dir1}/${toolchian}/bin ]]; then
            echo "find toolchain from ${toolchains_try_dir1}"
            export PATH="${toolchains_try_dir1}/${toolchian}/bin:$PATH"
            return
        fi
    fi
    
    if [[ -d ${toolchains_try_dir2} ]]; then
        if [[ -d ${toolchains_try_dir2}/${toolchian}/bin ]]; then
            echo "find toolchain from ${toolchains_try_dir2}"
            export PATH="${toolchains_try_dir2}/${toolchian}/bin:$PATH"
            return
        fi
    fi
    echo "cann't find toolchain, please run root script './build.sh --toolchains' ,exit ...."
    exit
}

get_and_export_toolchain

# gdb
arm-none-eabi-gdb \
-ex 'target remote localhost:1234' \
-ex "add-symbol-file ${tfm_home}/build/bin/tfm_s.elf" \
-ex "add-symbol-file ${tfm_home}/../FreeRTOS/FreeRTOS/Demo/THOMAS_M33_QEMU/build/RTOSDemo.axf" \
-q 
