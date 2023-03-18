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
#include "ui_service.h"
#include "led_service.h"
#include "shci_manager.h"
#include "hci_manager.h"
#include "adv_manager.h"

#define ble_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define ble_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define ble_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define ble_debug(str, ...)   //pr_debug(str, ##__VA_ARGS__)

/**
 * @brief   Private structure for ble service.
 */
typedef struct
{
    system_sm_e system_state;

    uint8_t     shci_is_ready;
    uint8_t     hci_is_connected;
    uint8_t     adv_is_running;
} ble_service_priv_t;

static void ble_service_sm_processing(ble_service_priv_t*   priv_data,
                                      adv_state_e           new_adv_state,
                                      uint32_t              new_connect_state);
static int32_t ble_service_shci_ready(void);
static int32_t ble_service_adv_timeout(void);
static int32_t ble_service_hci_connected(void);
static int32_t ble_service_hci_disconnected(void);

static void ble_service_shci_tl_user_clbk(uint32_t      evt_code,
                                          const void*   user_ctx)
{
    ble_service_priv_t* priv_data = (ble_service_priv_t*)user_ctx;

    (void)priv_data;

    if (evt_code == SHCI_SUB_EVT_CODE_READY)
    {
        ble_service_shci_ready();
    }
}

static void ble_service_hci_tl_user_clbk(uint32_t       evt_code,
                                         const void*    user_ctx)
{
    ble_service_priv_t* priv_data = (ble_service_priv_t*)user_ctx;

    (void)priv_data;

    if (evt_code == HCI_DISCONNECTION_COMPLETE_EVT_CODE)
    {
        ble_service_hci_disconnected();
    }
    else if (evt_code == HCI_LE_CONNECTION_COMPLETE_SUBEVT_CODE)
    {
        ble_service_hci_connected();
    }
}

static void ble_service_adv_user_clbk(uint32_t      evt_code,
                                      const void*   user_ctx)
{
    ble_service_priv_t* priv_data = (ble_service_priv_t*)user_ctx;

    (void)priv_data;

    if (evt_code == ADV_TIMEOUT_EVT_CODE)
    {
        ble_service_adv_timeout();
    }
}

/**
 * @brief   Initialize the ble service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t ble_service_init(const object* obj)
{
    ble_service_priv_t* priv_data = service_get_priv_data(obj);
    int32_t ret;

    (void)memset(priv_data, 0, sizeof(ble_service_priv_t));

    priv_data->system_state = SYSTEM_SM_INITIALIZE;
    priv_data->shci_is_ready = 0;
    priv_data->adv_is_running = 0;
    priv_data->hci_is_connected = 0;

    ret = shci_tl_init(ble_service_shci_tl_user_clbk, priv_data);
    if (ret)
    {
        ble_error(
            "Service <%s> initialize shci transport layer failed, ret %d.",
            obj->name,
            ret);

        return ret;
    }

    ble_info("Service <%s> initialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Deinitialize the ble service.
 *
 * @param   obj Pointer to the service object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t ble_service_deinit(const object* obj)
{
    ble_service_priv_t* priv_data = service_get_priv_data(obj);

    ble_info("Service <%s> deinitialize succeed.", obj->name);

    return 0;
}

/**
 * @brief   Handle the ble service message.
 *
 * @param   obj Pointer to the service object handle.
 * @param   message Pointer to the received message.
 *
 * @retval  None.
 */
static void ble_service_message_handler(const object*           obj,
                                        const message_t* const  message)
{
    ble_service_priv_t* priv_data = service_get_priv_data(obj);
    adv_state_e new_adv_state;
    uint32_t new_connect_state;
    int32_t ret;

    ble_debug("Service <%s> Received %s(0x%x): 0x%x, 0x%x, 0x%x, 0x%x.",
              obj->name,
              msg_id_to_name(message->id),
              message->id,
              message->param0,
              message->param1,
              message->param2,
              message->param3);

    switch (message->id)
    {
    case MSG_ID_BLE_SHCI_READY:

        priv_data->shci_is_ready = 1;

        ret = hci_tl_init(ble_service_hci_tl_user_clbk, priv_data);
        if (ret)
        {
            ble_error(
                "Service <%s> initialize hci transport layer failed, ret %d.",
                obj->name,
                ret);
        }

        ret = adv_init(ble_service_adv_user_clbk, priv_data);
        if (ret)
        {
            ble_error(
                "Service <%s> initialize advertisement failed, ret %d.",
                obj->name,
                ret);
        }

        break;

    case MSG_ID_BLE_ADV_TIMEOUT:

        new_adv_state = ADV_STATE_LP;
        new_connect_state = 0xFF;

        ble_service_sm_processing(priv_data, new_adv_state, new_connect_state);

        break;

    case MSG_ID_BLE_HCI_CONNECTED:

        /*
            The advertising is automatically stopped when
            the device is connected.
        */
        priv_data->adv_is_running = 0;

        new_adv_state = ADV_STATE_IDLE;
        new_connect_state = 1;

        ble_service_sm_processing(priv_data, new_adv_state, new_connect_state);

        break;

    case MSG_ID_BLE_HCI_DISCONNECTED:

        new_adv_state = ADV_STATE_FAST;
        new_connect_state = 0;

        ble_service_sm_processing(priv_data, new_adv_state, new_connect_state);

        break;

    case MSG_ID_SYS_SM_CHANGED:

        priv_data->system_state = (system_sm_e)message->param0;

        new_adv_state = ADV_STATE_FAST;
        new_connect_state = 0xFF;

        ble_service_sm_processing(priv_data, new_adv_state, new_connect_state);

        break;
    }
}

static void ble_service_sm_processing(ble_service_priv_t*   priv_data,
                                      adv_state_e           new_adv_state,
                                      uint32_t              new_connect_state)
{
    ble_info(
        "Process ble begin: sys %d, shci %d, con %d, adv %d, new_adv %d, new_con %d.",
        priv_data->system_state,
        priv_data->shci_is_ready,
        priv_data->hci_is_connected,
        priv_data->adv_is_running,
        new_adv_state,
        new_connect_state);

    switch (priv_data->system_state)
    {
    case SYSTEM_SM_PREPARE:
    case SYSTEM_SM_RUNNING:

        if (new_connect_state == 1)
        {
            priv_data->hci_is_connected = 1;

            if (priv_data->adv_is_running)
            {
                adv_setup(ADV_STATE_IDLE);

                priv_data->adv_is_running = 0;
            }

            led_service_setup_send(LED_ID_1, LED_TYPE_TURN_ON);
        }
        else if (new_connect_state == 0)
        {
            priv_data->hci_is_connected = 0;

            adv_setup(ADV_STATE_FAST);

            priv_data->adv_is_running = 1;

            led_service_setup_send(LED_ID_1, LED_TYPE_QUICK_FLASH);
        }
        else
        {
            if (!priv_data->hci_is_connected)
            {
                if (new_adv_state == ADV_STATE_LP)
                {
                    adv_setup(ADV_STATE_LP);

                    priv_data->adv_is_running = 1;

                    led_service_setup_send(LED_ID_1, LED_TYPE_SLOW_FLASH);
                }
                else if (new_adv_state == ADV_STATE_FAST)
                {
                    if (!priv_data->adv_is_running)
                    {
                        adv_setup(ADV_STATE_FAST);

                        priv_data->adv_is_running = 1;

                        led_service_setup_send(LED_ID_1, LED_TYPE_QUICK_FLASH);
                    }
                }
            }
        }

        break;

    case SYSTEM_SM_IDLE:
    case SYSTEM_SM_SHUTDOWN:

        if (priv_data->adv_is_running)
        {
            adv_setup(ADV_STATE_IDLE);

            priv_data->adv_is_running = 0;
        }

        if (priv_data->hci_is_connected)
        {
            hci_tl_disconnect();

            priv_data->hci_is_connected = 0;
        }

        led_service_setup_send(LED_ID_1, LED_TYPE_TURN_OFF);

        break;
    }

    ble_info("Process ble end: sys %d, shci %d, con %d, adv %d.",
             priv_data->system_state,
             priv_data->shci_is_ready,
             priv_data->hci_is_connected,
             priv_data->adv_is_running);
}

static int32_t ble_service_shci_ready(void)
{
    message_t message;

    (void)memset(&message, 0, sizeof(message));

    message.id = MSG_ID_BLE_SHCI_READY;

    return service_broadcast_message(&message);
}

static int32_t ble_service_adv_timeout(void)
{
    message_t message;

    (void)memset(&message, 0, sizeof(message));

    message.id = MSG_ID_BLE_ADV_TIMEOUT;

    return service_broadcast_message(&message);
}

static int32_t ble_service_hci_connected(void)
{
    message_t message;

    (void)memset(&message, 0, sizeof(message));

    message.id = MSG_ID_BLE_HCI_CONNECTED;

    return service_broadcast_message(&message);
}

static int32_t ble_service_hci_disconnected(void)
{
    message_t message;

    (void)memset(&message, 0, sizeof(message));

    message.id = MSG_ID_BLE_HCI_DISCONNECTED;

    return service_broadcast_message(&message);
}

static ble_service_priv_t ble_service_priv;

static const service_config_t ble_service_config =
{
    .thread_attr    =
    {
        .name       = CONFIG_BLE_SERVICE_THREAD_NAME,
        .attr_bits  = osThreadDetached,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .stack_mem  = NULL,
        .stack_size = CONFIG_BLE_SERVICE_THREAD_STACK_SIZE,
        .priority   = CONFIG_BLE_SERVICE_THREAD_PRIORITY,
    },

    .queue_attr     =
    {
        .name       = CONFIG_BLE_SERVICE_QUEUE_NAME,
        .attr_bits  = 0,
        .cb_mem     = NULL,
        .cb_size    = 0,
        .mq_mem     = NULL,
        .mq_size    = 0,
    },

    .msg_count      = CONFIG_BLE_SERVICE_MSG_COUNT,
};

DECLARE_SERVICE(CONFIG_BLE_SERVICE_NAME,
                CONFIG_BLE_SERVICE_LABEL,
                &ble_service_priv,
                &ble_service_config,
                ble_service_init,
                ble_service_deinit,
                ble_service_message_handler);
