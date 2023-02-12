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

#include "cmsis_version.h"
#include "cmsis_os.h"
#include "framework.h"
#include "stm32wbxx.h"

static void hardware_print_info(void);
static void hardware_clk_enable(void);

#define CONFIG_CPU_NAME   "STM32WBxx"
#define CONFIG_BOARD_NAME "P-NUCLEO-WB55"

/**
 * @brief   Startup hardware early.
 *
 * @retval  None.
 *
 * @note    This process is called before the OS start.
 */
void hardware_early_startup(void)
{
    HAL_Init();

    hardware_clk_enable();
}

/**
 * @brief   Startup hardware later.
 *
 * @retval  None.
 *
 * @note    This process is called after initialization thread is completed.
 */
void hardware_later_startup(void)
{
    hardware_print_info();
}

/**
 * @brief   Display some board info.
 *
 * @retval  None.
 */
static void hardware_print_info(void)
{
    char version[25];

    pr_info("");

    pr_info("*************************************************************");

    pr_info("%s - %s (Build %s %s)",
            CONFIG_ISSUE_NAME,
            CONFIG_ISSUE_VERSION,
            CONFIG_ISSUE_DATE,
            CONFIG_ISSUE_TIME);

    pr_info("CPU name: %s", CONFIG_CPU_NAME);

    pr_info("Board name: %s", CONFIG_BOARD_NAME);

#ifdef __ARMCC_VERSION
    pr_info("ARMCC version: 0x%08x", __ARMCC_VERSION);
#endif

    pr_info("CMSIS version: 0x%08x", __CM_CMSIS_VERSION);

    (void)osKernelGetInfo(NULL, version, sizeof(version));

    pr_info("OS version: %s", version);

    pr_info("System Clock: %d Hz", SystemCoreClock);

    pr_info("*************************************************************");

    pr_info("");
}

/**
 * @brief   Enable peripheral clk.
 *
 * @retval  None.
 */
static void hardware_clk_enable(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
}
