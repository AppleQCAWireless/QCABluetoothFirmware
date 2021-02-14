/** @file
  Copyright (c) 2021 cjiang. All rights reserved.
  Copyright (c) 2021 zxtstd. All rights reserved.
  SPDX-License-Identifier: GPL-2.0-or-later
**/

//
//  Ath3KFirmware.cpp
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

#include "Ath3KFirmware.hpp"

#define super QCABluetoothFirmware
OSDefineMetaClassAndStructors(Ath3KFirmware, QCABluetoothFirmware)

bool Ath3KFirmware::start(IOService * provider)
{
    if (!isAth3K())
    {
        return false;
    }
    
    FuncLog("start");
    
    m_pUSBDevice = OSDynamicCast(IOUSBHostDevice, provider);
    
    if (!m_pUSBDevice)
    {
        ErrorLog("(start) Provider is not a USB device!!!\n");
        releaseAll();
        return false;
    }

    powerStart(provider);
    
    if (!super::start(provider))
    {
        ErrorLog("(start) Super start failed!!!\n");
        releaseAll();
        return false;
    }
    
    if (m_pUSBDevice->setConfiguration(0))
    {
        ErrorLog("(start) Failed to reset the device!!!\n");
        releaseAll();
        return false;
    }
    
    DebugLog("(start) Device successfully reset.\n");
    
    if (!initUSBConfiguration())
    {
        ErrorLog("(start) Failed to initialize USB configuration!!!\n");
        releaseAll();
        return false;
    }
    
    if (!initInterface())
    {
        ErrorLog("(start) Failed to initialize interface!!!\n");
        releaseAll();
        return false;
    }
    
    if (setNormalMode())
    {
        WarningLog("(start) Failed to set normal mode!\n");
        releaseAll();
        return true;
    }
    
    if (!loadPatchRom())
    {
        WarningLog("(start) Failed to load patch rom!\n");
        releaseAll();
        return true;
    }
    
    if (!loadSysCfg())
    {
        WarningLog("(start) Failed to load system configuration!\n");
        releaseAll();
        return true;
    }
    
    if (switchPID())
    {
        WarningLog("(start) Failed to switch VID to PID!\n");
        releaseAll();
        return true;
    }
    
    DebugLog("(start) Firmware loaded successfully!!!\n");

    if (getDeviceStatus())
    {
        WarningLog("(start) Unable to obtain device status!\n");
        releaseAll();
        return true;
    }
    
    InfoLog("(start) Device Status: %d\n", (int) * m_devStatus);
    
    releaseAll();
    return true;
}

inline IOReturn Ath3KFirmware::getVendorState()
{
    return sendVendorRequestIn(QCA_GET_STATUS, (void *) m_fwState, sizeof(char));
}

inline IOReturn Ath3KFirmware::getVendorVersion()
{
    return sendVendorRequestIn(QCA_GET_VERSION, (void *) m_fwVersion, sizeof(Ath3KVersion));
}

inline IOReturn Ath3KFirmware::getDeviceStatus()
{
    return sendVendorRequestIn(kDeviceRequestGetStatus, (void *) m_devStatus, sizeof(uint16_t));
}

inline IOReturn Ath3KFirmware::switchPID()
{
    return sendVendorRequestIn(QCA_SWITCH_VID_PID, (void *) NULL, 0);
}

IOReturn Ath3KFirmware::setNormalMode()
{
    if (getVendorState())
    {
        ErrorLog("(setNormalMode) Unable to get vendor state!!!\n");
        return kIOReturnError;
    }
    
    if ((* m_fwState & ATH3K_MODE_MASK) == ATH3K_NORMAL_MODE)
    {
        WarningLog("(setNormalMode) Firmware is already in normal mode!\n");
        return kIOReturnSuccess;
    }
    
    return sendVendorRequestIn(QCA_NORMAL_MODE_SET, (void *) NULL, 0);
}

bool Ath3KFirmware::loadPatchRom()
{
    if (getVendorState())
    {
        ErrorLog("(loadPatchRom) Unable to get vendor state!!!\n");
        
        return false;
    }
    
    if (getVendorVersion())
    {
        ErrorLog("(loadPatchRom) Unable to get vendor version!!!\n");
        
        return false;
    }
    
    if (* m_fwState & QCA_PATCH_UPDATED)
    {
        WarningLog("(loadPatchRom) Firmware is already patched.\n");
        
        return true;
    }
    
    snprintf(m_fwFilename, ATH3K_NAME_LEN, "AthrBT_0x%08x.dfu", __le32_to_cpu(m_fwVersion->romVersion));
    
    DebugLog("Attempting to load patch rom file %s...\n", m_fwFilename);
    
    m_fwData = getFwDescByName(m_fwFilename);
    
    if (!m_fwData)
    {
        ErrorLog("(loadPatchRom) Patch rom file not found!!!\n");
        
        return false;
    }
    
    UInt32 patchRomVersion      = get_unaligned_le32((char *) m_fwData->getBytesNoCopy() + m_fwData->getLength() - 8);
    UInt32 patchBuildVersion    = get_unaligned_le32((char *) m_fwData->getBytesNoCopy() + m_fwData->getLength() - 4);
    
    if (patchRomVersion != __le32_to_cpu(m_fwVersion->romVersion) || patchBuildVersion <= __le32_to_cpu(m_fwVersion->buildVersion))
    {
        ErrorLog("(loadPatchRom) Patch file version did not match firmware version!!!\n");
        
        m_fwData->free();
        m_fwData = NULL;
        
        return false;
    }
    
    if (loadFirmware(m_fwData, ATH3K_FW_HDR_SIZE))
    {
        DebugLog("Successfully loaded patch rom file %s...\n", m_fwFilename);
        
        m_fwData->free();
        m_fwData = NULL;
        
        return true;
    }
    else
    {
        ErrorLog("Failed to system configuration file %s...\n", m_fwFilename);
        
        m_fwData->free();
        m_fwData = NULL;
        
        return false;
    }
}

bool Ath3KFirmware::loadSysCfg()
{
    int clkValue;
    
    if (getVendorState())
    {
        ErrorLog("(loadSysCfg) Unable to get vendor state!!!\n");
        
        return false;
    }
    
    if (getVendorVersion())
    {
        ErrorLog("(loadSysCfg) Unable to get vendor version!!!\n");
        
        return false;
    }
    
    if (* m_fwState & QCA_SYSCFG_UPDATED)
    {
        WarningLog("(loadSysCfg) System Configuration is already loaded.\n");
        
        return true;
    }
    
    switch (m_fwVersion->refClock)
    {
    case ATH3K_XTAL_FREQ_26M:
    {
        clkValue = 26;
        
        break;
    }
    case ATH3K_XTAL_FREQ_40M:
    {
        clkValue = 40;
        
        break;
    }
    case ATH3K_XTAL_FREQ_19P2:
    {
        clkValue = 19;
        
        break;
    }
    default:
    {
        clkValue = 0;
        
        break;
    }
    }
    
    snprintf(m_fwFilename, ATH3K_NAME_LEN, "ramps_0x%08x_%d%s", __le32_to_cpu(m_fwVersion->romVersion), clkValue, ".dfu");
    
    DebugLog("Attempting to load system configuration file %s...\n", m_fwFilename);
    
    m_fwData = getFwDescByName(m_fwFilename);
    
    if (!m_fwData)
    {
        ErrorLog("(loadSysCfg) System configuration file not found!!!\n");
        
        return false;
    }
    
    if (loadFirmware(m_fwData, ATH3K_FW_HDR_SIZE))
    {
        DebugLog("Successfully loaded system configuration file: %s.\n", m_fwFilename);
        
        m_fwData->free();
        m_fwData = NULL;
        
        return true;
    }
    
    ErrorLog("Failed to load system configuration file: %s!!!\n", m_fwFilename);
        
    m_fwData->free();
    m_fwData = NULL;
        
    return false;
}
