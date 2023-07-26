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
#include <assert.h>
#include "cmsis_os.h"
#include "FreeRTOS_CLI.h"
#include "framework.h"
#include "mmi_service.h"
#include "dbg_cli.h"

#define mmi_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define mmi_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define mmi_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define mmi_debug(str, ...)   //pr_debug(str, ## __VA_ARGS__)

/**
 * @brief   Private structure for man-machine service.
 */
typedef struct
{
    const service_t* owner_svc;
} mmi_service_priv_t;

static mmi_service_priv_t mmi_service_priv;

/**
 * @brief   Private structure for client.
 */
typedef struct
{
    mmi_cli_type_e      type;

    mmi_service_priv_t* service_priv;
} mmi_client_priv_t;

static mmi_client_priv_t mmi_client_priv[MMI_CLI_BUTT];

/**
 * @brief   Client user callback.
 *
 * @param   user_ctx Pointer to the user_ctx handle.
 *
 * @retval  None.
 */
static void mmi_service_client_user_clbk(const void* user_ctx)
{
    mmi_client_priv_t* priv_data = (mmi_client_priv_t*)user_ctx;
    message_t message;

    (void)memset(&message, 0, sizeof(message));

    message.id = MSG_ID_MMI_CLIENT_INPUT_NOTIFY;
    message.param0 = priv_data->type;

    (void)service_unicast_message(mmi_service_priv.owner_svc, &message);
}

/**
 * @brief   Register the man-machine command.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t mmi_service_register_command(void)
{
    extern CLI_Command_Definition_t mmi_command$$Base[];
    extern CLI_Command_Definition_t mmi_command$$Limit[];

    const CLI_Command_Definition_t* start = mmi_command$$Base;
    const CLI_Command_Definition_t* end = mmi_command$$Limit;
    const CLI_Command_Definition_t* command;
    BaseType_t ret;

    for (command = start; command < end; command++)
    {
        ret = FreeRTOS_CLIRegisterCommand(command);
        if (ret != pdPASS)
        {
            return -ENOMEM;
        }
    }

    return 0;
}

/**
 * @brief   Initialize the man-machine service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t mmi_service_init(const object* obj)
{
    mmi_service_priv_t* priv_data = service_get_priv_data(obj);
    int32_t ret;

    (void)memset(priv_data, 0, sizeof(mmi_service_priv_t));
    (void)memset(mmi_client_priv, 0, sizeof(mmi_client_priv));

    priv_data->owner_svc = service_get_svc(obj);

    mmi_client_priv[MMI_CLI_DBG].type = MMI_CLI_DBG;
    mmi_client_priv[MMI_CLI_DBG].service_priv = priv_data;
    ret = dbg_cli_input_register_user_clbk(mmi_service_client_user_clbk,
                                           &mmi_client_priv[MMI_CLI_DBG]);
    if (ret)
    {
        mmi_error("Service <%s> register dbg client callback failed, ret %d.",
                  obj->name,
                  ret);
        return ret;
    }

    ret = mmi_service_register_command();
    if (ret)
    {
        mmi_error("Service <%s> register command failed, ret %d.",
                  obj->name,
                  ret);
        return ret;
    }

    mmi_info("Service <%s> initialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Deinitialize the man-machine service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t mmi_service_deinit(const object* obj)
{
    mmi_service_priv_t* priv_data = service_get_priv_data(obj);

    dbg_cli_input_unregister_user_clbk();

    mmi_info("Service <%s> deinitialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Handle the man-machine service message.
 *
 * @param   obj Pointer to the service object handle.
 * @param   message Pointer to the received message.
 *
 * @retval  None.
 */
static void mmi_service_message_handler(const object*           obj,
                                        const message_t* const  message)
{
    mmi_service_priv_t* priv_data = service_get_priv_data(obj);
    const char* input;
    char* output;
    size_t output_size;

    void (* free_fn)(void);
    BaseType_t more_data;
    int32_t ret;

    mmi_debug("Service <%s> Received %s(0x%x): 0x%x, 0x%x, 0x%x, 0x%x.",
              obj->name,
              msg_id_to_str(message->id),
              message->id,
              message->param0,
              message->param1,
              message->param2,
              message->param3);

    switch (message->id)
    {
    case MSG_ID_SYS_STARTUP_COMPLETED:

        ret = dbg_cli_input_enable(1);
        if (ret)
        {
            mmi_error("Enable dbg client failed, ret %d.", ret);
        }

        break;

    case MSG_ID_MMI_CLIENT_INPUT_NOTIFY:

        switch (message->param0)
        {
        case MMI_CLI_DBG:
            input = dbg_cli_input_get();
            free_fn = dbg_cli_input_free;
            break;

        case MMI_CLI_BLE:
            break;

        case MMI_CLI_USB:
            break;
        }

        output = FreeRTOS_CLIGetOutputBuffer();
        output_size = configCOMMAND_INT_MAX_OUTPUT_SIZE;

        do
        {
            more_data = FreeRTOS_CLIProcessCommand(input, output, output_size);

            mmi_info("%s", output);
        }
        while (more_data != pdFALSE);

        (*free_fn)();

        break;
    }
}

static const service_config_t mmi_service_config =
{
    .thread_attr    =
    {
        .name       = CONFIG_MMI_SERVICE_THREAD_NAME,
        .attr_bits  = osThreadDetached,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .stack_mem  = NULL,
        .stack_size = CONFIG_MMI_SERVICE_THREAD_STACK_SIZE,
        .priority   = CONFIG_MMI_SERVICE_THREAD_PRIORITY,
    },

    .queue_attr     =
    {
        .name       = CONFIG_MMI_SERVICE_QUEUE_NAME,
        .attr_bits  = 0,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .mq_mem     = NULL,
        .mq_size    = 0,
    },

    .msg_count      = CONFIG_MMI_SERVICE_MSG_COUNT,
};

DECLARE_SERVICE(CONFIG_MMI_SERVICE_NAME,
                CONFIG_MMI_SERVICE_LABEL,
                &mmi_service_priv,
                &mmi_service_config,
                mmi_service_init,
                mmi_service_deinit,
                mmi_service_message_handler);

#ifdef CONFIG_MMI_SERVICE_INTERNAL_COMMAND_ENABLE
static BaseType_t mmi_command_version(char*         output,
                                      size_t        output_size,
                                      const char*   input)
{
    snprintf(output,
             output_size,
             "\r\n%s:\r\n %s - %s (Build %s %s)\r\n",
             input,
             CONFIG_ISSUE_NAME,
             CONFIG_ISSUE_VERSION,
             CONFIG_ISSUE_DATE,
             CONFIG_ISSUE_TIME);

    return pdFALSE;
}

DECLARE_MMI_COMMAND("version",
                    version,
                    "\r\nversion:\r\n Print certain system version.\r\n",
                    mmi_command_version,
                    0);
#endif
