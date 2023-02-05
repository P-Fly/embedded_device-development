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
#include "led_manager.h"
#include "stm32wbxx.h"
#include "stm32wbxx_led.h"

#define CONFIG_LED1_PORT    GPIOB
#define CONFIG_LED1_PIN     GPIO_PIN_5  /* Blue */
#define CONFIG_LED2_PORT    GPIOB
#define CONFIG_LED2_PIN     GPIO_PIN_0  /* Green */
#define CONFIG_LED3_PORT    GPIOB
#define CONFIG_LED3_PIN     GPIO_PIN_1  /* Red */

/**
 * @brief   Led handle definition.
 */
typedef struct
{
    int32_t reserved;
} stm32wbxx_led_handle_t;

/**
 * @brief   Led hardware config definition.
 */
typedef struct
{
    led_id_e        id;

    GPIO_TypeDef*   gpio_port;
    uint16_t        gpio_pin;
} stm32wbxx_led_hw_config_t;

static const stm32wbxx_led_hw_config_t led_hw_config[] =
{
    { LED_ID_1, CONFIG_LED1_PORT, CONFIG_LED1_PIN },
    { LED_ID_2, CONFIG_LED2_PORT, CONFIG_LED2_PIN },
    { LED_ID_3, CONFIG_LED3_PORT, CONFIG_LED3_PIN },
};

/**
 * @brief   Initialize the led driver.
 *
 * @retval  None.
 */
void stm32wbxx_led_init(void)
{
    GPIO_InitTypeDef led;
    int32_t i;

    for (i = 0; i < sizeof(led_hw_config) / sizeof(led_hw_config[0]); i++)
    {
        led.Pin = led_hw_config[i].gpio_pin;
        led.Mode = GPIO_MODE_OUTPUT_PP;
        led.Pull = GPIO_NOPULL;
        led.Speed = GPIO_SPEED_FREQ_HIGH;

        HAL_GPIO_Init(led_hw_config[i].gpio_port, &led);

        HAL_GPIO_WritePin(led_hw_config[i].gpio_port,
                          led_hw_config[i].gpio_pin,
                          GPIO_PIN_RESET);
    }
}

/**
 * @brief   Remove the led driver.
 *
 * @retval  None.
 */
void stm32wbxx_led_deinit(void)
{
    int32_t i;

    for (i = 0; i < sizeof(led_hw_config) / sizeof(led_hw_config[0]); i++)
    {
        HAL_GPIO_WritePin(led_hw_config[i].gpio_port,
                          led_hw_config[i].gpio_pin,
                          GPIO_PIN_RESET);
        HAL_GPIO_DeInit(led_hw_config[i].gpio_port, led_hw_config[i].gpio_pin);
    }
}

/**
 * @brief   Turn on selected LED.
 *
 * @param   id Selected id.
 *
 * @retval  None.
 */
void stm32wbxx_led_on(led_id_e id)
{
    int32_t i;

    for (i = 0; i < sizeof(led_hw_config) / sizeof(led_hw_config[0]); i++)
    {
        if (id == led_hw_config[i].id)
        {
            HAL_GPIO_WritePin(led_hw_config[i].gpio_port,
                              led_hw_config[i].gpio_pin,
                              GPIO_PIN_SET);
        }
    }
}

/**
 * @brief   Turn off selected LED.
 *
 * @param   id Selected id.
 *
 * @retval  None.
 */
void stm32wbxx_led_off(led_id_e id)
{
    int32_t i;

    for (i = 0; i < sizeof(led_hw_config) / sizeof(led_hw_config[0]); i++)
    {
        if (id == led_hw_config[i].id)
        {
            HAL_GPIO_WritePin(led_hw_config[i].gpio_port,
                              led_hw_config[i].gpio_pin,
                              GPIO_PIN_RESET);
        }
    }
}

/**
 * @brief   Toggle selected LED.
 *
 * @param   id Selected id.
 *
 * @retval  None.
 */
void stm32wbxx_led_toggle(led_id_e id)
{
    int32_t i;

    for (i = 0; i < sizeof(led_hw_config) / sizeof(led_hw_config[0]); i++)
    {
        if (id == led_hw_config[i].id)
        {
            HAL_GPIO_TogglePin(led_hw_config[i].gpio_port,
                               led_hw_config[i].gpio_pin);
        }
    }
}
