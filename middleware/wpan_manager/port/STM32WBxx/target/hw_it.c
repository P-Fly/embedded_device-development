#include "wpan_app_common.h"

extern IPCC_HandleTypeDef hipcc;
extern RTC_HandleTypeDef hrtc;
/* TBD: */
#if 0
extern TIM_HandleTypeDef htim17;
#endif

/* TBD: */
/**
 * @brief This function handles RTC wake-up interrupt through EXTI line 19.
 */
void RTC_WKUP_IRQHandler(void)
{
    HAL_RTCEx_WakeUpTimerIRQHandler(&hrtc);
}

/* TBD: */
#if 0
/**
 * @brief This function handles TIM1 trigger and commutation interrupts and TIM17 global interrupt.
 */
void TIM1_TRG_COM_TIM17_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim17);
}
#endif

/**
 * @brief This function handles IPCC RX occupied interrupt.
 */
void IPCC_C1_RX_IRQHandler(void)
{
    HAL_IPCC_RX_IRQHandler(&hipcc);
}

/**
 * @brief This function handles IPCC TX free interrupt.
 */
void IPCC_C1_TX_IRQHandler(void)
{
    HAL_IPCC_TX_IRQHandler(&hipcc);
}

/* TBD: */
/**
 * @brief This function handles HSEM global interrupt.
 */
void HSEM_IRQHandler(void)
{
    HAL_HSEM_IRQHandler();
}
