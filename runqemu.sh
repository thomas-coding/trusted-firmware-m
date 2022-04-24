#!/bin/bash

# shell folder
shell_folder=$(cd "$(dirname "$0")" || exit;pwd)
workspace_dir=${shell_folder}/../../..

qemu_version=qemu-6.0.0

# toolcqemuhain
qemu_try_dir1=${workspace_dir}/software/qemu
qemu_try_dir2=~/.toolchains/qemu

# get qemu and export
function get_and_export_qemu()
{
    echo "get qemu"
    if [[ -d ${qemu_try_dir1} ]]; then
        if [[ -d ${qemu_try_dir1}/${qemu_version}/build ]]; then
            echo "find qemu from ${qemu_try_dir1}"
            export PATH="${qemu_try_dir1}/${qemu_version}/build:$PATH"
            return
        fi
    fi

    if [[ -d ${qemu_try_dir2} ]]; then
        if [[ -d ${qemu_try_dir2}/${qemu_version}/build ]]; then
            echo "find qemu from ${qemu_try_dir2}"
            export PATH="${qemu_try_dir2}/${qemu_version}/build:$PATH"
            return
        fi
    fi
    
    echo "cann't find qemu, please run root script './build.sh --qemu' ,exit ...."
    exit
}

get_and_export_qemu

qemu_option=
if [[ $1  = "--gdb" ]]; then
    qemu_option+=" -s -S"
    echo "enable gdb, please run script './rungdb', and enter c "
else
    echo "not use gdb, just run"
fi

qemu_option+=" -machine thomas-m33"
qemu_option+=" -kernel ${shell_folder}/build/bin/tfm_s.elf"
qemu_option+=" -device loader,file=${shell_folder}/../FreeRTOS/FreeRTOS/Demo/THOMAS_M33_QEMU/build/RTOSDemo.bin,addr=0x10200000"
qemu_option+=" -serial stdio"
qemu_option+=" -d guest_errors"

# run qemu
gdb --args ../qemu/build/arm-softmmu/qemu-system-arm ${qemu_option}

