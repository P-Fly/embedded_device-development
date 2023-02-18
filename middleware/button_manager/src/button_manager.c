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
#include "timers.h"
#include "framework.h"
#include "button_manager.h"
#include "button_manager_wrappers.h"

#define button_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define button_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define button_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define button_debug(str, ...)   pr_debug(str, ## __VA_ARGS__)

/**
 * @brief   Button manager handle definition.
 */
typedef struct
{
    TimerHandle_t       timer[BUTTON_ID_BUTT];
    uint32_t            press_down_count[BUTTON_ID_BUTT];

    button_user_clbk_t  user_clbk;
    const void*         user_ctx;
} button_manager_handle_t;

static button_manager_handle_t button_manager_handle;

/**
 * @brief   Button manager timer callback function.
 */
static void button_manager_timer_callback(TimerHandle_t xTimer)
{
    button_id_e id;
    button_state_e state;
    int32_t i;

    for (i = 0; i < BUTTON_ID_BUTT; i++)
    {
        if (xTimer == button_manager_handle.timer[i])
        {
            id = (button_id_e)i;
            state = button_get_state(id);

            if (state == BUTTON_STATE_DOWN)
            {
                if (button_manager_handle.press_down_count[i] < 100000)
                {
                    button_manager_handle.press_down_count[i]++;
                }
                else
                {
                    button_warning("Button %d was pressed too long.", id);
                }

                if (button_manager_handle.press_down_count[i] == 1)
                {
                    if (button_manager_handle.user_clbk)
                    {
                        button_manager_handle.user_clbk(id,
                                                        BUTTON_STATE_FIRST_DOWN,
                                                        button_manager_handle.user_ctx);
                    }
                }

                if (xTimerStart(button_manager_handle.timer[i], 0) != pdPASS)
                {
                    button_error("Button %d restart failed.", id);
                }
            }
            else if (state == BUTTON_STATE_UP)
            {
                button_manager_handle.press_down_count[i] = 0;

                if (button_manager_handle.user_clbk)
                {
                    button_manager_handle.user_clbk(id,
                                                    BUTTON_STATE_UP,
                                                    button_manager_handle.user_ctx);
                    button_manager_handle.user_clbk(id,
                                                    BUTTON_STATE_CLICK,
                                                    button_manager_handle.user_ctx);
                }
            }
        }
    }
}

/**
 * @brief   Callback handler from driver layer.
 *
 * @param   id Button ID.
 *
 * @retval  None.
 */
void button_manager_driver_clbk(button_id_e id)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t need_yield = pdFALSE;

    if (xTimerStopFromISR(button_manager_handle.timer[id],
                          &xHigherPriorityTaskWoken) != pdPASS)
    {
        button_error("Button %d stop failed.", id);
    }

    if (xHigherPriorityTaskWoken != pdFALSE)
    {
        need_yield = pdTRUE;
    }

    if (xTimerStartFromISR(button_manager_handle.timer[id],
                           &xHigherPriorityTaskWoken) != pdPASS)
    {
        button_error("Button %d start failed.", id);
    }

    if (xHigherPriorityTaskWoken != pdFALSE)
    {
        need_yield = pdTRUE;
    }

    portYIELD_FROM_ISR(need_yield);
}

/**
 * @brief   Register user callback.
 *
 * @param   user_clbk Pointer to the user callback handler.
 * @param   user_ctx Pointer to user context.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t button_manager_register_user_clbk(button_user_clbk_t    user_clbk,
                                          const void*           user_ctx)
{
    if (button_manager_handle.user_clbk != NULL)
    {
        return -EBUSY;
    }

    button_manager_handle.user_clbk = user_clbk;
    button_manager_handle.user_ctx = user_ctx;

    return 0;
}

/**
 * @brief   Unregister user callback.
 *
 * @retval  None.
 */
void button_manager_unregister_user_clbk(void)
{
    button_manager_handle.user_clbk = NULL;
    button_manager_handle.user_ctx = NULL;
}

/**
 * @brief   Probe the button manager.
 *
 * @param   obj Pointer to the button manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t button_manager_probe(const object* obj)
{
    button_manager_handle_t* handle =
        (button_manager_handle_t*)obj->object_data;
    int32_t i;

    (void)memset(handle, 0, sizeof(button_manager_handle_t));

    for (i = 0; i < BUTTON_ID_BUTT; i++)
    {
        handle->press_down_count[i] = 0;

        handle->timer[i] = xTimerCreate(
            CONFIG_BUTTON_MANAGER_TIMER_NAME,
            pdMS_TO_TICKS(20),
            pdFALSE,
            NULL,
            button_manager_timer_callback);
        if (!handle->timer[i])
        {
            button_error(
                "Manager <%s> create timer %d <%s> failed.",
                obj->name,
                i,
                CONFIG_BUTTON_MANAGER_TIMER_NAME);
            return -EINVAL;
        }
        else
        {
            button_info(
                "Manager <%s> create timer %d <%s> succeed.",
                obj->name,
                i,
                CONFIG_BUTTON_MANAGER_TIMER_NAME);
        }
    }

    button_init();

    button_info("Manager <%s> probe succeed.", obj->name);

    return 0;
}

/**
 * @brief   Remove the button manager.
 *
 * @param   obj Pointer to the button manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t button_manager_shutdown(const object* obj)
{
    button_manager_handle_t* handle =
        (button_manager_handle_t*)obj->object_data;
    BaseType_t ret;
    uint32_t i;

    button_deinit();

    for (i = 0; i < BUTTON_ID_BUTT; i++)
    {
        ret = xTimerDelete(handle->timer[i], 0);
        if (ret != pdPASS)
        {
            button_error(
                "Manager <%s> delete timer %d <%s> failed.",
                obj->name,
                i,
                CONFIG_BUTTON_MANAGER_TIMER_NAME);
            return -EINVAL;
        }
        else
        {
            button_info(
                "Manager <%s> delete timer %d <%s> succeed.",
                obj->name,
                i,
                CONFIG_BUTTON_MANAGER_TIMER_NAME);
        }
    }

    button_info("Manager <%s> shutdown succeed.", obj->name);

    return 0;
}

module_middleware(CONFIG_BUTTON_MANAGER_NAME,
                  CONFIG_BUTTON_MANAGER_LABEL,
                  button_manager_probe,
                  button_manager_shutdown,
                  NULL, &button_manager_handle, NULL);
