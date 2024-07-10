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
#include "log.h"
#include "err.h"
#include "trace_impl.h"
#include "trace_wrappers.h"
#include "bsp_conf.h"

/**
 * @brief   Output the formatted data to UART.
 *
 * @retval  The number of data bytes write to the slave on success,
 *          negative error code otherwise.
 */
int32_t trace_output(const char* format, ...)
{
    char trace_buff[CONFIG_TRACE_MAX_LEN];
    int32_t len;
    int32_t ret;

    va_list args;

    va_start(args, format);
    len = vsnprintf(trace_buff, CONFIG_TRACE_MAX_LEN, format, args);
    va_end(args);

    ret = uart_write(trace_buff, len);
    if (ret < 0)
    {
        return -EIO;
    }

    return ret;
}

/**
 * @brief   Get sys tick.
 *
 * @retval  Sys tick value.
 */
uint32_t trace_get_tick(void)
{
    return sys_get_tick();
}

/**
 * @brief   Initialize the trace.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t trace_init(void)
{
    int32_t ret;

    ret = uart_init();
    if (ret)
    {
        return -EIO;
    }

    return 0;
}

/**
 * @brief   De-Initialize the trace.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t trace_deinit(void)
{
    int32_t ret;

    ret = uart_deinit();
    if (ret)
    {
        return -EIO;
    }

    return 0;
}
