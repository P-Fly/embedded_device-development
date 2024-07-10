/**
 * @file signal_generator.h
 * @brief A simple signal generator.
 * @author Peter.Peng <27144363@qq.com>
 * @date 2022
 *
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

#ifndef __SIGNAL_GENERATOR_H__
#define __SIGNAL_GENERATOR_H__

/**
 * @defgroup signal_generator Signal Generator
 *
 * @brief A simple signal generator.
 *
 * @ingroup utils_group
 *
 * @{
 *
 */

#include <stddef.h>
#include <stdint.h>

/**
 * @brief   Signal Type for signal generator.
 */
typedef enum
{
    SG_TYPE_SIN_1K,
    SG_TYPE_SIN_10K,

    SG_TYPE_BUTT,
} sg_type_e;

/**
 * @brief   Sample rate for signal generator.
 */
typedef enum
{
    SG_SR_16K,
    SG_SR_48K,

    SG_SR_BUTT,
} sg_sample_rate_e;

/**
 * @brief   Bit width for signal generator.
 */
typedef enum
{
    SG_BIT_16   = 16,
    SG_BIT_24   = 24,
    SG_BIT_32   = 32,
} sg_bits_e;

/**
 * @brief   Getting size data of the signal is based on the current parameters.
 *
 * @param   type Signal Type.
 * @param   sample_rate Sample rate.
 * @param   bits Bit width.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
extern int32_t signal_generator_get_data_size(sg_type_e         type,
                                              sg_sample_rate_e  sample_rate,
                                              sg_bits_e         bits);

/**
 * @brief   Getting data of the signal.
 *
 * @param   type Signal Type.
 * @param   sample_rate Sample rate.
 * @param   bits Bit width.
 * @param   signal_buff Point to the signal buffer.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
extern int32_t signal_generator_get_data(sg_type_e          type,
                                         sg_sample_rate_e   sample_rate,
                                         sg_bits_e          bits,
                                         void*              signal_buff);

/**
 * @}
 */

#endif /* __SIGNAL_GENERATOR_H__ */
