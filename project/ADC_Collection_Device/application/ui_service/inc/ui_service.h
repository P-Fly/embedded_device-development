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

#ifndef __UI_SERVICE_H__
#define __UI_SERVICE_H__

typedef enum
{
    SYSTEM_SM_INITIALIZE,
    SYSTEM_SM_IDLE,
    SYSTEM_SM_PREPARE,
    SYSTEM_SM_RUNNING,
    SYSTEM_SM_SHUTDOWN,

    SYSTEM_SM_BUTT,
} system_sm_e;

typedef struct
{
    system_sm_e state;
    const char* name;
} system_sm_recorder_t;

static const system_sm_recorder_t system_sm_recorder[] =
{
    { SYSTEM_SM_INITIALIZE, "Initialize" },
    { SYSTEM_SM_IDLE,       "Idle"       },
    { SYSTEM_SM_PREPARE,    "Prepare"    },
    { SYSTEM_SM_RUNNING,    "Running"    },
    { SYSTEM_SM_SHUTDOWN,   "Shutdown"   },
};

static inline const char* system_sm_to_name(system_sm_e state)
{
    uint32_t i;

    for (i = 0;
         i < sizeof(system_sm_recorder) / sizeof(system_sm_recorder[0]);
         i++)
    {
        if (system_sm_recorder[i].state == state)
        {
            return system_sm_recorder[i].name;
        }
    }

    return "UNKNOW";
}

extern int32_t ui_service_startup_completed_send(void);

#endif /* __UI_SERVICE_H__ */
