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

#ifndef __STM32WBXX_BUTTON_H__
#define __STM32WBXX_BUTTON_H__

#include <stddef.h>
#include <stdint.h>
#include "button_manager.h"

extern void stm32wbxx_button_init(void);
extern void stm32wbxx_button_deinit(void);
extern button_state_e stm32wbxx_button_get_state(button_id_e id);

#endif /* __STM32WBXX_BUTTON_H__ */
