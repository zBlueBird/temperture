#include <string.h>
#include <gatt.h>
#include <bt_types.h>
#include "trace.h"
#include "gap_le.h"
#include "gap_conn_le.h"
#include "gap_msg.h"
#include "app_msg.h"
#include "flash_device.h"
#include "rtl876x_wdg.h"
#include "ota_service.h"
#include "dfu_service.h"
#include "silent_dfu_flash.h"
#include "dfu_api.h"
#include "patch_header_check.h"
#include "board.h"
#include "flash_adv_cfg.h"
#include "os_sched.h"
#include "mem_config.h"
#include "otp.h"
#include "rtl876x_hw_aes.h"
#include "patch_header_check.h"
#include "os_sched.h"
#include "app_section.h"
#include "os_sync.h"
#include "platform_utils.h"
#include "hw_aes.h"

#if SUPPORT_SILENT_OTA
#define CAL_OFFSET(type, member) ((size_t)(&((type *)0)->member))

extern  uint8_t g_dfu_service_id;

uint8_t ota_temp_buffer_head[DFU_TEMP_BUFFER_SIZE];
uint8_t *p_ota_temp_buffer_head;
uint16_t g_ota_tmp_buf_used_size;
uint16_t mBufSize;
uint16_t mCrcVal;
T_DFU_CB g_dfu_para;
bool g_sil_buf_check_en = false;
uint32_t g_sil_dfu_resend_offset = 0;
P_FUN_SERVER_GENERAL_CB pfn_dfu_extended_cb = NULL;

P_FUN_DFU_OPCODE_CB dfu_service_handle_control_point_req_cb __attribute__((weak)) = NULL;


const uint8_t   SILENCE_GATT_UUID128_DFU_SERVICE[16] = {0x12, 0xA2, 0x4D, 0x2E, 0xFE, 0x14, 0x48, 0x8e, 0x93, 0xD2, 0x17, 0x3C, 0x87, 0x62, 0x00, 0x00};

T_ATTRIB_APPL silence_dfu_service[] =
{

    /*-------------------------- DFU Service ---------------------------*/
    /* <<Primary Service>>, .. */
    {
        (ATTRIB_FLAG_VOID | ATTRIB_FLAG_LE),           /* flags     */
        {
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),              /* type_value */
        },
        UUID_128BIT_SIZE,                                    /* bValueLen     */
        (void *)SILENCE_GATT_UUID128_DFU_SERVICE,           /* p_value_context */
        GATT_PERM_READ                                      /* permissions  */
    },



    /* <<Characteristic>>, .. */
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* flags */
        {                                           /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_WRITE_NO_RSP/* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* permissions */
    },
    /*--- DFU packet characteristic value ---*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,                              /* flags */
        {                                                           /* type_value */
            GATT_UUID128_DFU_PACKET
        },
        0,                                                 /* bValueLen */
        NULL,
#if DFU_SERVER_REQUIRE_AUTH
        GATT_PERM_WRITE_AUTHEN_MITM_REQ                             /* permissions */
#else
        GATT_PERM_WRITE           /* permissions */
#endif
    },
    /* <<Characteristic>>, .. */
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* flags */
        {                                           /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            (GATT_CHAR_PROP_WRITE |                   /* characteristic properties */
             GATT_CHAR_PROP_NOTIFY)
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* permissions */
    },
    /*--- DFU Control Point value ---*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,                              /* flags */
        {                                                           /* type_value */
            GATT_UUID128_DFU_CONTROL_POINT
        },
        0,                                                 /* bValueLen */
        NULL,
#if DFU_SERVER_REQUIRE_AUTH
        GATT_PERM_WRITE_AUTHEN_MITM_REQ                             /* permissions */
#else
        GATT_PERM_WRITE           /* permissions */
#endif
    },
    /* client characteristic configuration */
    {
        (ATTRIB_FLAG_VALUE_INCL |                   /* flags */
         ATTRIB_FLAG_CCCD_APPL),
        {                                           /* type_value */
            LO_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            HI_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            /* NOTE: this value has an instantiation for each client, a write to */
            /* this attribute does not modify this default value:                */
            LO_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT), /* client char. config. bit field */
            HI_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT)
        },
        2,                                          /* bValueLen */
        NULL,
#if DFU_SERVER_REQUIRE_AUTH
        (GATT_PERM_READ | GATT_PERM_WRITE_AUTHEN_MITM_REQ)          /* permissions */
#else
        (GATT_PERM_READ | GATT_PERM_WRITE)          /* permissions */
#endif
    }

};


/**
 * @brief silence_BufferCheckProc
 *
 * @param buffer_size     size for buffer check.
 * @param crc            calced buffer crc value.
 * @return None
*/
void silence_BufferCheckProc(uint8_t conn_id, uint16_t buffer_size, uint16_t crc)
{
    uint32_t signal;//os lock signal
    uint16_t offset = 0;
    uint8_t notif_data[DFU_NOTIFY_LENGTH_REPORT_BUFFER_CRC] = {0};
    notif_data[0] = DFU_OPCODE_NOTIFICATION;
    notif_data[1] = DFU_OPCODE_REPORT_BUFFER_CRC;

    if (mBufSize > DFU_TEMP_BUFFER_SIZE)
    {
        //invalid para
        g_ota_tmp_buf_used_size = 0;
        notif_data[2] = DFU_ARV_FAIL_INVALID_PARAMETER;
        LE_UINT32_TO_ARRAY(&notif_data[3], g_dfu_para.cur_offset);
        server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                         notif_data, DFU_NOTIFY_LENGTH_REPORT_BUFFER_CRC, GATT_PDU_TYPE_NOTIFICATION);
        return;
    }

    if (g_ota_tmp_buf_used_size == mBufSize ||
        g_dfu_para.cur_offset + g_ota_tmp_buf_used_size == g_dfu_para.image_total_length
       )
    {
        if (dfu_checkbufcrc(p_ota_temp_buffer_head, g_ota_tmp_buf_used_size, crc))     //crc error

        {
            g_ota_tmp_buf_used_size = 0;
            notif_data[2] = DFU_ARV_FAIL_CRC_ERROR;
            LE_UINT32_TO_ARRAY(&notif_data[3], g_dfu_para.cur_offset);
            server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                             notif_data, DFU_NOTIFY_LENGTH_REPORT_BUFFER_CRC, GATT_PDU_TYPE_NOTIFICATION);
            return;
        }

        else //crc ok
        {
            if (OTP->ota_with_encryption_data)
            {
                //aes
                do
                {
                    if ((g_ota_tmp_buf_used_size - offset) >= 16)
                    {
#if defined ( __ICCARM__   )
#pragma diag_suppress=Pa039
#endif
                        hw_aes_init(OTP->aes_key, NULL, AES_MODE_ECB, OTP->ota_with_encryption_use_aes256);
#if defined ( __ICCARM__   )
#pragma diag_warning=Pa039
#endif
                        hw_aes_decrypt_16byte(p_ota_temp_buffer_head + offset, p_ota_temp_buffer_head + offset);
                        offset += 16;
                    }
                    else
                    {
                        break;
                    }
                }
                while (1);
            }
            unlock_flash_all();
            DFU_PRINT_TRACE1("g_dfu_para.cur_offset=%d", g_dfu_para.cur_offset);

            uint32_t result = sil_dfu_update(g_dfu_para.signature, g_dfu_para.cur_offset,
                                             g_ota_tmp_buf_used_size,
                                             (uint32_t *)p_ota_temp_buffer_head);

            if (result == 0)
            {
                g_dfu_para.cur_offset += g_ota_tmp_buf_used_size;

                if ((g_dfu_para.cur_offset - g_sil_dfu_resend_offset) >= FMC_SEC_SECTION_LEN)
                {
                    g_sil_dfu_resend_offset += FMC_SEC_SECTION_LEN;
                }
                g_ota_tmp_buf_used_size = 0;
                notif_data[2] = DFU_ARV_SUCCESS; //valid
                LE_UINT32_TO_ARRAY(&notif_data[3], g_dfu_para.cur_offset);
                server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                                 notif_data, DFU_NOTIFY_LENGTH_REPORT_BUFFER_CRC, GATT_PDU_TYPE_NOTIFICATION);
                lock_flash();
                return;
            }
            else
            {
                uint32_t ret = 0;
                uint32_t cnt = 0;
                do
                {
                    if (g_dfu_para.signature == SIGNATURE_APP_DEFINE)
                    {
                        flash_erase_locked(FLASH_ERASE_SECTOR, APP_DEFINE_DATA_ADDR + g_sil_dfu_resend_offset);
                        ret = dfu_flash_check_appdefine_blank(APP_DEFINE_DATA_ADDR, g_sil_dfu_resend_offset,
                                                              FMC_SEC_SECTION_LEN);
                    }
                    else
                    {
                        dfu_flash_erase(g_dfu_para.signature, g_sil_dfu_resend_offset);
                        signal = os_lock();
                        ret = dfu_flash_check_blank(g_dfu_para.signature, g_sil_dfu_resend_offset, FMC_SEC_SECTION_LEN);
                        os_unlock(signal);
                    }

                    if (ret)
                    {
                        cnt++;
                    }
                    else
                    {
                        break;
                    }
                    if (cnt >= 3)     //check 0xff failed,erase failed
                    {
                        //erase error
                        g_ota_tmp_buf_used_size = 0;
                        g_dfu_para.cur_offset =  g_sil_dfu_resend_offset;
                        notif_data[2] = DFU_ARV_FAIL_ERASE_ERROR;
                        LE_UINT32_TO_ARRAY(&notif_data[3], g_dfu_para.cur_offset);
                        server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                                         notif_data, DFU_NOTIFY_LENGTH_REPORT_BUFFER_CRC, GATT_PDU_TYPE_NOTIFICATION);
                        lock_flash();
                        return;
                    }
                }
                while (1);

                if ((g_dfu_para.cur_offset - g_sil_dfu_resend_offset) > FMC_SEC_SECTION_LEN) //need erase two sector
                {
                    cnt = 0;
                    do
                    {
                        //erase sector :addr ~ gDfuResendOffset+FMC_SEC_SECTION_LEN;
                        sil_dfu_flash_erase(g_dfu_para.signature, g_sil_dfu_resend_offset + FMC_SEC_SECTION_LEN);
                        signal = os_lock();
                        ret = dfu_flash_check_blank(g_dfu_para.signature, g_sil_dfu_resend_offset + FMC_SEC_SECTION_LEN,
                                                    FMC_SEC_SECTION_LEN);
                        os_unlock(signal);
                        if (ret)
                        {
                            cnt++;
                        }
                        else
                        {
                            break;
                        }
                        if (cnt >= 3)     //check 0xff failed,erase failed
                        {
                            //erase error
                            g_ota_tmp_buf_used_size = 0;
                            g_dfu_para.cur_offset =  g_sil_dfu_resend_offset;
                            notif_data[2] = DFU_ARV_FAIL_ERASE_ERROR;
                            LE_UINT32_TO_ARRAY(&notif_data[3], g_dfu_para.cur_offset);
                            server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                                             notif_data, DFU_NOTIFY_LENGTH_REPORT_BUFFER_CRC, GATT_PDU_TYPE_NOTIFICATION);
                            lock_flash();
                            return;
                        }
                    }
                    while (1);
                }
                //erase ok
                g_ota_tmp_buf_used_size = 0;
                g_dfu_para.cur_offset =  g_sil_dfu_resend_offset;
                notif_data[2] = DFU_ARV_FAIL_PROG_ERROR;
                LE_UINT32_TO_ARRAY(&notif_data[3], g_dfu_para.cur_offset);
                server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                                 notif_data, DFU_NOTIFY_LENGTH_REPORT_BUFFER_CRC, GATT_PDU_TYPE_NOTIFICATION);
                lock_flash();
                return;
            }
        }
    }
    else
    {
        DFU_PRINT_TRACE2("ota_temp_buf_used_size is %d,buffer_size=%d", g_ota_tmp_buf_used_size,
                         buffer_size);
        //flush buffer.
        g_ota_tmp_buf_used_size = 0;
        notif_data[2] = DFU_ARV_FAIL_LENGTH_ERROR;
        LE_UINT32_TO_ARRAY(&notif_data[3], g_dfu_para.cur_offset);
        server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                         notif_data, DFU_NOTIFY_LENGTH_REPORT_BUFFER_CRC, GATT_PDU_TYPE_NOTIFICATION);
        return;
    }
}

/**
 * @brief silence_dfu_service_handle_control_point_req
 *
 * @param length     control point cmd length.
 * @param p_value    control point cmd address..
 * @return None
*/
void  silence_dfu_service_handle_control_point_req(uint8_t conn_id, uint16_t length,
                                                   uint8_t *p_value)
{
    T_DFU_CTRL_POINT dfu_control_point;
    uint8_t notif_data[DFU_NOTIFY_LENGTH_MAX] = {0};

    dfu_control_point.opcode = * p_value;
    uint8_t *p = p_value + 1;

    DFU_PRINT_TRACE2("Silent_dfu_service_handle_control_point_req: opcode=0x%x, length=%d",
                     dfu_control_point.opcode, length);

    if (dfu_control_point.opcode >= DFU_OPCODE_MAX || dfu_control_point.opcode <= DFU_OPCODE_MIN)
    {
        notif_data[0] = DFU_OPCODE_NOTIFICATION;
        notif_data[1] = dfu_control_point.opcode;
        notif_data[2] = 0xff;
        server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                         notif_data, DFU_NOTIFY_LENGTH_ARV, GATT_PDU_TYPE_NOTIFICATION);
        return;
    }

    switch (dfu_control_point.opcode)
    {
    case DFU_OPCODE_START_DFU: //0x01
        if (length == DFU_LENGTH_START_DFU)/*4 bytes is pending for encrypt*/
        {
            if (OTP->ota_with_encryption_data)
            {
                DFU_PRINT_TRACE1("Data before decryped: %b", TRACE_BINARY(16, p));
#if defined ( __ICCARM__   )
#pragma diag_suppress=Pa039
#endif
                hw_aes_init(OTP->aes_key, NULL, AES_MODE_ECB, OTP->ota_with_encryption_use_aes256);
#if defined ( __ICCARM__   )
#pragma diag_warning=Pa039
#endif
                hw_aes_decrypt_16byte(p, p);
                DFU_PRINT_TRACE1("Data after decryped: %b", TRACE_BINARY(16, p));
            }

            dfu_control_point.p.start_dfu.ic_type = (*p);
            p += 1;
            dfu_control_point.p.start_dfu.secure_version = (*p);
            p += 1;
            LE_ARRAY_TO_UINT16(dfu_control_point.p.start_dfu.ctrl_flag.value, p);
            p += 2;
            LE_ARRAY_TO_UINT16(dfu_control_point.p.start_dfu.signature, p);
            p += 2;
            LE_ARRAY_TO_UINT16(dfu_control_point.p.start_dfu.crc16, p);
            p += 2;
            LE_ARRAY_TO_UINT32(dfu_control_point.p.start_dfu.image_length, p);

            DFU_PRINT_TRACE6("DFU_OPCODE_START_DFU: ic_type=0x%x, secure_version=0x%x, ctrl_flag.value=0x%x, signature=0x%x,crc16=0x%x, image_length=0x%x",
                             dfu_control_point.p.start_dfu.ic_type,
                             dfu_control_point.p.start_dfu.secure_version,
                             dfu_control_point.p.start_dfu.ctrl_flag.value,
                             dfu_control_point.p.start_dfu.signature,
                             dfu_control_point.p.start_dfu.crc16,
                             dfu_control_point.p.start_dfu.image_length
                            );
            g_dfu_para.ic_type = dfu_control_point.p.start_dfu.ic_type;
            g_dfu_para.ctrl_flag.value = dfu_control_point.p.start_dfu.ctrl_flag.value;
            g_dfu_para.signature = dfu_control_point.p.start_dfu.signature;
            g_dfu_para.crc16 = dfu_control_point.p.start_dfu.crc16;
            g_dfu_para.image_length = dfu_control_point.p.start_dfu.image_length;

            g_dfu_para.image_total_length = g_dfu_para.image_length + IMG_HEADER_SIZE;

            /*check if start dfu fileds are vaild*/
#ifdef SDK_8772
            if (g_dfu_para.ic_type == 0x09)
#else
            if (g_dfu_para.ic_type == 0x05)
#endif
            {
                if (((g_dfu_para.signature >= OTA) && (g_dfu_para.signature < IMAGE_MAX))
                    || (g_dfu_para.signature == SIGNATURE_APP_DEFINE))
                {
                    unlock_flash_all();
                    if (sil_dfu_update(g_dfu_para.signature, 0, DFU_HEADER_SIZE, (uint32_t *)&g_dfu_para.ic_type) == 0)
                    {
                        lock_flash();
                        g_dfu_para.cur_offset += DFU_HEADER_SIZE;
                    }
                    else
                    {
                        lock_flash();
                        silent_dfu_reset(g_dfu_para.signature);
                        dfu_fw_active_reset();
                    }

                    PROFILE_PRINT_INFO0("dfu_act_notify_start_dfu");
                    notif_data[0] = DFU_OPCODE_NOTIFICATION;
                    notif_data[1] = DFU_OPCODE_START_DFU;
                    notif_data[2] = DFU_ARV_SUCCESS;

                    server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                                     notif_data, DFU_NOTIFY_LENGTH_ARV, GATT_PDU_TYPE_NOTIFICATION);

                }
                else
                {
                    DFU_PRINT_TRACE0("[DFU] err: start dfu signature is invalid.");
                    notif_data[0] = DFU_OPCODE_NOTIFICATION;
                    notif_data[1] = DFU_OPCODE_START_DFU;
                    notif_data[2] = DFU_ARV_FAIL_INVALID_PARAMETER;
                    server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                                     notif_data, DFU_NOTIFY_LENGTH_ARV, GATT_PDU_TYPE_NOTIFICATION);
                    return;
                }
            }
            else
            {
                DFU_PRINT_TRACE0("[DFU] err: start dfu ic type is invalid.");
                notif_data[0] = DFU_OPCODE_NOTIFICATION;
                notif_data[1] = DFU_OPCODE_START_DFU;
                notif_data[2] = DFU_ARV_FAIL_INVALID_PARAMETER;
                server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                                 notif_data, DFU_NOTIFY_LENGTH_ARV, GATT_PDU_TYPE_NOTIFICATION);
                return;
            }
        }
        break;

    case DFU_OPCODE_RECEIVE_FW_IMAGE_INFO://0x02
        if (length == DFU_LENGTH_RECEIVE_FW_IMAGE_INFO)
        {
            LE_ARRAY_TO_UINT16(g_dfu_para.signature, p);
            p += 2;
            LE_ARRAY_TO_UINT32(g_dfu_para.cur_offset, p);
            if (g_dfu_para.cur_offset == 0) // || g_dfu_para.cur_offset == DFU_HEADER_SIZE)
            {
                //g_ota_tmp_buf_used_size = 0;
                g_sil_dfu_resend_offset = 0;
            }
            g_ota_tmp_buf_used_size = 0;
            DFU_PRINT_TRACE2("DFU_OPCODE_RECEIVE_FW_IMAGE_INFO: signature = 0x%x, cur_offset = %d",
                             g_dfu_para.signature, g_dfu_para.cur_offset);
        }
        else
        {
            DFU_PRINT_TRACE0("DFU_OPCODE_RECEIVE_FW_IMAGE_INFO: invalid length");
        }
        break;

    case DFU_OPCODE_VALID_FW://0x03

        if (length == DFU_LENGTH_VALID_FW)
        {
            bool check_result;
            LE_ARRAY_TO_UINT16(g_dfu_para.signature, p);
            DFU_PRINT_TRACE1("DFU_OPCODE_VALID_FW: signature = 0x%x", g_dfu_para.signature);

            check_result = silent_dfu_check_checksum(g_dfu_para.signature);
            DFU_PRINT_INFO1("dfu_act_notify_valid, check_result:%d (1: Success, 0: Fail)", check_result);

            if (check_result)
            {
                notif_data[2] = DFU_ARV_SUCCESS;
            }
            else
            {
                notif_data[2] = DFU_ARV_FAIL_CRC_ERROR;
            }
            notif_data[0] = DFU_OPCODE_NOTIFICATION;
            notif_data[1] = DFU_OPCODE_VALID_FW;
            server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                             notif_data, DFU_NOTIFY_LENGTH_ARV, GATT_PDU_TYPE_NOTIFICATION);
        }
        else
        {
            DFU_PRINT_TRACE0("DFU_OPCODE_VALID_FW: invalid length");
        }
        break;

    case DFU_OPCODE_ACTIVE_IMAGE_RESET://0x04
        /*notify bootloader to reset and use new image*/
        DFU_PRINT_TRACE0("DFU_OPCODE_ACTIVE_IMAGE_RESET:");
        break;

    case DFU_OPCODE_SYSTEM_RESET://0x05
        {
            DFU_PRINT_ERROR0("DFU_OPCODE_SYSTEM_RESET");

            /*if select not active image even if image transport successful*/
            if (g_dfu_para.signature >= OTA && g_dfu_para.signature < IMAGE_MAX)
            {

                uint32_t temp_addr = get_temp_ota_bank_addr_by_img_id((T_IMG_ID)g_dfu_para.signature);
                T_IMG_CTRL_HEADER_FORMAT *p_temp_header = (T_IMG_CTRL_HEADER_FORMAT *)temp_addr;
                if (p_temp_header && !p_temp_header->ctrl_flag.flag_value.not_ready)
                {
                    flash_erase_locked(FLASH_ERASE_SECTOR, temp_addr & 0xffffff);
                }
            }

            os_delay(2000); //for print log

            WDG_SystemReset(RESET_ALL, DFU_SYSTEM_RESET);
        }
        break;

    case DFU_OPCODE_REPORT_TARGET_INFO://0x06
        if (length == DFU_LENGTH_REPORT_TARGET_INFO)
        {
            LE_ARRAY_TO_UINT16(g_dfu_para.signature, p);
            PROFILE_PRINT_INFO1("g_dfu_para.signature is %x\r\n", g_dfu_para.signature);

            if (g_dfu_para.signature == SIGNATURE_APP_DEFINE)
            {
                flash_read_locked(APP_DEFINE_DATA_ADDR + CAL_OFFSET(T_IMG_HEADER_FORMAT, git_ver), 4,
                                  (uint8_t *)&g_dfu_para.origin_image_version);
                PROFILE_PRINT_INFO1("g_dfu_para.origin_image_version =0x%X", g_dfu_para.origin_image_version);
            }
            else
            {
                dfu_report_target_fw_info(g_dfu_para.signature, &g_dfu_para.origin_image_version,
                                          (uint32_t *)&g_dfu_para.cur_offset);
            }
            g_dfu_para.cur_offset = 0;

            notif_data[0] = DFU_OPCODE_NOTIFICATION;
            notif_data[1] = DFU_OPCODE_REPORT_TARGET_INFO;
            notif_data[2] = DFU_ARV_SUCCESS;

            LE_UINT16_TO_ARRAY(&notif_data[3], g_dfu_para.origin_image_version);
            LE_UINT32_TO_ARRAY(&notif_data[7], g_dfu_para.cur_offset);

            PROFILE_PRINT_INFO1("g_dfu_para.cur_offset is %x\r\n", g_dfu_para.cur_offset);

            server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                             notif_data, DFU_NOTIFY_LENGTH_REPORT_TARGET_INFO, GATT_PDU_TYPE_NOTIFICATION);
        }
        else
        {
            DFU_PRINT_TRACE0("DFU_OPCODE_REPORT_TARGET_INFO: invalid length");
        }
        break;

    case DFU_OPCODE_CONN_PARA_TO_UPDATE_REQ://0x07
        {
            notif_data[0] = DFU_OPCODE_NOTIFICATION;
            notif_data[1] = DFU_OPCODE_CONN_PARA_TO_UPDATE_REQ;

            if (length  == DFU_LENGTH_CONN_PARA_TO_UPDATE_REQ)
            {
                if (g_dfu_para.ota_conn_para_upd_in_progress)
                {
                    DFU_PRINT_ERROR0("DFU_OPCODE_CONN_PARA_TO_UPDATE_REQ bOTA_ConnParaUpdInProgress!!");
                    notif_data[2] = DFU_ARV_FAIL_OPERATION;
                    server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                                     notif_data, DFU_NOTIFY_LENGTH_ARV, GATT_PDU_TYPE_NOTIFICATION);
                }
                else
                {
                    uint16_t conn_interval_min;
                    uint16_t conn_interval_max;
                    uint16_t conn_latency;
                    uint16_t superv_tout;

                    LE_ARRAY_TO_UINT16(conn_interval_min, p_value + 1);
                    LE_ARRAY_TO_UINT16(conn_interval_max, p_value + 3);
                    LE_ARRAY_TO_UINT16(conn_latency, p_value + 5);
                    LE_ARRAY_TO_UINT16(superv_tout, p_value + 7);

                    if (le_update_conn_param(0, conn_interval_min, conn_interval_max, conn_latency,
                                             2000 / 10, conn_interval_min * 2 - 2, conn_interval_max * 2 - 2) == GAP_CAUSE_SUCCESS)
                    {
                        /* Connection Parameter Update Request sent successfully, means this procedure is in progress. */
                        g_dfu_para.ota_conn_para_upd_in_progress = true;
                        DFU_PRINT_INFO4("DFU_OPCODE_CONN_PARA_TO_UPDATE_REQ intMin=0x%x, intMax=0x%x, lat=0x%x, supto=0x%x.",
                                        conn_interval_min, conn_interval_max, conn_latency, superv_tout);
                    }
                    else
                    {
                        notif_data[2] = DFU_ARV_FAIL_OPERATION;
                        server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                                         notif_data, DFU_NOTIFY_LENGTH_ARV, GATT_PDU_TYPE_NOTIFICATION);
                    }
                }
            }
            else
            {
                /*TODO: to be masked.*/
                DFU_PRINT_ERROR1("DFU_OPCODE_CONN_PARA_TO_UPDATE_REQ length = %d Error!", length);
                notif_data[2] = DFU_ARV_FAIL_INVALID_PARAMETER;
                server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                                 notif_data, DFU_NOTIFY_LENGTH_ARV, GATT_PDU_TYPE_NOTIFICATION);
            }
        }
        break;

    case DFU_OPCODE_BUFFER_CHECK_EN: //0x09
        {
            DFU_PRINT_TRACE1("DFU_OPCODE_BUFFER_CHECK_EN,MTUSIZE is %d", g_dfu_para.mtu_size);
            g_sil_buf_check_en = true;
            notif_data[0] = DFU_OPCODE_NOTIFICATION;
            notif_data[1] = DFU_OPCODE_BUFFER_CHECK_EN;
            notif_data[2] = 0x01;
            LE_UINT16_TO_ARRAY(&notif_data[3], DFU_TEMP_BUFFER_SIZE);
            LE_UINT16_TO_ARRAY(&notif_data[5], g_dfu_para.mtu_size);
            server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                             notif_data, DFU_NOTIFY_LENGTH_BUFFER_CHECK_EN, GATT_PDU_TYPE_NOTIFICATION);
        }
        break;

    case DFU_OPCODE_REPORT_BUFFER_CRC:        //0x0a
        {
            LE_ARRAY_TO_UINT16(mBufSize, p);
            p += 2;
            LE_ARRAY_TO_UINT16(mCrcVal, p);
            DFU_PRINT_TRACE2("DFU_OPCODE_REPORT_BUFFER_CRC mBufferSize is 0x%x,mCrc is 0x%x", mBufSize,
                             mCrcVal);
            silence_BufferCheckProc(conn_id, mBufSize, mCrcVal);
        }
        break;

    case DFU_OPCODE_RECEIVE_IC_TYPE://0x0b
        {
            uint8_t ic_type;
            notif_data[0] = DFU_OPCODE_NOTIFICATION;
            notif_data[1] = DFU_OPCODE_RECEIVE_IC_TYPE;
            if (dfu_report_target_ic_type(OTA, &ic_type))
            {
                notif_data[2] = DFU_ARV_FAIL_INVALID_PARAMETER;
                notif_data[3] =  0xff;
            }
            else
            {
                notif_data[2] = DFU_ARV_SUCCESS;
                notif_data[3] =  ic_type;
            }
            server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                             notif_data, DFU_NOTIFY_LENGTH_RECEIVE_IC_TYPE, GATT_PDU_TYPE_NOTIFICATION);
        }
        break;

#if USER_DATA_COPY_ENABLE
    case DFU_OPCODE_COPY_IMG://0x0c
        {
            uint32_t dlAddress, dlSize;
            LE_ARRAY_TO_UINT16(g_dfu_para.signature, p);
            p += 2;
            LE_ARRAY_TO_UINT32(dlAddress, p);
            p += 4;
            LE_ARRAY_TO_UINT32(dlSize, p);
            DFU_PRINT_TRACE2("DFU_OPCODE_COPY_IMG dlAddress is 0x%x,dlSize is 0x%x", dlAddress,
                             dlSize);

            notif_data[0] = DFU_OPCODE_NOTIFICATION;
            notif_data[1] = DFU_OPCODE_COPY_IMG;

            if (sil_dfu_copy_img(g_dfu_para.signature, dlAddress, dlSize))
            {
                notif_data[2] = DFU_ARV_SUCCESS;
            }
            else
            {
                notif_data[2] = DFU_ARV_FAIL_INVALID_PARAMETER;
            }
            server_send_data(conn_id, g_dfu_service_id, INDEX_DFU_CONTROL_POINT_CHAR_VALUE, \
                             notif_data, DFU_NOTIFY_LENGTH_ARV, GATT_PDU_TYPE_NOTIFICATION);
        }
        break;
#endif  //#if USER_DATA_COPY_ENABLE

    default:
        {
            DFU_PRINT_TRACE1("dfu_service_handle_control_point_req: Unknown Opcode=0x%x",
                             dfu_control_point.opcode
                            );
        }
        break;
    }

    /*if after dfu sevice done application still need do something, define this callback*/
    if (dfu_service_handle_control_point_req_cb)
    {
        dfu_service_handle_control_point_req_cb(dfu_control_point.opcode);
    }

}


/**
 * @brief silence_dfu_service_handle_packet_req
 *
 * @param length     data reviewed length.
 * @param p_value    data receive point address.
 * @return None
*/
void silence_dfu_service_handle_packet_req(uint16_t length, uint8_t *p_value)
{
    DFU_PRINT_TRACE4("dfu_service_handle_packet_req: length=%d, cur_offset =%d, ota_temp_buf_used_size = %d,image_total_length= %d",
                     length,
                     g_dfu_para.cur_offset,
                     g_ota_tmp_buf_used_size,
                     g_dfu_para.image_total_length
                    );

    if (g_dfu_para.cur_offset + g_ota_tmp_buf_used_size + length > g_dfu_para.image_total_length)
    {
        DFU_PRINT_TRACE4("dfu_service_handle_packet_req: p_dfu->cur_offset=%d, ota_temp_buf_used_size =%d, length= %d, image_total_length = %d ",
                         g_dfu_para.cur_offset,
                         g_ota_tmp_buf_used_size,
                         length,
                         g_dfu_para.image_total_length
                        );
        PROFILE_PRINT_INFO0("dfu_act_reset_and_activate");
    }
    else
    {
        if (g_sil_buf_check_en == true)
        {
            memcpy(p_ota_temp_buffer_head + g_ota_tmp_buf_used_size, p_value, length);
            g_ota_tmp_buf_used_size += length;
        }
        else
        {
            if (length >= 16)
            {
                if (OTP->ota_with_encryption_data)
                {
                    uint16_t offset = 0;
                    do
                    {
                        if ((length - offset) >= 16)
                        {
#if defined ( __ICCARM__   )
#pragma diag_suppress=Pa039
#endif
                            hw_aes_init(OTP->aes_key, NULL, AES_MODE_ECB, OTP->ota_with_encryption_use_aes256);
#if defined ( __ICCARM__   )
#pragma diag_warning=Pa039
#endif
                            hw_aes_decrypt_16byte(p_value + offset, p_value + offset);
                            offset += 16;
                        }
                        else
                        {
                            break;
                        }
                    }
                    while (1);
                }
            }

            memcpy(p_ota_temp_buffer_head + g_ota_tmp_buf_used_size, p_value, length);
            g_ota_tmp_buf_used_size += length;

            if (g_ota_tmp_buf_used_size == 2000 ||
                g_dfu_para.cur_offset + g_ota_tmp_buf_used_size == g_dfu_para.image_total_length)
            {
                unlock_flash_all();
                if (sil_dfu_update(g_dfu_para.signature, g_dfu_para.cur_offset, g_ota_tmp_buf_used_size,
                                   (uint32_t *)p_ota_temp_buffer_head) == 0)
                {
                    lock_flash();
                }
                else
                {
                    /*eflash write fail, we should restart ota procedure.*/
                    lock_flash();
                    silent_dfu_reset(g_dfu_para.signature);
                    dfu_fw_active_reset();
                }
                g_dfu_para.cur_offset += g_ota_tmp_buf_used_size;
                g_ota_tmp_buf_used_size = 0;
            }
        }
    }
}

/**
 * @brief write characteristic data from service.
 *
 * @param ServiceID          ServiceID to be written.
 * @param iAttribIndex       Attribute index of characteristic.
 * @param wLength            length of value to be written.
 * @param pValue             value to be written.
 * @return Profile procedure result
*/
T_APP_RESULT dfu_attr_write_cb(uint8_t conn_id, uint8_t service_id, uint16_t attrib_index,
                               T_WRITE_TYPE write_type,
                               uint16_t length, uint8_t *p_value, P_FUN_WRITE_IND_POST_PROC *p_write_ind_post_proc)
{
    T_APP_RESULT  w_cause = APP_RESULT_SUCCESS;

    if (attrib_index == INDEX_DFU_CONTROL_POINT_CHAR_VALUE)
    {
        silence_dfu_service_handle_control_point_req(conn_id, /*(T_DFU *)p_dfu, */length, p_value);
    }
    else if (attrib_index == INDEX_DFU_PACKET_VALUE)
    {
        silence_dfu_service_handle_packet_req(/*(T_DFU *)p_dfu, */length, p_value);
    }
    else
    {
        PROFILE_PRINT_INFO1("dfuServiceAttribPut Fail: iAttribIndex=%d.", attrib_index);

        return APP_RESULT_ATTR_NOT_FOUND;

    }
    return w_cause;
}

/**
 * @brief update CCCD bits from stack.
 *
 * @param ServiceId          Service ID.
 * @param Index          Attribute index of characteristic data.
 * @param wCCCBits         CCCD bits from stack.
 * @return None
*/
void dfu_cccd_update_cb(uint8_t conn_id, T_SERVER_ID service_id, uint16_t index, uint16_t ccc_bits)
{
    TDFU_CALLBACK_DATA callback_data;
    callback_data.msg_type = SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION;
    callback_data.conn_id = conn_id;
    bool b_handle = true;
    PROFILE_PRINT_INFO2("DfuCccdUpdateCb  Index = %d wCCCDBits %x", index, ccc_bits);
    switch (index)
    {
    case INDEX_DFU_CHAR_CCCD_INDEX:
        {
            if (ccc_bits & GATT_CLIENT_CHAR_CONFIG_NOTIFY)
            {
                // Enable Notification
                callback_data.msg_type = SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION;
                callback_data.msg_data.notification_indification_index = DFU_NOTIFY_ENABLE;
            }
            else
            {
                callback_data.msg_type = SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION;
                callback_data.msg_data.notification_indification_index = DFU_NOTIFY_DISABLE;
            }
            break;
        }
    default:
        {
            b_handle = false;
            break;
        }

    }
    /* Notify Application. */
    if (pfn_dfu_extended_cb && (b_handle == true))
    {
        pfn_dfu_extended_cb(service_id, (void *)&callback_data);
    }

    return;
}

/**
 * @brief OTA ble Service Callbacks.
*/
const T_FUN_GATT_SERVICE_CBS DfuServiceCBs =
{
    NULL,   // Read callback function pointer
    dfu_attr_write_cb,  // Write callback function pointer
    dfu_cccd_update_cb                    // CCCD update callback function pointer
};

/**
 * @brief  add OTA ble service to application.
 *
 * @param  pFunc          pointer of app callback function called by profile.
 * @return service ID auto generated by profile layer.
 * @retval ServiceId
*/
uint8_t dfu_add_service(void *pFunc)
{
    uint8_t service_id;

    if (false == server_add_service(&service_id,
                                    (uint8_t *)silence_dfu_service,
                                    sizeof(silence_dfu_service),
                                    DfuServiceCBs))
    {
        PROFILE_PRINT_ERROR1("DFUService_AddService: ServiceId %d", service_id);
        service_id = 0xff;
        return service_id;
    }
    pfn_dfu_extended_cb = (P_FUN_SERVER_GENERAL_CB)pFunc;

    p_ota_temp_buffer_head = ota_temp_buffer_head;
    return service_id;
}


#endif  // #if SUPPORT_SILENT_OTA


