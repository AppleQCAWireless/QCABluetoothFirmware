/** @file
  Copyright (c) 2021 cjiang. All rights reserved.
  SPDX-License-Identifier: GPL-2.0-or-later
**/

//
//  QCABluetoothFirmware.cpp
//  QCABluetoothFirmware
//
//  Created by Charlie Jiang on 2/10/21.
//  Copyright © 2021 Charlie Jiang. All rights reserved.
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

#include "QCABluetoothFirmware.hpp"

#define super IOService

OSDefineMetaClassAndAbstractStructors(QCABluetoothFirmware, IOService)

IOService * QCABluetoothFirmware::probe(IOService * provider, SInt32 * score)
{
    FuncLog("probe");
    
    if (!super::probe(provider, score))
    {
        ErrorLog("(probe) Super probe failed!!!\n");
        return NULL;
    }
    
    m_pUSBDevice = OSDynamicCast(IOUSBHostDevice, provider);
    if (!m_pUSBDevice)
    {
        ErrorLog("(probe) Provider is not a USB device!!!\n");
        return NULL;
    }
    
    u16 vendorID  = USBToHost16(m_pUSBDevice->getDeviceDescriptor()->idVendor);
    u16 productID = USBToHost16(m_pUSBDevice->getDeviceDescriptor()->idProduct);
    
    InfoLog("(probe) deviceName = %s\n",             m_pUSBDevice->getName());
    InfoLog("(probe) className  = %s\n",             provider->metaClass->getClassName());
    InfoLog("(probe) vendorID   = 0x%04X\n",         vendorID);
    InfoLog("(probe) productID  = 0x%04X\n",         productID);
    
    for (int i = 0; i < ARRAY_SIZE(ID_MODEL); ++i)
    {
        if (ID_MODEL[i].usbDevice.vendorID == vendorID && ID_MODEL[i].usbDevice.productID == productID)
        {
            m_socType = ID_MODEL[i].model;
            break;
        }
    }
    
    if (m_pUSBDevice)
    {
        m_pUSBDevice = NULL;
    }
    
    return this;
}

void QCABluetoothFirmware::stop(IOService * provider)
{
    FuncLog("stop");
    
    releaseAll();
    PMstop();
    
    super::stop(provider);
}

bool QCABluetoothFirmware::handleOpen(IOService * forClient, IOOptionBits options, void * arg)
{
    FuncLog("handleOpen");
    
    return super::handleOpen(forClient, options, arg);
}

void QCABluetoothFirmware::handleClose(IOService * forClient, IOOptionBits options)
{
    FuncLog("handleClose");
    
    super::handleClose(forClient, options);
}

bool QCABluetoothFirmware::terminate(IOOptionBits options)
{
    FuncLog("terminate");
    
    return super::terminate(options);
}

bool QCABluetoothFirmware::finalize(IOOptionBits options)
{
    FuncLog("finalize");
    
    return super::finalize(options);
}

bool QCABluetoothFirmware::init(OSDictionary * propTable)
{
    FuncLog("init");
    
    m_socType           = QCA_INVALID;
    
    m_pUSBDevice        = NULL;
    m_pBulkWritePipe    = NULL;
    m_pBulkWritePipe    = NULL;
    
    m_fwState = NULL;
    m_fwVersion = NULL;
    m_devInfo = NULL;
    m_rpVersion = NULL;
    
    m_devStatus = NULL;
    
    m_socVersion = NULL;
        
    m_fwData = NULL;
    m_hciCommand = NULL;
    m_fwFilename = "";
    
    m_tlvType = TLV_TYPE_INVALID;
    m_bdRate  = QCA_BAUDRATE_115200;
    m_dnldMode = QCA_SKIP_EVT_NONE;
    
    return super::init(propTable);
}

void QCABluetoothFirmware::free()
{
    FuncLog("free");
    
    PMstop();
    releaseAll();
    
    super::free();
}

IOReturn QCABluetoothFirmware::message(UInt32 type, IOService * provider, void * argument)
{
    FuncLog("message");
    
    /* TO-DO: add message logs */
    switch (type)
    {
        case kIOMessageServiceIsTerminated:
        {
            if (m_pUSBDevice && m_pUSBDevice->isOpen(this))
            {
                DebugLog("(message) Service is terminated - closing device.");
            }
            break;
        }
        case kIOMessageServiceIsSuspended:
        {
            break;
        }
        case kIOMessageServiceIsResumed:
        {
            break;
        }
        case kIOMessageServiceIsRequestingClose:
        {
            break;
        }
        case kIOMessageServiceIsAttemptingOpen:
        {
            break;
        }
        case kIOMessageServiceWasClosed:
        {
            break;
        }
        case kIOMessageServiceBusyStateChange:
        {
            break;
        }
        case kIOMessageServicePropertyChange:
        {
            break;
        }
        default:
        {
            DebugLog("(message) Unknown message.");
            
            break;
        }
    }
    
    return super::message(type, provider, argument);
}

IOReturn QCABluetoothFirmware::setPowerState(unsigned long powerStateOrdinal, IOService * whatDevice)
{
    FuncLog("setPowerState");
    
    InfoLog("(setPowerState) Setting power state to: %lu\n", powerStateOrdinal);
    
    return super::setPowerState(powerStateOrdinal, whatDevice);
}

void QCABluetoothFirmware::releaseAll()
{
    resetDevice();
    
    if (m_pUSBDevice)
    {
        if (m_pUSBDevice->isOpen(this))
        {
            if (m_pUSBDevice->setConfiguration(0))
            {
                ErrorLog("Failed to reset the device!!!\n");
            }
            m_pUSBDevice->close(this);
        }
        m_pUSBDevice = NULL;
    }
    
    if (m_pInterface)
    {
        if (m_pInterface->isOpen(this))
        {
            m_pInterface->close(this);
        }
        m_pInterface = NULL;
    }
    
    if (m_pBulkWritePipe)
    {
        m_pBulkWritePipe->abort();
        OSSafeReleaseNULL(m_pBulkWritePipe);
    }
    
    safe_delete(m_fwState);
    
    safe_delete(m_fwVersion);
    
    safe_delete(m_devStatus);
    
    safe_delete(m_rpVersion);
    
    safe_delete(m_devInfo);
    
    safe_delete(m_socVersion);
    
    safe_delete(m_hciCommand);
    
    OSSafeReleaseNULL(m_fwData);
}

IOReturn QCABluetoothFirmware::sendVendorRequestIn(u8 bRequest, void * dataBuffer, UInt16 size)
{
    UInt32 bytesTransferred;
    
    StandardUSB::DeviceRequest request =
    {
        .bmRequestType = USBmakebmRequestType( kRequestDirectionIn, kRequestTypeVendor, kRequestRecipientDevice ),
        .bRequest = bRequest,
        .wValue = 0,
        .wIndex = 0,
        .wLength = size
    };
    
    return  m_pUSBDevice->deviceRequest( this, request, dataBuffer, bytesTransferred, kUSBHostStandardRequestCompletionTimeout );
}

IOReturn QCABluetoothFirmware::sendVendorRequestOut(u8 bRequest, void * dataBuffer, UInt16 size)
{
    UInt32 bytesTransferred;
    
    StandardUSB::DeviceRequest request =
    {
        .bmRequestType = USBmakebmRequestType( kRequestDirectionOut, kRequestTypeVendor, kRequestRecipientDevice ),
        .bRequest = bRequest,
        .wValue = 0,
        .wIndex = 0,
        .wLength = size
    };
    
    return  m_pUSBDevice->deviceRequest( this, request, dataBuffer, bytesTransferred, kUSBHostStandardRequestCompletionTimeout );
}

IOReturn QCABluetoothFirmware::sendHCIRequest(uint16_t opCode, uint8_t paramLen, const void * param)
{
    FuncLog("sendHCIRequest");
    InfoLog("opCode:        0x%02x\n",  opCode);
    InfoLog("paramLen:      %d\n",      paramLen);
    
    StandardUSB::DeviceRequest request =
    {
        .bmRequestType = makeDeviceRequestbmRequestType(kRequestDirectionOut, kRequestTypeClass, kRequestRecipientDevice),
        .bRequest = 0,
        .wValue = 0,
        .wIndex = 0,
        .wLength = (uint16_t)(HCI_COMMAND_HDR_SIZE + paramLen)
    };
    
    UInt32 bytesTransfered;
    
    bzero(m_hciCommand, sizeof( HciCommandHdr ));
    m_hciCommand->opcode = opCode;
    m_hciCommand->plen = paramLen;
    
    memcpy((void *) m_hciCommand->pData, param, paramLen);
    
    return m_pInterface->deviceRequest(request, (void *) m_hciCommand, bytesTransfered);
}

bool QCABluetoothFirmware::resetDevice()
{
    FuncLog("resetDevice");
    
    if (sendHCIRequest(HCI_OP_RESET, 0, NULL))
    {
        ErrorLog("Failed to reset device!!!\n");
        return false;
    }

    return true;
}

void QCABluetoothFirmware::powerStart( IOService * provider )
{
    #ifndef kIOPMPowerOff
    #define kIOPMPowerOff 0
    #endif /* kIOPMPowerOff */
    
    static IOPMPowerState myTwoStates[2] =
    {
        {1, kIOPMPowerOff, 0,            0,            0, 0, 0, 0, 0, 0, 0, 0},
        {1, kIOPMPowerOn,  kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0}
    };
    
    PMinit();
    registerPowerDriver(this, myTwoStates, 2);
    provider->joinPMtree(this);
    makeUsable();
}

//don't use if m_pUSBDevice is not opened
bool QCABluetoothFirmware::initUSBConfiguration()
{
    if (!m_pUSBDevice)
    {
        ErrorLog("(initUSBConfiguration) Provider not found!!!\n");
        
        return false;
    }
    
    int                                          cntCfg           = m_pUSBDevice->getDeviceDescriptor()->bNumConfigurations;
    const StandardUSB::ConfigurationDescriptor * configDescriptor = m_pUSBDevice->getConfigurationDescriptor(CONFIG_INDEX);
    
    if (cntCfg < 1)
    {
        ErrorLog("(initUSBConfiguration) No composite configurations!!!\n");
        
        return false;
    }
    
    InfoLog("(initUSBConfiguration) Number of configurations: %d\n", cntCfg);
    
    if (!configDescriptor)
    {
        ErrorLog("(initUSBConfiguration) No configuration descriptor!!!\n");
        
        return false;
    }
    
    InfoLog("(initUSBConfiguration) Setting configuration to %d\n", configDescriptor->bConfigurationValue);
    
    if (m_pUSBDevice->setConfiguration(configDescriptor->bConfigurationValue))
    {
        ErrorLog("(initUSBConfiguration) Failed setting configuration to %d\n", configDescriptor->bConfigurationValue);
        
        return false;
    }
    
    return true;
}

bool QCABluetoothFirmware::initInterface()
{
    if (!m_pUSBDevice || !m_pUSBDevice->isOpen(this))
    {
        ErrorLog("(initInterface) Provider not found!!!\n");
        
        return false;
    }
    
    OSIterator * iterator = m_pUSBDevice->getChildIterator(gIOServicePlane);
    
    if (!iterator)
    {
        ErrorLog("(initInterface) Iterator failed to initialize!!!\n");
        
        return false;
    }
    
    while (1)
    {
        m_pInterface = OSDynamicCast(IOUSBHostInterface, iterator->getNextObject());
        
        if (m_pInterface)
        {
            break;
        }
    }
    
    OSSafeReleaseNULL(iterator);
    
    if (!m_pInterface)
    {
        ErrorLog("(initInterface) Unable to find USB Interface!!!\n");
        return false;
    }

    if (!m_pInterface->open(this))
    {
        ErrorLog("(initInterface) Unable to open USB Interface!!!\n");
        
        if (m_pInterface)
        {
            m_pInterface = NULL;
        }
        
        return false;
    }
    
    const StandardUSB::ConfigurationDescriptor      *       configDescriptor        =   m_pInterface->getConfigurationDescriptor();
    const StandardUSB::InterfaceDescriptor          *       interfaceDescriptor     =   m_pInterface->getInterfaceDescriptor();
    const EndpointDescriptor                        *       endpointDescriptor      =   NULL;
    uint8_t                                                 epDir;
    uint8_t                                                 epType;
    uint8_t                                                 epNum;
    
    if (!configDescriptor || !interfaceDescriptor)
    {
        ErrorLog("(initInterface) A descriptor is invalid!!!\n");
        
        return false;
    }
    
    while ((endpointDescriptor = StandardUSB::getNextEndpointDescriptor(configDescriptor, interfaceDescriptor, endpointDescriptor)))
    {
        epDir       = StandardUSB::getEndpointDirection(endpointDescriptor);
        epType      = StandardUSB::getEndpointType(endpointDescriptor);
        epNum       = StandardUSB::getEndpointNumber(endpointDescriptor);
        
        if (epDir  == kUSBOut  && epType == kUSBBulk && epNum == 2)
        {
            DebugLog("(initInterface) Found bulk-out endpoint.\n");
            
            m_pBulkWritePipe = m_pInterface->copyPipe(StandardUSB::getEndpointAddress(endpointDescriptor));
            
            if (!m_pBulkWritePipe)
            {
                ErrorLog("(initInterface) Failed to copy BulkWritePipe!!!\n");
                
                return false;
            }
            
            m_pBulkWritePipe->retain();
            m_pBulkWritePipe->release();
        }
        else
        {
            ErrorLog("(initInterface) Endpoint invalid!!!\n");
            
            return false;
        }
    }
    
    DebugLog("Interface successfully initialized!");
    
    return true;
}

bool QCABluetoothFirmware::loadFirmware(OSData * fwData, size_t headerSize)
{
    u8 * sendBuf = (u8 *) fwData->getBytesNoCopy();
    
    if (!sendBuf)
    {
        ErrorLog("(loadFirmware) Unable to allocate memory chunk for firmware!!!\n");
        
        return false;
    }
    
    if (sendVendorRequestOut(QCA_DOWNLOAD, (void *) fwData->getBytesNoCopy(), headerSize))
    {
        ErrorLog("(loadFirmware) Failed to download firmware!!!\n");
        
        return false;
    }
    
    unsigned long size = fwData->getLength() - headerSize; /* size of the firmware */
    sendBuf += headerSize;
    
    int i = 1; /* Indicator of current bulk pipe block */
    u32 toSend; /* Size to send in each block */
    
    while (size)
    {
        toSend = (size < BULK_SIZE) ? size : BULK_SIZE; /* <= is also OK, but I presume that calling a macro is faster than calling an int. */
        
        char * tempBuf = new char [size];
        IOMemoryDescriptor * memBuf = IOMemoryDescriptor::withAddress(& tempBuf, BULK_SIZE, kIODirectionOut);
        
        if (!memBuf)
        {
            ErrorLog("(loadFirmware) Failed to map memory descriptor!!!\n");
            
            safe_delete_arr(tempBuf);
            
            return false;
        }
        
        if (memBuf->prepare())
        {
            ErrorLog("(loadFirmware) Failed to prepare memory descriptor!!!\n");
            
            OSSafeReleaseNULL(memBuf);
            safe_delete_arr(tempBuf);
            
            return false;
        }
        
        DebugLog("(loadFirmware) Memory descriptor successfully prepared.\n");
        
        memcpy(tempBuf, sendBuf, toSend);

        int ret = m_pBulkWritePipe->io(memBuf, toSend, (IOUSBHostCompletion *) NULL, 0);
        if (ret)
        {
            ErrorLog("(loadFirmware) Failed writing firmware to bulk pipe (err: %d, block: %d, to_send: %u)!!!\n", ret, i, (unsigned int) toSend);
            
            OSSafeReleaseNULL(memBuf);
            safe_delete_arr(tempBuf);
            
            return false;
        }
        
        DebugLog("(loadFirmware) Loaded firmware to bulk pipe block %d.\n", i);
        
        memBuf->complete();
        OSSafeReleaseNULL(memBuf);
        safe_delete_arr(tempBuf);
        
        sendBuf += toSend;
        size    -= toSend;
        
        ++i; /* Going to the next block... */
    }
    
    DebugLog("(loadFirmware) Successfully loaded firmware.\n");
    
    return true;
}

inline bool QCABluetoothFirmware::isAth3K()
{
    return (m_socType == QCA_ATH3012);
}

inline bool QCABluetoothFirmware::isQcaUsb()
{
    return (m_socType == QCA_ROME_USB) || (m_socType == QCA_WCN6855);
    
}

inline bool QCABluetoothFirmware::isQcaSoc()
{
    return (m_socType == QCA_AR3002) || (m_socType == QCA_ROME) || (m_socType == QCA_WCN3990) || (m_socType == QCA_WCN3998) || (m_socType == QCA_WCN3991) || (m_socType == QCA_QCA6390);
}
