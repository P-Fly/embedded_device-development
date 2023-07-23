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

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "cmsis_os.h"
#include "err.h"
#include "crc_manager.h"
#include "stm32wbxx.h"
#include "stm32wbxx_crc.h"
#include "middleware_conf.h"

/**
 * @brief   CRC handle definition.
 */
typedef struct
{
    CRC_HandleTypeDef crc;
} stm32wbxx_crc_handle_t;

static stm32wbxx_crc_handle_t stm32wbxx_crc_handle;

/**
 * @brief   Calculate the CRC value.
 *
 * @param   buf Pointer to data buffer to calculate.
 * @param   len Data length to calculate.
 * @param   crc Return the CRC value.
 * @param   config Pointer to the config data.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t stm32wbxx_crc_calculate(const void*             buf,
                                int32_t                 len,
                                uint32_t*               crc,
                                crc_manager_config_t*   config)
{
    if (!buf)
    {
        return -EINVAL;
    }

    if (len <= 0)
    {
        return -EINVAL;
    }

    if (!crc)
    {
        return -EINVAL;
    }

    if (!config)
    {
        return -EINVAL;
    }

    if (config->use_init_value)
    {
        stm32wbxx_crc_handle.crc.Init.DefaultInitValueUse =
            DEFAULT_INIT_VALUE_DISABLE;
        stm32wbxx_crc_handle.crc.Init.InitValue =
            config->init_value;
    }
    else
    {
        stm32wbxx_crc_handle.crc.Init.DefaultInitValueUse =
            DEFAULT_INIT_VALUE_ENABLE;
    }

    if (config->use_polynomial)
    {
        stm32wbxx_crc_handle.crc.Init.DefaultPolynomialUse =
            DEFAULT_POLYNOMIAL_DISABLE;
        stm32wbxx_crc_handle.crc.Init.GeneratingPolynomial =
            config->polynomial;

        switch (config->polynomial_size)
        {
        case CRC_POLYNOMIAL_SIZE_32B:
            stm32wbxx_crc_handle.crc.Init.CRCLength = CRC_POLYLENGTH_32B;
            break;

        case CRC_POLYNOMIAL_SIZE_16B:
            stm32wbxx_crc_handle.crc.Init.CRCLength = CRC_POLYLENGTH_16B;
            break;

        case CRC_POLYNOMIAL_SIZE_8B:
            stm32wbxx_crc_handle.crc.Init.CRCLength = CRC_POLYLENGTH_8B;
            break;

        case CRC_POLYNOMIAL_SIZE_7B:
            stm32wbxx_crc_handle.crc.Init.CRCLength = CRC_POLYLENGTH_7B;
            break;

        default:
            return -EINVAL;
        }
    }
    else
    {
        stm32wbxx_crc_handle.crc.Init.DefaultPolynomialUse =
            DEFAULT_POLYNOMIAL_ENABLE;
    }

    switch (config->input_reverse)
    {
    case CRC_INPUT_REVERSE_NONE:
        stm32wbxx_crc_handle.crc.Init.InputDataInversionMode =
            CRC_INPUTDATA_INVERSION_NONE;
        break;

    case CRC_INPUT_REVERSE_BYTE:
        stm32wbxx_crc_handle.crc.Init.InputDataInversionMode =
            CRC_INPUTDATA_INVERSION_BYTE;
        break;

    case CRC_INPUT_REVERSE_HALFWORD:
        stm32wbxx_crc_handle.crc.Init.InputDataInversionMode =
            CRC_INPUTDATA_INVERSION_HALFWORD;
        break;

    case CRC_INPUT_REVERSE_WORD:
        stm32wbxx_crc_handle.crc.Init.InputDataInversionMode =
            CRC_INPUTDATA_INVERSION_WORD;
        break;

    default:
        return -EINVAL;
    }

    switch (config->output_reverse)
    {
    case CRC_OUTPUT_REVERSE_DISABLE:
        stm32wbxx_crc_handle.crc.Init.OutputDataInversionMode =
            CRC_OUTPUTDATA_INVERSION_DISABLED;
        break;

    case CRC_OUTPUT_REVERSE_ENABLE:
        stm32wbxx_crc_handle.crc.Init.OutputDataInversionMode =
            CRC_OUTPUTDATA_INVERSION_ENABLED;
        break;

    default:
        return -EINVAL;
    }

    switch (config->input_format)
    {
    case CRC_INPUT_FORMAT_BYTES:
        stm32wbxx_crc_handle.crc.InputDataFormat =
            CRC_INPUTDATA_FORMAT_BYTES;
        break;

    case CRC_INPUT_FORMAT_HALFWORDS:
        stm32wbxx_crc_handle.crc.InputDataFormat =
            CRC_INPUTDATA_FORMAT_HALFWORDS;
        break;

    case CRC_INPUT_FORMAT_WORDS:
        stm32wbxx_crc_handle.crc.InputDataFormat =
            CRC_INPUTDATA_FORMAT_WORDS;
        break;

    default:
        return -EINVAL;
    }

    if (HAL_CRC_Init(&stm32wbxx_crc_handle.crc) != HAL_OK)
    {
        return -EIO;
    }

    if (config->use_init_value)
    {
        *crc = HAL_CRC_Accumulate(&stm32wbxx_crc_handle.crc, (uint32_t*)buf, len);
    }
    else
    {
        *crc = HAL_CRC_Calculate(&stm32wbxx_crc_handle.crc, (uint32_t*)buf, len);
    }

    return 0;
}

/**
 * @brief   Initialize the CRC driver.
 *
 * @retval  None.
 */
int32_t stm32wbxx_crc_init(void)
{
    (void)memset(&stm32wbxx_crc_handle, 0, sizeof(stm32wbxx_crc_handle_t));

    stm32wbxx_crc_handle.crc.Instance = CRC;

    return 0;
}

/**
 * @brief   Remove the CRC driver.
 *
 * @retval  None.
 */
int32_t stm32wbxx_crc_deinit(void)
{
    return 0;
}
