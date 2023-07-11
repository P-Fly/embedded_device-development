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
} msg_id_recorder_t;

static const msg_id_recorder_t msg_id_recorder[] =
{
    { MSG_ID_SYS_STARTUP_COMPLETED, "SYS_STARTUP_COMPLETED" },
    { MSG_ID_LED_DISABLE,           "LED_DISABLE"           },
    { MSG_ID_LED_ENABLE,            "LED_ENABLE"            },
    { MSG_ID_LED_SETUP,             "LED_SETUP"             },
    { MSG_ID_BTN_DISABLE,           "BTN_DISABLE"           },
    { MSG_ID_BTN_ENABLE,            "BTN_ENABLE"            },
    { MSG_ID_BTN_NOTIFY,            "BTN_NOTIFY"            },
    { MSG_ID_BLE_DISABLE,           "BLE_DISABLE"           },
    { MSG_ID_BLE_ENABLE,            "BLE_ENABLE"            },
    { MSG_ID_BLE_SHCI_READY,        "BLE_SHCI_READY"        },
    { MSG_ID_BLE_ADV_TIMEOUT,       "BLE_ADV_TIMEOUT"       },
    { MSG_ID_BLE_HCI_CONNECTED,     "BLE_HCI_CONNECTED"     },
    { MSG_ID_BLE_HCI_DISCONNECTED,  "BLE_HCI_DISCONNECTED"  },
};

const char* msg_id_to_name(uint32_t id)
{
    uint32_t i;

    for (i = 0; i < sizeof(msg_id_recorder) / sizeof(msg_id_recorder[0]); i++)
    {
        if (msg_id_recorder[i].id == id)
        {
            return msg_id_recorder[i].name;
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
