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

#define display_error(str, ...)   pr_error(str, ##__VA_ARGS__)
#define display_warning(str, ...) pr_warning(str, ##__VA_ARGS__)
#define display_info(str, ...)    pr_info(str, ##__VA_ARGS__)
#define display_debug(str, ...)   //pr_debug(str, ##__VA_ARGS__)

/**
 * @brief   Private structure for display service.
 */
typedef struct
{
    int32_t reserved;
} display_service_priv_t;

/**
 * @brief   Initialize the display service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t display_service_init(const object* obj)
{
    display_service_priv_t* priv_data = service_get_priv_data(obj);

    (void)memset(priv_data, 0, sizeof(display_service_priv_t));

    display_info("Service <%s> initialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Deinitialize the display service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t display_service_deinit(const object* obj)
{
    display_service_priv_t* priv_data = service_get_priv_data(obj);

    display_info("Service <%s> deinitialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Handle the display service message.
 *
 * @param   obj Pointer to the service object handle.
 * @param   message Pointer to the received message.
 *
 * @retval  None.
 */
static void display_service_message_handler(const object*           obj,
                                            const message_t* const  message)
{
    display_service_priv_t* priv_data = service_get_priv_data(obj);

    display_debug("Service <%s> Received %s(0x%x): 0x%x, 0x%x, 0x%x, 0x%x.",
            obj->name,
            msg_id_to_name(message->id),
            message->id,
            message->param0,
            message->param1,
            message->param2,
            message->param3);
}

static display_service_priv_t display_service_priv;

static const service_config_t display_service_config =
{
    .thread_attr    =
    {
        .name       = CONFIG_DISPLAY_SERVICE_THREAD_NAME,
        .attr_bits  = osThreadDetached,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .stack_mem  = NULL,
        .stack_size = CONFIG_DISPLAY_SERVICE_THREAD_STACK_SIZE,
        .priority   = CONFIG_DISPLAY_SERVICE_THREAD_PRIORITY,
    },

    .queue_attr     =
    {
        .name       = CONFIG_DISPLAY_SERVICE_QUEUE_NAME,
        .attr_bits  = 0,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .mq_mem     = NULL,
        .mq_size    = 0,
    },

    .msg_count      = CONFIG_DISPLAY_SERVICE_MSG_COUNT,
};

DECLARE_SERVICE(CONFIG_DISPLAY_SERVICE_NAME,
                CONFIG_DISPLAY_SERVICE_LABEL,
                &display_service_priv,
                &display_service_config,
                display_service_init,
                display_service_deinit,
                display_service_message_handler);
