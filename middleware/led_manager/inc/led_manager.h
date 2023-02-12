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

#ifndef __LED_MANAGER_H__
#define __LED_MANAGER_H__

typedef enum
{
    LED_TYPE_TURN_OFF = 0,
    LED_TYPE_TURN_ON,
    LED_TYPE_QUICK_FLASH,
    LED_TYPE_SLOW_FLASH,

    LED_TYPE_BUTT,
} led_type_e;

typedef enum
{
    LED_ID_1 = 0,
    LED_ID_2,
    LED_ID_3,

    LED_ID_BUTT,
} led_id_e;

typedef struct
{
    led_type_e  type;
    const char* name;
} led_type_recorder_t;

static const led_type_recorder_t led_type_recorder[] =
{
    { LED_TYPE_TURN_OFF,    "TURN_OFF"        },
    { LED_TYPE_TURN_ON,     "TURN_ON"         },
    { LED_TYPE_QUICK_FLASH, "QUICK_FLASH"     },
    { LED_TYPE_SLOW_FLASH,  "SLOW_FLASH"      },
};

static inline const char* led_type_to_name(led_type_e type)
{
    uint32_t i;

    for (i = 0;
         i < sizeof(led_type_recorder) / sizeof(led_type_recorder[0]);
         i++)
    {
        if (led_type_recorder[i].type == type)
        {
            return led_type_recorder[i].name;
        }
    }

    return "UNKNOW";
}

extern int32_t led_manager_setup(led_id_e id, led_type_e type);

#endif /* __LED_MANAGER_H__ */
