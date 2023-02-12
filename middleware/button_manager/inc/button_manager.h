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

#ifndef __BUTTON_MANAGER_H__
#define __BUTTON_MANAGER_H__

typedef enum
{
    BUTTON_STATE_FIRST_DOWN,
    BUTTON_STATE_DOWN,
    BUTTON_STATE_UP,
    BUTTON_STATE_CLICK,

    BUTTON_STATE_BUTT,
} button_state_e;

typedef enum
{
    BUTTON_ID_1 = 0,
    BUTTON_ID_2,
    BUTTON_ID_3,

    BUTTON_ID_BUTT,
} button_id_e;

typedef struct
{
    button_state_e  state;
    const char*     name;
} button_state_recorder_t;

static const button_state_recorder_t button_state_recorder[] =
{
    { BUTTON_STATE_FIRST_DOWN, "FIRST_DOWN"                 },
    { BUTTON_STATE_DOWN,       "DOWN"                       },
    { BUTTON_STATE_UP,         "UP"                         },
    { BUTTON_STATE_CLICK,      "CLICK"                      },
};

static inline const char* button_state_to_name(button_state_e state)
{
    uint32_t i;

    for (i = 0;
         i < sizeof(button_state_recorder) / sizeof(button_state_recorder[0]);
         i++)
    {
        if (button_state_recorder[i].state == state)
        {
            return button_state_recorder[i].name;
        }
    }

    return "UNKNOW";
}

typedef void (* button_user_clbk_t)(button_id_e id, button_state_e state,
                                    const void* user_ctx);

extern int32_t button_manager_register_user_clbk(button_user_clbk_t user_clbk,
                                                 const void*        user_ctx);
extern void button_manager_unregister_user_clbk(void);
extern void button_manager_driver_clbk(button_id_e id);

#endif /* __BUTTON_MANAGER_H__ */
