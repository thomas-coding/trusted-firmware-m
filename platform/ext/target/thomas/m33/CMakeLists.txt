#-------------------------------------------------------------------------------
# Copyright (c) 2020-2021, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_policy(SET CMP0076 NEW)
set(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

#========================= Platform region defs ===============================#

target_include_directories(platform_region_defs
    INTERFACE
        partition
)

#========================= Platform common defs ===============================#

# Specify the location of platform specific build dependencies.
target_sources(tfm_s
    PRIVATE
    $<$<C_COMPILER_ID:GNU>:${CMAKE_CURRENT_SOURCE_DIR}/gcc/startup_ARMCM33_s.S>
)

target_add_scatter_file(tfm_s
    $<$<AND:$<VERSION_LESS:${TFM_ISOLATION_LEVEL},3>,$<C_COMPILER_ID:GNU>>:${CMAKE_SOURCE_DIR}/platform/ext/common/gcc/tfm_common_s.ld>
)

#========================= Platform Secure ====================================#

target_include_directories(platform_s
    PUBLIC
        .
        cmsis_core
        partition
        native_drivers
        cmsis_drivers/config
        ${PLATFORM_DIR}/..
)

set(tfm_sys ${CMAKE_CURRENT_SOURCE_DIR}/services/src/tfm_platform_system.c)
target_sources(platform_s
    PRIVATE
	    cmsis_core/system_ARMCM33.c
        tfm_hal_isolation.c
		device_definition.c
		spm_hal.c
		target_cfg.c
		dummy/dummy_nv_counters.c
		dummy/dummy_otp.c
		dummy/dummy_provisioning.c
		cmsis_drivers/Driver_USART.c
		native_drivers/uart_cmsdk_drv.c
		native_drivers/mpu_armv8m_drv.c
        ${CMAKE_SOURCE_DIR}/platform/ext/common/tfm_hal_isolation_mpu_v8m.c
        ${CMAKE_SOURCE_DIR}/platform/ext/common/tfm_hal_nvic.c
        $<$<BOOL:${TFM_PARTITION_PLATFORM}>:${tfm_sys}>
)

target_compile_options(platform_s
    PUBLIC
        ${COMPILER_CMSE_FLAG}
)

