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
#include "button_service.h"

#define button_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define button_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define button_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define button_debug(str, ...)   //pr_debug(str, ##__VA_ARGS__)

static void button_service_user_clbk(button_id_e    id,
                                     button_state_e state,
                                     const void*    user_ctx);

/**
 * @brief   Private structure for button service.
 */
typedef struct
{
    int32_t reserved;
} button_service_priv_t;

static button_service_priv_t button_service_priv;

/**
 * @brief   Initialize the button service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t button_service_init(const object* obj)
{
    button_service_priv_t* priv_data = service_get_priv_data(obj);
    int32_t ret;

    (void)memset(priv_data, 0, sizeof(button_service_priv_t));

    ret = button_manager_register_user_clbk(button_service_user_clbk,
                                            &button_service_priv);
    if (ret)
    {
        button_error("Service <%s> register user callback failed, ret %d.",
                     obj->name,
                     ret);

        return ret;
    }

    button_info("Service <%s> initialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Deinitialize the button service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t button_service_deinit(const object* obj)
{
    button_service_priv_t* priv_data = service_get_priv_data(obj);

    button_manager_unregister_user_clbk();

    button_info("Service <%s> deinitialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Handle the button service message.
 *
 * @param   obj Pointer to the service object handle.
 * @param   message Pointer to the received message.
 *
 * @retval  None.
 */
static void button_service_message_handler(const object*            obj,
                                           const message_t* const   message)
{
    button_service_priv_t* priv_data = service_get_priv_data(obj);

    button_debug("Service <%s> Received %s(0x%x): 0x%x, 0x%x, 0x%x, 0x%x.",
                 obj->name,
                 msg_id_to_name(message->id),
                 message->id,
                 message->param0,
                 message->param1,
                 message->param2,
                 message->param3);
}

int32_t button_service_state_notify(button_id_e id, button_state_e state)
{
    message_t message;

    (void)memset(&message, 0, sizeof(message));

    message.id = MSG_ID_BUTTON_STATE_NOTIFY;
    message.param0 = id;
    message.param1 = state;

    return service_broadcast_message(&message);
}

static void button_service_user_clbk(button_id_e    id,
                                     button_state_e state,
                                     const void*    user_ctx)
{
    (void)user_ctx;

    button_info("Notify button %d state %s(%d).",
                id,
                button_state_to_name(state),
                state);

    (void)button_service_state_notify(id, state);
}

static const service_config_t button_service_config =
{
    .thread_attr    =
    {
        .name       = CONFIG_BUTTON_SERVICE_THREAD_NAME,
        .attr_bits  = osThreadDetached,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .stack_mem  = NULL,
        .stack_size = CONFIG_BUTTON_SERVICE_THREAD_STACK_SIZE,
        .priority   = CONFIG_BUTTON_SERVICE_THREAD_PRIORITY,
    },

    .queue_attr     =
    {
        .name       = CONFIG_BUTTON_SERVICE_QUEUE_NAME,
        .attr_bits  = 0,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .mq_mem     = NULL,
        .mq_size    = 0,
    },

    .msg_count      = CONFIG_BUTTON_SERVICE_MSG_COUNT,
};

DECLARE_SERVICE(CONFIG_BUTTON_SERVICE_NAME,
                CONFIG_BUTTON_SERVICE_LABEL,
                &button_service_priv,
                &button_service_config,
                button_service_init,
                button_service_deinit,
                button_service_message_handler);
