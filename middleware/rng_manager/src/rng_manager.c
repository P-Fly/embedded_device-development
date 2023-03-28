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
#include "rng_manager.h"
#include "rng_manager_wrappers.h"

#define rng_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define rng_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define rng_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define rng_debug(str, ...)   pr_debug(str, ## __VA_ARGS__)

/**
 * @brief   RNG manager handle definition.
 */
typedef struct
{
    uint32_t reserved;
} rng_manager_handle_t;

static rng_manager_handle_t rng_manager_handle;

int32_t rng_manager_generate_random_number(uint32_t* random_data)
{
    int32_t ret;

    if (!random_data)
    {
        return -EINVAL;
    }

    ret = rng_generate_random_number(random_data);
    if (ret)
    {
        rng_error("Generate random number failed, ret %d.", ret);

        return ret;
    }

    return 0;
}

/**
 * @brief   Probe the RNG manager.
 *
 * @param   obj Pointer to the RNG manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t rng_manager_probe(const object* obj)
{
    rng_manager_handle_t* handle = (rng_manager_handle_t*)obj->object_data;
    int32_t ret;

    (void)memset(handle, 0, sizeof(rng_manager_handle_t));

    ret = rng_init();
    if (ret)
    {
        rng_error("Manager <%s> initialize rng failed, ret %d.", obj->name,
                  ret);

        return ret;
    }

    rng_info("Manager <%s> probe succeed.", obj->name);

    return 0;
}

/**
 * @brief   Remove the RNG manager.
 *
 * @param   obj Pointer to the RNG manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t rng_manager_shutdown(const object* obj)
{
    rng_manager_handle_t* handle = (rng_manager_handle_t*)obj->object_data;
    int32_t ret;

    (void)handle;

    ret = rng_deinit();
    if (ret)
    {
        rng_error("Manager <%s> deinitialize rng failed, ret %d.",
                  obj->name,
                  ret);

        return ret;
    }

    rng_info("Manager <%s> shutdown succeed.", obj->name);

    return 0;
}

module_middleware(CONFIG_RNG_MANAGER_NAME,
                  CONFIG_RNG_MANAGER_LABEL,
                  rng_manager_probe,
                  rng_manager_shutdown,
                  NULL, &rng_manager_handle, NULL);
