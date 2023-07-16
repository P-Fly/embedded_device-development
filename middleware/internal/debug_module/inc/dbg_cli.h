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

#ifndef __DBG_CLI_H__
#define __DBG_CLI_H__

typedef void (* dbg_cli_input_user_clbk_t)(const void* user_ctx);

extern int32_t dbg_cli_input_register_user_clbk(
    dbg_cli_input_user_clbk_t   user_clbk,
    const void*                 user_ctx);
extern void dbg_cli_input_unregister_user_clbk(void);
extern const char* dbg_cli_input_get(void);
extern void dbg_cli_input_free(void);
extern int32_t dbg_cli_input_enable(uint32_t enable_disable);
extern void dbg_cli_input_driver_clbk(char ch);
extern int32_t dbg_cli_output(const char* format, ...);
extern uint32_t dbg_cli_get_tick(void);

#endif /* __DBG_CLI_H__ */
