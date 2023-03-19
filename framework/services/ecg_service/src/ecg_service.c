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
#include "cmsis_os.h"
#include "framework.h"
#include "ecg_service.h"
#include "ecg_gatt_service.h"
#include "ui_service.h"

#define ecg_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define ecg_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define ecg_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define ecg_debug(str, ...)   //pr_debug(str, ##__VA_ARGS__)

/**
 * @brief   Private structure for ECG service.
 */
typedef struct
{
    uint32_t    enable;
    system_sm_e system_state;

    uint16_t    data_buff[8];
    uint8_t     data_cnt;
} ecg_service_priv_t;

static void ecg_service_user_clbk(adc_id_e      id,
                                  uint16_t      data,
                                  const void*   user_ctx);

/**
 * @brief   Initialize the ECG service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t ecg_service_init(const object* obj)
{
    ecg_service_priv_t* priv_data = service_get_priv_data(obj);
    int32_t ret;

    (void)memset(priv_data, 0, sizeof(ecg_service_priv_t));

    priv_data->system_state = SYSTEM_SM_INITIALIZE;
    priv_data->enable = 0;
    priv_data->data_cnt = 0;

    ret = adc_manager_register_user_clbk(ADC_ID_2,
                                         ecg_service_user_clbk,
                                         priv_data);
    if (ret)
    {
        ecg_error("Service <%s> register user callback failed, ret %d.",
                  obj->name,
                  ret);
        return ret;
    }

    ecg_info("Service <%s> initialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Deinitialize the ECG service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t ecg_service_deinit(const object* obj)
{
    ecg_service_priv_t* priv_data = service_get_priv_data(obj);

    ecg_info("Service <%s> deinitialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Handle the ECG service message.
 *
 * @param   obj Pointer to the service object handle.
 * @param   message Pointer to the received message.
 *
 * @retval  None.
 */
static void ecg_service_message_handler(const object*           obj,
                                        const message_t* const  message)
{
    ecg_service_priv_t* priv_data = service_get_priv_data(obj);
    uint16_t data_buff[8];

    ecg_debug("Service <%s> Received %s(0x%x): 0x%x, 0x%x, 0x%x, 0x%x.",
              obj->name,
              msg_id_to_name(message->id),
              message->id,
              message->param0,
              message->param1,
              message->param2,
              message->param3);

    switch (message->id)
    {
    case MSG_ID_SYS_SM_CHANGED:

        priv_data->system_state = (system_sm_e)message->param0;

        break;

    case MSG_ID_BLE_HCI_CONNECTED:

        ecg_info("Service <%s> enable.", obj->name);

        priv_data->enable = 1;

        break;

    case MSG_ID_BLE_HCI_DISCONNECTED:

        ecg_info("Service <%s> disable.", obj->name);

        priv_data->enable = 0;

        break;

    case MSG_ID_ECG_DATA:

        if (priv_data->enable == 1 &&
            priv_data->system_state == SYSTEM_SM_RUNNING)
        {
            (void)memset(data_buff, 0, sizeof(data_buff));

            data_buff[0] = (uint16_t)((message->param0 >> 0) & 0xFFFF);
            data_buff[1] = (uint16_t)((message->param0 >> 16) & 0xFFFF);
            data_buff[2] = (uint16_t)((message->param1 >> 0) & 0xFFFF);
            data_buff[3] = (uint16_t)((message->param1 >> 16) & 0xFFFF);
            data_buff[4] = (uint16_t)((message->param2 >> 0) & 0xFFFF);
            data_buff[5] = (uint16_t)((message->param2 >> 16) & 0xFFFF);
            data_buff[6] = (uint16_t)((message->param3 >> 0) & 0xFFFF);
            data_buff[7] = (uint16_t)((message->param3 >> 16) & 0xFFFF);

            ecg_gatt_service_update_measurment_value((uint8_t*)data_buff,
                                                     sizeof(data_buff));
        }

        break;
    }
}

static int32_t ecg_service_data_notify(uint16_t* data)
{
    message_t message;

    (void)memset(&message, 0, sizeof(message));

    message.id = MSG_ID_ECG_DATA;
    message.param0 = ((data[1] & 0xFFFF) << 16) | (data[0] & 0xFFFF);
    message.param1 = ((data[3] & 0xFFFF) << 16) | (data[2] & 0xFFFF);
    message.param2 = ((data[5] & 0xFFFF) << 16) | (data[4] & 0xFFFF);
    message.param3 = ((data[7] & 0xFFFF) << 16) | (data[6] & 0xFFFF);

    return service_broadcast_message(&message);
}

static void ecg_service_user_clbk(adc_id_e      id,
                                  uint16_t      data,
                                  const void*   user_ctx)
{
    ecg_service_priv_t* priv_data = (ecg_service_priv_t*)user_ctx;

    if (priv_data->enable != 1 || priv_data->system_state != SYSTEM_SM_RUNNING)
    {
        priv_data->data_cnt = 0;

        return;
    }

    priv_data->data_buff[priv_data->data_cnt] = data;

    priv_data->data_cnt++;

    if (priv_data->data_cnt == 8)
    {
        priv_data->data_cnt = 0;

        (void)ecg_service_data_notify(priv_data->data_buff);
    }
}

static ecg_service_priv_t ecg_service_priv;

static const service_config_t ecg_service_config =
{
    .thread_attr    =
    {
        .name       = CONFIG_ECG_SERVICE_THREAD_NAME,
        .attr_bits  = osThreadDetached,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .stack_mem  = NULL,
        .stack_size = CONFIG_ECG_SERVICE_THREAD_STACK_SIZE,
        .priority   = CONFIG_ECG_SERVICE_THREAD_PRIORITY,
    },

    .queue_attr     =
    {
        .name       = CONFIG_ECG_SERVICE_QUEUE_NAME,
        .attr_bits  = 0,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .mq_mem     = NULL,
        .mq_size    = 0,
    },

    .msg_count      = CONFIG_ECG_SERVICE_MSG_COUNT,
};

DECLARE_SERVICE(CONFIG_ECG_SERVICE_NAME,
                CONFIG_ECG_SERVICE_LABEL,
                &ecg_service_priv,
                &ecg_service_config,
                ecg_service_init,
                ecg_service_deinit,
                ecg_service_message_handler);
