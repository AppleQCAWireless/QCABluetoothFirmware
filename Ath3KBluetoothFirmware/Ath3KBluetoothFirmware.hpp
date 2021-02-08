//
//  Ath3KBluetoothFirmware.hpp
//  Ath3KBluetoothFirmware
//
//  Created by Charlie Jiang on 2/4/21.
//  Copyright © 2021 cjiang. All rights reserved.
//  Copyright © 2021 zxystd. All rights reserved.
//

#ifndef Ath3KBluetoothFirmware_hpp
#define Ath3KBluetoothFirmware_hpp

#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOMessage.h>

#include <IOKit/usb/IOUSBHostDevice.h>
#include <IOKit/usb/IOUSBHostInterface.h>
#include <IOKit/usb/USB.h>

#include "Common.h"
#include "Firmware.h"

#define ATH3K_DNLOAD                    0x01
#define ATH3K_GETSTATE                  0x05
#define ATH3K_SET_NORMAL_MODE           0x07
#define ATH3K_GETVERSION                0x09
#define USB_REG_SWITCH_VID_PID          0x0a

#define ATH3K_MODE_MASK                 0x3F
#define ATH3K_NORMAL_MODE               0x0E

#define ATH3K_PATCH_UPDATE              0x80
#define ATH3K_SYSCFG_UPDATE             0x40

#define FW_HDR_SIZE                     20
#define BULK_SIZE                       4096
#define ATH3K_XTAL_FREQ_26M             0x00
#define ATH3K_XTAL_FREQ_40M             0x01
#define ATH3K_XTAL_FREQ_19P2            0x02
#define ATH3K_NAME_LEN                  0xFF

#define CONFIG_INDEX                    0

struct Ath3KVersion
{
    SInt32                  ramVersion;
    SInt32                  romVersion;
    SInt32                  buildVersion;
    UInt8                   refClock;
} __packed;

class Ath3KBluetoothFirmware : public IOService
{
    OSDeclareDefaultStructors(Ath3KBluetoothFirmware)
    
public:
    virtual IOService *     probe(
                                IOService       *   provider,
                                SInt32          *   score                   ) override;
    virtual bool            start(
                                IOService       *   provider                ) override;
    virtual void            stop(
                                IOService       *   provider                ) override;
    virtual bool            handleOpen(
                                IOService       *   forClient,
                                IOOptionBits        options = 0,
                                void            *   arg = 0                 ) override;
    virtual void            handleClose(
                                IOService       *   forClient,
                                IOOptionBits        options = 0             ) override;
    virtual bool            terminate(
                                IOOptionBits        options = 0             ) override;
    virtual bool            finalize(
                                IOOptionBits        options                 ) override;
    virtual bool            init(
                                OSDictionary    *   dictionary = NULL       ) override;
    virtual void            free(                                           ) override;
    virtual IOReturn        message(
                                UInt32              type,
                                IOService       *   provider,
                                void            *   argument                ) override;
    virtual IOReturn        setPowerState(
                                unsigned long       powerStateOrdinal,
                                IOService       *   whatDevice              ) override;

protected:
    void                    releaseAll();
    bool                    initUSBConfiguration();
    bool                    initInterface();
    bool                    loadFirmware( OSData * fwData );
    IOReturn                getVendorState();
    IOReturn                getVendorVersion();
    IOReturn                getDeviceStatus();
    IOReturn                switchPID();
    IOReturn                setNormalMode();
    bool                    loadPatchRom();
    bool                    loadSysCfg();

public:
    IOUSBHostDevice             *       m_pUSBDevice;
    IOUSBHostInterface          *       m_pInterface;
    IOUSBHostPipe               *       m_pBulkWritePipe;
    
    unsigned char               *       m_fwState;
    Ath3KVersion                *       m_fwVersion;
    USBStatus                   *       m_devStatus;
    
private:
    OSData                      *       m_fwData;
    char                                m_fwFilename[ATH3K_NAME_LEN];
};

#endif /* Ath3KBluetoothFirmware_hpp */
