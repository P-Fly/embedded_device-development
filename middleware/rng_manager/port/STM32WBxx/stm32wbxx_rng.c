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
#include "rng_manager.h"
#include "stm32wbxx.h"
#include "stm32wbxx_rng.h"

/**
 * @brief   RNG handle definition.
 */
typedef struct
{
    RNG_HandleTypeDef rng;
} stm32wbxx_rng_handle_t;

static stm32wbxx_rng_handle_t stm32wbxx_rng_handle;

/**
 * @brief   Initialize the RNG driver.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t stm32wbxx_rng_init(void)
{
    stm32wbxx_rng_handle.rng.Instance = RNG;
    stm32wbxx_rng_handle.rng.Init.ClockErrorDetection = RNG_CED_ENABLE;

    if (HAL_RNG_Init(&stm32wbxx_rng_handle.rng) != HAL_OK)
    {
        return -EIO;
    }

    return 0;
}

/**
 * @brief   Remove the RNG driver.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t stm32wbxx_rng_deinit(void)
{
    if (HAL_RNG_DeInit(&stm32wbxx_rng_handle.rng) != HAL_OK)
    {
        return -EIO;
    }

    return 0;
}

/**
 * @brief   Generates a 32-bit random number.
 *
 * @param   data Pointer to data buffer.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t stm32wbxx_rng_generate_random_number(uint32_t* random_data)
{
    *random_data = 0;

    if (HAL_RNG_GenerateRandomNumber(&stm32wbxx_rng_handle.rng,
                                     random_data) != HAL_OK)
    {
        return -EIO;
    }

    return 0;
}
