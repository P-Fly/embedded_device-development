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

#define key_error(str, ...)   pr_error(str, ##__VA_ARGS__)
#define key_warning(str, ...) pr_warning(str, ##__VA_ARGS__)
#define key_info(str, ...)    pr_info(str, ##__VA_ARGS__)
#define key_debug(str, ...)   //pr_debug(str, ##__VA_ARGS__)

/**
 * @brief   Private structure for key service.
 */
typedef struct
{
    int32_t reserved;
} key_service_priv_t;

/**
 * @brief   Initialize the key service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t key_service_init(const object* obj)
{
    key_service_priv_t* priv_data = service_get_priv_data(obj);

    (void)memset(priv_data, 0, sizeof(key_service_priv_t));

    key_info("Service <%s> initialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Deinitialize the key service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t key_service_deinit(const object* obj)
{
    key_service_priv_t* priv_data = service_get_priv_data(obj);

    key_info("Service <%s> deinitialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Handle the key service message.
 *
 * @param   obj Pointer to the service object handle.
 * @param   message Pointer to the received message.
 *
 * @retval  None.
 */
static void key_service_message_handler(const object*           obj,
                                        const message_t* const  message)
{
    key_service_priv_t* priv_data = service_get_priv_data(obj);

    key_debug("Service <%s> Received %s(0x%x): 0x%x, 0x%x, 0x%x, 0x%x.",
            obj->name,
            msg_id_to_name(message->id),
            message->id,
            message->param0,
            message->param1,
            message->param2,
            message->param3);
}

static key_service_priv_t key_service_priv;

static const service_config_t key_service_config =
{
    .thread_attr    =
    {
        .name       = CONFIG_KEY_SERVICE_THREAD_NAME,
        .attr_bits  = osThreadDetached,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .stack_mem  = NULL,
        .stack_size = CONFIG_KEY_SERVICE_THREAD_STACK_SIZE,
        .priority   = CONFIG_KEY_SERVICE_THREAD_PRIORITY,
    },

    .queue_attr     =
    {
        .name       = CONFIG_KEY_SERVICE_QUEUE_NAME,
        .attr_bits  = 0,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .mq_mem     = NULL,
        .mq_size    = 0,
    },

    .msg_count      = CONFIG_KEY_SERVICE_MSG_COUNT,
};

DECLARE_SERVICE(CONFIG_KEY_SERVICE_NAME,
                CONFIG_KEY_SERVICE_LABEL,
                &key_service_priv,
                &key_service_config,
                key_service_init,
                key_service_deinit,
                key_service_message_handler);
