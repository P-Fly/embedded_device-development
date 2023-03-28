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

#ifndef __RNG_MANAGER_WRAPPERS_H__
#define __RNG_MANAGER_WRAPPERS_H__

#include <stddef.h>
#include <stdint.h>
#include "stm32wbxx_rng.h"

static inline int32_t rng_init(void)
{
    return stm32wbxx_rng_init();
}

static inline int32_t rng_deinit(void)
{
    return stm32wbxx_rng_deinit();
}

static inline int32_t rng_generate_random_number(uint32_t* random_data)
{
    return stm32wbxx_rng_generate_random_number(random_data);
}

#endif /* __RNG_MANAGER_WRAPPERS_H__ */
