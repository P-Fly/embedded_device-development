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

#ifndef __BSP_CONF_H__
#define __BSP_CONF_H__

#define CONFIG_CPU_NAME   "STM32WBxx"
#define CONFIG_BOARD_NAME "P-NUCLEO-WB55"

#define CONFIG_UART1_HW_BAUDRATE 2000000
#define CONFIG_UART1_TX_RING_BUFF_SIZE (8 * 1024)
#define CONFIG_UART1_RX_RING_BUFF_SIZE (2 * 1024)

#define CONFIG_TRACE_MAX_LEN 256

#endif /* __BSP_CONF_H__ */
