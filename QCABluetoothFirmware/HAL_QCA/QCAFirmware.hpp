/** @file
  Copyright (c) 2021 cjiang. All rights reserved.
  SPDX-License-Identifier: GPL-2.0-or-later
**/

//
//  QCAFirmware.hpp
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

#ifndef QCAFirmware_hpp
#define QCAFirmware_hpp

#include "QCABluetoothFirmware.hpp"

class QCAFirmware : public QCABluetoothFirmware
{
    OSDeclareDefaultStructors(QCAFirmware)
    
public:
    virtual bool start( IOService * provider ) override;
    
private:
    IOReturn getFirmwareState();
    IOReturn getFirmwareVersion();
    bool getRamPatchVersion();
    bool getDeviceInfo();
    
    void getRamPatchUSBName();
    void getNVMUSBName();
    bool loadRamPatch();
    bool loadNVM();
};

#endif /* QCAFirmware_hpp */
