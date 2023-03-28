#include "wpan_app_common.h"

/**
 * @brief IPCC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hipcc: IPCC handle pointer
 * @retval None
 */
void HAL_IPCC_MspInit(IPCC_HandleTypeDef* hipcc)
{
    if (hipcc->Instance == IPCC)
    {
        __HAL_RCC_IPCC_CLK_ENABLE();

        HAL_NVIC_SetPriority(IPCC_C1_RX_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(IPCC_C1_RX_IRQn);
        HAL_NVIC_SetPriority(IPCC_C1_TX_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(IPCC_C1_TX_IRQn);
    }
}

/**
 * @brief IPCC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hipcc: IPCC handle pointer
 * @retval None
 */
void HAL_IPCC_MspDeInit(IPCC_HandleTypeDef* hipcc)
{
    if (hipcc->Instance == IPCC)
    {
        __HAL_RCC_IPCC_CLK_DISABLE();

        HAL_NVIC_DisableIRQ(IPCC_C1_RX_IRQn);
        HAL_NVIC_DisableIRQ(IPCC_C1_TX_IRQn);
    }
}

/**
 * @brief RTC MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hrtc: RTC handle pointer
 * @retval None
 */
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };

    if (hrtc->Instance == RTC)
    {
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
        PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
        {
            while (1)
            {
                ;
            }
        }

        __HAL_RCC_RTC_ENABLE();
        __HAL_RCC_RTCAPB_CLK_ENABLE();

        HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);

        HAL_RTCEx_EnableBypassShadow(hrtc);
    }
}

/**
 * @brief RTC MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hrtc: RTC handle pointer
 * @retval None
 */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc)
{
    if (hrtc->Instance == RTC)
    {
        __HAL_RCC_RTC_DISABLE();
        __HAL_RCC_RTCAPB_CLK_DISABLE();

        HAL_NVIC_DisableIRQ(RTC_WKUP_IRQn);
    }
}
