/*
 * Copyright (c) 2017-2021 Arm Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __REGION_DEFS_H__
#define __REGION_DEFS_H__

#include "flash_layout.h"

#define S_HEAP_SIZE             (0x0001000)
#define S_MSP_STACK_SIZE_INIT   (0x0000400)
#define S_MSP_STACK_SIZE        (0x0000800)
#define S_PSP_STACK_SIZE        (0x0000800)

/*
*  0x00000000 .. 0x0fffffff : Reserved
*  0x10000000 .. 0x101fffff : Secure Flash(2M)
	*  0x10000000 .. 0x100fffff : Secure code(1M)
	*  0x10100000 .. 0x101fffff : Non-Secure-callable code(1M)
*  0x10200000 .. 0x103fffff : Non-Secure Flash(2M)
*  0x20000000 .. 0x201fffff : Secure SRAM(2M)
*  0x20200000 .. 0x203fffff : Non-Secure SRAM(2M)
*  0x40000000 .. 0x40000fff : APB UART(4K)
*  0x50000000 .. 0x50000fff : Thomas Test Device(4K)
*/
/* Secure regions */
#define S_CODE_START    0x10000000
#define S_CODE_SIZE     0x100000 //1M
#define S_CODE_LIMIT    (S_CODE_START + S_CODE_SIZE - 1)

#define S_DATA_START    0x20000000
#define S_DATA_SIZE     0x200000 //2M
#define S_DATA_LIMIT    (S_DATA_START + S_DATA_SIZE - 1)

/* CMSE Veneers region */
#define CMSE_VENEER_REGION_START  (S_CODE_LIMIT + 1)
#define CMSE_VENEER_REGION_SIZE   (0x340)

/* Non-secure regions */
#define NS_CODE_START   0x10200000
#define NS_CODE_SIZE    0x200000 //2M
#define NS_CODE_LIMIT   (NS_CODE_START + NS_CODE_SIZE - 1)

#define NS_DATA_START   0x20200000
#define NS_DATA_SIZE    0x200000 //2M
#define NS_DATA_LIMIT   (NS_DATA_START + NS_DATA_SIZE - 1)

/* Shared between BL and TFM */
#define BOOT_TFM_SHARED_DATA_SIZE	(0)


/* NS partition information is used for SAU configuration */
#define NS_PARTITION_START  (0x10200000)
#define NS_PARTITION_SIZE   (0x200000)

/* Peripherals NS address range for the platform */
#define PERIPHERALS_BASE_NS_START   (0x40000000)
#define PERIPHERALS_BASE_NS_END     (0x5fffffff)

#endif /* __REGION_DEFS_H__ */
