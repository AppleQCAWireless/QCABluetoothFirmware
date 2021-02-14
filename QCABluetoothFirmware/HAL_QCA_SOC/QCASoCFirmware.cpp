//
//  QCASoCFirmware.cpp
//  QCABluetoothFirmware
//
//  Created by Charlie Jiang on 2/11/21.
//  Copyright © 2021 Charlie Jiang. All rights reserved.
//

#include "QCASoCFirmware.hpp"

#define super QCABluetoothFirmware
OSDefineMetaClassAndStructors(QCASoCFirmware, QCABluetoothFirmware)

bool QCASoCFirmware::start(IOService * provider)
{
    if (!isQcaSoc())
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
    
    if (!restDevice() || m_pUSBDevice->setConfiguration(0))
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
    
    getSoCVersion();

    //getBaudRate();
    //m_bdRate = baudrate;

    if (!loadRamPatch())
    {
        return false;
    }

    if (!loadNVM())
    {
        return false;
    }

    if (m_socType >= QCA_WCN3991 && !disableSoCLogging())
    {
        return false;
    }

    /* Perform HCI reset */
    if (!resetDevice())
    {
        return false;
    }

    //setBluetoothAddress...
    
    return true;
}

bool QCASoCFirmware::getSoCVersion()
{
    EdlEventHdr   * edl;
    
    u8 event_type = HCI_EV_VENDOR;
    u8 rlen = sizeof( EdlEventHdr ) + sizeof( QCASoCVersion );
    u8 rtype = EDL_APP_VER_RES_EVT;

    /* Unlike other SoC's sending version command response as payload to
     * VSE event. WCN3991 sends version command response as a payload to
     * command complete event.
    */
    if (m_socType >= QCA_WCN3991)
    {
        event_type = 0;
        ++rlen;
        rtype = EDL_PATCH_VER_REQ_CMD;
    }

    if (sendHCIRequest(EDL_PATCH_CMD_OPCODE, EDL_PATCH_CMD_LEN, (void *) EDL_PATCH_VER_REQ_CMD))
    {
        ErrorLog("Failed to read version!!!\n");
        return false;
    }

    if (m_hciCommand->plen != rlen)
    {
        ErrorLog("Version size mismatch (len: %d)!!!\n", m_hciCommand->plen);
        return false;
    }

    edl = (EdlEventHdr *) m_hciCommand->pData;
    
    if (!edl)
    {
        ErrorLog("TLV has no header!!!\n");
        return false;
    }

    if (edl->cresp != EDL_CMD_REQ_RES_EVT || edl->rtype != rtype)
    {
        ErrorLog("Received wrong packet (cresp: %d, rtype: %d)!!!\n", edl->cresp, edl->rtype);
        return false;
    }

    if (m_socType >= QCA_WCN3991)
    {
        memcpy(m_fwVersion, edl->data + 1, sizeof( QCASoCVersion ));
    }
    else
    {
        memcpy(m_fwVersion, &edl->data, sizeof( QCASoCVersion ));
    }

    InfoLog("Product ID:        0x%08x", le32_to_cpu(m_fwVersion->product_id));
    InfoLog("SOC Version:       0x%08x", le32_to_cpu(m_fwVersion->soc_id));
    InfoLog("ROM Version:       0x%08x", le32_to_cpu(m_fwVersion->rom_ver));
    InfoLog("Patch Version:     0x%08x", le32_to_cpu(m_fwVersion->patch_ver));

    if (m_fwVersion->soc_id == 0 || m_fwVersion->rom_ver == 0)
    {
        ErrorLog("Failed to get SoC version!!!\n");
        return false;
    }
    
    return true;
}

bool QCASoCFirmware::sendPreShutdownCommand()
{
    FuncLog("sendPreShutdownCommand");
    
    if (sendHCIRequest(QCA_PRE_SHUTDOWN_CMD, 0, NULL))
    {
        ErrorLog("Failed to send pre-shutdown command!!!\n");
        return false;
    }

    return true;
}

void QCASoCFirmware::checkTLVData(OSData * fwData)
{
    u16 tag_id, tag_len;

    TlvHdr * tlv = (TlvHdr *) fwData;

    u32 type_len = le32_to_cpu(tlv->type_len);
    int length = (type_len >> 8) & 0x00ffffff;

    InfoLog("TLV Type:                      0x%x",      type_len & 0x000000ff);
    InfoLog("Length:                        %d bytes",  length);

    m_dnldMode = QCA_SKIP_EVT_NONE;

    switch (m_tlvType)
    {
        case TLV_TYPE_PATCH:
        {
            TlvPatch * tlv_patch = (TlvPatch *) tlv->data;

            /* For Rome version 1.1 to 3.1, all segment commands
             * are acked by a vendor specific event (VSE).
             * For Rome >= 3.2, the download mode field indicates
             * if VSE is skipped by the controller.
             * In case VSE is skipped, only the last segment is acked.
             */
            
            m_dnldMode = tlv_patch->download_mode;

            InfoLog("Total Length:                  %d bytes",      le32_to_cpu(tlv_patch->total_size));
            InfoLog("Patch Data Length:             %d bytes",      le32_to_cpu(tlv_patch->data_length));
            InfoLog("Signing Format Version:        0x%x",          le32_to_cpu(tlv_patch->format_version));
            InfoLog("Signature Algorithm:           0x%x",          tlv_patch->signature);
            InfoLog("Download mode:                 0x%x",          tlv_patch->download_mode);
            InfoLog("Product ID:                    0x%04x",        le16_to_cpu(tlv_patch->product_id));
            InfoLog("Rom Build Version:             0x%04x",        le16_to_cpu(tlv_patch->rom_build));
            InfoLog("Patch Version:                 0x%04x",        le16_to_cpu(tlv_patch->patch_version));
            InfoLog("Patch Entry Address:           0x%x",          le32_to_cpu(tlv_patch->entry));
            break;
        }

        case TLV_TYPE_NVM:
        {
            TlvNvm * tlv_nvm;
            int i = 0;
            while (i < length) {
                tlv_nvm = (TlvNvm *) (tlv->data + i);

                tag_id  = le16_to_cpu(tlv_nvm->tag_id);
                tag_len = le16_to_cpu(tlv_nvm->tag_len);

                /* Update NVM tags as needed */
                switch (tag_id)
                {
                    case EDL_TAG_ID_HCI:
                    {
                        /* HCI transport layer parameters
                         * enabling software inband sleep
                         * onto controller side.
                         */
                        tlv_nvm->data[0] |= 0x80;

                        /* UART Baud Rate */
                        if (m_socType >= QCA_WCN3991)
                            tlv_nvm->data[1] = m_bdRate;
                        else
                            tlv_nvm->data[2] = m_bdRate;

                        break;
                    }
                    case EDL_TAG_ID_DEEP_SLEEP:
                    {
                        /* Sleep enable mask
                         * enabling deep sleep feature on controller.
                         */
                        tlv_nvm->data[0] |= 0x01;

                        break;
                    }
                    default:
                    {
                        ErrorLog("Tag ID invalid!!!\n");
                        break;
                    }
                }

                i += (sizeof(u16) + sizeof(u16) + 8 + tag_len);
            }
            break;
        }

        default:
        {
            ErrorLog("Unknown TLV type (%d)!!!", m_tlvType);
            break;
        }
    }
}

bool QCASoCFirmware::sendTLVSegment(int seg_size, const u8 * data)
{
    EdlEventHdr *edl;
    
    u8 * tlv_resp;
    u8 cmd[MAX_SIZE_PER_TLV_SEGMENT + 2];
    u8 event_type = HCI_EV_VENDOR;
    u8 rlen = sizeof( EdlEventHdr ) + sizeof( u8 );
    u8 rtype = EDL_TVL_DNLD_RES_EVT;

    cmd[0] = EDL_PATCH_TLV_REQ_CMD;
    cmd[1] = seg_size;
    memcpy(cmd + 2, data, seg_size);

    if (m_dnldMode == QCA_SKIP_EVT_VSE_CC || m_dnldMode == QCA_SKIP_EVT_VSE)
    {
        return sendHCIRequest(EDL_PATCH_CMD_OPCODE, seg_size + 2, cmd);
    }

    /* Unlike other SoC's sending version command response as payload to
     * VSE event. WCN3991 sends version command response as a payload to
     * command complete event.
     */
    if (m_socType >= QCA_WCN3991)
    {
        event_type = 0;
        rlen = sizeof( EdlEventHdr );
        rtype = EDL_PATCH_TLV_REQ_CMD;
    }

    if (sendHCIRequest(EDL_PATCH_CMD_OPCODE, seg_size + 2, cmd))
    {
        ErrorLog("Failed to send TLV segment!!!\n");
        return false;
    }

    if (m_hciCommand->plen != rlen)
    {
        ErrorLog("TLV response size mismatch!!!\n");
        return false;
    }

    edl = (EdlEventHdr *) m_hciCommand->pData;
    if (!edl)
    {
        ErrorLog("TLV has no header!!!\n");
        return false;
    }

    if (edl->cresp != EDL_CMD_REQ_RES_EVT || edl->rtype != rtype)
    {
        ErrorLog("TLV with error (stat: 0x%x, rtype: 0x%x)!!!", edl->cresp, edl->rtype);
        return false;
    }

    if (m_socType >= QCA_WCN3991)
    {
        return true;
    }

    tlv_resp = (u8 *) edl->data;
    if (* tlv_resp)
    {
        ErrorLog("TLV with error (stat: 0x%x, rtype: 0x%x, tlv_resp: 0x%x)!!!", edl->cresp, edl->rtype, * tlv_resp);
    }
    return true;
}

bool QCASoCFirmware::disableSoCLogging()
{
    u8 cmd[2];

    cmd[0] = QCA_DISABLE_LOGGING_SUB_OP;
    cmd[1] = 0x00;
    
    if (sendHCIRequest(QCA_DISABLE_LOGGING, sizeof(cmd), cmd))
    {
        ErrorLog("Failed to disable SoC logging!!!\n");
        return false;
    }

    return true;
}

void QCASoCFirmware::getRamPatchName()
{
    m_tlvType = TLV_TYPE_PATCH;
    u8 romVersion = ((GET_SOC_VERSION(m_fwVersion->soc_id, m_fwVersion->rom_ver) & 0x00000f00) >> 0x04) | (GET_SOC_VERSION(m_fwVersion->soc_id, m_fwVersion->rom_ver) & 0x0000000f);
    
    if (m_fwVersion->soc_id == QCA_WCN3990 || m_fwVersion->soc_id == QCA_WCN3998 || m_fwVersion->soc_id == QCA_WCN3991)
    {
        snprintf(m_fwFilename, sizeof(m_fwFilename), "crbtfw%02x.tlv", romVersion);
    }
    else if (m_fwVersion->soc_id == QCA_QCA6390)
    {
        snprintf(m_fwFilename, sizeof(m_fwFilename), "htbtfw%02x.tlv", romVersion);
    }
    else
    {
        snprintf(m_fwFilename, sizeof(m_fwFilename), "rampatch_%08x.bin", GET_SOC_VERSION(m_fwVersion->soc_id, m_fwVersion->rom_ver));
    }
}

void QCASoCFirmware::getNVMName()
{
    m_tlvType = TLV_TYPE_NVM;
    if (m_fwVersion->soc_id == QCA_WCN3991)
    {
         snprintf(m_fwFilename, sizeof(m_fwFilename), "crnv%02xu.bin", le32_to_cpu(m_fwVersion->rom_ver));
    }
    else if (m_fwVersion->soc_id == QCA_WCN3990 || m_fwVersion->soc_id == QCA_WCN3998)
    {
            snprintf(m_fwFilename, sizeof(m_fwFilename), "crnv%02x.bin", le32_to_cpu(m_fwVersion->rom_ver));
    }
    else if (m_fwVersion->soc_id == QCA_QCA6390)
    {
        snprintf(m_fwFilename, sizeof(m_fwFilename), "htnv%02x.bin", le32_to_cpu(m_fwVersion->rom_ver));
    }
    else
    {
        snprintf(m_fwFilename, sizeof(m_fwFilename), "nvm_%08x.bin", GET_SOC_VERSION(m_fwVersion->soc_id, m_fwVersion->rom_ver));
    }
}

bool QCASoCFirmware::loadSoCFirmware(OSData * fwData)
{
    InfoLog("Downloading firmware %s...", m_fwFilename);

    checkTLVData(fwData);

    u8 * segment = (u8 *) fwData->getBytesNoCopy();
    int remain = fwData->getLength();
    int segsize, i = 0;
    
    while (remain > 0)
    {
        segsize = min(MAX_SIZE_PER_TLV_SEGMENT, remain);

        InfoLog("Sending segment %d (size: %d)...", ++i, segsize);

        remain -= segsize;
        
        /* The last segment is always acked regardless download mode */
        if (!remain || segsize < MAX_SIZE_PER_TLV_SEGMENT)
        {
            m_dnldMode = QCA_SKIP_EVT_NONE;
        }

        if (!sendTLVSegment(segsize, segment))
        {
            OSSafeReleaseNULL(fwData);
            return false;
        }

        segment += segsize;
    }

    /* Latest qualcomm chipsets are not sending a command complete event
     * for every fw packet sent. They only respond with a vendor specific
     * event for the last packet. This optimization in the chip will
     * decrease the BT in initialization time. Here we will inject a command
     * complete event to avoid a command timeout error message.
     */
    if (m_dnldMode == QCA_SKIP_EVT_VSE_CC || m_dnldMode == QCA_SKIP_EVT_VSE)
    {
        OSSafeReleaseNULL(fwData);
        //Inject HCI_EV_CMD_COMPLETE
        return true;
    }

    OSSafeReleaseNULL(fwData);
    return true;
}

bool QCASoCFirmware::loadRamPatch()
{
    getRamPatchName();
    
    m_fwData = getFwDescByName(m_fwFilename);
    
    if (!m_fwData)
    {
        ErrorLog("(loadRamPatch) Failed to request ram patch file: %s!!!\n", m_fwFilename);
        return false;
    }

    if (!loadSoCFirmware(m_fwData))
    {
        ErrorLog("Failed to load ram patch file!!!\n");
        return false;
    }
    return true;
}

bool QCASoCFirmware::loadNVM()
{
    getNVMName();

    m_fwData = getFwDescByName(m_fwFilename);
    
    if (!m_fwData)
    {
        ErrorLog("(loadNvm) Failed to request NVM file: %s!!!\n", m_fwFilename);
        return false;
    }
    
    if (!loadSoCFirmware(m_fwData))
    {
        ErrorLog("Failed to load NVM file!!!\n");
        return false;
    }
    return true;
}

IOReturn QCASoCFirmware::setBluetoothDeviceAddressROME(bdaddr_t bdaddr)
{
    u8 cmd[9];

    cmd[0] = EDL_NVM_ACCESS_SET_REQ_CMD;
    cmd[1] = 0x02;                  /* TAG ID */
    cmd[2] = sizeof(bdaddr_t);      /*  size  */
    
    memcpy(cmd + 3, bdaddr, sizeof(bdaddr_t));
    
    return sendHCIRequest(EDL_NVM_ACCESS_OPCODE, sizeof(cmd), cmd);
}

inline IOReturn QCASoCFirmware::setBluetoothDeviceAddress(const bdaddr_t *bdaddr)
{
    return sendHCIRequest(EDL_WRITE_BD_ADDR_OPCODE, 6, bdaddr);
}
