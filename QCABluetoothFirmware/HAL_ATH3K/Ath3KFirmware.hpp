/** @file
  Copyright (c) 2021 cjiang. All rights reserved.
  Copyright (c) 2021 zxtstd. All rights reserved.
  SPDX-License-Identifier: GPL-2.0-or-later
**/

//
//  Ath3KFirmware.hpp
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

#ifndef Ath3KFirmware_hpp
#define Ath3KFirmware_hpp

#include "QCABluetoothFirmware.hpp"

class Ath3KFirmware : public QCABluetoothFirmware
{
    OSDeclareDefaultStructors(Ath3KFirmware)
    
public:
    virtual bool            start(
                                IOService       *   provider                ) override;
protected:
    IOReturn                getVendorState();
    IOReturn                getVendorVersion();
    IOReturn                getDeviceStatus();
    IOReturn                switchPID();
    IOReturn                setNormalMode();
    bool                    loadPatchRom();
    bool                    loadSysCfg();
};

#endif /* Ath3KFirmware_hpp */
