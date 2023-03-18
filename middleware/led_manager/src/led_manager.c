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
#include "led_manager.h"
#include "led_manager_wrappers.h"

#define led_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define led_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define led_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define led_debug(str, ...)   pr_debug(str, ## __VA_ARGS__)

/**
 * @brief   Led manager handle definition.
 */
typedef struct
{
    led_type_e  type[LED_ID_BUTT];
    osTimerId_t timer[LED_ID_BUTT];
    uint32_t    interval_millisec[LED_ID_BUTT];
} led_manager_handle_t;

static led_manager_handle_t led_manager_handle;

/**
 * @brief   Attributes structure for led timer.
 */
const osTimerAttr_t led_manager_timer_attr =
{
    .name       = CONFIG_LED_MANAGER_TIMER_NAME,
    .attr_bits  = 0,
    .cb_mem     = NULL,
    .cb_size    = 0,
};

/**
 * @brief   Setup led type.
 *
 * @param   id Led id.
 * @param   Type Led type.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t led_manager_setup(led_id_e id, led_type_e type)
{
    osStatus_t stat;

    if (id >= LED_ID_BUTT)
    {
        return -EINVAL;
    }

    if (type >= LED_TYPE_BUTT)
    {
        return -EINVAL;
    }

    (void)osTimerStop(led_manager_handle.timer[id]);

    switch (type)
    {
    case LED_TYPE_QUICK_FLASH:
        led_on(id);
        led_manager_handle.interval_millisec[id] = 300;
        stat = osTimerStart(led_manager_handle.timer[id],
                            led_manager_handle.interval_millisec[id] * osKernelGetTickFreq() /
                            1000);
        if (stat != osOK)
        {
            led_error("Led manager timer %d start failed, stat %d.", id, stat);
        }
        break;

    case LED_TYPE_SLOW_FLASH:
        led_on(id);
        led_manager_handle.interval_millisec[id] = 1000;
        stat = osTimerStart(led_manager_handle.timer[id],
                            led_manager_handle.interval_millisec[id] * osKernelGetTickFreq() /
                            1000);
        if (stat != osOK)
        {
            led_error("Led manager timer %d start failed, stat %d.", id, stat);
        }
        break;

    case LED_TYPE_TURN_ON:
        led_on(id);
        break;

    case LED_TYPE_TURN_OFF:
        led_off(id);
        break;
    }

    led_manager_handle.type[id] = type;

    return 0;
}

/**
 * @brief   Led manager timer callback function.
 */
static void led_manager_timer_callback(void* argument)
{
    int32_t id = (int32_t)argument;
    osStatus_t stat;

    led_toggle((led_id_e)id);

    stat = osTimerStart(led_manager_handle.timer[id],
                        led_manager_handle.interval_millisec[id] * osKernelGetTickFreq() /
                        1000);
    if (stat != osOK)
    {
        led_error("Led manager timer %d restart failed, stat %d.", id, stat);
    }
}

/**
 * @brief   Probe the led manager.
 *
 * @param   obj Pointer to the led manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t led_manager_probe(const object* obj)
{
    led_manager_handle_t* handle = (led_manager_handle_t*)obj->object_data;
    int32_t i;

    (void)memset(handle, 0, sizeof(led_manager_handle_t));

    led_init();

    for (i = 0; i < LED_ID_BUTT; i++)
    {
        handle->timer[i] = osTimerNew(
            led_manager_timer_callback,
            osTimerOnce,
            (void*)i,
            &led_manager_timer_attr);
        if (!handle->timer[i])
        {
            led_error(
                "Manager <%s> create timer %d <%s> failed.",
                obj->name,
                i,
                led_manager_timer_attr.name);
            return -EINVAL;
        }
        else
        {
            led_info(
                "Manager <%s> create timer %d <%s> succeed.",
                obj->name,
                i,
                led_manager_timer_attr.name);
        }
    }

    led_info("Manager <%s> probe succeed.", obj->name);

    return 0;
}

/**
 * @brief   Remove the led manager.
 *
 * @param   obj Pointer to the led manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t led_manager_shutdown(const object* obj)
{
    led_manager_handle_t* handle = (led_manager_handle_t*)obj->object_data;
    osStatus_t stat;
    uint32_t i;

    for (i = 0; i < LED_ID_BUTT; i++)
    {
        stat = osTimerDelete(handle->timer[i]);
        if (stat != osOK)
        {
            led_error(
                "Manager <%s> delete timer %d <%s> failed, stat %d",
                obj->name,
                i,
                led_manager_timer_attr.name,
                stat);
            return -EINVAL;
        }
        else
        {
            led_info(
                "Manager <%s> delete timer %d <%s> succeed.",
                obj->name,
                i,
                led_manager_timer_attr.name);
        }
    }

    led_deinit();

    led_info("Manager <%s> shutdown succeed.", obj->name);

    return 0;
}

module_middleware(CONFIG_LED_MANAGER_NAME,
                  CONFIG_LED_MANAGER_LABEL,
                  led_manager_probe,
                  led_manager_shutdown,
                  NULL, &led_manager_handle, NULL);
