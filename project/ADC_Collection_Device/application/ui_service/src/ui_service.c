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
#include "ui_service.h"
#include "led_service.h"
#include "button_service.h"
#include "battery_service.h"

#define ui_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define ui_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define ui_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define ui_debug(str, ...)   //pr_debug(str, ## __VA_ARGS__)

static int32_t ui_service_system_sm_changed(system_sm_e new);

/**
 * @brief   Private structure for ui service.
 */
typedef struct
{
#if CONFIG_UI_SERVICE_MONITOR_TIMER_ENABLE == 1
    osTimerId_t monitor_timer;
    uint32_t    monitor_interval_millisec;
    uint32_t    monitor_value;
#endif

    system_sm_e system_state;
} ui_service_priv_t;

static ui_service_priv_t ui_service_priv;

#if CONFIG_UI_SERVICE_MONITOR_TIMER_ENABLE == 1
/**
 * @brief   Attributes structure for monitor timer.
 */
static const osTimerAttr_t ui_service_monitor_timer_attr =
{
    .name       = CONFIG_UI_SERVICE_MONITOR_TIMER_NAME,
    .attr_bits  = 0,
    .cb_mem     = NULL,
    .cb_size    = 0,
};

static int32_t ui_service_heartbeat_send(uint32_t count)
{
    message_t message;

    (void)memset(&message, 0, sizeof(message));

    message.id = MSG_ID_SYS_HEARTBEAT;
    message.param0 = count;

    return service_broadcast_message(&message);
}

/**
 * @brief   Monitor timer callback function.
 */
static void ui_service_monitor_timer_callback(void* argument)
{
    ui_service_priv_t* priv_data = (ui_service_priv_t*)argument;

    priv_data->monitor_value += 1;

    (void)ui_service_heartbeat_send(priv_data->monitor_value);
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

    priv_data->system_state = SYSTEM_SM_INITIALIZE;

#if CONFIG_UI_SERVICE_MONITOR_TIMER_ENABLE == 1
    priv_data->monitor_interval_millisec =
        CONFIG_UI_SERVICE_MONITOR_TIMER_INTERVAL_MILLISEC;
    priv_data->monitor_value = 0;
    priv_data->monitor_timer = osTimerNew(
        ui_service_monitor_timer_callback,
        osTimerOnce,
        (void*)priv_data,
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
            "Service <%s> delete timer <%s> failed, stat %d",
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
    {
#if CONFIG_UI_SERVICE_MONITOR_TIMER_ENABLE == 1
        osStatus_t stat;

        stat = osTimerStart(priv_data->monitor_timer,
                            priv_data->monitor_interval_millisec * osKernelGetTickFreq() /
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
    }
    break;

    case MSG_ID_SYS_HEARTBEAT:
    {
#if CONFIG_UI_SERVICE_MONITOR_TIMER_ENABLE == 1
        osStatus_t stat;

        stat = osTimerStart(priv_data->monitor_timer,
                            priv_data->monitor_interval_millisec * osKernelGetTickFreq() /
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
    }
    break;

    case MSG_ID_BUTTON_STATE_NOTIFY:
    {
        button_id_e button_id = (button_id_e)message->param0;
        button_state_e button_state = (button_state_e)message->param1;

        if (priv_data->system_state == SYSTEM_SM_IDLE)
        {
            if (button_id == BUTTON_ID_1 && button_state == BUTTON_STATE_CLICK)
            {
                ui_service_system_sm_changed(SYSTEM_SM_PREPARE);
            }
        }
        else if (priv_data->system_state == SYSTEM_SM_PREPARE)
        {
            if (button_id == BUTTON_ID_1 && button_state == BUTTON_STATE_CLICK)
            {
                ui_service_system_sm_changed(SYSTEM_SM_IDLE);
            }

            if (button_id == BUTTON_ID_2 && button_state == BUTTON_STATE_CLICK)
            {
                ui_service_system_sm_changed(SYSTEM_SM_RUNNING);
            }
        }
        else if (priv_data->system_state == SYSTEM_SM_RUNNING)
        {
            if (button_id == BUTTON_ID_1 && button_state == BUTTON_STATE_CLICK)
            {
                ui_service_system_sm_changed(SYSTEM_SM_IDLE);
            }

            if (button_id == BUTTON_ID_2 && button_state == BUTTON_STATE_CLICK)
            {
                ui_service_system_sm_changed(SYSTEM_SM_PREPARE);
            }
        }
    }
    break;

    case MSG_ID_BATTERY_STATE_NOTIFY:
    {
        battery_state_e battery_state = (battery_state_e)message->param0;

        if (battery_state == BATTERY_STATE_LOW)
        {
            ui_service_system_sm_changed(SYSTEM_SM_SHUTDOWN);
        }
        else
        {
            ui_service_system_sm_changed(SYSTEM_SM_IDLE);
        }
    }
    break;
    }
}

int32_t ui_service_startup_completed_send(void)
{
    message_t message;

    (void)memset(&message, 0, sizeof(message));

    message.id = MSG_ID_SYS_STARTUP_COMPLETED;

    return service_broadcast_message(&message);
}

static int32_t ui_service_system_sm_changed(system_sm_e new_state)
{
    message_t message;
    system_sm_e old_state;

    old_state = ui_service_priv.system_state;
    ui_service_priv.system_state = new_state;

    ui_info("system sm changed: %s(%d) -> %s(%d)",
            system_sm_to_name(old_state),
            old_state,
            system_sm_to_name(new_state),
            new_state);

    if (new_state == SYSTEM_SM_IDLE)
    {
        (void)led_service_setup_send(LED_ID_2, LED_TYPE_SLOW_FLASH);
    }
    else if (new_state == SYSTEM_SM_PREPARE)
    {
        (void)led_service_setup_send(LED_ID_2, LED_TYPE_QUICK_FLASH);
    }
    else if (new_state == SYSTEM_SM_RUNNING)
    {
        (void)led_service_setup_send(LED_ID_2, LED_TYPE_TURN_ON);
    }
    else
    {
        (void)led_service_setup_send(LED_ID_2, LED_TYPE_TURN_OFF);
    }

    (void)memset(&message, 0, sizeof(message));

    message.id = MSG_ID_SYS_SM_CHANGED;
    message.param0 = new_state;
    message.param1 = old_state;

    return service_broadcast_message(&message);
}

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
