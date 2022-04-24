#-------------------------------------------------------------------------------
# Copyright (c) 2020, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set(TFM_PARTITION_CRYPTO                OFF          CACHE BOOL      "Disable Crypto partition")
set(TFM_PARTITION_INITIAL_ATTESTATION   OFF          CACHE BOOL      "Disable Initial Attestation partition")
set(TFM_PARTITION_PROTECTED_STORAGE     OFF          CACHE BOOL      "Disable Protected Storage partition")
set(TFM_PARTITION_INTERNAL_TRUSTED_STORAGE OFF       CACHE BOOL      "Disable Internal Trusted Storage partition")
set(TFM_PARTITION_PLATFORM              ON           CACHE BOOL      "Enable Platform partition")
#set(PLATFORM_DEFAULT_NV_COUNTERS        OFF          CACHE BOOL      "not Use default nv counter implementation.")
set(PLATFORM_DEFAULT_NV_COUNTERS        OFF          CACHE BOOL      "Not use default nv counter implementation.")
set(PLATFORM_DEFAULT_OTP                OFF          CACHE BOOL      "Not use trusted on-chip flash to implement OTP memory")
#set(OTP_NV_COUNTERS_RAM_EMULATION       ON         CACHE BOOL      "Enable OTP/NV_COUNTERS emulation in RAM. Has no effect on non-default implementations of the OTP and NV_COUNTERS")

set(NS                                  OFF          CACHE BOOL      "Whether to build NS app")
