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

#ifndef __MMI_SERVICE_H__
#define __MMI_SERVICE_H__

#include "cmsis_os.h"
#include "FreeRTOS_CLI.h"

/**
 * @brief   Man-machine client type.
 */
typedef enum
{
    MMI_CLI_DBG = 0,
    MMI_CLI_BLE = 1,
    MMI_CLI_USB = 2,

    MMI_CLI_BUTT,
} mmi_cli_type_e;

#define DECLARE_MMI_COMMAND(command, \
                            command_label, \
                            command_help, \
                            command_fn, \
                            command_parameters_number) \
    static const CLI_Command_Definition_t __mmi_command_def_ ## command_label \
    __attribute__((used, section("mmi_command"))) = { \
        .pcCommand                      = (command), \
        .pcHelpString                   = (command_help), \
        .pxCommandInterpreter           = (command_fn), \
        .cExpectedNumberOfParameters    = (command_parameters_number) }

#endif /* __MMI_SERVICE_H__ */
