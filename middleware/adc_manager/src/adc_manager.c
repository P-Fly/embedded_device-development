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
#include "adc_manager.h"
#include "adc_manager_wrappers.h"

#define adc_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define adc_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define adc_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define adc_debug(str, ...)   pr_debug(str, ## __VA_ARGS__)

/**
 * @brief   ADC manager handle definition.
 */
typedef struct
{
    osTimerId_t     timer;

    adc_user_clbk_t user_clbk[ADC_ID_BUTT];
    const void*     user_ctx[ADC_ID_BUTT];
} adc_manager_handle_t;

static adc_manager_handle_t adc_manager_handle;

/**
 * @brief   Attributes structure for adc timer.
 */
const osTimerAttr_t adc_manager_timer_attr =
{
    .name       = CONFIG_ADC_MANAGER_TIMER_NAME,
    .attr_bits  = 0,
    .cb_mem     = NULL,
    .cb_size    = 0,
};

/**
 * @brief   ADC manager timer callback function.
 */
static void adc_manager_timer_callback(void* argument)
{
    osStatus_t stat;
    int32_t ret;

    ret = adc_start();
    if (ret)
    {
        adc_error("ADC manager start conv failed, ret %d.", ret);
    }

    stat = osTimerStart(adc_manager_handle.timer,
                        2 * osKernelGetTickFreq() / 1000);
    if (stat != osOK)
    {
        adc_error("ADC manager timer start failed, stat %d.", stat);
    }
}

/**
 * @brief   Conversion complete callback.
 *
 * @param   id ADC id.
 * @param   data ADC data.
 *
 * @retval  None.
 */
void adc_manager_driver_conv_cplt_clbk(adc_id_e id, uint16_t data)
{
    if (adc_manager_handle.user_clbk[id])
    {
        adc_manager_handle.user_clbk[id](id, data,
                                         adc_manager_handle.user_ctx[id]);
    }
}

/**
 * @brief   ADC error callback.
 *
 * @retval  None.
 */
void adc_manager_driver_error_clbk(void)
{
    adc_error("ADC manager driver error callback.");
}

/**
 * @brief   Register user callback.
 *
 * @param   id ADC id.
 * @param   user_clbk Pointer to the user callback handler.
 * @param   user_ctx Pointer to user context.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t adc_manager_register_user_clbk(adc_id_e         id,
                                       adc_user_clbk_t  user_clbk,
                                       const void*      user_ctx)
{
    if (adc_manager_handle.user_clbk[id] != NULL)
    {
        return -EBUSY;
    }

    adc_manager_handle.user_ctx[id] = user_ctx;
    adc_manager_handle.user_clbk[id] = user_clbk;

    return 0;
}

/**
 * @brief   Unregister user callback.
 *
 * @param   id ADC id.
 *
 * @retval  None.
 */
void adc_manager_unregister_user_clbk(adc_id_e id)
{
    adc_manager_handle.user_clbk[id] = NULL;
    adc_manager_handle.user_ctx[id] = NULL;
}

/**
 * @brief   Probe the ADC manager.
 *
 * @param   obj Pointer to the ADC manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t adc_manager_probe(const object* obj)
{
    adc_manager_handle_t* handle = (adc_manager_handle_t*)obj->object_data;
    osStatus_t stat;
    int32_t ret;

    (void)memset(handle, 0, sizeof(adc_manager_handle_t));

    ret = adc_init();
    if (ret)
    {
        return ret;
    }

    handle->timer = osTimerNew(
        adc_manager_timer_callback,
        osTimerOnce,
        NULL,
        &adc_manager_timer_attr);
    if (!handle->timer)
    {
        adc_error(
            "Manager <%s> create timer <%s> failed.",
            obj->name,
            adc_manager_timer_attr.name);
        return -EINVAL;
    }
    else
    {
        adc_info(
            "Manager <%s> create timer <%s> succeed.",
            obj->name,
            adc_manager_timer_attr.name);
    }

    stat = osTimerStart(adc_manager_handle.timer,
                        2 * osKernelGetTickFreq() / 1000);
    if (stat != osOK)
    {
        adc_error("ADC manager timer start failed, stat %d.", stat);
    }

    adc_info("Manager <%s> probe succeed.", obj->name);

    return 0;
}

/**
 * @brief   Remove the ADC manager.
 *
 * @param   obj Pointer to the ADC manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t adc_manager_shutdown(const object* obj)
{
    adc_manager_handle_t* handle = (adc_manager_handle_t*)obj->object_data;
    osStatus_t stat;
    int32_t ret;

    stat = osTimerDelete(handle->timer);
    if (stat != osOK)
    {
        adc_error(
            "Manager <%s> delete timer <%s> failed, stat %d",
            obj->name,
            adc_manager_timer_attr.name,
            stat);
        return -EINVAL;
    }
    else
    {
        adc_info(
            "Manager <%s> delete timer <%s> succeed.",
            obj->name,
            adc_manager_timer_attr.name);
    }

    ret = adc_deinit();
    if (ret)
    {
        return -EIO;
    }

    adc_info("Manager <%s> shutdown succeed.", obj->name);

    return 0;
}

module_middleware(CONFIG_ADC_MANAGER_NAME,
                  CONFIG_ADC_MANAGER_LABEL,
                  adc_manager_probe,
                  adc_manager_shutdown,
                  NULL, &adc_manager_handle, NULL);
