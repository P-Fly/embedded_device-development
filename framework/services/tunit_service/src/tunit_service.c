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

#include <stdlib.h>
#include <string.h>
#include "cmsis_os.h"
#include "framework.h"
#include "tunit_service.h"
#include "mmi_service.h"

#define tunit_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define tunit_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define tunit_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define tunit_debug(str, ...)   //pr_debug(str, ## __VA_ARGS__)

/**
 * @brief   Private structure for tunit service.
 */
typedef struct
{
    const service_t* owner_svc;
} tunit_service_priv_t;

static tunit_service_priv_t tunit_service_priv;

/**
 * @brief   Initialize the tunit service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t tunit_service_init(const object* obj)
{
    tunit_service_priv_t* priv_data = service_get_priv_data(obj);

    (void)memset(priv_data, 0, sizeof(tunit_service_priv_t));

    priv_data->owner_svc = service_get_svc(obj);

    tunit_info("Service <%s> initialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Deinitialize the tunit service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t tunit_service_deinit(const object* obj)
{
    tunit_service_priv_t* priv_data = service_get_priv_data(obj);

    tunit_info("Service <%s> deinitialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Handle the tunit service message.
 *
 * @param   obj Pointer to the service object handle.
 * @param   message Pointer to the received message.
 *
 * @retval  None.
 */
static void tunit_service_message_handler(const object*             obj,
                                          const message_t* const    message)
{
    tunit_service_priv_t* priv_data = service_get_priv_data(obj);
    int32_t ret;

    tunit_debug("Service <%s> Received %s(0x%x): 0x%x, 0x%x, 0x%x, 0x%x.",
                obj->name,
                msg_id_to_str(message->id),
                message->id,
                message->param0,
                message->param1,
                message->param2,
                message->param3);

    switch (message->id)
    {
    case MSG_ID_SYS_RUN_AUTOMATIC_TEST:

        osDelay(20);

        ret = tunit_manager_run_tests();
        if (ret)
        {
            tunit_error("Service <%s> run automatic test failed, ret %d.",
                        obj->name,
                        ret);
        }
        else
        {
            tunit_info("Service <%s> run automatic test succeed.",
                       obj->name);
        }

        break;
    }
}

int32_t tunit_service_run_tests(void)
{
    message_t message;

    (void)memset(&message, 0, sizeof(message));

    message.id = MSG_ID_SYS_RUN_AUTOMATIC_TEST;

    return service_unicast_message(tunit_service_priv.owner_svc, &message);
}

static const service_config_t tunit_service_config =
{
    .thread_attr    =
    {
        .name       = CONFIG_TUNIT_SERVICE_THREAD_NAME,
        .attr_bits  = osThreadDetached,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .stack_mem  = NULL,
        .stack_size = CONFIG_TUNIT_SERVICE_THREAD_STACK_SIZE,
        .priority   = CONFIG_TUNIT_SERVICE_THREAD_PRIORITY,
    },

    .queue_attr     =
    {
        .name       = CONFIG_TUNIT_SERVICE_QUEUE_NAME,
        .attr_bits  = 0,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .mq_mem     = NULL,
        .mq_size    = 0,
    },

    .msg_count      = CONFIG_TUNIT_SERVICE_MSG_COUNT,
};

DECLARE_SERVICE(CONFIG_TUNIT_SERVICE_NAME,
                CONFIG_TUNIT_SERVICE_LABEL,
                &tunit_service_priv,
                &tunit_service_config,
                tunit_service_init,
                tunit_service_deinit,
                tunit_service_message_handler);

#ifdef CONFIG_MMI_SERVICE_INTERNAL_COMMAND_ENABLE
static BaseType_t mmi_command_run_tests(char*       output,
                                        size_t      output_size,
                                        const char* input)
{
    tunit_service_run_tests();

    snprintf(output,
             output_size,
             "\r\n%s: \r\n Command execute done.\r\n",
             input);

    return pdFALSE;
}

DECLARE_MMI_COMMAND("run_tests",
                    run_tests,
                    "\r\nrun_tests:\r\n Run automatic test.\r\n",
                    mmi_command_run_tests,
                    0);
#endif
