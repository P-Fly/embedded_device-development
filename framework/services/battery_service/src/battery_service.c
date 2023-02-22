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
#include <assert.h>

#include "cmsis_os.h"
#include "framework.h"
#include "battery_service.h"
#include "led_service.h"
#include "ui_service.h"

#define battery_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define battery_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define battery_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define battery_debug(str, ...)   //pr_debug(str, ##__VA_ARGS__)

#define BATTERY_SERVICE_VOLT_LEVEL_1 3100
#define BATTERY_SERVICE_VOLT_LEVEL_2 3000
#define BATTERY_SERVICE_VOLT_LEVEL_3 2900

static void battery_service_user_clbk(adc_id_e      id,
                                      uint16_t      data,
                                      const void*   user_ctx);

/**
 * @brief   Private structure for battery service.
 */
typedef struct
{
    uint32_t        enable;
    battery_state_e state;
    uint32_t        cnt;
} battery_service_priv_t;

static battery_service_priv_t battery_service_priv;

/**
 * @brief   Initialize the battery service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t battery_service_init(const object* obj)
{
    battery_service_priv_t* priv_data = service_get_priv_data(obj);
    int32_t ret;

    (void)memset(priv_data, 0, sizeof(battery_service_priv_t));

    priv_data->enable = 0;
    priv_data->state = BATTERY_STATE_BUTT;
    priv_data->cnt = 0;

    ret = adc_manager_register_user_clbk(ADC_ID_1,
                                         battery_service_user_clbk,
                                         &battery_service_priv);
    if (ret)
    {
        battery_error("Service <%s> register user callback failed, ret %d.",
                      obj->name,
                      ret);
        return ret;
    }

    battery_info("Service <%s> initialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Deinitialize the battery service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t battery_service_deinit(const object* obj)
{
    battery_service_priv_t* priv_data = service_get_priv_data(obj);

    (void)adc_manager_unregister_user_clbk(ADC_ID_1);

    battery_info("Service <%s> deinitialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Handle the battery service message.
 *
 * @param   obj Pointer to the service object handle.
 * @param   message Pointer to the received message.
 *
 * @retval  None.
 */
static void battery_service_message_handler(const object*           obj,
                                            const message_t* const  message)
{
    battery_service_priv_t* priv_data = service_get_priv_data(obj);

    battery_debug("Service <%s> Received %s(0x%x): 0x%x, 0x%x, 0x%x, 0x%x.",
                  obj->name,
                  msg_id_to_name(message->id),
                  message->id,
                  message->param0,
                  message->param1,
                  message->param2,
                  message->param3);

    switch (message->id)
    {
    case MSG_ID_SYS_STARTUP_COMPLETED:
    {
        battery_info("Service <%s> enable.", obj->name);

        priv_data->enable = 1;
    }
    break;
    }
}

int32_t battery_service_state_notify(battery_state_e state)
{
    message_t message;

    (void)memset(&message, 0, sizeof(message));

    message.id = MSG_ID_BATTERY_STATE_NOTIFY;
    message.param0 = state;

    return service_broadcast_message(&message);
}

static void battery_service_user_clbk(adc_id_e      id,
                                      uint16_t      data,
                                      const void*   user_ctx)
{
    battery_service_priv_t* priv_data = (battery_service_priv_t*)user_ctx;
    battery_state_e state = BATTERY_STATE_BUTT;
    uint32_t voltage = data;

    if (priv_data->enable != 1)
    {
        return;
    }

    if (priv_data->cnt == 0)
    {
        if (priv_data->state == BATTERY_STATE_LOW)
        {
            if (voltage >= BATTERY_SERVICE_VOLT_LEVEL_1)
            {
                state = BATTERY_STATE_NORMAL;
            }
        }
        else if (priv_data->state == BATTERY_STATE_NORMAL)
        {
            if (voltage < BATTERY_SERVICE_VOLT_LEVEL_3)
            {
                state = BATTERY_STATE_LOW;
            }
        }
        else
        {
            if (voltage >= BATTERY_SERVICE_VOLT_LEVEL_2)
            {
                state = BATTERY_STATE_NORMAL;
            }
            else
            {
                state = BATTERY_STATE_LOW;
            }
        }

        if (state != BATTERY_STATE_BUTT)
        {
            battery_info("Notify battery state %s(%d), current voltage %d.",
                         battery_state_to_name(state),
                         state,
                         voltage);

            priv_data->state = state;

            (void)battery_service_state_notify(state);

            if (state == BATTERY_STATE_NORMAL)
            {
                (void)led_service_setup_send(LED_ID_3, LED_TYPE_TURN_ON);
            }
            else if (state == BATTERY_STATE_LOW)
            {
                (void)led_service_setup_send(LED_ID_3, LED_TYPE_SLOW_FLASH);
            }
        }
    }

    priv_data->cnt++;

    priv_data->cnt = priv_data->cnt % 500;
}

static const service_config_t battery_service_config =
{
    .thread_attr    =
    {
        .name       = CONFIG_BATTERY_SERVICE_THREAD_NAME,
        .attr_bits  = osThreadDetached,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .stack_mem  = NULL,
        .stack_size = CONFIG_BATTERY_SERVICE_THREAD_STACK_SIZE,
        .priority   = CONFIG_BATTERY_SERVICE_THREAD_PRIORITY,
    },

    .queue_attr     =
    {
        .name       = CONFIG_BATTERY_SERVICE_QUEUE_NAME,
        .attr_bits  = 0,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .mq_mem     = NULL,
        .mq_size    = 0,
    },

    .msg_count      = CONFIG_BATTERY_SERVICE_MSG_COUNT,
};

DECLARE_SERVICE(CONFIG_BATTERY_SERVICE_NAME,
                CONFIG_BATTERY_SERVICE_LABEL,
                &battery_service_priv,
                &battery_service_config,
                battery_service_init,
                battery_service_deinit,
                battery_service_message_handler);
