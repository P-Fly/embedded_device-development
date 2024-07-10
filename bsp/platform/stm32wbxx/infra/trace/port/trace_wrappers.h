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

#ifndef __TRACE_WRAPPERS_H__
#define __TRACE_WRAPPERS_H__

#include <stddef.h>
#include <stdint.h>
#include "stm32wbxx.h"
#include "stm32wbxx_uart.h"

static inline int32_t uart_init(void)
{
    return stm32wbxx_uart1_init();
}

static inline int32_t uart_deinit(void)
{
    return stm32wbxx_uart1_deinit();
}

static inline int32_t uart_write(const void* tx_buf, int32_t tx_len)
{
    return stm32wbxx_uart1_write(tx_buf, tx_len);
}

static inline uint32_t sys_get_tick(void)
{
    return HAL_GetTick();
}

#endif /* __TRACE_WRAPPERS_H__ */
