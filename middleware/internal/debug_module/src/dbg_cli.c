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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "framework.h"
#include "dbg_cli.h"
#include "dbg_module_wrappers.h"

/**
 * @brief   Dbg client handle definition.
 */
typedef struct
{
    uint32_t                    input_ready;
    uint32_t                    input_offset;
    char                        input_buff[CONFIG_DBG_CLI_INPUT_BUFF_SIZE];

    dbg_cli_input_user_clbk_t   input_user_clbk;
    const void*                 input_user_ctx;
} dbg_cli_handle_t;

static dbg_cli_handle_t dbg_cli_handle;

/**
 * @brief   Get the client input buffer.
 *
 * @retval  Client input buffer for reference or NULL in case of not ready.
 */
const char* dbg_cli_input_get(void)
{
    const char* ret;

    taskENTER_CRITICAL();

    if (dbg_cli_handle.input_ready)
    {
        ret = dbg_cli_handle.input_buff;
    }
    else
    {
        ret = NULL;
    }

    taskEXIT_CRITICAL();

    return ret;
}

/**
 * @brief   Free the client input.
 *
 * @retval  None.
 */
void dbg_cli_input_free(void)
{
    taskENTER_CRITICAL();

    dbg_cli_handle.input_ready = 0;

    taskEXIT_CRITICAL();
}

/**
 * @brief   Enable or disable the client input.
 *
 * @param   enable_disable Enable or disable.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t dbg_cli_input_enable(uint32_t enable_disable)
{
    return dbg_uart_read_clbk_enable(enable_disable);
}

/**
 * @brief   Register user callback.
 *
 * @param   user_clbk Pointer to the user callback handler.
 * @param   user_ctx Pointer to user context.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t dbg_cli_input_register_user_clbk(dbg_cli_input_user_clbk_t  user_clbk,
                                         const void*                user_ctx)
{
    if (dbg_cli_handle.input_user_clbk != NULL)
    {
        return -EBUSY;
    }

    dbg_cli_handle.input_user_clbk = user_clbk;
    dbg_cli_handle.input_user_ctx = user_ctx;

    return 0;
}

/**
 * @brief   Unregister user callback.
 *
 * @retval  None.
 */
void dbg_cli_input_unregister_user_clbk(void)
{
    dbg_cli_handle.input_user_clbk = NULL;
    dbg_cli_handle.input_user_ctx = NULL;
}

/**
 * @brief   Callback handler from driver layer.
 *
 * @param   ch Value for input.
 *
 * @retval  None.
 */
void dbg_cli_input_driver_clbk(char ch)
{
    uint32_t isrm = taskENTER_CRITICAL_FROM_ISR();

    if (dbg_cli_handle.input_ready == 0)
    {
        dbg_cli_handle.input_buff[dbg_cli_handle.input_offset++] = ch;

        /* Clear the buffer if input overflow */
        if (dbg_cli_handle.input_offset == CONFIG_DBG_CLI_INPUT_BUFF_SIZE)
        {
            dbg_cli_handle.input_offset = 0;
        }

        /* Ignore the line breaks entered by user */
        if (dbg_cli_handle.input_offset == 2)
        {
            if (dbg_cli_handle.input_buff[0] == 0x0D &&
                dbg_cli_handle.input_buff[1] == 0x0A)
            {
                dbg_cli_handle.input_offset = 0;
            }
        }

        if (dbg_cli_handle.input_offset > 2)
        {
            uint32_t offset = dbg_cli_handle.input_offset;

            if (dbg_cli_handle.input_buff[offset - 2] == 0x0D &&
                dbg_cli_handle.input_buff[offset - 1] == 0x0A)
            {
                dbg_cli_handle.input_ready = 1;
                dbg_cli_handle.input_offset = 0;

                /* Remove the line breaks */
                dbg_cli_handle.input_buff[offset - 2] = 0x00;
                dbg_cli_handle.input_buff[offset - 1] = 0x00;

                if (dbg_cli_handle.input_user_clbk)
                {
                    dbg_cli_handle.input_user_clbk(dbg_cli_handle.input_user_ctx);
                }
            }
        }
    }

    taskEXIT_CRITICAL_FROM_ISR(isrm);
}

/**
 * @brief   Output the formatted data to UART.
 *
 * @retval  The number of data bytes write to the slave on success,
 *          negative error code otherwise.
 */
int32_t dbg_cli_output(const char* format, ...)
{
    char output_buff[CONFIG_DBG_CLI_OUTPUT_BUFF_SIZE];
    int32_t len;
    int32_t ret;

    va_list args;

    va_start(args, format);
    len = vsnprintf(output_buff, CONFIG_DBG_CLI_OUTPUT_BUFF_SIZE, format, args);
    va_end(args);

    ret = dbg_uart_write(output_buff, len);
    if (ret < 0)
    {
        return -EIO;
    }

    return ret;
}

/**
 * @brief   Get sys tick.
 *
 * @retval  Sys tick value.
 */
uint32_t dbg_cli_get_tick(void)
{
    return dbg_get_tick();
}

/**
 * @brief   Probe the dbg client.
 *
 * @param   obj Pointer to the dbg client object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t dbg_cli_probe(const object* obj)
{
    dbg_cli_handle_t* handle = (dbg_cli_handle_t*)obj->object_data;
    int32_t ret;

    (void)memset(handle, 0, sizeof(dbg_cli_handle_t));

    ret = dbg_uart_init();
    if (ret)
    {
        return -EIO;
    }

    return 0;
}

/**
 * @brief   Remove the dbg client.
 *
 * @param   obj Pointer to the dbg client object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t dbg_cli_shutdown(const object* obj)
{
    dbg_cli_handle_t* handle = (dbg_cli_handle_t*)obj->object_data;
    int32_t ret;

    (void)handle;

    ret = dbg_uart_deinit();
    if (ret)
    {
        return -EIO;
    }

    return 0;
}

module_early_driver(CONFIG_DBG_CLI_NAME,
                    CONFIG_DBG_CLI_LABEL,
                    dbg_cli_probe,
                    dbg_cli_shutdown,
                    NULL, &dbg_cli_handle, NULL);
