#!/bin/bash

shell_folder=$(cd "$(dirname "$0")" || exit;pwd)

tfm_home=${shell_folder}

# shell folder
workspace_dir=${shell_folder}/../../..

tfm_mode=ipc
tfm_debug=y
tfm_test=n

# toolchain
toolchains_try_dir1=~/.toolchains;
toolchains_try_dir2=${workspace_dir}/.toolchains;
toolchian=gcc-arm-none-eabi-10.3-2021.10

# cmake
cmake_try_dir1=${toolchains_try_dir1}/cmake-3.20.5-linux-x86_64
cmake_try_dir2=${toolchains_try_dir2}/cmake-3.20.5-linux-x86_64

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

# get cmake and export
function get_and_export_cmake()
{
    echo "get cmake"
    if [[ -d ${cmake_try_dir1} ]]; then
        if [[ -d ${cmake_try_dir1}/bin ]]; then
            echo "find cmake from ${cmake_try_dir1}"
            export PATH="${cmake_try_dir1}/bin:$PATH"
            return
        fi
    fi
    
    if [[ -d ${cmake_try_dir2} ]]; then
        if [[ -d ${cmake_try_dir2}/bin ]]; then
            echo "find cmake from ${cmake_try_dir2}"
            export PATH="${cmake_try_dir2}/bin:$PATH"
            return
        fi
    fi
    echo "cann't find cmake, please run root script './build.sh --cmake' ,exit ...."
    exit
}

function get_external_lib()
{
    if [[ ! -d ${tfm_home}/external ]]; then
        echo "get external lib"
        mkdir "${tfm_home}"/external

        #mbedcrypto
        cd "${tfm_home}"/external || exit
        git clone --no-checkout --depth 1 --no-single-branch --progress --config "advice.detachedHead=false" "https://github.com/ARMmbed/mbedtls.git" "mbedcrypto-src"
        cd "${tfm_home}"/external/mbedcrypto-src/ || exit
        git checkout mbedtls-3.0.0
		git apply ${tfm_home}/lib/ext/mbedcrypto/*.patch

        #tfm test
        cd "${tfm_home}"/external || exit
        git clone --no-checkout --progress --config "advice.detachedHead=false" "https://git.trustedfirmware.org/TF-M/tf-m-tests.git" "tfm_test_repo-src"
        cd "${tfm_home}"/external/tfm_test_repo-src/ || exit
        git checkout release/1.5.x
	
        #tfm psa test
        cd "${tfm_home}"/external || exit
        git clone --no-checkout --progress --config "advice.detachedHead=false" "https://github.com/ARM-software/psa-arch-tests.git" "pas-arch-tests-src"
        cd "${tfm_home}"/external/pas-arch-tests-src/ || exit
        git checkout v21.10_API1.3_ADAC_ALPHA-1
	
        #mcu boot
        cd "${tfm_home}"/external || exit
        git clone --no-checkout --progress --config "advice.detachedHead=false" "https://github.com/mcu-tools/mcuboot.git" "mcuboot-src"
        cd "${tfm_home}"/external/mcuboot-src/ || exit
        git checkout v1.8.0
		cp -rf ${tfm_home}/lib/ext/mbedcrypto/CMakeLists.txt ${tfm_home}/external/mcuboot-src/

    fi    
}

get_and_export_toolchain
get_and_export_cmake
get_external_lib

#build_option
build_option=
build_option+=" .. -DTFM_PLATFORM=thomas/m33 -DBL2=False"
#build_option+=" .. -DTFM_PLATFORM=arm/mps2/an521 -DBL2=False"
build_option+=" -DTFM_TOOLCHAIN_FILE=${tfm_home}/toolchain_GNUARM.cmake"

build_option+=" -DMBEDCRYPTO_PATH=${tfm_home}/external/mbedcrypto-src"
build_option+=" -DTFM_TEST_REPO_PATH=${tfm_home}/external/tfm_test_repo-src"
build_option+=" -DMCUBOOT_PATH=${tfm_home}/external/mcuboot-src"
build_option+=" -DPSA_ARCH_TESTS_PATH=${tfm_home}/external/pas-arch-tests-src"


if [[ "${tfm_mode}" = "ipc" ]]; then
    echo "tfm IPC mode"
    build_option+=" -DTFM_PROFILE=profile_medium"
    build_option+=" -DTFM_PSA_API=ON"
fi

if [[ "${tfm_debug}" = "y" ]]; then
    echo "tfm enable debug"
    build_option+=" -DCMAKE_BUILD_TYPE=Debug"
fi


if [[ "${tfm_test}" = "y" ]]; then
    echo "tfm enable test"
    build_option+=" -DTEST_NS=ON"
    build_option+=" -DTEST_S=ON"
fi

# Build TFM
rm -rf "${tfm_home}"/build
mkdir "${tfm_home}"/build

cd "${tfm_home}"/build || exit

cmake ${build_option}
make install

arm-none-eabi-objdump -xD bin/tfm_s.elf > bin/tfm_s.asm

# Build FreeRTOS
cd ${shell_folder}/../FreeRTOS/FreeRTOS/Demo/THOMAS_M33_QEMU
rm -rf build
make DEBUG=1
arm-none-eabi-objdump -xd build/RTOSDemo.axf > build/RTOSDemo.asm
