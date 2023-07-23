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

#ifndef __CRC_MANAGER_H__
#define __CRC_MANAGER_H__

typedef enum
{
    CRC_POLYNOMIAL_SIZE_32B = 0,
    CRC_POLYNOMIAL_SIZE_16B,
    CRC_POLYNOMIAL_SIZE_8B,
    CRC_POLYNOMIAL_SIZE_7B,

    CRC_POLYNOMIAL_SIZE_BUTT,
} crc_polynomial_size_e;

typedef enum
{
    CRC_INPUT_FORMAT_BYTES = 0,
    CRC_INPUT_FORMAT_HALFWORDS,
    CRC_INPUT_FORMAT_WORDS,

    CRC_INPUT_FORMAT_BUTT,
} crc_input_format_e;

typedef enum
{
    CRC_INPUT_REVERSE_NONE = 0,
    CRC_INPUT_REVERSE_BYTE,
    CRC_INPUT_REVERSE_HALFWORD,
    CRC_INPUT_REVERSE_WORD,

    CRC_INPUT_REVERSE_BUTT,
} crc_input_reverse_e;

typedef enum
{
    CRC_OUTPUT_REVERSE_DISABLE = 0,
    CRC_OUTPUT_REVERSE_ENABLE,

    CRC_OUTPUT_REVERSE_BUTT,
} crc_output_reverse_e;

typedef struct
{
    uint8_t                 use_init_value;
    uint32_t                init_value;

    uint8_t                 use_polynomial;
    uint32_t                polynomial;
    crc_polynomial_size_e   polynomial_size;

    crc_input_format_e      input_format;
    crc_input_reverse_e     input_reverse;

    crc_output_reverse_e    output_reverse;
} crc_manager_config_t;

extern int32_t crc_manager_calculate(const void*            buf,
                                     int32_t                len,
                                     uint32_t*              crc,
                                     crc_manager_config_t*  config);

#endif /* __CRC_MANAGER_H__ */
