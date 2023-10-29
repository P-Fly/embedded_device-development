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

#include <string.h>

#include "cmsis_version.h"
#include "cmsis_os.h"
#include "framework.h"
#include "stm32wbxx.h"

static void hardware_print_info(void);
static void hardware_system_clock_config(void);
static void hardware_periph_clock_config(void);

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

    hardware_system_clock_config();

    hardware_periph_clock_config();
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
    char os_version[20];

    (void)osKernelGetInfo(NULL, os_version, sizeof(os_version));

    pr_info("");
    pr_info("*************************************************************");
    pr_info("");
    pr_info("%s - %s (Build %s %s)",
            CONFIG_ISSUE_NAME,
            CONFIG_ISSUE_VERSION,
            CONFIG_ISSUE_DATE,
            CONFIG_ISSUE_TIME);
    pr_info("");
    pr_info("%-15s: %s", "CPU name", CONFIG_CPU_NAME);
    pr_info("%-15s: 0x%08X", "Device number", LL_FLASH_GetUDN());
    pr_info("%-15s: 0x%08X", "Device ID", LL_FLASH_GetDeviceID());
    pr_info("%-15s: 0x%08X", "ST company ID", LL_FLASH_GetSTCompanyID());
    pr_info("%-15s: %s", "Board name", CONFIG_BOARD_NAME);
#ifdef __ARMCC_VERSION
    pr_info("%-15s: 0x%08X", "ARMCC version", __ARMCC_VERSION);
#endif
    pr_info("%-15s: 0x%08X", "CMSIS version", __CM_CMSIS_VERSION);
    pr_info("%-15s: %s", "OS version", os_version);
    pr_info("");
    pr_info("%-15s: 0x%08X - 0x%08X",
            "FLASH",
            FLASH_BASE,
            FLASH_BASE + FLASH_SIZE - 1);
    pr_info("%-15s: 0x%08X - 0x%08X",
            "SYSTEM MEMORY",
            SYSTEM_MEMORY_BASE,
            SYSTEM_MEMORY_END_ADDR);
    pr_info("%-15s: 0x%08X - 0x%08X",
            "OTP AREA",
            OTP_AREA_BASE,
            OTP_AREA_END_ADDR);
    pr_info("%-15s: 0x%08X - 0x%08X",
            "ENGI BYTE",
            ENGI_BYTE_BASE,
            ENGI_BYTE_END_ADDR);
    pr_info("%-15s: 0x%08X - 0x%08X",
            "OPTION BYTE",
            OPTION_BYTE_BASE,
            OPTION_BYTE_END_ADDR);
    pr_info("%-15s: 0x%08X - 0x%08X",
            "SRAM1",
            SRAM1_BASE,
            SRAM1_END_ADDR);
    pr_info("%-15s: 0x%08X - 0x%08X",
            "SRAM2A",
            SRAM2A_BASE,
            SRAM2A_END_ADDR);
    pr_info("%-15s: 0x%08X - 0x%08X",
            "SRAM2B",
            SRAM2B_BASE,
            SRAM2B_END_ADDR);
    pr_info("");
    pr_info("%-15s: %d Hz", "System Clock", HAL_RCC_GetSysClockFreq());
    pr_info("%-15s: %d Hz", "HCLK Freq", HAL_RCC_GetHCLKFreq());
    pr_info("%-15s: %d Hz", "HCLK2 Freq", HAL_RCC_GetHCLK2Freq());
    pr_info("%-15s: %d Hz", "HCLK4 Freq", HAL_RCC_GetHCLK4Freq());
    pr_info("%-15s: %d Hz", "PCLK1 Freq", HAL_RCC_GetPCLK1Freq());
    pr_info("%-15s: %d Hz", "PCLK2 Freq", HAL_RCC_GetPCLK2Freq());
    pr_info("");
    pr_info("*************************************************************");
    pr_info("");
}

/**
 * @brief  Configure the system clock.
 *
 * @note   This API configures
 *           - The system clock source
 *           - The AHBCLK, APBCLK dividers
 *           - The flash latency
 *           - The PLL settings (when required)
 *
 * @param  None.
 * @retval None.
 */
static void hardware_system_clock_config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    /**
     * Configure LSE Drive Capability
     */
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    /**
     * Configure the main internal regulator output voltage
     */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**
     * Initializes the CPU, AHB and APB busses clocks
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI |
                                       RCC_OSCILLATORTYPE_HSE |
                                       RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        assert(0);
    }

    /**
     * Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4 | RCC_CLOCKTYPE_HCLK2
                                  | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
    {
        assert(0);
    }
}

/**
 * @brief Peripherals clock configuration.
 *
 * @retval None.
 */
static void hardware_periph_clock_config(void)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };

    /**
     * Initializes the peripherals clocks
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS |
                                               RCC_PERIPHCLK_RFWAKEUP |
                                               RCC_PERIPHCLK_RTC |
                                               RCC_PERIPHCLK_USART1 |
                                               RCC_PERIPHCLK_RNG;
    PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSE;
    PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE1;
    PeriphClkInitStruct.RFWakeUpClockSelection = RCC_RFWKPCLKSOURCE_LSE;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_HSI48;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        assert(0);
    }
}
