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
#include "hci_manager.h"
#include "adv_manager.h"

#define adv_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define adv_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define adv_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define adv_debug(str, ...)   pr_debug(str, ## __VA_ARGS__)

#define DEF_FAST_ADV_TIMEOUT 60000  /* 60s Timeout */

static const char adv_local_name[] =
{
    AD_TYPE_COMPLETE_LOCAL_NAME, 'E', 'C', 'G'
};

static uint8_t adv_manuf_data[14] =
{
    sizeof(adv_manuf_data) - 1,
    AD_TYPE_MANUFACTURER_SPECIFIC_DATA,
    0x01,   /* Generic */
    0x00,   /* Generic */
    0x00,   /* GROUP A Feature  */
    0x00,   /* GROUP A Feature */
    0x00,   /* GROUP B Feature */
    0x00,   /* GROUP B Feature */
    0x00,   /* BLE MAC start - MSB */
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,   /* BLE MAC stop */
};

const osTimerAttr_t adv_timer_attr =
{
    .name       = CONFIG_ADV_MANAGER_TIMER_NAME,
    .attr_bits  = 0,
    .cb_mem     = NULL,
    .cb_size    = 0,
};

/**
 * @brief   ADV manager handle definition.
 */
typedef struct
{
    adv_user_clbk_t user_clbk;
    const void*     user_ctx;

    osTimerId_t     timer;
} adv_manager_handle_t;

static adv_manager_handle_t adv_manager_handle;

static void adv_timer_callback(void* argument)
{
    if (adv_manager_handle.user_clbk)
    {
        adv_manager_handle.user_clbk(ADV_TIMEOUT_EVT_CODE,
                                     adv_manager_handle.user_ctx);
    }
}

int32_t adv_init(adv_user_clbk_t user_clbk, const void* user_ctx)
{
    const uint8_t* p_bd_addr = ble_get_public_bd_addr();

    if (user_clbk == NULL)
    {
        return -EINVAL;
    }

    adv_manager_handle.user_clbk = user_clbk;
    adv_manager_handle.user_ctx = user_ctx;

    adv_manuf_data[sizeof(adv_manuf_data) - 6] = p_bd_addr[5];
    adv_manuf_data[sizeof(adv_manuf_data) - 5] = p_bd_addr[4];
    adv_manuf_data[sizeof(adv_manuf_data) - 4] = p_bd_addr[3];
    adv_manuf_data[sizeof(adv_manuf_data) - 3] = p_bd_addr[2];
    adv_manuf_data[sizeof(adv_manuf_data) - 2] = p_bd_addr[1];
    adv_manuf_data[sizeof(adv_manuf_data) - 1] = p_bd_addr[0];

    adv_info("Initialize advertisement succeed.");

    return 0;
}

int32_t adv_setup(adv_state_e state)
{
    tBleStatus status = BLE_STATUS_INVALID_PARAMS;
    uint16_t Min_Inter, Max_Inter;
    uint8_t adv_service_uuid[20];
    uint8_t adv_service_uuid_len = 0;

    (void)osTimerStop(adv_manager_handle.timer);

    status = aci_gap_set_non_discoverable();
    if (status != BLE_STATUS_SUCCESS)
    {
        //adv_error("Set gap non discoverable failed, status %d.", status);
    }

    if (state == ADV_STATE_IDLE)
    {
        adv_info("Stop advertising.");

        return 0;
    }

    if (state == ADV_STATE_FAST)
    {
        Min_Inter = CFG_FAST_CONN_ADV_INTERVAL_MIN;
        Max_Inter = CFG_FAST_CONN_ADV_INTERVAL_MAX;
    }
    else
    {
        Min_Inter = CFG_LP_CONN_ADV_INTERVAL_MIN;
        Max_Inter = CFG_LP_CONN_ADV_INTERVAL_MAX;
    }

    adv_service_uuid[0] = AD_TYPE_16_BIT_SERV_UUID;
    adv_service_uuid_len += 1;
    adv_service_uuid[adv_service_uuid_len] =
        (uint8_t)(HEART_RATE_SERVICE_UUID & 0xFF);
    adv_service_uuid_len += 1;
    adv_service_uuid[adv_service_uuid_len] =
        (uint8_t)(HEART_RATE_SERVICE_UUID >> 8) & 0xFF;
    adv_service_uuid_len += 1;

    status = aci_gap_set_discoverable(ADV_IND,
                                      Min_Inter,
                                      Max_Inter,
                                      CFG_BLE_ADDRESS_TYPE,
                                      NO_WHITE_LIST_USE,
                                      sizeof(adv_local_name),
                                      (uint8_t*)adv_local_name,
                                      adv_service_uuid_len,
                                      adv_service_uuid,
                                      0,
                                      0);
    if (status != BLE_STATUS_SUCCESS)
    {
        adv_error("Set gap discoverable failed, status %d.", status);
        return -EIO;
    }

    status = aci_gap_update_adv_data(sizeof(adv_manuf_data), adv_manuf_data);
    if (status != BLE_STATUS_SUCCESS)
    {
        adv_error("Update gap adv data failed, status %d.", status);
        return -EIO;
    }

    if (state == ADV_STATE_FAST)
    {
        adv_info("Start fast advertising.");

        osStatus_t stat = osTimerStart(adv_manager_handle.timer,
                                       DEF_FAST_ADV_TIMEOUT * osKernelGetTickFreq() /
                                       1000);
        if (stat != osOK)
        {
            adv_error("Adv timer start failed, stat %d.", stat);
            return -EIO;
        }
    }
    else
    {
        adv_info("Start low power advertising.");
    }

    return 0;
}

/**
 * @brief   Probe the ADV manager.
 *
 * @param   obj Pointer to the ADV manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t adv_manager_probe(const object* obj)
{
    adv_manager_handle_t* handle = (adv_manager_handle_t*)obj->object_data;

    (void)memset(handle, 0, sizeof(adv_manager_handle_t));

    handle->timer = osTimerNew(
        adv_timer_callback,
        osTimerOnce,
        NULL,
        &adv_timer_attr);
    if (!handle->timer)
    {
        adv_error(
            "Manager <%s> create timer <%s> failed.",
            obj->name,
            adv_timer_attr.name);
        return -EINVAL;
    }
    else
    {
        adv_info(
            "Manager <%s> create timer <%s> succeed.",
            obj->name,
            adv_timer_attr.name);
    }

    adv_info("Manager <%s> probe succeed.", obj->name);

    return 0;
}

/**
 * @brief   Remove the ADV manager.
 *
 * @param   obj Pointer to the ADV manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t adv_manager_shutdown(const object* obj)
{
    adv_manager_handle_t* handle = (adv_manager_handle_t*)obj->object_data;

    (void)handle;

    adv_info("Manager <%s> shutdown succeed.", obj->name);

    return 0;
}

module_middleware(CONFIG_ADV_MANAGER_NAME,
                  CONFIG_ADV_MANAGER_LABEL,
                  adv_manager_probe,
                  adv_manager_shutdown,
                  NULL, &adv_manager_handle, NULL);
