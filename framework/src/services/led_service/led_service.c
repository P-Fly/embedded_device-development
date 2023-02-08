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
#include "led_service.h"

#define led_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define led_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define led_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define led_debug(str, ...)   pr_debug(str, ## __VA_ARGS__)

/**
 * @brief   Private structure for led service.
 */
typedef struct
{
    int32_t reserved;
} led_service_priv_t;

/**
 * @brief   Initialize the led service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t led_service_init(const object* obj)
{
    led_service_priv_t* priv_data = service_get_priv_data(obj);

    (void)memset(priv_data, 0, sizeof(led_service_priv_t));

    led_info("Service <%s> initialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Deinitialize the led service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t led_service_deinit(const object* obj)
{
    led_service_priv_t* priv_data = service_get_priv_data(obj);

    led_info("Service <%s> deinitialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Handle the led service message.
 *
 * @param   obj Pointer to the service object handle.
 * @param   message Pointer to the received message.
 *
 * @retval  None.
 */
static void led_service_message_handler(const object*           obj,
                                        const message_t* const  message)
{
    led_service_priv_t* priv_data = service_get_priv_data(obj);
    led_id_e id;
    led_type_e type;
    int32_t ret;

    led_debug("Service <%s> Received %s(0x%x): 0x%x, 0x%x, 0x%x, 0x%x.",
              obj->name,
              msg_id_to_name(message->id),
              message->id,
              message->param0,
              message->param1,
              message->param2,
              message->param3);

    switch (message->id)
    {
    case MSG_ID_LED_SETUP:

        id = (led_id_e)message->param0;
        type = (led_type_e)message->param1;

        ret = led_manager_setup(id, type);
        if (ret)
        {
            pr_error("Service <%s> setup led %d, type %d failed, ret 0x%x.",
                     obj->name,
                     id,
                     type,
                     ret);
        }
        else
        {
            pr_info("Service <%s> setup led %d, type %d succeed.",
                    obj->name,
                    id,
                    type);
        }

        break;
    }
}

int32_t led_service_setup_send(led_id_e id, led_type_e type)
{
    message_t message;

    (void)memset(&message, 0, sizeof(message));

    message.id = MSG_ID_LED_SETUP;
    message.param0 = id;
    message.param1 = type;

    return service_broadcast_message(&message);
}

static led_service_priv_t led_service_priv;

static const service_config_t led_service_config =
{
    .thread_attr    =
    {
        .name       = CONFIG_LED_SERVICE_THREAD_NAME,
        .attr_bits  = osThreadDetached,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .stack_mem  = NULL,
        .stack_size = CONFIG_LED_SERVICE_THREAD_STACK_SIZE,
        .priority   = CONFIG_LED_SERVICE_THREAD_PRIORITY,
    },

    .queue_attr     =
    {
        .name       = CONFIG_LED_SERVICE_QUEUE_NAME,
        .attr_bits  = 0,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .mq_mem     = NULL,
        .mq_size    = 0,
    },

    .msg_count      = CONFIG_LED_SERVICE_MSG_COUNT,
};

DECLARE_SERVICE(CONFIG_LED_SERVICE_NAME,
                CONFIG_LED_SERVICE_LABEL,
                &led_service_priv,
                &led_service_config,
                led_service_init,
                led_service_deinit,
                led_service_message_handler);
