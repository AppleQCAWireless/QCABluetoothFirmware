/** @file
  Copyright (c) 2021 cjiang. All rights reserved.
  SPDX-License-Identifier: GPL-2.0-or-later
**/

//
//  QCAFirmware.cpp
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

#include "QCAFirmware.hpp"

#define super QCABluetoothFirmware
OSDefineMetaClassAndStructors(QCAFirmware, QCABluetoothFirmware)

bool QCAFirmware::start(IOService * provider) //TO-DO!!!
{
    if (!isQcaUsb())
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
        
        //releaseAll();
        
        return false;
    }
    
    if (m_pUSBDevice->setConfiguration(0))
    {
        ErrorLog("(start) Failed to reset the device!!!\n");
        
        //releaseAll();
        
        return false;
    }
    
    DebugLog("(start) Device successfully reset.\n");
    
    
    if (getFirmwareVersion())
    {
        ErrorLog("(start) Failed to send vendor request!!!\n");
        return false;
    }

    
    getDeviceInfo();
    
    if (!m_devInfo)
    {
        ErrorLog("(start) Provider does not support firmware rome version 0x%x!", le32_to_cpu(m_fwVersion->romVersion));
        
        //releaseAll();
        
        return false;
    }

    if (getFirmwareState())
    {
        ErrorLog("(start) Failed to send vendor request!!!\n");
        return false;
    }

    if (!(* m_fwState & QCA_PATCH_UPDATED))
    {
        if (!loadRamPatch())
        {
            ErrorLog("(start) Failed to load ram patch file!!!");
            return false;
        }
    }

    if (getFirmwareVersion())
    {
        ErrorLog("(start) Failed to send vendor request!!!\n");
        return false;
    }

    if (!(* m_fwState & QCA_SYSCFG_UPDATED))
    {
        if (!loadNVM())
        {
            ErrorLog("(start) Failed to load NVM file!!!");
            return false;
        }
    }

    return true;
}

inline IOReturn QCAFirmware::getFirmwareVersion()
{
    return sendVendorRequestIn(QCA_GET_VERSION, m_fwVersion, sizeof(QCAVersion));
}

inline IOReturn QCAFirmware::getFirmwareState()
{
    return sendVendorRequestIn(QCA_GET_STATUS, m_fwState, sizeof(unsigned char));
}

inline bool QCAFirmware::getRamPatchVersion()
{
    m_rpVersion = (QCARamPatchVersion *) (m_fwData + m_devInfo->versionOffset);
    if (m_rpVersion)
    {
        return true;
    }
    return false;
}

bool QCAFirmware::getDeviceInfo()
{
    static QCADeviceInfo QCADevicesTable[] =
    {
        { 0x00000100, 20, 4,  8 },      /* Rome 1.0 */
        { 0x00000101, 20, 4,  8 },      /* Rome 1.1 */
        { 0x00000200, 28, 4, 16 },      /* Rome 2.0 */
        { 0x00000201, 28, 4, 16 },      /* Rome 2.1 */
        { 0x00000300, 28, 4, 16 },      /* Rome 3.0 */
        { 0x00000302, 28, 4, 16 },      /* Rome 3.2 */
        { 0x00130100, 40, 4, 16 },      /* WCN6855 1.0 */
        { 0x00130200, 40, 4, 16 },      /* WCN6855 2.0 */
    };
    
    for (int i = 0; i < ARRAY_SIZE(QCADevicesTable); ++i)
    {
        if (le32_to_cpu(m_fwVersion->romVersion) == QCADevicesTable[i].romVersion)
        {
            m_devInfo = &QCADevicesTable[i];
            return true;
        }
    }
    return false;
}

void QCAFirmware::getRamPatchUSBName()
{
    snprintf(m_fwFilename, sizeof(m_fwFilename), "rampatch_usb_%08x.bin", le32_to_cpu(m_fwVersion->romVersion));
}

void QCAFirmware::getNVMUSBName()
{
    if (((m_fwVersion->flag >> 8) & 0xff) == QCA_FLAG_MULTI_NVM)
    {
        snprintf(m_fwFilename, sizeof(m_fwFilename), "nvm_usb_%08x_%04x.bin", le32_to_cpu(m_fwVersion->romVersion), le16_to_cpu(m_fwVersion->boardId));
    }
    else
    {
        snprintf(m_fwFilename, sizeof(m_fwFilename), "nvm_usb_%08x.bin", le32_to_cpu(m_fwVersion->romVersion));
    }
}

bool QCAFirmware::loadRamPatch()
{
    u32 romVersion = le32_to_cpu(m_fwVersion->romVersion);
    u32 patchVersion = le32_to_cpu(m_fwVersion->patchVersion);
    u32 rpRomVersion;
    
    getRamPatchUSBName();

    m_fwData = getFwDescByName(m_fwFilename);
    
    if (!m_fwData)
    {
        ErrorLog("(loadRamPatch) Failed to request ram patch file: %s!!!\n", m_fwFilename);
        return false;
    }

    InfoLog("(loadRamPatch) Using ram patch file: %s.", m_fwFilename);
    
    getRamPatchVersion();
    
    u16 rpRomVersionLow = le16_to_cpu(m_rpVersion->romVersionLow);
    u16 rpPatchVersion = le16_to_cpu(m_rpVersion->patchVersion);

    if (romVersion & ~0xffffU)
    {
        u16 rpRomVersionHigh = le16_to_cpu(m_rpVersion->romVersionHigh);
        rpRomVersion = le32_to_cpu(rpRomVersionHigh << 16 | rpRomVersionLow);
    }
    else
    {
        rpRomVersion = rpRomVersionLow;
    }

    InfoLog("Patch Rome Version:        0x%x\n",    rpRomVersion);
    InfoLog("Patch Rome Build:          0x%x\n",    rpPatchVersion);
    InfoLog("Firmware Rome Version:     0x%x\n",    romVersion);
    InfoLog("Firmware Rome Build:       0x%x\n",    patchVersion);

    if (rpRomVersion != romVersion || rpPatchVersion <= patchVersion)
    {
        ErrorLog("(loadRamPatch) Ram patch file version did not match with the firmware version!!!\n");
        
        m_fwData->free();
        m_fwData = NULL;
        
        return false;
    }
    
    if (loadFirmware(m_fwData, m_devInfo->ramPatchHdr))
    {
        DebugLog("Successfully loaded ram patch file: %s.\n", m_fwFilename);
        
        m_fwData->free();
        m_fwData = NULL;
        
        return true;
    }
    
    ErrorLog("Failed to load ram patch file: %s!!!\n", m_fwFilename);
    
    m_fwData->free();
    m_fwData = NULL;
    
    return false;
    
}

bool QCAFirmware::loadNVM()
{
    getNVMUSBName();
    
    m_fwData = getFwDescByName(m_fwFilename);
    
    if (!m_fwData)
    {
        ErrorLog("(loadNvmUSB) Failed to request NVM file: %s!!!\n", m_fwFilename);
        return false;
    }

    InfoLog("(loadNvmUSB) Using NVM file: %s.", m_fwFilename);

    if (loadFirmware(m_fwData, m_devInfo->nvmHdr))
    {
        DebugLog("Successfully loaded NVM USB file: %s.\n", m_fwFilename);
        
        m_fwData->free();
        m_fwData = NULL;
        
        return true;
    }
    
    ErrorLog("Failed to load NVM USB file: %s!!!\n", m_fwFilename);
    
    m_fwData->free();
    m_fwData = NULL;
    
    return false;
}
