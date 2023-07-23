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

#ifndef __LED_MANAGER_WRAPPERS_H__
#define __LED_MANAGER_WRAPPERS_H__

#include <stddef.h>
#include <stdint.h>
#include "stm32wbxx_led.h"

static inline int32_t led_init(void)
{
    return stm32wbxx_led_init();
}

static inline int32_t led_deinit(void)
{
    return stm32wbxx_led_deinit();
}

static inline int32_t led_on(led_id_e id)
{
    return stm32wbxx_led_on(id);
}

static inline int32_t led_off(led_id_e id)
{
    return stm32wbxx_led_off(id);
}

static inline int32_t led_toggle(led_id_e id)
{
    return stm32wbxx_led_toggle(id);
}

#endif /* __LED_MANAGER_WRAPPERS_H__ */
