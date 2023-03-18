/**
 * Embedded Device Software
 * Copyright (C) 2022 Peter.Peng
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "cmsis_os.h"
#include "framework.h"
#include "hci_manager.h"

#define hci_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define hci_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define hci_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define hci_debug(str, ...)   pr_debug(str, ## __VA_ARGS__)

#define APPBLE_GAP_DEVICE_NAME_LENGTH 7

PLACE_IN_SECTION("MB_MEM1") ALIGN(4) static TL_CmdPacket_t BleCmdBuffer;

static const osThreadAttr_t hci_user_thread_attr =
{
    .name       = CFG_HCI_USER_EVT_PROCESS_NAME,
    .attr_bits  = CFG_HCI_USER_EVT_PROCESS_ATTR_BITS,
    .cb_mem     = CFG_HCI_USER_EVT_PROCESS_CB_MEM,
    .cb_size    = CFG_HCI_USER_EVT_PROCESS_CB_SIZE,
    .stack_mem  = CFG_HCI_USER_EVT_PROCESS_STACK_MEM,
    .priority   = CFG_HCI_USER_EVT_PROCESS_PRIORITY,
    .stack_size = CFG_HCI_USER_EVT_PROCESS_STACK_SIZE
};

/**
 * Identity root key used to derive LTK and CSRK
 */
static const uint8_t ble_irk[16] = CFG_BLE_IRK;

/**
 * Encryption root key used to derive LTK and CSRK
 */
static const uint8_t ble_erk[16] = CFG_BLE_ERK;

/**
 * @brief   HCI manager handle definition.
 */
typedef struct
{
    hci_tl_user_clbk_t  user_clbk;
    const void*         user_ctx;

    osMutexId_t         hci_mutex_id;
    osSemaphoreId_t     hci_sem_id;
    osThreadId_t        hci_thread_id;

    uint8_t             bd_addr[6];
    uint8_t             srd_bd_addr[6];

    /**
     * connection handle of the current active connection
     * When not in connection, the handle is set to 0xFFFF
     */
    uint16_t            connection_handle;
} hci_manager_handle_t;

static hci_manager_handle_t hci_manager_handle;

static void hci_tl_generate_bd_addr(hci_manager_handle_t* handle);
static int32_t hci_tl_gap_gatt_init(void);
static void hci_tl_thread(void* argument);
static void hci_tl_user_evt(void* p_Payload);
static void hci_tl_status_not(HCI_TL_CmdStatus_t Status);

static void hci_tl_generate_bd_addr(hci_manager_handle_t* handle)
{
    uint8_t* p_otp_addr;
    const uint8_t* p_bd_addr;
    uint32_t udn;
    uint32_t company_id;
    uint32_t device_id;

    udn = LL_FLASH_GetUDN();

    if (udn != 0xFFFFFFFF)
    {
        company_id = LL_FLASH_GetSTCompanyID();
        device_id = LL_FLASH_GetDeviceID();

        handle->bd_addr[0] = (uint8_t)((udn & 0x000000FF) >> 0);
        handle->bd_addr[1] = (uint8_t)((udn & 0x0000FF00) >> 8);
        handle->bd_addr[2] = (uint8_t)device_id;
        handle->bd_addr[3] = (uint8_t)((company_id & 0x000000FF) >> 0);
        handle->bd_addr[4] = (uint8_t)((company_id & 0x0000FF00) >> 8);
        handle->bd_addr[5] = (uint8_t)((company_id & 0x00FF0000) >> 16);
    }
    else
    {
        p_otp_addr = OTP_Read(0);
        if (p_otp_addr)
        {
            p_bd_addr = ((OTP_ID0_t*)p_otp_addr)->bd_address;

            handle->bd_addr[0] = p_bd_addr[0];
            handle->bd_addr[1] = p_bd_addr[1];
            handle->bd_addr[2] = p_bd_addr[2];
            handle->bd_addr[3] = p_bd_addr[3];
            handle->bd_addr[4] = p_bd_addr[4];
            handle->bd_addr[5] = p_bd_addr[5];
        }
        else
        {
            handle->bd_addr[0] =
                (uint8_t)((CFG_ADV_BD_ADDRESS & 0x0000000000FF) >> 0);
            handle->bd_addr[1] =
                (uint8_t)((CFG_ADV_BD_ADDRESS & 0x00000000FF00) >> 8);
            handle->bd_addr[2] =
                (uint8_t)((CFG_ADV_BD_ADDRESS & 0x000000FF0000) >> 16);
            handle->bd_addr[3] =
                (uint8_t)((CFG_ADV_BD_ADDRESS & 0x0000FF000000) >> 24);
            handle->bd_addr[4] =
                (uint8_t)((CFG_ADV_BD_ADDRESS & 0x00FF00000000) >> 32);
            handle->bd_addr[5] =
                (uint8_t)((CFG_ADV_BD_ADDRESS & 0xFF0000000000) >> 40);
        }
    }
}

const uint8_t* ble_get_public_bd_addr(void)
{
    return hci_manager_handle.bd_addr;
}

const uint8_t* ble_get_random_bd_addr(void)
{
    return hci_manager_handle.srd_bd_addr;
}

int32_t hci_tl_disconnect(void)
{
    tBleStatus status = BLE_STATUS_INVALID_PARAMS;

    if (hci_manager_handle.connection_handle != 0xFFFF)
    {
        hci_disconnect(hci_manager_handle.connection_handle, 0x16);
        if (status != BLE_STATUS_SUCCESS)
        {
            //hci_error("Disconnect hci failed, status %d.", status);

            return -EIO;
        }
    }

    return 0;
}

static int32_t hci_tl_gap_gatt_init(void)
{
    uint8_t role;
    uint16_t gap_service_handle, gap_dev_name_char_handle,
             gap_appearance_char_handle;
    const uint8_t* p_bd_addr;
    uint16_t a_appearance[1] = { BLE_CFG_GAP_APPEARANCE };
    tBleStatus status = BLE_STATUS_INVALID_PARAMS;

    status = hci_reset();
    if (status != BLE_STATUS_SUCCESS)
    {
        hci_error("Reset hci failed, status %d.", status);
        return -EIO;
    }

    hci_tl_generate_bd_addr(&hci_manager_handle);

    hci_info("Get public address: %02x:%02x:%02x:%02x:%02x:%02x",
             hci_manager_handle.bd_addr[5],
             hci_manager_handle.bd_addr[4],
             hci_manager_handle.bd_addr[3],
             hci_manager_handle.bd_addr[2],
             hci_manager_handle.bd_addr[1],
             hci_manager_handle.bd_addr[0]);

    p_bd_addr = ble_get_public_bd_addr();

    status = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
                                       CONFIG_DATA_PUBADDR_LEN,
                                       p_bd_addr);
    if (status != BLE_STATUS_SUCCESS)
    {
        hci_error("Write config data failed, offset %d, len %d, status %d.",
                  CONFIG_DATA_PUBADDR_OFFSET,
                  CONFIG_DATA_PUBADDR_LEN,
                  status);
        return -EIO;
    }

    /* TBD: Remove the random address. */

    status = aci_hal_write_config_data(CONFIG_DATA_IR_OFFSET,
                                       CONFIG_DATA_IR_LEN,
                                       ble_irk);
    if (status != BLE_STATUS_SUCCESS)
    {
        hci_error("Write config data failed, offset %d, len %d, status %d.",
                  CONFIG_DATA_IR_OFFSET,
                  CONFIG_DATA_IR_LEN,
                  status);
        return -EIO;
    }

    status = aci_hal_write_config_data(CONFIG_DATA_ER_OFFSET,
                                       CONFIG_DATA_ER_LEN,
                                       ble_erk);
    if (status != BLE_STATUS_SUCCESS)
    {
        hci_error("Write config data failed, offset %d, len %d, status %d.",
                  CONFIG_DATA_ER_OFFSET,
                  CONFIG_DATA_ER_LEN,
                  status);
        return -EIO;
    }

    status = aci_hal_set_tx_power_level(1, CFG_TX_POWER);
    if (status != BLE_STATUS_SUCCESS)
    {
        hci_error("Set tx power level failed, status %d.", status);
        return -EIO;
    }

    status = aci_gatt_init();
    if (status != BLE_STATUS_SUCCESS)
    {
        hci_error("Initialize gatt failed, status %d.", status);
        return -EIO;
    }

    role = 0;

#if (BLE_CFG_PERIPHERAL == 1)
    role |= GAP_PERIPHERAL_ROLE;
#endif /* BLE_CFG_PERIPHERAL == 1 */

#if (BLE_CFG_CENTRAL == 1)
    role |= GAP_CENTRAL_ROLE;
#endif /* BLE_CFG_CENTRAL == 1 */

    if (role > 0)
    {
        const char* name = "ECG";
        status = aci_gap_init(role,
                              CFG_PRIVACY,
                              APPBLE_GAP_DEVICE_NAME_LENGTH,
                              &gap_service_handle,
                              &gap_dev_name_char_handle,
                              &gap_appearance_char_handle);
        if (status != BLE_STATUS_SUCCESS)
        {
            hci_error("Initialize gap failed, status %d.", status);
            return -EIO;
        }

        status = aci_gatt_update_char_value(gap_service_handle,
                                            gap_dev_name_char_handle,
                                            0,
                                            strlen(name),
                                            (uint8_t*)name);
        if (status != BLE_STATUS_SUCCESS)
        {
            hci_error("Update gatt name char failed, status %d.", status);
            return -EIO;
        }
    }

    status = aci_gatt_update_char_value(gap_service_handle,
                                        gap_appearance_char_handle,
                                        0,
                                        2,
                                        (uint8_t*)&a_appearance);
    if (status != BLE_STATUS_SUCCESS)
    {
        hci_error("Update gatt appearance char failed, status %d.", status);
        return -EIO;
    }

    status = hci_le_set_default_phy(ALL_PHYS_PREFERENCE,
                                    TX_2M_PREFERRED,
                                    RX_2M_PREFERRED);
    if (status != BLE_STATUS_SUCCESS)
    {
        hci_error("Set default phy failed, status %d.", status);
        return -EIO;
    }

    status = aci_gap_set_io_capability(CFG_IO_CAPABILITY);
    if (status != BLE_STATUS_SUCCESS)
    {
        hci_error("Set IO capability failed, status %d.", status);
        return -EIO;
    }

    status = aci_gap_set_authentication_requirement(CFG_BONDING_MODE,
                                                    CFG_MITM_PROTECTION,
                                                    CFG_SC_SUPPORT,
                                                    CFG_KEYPRESS_NOTIFICATION_SUPPORT,
                                                    CFG_ENCRYPTION_KEY_SIZE_MIN,
                                                    CFG_ENCRYPTION_KEY_SIZE_MAX,
                                                    CFG_USED_FIXED_PIN,
                                                    CFG_FIXED_PIN,
                                                    CFG_IDENTITY_ADDRESS);
    if (status != BLE_STATUS_SUCCESS)
    {
        hci_error("Set authentication requirement failed, status %d.", status);
        return -EIO;
    }

    status = aci_gap_configure_whitelist();
    if (status != BLE_STATUS_SUCCESS)
    {
        hci_error("Configure gap whitelist failed, status %d.", status);
        return -EIO;
    }

    return 0;
}

int32_t hci_tl_init(hci_tl_user_clbk_t user_clbk, const void* user_ctx)
{
    SHCI_CmdStatus_t status;
    SHCI_C2_Ble_Init_Cmd_Packet_t ble_init_cmd_packet =
    {
        { { 0, 0, 0 } },                    /**< Header unused */
        { 0,                                /** pBleBufferAddress not used */
          0,                                /** BleBufferSize not used */
          CFG_BLE_NUM_GATT_ATTRIBUTES,
          CFG_BLE_NUM_GATT_SERVICES,
          CFG_BLE_ATT_VALUE_ARRAY_SIZE,
          CFG_BLE_NUM_LINK,
          CFG_BLE_DATA_LENGTH_EXTENSION,
          CFG_BLE_PREPARE_WRITE_LIST_SIZE,
          CFG_BLE_MBLOCK_COUNT,
          CFG_BLE_MAX_ATT_MTU,
          CFG_BLE_SLAVE_SCA,
          CFG_BLE_MASTER_SCA,
          CFG_BLE_LS_SOURCE,
          CFG_BLE_MAX_CONN_EVENT_LENGTH,
          CFG_BLE_HSE_STARTUP_TIME,
          CFG_BLE_VITERBI_MODE,
          CFG_BLE_OPTIONS,
          0,
          CFG_BLE_MAX_COC_INITIATOR_NBR,
          CFG_BLE_MIN_TX_POWER,
          CFG_BLE_MAX_TX_POWER,
          CFG_BLE_RX_MODEL_CONFIG,
          CFG_BLE_MAX_ADV_SET_NBR,
          CFG_BLE_MAX_ADV_DATA_LEN,
          CFG_BLE_TX_PATH_COMPENS,
          CFG_BLE_RX_PATH_COMPENS,
          CFG_BLE_CORE_VERSION }
    };
    HCI_TL_HciInitConf_t Hci_Tl_Init_Conf;
    int32_t ret;

    if (user_clbk == NULL)
    {
        return -EINVAL;
    }

    hci_manager_handle.user_clbk = user_clbk;
    hci_manager_handle.user_ctx = user_ctx;

    Hci_Tl_Init_Conf.p_cmdbuffer = (uint8_t*)&BleCmdBuffer;
    Hci_Tl_Init_Conf.StatusNotCallBack = hci_tl_status_not;
    hci_init(hci_tl_user_evt, (void*)&Hci_Tl_Init_Conf);

    status = SHCI_C2_BLE_Init(&ble_init_cmd_packet);
    if (status != SHCI_Success)
    {
        hci_error("Initialize C2 BLE stack failed, status %d.", status);
        return -EIO;
    }

    ret = hci_tl_gap_gatt_init();
    if (ret)
    {
        hci_error("Initialize hci gap gatt failed, ret %d.", ret);
        return -EIO;
    }

    SVCCTL_Init();

    hci_info("Initialize hci succeed.");

    return 0;
}

static void hci_tl_thread(void* argument)
{
    UNUSED(argument);

    for (;;)
    {
        osThreadFlagsWait(1, osFlagsWaitAny, osWaitForever);
        hci_user_evt_proc();
    }
}

static void hci_tl_user_evt(void* p_Payload)
{
    SVCCTL_UserEvtFlowStatus_t svctl_return_status;
    tHCI_UserEvtRxParam* p_param;

    p_param = (tHCI_UserEvtRxParam*)p_Payload;

    svctl_return_status = SVCCTL_UserEvtRx((void*)&(p_param->pckt->evtserial));

    hci_info("Received hci user event, svctl_return_status 0x%x.",
             svctl_return_status);

    if (svctl_return_status != SVCCTL_UserEvtFlowDisable)
    {
        p_param->status = HCI_TL_UserEventFlow_Enable;
    }
    else
    {
        p_param->status = HCI_TL_UserEventFlow_Disable;
    }

    return;
}

static void hci_tl_status_not(HCI_TL_CmdStatus_t Status)
{
    switch (Status)
    {
    case HCI_TL_CmdBusy:
        osMutexAcquire(hci_manager_handle.hci_mutex_id, osWaitForever);
        break;

    case HCI_TL_CmdAvailable:
        osMutexRelease(hci_manager_handle.hci_mutex_id);
        break;

    default:
        break;
    }

    return;
}

void hci_notify_asynch_evt(void* p_Data)
{
    UNUSED(p_Data);
    osThreadFlagsSet(hci_manager_handle.hci_thread_id, 1);

    return;
}

void hci_cmd_resp_release(uint32_t Flag)
{
    UNUSED(Flag);
    osSemaphoreRelease(hci_manager_handle.hci_sem_id);

    return;
}

void hci_cmd_resp_wait(uint32_t Timeout)
{
    UNUSED(Timeout);
    osSemaphoreAcquire(hci_manager_handle.hci_sem_id, osWaitForever);

    return;
}

SVCCTL_UserEvtFlowStatus_t SVCCTL_App_Notification(void* p_Pckt)
{
    hci_event_pckt* p_event_pckt;
    evt_le_meta_event* p_meta_evt;
    evt_blecore_aci* p_blecore_evt;
    uint8_t Tx_phy, Rx_phy;
    tBleStatus status = BLE_STATUS_INVALID_PARAMS;
    hci_le_connection_complete_event_rp0* p_connection_complete_event;
    hci_disconnection_complete_event_rp0* p_disconnection_complete_event;
    hci_le_phy_update_complete_event_rp0* p_evt_le_phy_update_complete;
    hci_le_connection_update_complete_event_rp0*
        p_connection_update_complete_event;

    p_event_pckt = (hci_event_pckt*)((hci_uart_pckt*)p_Pckt)->data;

    switch (p_event_pckt->evt)
    {
    case HCI_DISCONNECTION_COMPLETE_EVT_CODE:

        p_disconnection_complete_event =
            (hci_disconnection_complete_event_rp0*)p_event_pckt->data;

        if (p_disconnection_complete_event->Connection_Handle ==
            hci_manager_handle.connection_handle)
        {
            hci_manager_handle.connection_handle = 0xFFFF;

            hci_info("Event: HCI_DISCONNECTION_COMPLETE_EVT_CODE.");
            hci_info(" - connection handle: 0x%x.",
                     p_disconnection_complete_event->Connection_Handle);
            hci_info(" - reason: 0x%x.",
                     p_disconnection_complete_event->Reason);
        }

        if (hci_manager_handle.user_clbk)
        {
            hci_manager_handle.user_clbk(HCI_DISCONNECTION_COMPLETE_EVT_CODE,
                                         hci_manager_handle.user_ctx);
        }

        break;

    case HCI_LE_META_EVT_CODE:

        p_meta_evt = (evt_le_meta_event*)p_event_pckt->data;

        switch (p_meta_evt->subevent)
        {
        case HCI_LE_CONNECTION_UPDATE_COMPLETE_SUBEVT_CODE:

            p_connection_update_complete_event =
                (hci_le_connection_update_complete_event_rp0*)p_meta_evt->data;

            hci_info("Event: HCI_LE_CONNECTION_UPDATE_COMPLETE_SUBEVT_CODE.");
            hci_info(" - connection interval: %.2f ms.",
                     p_connection_update_complete_event->Conn_Interval * 1.25);
            hci_info(" - connection latency: %d.",
                     p_connection_update_complete_event->Conn_Latency);
            hci_info(" - supervision timeout: %d ms.",
                     p_connection_update_complete_event->Supervision_Timeout *
                     10);

            break;

        case HCI_LE_PHY_UPDATE_COMPLETE_SUBEVT_CODE:

            p_evt_le_phy_update_complete =
                (hci_le_phy_update_complete_event_rp0*)p_meta_evt->data;

            hci_info("Event: HCI_LE_PHY_UPDATE_COMPLETE_SUBEVT_CODE.");

            if (p_evt_le_phy_update_complete->Status == 0)
            {
                hci_info(" - le phy update succeed.");
            }
            else
            {
                hci_error(" - le phy update failed, status 0x%x.",
                          p_evt_le_phy_update_complete->Status);
            }

            status = hci_le_read_phy(
                p_evt_le_phy_update_complete->Connection_Handle,
                &Tx_phy,
                &Rx_phy);
            if (status != BLE_STATUS_SUCCESS)
            {
                hci_error(" - le phy read failed, status 0x%x.", status);
            }
            else
            {
                hci_info(" - le phy read succeed, TX %d, RX %d.", Tx_phy,
                         Rx_phy);
            }

            break;

        case HCI_LE_CONNECTION_COMPLETE_SUBEVT_CODE:

            p_connection_complete_event =
                (hci_le_connection_complete_event_rp0*)p_meta_evt->data;

            hci_info("Event: HCI_LE_CONNECTION_COMPLETE_SUBEVT_CODE.");
            hci_info(" - connection handle: 0x%x.",
                     p_connection_complete_event->Connection_Handle);
            hci_info(" - central addr: %02x:%02x:%02x:%02x:%02x:%02x.",
                     p_connection_complete_event->Peer_Address[5],
                     p_connection_complete_event->Peer_Address[4],
                     p_connection_complete_event->Peer_Address[3],
                     p_connection_complete_event->Peer_Address[2],
                     p_connection_complete_event->Peer_Address[1],
                     p_connection_complete_event->Peer_Address[0]);
            hci_info(" - connection interval: %.2f ms.",
                     p_connection_complete_event->Conn_Interval * 1.25);
            hci_info(" - connection latency: %d.",
                     p_connection_complete_event->Conn_Latency);
            hci_info(" - supervision timeout: %d ms.",
                     p_connection_complete_event->Supervision_Timeout * 10);

            hci_manager_handle.connection_handle =
                p_connection_complete_event->Connection_Handle;

            if (hci_manager_handle.user_clbk)
            {
                hci_manager_handle.user_clbk(
                    HCI_LE_CONNECTION_COMPLETE_SUBEVT_CODE,
                    hci_manager_handle.user_ctx);
            }

            break;

        default:

            break;
        }

        break;

    case HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE:

        p_blecore_evt = (evt_blecore_aci*)p_event_pckt->data;

        switch (p_blecore_evt->ecode)
        {
            aci_gap_pairing_complete_event_rp0* pairing_complete;

        case ACI_GAP_LIMITED_DISCOVERABLE_VSEVT_CODE:

            hci_info("Event: ACI_GAP_LIMITED_DISCOVERABLE_VSEVT_CODE.");

            break;

        case ACI_GAP_PASS_KEY_REQ_VSEVT_CODE:

            hci_info("Event: ACI_GAP_PASS_KEY_REQ_VSEVT_CODE.");

            status = aci_gap_pass_key_resp(hci_manager_handle.connection_handle,
                                           123456);
            if (status != BLE_STATUS_SUCCESS)
            {
                hci_error(" - pass key failed, reason: 0x%x.", status);
            }
            else
            {
                hci_info(" - pass key succeed.");
            }

            break;

        case ACI_GAP_AUTHORIZATION_REQ_VSEVT_CODE:

            hci_info("Event: ACI_GAP_AUTHORIZATION_REQ_VSEVT_CODE.");

            break;

        case ACI_GAP_SLAVE_SECURITY_INITIATED_VSEVT_CODE:

            hci_info("Event: ACI_GAP_SLAVE_SECURITY_INITIATED_VSEVT_CODE.");

            break;

        case ACI_GAP_BOND_LOST_VSEVT_CODE:

            hci_info("Event: ACI_GAP_BOND_LOST_VSEVT_CODE.");

            status = aci_gap_allow_rebond(hci_manager_handle.connection_handle);
            if (status != BLE_STATUS_SUCCESS)
            {
                hci_error(" - gap allow rebond failed, reason: 0x%x.", status);
            }
            else
            {
                hci_info(" - gap allow rebond succeed.");
            }

            break;

        case ACI_GAP_ADDR_NOT_RESOLVED_VSEVT_CODE:

            hci_info("Event: ACI_GAP_ADDR_NOT_RESOLVED_VSEVT_CODE.");

            break;

        case (ACI_GAP_KEYPRESS_NOTIFICATION_VSEVT_CODE):

            hci_info("Event: ACI_GAP_KEYPRESS_NOTIFICATION_VSEVT_CODE.");

            break;

        case (ACI_GAP_NUMERIC_COMPARISON_VALUE_VSEVT_CODE):

            hci_info("Event: ACI_GAP_NUMERIC_COMPARISON_VALUE_VSEVT_CODE.");
            hci_info(" - numeric value = %ld.",
                     ((aci_gap_numeric_comparison_value_event_rp0*)(
                          p_blecore_evt->data))->Numeric_Value);
            hci_info(" - hex value = %lx.",
                     ((aci_gap_numeric_comparison_value_event_rp0*)(
                          p_blecore_evt->data))->Numeric_Value);

            status = aci_gap_numeric_comparison_value_confirm_yesno(
                hci_manager_handle.connection_handle,
                YES);
            if (status != BLE_STATUS_SUCCESS)
            {
                hci_error(" - confirm YES failed, reason: 0x%x.", status);
            }
            else
            {
                hci_info(" - confirm YES succeed.");
            }

            break;

        case (ACI_GAP_PAIRING_COMPLETE_VSEVT_CODE):

            pairing_complete =
                (aci_gap_pairing_complete_event_rp0*)p_blecore_evt->data;

            hci_info("Event: ACI_GAP_PAIRING_COMPLETE_VSEVT_CODE.");

            if (pairing_complete->Status == 0)
            {
                hci_info(" - pairing succeed.");
            }
            else
            {
                hci_error(" - pairing failed, status: 0x%x, reason: 0x%x.",
                          pairing_complete->Status,
                          pairing_complete->Reason);
            }

            break;

        case ACI_GAP_PROC_COMPLETE_VSEVT_CODE:

            hci_info("Event: ACI_GAP_PROC_COMPLETE_VSEVT_CODE.");

            break;
        }

        break;

    default:

        break;
    }

    return SVCCTL_UserEvtFlowEnable;
}

/**
 * @brief   Probe the HCI manager.
 *
 * @param   obj Pointer to the HCI manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t hci_manager_probe(const object* obj)
{
    hci_manager_handle_t* handle = (hci_manager_handle_t*)obj->object_data;

    (void)memset(handle, 0, sizeof(hci_manager_handle_t));

    handle->connection_handle = 0xFFFF;

    handle->hci_mutex_id = osMutexNew(NULL);
    if (!handle->hci_mutex_id)
    {
        hci_error("Manager <%s> create mutex failed.", obj->name);

        return -EINVAL;
    }
    else
    {
        hci_info("Manager <%s> create mutex succeed.", obj->name);
    }

    handle->hci_sem_id = osSemaphoreNew(1, 0, NULL);
    if (!handle->hci_sem_id)
    {
        hci_error("Manager <%s> create semaphore failed.", obj->name);

        return -EINVAL;
    }
    else
    {
        hci_info("Manager <%s> create semaphore succeed.", obj->name);
    }

    handle->hci_thread_id = osThreadNew(hci_tl_thread,
                                        NULL,
                                        &hci_user_thread_attr);
    if (!handle->hci_thread_id)
    {
        hci_error("Manager <%s> create thread <%s> failed.",
                  obj->name,
                  hci_user_thread_attr.name);

        return -EINVAL;
    }
    else
    {
        hci_info("Manager <%s> create thread <%s> succeed.",
                 obj->name,
                 hci_user_thread_attr.name);
    }

    hci_info("Manager <%s> probe succeed.", obj->name);

    return 0;
}

/**
 * @brief   Remove the HCI manager.
 *
 * @param   obj Pointer to the HCI manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t hci_manager_shutdown(const object* obj)
{
    hci_manager_handle_t* handle = (hci_manager_handle_t*)obj->object_data;

    (void)handle;

    hci_info("Manager <%s> shutdown succeed.", obj->name);

    return 0;
}

module_middleware(CONFIG_HCI_MANAGER_NAME,
                  CONFIG_HCI_MANAGER_LABEL,
                  hci_manager_probe,
                  hci_manager_shutdown,
                  NULL, &hci_manager_handle, NULL);
