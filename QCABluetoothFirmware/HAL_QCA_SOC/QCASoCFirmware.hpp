//
//  QCASoCFirmware.hpp
//  QCABluetoothFirmware
//
//  Created by Charlie Jiang on 2/11/21.
//  Copyright © 2021 Charlie Jiang. All rights reserved.
//

#ifndef QCASoCFirmware_hpp
#define QCASoCFirmware_hpp

#include "QCABluetoothFirmware.hpp"

#define GET_SOC_VERSION(socID, romVersion)  (le32_to_cpu(socID) << 16) | (le16_to_cpu(romVersion))

#define EDL_PATCH_CMD_OPCODE            0xFC00
#define EDL_NVM_ACCESS_OPCODE           0xFC0B
#define EDL_WRITE_BD_ADDR_OPCODE        0xFC14
#define EDL_PATCH_CMD_LEN               1
#define EDL_PATCH_VER_REQ_CMD           0x19
#define EDL_PATCH_TLV_REQ_CMD           0x1E
#define EDL_NVM_ACCESS_SET_REQ_CMD      0x01
#define MAX_SIZE_PER_TLV_SEGMENT        243
#define QCA_PRE_SHUTDOWN_CMD            0xFC08
#define QCA_DISABLE_LOGGING             0xFC17

#define EDL_CMD_REQ_RES_EVT             0x00
#define EDL_PATCH_VER_RES_EVT           0x19
#define EDL_APP_VER_RES_EVT             0x02
#define EDL_TVL_DNLD_RES_EVT            0x04
#define EDL_CMD_EXE_STATUS_EVT          0x00
#define EDL_SET_BAUDRATE_RSP_EVT        0x92
#define EDL_NVM_ACCESS_CODE_EVT         0x0B
#define QCA_DISABLE_LOGGING_SUB_OP      0x14

#define EDL_TAG_ID_HCI                  17
#define EDL_TAG_ID_DEEP_SLEEP           27

#define QCA_WCN3990_POWERON_PULSE       0xFC
#define QCA_WCN3990_POWEROFF_PULSE      0xC0

#define QCA_WCN3991_SOC_ID              0x40014320

#define QCA_FW_BUILD_VER_LEN            255

static inline uint8_t getBaudRateValue(int speed)
{
    switch (speed)
    {
        case 9600:
            return QCA_BAUDRATE_9600;
        case 19200:
            return QCA_BAUDRATE_19200;
        case 38400:
            return QCA_BAUDRATE_38400;
        case 57600:
            return QCA_BAUDRATE_57600;
        case 115200:
            return QCA_BAUDRATE_115200;
        case 230400:
            return QCA_BAUDRATE_230400;
        case 460800:
            return QCA_BAUDRATE_460800;
        case 500000:
            return QCA_BAUDRATE_500000;
        case 921600:
            return QCA_BAUDRATE_921600;
        case 1000000:
            return QCA_BAUDRATE_1000000;
        case 2000000:
            return QCA_BAUDRATE_2000000;
        case 3000000:
            return QCA_BAUDRATE_3000000;
        case 3200000:
            return QCA_BAUDRATE_3200000;
        case 3500000:
            return QCA_BAUDRATE_3500000;
        default:
            return QCA_BAUDRATE_115200;
    }
}

class QCASoCFirmware : public QCABluetoothFirmware
{
    OSDeclareDefaultStructors(QCASoCFirmware)
    
public:
    virtual bool start( IOService * provider ) override;
    
private:
    bool sendPreShutdownCommand();
    void checkTLVData(OSData * fwData);
    bool sendTLVSegment(int seg_size, const u8 *data);
    bool disableSoCLogging();
    bool getSoCVersion();
    bool loadSoCFirmware(OSData * fwData);
    void getRamPatchName();
    void getNVMName();
    bool loadRamPatch();
    bool loadNVM();
    IOReturn setBluetoothDeviceAddressROME(bdaddr_t bdaddr);
    IOReturn setBluetoothDeviceAddress(bdaddr_t bdaddr);
};
#endif /* QCASoCFirmware_hpp */
