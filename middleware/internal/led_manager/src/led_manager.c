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

typedef struct
{
    led_type_e  type;
    const char* name;
} led_manager_type_str_mapping_t;

static const led_manager_type_str_mapping_t led_manager_type_str_mapping[] =
{
    { LED_TYPE_TURN_OFF,    "TURN_OFF"    },
    { LED_TYPE_TURN_ON,     "TURN_ON"     },
    { LED_TYPE_QUICK_FLASH, "QUICK_FLASH" },
    { LED_TYPE_SLOW_FLASH,  "SLOW_FLASH"  },
};

const char* led_manager_type_to_str(led_type_e type)
{
    uint32_t i;

    for (i = 0;
         i <
         sizeof(led_manager_type_str_mapping) /
         sizeof(led_manager_type_str_mapping[0]);
         i++)
    {
        if (led_manager_type_str_mapping[i].type == type)
        {
            return led_manager_type_str_mapping[i].name;
        }
    }

    return "UNKNOW";
}

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
    int32_t ret;

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

        ret = led_on(id);
        if (ret)
        {
            led_error("Led manager turn on failed, ret %d.", ret);
            return ret;
        }

        led_manager_handle.interval_millisec[id] =
            CONFIG_LED_MANAGER_QUICK_FLASH_INTERVAL_MS;

        stat = osTimerStart(led_manager_handle.timer[id],
                            led_manager_handle.interval_millisec[id] * osKernelGetTickFreq() /
                            1000);
        if (stat != osOK)
        {
            led_error("Led manager timer %d start failed, stat %d.", id, stat);
        }

        break;

    case LED_TYPE_SLOW_FLASH:

        ret = led_on(id);
        if (ret)
        {
            led_error("Led manager turn on failed, ret %d.", ret);
            return ret;
        }

        led_manager_handle.interval_millisec[id] =
            CONFIG_LED_MANAGER_SLOW_FLASH_INTERVAL_MS;

        stat = osTimerStart(led_manager_handle.timer[id],
                            led_manager_handle.interval_millisec[id] * osKernelGetTickFreq() /
                            1000);
        if (stat != osOK)
        {
            led_error("Led manager timer %d start failed, stat %d.", id, stat);
        }

        break;

    case LED_TYPE_TURN_ON:

        ret = led_on(id);
        if (ret)
        {
            led_error("Led manager turn on failed, ret %d.", ret);
            return ret;
        }

        led_manager_handle.interval_millisec[id] = 0;

        break;

    case LED_TYPE_TURN_OFF:

        ret = led_off(id);
        if (ret)
        {
            led_error("Led manager turn off failed, ret %d.", ret);
            return ret;
        }


        led_manager_handle.interval_millisec[id] = 0;

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
    int32_t ret;

    ret = led_toggle((led_id_e)id);
    if (ret)
    {
        led_error("Led manager toggle failed, ret %d.", ret);
    }

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
    int32_t ret;
    uint32_t i;

    (void)memset(handle, 0, sizeof(led_manager_handle_t));

    ret = led_init();
    if (ret)
    {
        led_error("Manager <%s> initialize led failed, ret %d.",
                  obj->name,
                  ret);

        return ret;
    }

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
    int32_t ret;
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
    }

    ret = led_deinit();
    if (ret)
    {
        led_error("Manager <%s> deinitialize led failed, ret %d.",
                  obj->name,
                  ret);
        return ret;
    }

    led_info("Manager <%s> shutdown succeed.", obj->name);

    return 0;
}

module_middleware(CONFIG_LED_MANAGER_NAME,
                  CONFIG_LED_MANAGER_LABEL,
                  led_manager_probe,
                  led_manager_shutdown,
                  NULL, &led_manager_handle, NULL);
