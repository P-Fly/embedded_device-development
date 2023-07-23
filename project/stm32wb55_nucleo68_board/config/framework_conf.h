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

#ifndef __FRAMEWORK_CONF__
#define __FRAMEWORK_CONF__

#define CONFIG_CPU_NAME   "STM32WBxx"
#define CONFIG_BOARD_NAME "P-NUCLEO-WB55"

#define CONFIG_MSG_SEND_BLOCK_TIMEOUT_MS 50

#define CONFIG_INIT_THREAD_NAME "init thread"
#define CONFIG_INIT_THREAD_STACK_SIZE 1024
#define CONFIG_INIT_THREAD_PRIORITY osPriorityRealtime

#define CONFIG_MMI_SERVICE_NAME "mmi service"
#define CONFIG_MMI_SERVICE_LABEL mmi_service
#define CONFIG_MMI_SERVICE_THREAD_NAME "mmi thread"
#define CONFIG_MMI_SERVICE_THREAD_STACK_SIZE 2048
#define CONFIG_MMI_SERVICE_THREAD_PRIORITY osPriorityNormal
#define CONFIG_MMI_SERVICE_QUEUE_NAME "mmi queue"
#define CONFIG_MMI_SERVICE_MSG_COUNT 10
#define CONFIG_MMI_SERVICE_INTERNAL_COMMAND_ENABLE

#define CONFIG_LED_SERVICE_NAME "led service"
#define CONFIG_LED_SERVICE_LABEL led_service
#define CONFIG_LED_SERVICE_THREAD_NAME "led thread"
#define CONFIG_LED_SERVICE_THREAD_STACK_SIZE 2048
#define CONFIG_LED_SERVICE_THREAD_PRIORITY osPriorityNormal
#define CONFIG_LED_SERVICE_QUEUE_NAME "led queue"
#define CONFIG_LED_SERVICE_MSG_COUNT 10

#define CONFIG_LED_MANAGER_NAME "led manager"
#define CONFIG_LED_MANAGER_LABEL led_manager
#define CONFIG_LED_MANAGER_TIMER_NAME "led manager timer"
#define CONFIG_LED_MANAGER_QUICK_FLASH_INTERVAL_MS 300
#define CONFIG_LED_MANAGER_SLOW_FLASH_INTERVAL_MS 1000

#define CONFIG_BUTTON_SERVICE_NAME "button service"
#define CONFIG_BUTTON_SERVICE_LABEL button_service
#define CONFIG_BUTTON_SERVICE_THREAD_NAME "button thread"
#define CONFIG_BUTTON_SERVICE_THREAD_STACK_SIZE 2048
#define CONFIG_BUTTON_SERVICE_THREAD_PRIORITY osPriorityNormal
#define CONFIG_BUTTON_SERVICE_QUEUE_NAME "button queue"
#define CONFIG_BUTTON_SERVICE_MSG_COUNT 10

#define CONFIG_BUTTON_MANAGER_NAME "button manager"
#define CONFIG_BUTTON_MANAGER_LABEL button_manager
#define CONFIG_BUTTON_MANAGER_TIMER_NAME "button manager timer"
#define CONFIG_BUTTON_MANAGER_TIMER_INTERVAL_MS 20
#define CONFIG_BUTTON_MANAGER_TIMER_LONGLONG_CLICK_MS 10000
#define CONFIG_BUTTON_MANAGER_TIMER_LONG_CLICK_MS 5000

#define CONFIG_CRC_MANAGER_NAME "crc manager"
#define CONFIG_CRC_MANAGER_LABEL crc_manager

#define CONFIG_CLOCK_MANAGER_NAME "sys manager"
#define CONFIG_CLOCK_MANAGER_LABEL sys_manager

#define CONFIG_DBG_CLI_NAME "dbg cli"
#define CONFIG_DBG_CLI_LABEL dbg_cli
#define CONFIG_DBG_CLI_OUTPUT_BUFF_SIZE 256
#define CONFIG_DBG_CLI_INPUT_BUFF_SIZE 128

#define CONFIG_BLE_SERVICE_NAME "ble service"
#define CONFIG_BLE_SERVICE_LABEL ble_service
#define CONFIG_BLE_SERVICE_THREAD_NAME "ble thread"
#define CONFIG_BLE_SERVICE_THREAD_STACK_SIZE 2048
#define CONFIG_BLE_SERVICE_THREAD_PRIORITY osPriorityNormal
#define CONFIG_BLE_SERVICE_QUEUE_NAME "ble queue"
#define CONFIG_BLE_SERVICE_MSG_COUNT 10

#define CONFIG_UI_SERVICE_NAME "ui service"
#define CONFIG_UI_SERVICE_LABEL ui_service
#define CONFIG_UI_SERVICE_THREAD_NAME "ui thread"
#define CONFIG_UI_SERVICE_THREAD_STACK_SIZE 2048
#define CONFIG_UI_SERVICE_THREAD_PRIORITY osPriorityNormal
#define CONFIG_UI_SERVICE_QUEUE_NAME "ui queue"
#define CONFIG_UI_SERVICE_MSG_COUNT 10

#define CONFIG_ADC_MANAGER_NAME "adc manager"
#define CONFIG_ADC_MANAGER_LABEL adc_manager
#define CONFIG_ADC_MANAGER_TIMER_NAME "adc manager timer"

#define CONFIG_RNG_MANAGER_NAME "rng manager"
#define CONFIG_RNG_MANAGER_LABEL rng_manager

#define CONFIG_SHCI_MANAGER_NAME  "shci manager"
#define CONFIG_SHCI_MANAGER_LABEL shci_manager

#define CONFIG_HCI_MANAGER_NAME "hci manager"
#define CONFIG_HCI_MANAGER_LABEL hci_manager

#define CONFIG_ADV_MANAGER_NAME "adv manager"
#define CONFIG_ADV_MANAGER_LABEL adv_manager
#define CONFIG_ADV_MANAGER_TIMER_NAME "adv manager timer"

#endif /* __FRAMEWORK_CONF__ */
