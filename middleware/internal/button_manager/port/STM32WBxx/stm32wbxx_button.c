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
#include "button_manager.h"
#include "stm32wbxx.h"
#include "stm32wbxx_button.h"
#include "middleware_conf.h"

#define CONFIG_BUTTON1_PORT         GPIOC
#define CONFIG_BUTTON1_PIN          GPIO_PIN_4
#define CONFIG_BUTTON1_EXTI_IRQn    EXTI4_IRQn
#define CONFIG_BUTTON2_PORT         GPIOD
#define CONFIG_BUTTON2_PIN          GPIO_PIN_0
#define CONFIG_BUTTON2_EXTI_IRQn    EXTI0_IRQn
#define CONFIG_BUTTON3_PORT         GPIOD
#define CONFIG_BUTTON3_PIN          GPIO_PIN_1
#define CONFIG_BUTTON3_EXTI_IRQn    EXTI1_IRQn

/**
 * @brief   Button handle definition.
 */
typedef struct
{
    int32_t reserved;
} stm32wbxx_button_handle_t;

/**
 * @brief   Button hardware config definition.
 */
typedef struct
{
    button_id_e     id;

    GPIO_TypeDef*   gpio_port;
    uint16_t        gpio_pin;
    IRQn_Type       irq_type;
} stm32wbxx_button_hw_config_t;

static const stm32wbxx_button_hw_config_t stm32wbxx_button_hw_config[] =
{
    { BUTTON_ID_1, CONFIG_BUTTON1_PORT, CONFIG_BUTTON1_PIN,
      CONFIG_BUTTON1_EXTI_IRQn },
    { BUTTON_ID_2, CONFIG_BUTTON2_PORT, CONFIG_BUTTON2_PIN,
      CONFIG_BUTTON2_EXTI_IRQn },
    { BUTTON_ID_3, CONFIG_BUTTON3_PORT, CONFIG_BUTTON3_PIN,
      CONFIG_BUTTON3_EXTI_IRQn },
};

/**
 * @brief   Initialize the button driver.
 *
 * @retval  None.
 */
int32_t stm32wbxx_button_init(void)
{
    GPIO_InitTypeDef button;
    uint32_t i;

    for (i = 0;
         i <
         sizeof(stm32wbxx_button_hw_config) /
         sizeof(stm32wbxx_button_hw_config[0]);
         i++)
    {
        button.Pin = stm32wbxx_button_hw_config[i].gpio_pin;
        button.Pull = GPIO_PULLUP;
        button.Mode = GPIO_MODE_IT_FALLING;
        HAL_GPIO_Init(stm32wbxx_button_hw_config[i].gpio_port, &button);

        HAL_NVIC_SetPriority(stm32wbxx_button_hw_config[i].irq_type, 0x0F,
                             0x00);
        HAL_NVIC_EnableIRQ(stm32wbxx_button_hw_config[i].irq_type);
    }

    return 0;
}

/**
 * @brief   Remove the button driver.
 *
 * @retval  None.
 */
int32_t stm32wbxx_button_deinit(void)
{
    uint32_t i;

    for (i = 0;
         i <
         sizeof(stm32wbxx_button_hw_config) /
         sizeof(stm32wbxx_button_hw_config[0]);
         i++)
    {
        HAL_NVIC_DisableIRQ(stm32wbxx_button_hw_config[i].irq_type);
        HAL_GPIO_DeInit(stm32wbxx_button_hw_config[i].gpio_port,
                        stm32wbxx_button_hw_config[i].gpio_pin);
    }

    return 0;
}

/**
 * @brief   Get button state.
 *
 * @param   id Button id.
 *
 * @retval  Returns button state.
 */
button_state_e stm32wbxx_button_get_state(button_id_e id)
{
    GPIO_PinState state;
    uint32_t i;

    for (i = 0;
         i <
         sizeof(stm32wbxx_button_hw_config) /
         sizeof(stm32wbxx_button_hw_config[0]);
         i++)
    {
        if (stm32wbxx_button_hw_config[i].id == id)
        {
            state = HAL_GPIO_ReadPin(stm32wbxx_button_hw_config[i].gpio_port,
                                     stm32wbxx_button_hw_config[i].gpio_pin);

            if (GPIO_PIN_RESET == state)
            {
                return BUTTON_STATE_DOWN;
            }
            else
            {
                return BUTTON_STATE_UP;
            }
        }
    }

    return BUTTON_STATE_BUTT;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    uint32_t i;

    for (i = 0;
         i <
         sizeof(stm32wbxx_button_hw_config) /
         sizeof(stm32wbxx_button_hw_config[0]);
         i++)
    {
        if (GPIO_Pin == stm32wbxx_button_hw_config[i].gpio_pin)
        {
            button_manager_driver_clbk(stm32wbxx_button_hw_config[i].id);
        }
    }
}

void EXTI4_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(stm32wbxx_button_hw_config[0].gpio_pin);
}

void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(stm32wbxx_button_hw_config[1].gpio_pin);
}

void EXTI1_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(stm32wbxx_button_hw_config[2].gpio_pin);
}
