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
#include "cmsis_os.h"
#include "framework.h"
#include "shci_manager.h"

#define shci_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define shci_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define shci_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define shci_debug(str, ...)   pr_debug(str, ## __VA_ARGS__)

#define POOL_SIZE (CFG_TLBLE_EVT_QUEUE_LENGTH * 4U * \
                   DIVC((sizeof(TL_PacketHeader_t) + \
                         TL_BLE_EVENT_FRAME_SIZE), 4U))

PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t EvtPool[POOL_SIZE];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t SystemCmdBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t \
    SystemSpareEvtBuffer[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255U];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t \
    BleSpareEvtBuffer[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255];

static const osThreadAttr_t shci_user_thread_attr =
{
    .name       = CFG_SHCI_USER_EVT_PROCESS_NAME,
    .attr_bits  = CFG_SHCI_USER_EVT_PROCESS_ATTR_BITS,
    .cb_mem     = CFG_SHCI_USER_EVT_PROCESS_CB_MEM,
    .cb_size    = CFG_SHCI_USER_EVT_PROCESS_CB_SIZE,
    .stack_mem  = CFG_SHCI_USER_EVT_PROCESS_STACK_MEM,
    .priority   = CFG_SHCI_USER_EVT_PROCESS_PRIORITY,
    .stack_size = CFG_SHCI_USER_EVT_PROCESS_STACK_SIZE
};

/**
 * @brief   SHCI manager handle definition.
 */
typedef struct
{
    shci_tl_user_clbk_t user_clbk;
    const void*         user_ctx;

    osMutexId_t         shci_mutex_id;
    osSemaphoreId_t     shci_sem_id;
    osThreadId_t        shci_thread_id;
} shci_manager_handle_t;

static shci_manager_handle_t shci_manager_handle;

static void shci_tl_thread(void* argument);
static void shci_tl_status_not(SHCI_TL_CmdStatus_t status);
static void shci_tl_user_evt(void* pPayload);
static void shci_tl_evt_error(void* pPayload);
static void shci_tl_ready_processing(void* pPayload);

int32_t shci_tl_init(shci_tl_user_clbk_t user_clbk, const void* user_ctx)
{
    TL_MM_Config_t tl_mm_config;
    SHCI_TL_HciInitConf_t SHci_Tl_Init_Conf;

    if (user_clbk == NULL)
    {
        return -EINVAL;
    }

    shci_manager_handle.user_clbk = user_clbk;
    shci_manager_handle.user_ctx = user_ctx;

    TL_Init();

    SHci_Tl_Init_Conf.p_cmdbuffer = (uint8_t*)&SystemCmdBuffer;
    SHci_Tl_Init_Conf.StatusNotCallBack = shci_tl_status_not;
    shci_init(shci_tl_user_evt, (void*)&SHci_Tl_Init_Conf);

    tl_mm_config.p_BleSpareEvtBuffer = BleSpareEvtBuffer;
    tl_mm_config.p_SystemSpareEvtBuffer = SystemSpareEvtBuffer;
    tl_mm_config.p_AsynchEvtPool = EvtPool;
    tl_mm_config.AsynchEvtPoolSize = POOL_SIZE;
    TL_MM_Init(&tl_mm_config);

    TL_Enable();

    return 0;
}

static void shci_tl_status_not(SHCI_TL_CmdStatus_t status)
{
    switch (status)
    {
    case SHCI_TL_CmdBusy:
        osMutexAcquire(shci_manager_handle.shci_mutex_id, osWaitForever);
        break;

    case SHCI_TL_CmdAvailable:
        osMutexRelease(shci_manager_handle.shci_mutex_id);
        break;

    default:
        break;
    }
    return;
}

/**
 * The type of the payload for a system user event is tSHCI_UserEvtRxParam
 * When the system event is both :
 *    - a ready event (subevtcode = SHCI_SUB_EVT_CODE_READY)
 *    - reported by the FUS (sysevt_ready_rsp == FUS_FW_RUNNING)
 * The buffer shall not be released
 * (eg ((tSHCI_UserEvtRxParam*)pPayload)->status shall be set to SHCI_TL_UserEventFlow_Disable)
 * When the status is not filled, the buffer is released by default
 */
static void shci_tl_user_evt(void* pPayload)
{
    TL_AsynchEvt_t* p_sys_event;
    WirelessFwInfo_t WirelessInfo;

    p_sys_event = (TL_AsynchEvt_t*) \
                  (((tSHCI_UserEvtRxParam*)pPayload)->pckt->evtserial.evt.
                   payload);

    shci_info("Received shci user event, subevtcode 0x%x.",
              p_sys_event->subevtcode);

    switch (p_sys_event->subevtcode)
    {
    case SHCI_SUB_EVT_CODE_READY:
        SHCI_GetWirelessFwInfo(&WirelessInfo);
        shci_info("Wireless Firmware version %d.%d.%d",
                  WirelessInfo.VersionMajor,
                  WirelessInfo.VersionMinor,
                  WirelessInfo.VersionSub);
        shci_info("Wireless Firmware build %d",
                  WirelessInfo.VersionReleaseType);
        shci_info("FUS version %d.%d.%d",
                  WirelessInfo.FusVersionMajor,
                  WirelessInfo.FusVersionMinor,
                  WirelessInfo.FusVersionSub);
        shci_tl_ready_processing(pPayload);
        break;

    case SHCI_SUB_EVT_ERROR_NOTIF:
        shci_tl_evt_error(pPayload);
        break;

    case SHCI_SUB_EVT_BLE_NVM_RAM_UPDATE:
        shci_info(
            "NVM ram update, address = %lx, size = %ld.",
            ((SHCI_C2_BleNvmRamUpdate_Evt_t*)p_sys_event->payload)->StartAddress,
            ((SHCI_C2_BleNvmRamUpdate_Evt_t*)p_sys_event->payload)->Size);
        break;

    case SHCI_SUB_EVT_NVM_START_WRITE:
        shci_info(
            "NVM start write, NumberOfWords = %ld.",
            ((SHCI_C2_NvmStartWrite_Evt_t*)p_sys_event->payload)->NumberOfWords);
        break;

    case SHCI_SUB_EVT_NVM_END_WRITE:
        shci_info("NVM end write.");
        break;

    case SHCI_SUB_EVT_NVM_START_ERASE:
        shci_info(
            "NVM start erase, NumberOfSectors = %ld.",
            ((SHCI_C2_NvmStartErase_Evt_t*)p_sys_event->payload)->NumberOfSectors);
        break;

    case SHCI_SUB_EVT_NVM_END_ERASE:
        shci_info("NVM end erase.");
        break;

    default:
        break;
    }

    if (shci_manager_handle.user_clbk)
    {
        shci_manager_handle.user_clbk(
            p_sys_event->subevtcode,
            shci_manager_handle.user_ctx);
    }

    return;
}

/**
 * @brief Notify a system error coming from the M0 firmware
 * @param  ErrorCode  : errorCode detected by the M0 firmware
 *
 * @retval None
 */
static void shci_tl_evt_error(void* pPayload)
{
    TL_AsynchEvt_t* p_sys_event;
    SCHI_SystemErrCode_t* p_sys_error_code;

    p_sys_event = (TL_AsynchEvt_t*) \
                  (((tSHCI_UserEvtRxParam*)pPayload)->pckt->evtserial.evt.
                   payload);
    p_sys_error_code = (SCHI_SystemErrCode_t*)p_sys_event->payload;

    shci_error("Received shci event error, sys_error_code %d.",
               (*p_sys_error_code));

    return;
}

static void shci_tl_ready_processing(void* pPayload)
{
    TL_AsynchEvt_t* p_sys_event;
    SHCI_C2_Ready_Evt_t* p_sys_ready_event;

    SHCI_C2_CONFIG_Cmd_Param_t config_param = { 0 };
    uint32_t RevisionID = 0;
    uint32_t DeviceID = 0;

    p_sys_event = (TL_AsynchEvt_t*) \
                  (((tSHCI_UserEvtRxParam*)pPayload)->pckt->evtserial.evt.
                   payload);
    p_sys_ready_event = (SHCI_C2_Ready_Evt_t*)p_sys_event->payload;

    if (p_sys_ready_event->sysevt_ready_rsp == WIRELESS_FW_RUNNING)
    {
        /**
         * The wireless firmware is running on the CPU2
         */
        shci_info("The wireless firmware is running on the CPU2.");

        /* Traces channel initialization */
        /* TBD: */
        //APPD_EnableCPU2();

        /* Enable all events Notification */
        config_param.PayloadCmdSize = SHCI_C2_CONFIG_PAYLOAD_CMD_SIZE;
        config_param.EvtMask1 = SHCI_C2_CONFIG_EVTMASK1_BIT0_ERROR_NOTIF_ENABLE
                                +
                                SHCI_C2_CONFIG_EVTMASK1_BIT1_BLE_NVM_RAM_UPDATE_ENABLE
                                +
                                SHCI_C2_CONFIG_EVTMASK1_BIT2_THREAD_NVM_RAM_UPDATE_ENABLE
                                +
                                SHCI_C2_CONFIG_EVTMASK1_BIT3_NVM_START_WRITE_ENABLE
                                +
                                SHCI_C2_CONFIG_EVTMASK1_BIT4_NVM_END_WRITE_ENABLE
                                +
                                SHCI_C2_CONFIG_EVTMASK1_BIT5_NVM_START_ERASE_ENABLE
                                +
                                SHCI_C2_CONFIG_EVTMASK1_BIT6_NVM_END_ERASE_ENABLE;

        /* Read revision identifier */
        /**
         * @brief  Return the device revision identifier
         * @note   This field indicates the revision of the device.
         * @rmtoll DBGMCU_IDCODE REV_ID        LL_DBGMCU_GetRevisionID
         * @retval Values between Min_Data=0x00 and Max_Data=0xFFFF
         */
        RevisionID = LL_DBGMCU_GetRevisionID();
        config_param.RevisionID = (uint16_t)RevisionID;
        DeviceID = LL_DBGMCU_GetDeviceID();
        config_param.DeviceID = (uint16_t)DeviceID;

        shci_info("Get RevisionID 0x%x, DeviceID 0x%x.", RevisionID, DeviceID);

        (void)SHCI_C2_Config(&config_param);

        shci_info("Initialize shci succeed.");
    }
    else if (p_sys_ready_event->sysevt_ready_rsp == FUS_FW_RUNNING)
    {
        /**
         * The FUS firmware is running on the CPU2
         * In the scope of this application, there should be no case when we get here
         */
        shci_error("The FUS firmware is running on the CPU2.");

        /* The packet shall not be released as this is not supported by the FUS */
        ((tSHCI_UserEvtRxParam*)pPayload)->status =
            SHCI_TL_UserEventFlow_Disable;
    }
    else
    {
        shci_error("Ready rsp unexpected case.");
    }

    return;
}

static void shci_tl_thread(void* argument)
{
    UNUSED(argument);
    for (;;)
    {
        osThreadFlagsWait(1, osFlagsWaitAny, osWaitForever);
        shci_user_evt_proc();
    }
}

void shci_notify_asynch_evt(void* pdata)
{
    UNUSED(pdata);
    osThreadFlagsSet(shci_manager_handle.shci_thread_id, 1);
    return;
}

void shci_cmd_resp_release(uint32_t flag)
{
    UNUSED(flag);
    osSemaphoreRelease(shci_manager_handle.shci_sem_id);
    return;
}

void shci_cmd_resp_wait(uint32_t timeout)
{
    UNUSED(timeout);
    osSemaphoreAcquire(shci_manager_handle.shci_sem_id, osWaitForever);
    return;
}

/**
 * @brief   Probe the SHCI manager.
 *
 * @param   obj Pointer to the SHCI manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t shci_manager_probe(const object* obj)
{
    shci_manager_handle_t* handle = (shci_manager_handle_t*)obj->object_data;

    (void)memset(handle, 0, sizeof(shci_manager_handle_t));

    handle->shci_mutex_id = osMutexNew(NULL);
    if (!handle->shci_mutex_id)
    {
        shci_error("Manager <%s> create mutex failed.", obj->name);

        return -EINVAL;
    }
    else
    {
        shci_info("Manager <%s> create mutex succeed.", obj->name);
    }

    handle->shci_sem_id = osSemaphoreNew(1, 0, NULL);
    if (!handle->shci_sem_id)
    {
        shci_error("Manager <%s> create semaphore failed.", obj->name);

        return -EINVAL;
    }
    else
    {
        shci_info("Manager <%s> create semaphore succeed.", obj->name);
    }

    handle->shci_thread_id = osThreadNew(shci_tl_thread,
                                         NULL,
                                         &shci_user_thread_attr);
    if (!handle->shci_thread_id)
    {
        shci_error("Manager <%s> create thread <%s> failed.",
                   obj->name,
                   shci_user_thread_attr.name);

        return -EINVAL;
    }
    else
    {
        shci_info("Manager <%s> create thread <%s> succeed.",
                  obj->name,
                  shci_user_thread_attr.name);
    }

    shci_info("Manager <%s> probe succeed.", obj->name);

    return 0;
}

/**
 * @brief   Remove the SHCI manager.
 *
 * @param   obj Pointer to the SHCI manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t shci_manager_shutdown(const object* obj)
{
    shci_manager_handle_t* handle = (shci_manager_handle_t*)obj->object_data;

    (void)handle;

    shci_info("Manager <%s> shutdown succeed.", obj->name);

    return 0;
}

module_middleware(CONFIG_SHCI_MANAGER_NAME,
                  CONFIG_SHCI_MANAGER_LABEL,
                  shci_manager_probe,
                  shci_manager_shutdown,
                  NULL, &shci_manager_handle, NULL);
