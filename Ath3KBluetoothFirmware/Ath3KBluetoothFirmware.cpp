//
//  Ath3KBluetoothFirmware.cpp
//  Ath3KBluetoothFirmware
//
//  Created by Charlie Jiang on 2/4/21.
//  Copyright © 2021 cjiang. All rights reserved.
//  Copyright © 2021 zxystd. All rights reserved.
//

#include "Ath3KBluetoothFirmware.hpp"

#define super IOService
OSDefineMetaClassAndStructors(Ath3KBluetoothFirmware, IOService)

#define kIOPMPowerOff 0

static IOPMPowerState myTwoStates[2] =
{
    {1, kIOPMPowerOff, 0,            0,            0, 0, 0, 0, 0, 0, 0, 0},
    {1, kIOPMPowerOn,  kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0}
};

IOService * Ath3KBluetoothFirmware::probe(IOService * provider, SInt32 * score)
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
    
    InfoLog("(probe) devName = %s\n",             m_pUSBDevice->getName());
    InfoLog("(probe) className = %s\n",           provider->metaClass->getClassName());
    InfoLog("(probe) vendorID = 0x%04X\n",        USBToHost16(m_pUSBDevice->getDeviceDescriptor()->idVendor));
    InfoLog("(probe) productID = 0x%04X\n",       USBToHost16(m_pUSBDevice->getDeviceDescriptor()->idProduct));
    
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
    
    return this;
}

bool Ath3KBluetoothFirmware::start(IOService * provider)
{
    FuncLog("start");
    
    m_pUSBDevice = OSDynamicCast(IOUSBHostDevice, provider);
    
    if (!m_pUSBDevice)
    {
        ErrorLog("(start) Provider is not a USB device!!!\n");
        
        releaseAll();
        
        return false;
    }

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
    
    PMinit();
    registerPowerDriver(this, myTwoStates, 2);
    provider->joinPMtree(this);
    makeUsable();
    
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

void Ath3KBluetoothFirmware::stop(IOService * provider)
{
    FuncLog("stop");
    
    releaseAll();
    PMstop();
    
    super::stop(provider);
}

bool Ath3KBluetoothFirmware::handleOpen(IOService * forClient, IOOptionBits options, void * arg)
{
    FuncLog("handleOpen");
    
    return super::handleOpen(forClient, options, arg);
}

void Ath3KBluetoothFirmware::handleClose(IOService * forClient, IOOptionBits options)
{
    FuncLog("handleClose");
    
    super::handleClose(forClient, options);
}

bool Ath3KBluetoothFirmware::terminate(IOOptionBits options)
{
    FuncLog("terminate");
    
    return super::terminate(options);
}

bool Ath3KBluetoothFirmware::finalize(IOOptionBits options)
{
    FuncLog("finalize");
    
    return super::finalize(options);
}

bool Ath3KBluetoothFirmware::init(OSDictionary * propTable)
{
    FuncLog("init");
    m_pUSBDevice        = NULL;
    m_pBulkWritePipe    = NULL;
    m_pBulkWritePipe    = NULL;
    m_fwState           = NULL;
    m_devStatus         = NULL;
    m_fwData            = NULL;
    
    return super::init(propTable);
}

void Ath3KBluetoothFirmware::free()
{
    FuncLog("free");
    
    PMstop();
    releaseAll();
    
    super::free();
}

IOReturn Ath3KBluetoothFirmware::message(UInt32 type, IOService * provider, void * argument)
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

IOReturn Ath3KBluetoothFirmware::setPowerState(unsigned long powerStateOrdinal, IOService * whatDevice)
{
    FuncLog("setPowerState");
    
    InfoLog("(setPowerState) Setting power state to: %lu\n", powerStateOrdinal);
    
    return super::setPowerState(powerStateOrdinal, whatDevice);
}

void Ath3KBluetoothFirmware::releaseAll()
{
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
    
    if (m_fwData)
    {
        OSSafeReleaseNULL(m_fwData);
    }
}


//don't use if m_pUSBDevice is not opened

bool Ath3KBluetoothFirmware::initUSBConfiguration()
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

bool Ath3KBluetoothFirmware::initInterface()
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

bool Ath3KBluetoothFirmware::loadFirmware(OSData * fwData)
{
    uint32_t bytesTransferred;
    
    u8 * sendBuf = (u8 *) fwData->getBytesNoCopy();
    
    if (!sendBuf)
    {
        ErrorLog("(loadFirmware) Unable to allocate memory chunk for firmware!!!\n");
        
        return false;
    }
    
    StandardUSB::DeviceRequest      request;
    request.bmRequestType           =   USBmakebmRequestType
                                        (
                                            kRequestDirectionOut,
                                            kRequestTypeVendor,
                                            kRequestRecipientDevice
                                        );
    request.bRequest                =   ATH3K_DNLOAD;
    request.wValue                  =   0;
    request.wIndex                  =   0;
    request.wLength                 =   FW_HDR_SIZE;
    
    if ( m_pUSBDevice->deviceRequest( this, request, (void *) fwData->getBytesNoCopy(), bytesTransferred, (uint32_t) kUSBHostStandardRequestCompletionTimeout ) )
    {
        ErrorLog("(loadFirmware) Failed to download firmware!!!\n");
        
        return false;
    }
    
    int size = fwData->getLength() - FW_HDR_SIZE; /* size of the firmware */
    sendBuf += FW_HDR_SIZE;
    
    int i = 1; /* Indicator of current bulk pipe block */
    int toSend; /* Size to send in each block */
    
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
            ErrorLog("(loadFirmware) Failed writing firmware to bulk pipe (err: %d, block: %d, to_send: %d)!!!\n", ret, i, toSend);
            
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

IOReturn Ath3KBluetoothFirmware::getVendorState()
{
    uint32_t bytesTransferred;
    
    
    StandardUSB::DeviceRequest  request;
    request.bmRequestType       =   makeDeviceRequestbmRequestType
                                    (
                                        kRequestDirectionIn,
                                        kRequestTypeVendor,
                                        kRequestRecipientDevice
                                    );
    request.bRequest            =   ATH3K_GETSTATE;
    request.wValue              =   0;
    request.wIndex              =   0;
    request.wLength             =   sizeof( char );

    return  m_pUSBDevice->deviceRequest( this, request, m_fwState, bytesTransferred, kUSBHostStandardRequestCompletionTimeout );
}

IOReturn Ath3KBluetoothFirmware::getVendorVersion()
{
    uint32_t bytesTransferred;
    
    StandardUSB::DeviceRequest      request;
    request.bmRequestType       =   makeDeviceRequestbmRequestType
                                    (
                                        kRequestDirectionIn,
                                        kRequestTypeVendor,
                                        kRequestRecipientDevice
                                    );
    request.bRequest            =   ATH3K_GETVERSION;
    request.wValue              =   0;
    request.wIndex              =   0;
    request.wLength             =   sizeof( Ath3KVersion );
    
    return  m_pUSBDevice->deviceRequest( this, request, m_fwVersion, bytesTransferred, kUSBHostStandardRequestCompletionTimeout );
}

IOReturn Ath3KBluetoothFirmware::getDeviceStatus()
{
    uint32_t bytesTransferred;
    
    StandardUSB::DeviceRequest      request;
    request.bmRequestType       =   makeDeviceRequestbmRequestType
                                    (
                                        kRequestDirectionIn,
                                        kRequestTypeStandard,
                                        kRequestRecipientDevice
                                    );
    request.bRequest            =   kDeviceRequestGetStatus;
    request.wValue              =   0;
    request.wIndex              =   0;
    request.wLength             =   sizeof( uint16_t );
    
    return  m_pUSBDevice->deviceRequest( this, request, m_devStatus, bytesTransferred, kUSBHostStandardRequestCompletionTimeout );
}

IOReturn Ath3KBluetoothFirmware::switchPID()
{
    uint32_t bytesTransferred;
    
    StandardUSB::DeviceRequest      request;
    request.bmRequestType       =   makeDeviceRequestbmRequestType
                                    (
                                        kRequestDirectionIn,
                                        kRequestTypeVendor,
                                        kRequestRecipientDevice
                                    );
    request.bRequest            =   USB_REG_SWITCH_VID_PID;
    request.wValue              =   0;
    request.wIndex              =   0;
    request.wLength             =   0;
    
    return  m_pUSBDevice->deviceRequest( this, request, (void *) NULL, bytesTransferred, kUSBHostStandardRequestCompletionTimeout );
}

IOReturn Ath3KBluetoothFirmware::setNormalMode()
{
    if (getVendorState())
    {
        ErrorLog("(setNormalMode) Unable to get vendor state!!!\n");
        return false;
    }
    
    uint32_t    bytesTransferred;
    
    if ((* m_fwState & ATH3K_MODE_MASK) == ATH3K_NORMAL_MODE)
    {
        WarningLog("(setNormalMode) Firmware is already in normal mode!\n");
        return true;
    }
    
    StandardUSB::DeviceRequest  request;
    request.bmRequestType       =   makeDeviceRequestbmRequestType
                                    (
                                        kRequestDirectionIn,
                                        kRequestTypeVendor,
                                        kRequestRecipientDevice
                                    );
    request.bRequest            =   ATH3K_SET_NORMAL_MODE;
    request.wValue              =   0;
    request.wIndex              =   0;
    request.wLength             =   0;
    
    return  m_pUSBDevice->deviceRequest( this, request, (void *) NULL, bytesTransferred, kUSBHostStandardRequestCompletionTimeout );
}

bool Ath3KBluetoothFirmware::loadPatchRom()
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
    
    if (* m_fwState & ATH3K_PATCH_UPDATE)
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
    
    if (loadFirmware(m_fwData))
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

bool Ath3KBluetoothFirmware::loadSysCfg()
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
    
    if (* m_fwState & ATH3K_SYSCFG_UPDATE)
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
    
    if (loadFirmware(m_fwData))
    {
        DebugLog("Successfully loaded system configuration file %s...\n", m_fwFilename);
        
        m_fwData->free();
        m_fwData = NULL;
        
        return true;
    }
    else
    {
        ErrorLog("Failed to load system configuration file %s...\n", m_fwFilename);
        
        m_fwData->free();
        m_fwData = NULL;
        
        return false;
    }
}
