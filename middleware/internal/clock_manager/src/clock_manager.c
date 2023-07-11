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
#include "clock_manager.h"
#include "clock_manager_wrappers.h"

/**
 * @brief   Clock manager handle definition.
 */
typedef struct
{
    uint32_t reserved;
} clock_manager_handle_t;

static clock_manager_handle_t clock_manager_handle;

/**
 * @brief   Probe the clock manager.
 *
 * @param   obj Pointer to the clock manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t clock_manager_probe(const object* obj)
{
    clock_manager_handle_t* handle = (clock_manager_handle_t*)obj->object_data;
    int32_t ret;

    (void)memset(handle, 0, sizeof(clock_manager_handle_t));

    ret = clock_init();
    if (ret)
    {
        return -EIO;
    }

    return 0;
}

/**
 * @brief   Remove the clock manager.
 *
 * @param   obj Pointer to the clock manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t clock_manager_shutdown(const object* obj)
{
    clock_manager_handle_t* handle = (clock_manager_handle_t*)obj->object_data;
    int32_t ret;

    (void)handle;

    ret = clock_deinit();
    if (ret)
    {
        return -EIO;
    }

    return 0;
}

module_core(CONFIG_CLOCK_MANAGER_NAME,
            CONFIG_CLOCK_MANAGER_LABEL,
            clock_manager_probe,
            clock_manager_shutdown,
            NULL, &clock_manager_handle, NULL);
