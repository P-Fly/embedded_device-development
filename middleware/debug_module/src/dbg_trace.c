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
#include "framework.h"
#include "dbg_trace.h"
#include "dbg_module_wrappers.h"

/**
 * @brief   Dbg trace handle definition.
 */
typedef struct
{
    uint32_t reserved;
} dbg_trace_handle_t;

static dbg_trace_handle_t dbg_trace_handle;

/**
 * @brief   Output the formatted data to UART.
 *
 * @retval  The number of data bytes write to the slave on success,
 *          negative error code otherwise.
 */
int32_t dbg_trace_output(const char* format, ...)
{
    char trace_buff[CONFIG_DBG_TRACE_MAX_LEN];
    int32_t len;
    int32_t ret;

    va_list args;

    va_start(args, format);
    len = vsnprintf(trace_buff, CONFIG_DBG_TRACE_MAX_LEN, format, args);
    va_end(args);

    ret = uart_write(trace_buff, len);
    if (ret < 0)
    {
        return -EIO;
    }

    return ret;
}

/**
 * @brief   Probe the dbg trace.
 *
 * @param   obj Pointer to the dbg trace object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t dbg_trace_probe(const object* obj)
{
    dbg_trace_handle_t* handle = (dbg_trace_handle_t*)obj->object_data;
    int32_t ret;

    (void)memset(handle, 0, sizeof(dbg_trace_handle_t));

    ret = uart_init();
    if (ret)
    {
        return -EIO;
    }

    return 0;
}

/**
 * @brief   Remove the dbg trace.
 *
 * @param   obj Pointer to the dbg trace object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t dbg_trace_shutdown(const object* obj)
{
    dbg_trace_handle_t* handle = (dbg_trace_handle_t*)obj->object_data;
    int32_t ret;

    (void)handle;

    ret = uart_deinit();
    if (ret)
    {
        return -EIO;
    }

    return 0;
}

module_early_driver(CONFIG_DBG_TRACE_NAME,
                    CONFIG_DBG_TRACE_LABEL,
                    dbg_trace_probe,
                    dbg_trace_shutdown,
                    NULL, &dbg_trace_handle, NULL);
