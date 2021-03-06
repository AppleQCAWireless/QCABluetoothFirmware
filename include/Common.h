/** @file
  Copyright (c) 2021 cjiang. All rights reserved.
  SPDX-License-Identifier: GPL-2.0-or-later
**/

//
//  Common.h
//  QCABluetoothFirmware
//
//  Copyright © 2021 cjiang. All rights reserved.
//
//-----------------------------------------------------------------------
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*/

#ifndef Common_h
#define Common_h

#include <libkern/OSTypes.h>

typedef UInt8       u8;
typedef UInt16      u16;
typedef UInt32      u32;
typedef UInt64      u64;

typedef u8          __u8;
typedef u16         __u16;
typedef u32         __u32;
typedef u64         __u64;

typedef SInt16      __be16;
typedef SInt32      __be32;
typedef SInt64      __be64;
typedef SInt16      __le16;
typedef SInt32      __le32;
typedef SInt64      __le64;

typedef SInt8       s8;
typedef SInt16      s16;
typedef SInt32      s32;
typedef SInt64      s64;

typedef s8          __s8;
typedef s16         __s16;
typedef s32         __s32;
typedef s64         __s64;

#define __cpu_to_le64(x) ((__le64)(__u64)(x))
#define __le64_to_cpu(x) ((__u64)(__le64)(x))
#define __cpu_to_le32(x) ((__le32)(__u32)(x))
#define __le32_to_cpu(x) ((__u32)(__le32)(x))
#define __cpu_to_le16(x) ((__le16)(__u16)(x))
#define __le16_to_cpu(x) ((__u16)(__le16)(x))
#define __cpu_to_be64(x) ((__be64)__swab64((x)))
#define __be64_to_cpu(x) __swab64((__u64)(__be64)(x))
#define __cpu_to_be32(x) ((__be32)__swab32((x)))
#define __be32_to_cpu(x) __swab32((__u32)(__be32)(x))
#define __cpu_to_be16(x) ((__be16)__swab16((x)))
#define __be16_to_cpu(x) __swab16((__u16)(__be16)(x))

#define cpu_to_le64 __cpu_to_le64
#define le64_to_cpu __le64_to_cpu
#define cpu_to_le32 __cpu_to_le32
#define le32_to_cpu __le32_to_cpu
#define cpu_to_le16 __cpu_to_le16
#define le16_to_cpu __le16_to_cpu
#define cpu_to_be64 OSSwapHostToBigInt64
#define be64_to_cpu OSSwapBigToHostInt64
#define cpu_to_be32 OSSwapHostToBigInt32
#define be32_to_cpu OSSwapBigToHostInt32
#define cpu_to_be16 OSSwapHostToBigInt16
#define be16_to_cpu OSSwapBigToHostInt16

#define __packed __attribute__((packed)) __attribute__((aligned(1)))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static inline __u32 __le32_to_cpup(const __le32 *p)
{
    return (__u32) *p;
}

static inline u32 get_unaligned_le32(const void *p)
{
    return __le32_to_cpup((__le32 *) p);
}

#define safe_delete(x) do { if (x) { delete x; x = NULL; } } while (0)
#define safe_delete_arr(x) do { if (x) { delete[] x; x = NULL; } } while (0)

#define DRIVER_NAME "Ath3KBluetoothFirmware"

#ifdef DEBUG
#define DebugLog(args...) do { IOLog(DRIVER_NAME ": " args); } while (0)
#else
#define DebugLog(args...) do { } while (0)
#endif /* DEBUG */

#define AlwaysLog(args...) do { IOLog(DRIVER_NAME ": " args); } while (0)

#define ErrorLog(args...) AlwaysLog("Error! " args)
#define InfoLog(args...) AlwaysLog(args)
#define WarningLog(args...) DebugLog("Warning! " args)
#define FuncLog(args...) DebugLog(args "()\n")

#endif /* Common_h */
