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

#ifndef __STM32WBXX_CRC_H__
#define __STM32WBXX_CRC_H__

#include <stddef.h>
#include <stdint.h>
#include "crc_manager.h"

extern int32_t stm32wbxx_crc_init(void);
extern int32_t stm32wbxx_crc_deinit(void);
extern int32_t stm32wbxx_crc_calculate(const void*              buf,
                                       int32_t                  len,
                                       uint32_t*                crc,
                                       crc_manager_config_t*    config);

#endif /* __STM32WBXX_CRC_H__ */
