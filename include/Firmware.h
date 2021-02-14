/** @file
  Copyright (c) 2021 cjiang. All rights reserved.
  Copyright (c) 2021 zxtstd. All rights reserved.
  SPDX-License-Identifier: GPL-2.0-or-later
**/

//
//  Firmware.h
//  QCABluetoothFirmware
//
//  Copyright © 2021 cjiang. All rights reserved.
//  Copyright © 2021 zxystd. All rights reserved.
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

#ifndef Firmware_h
#define Firmware_h

#include <libkern/c++/OSData.h>

#include <FIRMWARE_ATHRBT.h>
#include <FIRMWARE_RAMPS.h>
#include <FIRMWARE_CRBTFW_HTBTFW.h>
#include <FIRMWARE_RAM_PATCH.h>
#include <FIRMWARE_RAM_PATCH_USB.h>
#include <FIRMWARE_CRNV_HTNV.h>
#include <FIRMWARE_NVM.h>
#include <FIRMWARE_NVM_USB.h>

struct FwDesc
{
    const char              *name;
    const unsigned char     *var;
};

const FwDesc fwList[] =
{
    {      "AthrBT_0x01020001.dfu",     AthrBT_0x01020001_dfu      },
    {      "AthrBT_0x01020200.dfu",     AthrBT_0x01020200_dfu      },
    {      "AthrBT_0x01020201.dfu",     AthrBT_0x01020201_dfu      },
    {      "AthrBT_0x11020000.dfu",     AthrBT_0x11020000_dfu      },
    {      "AthrBT_0x11020100.dfu",     AthrBT_0x11020100_dfu      },
    {      "AthrBT_0x31010000.dfu",     AthrBT_0x31010000_dfu      },
    {      "AthrBT_0x31010100.dfu",     AthrBT_0x31010100_dfu      },
    {      "AthrBT_0x41020000.dfu",     AthrBT_0x41020000_dfu      },
    
    {      "ramps_0x01020001_26.dfu",   ramps_0x01020001_26_dfu    },
    {      "ramps_0x01020200_26.dfu",   ramps_0x01020200_26_dfu    },
    {      "ramps_0x01020200_40.dfu",   ramps_0x01020200_40_dfu    },
    {      "ramps_0x01020201_26.dfu",   ramps_0x01020201_26_dfu    },
    {      "ramps_0x01020201_40.dfu",   ramps_0x01020201_40_dfu    },
    {      "ramps_0x11020000_40.dfu",   ramps_0x11020000_40_dfu    },
    {      "ramps_0x11020100_40.dfu",   ramps_0x11020100_40_dfu    },
    {      "ramps_0x31010000_40.dfu",   ramps_0x31010000_40_dfu    },
    {      "ramps_0x31010100_40.dfu",   ramps_0x31010100_40_dfu    },
    {      "ramps_0x41020000_40.dfu",   ramps_0x41020000_40_dfu    },
    
    {      "crbtfw21.tlv",              crbtfw21_tlv               },
    {      "crbtfw32.tlv",              crbtfw32_tlv               },
    {      "htbtfw20.tlv",              htbtfw20_tlv               },
    
    {      "rampatch_00130300.bin",     rampatch_00130300_bin      },
    {      "rampatch_00130302.bin",     rampatch_00130302_bin      },
    {      "rampatch_00230302.bin",     rampatch_00230302_bin      },
    {      "rampatch_00440302.bin",     rampatch_00440302_bin      },
    
    {      "rampatch_usb_00000200.bin", rampatch_usb_00000200_bin  },
    {      "rampatch_usb_00000201.bin", rampatch_usb_00000201_bin  },
    {      "rampatch_usb_00000300.bin", rampatch_usb_00000300_bin  },
    {      "rampatch_usb_00000302.bin", rampatch_usb_00000302_bin  },
    
    {      "crnv21.bin",                crnv21_bin                 },
    {      "crnv32.bin",                crnv32_bin                 },
    {      "crnv32u.bin",               crnv32u_bin                },
    {      "htnv20.bin",                htnv20_bin                 },
    
    {      "nvm_00130300.bin",          nvm_00130300_bin           },
    {      "nvm_00130302.bin",          nvm_00130302_bin           },
    {      "nvm_00230302.bin",          nvm_00230302_bin           },
    {      "nvm_00440302.bin",          nvm_00440302_bin           },
    {      "nvm_00440302_eu.bin",       nvm_00440302_eu_bin        },
    {      "nvm_00440302_i2s_eu.bin",   nvm_00440302_i2s_eu_bin    },
    
    {      "nvm_usb_00000200.bin",      nvm_usb_00000200_bin       },
    {      "nvm_usb_00000201.bin",      nvm_usb_00000201_bin       },
    {      "nvm_usb_00000300.bin",      nvm_usb_00000300_bin       },
    {      "nvm_usb_00000302.bin",      nvm_usb_00000302_bin       },
    {      "nvm_usb_00000302_eu.bin",   nvm_usb_00000302_eu_bin    }
};

static inline OSData *getFwDescByName(const char * name)
{
    for (int i = 0; i < ARRAY_SIZE(fwList); ++i)
    {
        if (fwList[i].name == name)
        {
            return OSData::withBytes(fwList[i].var, sizeof(* fwList[i].var));
        }
    }
    return NULL;
}

#endif /* Firmware_h */
