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

#include <string.h>
#include "cmsis_os.h"
#include "framework.h"

#define ble_error(str, ...)   pr_error(str, ##__VA_ARGS__)
#define ble_warning(str, ...) pr_warning(str, ##__VA_ARGS__)
#define ble_info(str, ...)    pr_info(str, ##__VA_ARGS__)
#define ble_debug(str, ...)   //pr_debug(str, ##__VA_ARGS__)

/**
 * @brief   Private structure for ble service.
 */
typedef struct
{
    int32_t reserved;
} ble_service_priv_t;

/**
 * @brief   Initialize the ble service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t ble_service_init(const object* obj)
{
    ble_service_priv_t* priv_data = service_get_priv_data(obj);

    (void)memset(priv_data, 0, sizeof(ble_service_priv_t));

    ble_info("Service <%s> initialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Deinitialize the ble service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t ble_service_deinit(const object* obj)
{
    ble_service_priv_t* priv_data = service_get_priv_data(obj);

    ble_info("Service <%s> deinitialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Handle the ble service message.
 *
 * @param   obj Pointer to the service object handle.
 * @param   message Pointer to the received message.
 *
 * @retval  None.
 */
static void ble_service_message_handler(const object*           obj,
                                        const message_t* const  message)
{
    ble_service_priv_t* priv_data = service_get_priv_data(obj);

    ble_debug("Service <%s> Received %s(0x%x): 0x%x, 0x%x, 0x%x, 0x%x.",
            obj->name,
            msg_id_to_name(message->id),
            message->id,
            message->param0,
            message->param1,
            message->param2,
            message->param3);
}

static ble_service_priv_t ble_service_priv;

static const service_config_t ble_service_config =
{
    .thread_attr    =
    {
        .name       = CONFIG_BLE_SERVICE_THREAD_NAME,
        .attr_bits  = osThreadDetached,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .stack_mem  = NULL,
        .stack_size = CONFIG_BLE_SERVICE_THREAD_STACK_SIZE,
        .priority   = CONFIG_BLE_SERVICE_THREAD_PRIORITY,
    },

    .queue_attr     =
    {
        .name       = CONFIG_BLE_SERVICE_QUEUE_NAME,
        .attr_bits  = 0,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .mq_mem     = NULL,
        .mq_size    = 0,
    },

    .msg_count      = CONFIG_BLE_SERVICE_MSG_COUNT,
};

DECLARE_SERVICE(CONFIG_BLE_SERVICE_NAME,
                CONFIG_BLE_SERVICE_LABEL,
                &ble_service_priv,
                &ble_service_config,
                ble_service_init,
                ble_service_deinit,
                ble_service_message_handler);
