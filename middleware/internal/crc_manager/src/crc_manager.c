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
#include "crc_manager.h"
#include "crc_manager_wrappers.h"

#define crc_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define crc_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define crc_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define crc_debug(str, ...)   pr_debug(str, ## __VA_ARGS__)

/**
 * @brief   CRC manager handle definition.
 */
typedef struct
{
    uint32_t reserved;
} crc_manager_handle_t;

static crc_manager_handle_t crc_manager_handle;

/**
 * @brief   Calculate the CRC value.
 *
 * @param   buf Pointer to data buffer to calculate.
 * @param   len Data length to calculate.
 * @param   crc Return the CRC value.
 * @param   config Pointer to the config data.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t crc_manager_calculate(const void*           buf,
                              int32_t               len,
                              uint32_t*             crc,
                              crc_manager_config_t* config)
{
    return crc_calculate(buf, len, crc, config);
}

/**
 * @brief   Probe the CRC manager.
 *
 * @param   obj Pointer to the CRC manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t crc_manager_probe(const object* obj)
{
    crc_manager_handle_t* handle = (crc_manager_handle_t*)obj->object_data;
    int32_t ret;

    (void)memset(handle, 0, sizeof(crc_manager_handle_t));

    ret = crc_init();
    if (ret)
    {
        crc_error("Manager <%s> initialize crc failed, ret %d.",
                  obj->name,
                  ret);

        return ret;
    }

    crc_info("Manager <%s> probe succeed.", obj->name);

    return 0;
}

/**
 * @brief   Remove the CRC manager.
 *
 * @param   obj Pointer to the CRC manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t crc_manager_shutdown(const object* obj)
{
    crc_manager_handle_t* handle = (crc_manager_handle_t*)obj->object_data;
    int32_t ret;

    (void)handle;

    ret = crc_deinit();
    if (ret)
    {
        crc_error("Manager <%s> deinitialize crc failed, ret %d.",
                  obj->name,
                  ret);
        return ret;
    }

    crc_info("Manager <%s> shutdown succeed.", obj->name);

    return 0;
}

module_middleware(CONFIG_CRC_MANAGER_NAME,
                  CONFIG_CRC_MANAGER_LABEL,
                  crc_manager_probe,
                  crc_manager_shutdown,
                  NULL, &crc_manager_handle, NULL);
