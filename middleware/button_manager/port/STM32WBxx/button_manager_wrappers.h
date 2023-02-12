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

#ifndef __BUTTON_MANAGER_WRAPPERS_H__
#define __BUTTON_MANAGER_WRAPPERS_H__

#include <stddef.h>
#include <stdint.h>
#include "stm32wbxx_button.h"

static inline void button_init(void)
{
    stm32wbxx_button_init();
}

static inline void button_deinit(void)
{
    stm32wbxx_button_deinit();
}

static inline button_state_e button_get_state(button_id_e id)
{
    return stm32wbxx_button_get_state(id);
}

#endif /* __BUTTON_MANAGER_WRAPPERS_H__ */
