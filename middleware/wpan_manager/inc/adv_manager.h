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

#ifndef __ADV_MANAGER_H__
#define __ADV_MANAGER_H__

#include "wpan_conf.h"

typedef enum
{
    ADV_STATE_IDLE,
    ADV_STATE_FAST,
    ADV_STATE_LP,

    ADV_STATE_BUTT,
} adv_state_e;

#define ADV_TIMEOUT_EVT_CODE 0x01

typedef void (* adv_user_clbk_t)(uint32_t evt_code, const void* user_ctx);

extern int32_t adv_init(adv_user_clbk_t user_clbk, const void* user_ctx);
extern int32_t adv_setup(adv_state_e state);

#endif /* __ADV_MANAGER_H__ */
