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

#include <string.h>
#include "cmsis_os.h"
#include "framework.h"

typedef struct
{
    uint32_t    id;
    const char* name;
} msg_id_str_mapping_t;

static const msg_id_str_mapping_t msg_id_str_mapping[] =
{
    { MSG_ID_SYS_STARTUP_COMPLETED,   "SYS_STARTUP_COMPLETED"   },
    { MSG_ID_LED_SETUP,               "LED_SETUP"               },
    { MSG_ID_BTN_STATE_NOTIFY,        "BTN_STATE_NOTIFY"        },
    { MSG_ID_BLE_SHCI_READY,          "BLE_SHCI_READY"          },
    { MSG_ID_BLE_ADV_TIMEOUT,         "BLE_ADV_TIMEOUT"         },
    { MSG_ID_BLE_HCI_CONNECTED,       "BLE_HCI_CONNECTED"       },
    { MSG_ID_BLE_HCI_DISCONNECTED,    "BLE_HCI_DISCONNECTED"    },
    { MSG_ID_MMI_CLIENT_INPUT_NOTIFY, "MMI_CLIENT_INPUT_NOTIFY" },
};

const char* msg_id_to_str(uint32_t id)
{
    uint32_t i;

    for (i = 0; i < sizeof(msg_id_str_mapping) / sizeof(msg_id_str_mapping[0]); i++)
    {
        if (msg_id_str_mapping[i].id == id)
        {
            return msg_id_str_mapping[i].name;
        }
    }

    return "MSG_ID_UNKNOW";
}

int32_t msg_sys_startup_completed(void)
{
    message_t message;

    (void)memset(&message, 0, sizeof(message));

    message.id = MSG_ID_SYS_STARTUP_COMPLETED;

    return service_broadcast_message(&message);
}
