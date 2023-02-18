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
#include "adc_manager.h"
#include "stm32wbxx.h"
#include "stm32wbxx_adc.h"

/**
 * @brief   ADC handle definition.
 */
typedef struct
{
    ADC_HandleTypeDef   adc;
    DMA_HandleTypeDef   dma_adc;

    uint16_t            adc_data[ADC_ID_BUTT];
} stm32wbxx_adc_handle_t;

static stm32wbxx_adc_handle_t stm32wbxx_adc_handle;

/**
 * @brief   Conversion complete callback in non-blocking mode.
 *
 * @param   hadc Pointer to the ADC driver handle.
 *
 * @retval  None.
 */
void stm32wbxx_adc_conv_cplt_clbk(ADC_HandleTypeDef* hadc)
{
    uint16_t data;

    data = __HAL_ADC_CALC_DATA_TO_VOLTAGE(3300U,
                                          stm32wbxx_adc_handle.adc_data[0],
                                          LL_ADC_RESOLUTION_12B);

    adc_manager_driver_conv_cplt_clbk(ADC_ID_1, data);

    data = __HAL_ADC_CALC_DATA_TO_VOLTAGE(3300U,
                                          stm32wbxx_adc_handle.adc_data[1],
                                          LL_ADC_RESOLUTION_12B);

    adc_manager_driver_conv_cplt_clbk(ADC_ID_2, data);

    data = __LL_ADC_CALC_TEMPERATURE(3300U,
                                     stm32wbxx_adc_handle.adc_data[2],
                                     LL_ADC_RESOLUTION_12B);

    adc_manager_driver_conv_cplt_clbk(ADC_ID_3, data);
}

/**
 * @brief   ADC error callback in non-blocking mode.
 *
 * @param   hadc Pointer to the ADC driver handle.
 *
 * @retval  None.
 */
void stm32wbxx_adc_error_clbk(ADC_HandleTypeDef* hadc)
{
    adc_manager_driver_error_clbk();
}

/**
 * @brief This function handles DMA1 channel1 global interrupt.
 */
void DMA1_Channel1_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&stm32wbxx_adc_handle.dma_adc);
}

/**
 * @brief This function handles ADC1 global interrupt.
 */
void ADC1_IRQHandler(void)
{
    HAL_ADC_IRQHandler(&stm32wbxx_adc_handle.adc);
}

/**
 * @brief   Start ADC sequence convert.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t stm32wbxx_adc_start(void)
{
    if (HAL_ADC_Start(&stm32wbxx_adc_handle.adc) != HAL_OK)
    {
        return -EIO;
    }

    return 0;
}

/**
 * @brief   Initialize ADC MSP.
 *
 * @param   hadc Pointer to the ADC driver handle.
 *
 * @retval  None.
 */
static void stm32wbxx_adc_msp_init(ADC_HandleTypeDef* hadc)
{
    GPIO_InitTypeDef gpio;

    if (hadc->Instance == ADC1)
    {
        gpio.Pin = GPIO_PIN_2;
        gpio.Mode = GPIO_MODE_ANALOG;
        gpio.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOC, &gpio);

        gpio.Pin = GPIO_PIN_3;
        gpio.Mode = GPIO_MODE_ANALOG;
        gpio.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOC, &gpio);

        stm32wbxx_adc_handle.dma_adc.Instance = DMA1_Channel1;
        stm32wbxx_adc_handle.dma_adc.Init.Request = DMA_REQUEST_ADC1;
        stm32wbxx_adc_handle.dma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
        stm32wbxx_adc_handle.dma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
        stm32wbxx_adc_handle.dma_adc.Init.MemInc = DMA_MINC_ENABLE;
        stm32wbxx_adc_handle.dma_adc.Init.PeriphDataAlignment =
            DMA_PDATAALIGN_HALFWORD;
        stm32wbxx_adc_handle.dma_adc.Init.MemDataAlignment =
            DMA_MDATAALIGN_HALFWORD;
        stm32wbxx_adc_handle.dma_adc.Init.Mode = DMA_CIRCULAR;
        stm32wbxx_adc_handle.dma_adc.Init.Priority = DMA_PRIORITY_HIGH;

        if (HAL_DMA_Init(&stm32wbxx_adc_handle.dma_adc) != HAL_OK)
        {
            return;
        }

        __HAL_LINKDMA(hadc, DMA_Handle, stm32wbxx_adc_handle.dma_adc);

        HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0x0F, 0);
        HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

        HAL_NVIC_SetPriority(ADC1_IRQn, 0x0F, 0);
        HAL_NVIC_EnableIRQ(ADC1_IRQn);
    }
}

/**
 * @brief   Deinitialize ADC MSP.
 *
 * @param   hadc Pointer to the ADC driver handle.
 *
 * @retval  None.
 */
static void stm32wbxx_adc_msp_deinit(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2);
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_3);

        HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
        HAL_NVIC_DisableIRQ(ADC1_IRQn);

        HAL_DMA_DeInit(hadc->DMA_Handle);
    }
}

/**
 * @brief   Initialize the ADC driver.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t stm32wbxx_adc_init(void)
{
    ADC_ChannelConfTypeDef channel;

    (void)memset(&stm32wbxx_adc_handle, 0, sizeof(stm32wbxx_adc_handle));

    stm32wbxx_adc_handle.adc.Instance = ADC1;
    stm32wbxx_adc_handle.adc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    stm32wbxx_adc_handle.adc.Init.Resolution = ADC_RESOLUTION_12B;
    stm32wbxx_adc_handle.adc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    stm32wbxx_adc_handle.adc.Init.ScanConvMode = ADC_SCAN_ENABLE;
    stm32wbxx_adc_handle.adc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    stm32wbxx_adc_handle.adc.Init.LowPowerAutoWait = DISABLE;
    stm32wbxx_adc_handle.adc.Init.ContinuousConvMode = DISABLE;
    stm32wbxx_adc_handle.adc.Init.NbrOfConversion = 3;
    stm32wbxx_adc_handle.adc.Init.DiscontinuousConvMode = ENABLE;
    stm32wbxx_adc_handle.adc.Init.NbrOfDiscConversion = 1;
    stm32wbxx_adc_handle.adc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    stm32wbxx_adc_handle.adc.Init.ExternalTrigConvEdge =
        ADC_EXTERNALTRIGCONVEDGE_NONE;
    stm32wbxx_adc_handle.adc.Init.DMAContinuousRequests = DISABLE;
    stm32wbxx_adc_handle.adc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    stm32wbxx_adc_handle.adc.Init.OversamplingMode = DISABLE;

    if (HAL_ADC_RegisterCallback(&stm32wbxx_adc_handle.adc,
                                 HAL_ADC_MSPINIT_CB_ID,
                                 stm32wbxx_adc_msp_init) != HAL_OK)
    {
        return -EIO;
    }

    if (HAL_ADC_RegisterCallback(&stm32wbxx_adc_handle.adc,
                                 HAL_ADC_MSPDEINIT_CB_ID,
                                 stm32wbxx_adc_msp_deinit) != HAL_OK)
    {
        return -EIO;
    }

    if (HAL_ADC_Init(&stm32wbxx_adc_handle.adc) != HAL_OK)
    {
        return -EIO;
    }

    if (HAL_ADC_RegisterCallback(&stm32wbxx_adc_handle.adc,
                                 HAL_ADC_CONVERSION_COMPLETE_CB_ID,
                                 stm32wbxx_adc_conv_cplt_clbk) != HAL_OK)
    {
        return -EIO;
    }

    if (HAL_ADC_RegisterCallback(&stm32wbxx_adc_handle.adc,
                                 HAL_ADC_ERROR_CB_ID,
                                 stm32wbxx_adc_error_clbk) != HAL_OK)
    {
        return -EIO;
    }

    (void)memset(&channel, 0, sizeof(channel));

    channel.Channel = ADC_CHANNEL_3;
    channel.Rank = ADC_REGULAR_RANK_1;
    channel.SamplingTime = ADC_SAMPLETIME_92CYCLES_5;
    channel.SingleDiff = ADC_SINGLE_ENDED;
    channel.OffsetNumber = ADC_OFFSET_NONE;
    channel.Offset = 0;

    if (HAL_ADC_ConfigChannel(&stm32wbxx_adc_handle.adc, &channel) != HAL_OK)
    {
        return -EIO;
    }

    (void)memset(&channel, 0, sizeof(channel));

    channel.Channel = ADC_CHANNEL_4;
    channel.Rank = ADC_REGULAR_RANK_2;
    channel.SamplingTime = ADC_SAMPLETIME_92CYCLES_5;
    channel.SingleDiff = ADC_SINGLE_ENDED;
    channel.OffsetNumber = ADC_OFFSET_NONE;
    channel.Offset = 0;

    if (HAL_ADC_ConfigChannel(&stm32wbxx_adc_handle.adc, &channel) != HAL_OK)
    {
        return -EIO;
    }

    (void)memset(&channel, 0, sizeof(channel));

    channel.Channel = ADC_CHANNEL_TEMPSENSOR;
    channel.Rank = ADC_REGULAR_RANK_3;

    if (HAL_ADC_ConfigChannel(&stm32wbxx_adc_handle.adc, &channel) != HAL_OK)
    {
        return -EIO;
    }

    if (HAL_ADCEx_Calibration_Start(&stm32wbxx_adc_handle.adc,
                                    ADC_SINGLE_ENDED) != HAL_OK)
    {
        return -EIO;
    }

    if (HAL_ADC_Start_DMA(&stm32wbxx_adc_handle.adc,
                          (uint32_t*)stm32wbxx_adc_handle.adc_data,
                          ADC_ID_BUTT) != HAL_OK)
    {
        return -EIO;
    }

    return 0;
}

/**
 * @brief   Deinitialize the ADC driver.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t stm32wbxx_adc_deinit(void)
{
    if (HAL_ADC_Stop_DMA(&stm32wbxx_adc_handle.adc) != HAL_OK)
    {
        return -EIO;
    }

    if (HAL_ADC_DeInit(&stm32wbxx_adc_handle.adc) != HAL_OK)
    {
        return -EIO;
    }

    return 0;
}
