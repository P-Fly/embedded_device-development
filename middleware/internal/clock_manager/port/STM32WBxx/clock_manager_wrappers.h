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

#ifndef __CLOCK_MANAGER_WRAPPERS_H__
#define __CLOCK_MANAGER_WRAPPERS_H__

#include <stddef.h>
#include <stdint.h>
#include "stm32wbxx_clock.h"

static inline int32_t clock_init(void)
{
    return stm32wbxx_clock_init();
}

static inline int32_t clock_deinit(void)
{
    return stm32wbxx_clock_deinit();
}

#endif /* __CLOCK_MANAGER_WRAPPERS_H__ */
