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

#ifndef __BATTERY_SERVICE_H__
#define __BATTERY_SERVICE_H__

#include "adc_manager.h"

typedef enum
{
    BATTERY_STATE_LOW,
    BATTERY_STATE_NORMAL,

    BATTERY_STATE_BUTT,
} battery_state_e;

typedef struct
{
    battery_state_e state;
    const char*     name;
} battery_state_recorder_t;

static const battery_state_recorder_t battery_state_recorder[] =
{
    { BATTERY_STATE_LOW,    "LOW"    },
    { BATTERY_STATE_NORMAL, "NORMAL" },
};

static inline const char* battery_state_to_name(battery_state_e state)
{
    uint32_t i;

    for (i = 0;
         i < sizeof(battery_state_recorder) / sizeof(battery_state_recorder[0]);
         i++)
    {
        if (battery_state_recorder[i].state == state)
        {
            return battery_state_recorder[i].name;
        }
    }

    return "UNKNOW";
}

#endif /* __BATTERY_SERVICE_H__ */
