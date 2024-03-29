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
#include "ring_buff.h"
#include "dbg_cli.h"
#include "stm32wbxx.h"
#include "stm32wbxx_uart.h"
#include "middleware_conf.h"

/**
 * @brief   Uart handle definition.
 */
typedef struct
{
    UART_HandleTypeDef  uart;

    ring_buff_t         tx;
    char                tx_ring_buff[CONFIG_UART1_TX_RING_BUFF_SIZE];
} stm32wbxx_uart_handle_t;

static stm32wbxx_uart_handle_t stm32wbxx_uart_handle;

/**
 * @brief   Enable or disable uart read request.
 *
 * @param   enable_disable Enable or disable.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t stm32wbxx_uart1_read_clbk_enable(uint32_t enable_disable)
{
    if (enable_disable == 0)
    {
        __HAL_UART_DISABLE_IT(&stm32wbxx_uart_handle.uart, UART_IT_RXNE);
    }
    else
    {
        __HAL_UART_ENABLE_IT(&stm32wbxx_uart_handle.uart, UART_IT_RXNE);
    }

    return 0;
}

/**
 * @brief   Write data to uart.
 *
 * @param   tx_buf Pointer to data buffer to write.
 * @param   tx_len Data length to write.
 *
 * @retval  The number of data bytes write to the slave on success,
 *          negative error code otherwise.
 */
int32_t stm32wbxx_uart1_write(const void* tx_buf, int32_t tx_len)
{
    char* buff = (char*)tx_buf;
    int32_t i;
    int32_t ret;
    uint32_t isrm;
    BaseType_t is_irq = xPortIsInsideInterrupt();

    if (!tx_buf)
    {
        return -EINVAL;
    }

    if (tx_len <= 0)
    {
        return -EINVAL;
    }

    if (is_irq)
    {
        isrm = taskENTER_CRITICAL_FROM_ISR();
    }
    else
    {
        taskENTER_CRITICAL();
    }

    for (i = 0; i < tx_len; i++)
    {
        ret = ring_buffer_write(&stm32wbxx_uart_handle.tx, buff[i]);
        if (ret)
        {
            break;
        }
    }

    if (is_irq)
    {
        taskEXIT_CRITICAL_FROM_ISR(isrm);
    }
    else
    {
        taskEXIT_CRITICAL();
    }

    /* Enable the UART Transmit data register empty Interrupt */
    __HAL_UART_ENABLE_IT(&stm32wbxx_uart_handle.uart, UART_IT_TXE);

    return i;
}

/**
 * @brief   Handles uart interrupt request.
 *
 * @param   handle Pointer to the uart driver handle.
 *
 * @retval  None.
 */
static void stm32wbxx_uart1_irq_handler(stm32wbxx_uart_handle_t* handle)
{
    int32_t ret;
    char value;

    /* UART in mode Transmitter */
    if (__HAL_UART_GET_IT_SOURCE(&handle->uart, UART_IT_TXE))
    {
        if (__HAL_UART_GET_FLAG(&handle->uart, UART_FLAG_TXE))
        {
            ret = ring_buffer_read(&handle->tx, &value);
            if (ret)
            {
                /* Disable the UART Transmit data register empty Interrupt */
                __HAL_UART_DISABLE_IT(&handle->uart, UART_IT_TXE);
            }
            else
            {
                /* Flushes the UART DR register */
                handle->uart.Instance->TDR = value;
            }
        }
    }

    /* UART in mode Receiver */
    if (__HAL_UART_GET_IT_SOURCE(&handle->uart, UART_IT_RXNE))
    {
        if (__HAL_UART_GET_FLAG(&handle->uart, UART_FLAG_RXNE))
        {
            value = handle->uart.Instance->RDR;

            dbg_cli_input_driver_clbk(value);
        }
    }
}

/**
 * @brief   This function handles USART1 Global Interrupt.
 *
 * @retval  None.
 */
void USART1_IRQHandler(void)
{
    stm32wbxx_uart1_irq_handler(&stm32wbxx_uart_handle);
}

/**
 * @brief   Initialize uart MSP.
 *
 * @param   uart Pointer to the uart driver handle.
 *
 * @retval  None.
 */
static void stm32wbxx_uart1_msp_init(UART_HandleTypeDef* uart)
{
    GPIO_InitTypeDef gpio;

    (void)memset(&gpio, 0, sizeof(gpio));

    /**
     * USART1 GPIO Configuration
     * PB6 ------> USART1_TX
     * PB7 ------> USART1_RX
     */
    gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOB, &gpio);

    HAL_NVIC_SetPriority(USART1_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}

/**
 * @brief   Deinitialize uart MSP.
 *
 * @param   uart Pointer to the uart driver handle.
 *
 * @retval  None.
 */
static void stm32wbxx_uart1_msp_deinit(UART_HandleTypeDef* uart)
{
    HAL_NVIC_DisableIRQ(USART1_IRQn);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6 | GPIO_PIN_7);
}

/**
 * @brief   Initialize the uart1 driver.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t stm32wbxx_uart1_init(void)
{
    int32_t ret;

    (void)memset(&stm32wbxx_uart_handle, 0, sizeof(stm32wbxx_uart_handle_t));

    stm32wbxx_uart_handle.uart.Instance = USART1;
    stm32wbxx_uart_handle.uart.Init.BaudRate = CONFIG_UART1_HW_BAUDRATE;
    stm32wbxx_uart_handle.uart.Init.WordLength = UART_WORDLENGTH_8B;
    stm32wbxx_uart_handle.uart.Init.StopBits = UART_STOPBITS_1;
    stm32wbxx_uart_handle.uart.Init.Parity = UART_PARITY_NONE;
    stm32wbxx_uart_handle.uart.Init.Mode = UART_MODE_TX_RX;
    stm32wbxx_uart_handle.uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    stm32wbxx_uart_handle.uart.Init.OverSampling = UART_OVERSAMPLING_16;
    stm32wbxx_uart_handle.uart.Init.OneBitSampling =
        UART_ONE_BIT_SAMPLE_DISABLE;
    stm32wbxx_uart_handle.uart.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    stm32wbxx_uart_handle.uart.AdvancedInit.AdvFeatureInit =
        UART_ADVFEATURE_NO_INIT;

    if (HAL_UART_RegisterCallback(&stm32wbxx_uart_handle.uart,
                                  HAL_UART_MSPINIT_CB_ID,
                                  stm32wbxx_uart1_msp_init) != HAL_OK)
    {
        return -EIO;
    }

    if (HAL_UART_RegisterCallback(&stm32wbxx_uart_handle.uart,
                                  HAL_UART_MSPDEINIT_CB_ID,
                                  stm32wbxx_uart1_msp_deinit) != HAL_OK)
    {
        return -EIO;
    }

    ret =
        ring_buffer_init(&stm32wbxx_uart_handle.tx,
                         stm32wbxx_uart_handle.tx_ring_buff,
                         sizeof(stm32wbxx_uart_handle.tx_ring_buff));
    if (ret)
    {
        return ret;
    }

    if (HAL_UART_Init(&stm32wbxx_uart_handle.uart) != HAL_OK)
    {
        return -EIO;
    }

    if (HAL_UARTEx_SetTxFifoThreshold(&stm32wbxx_uart_handle.uart,
                                      UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        return -EIO;
    }

    if (HAL_UARTEx_SetRxFifoThreshold(&stm32wbxx_uart_handle.uart,
                                      UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
        return -EIO;
    }

    if (HAL_UARTEx_EnableFifoMode(&stm32wbxx_uart_handle.uart) != HAL_OK)
    {
        return -EIO;
    }

    return 0;
}

/**
 * @brief   Remove the uart1 driver.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t stm32wbxx_uart1_deinit(void)
{
    if (HAL_UART_DeInit(&stm32wbxx_uart_handle.uart) != HAL_OK)
    {
        return -EIO;
    }

    return 0;
}
