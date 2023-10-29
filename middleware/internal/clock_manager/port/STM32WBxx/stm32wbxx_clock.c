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
#include "clock_manager.h"
#include "stm32wbxx.h"
#include "stm32wbxx_clock.h"
#include "middleware_conf.h"

/**
 * @brief   Clock handle definition.
 */
typedef struct
{
    uint32_t reserved;
} stm32wbxx_clock_handle_t;

static stm32wbxx_clock_handle_t stm32wbxx_clock_handle;

/**
 * @brief   Peripherals clock enable.
 *
 * @retval  None.
 */
static void stm32wbxx_periph_clock_enable(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_ADC_CLK_ENABLE();
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
    __HAL_RCC_IPCC_CLK_ENABLE();
    __HAL_RCC_CRC_CLK_ENABLE();
    __HAL_RCC_RNG_CLK_ENABLE();
    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_RTCAPB_CLK_ENABLE();
}

/**
 * @brief   Peripherals clock disable.
 *
 * @retval  None.
 */
static void stm32wbxx_periph_clock_disable(void)
{
    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();
    __HAL_RCC_GPIOC_CLK_DISABLE();
    __HAL_RCC_GPIOD_CLK_DISABLE();
    __HAL_RCC_GPIOE_CLK_DISABLE();
    __HAL_RCC_USART1_CLK_DISABLE();
    __HAL_RCC_ADC_CLK_DISABLE();
    __HAL_RCC_DMAMUX1_CLK_DISABLE();
    __HAL_RCC_DMA1_CLK_DISABLE();
    __HAL_RCC_DMA2_CLK_DISABLE();
    __HAL_RCC_IPCC_CLK_DISABLE();
    __HAL_RCC_CRC_CLK_DISABLE();
    __HAL_RCC_RNG_CLK_DISABLE();
    __HAL_RCC_RTC_DISABLE();
    __HAL_RCC_RTCAPB_CLK_DISABLE();
}

/**
 * @brief   Initialize the clock driver.
 *
 * @retval  None.
 */
int32_t stm32wbxx_clock_init(void)
{
    (void)memset(&stm32wbxx_clock_handle, 0, sizeof(stm32wbxx_clock_handle));

    stm32wbxx_periph_clock_enable();

    return 0;
}

/**
 * @brief   Remove the clock driver.
 *
 * @retval  None.
 */
int32_t stm32wbxx_clock_deinit(void)
{
    stm32wbxx_periph_clock_disable();

    return 0;
}
