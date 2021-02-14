/** @file
  Copyright (c) 2021 cjiang. All rights reserved.
  SPDX-License-Identifier: GPL-2.0-or-later
**/

//
//  QCABluetoothFirmware.hpp
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

#ifndef QCABluetoothFirmware_hpp
#define QCABluetoothFirmware_hpp

#include <IOKit/IOService.h>
#include <IOKit/IOLib.h>
#include <IOKit/IOMessage.h>

#include <IOKit/usb/IOUSBHostDevice.h>
#include <IOKit/usb/IOUSBHostInterface.h>
#include <IOKit/usb/USB.h>

#include <Hci.h>
#include <Firmware.h>

#define BULK_SIZE                   4096

#define QCA_DOWNLOAD                0x01
#define QCA_GET_STATUS              0x05
#define QCA_GET_VERSION             0x09

#define QCA_SYSCFG_UPDATED          0x40
#define QCA_PATCH_UPDATED           0x80

#define QCA_DFU_TIMEOUT             3000
#define QCA_FLAG_MULTI_NVM          0x80

#define QCA_NORMAL_MODE_SET         0x07
#define QCA_SWITCH_VID_PID          0x0a

#define ATH3K_MODE_MASK             0x3F
#define ATH3K_NORMAL_MODE           0x0E

#define ATH3K_FW_HDR_SIZE           20
#define ATH3K_XTAL_FREQ_26M         0x00
#define ATH3K_XTAL_FREQ_40M         0x01
#define ATH3K_XTAL_FREQ_19P2        0x02
#define ATH3K_NAME_LEN              0xFF

#define CONFIG_INDEX                0

#define 

enum SocType
{
    QCA_INVALID = -1,
    QCA_ATH3012,
    
    QCA_ROME_USB,
    QCA_WCN6855,
    
    QCA_AR3002,
    QCA_ROME,
    QCA_WCN3990,
    QCA_WCN3998,
    QCA_WCN3991,
    QCA_QCA6390
};

struct USB_DEVICE
{
    int vendorID;
    int productID;
};

struct USB_DEVICE_ID
{
    USB_DEVICE usbDevice;
    int model;
};

static const USB_DEVICE_ID ID_MODEL[] =
{
    /* Atheros 3012 with sflash firmware */
    { USB_DEVICE{0x0489, 0xe04d}, QCA_ATH3012 },
    { USB_DEVICE{0x0489, 0xe04e}, QCA_ATH3012 },
    { USB_DEVICE{0x0489, 0xe056}, QCA_ATH3012 },
    { USB_DEVICE{0x0489, 0xe057}, QCA_ATH3012 },
    { USB_DEVICE{0x0489, 0xe05f}, QCA_ATH3012 },
    { USB_DEVICE{0x0489, 0xe076}, QCA_ATH3012 },
    { USB_DEVICE{0x0489, 0xe078}, QCA_ATH3012 },
    { USB_DEVICE{0x0489, 0xe095}, QCA_ATH3012 },
    { USB_DEVICE{0x04c5, 0x1330}, QCA_ATH3012 },
    { USB_DEVICE{0x04ca, 0x3004}, QCA_ATH3012 },
    { USB_DEVICE{0x04ca, 0x3005}, QCA_ATH3012 },
    { USB_DEVICE{0x04ca, 0x3006}, QCA_ATH3012 },
    { USB_DEVICE{0x04ca, 0x3007}, QCA_ATH3012 },
    { USB_DEVICE{0x04ca, 0x3008}, QCA_ATH3012 },
    { USB_DEVICE{0x04ca, 0x300b}, QCA_ATH3012 },
    { USB_DEVICE{0x04ca, 0x300d}, QCA_ATH3012 },
    { USB_DEVICE{0x04ca, 0x300f}, QCA_ATH3012 },
    { USB_DEVICE{0x04ca, 0x3010}, QCA_ATH3012 },
    { USB_DEVICE{0x04ca, 0x3014}, QCA_ATH3012 },
    { USB_DEVICE{0x04ca, 0x3018}, QCA_ATH3012 },
    { USB_DEVICE{0x0930, 0x0219}, QCA_ATH3012 },
    { USB_DEVICE{0x0930, 0x021c}, QCA_ATH3012 },
    { USB_DEVICE{0x0930, 0x0220}, QCA_ATH3012 },
    { USB_DEVICE{0x0930, 0x0227}, QCA_ATH3012 },
    { USB_DEVICE{0x0b05, 0x17d0}, QCA_ATH3012 },
    { USB_DEVICE{0x0cf3, 0x0036}, QCA_ATH3012 },
    { USB_DEVICE{0x0cf3, 0x3004}, QCA_ATH3012 },
    { USB_DEVICE{0x0cf3, 0x3008}, QCA_ATH3012 },
    { USB_DEVICE{0x0cf3, 0x311d}, QCA_ATH3012 },
    { USB_DEVICE{0x0cf3, 0x311e}, QCA_ATH3012 },
    { USB_DEVICE{0x0cf3, 0x311f}, QCA_ATH3012 },
    { USB_DEVICE{0x0cf3, 0x3121}, QCA_ATH3012 },
    { USB_DEVICE{0x0cf3, 0x817a}, QCA_ATH3012 },
    { USB_DEVICE{0x0cf3, 0x817b}, QCA_ATH3012 },
    { USB_DEVICE{0x0cf3, 0xe003}, QCA_ATH3012 },
    { USB_DEVICE{0x0cf3, 0xe004}, QCA_ATH3012 },
    { USB_DEVICE{0x0cf3, 0xe005}, QCA_ATH3012 },
    { USB_DEVICE{0x0cf3, 0xe006}, QCA_ATH3012 },
    { USB_DEVICE{0x13d3, 0x3362}, QCA_ATH3012 },
    { USB_DEVICE{0x13d3, 0x3375}, QCA_ATH3012 },
    { USB_DEVICE{0x13d3, 0x3393}, QCA_ATH3012 },
    { USB_DEVICE{0x13d3, 0x3395}, QCA_ATH3012 },
    { USB_DEVICE{0x13d3, 0x3402}, QCA_ATH3012 },
    { USB_DEVICE{0x13d3, 0x3408}, QCA_ATH3012 },
    { USB_DEVICE{0x13d3, 0x3423}, QCA_ATH3012 },
    { USB_DEVICE{0x13d3, 0x3432}, QCA_ATH3012 },
    { USB_DEVICE{0x13d3, 0x3472}, QCA_ATH3012 },
    { USB_DEVICE{0x13d3, 0x3474}, QCA_ATH3012 },
    { USB_DEVICE{0x13d3, 0x3487}, QCA_ATH3012 },
    { USB_DEVICE{0x13d3, 0x3490}, QCA_ATH3012 },

    /* Atheros AR5BBU12 with sflash firmware */
    { USB_DEVICE{0x0489, 0xe036}, QCA_ATH3012 },
    { USB_DEVICE{0x0489, 0xe03c}, QCA_ATH3012 },
    
    /* QCA ROME chipset */
    { USB_DEVICE{0x0cf3, 0x535b}, QCA_ROME_USB },
    { USB_DEVICE{0x0cf3, 0xe007}, QCA_ROME_USB },
    { USB_DEVICE{0x0cf3, 0xe009}, QCA_ROME_USB },
    { USB_DEVICE{0x0cf3, 0xe010}, QCA_ROME_USB },
    { USB_DEVICE{0x0cf3, 0xe300}, QCA_ROME_USB },
    { USB_DEVICE{0x0cf3, 0xe301}, QCA_ROME_USB },
    { USB_DEVICE{0x0cf3, 0xe360}, QCA_ROME_USB },
    { USB_DEVICE{0x0489, 0xe092}, QCA_ROME_USB },
    { USB_DEVICE{0x0489, 0xe09f}, QCA_ROME_USB },
    { USB_DEVICE{0x0489, 0xe0a2}, QCA_ROME_USB },
    { USB_DEVICE{0x04ca, 0x3011}, QCA_ROME_USB },
    { USB_DEVICE{0x04ca, 0x3015}, QCA_ROME_USB },
    { USB_DEVICE{0x04ca, 0x3016}, QCA_ROME_USB },
    { USB_DEVICE{0x04ca, 0x301a}, QCA_ROME_USB },
    { USB_DEVICE{0x04ca, 0x3021}, QCA_ROME_USB },
    { USB_DEVICE{0x13d3, 0x3491}, QCA_ROME_USB },
    { USB_DEVICE{0x13d3, 0x3496}, QCA_ROME_USB },
    { USB_DEVICE{0x13d3, 0x3501}, QCA_ROME_USB },

    /* QCA WCN6855 chipset */
    { USB_DEVICE{0x0cf3, 0xe600}, QCA_WCN6855 },
    
    /* QCA QCA6390 chipset */
    { USB_DEVICE{0x0cf3, 0x6390}, QCA_QCA6390} //not sure
};

struct Ath3KVersion
{
    SInt32                  ramVersion;
    SInt32                  romVersion;
    SInt32                  buildVersion;
    UInt8                   refClock;
} __packed;

struct QCAVersion
{
    SInt32    ramVersion;
    SInt32    romVersion;
    SInt32    patchVersion;
    SInt16    boardId;
    SInt16    flag;
} __packed;

struct QCARamPatchVersion
{
    SInt16    romVersionHigh;
    SInt16    romVersionLow;
    SInt16    patchVersion;
} __packed;

struct QCADeviceInfo
{
    u32       romVersion;
    u8        ramPatchHdr;      /* length of header in rampatch */
    u8        nvmHdr;           /* length of header in NVM */
    u8        versionOffset;    /* offset of version structure in rampatch */
};

struct EdlEventHdr
{
    UInt8       cresp;
    UInt8       rtype;
    UInt8       data[];
} __packed;

struct TlvPatch
{
    SInt32      total_size;
    SInt32      data_length;
    UInt8       format_version;
    UInt8       signature;
    UInt8       download_mode;
    SInt16      product_id;
    SInt16      rom_build;
    SInt16      patch_version;
    SInt32      entry;
} __packed;

struct TlvNvm
{
    SInt16      tag_id;
    SInt16      tag_len;
    UInt8       data[];
} __packed;

struct TlvHdr
{
    SInt32      type_len;
    UInt8       data[];
} __packed;

struct QCASoCVersion
{
    SInt32      product_id;
    SInt32      patch_ver;
    SInt16      rom_ver;
    SInt32      soc_id;
} __packed;

enum BaudRates
{
    QCA_BAUDRATE_115200     = 0,
    QCA_BAUDRATE_57600,
    QCA_BAUDRATE_38400,
    QCA_BAUDRATE_19200,
    QCA_BAUDRATE_9600,
    QCA_BAUDRATE_230400,
    QCA_BAUDRATE_250000,
    QCA_BAUDRATE_460800,
    QCA_BAUDRATE_500000,
    QCA_BAUDRATE_720000,
    QCA_BAUDRATE_921600,
    QCA_BAUDRATE_1000000,
    QCA_BAUDRATE_1250000,
    QCA_BAUDRATE_2000000,
    QCA_BAUDRATE_3000000,
    QCA_BAUDRATE_4000000,
    QCA_BAUDRATE_1600000,
    QCA_BAUDRATE_3200000,
    QCA_BAUDRATE_3500000,
    QCA_BAUDRATE_AUTO       = 0xFE
};

enum TlvDnldMode
{
    QCA_SKIP_EVT_NONE,
    QCA_SKIP_EVT_VSE,
    QCA_SKIP_EVT_CC,
    QCA_SKIP_EVT_VSE_CC
};

enum TlvType
{
    TLV_TYPE_INVALID = 0,
    TLV_TYPE_PATCH,
    TLV_TYPE_NVM
};

struct bdaddr_t
{
    uint8_t b[6];
} __packed;

class QCABluetoothFirmware : public IOService
{
    OSDeclareDefaultStructors(QCABluetoothFirmware)
    
public:
    virtual IOService *     probe(
                                IOService       *   provider,
                                SInt32          *   score                   ) override;
    virtual bool            start(
                                IOService       *   provider                ) override = 0;
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
    bool                    isAth3K();
    bool                    isQcaUsb();
    bool                    isQcaSoc();
    void                    releaseAll();
    IOReturn                sendVendorRequestIn(u8 bRequest, void * dataBuffer, UInt16 size);
    IOReturn                sendVendorRequestOut(u8 bRequest, void * dataBuffer, UInt16 size);
    IOReturn                sendHCIRequest(u16 opCode, u8 paramLen, const void * param);
    bool                    resetDevice();
    void                    powerStart( IOService * provider );
    bool                    initUSBConfiguration();
    bool                    initInterface();
    bool                    loadFirmware(OSData * fwData, size_t headerSize);
    
public:
    IOUSBHostDevice             *       m_pUSBDevice;
    IOUSBHostInterface          *       m_pInterface;
    IOUSBHostPipe               *       m_pBulkWritePipe;

    int                                 m_socType;
    
#if isAth3K() || isQcaUSB()
    unsigned char               *       m_fwState;
#endif

#if isAth3K()
    Ath3KVersion                *       m_fwVersion;
    USBStatus                   *       m_devStatus;
#endif
    
#if isQcaUSB()
    QCAVersion                  *       m_fwVersion;
    QCARamPatchVersion          *       m_rpVersion; //used for getting ram patch name
    QCADeviceInfo               *       m_devInfo; //used for getting USB firmware name
#endif
    
#if isQcaSoc()
    QCASoCVersion               *       m_fwVersion;
#endif
    
protected:
    OSData                      *       m_fwData;
    HciCommandHdr               *       m_hciCommand;
    
#if isAth3K()
    char                                m_fwFilename[ATH3K_NAME_LEN];
#endif
    
#if isQcaUsb() || isQcaSoc()
    char                                m_fwFilename[64];
#endif
    
# isQcaSoc()
    int                                 m_tlvType;
    u8                                  m_bdRate;
    s32                                 m_dnldMode;
#endif
};


#endif /* QCABluetoothFirmware_hpp */
