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
#include "hardware_if.h"

static void hardware_print_info(void);
static void hardware_clk_enable(void);
static void hardware_appe_config(void);
static void hardware_system_clock_config(void);

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

    hardware_appe_config();

    hardware_system_clock_config();

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
    __HAL_RCC_ADC_CLK_ENABLE();
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
}

static void Reset_BackupDomain(void)
{
    if ((LL_RCC_IsActiveFlag_PINRST() != FALSE) &&
        (LL_RCC_IsActiveFlag_SFTRST() == FALSE))
    {
        /**< Enable access to the RTC registers */
        HAL_PWR_EnableBkUpAccess();

        /**
         *  Write twice the value to flush the APB-AHB bridge
         *  This bit shall be written in the register before writing the next one
         */
        HAL_PWR_EnableBkUpAccess();

        __HAL_RCC_BACKUPRESET_FORCE();
        __HAL_RCC_BACKUPRESET_RELEASE();
    }

    return;
}

static void Reset_IPCC(void)
{
    LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_IPCC);

    LL_C1_IPCC_ClearFlag_CHx(
        IPCC,
        LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 |
        LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4 |
        LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

    LL_C2_IPCC_ClearFlag_CHx(
        IPCC,
        LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 |
        LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4 |
        LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

    LL_C1_IPCC_DisableTransmitChannel(
        IPCC,
        LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 |
        LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4 |
        LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

    LL_C2_IPCC_DisableTransmitChannel(
        IPCC,
        LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 |
        LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4 |
        LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

    LL_C1_IPCC_DisableReceiveChannel(
        IPCC,
        LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 |
        LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4 |
        LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

    LL_C2_IPCC_DisableReceiveChannel(
        IPCC,
        LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 |
        LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4 |
        LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

    return;
}

static void Reset_Device(void)
{
    Reset_BackupDomain();

    Reset_IPCC();
}

static void hardware_appe_config(void)
{
    /**
     * The OPTVERR flag is wrongly set at power on
     * It shall be cleared before using any HAL_FLASH_xxx() api
     */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

    /**
     * Reset some configurations so that the system behave in the same way
     * when either out of nReset or Power On
     */
    Reset_Device();

    return;
}

/**
 * @brief  Configure the system clock
 *
 * @note   This API configures
 *         - The system clock source
 *           - The AHBCLK, APBCLK dividers
 *           - The flash latency
 *           - The PLL settings (when required)
 *
 * @param  None
 * @retval None
 */
static void hardware_system_clock_config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };

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
        while (1)
        {
            ;
        }
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
        while (1)
        {
            ;
        }
    }

    /**
     * Initializes the peripherals clocks
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS |
                                               RCC_PERIPHCLK_RFWAKEUP |
                                               RCC_PERIPHCLK_RTC |
                                               RCC_PERIPHCLK_USART1 |
                                               RCC_PERIPHCLK_LPUART1;
    PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInitStruct.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    PeriphClkInitStruct.RFWakeUpClockSelection = RCC_RFWKPCLKSOURCE_LSE;
    PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSE;
    PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE1;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        while (1)
        {
            ;
        }
    }

    /**
     * Configure and enable SMPS
     *
     * The SMPS configuration is not yet supported by CubeMx
     */
    LL_PWR_SMPS_SetStartupCurrent(LL_PWR_SMPS_STARTUP_CURRENT_80MA);
    LL_PWR_SMPS_SetOutputVoltageLevel(LL_PWR_SMPS_OUTPUT_VOLTAGE_1V40);
    LL_PWR_SMPS_Enable();
}
