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

#define CONFIG_INIT_THREAD_NAME "init"
#define CONFIG_INIT_THREAD_STACK_SIZE 2048
#define CONFIG_INIT_THREAD_PRIORITY osPriorityRealtime

#define CONFIG_MSG_SEND_BLOCK_TIMEOUT_MS 50

#define CONFIG_LED_SERVICE_NAME "led"
#define CONFIG_LED_SERVICE_LABEL led_service
#define CONFIG_LED_SERVICE_THREAD_NAME "led_task"
#define CONFIG_LED_SERVICE_THREAD_STACK_SIZE 2048
#define CONFIG_LED_SERVICE_THREAD_PRIORITY osPriorityNormal
#define CONFIG_LED_SERVICE_QUEUE_NAME "led_queue"
#define CONFIG_LED_SERVICE_MSG_COUNT 10

#define CONFIG_KEY_SERVICE_NAME "key"
#define CONFIG_KEY_SERVICE_LABEL key_service
#define CONFIG_KEY_SERVICE_THREAD_NAME "key_task"
#define CONFIG_KEY_SERVICE_THREAD_STACK_SIZE 2048
#define CONFIG_KEY_SERVICE_THREAD_PRIORITY osPriorityNormal
#define CONFIG_KEY_SERVICE_QUEUE_NAME "key_queue"
#define CONFIG_KEY_SERVICE_MSG_COUNT 10

#define CONFIG_DISPLAY_SERVICE_NAME "display"
#define CONFIG_DISPLAY_SERVICE_LABEL display_service
#define CONFIG_DISPLAY_SERVICE_THREAD_NAME "display_task"
#define CONFIG_DISPLAY_SERVICE_THREAD_STACK_SIZE 2048
#define CONFIG_DISPLAY_SERVICE_THREAD_PRIORITY osPriorityNormal
#define CONFIG_DISPLAY_SERVICE_QUEUE_NAME "display_queue"
#define CONFIG_DISPLAY_SERVICE_MSG_COUNT 10

#define CONFIG_DATA_SERVICE_NAME "data"
#define CONFIG_DATA_SERVICE_LABEL data_service
#define CONFIG_DATA_SERVICE_THREAD_NAME "data_task"
#define CONFIG_DATA_SERVICE_THREAD_STACK_SIZE 2048
#define CONFIG_DATA_SERVICE_THREAD_PRIORITY osPriorityNormal
#define CONFIG_DATA_SERVICE_QUEUE_NAME "data_queue"
#define CONFIG_DATA_SERVICE_MSG_COUNT 10

#define CONFIG_BLE_SERVICE_NAME "ble"
#define CONFIG_BLE_SERVICE_LABEL ble_service
#define CONFIG_BLE_SERVICE_THREAD_NAME "ble_task"
#define CONFIG_BLE_SERVICE_THREAD_STACK_SIZE 2048
#define CONFIG_BLE_SERVICE_THREAD_PRIORITY osPriorityNormal
#define CONFIG_BLE_SERVICE_QUEUE_NAME "ble_queue"
#define CONFIG_BLE_SERVICE_MSG_COUNT 10

#define CONFIG_BATTERY_SERVICE_NAME "battery"
#define CONFIG_BATTERY_SERVICE_LABEL battery_service
#define CONFIG_BATTERY_SERVICE_THREAD_NAME "battery_task"
#define CONFIG_BATTERY_SERVICE_THREAD_STACK_SIZE 2048
#define CONFIG_BATTERY_SERVICE_THREAD_PRIORITY osPriorityNormal
#define CONFIG_BATTERY_SERVICE_QUEUE_NAME "battery_queue"
#define CONFIG_BATTERY_SERVICE_MSG_COUNT 10

#define CONFIG_UI_SERVICE_NAME "ui"
#define CONFIG_UI_SERVICE_LABEL ui_service
#define CONFIG_UI_SERVICE_THREAD_NAME "ui_task"
#define CONFIG_UI_SERVICE_THREAD_STACK_SIZE 2048
#define CONFIG_UI_SERVICE_THREAD_PRIORITY osPriorityNormal
#define CONFIG_UI_SERVICE_QUEUE_NAME "ui_queue"
#define CONFIG_UI_SERVICE_MSG_COUNT 10
#define CONFIG_UI_SERVICE_MONITOR_TIMER_ENABLE 1
#define CONFIG_UI_SERVICE_MONITOR_TIMER_NAME "ui_monitor _timer"
#define CONFIG_UI_SERVICE_MONITOR_TIMER_INTERVAL 10000

#define CONFIG_DBG_TRACE_NAME "dbg"
#define CONFIG_DBG_TRACE_LABEL dbg_trace_module
#define CONFIG_DBG_TRACE_MAX_LEN 256

#endif /* __FRAMEWORK_CONF__ */
