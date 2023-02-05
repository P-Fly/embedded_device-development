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
#include "led_manager.h"

#define ui_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define ui_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define ui_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define ui_debug(str, ...)   pr_debug(str, ## __VA_ARGS__)

/**
 * @brief   Private structure for ui service.
 */
typedef struct
{
#if CONFIG_UI_SERVICE_MONITOR_TIMER_ENABLE == 1
    osTimerId_t monitor_timer;
    uint32_t    monitor_interval;
    uint32_t    monitor_value;
#endif
    int32_t     reserved;
} ui_service_priv_t;

#if CONFIG_UI_SERVICE_MONITOR_TIMER_ENABLE == 1
/**
 * @brief   Attributes structure for monitor timer.
 */
const osTimerAttr_t ui_service_monitor_timer_attr =
{
    .name       = CONFIG_UI_SERVICE_MONITOR_TIMER_NAME,
    .attr_bits  = 0,
    .cb_mem     = NULL,
    .cb_size    = 0,
};

/**
 * @brief   Monitor timer callback function.
 */
static void ui_service_monitor_timer_callback(void* argument)
{
    const object* obj = (const object*)argument;
    ui_service_priv_t* priv_data = service_get_priv_data(obj);
    message_t message;
    int32_t ret;

    (void)memset(&message, 0, sizeof(message));
    message.id = MSG_ID_SYS_HEARTBEAT;
    message.param0 = ++priv_data->monitor_value;
    ret = service_broadcast_message(&message);
    if (ret)
    {
        ui_error("Broadcast message %s(0x%x) failed, ret 0x%x.",
                 msg_id_to_name(message.id),
                 message.id,
                 ret);
    }
}
#endif

/**
 * @brief   Initialize the ui service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t ui_service_init(const object* obj)
{
    ui_service_priv_t* priv_data = service_get_priv_data(obj);

    (void)memset(priv_data, 0, sizeof(ui_service_priv_t));

#if CONFIG_UI_SERVICE_MONITOR_TIMER_ENABLE == 1
    priv_data->monitor_interval = CONFIG_UI_SERVICE_MONITOR_TIMER_INTERVAL;
    priv_data->monitor_value = 0;
    priv_data->monitor_timer = osTimerNew(
        ui_service_monitor_timer_callback,
        osTimerOnce,
        (void*)obj,
        &ui_service_monitor_timer_attr);
    if (!priv_data->monitor_timer)
    {
        ui_error(
            "Service <%s> create timer <%s> failed.",
            obj->name,
            ui_service_monitor_timer_attr.name);
        return -EINVAL;
    }
    else
    {
        ui_info(
            "Service <%s> create timer <%s> succeed.",
            obj->name,
            ui_service_monitor_timer_attr.name);
    }
#endif

    ui_info("Service <%s> initialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Deinitialize the ui service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t ui_service_deinit(const object* obj)
{
    ui_service_priv_t* priv_data = service_get_priv_data(obj);
    osStatus_t stat;

    (void)stat;

#if CONFIG_UI_SERVICE_MONITOR_TIMER_ENABLE == 1
    stat = osTimerDelete(priv_data->monitor_timer);
    if (stat != osOK)
    {
        ui_error(
            "Service <%s> delete timer <%s> failed, stat 0x%x",
            obj->name,
            ui_service_monitor_timer_attr.name,
            stat);
        return -EINVAL;
    }
    else
    {
        ui_info(
            "Service <%s> delete timer <%s> succeed.",
            obj->name,
            ui_service_monitor_timer_attr.name);
    }
#endif

    ui_info("Service <%s> deinitialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Handle the ui service message.
 *
 * @param   obj Pointer to the service object handle.
 * @param   message Pointer to the received message.
 *
 * @retval  None.
 */
static void ui_service_message_handler(const object*            obj,
                                       const message_t* const   message)
{
    ui_service_priv_t* priv_data = service_get_priv_data(obj);
    message_t send_message;
    int32_t ret;
    osStatus_t stat;

    (void)stat;

    ui_debug("Service <%s> Received %s(0x%x): 0x%x, 0x%x, 0x%x, 0x%x.",
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

#if CONFIG_UI_SERVICE_MONITOR_TIMER_ENABLE == 1
        stat = osTimerStart(priv_data->monitor_timer,
                            priv_data->monitor_interval * osKernelGetTickFreq() /
                            1000);
        if (stat != osOK)
        {
            ui_error(
                "Service <%s> start timer <%s> failed, stat %d.",
                obj->name,
                ui_service_monitor_timer_attr.name,
                stat);
        }
#endif

        (void)memset(&send_message, 0, sizeof(send_message));
        send_message.id = MSG_ID_LED_SETUP;
        send_message.param0 = LED_ID_1;
        send_message.param1 = LED_TYPE_QUICK_FLASH;
        ret = service_broadcast_message(&send_message);
        if (ret)
        {
            pr_error("Broadcast message %s(0x%x) failed, ret 0x%x.",
                     msg_id_to_name(send_message.id),
                     send_message.id,
                     ret);
        }
        else
        {
            pr_info("Broadcast message %s(0x%x) succeed.",
                    msg_id_to_name(send_message.id),
                    send_message.id);
        }

        send_message.id = MSG_ID_LED_SETUP;
        send_message.param0 = LED_ID_2;
        send_message.param1 = LED_TYPE_SLOW_FLASH;
        ret = service_broadcast_message(&send_message);
        if (ret)
        {
            pr_error("Broadcast message %s(0x%x) failed, ret 0x%x.",
                     msg_id_to_name(send_message.id),
                     send_message.id,
                     ret);
        }
        else
        {
            pr_info("Broadcast message %s(0x%x) succeed.",
                    msg_id_to_name(send_message.id),
                    send_message.id);
        }

        send_message.id = MSG_ID_LED_SETUP;
        send_message.param0 = LED_ID_3;
        send_message.param1 = LED_TYPE_QUICK_FLASH;
        ret = service_broadcast_message(&send_message);
        if (ret)
        {
            pr_error("Broadcast message %s(0x%x) failed, ret 0x%x.",
                     msg_id_to_name(send_message.id),
                     send_message.id,
                     ret);
        }
        else
        {
            pr_info("Broadcast message %s(0x%x) succeed.",
                    msg_id_to_name(send_message.id),
                    send_message.id);
        }

        break;

    case MSG_ID_SYS_HEARTBEAT:

#if CONFIG_UI_SERVICE_MONITOR_TIMER_ENABLE == 1
        stat = osTimerStart(priv_data->monitor_timer,
                            priv_data->monitor_interval * osKernelGetTickFreq() /
                            1000);
        if (stat != osOK)
        {
            ui_error(
                "Service <%s> start timer <%s> failed, stat %d.",
                obj->name,
                ui_service_monitor_timer_attr.name,
                stat);
        }
#endif
        break;
    }
}

static ui_service_priv_t ui_service_priv;

static const service_config_t ui_service_config =
{
    .thread_attr    =
    {
        .name       = CONFIG_UI_SERVICE_THREAD_NAME,
        .attr_bits  = osThreadDetached,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .stack_mem  = NULL,
        .stack_size = CONFIG_UI_SERVICE_THREAD_STACK_SIZE,
        .priority   = CONFIG_UI_SERVICE_THREAD_PRIORITY,
    },

    .queue_attr     =
    {
        .name       = CONFIG_UI_SERVICE_QUEUE_NAME,
        .attr_bits  = 0,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .mq_mem     = NULL,
        .mq_size    = 0,
    },

    .msg_count      = CONFIG_UI_SERVICE_MSG_COUNT,
};

DECLARE_SERVICE(CONFIG_UI_SERVICE_NAME,
                CONFIG_UI_SERVICE_LABEL,
                &ui_service_priv,
                &ui_service_config,
                ui_service_init,
                ui_service_deinit,
                ui_service_message_handler);
