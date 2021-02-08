//
//  Common.h
//  Ath3KBluetoothFirmware
//
//  Created by Charlie Jiang on 2/4/21.
//  Copyright © 2021 cjiang. All rights reserved.
//  Copyright © 2021 zxystd. All rights reserved.
//

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

#define __le32_to_cpu(x) ((__u32)(__le32)(x))
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
