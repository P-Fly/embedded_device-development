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

#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    uint32_t    id;

    uint32_t    param0;
    uint32_t    param1;
    uint32_t    param2;
    uint32_t    param3;
} __attribute__((packed)) message_t;

#define MSG_ID_SYS_BASE     0x00000100
#define MSG_ID_LED_BASE     0x00000200
#define MSG_ID_BTN_BASE     0x00000300
#define MSG_ID_BLE_BASE     0x00000400
#define MSG_ID_MMI_BASE     0x00000500

/**
 * @brief           Notify system startup is completed.
 *
 * @message.id      MSG_ID_SYS_STARTUP_COMPLETED
 * @message.param0  None.
 * @message.param1  None.
 * @message.param2  None.
 * @message.param3  None.
 */
#define MSG_ID_SYS_STARTUP_COMPLETED (MSG_ID_SYS_BASE | 0x01)

/**
 * @brief           Set LED type.
 *
 * @message.id      MSG_ID_LED_SETUP
 * @message.param0  led_id_e.
 * @message.param1  led_type_e.
 * @message.param2  None.
 * @message.param3  None.
 */
#define MSG_ID_LED_SETUP (MSG_ID_LED_BASE | 0x01)

/**
 * @brief           Notify button state.
 *
 * @message.id      MSG_ID_BTN_STATE_NOTIFY
 * @message.param0  button_id_e.
 * @message.param1  button_state_e.
 * @message.param2  None.
 * @message.param3  None.
 */
#define MSG_ID_BTN_STATE_NOTIFY (MSG_ID_BTN_BASE | 0x01)

/**
 * @brief           Notify BLE SHCI ready.
 *
 * @message.id      MSG_ID_BLE_SHCI_READY
 * @message.param0  None.
 * @message.param1  None.
 * @message.param2  None.
 * @message.param3  None.
 */
#define MSG_ID_BLE_SHCI_READY (MSG_ID_BLE_BASE | 0x01)

/**
 * @brief           Notify BLE ADV timeout.
 *
 * @message.id      MSG_ID_BLE_ADV_TIMEOUT
 * @message.param0  None.
 * @message.param1  None.
 * @message.param2  None.
 * @message.param3  None.
 */
#define MSG_ID_BLE_ADV_TIMEOUT (MSG_ID_BLE_BASE | 0x02)

/**
 * @brief           Notify BLE HCI connected.
 *
 * @message.id      MSG_ID_BLE_HCI_CONNECTED
 * @message.param0  None.
 * @message.param1  None.
 * @message.param2  None.
 * @message.param3  None.
 */
#define MSG_ID_BLE_HCI_CONNECTED (MSG_ID_BLE_BASE | 0x03)

/**
 * @brief           Notify BLE HCI disconnected.
 *
 * @message.id      MSG_ID_BLE_HCI_DISCONNECTED
 * @message.param0  None.
 * @message.param1  None.
 * @message.param2  None.
 * @message.param3  None.
 */
#define MSG_ID_BLE_HCI_DISCONNECTED (MSG_ID_BLE_BASE | 0x04)

/**
 * @brief           Notify client input is completed.
 *
 * @message.id      MSG_ID_MMI_CLIENT_INPUT_NOTIFY
 * @message.param0  mmi_cli_type_e.
 * @message.param1  None.
 * @message.param2  None.
 * @message.param3  None.
 */
#define MSG_ID_MMI_CLIENT_INPUT_NOTIFY (MSG_ID_MMI_BASE | 0x01)

extern const char* msg_id_to_str(uint32_t id);
extern int32_t msg_sys_startup_completed(void);

#endif /* __MESSAGE_H__ */
