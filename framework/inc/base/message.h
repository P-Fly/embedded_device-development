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

#define MSG_ID_SYS_SERVICE_BASE     0x00000000
#define MSG_ID_LED_SERVICE_BASE     0x00000100
#define MSG_ID_KEY_SERVICE_BASE     0x00000200
#define MSG_ID_DISPLAY_SERVICE_BASE 0x00000300
#define MSG_ID_DATA_SERVICE_BASE    0x00000400
#define MSG_ID_BLE_SERVICE_BASE     0x00000500
#define MSG_ID_BATTERY_SERVICE_BASE 0x00000600

/**
 * @brief           Notify system startup is completed.
 *
 * @message.id      MSG_ID_SYS_STARTUP_COMPLETED
 * @message.param0  None.
 * @message.param1  None.
 * @message.param2  None.
 * @message.param3  None.
 */
#define MSG_ID_SYS_STARTUP_COMPLETED (MSG_ID_SYS_SERVICE_BASE | 0x0001)

/**
 * @brief           Notify system heartbeat.
 *
 * @message.id      MSG_ID_SYS_HEARTBEAT
 * @message.param0  Increased counter value.
 * @message.param1  None.
 * @message.param2  None.
 * @message.param3  None.
 */
#define MSG_ID_SYS_HEARTBEAT (MSG_ID_SYS_SERVICE_BASE | 0x0002)

typedef struct
{
    uint32_t    id;
    const char* name;
} msg_recorder_t;

static const msg_recorder_t msg_recorder[] =
{
    { MSG_ID_SYS_STARTUP_COMPLETED, "SYS_STARTUP_COMPLETED" },
    { MSG_ID_SYS_HEARTBEAT,         "SYS_HEARTBEAT" },
};

static inline const char* msg_id_to_name(uint32_t id)
{
    uint32_t i;

    for (i = 0; i < sizeof(msg_recorder) / sizeof(msg_recorder[0]); i++)
    {
        if (msg_recorder[i].id == id)
        {
            return msg_recorder[i].name;
        }
    }

    return "UNKNOW";
}

#endif /* __MESSAGE_H__ */
