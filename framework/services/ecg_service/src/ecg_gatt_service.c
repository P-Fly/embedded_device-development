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
#include "ecg_gatt_service.h"

#define ecg_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define ecg_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define ecg_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define ecg_debug(str, ...)   pr_debug(str, ## __VA_ARGS__)

#define MAX_ATTR_ITEMS 16
#define MAX_ATTR_VALUE_LENGTH 128
#define ENCRY_KEY_SIZE 10

typedef struct
{
    uint16_t    gatt_service_handle;
    uint16_t    char_measurment_handle;
} ecg_gatt_service_handle_t;

static ecg_gatt_service_handle_t ecg_gatt_service_handle;

static SVCCTL_EvtAckStatus_t ecg_gatt_service_handler(void* Event)
{
    return SVCCTL_EvtNotAck;
}

int32_t ecg_gatt_service_init(void)
{
    uint16_t uuid;
    tBleStatus status = BLE_STATUS_SUCCESS;

    SVCCTL_RegisterSvcHandler(ecg_gatt_service_handler);

    uuid = HEART_RATE_SERVICE_UUID;
    status = aci_gatt_add_service(UUID_TYPE_16,
                                  (Service_UUID_t*)&uuid,
                                  PRIMARY_SERVICE,
                                  MAX_ATTR_ITEMS,
                                  &(ecg_gatt_service_handle.gatt_service_handle));
    if (status != BLE_STATUS_SUCCESS)
    {
        ecg_error("Add electrocar diogram service failed, status %d.", status);

        return -EINVAL;
    }
    else
    {
        ecg_info("Add electrocar diogram service succeed.");
    }

    uuid = HEART_RATE_MEASURMENT_UUID;
    status = aci_gatt_add_char(ecg_gatt_service_handle.gatt_service_handle,
                               UUID_TYPE_16,
                               (Char_UUID_t*)&uuid,
                               MAX_ATTR_VALUE_LENGTH,
                               CHAR_PROP_NOTIFY,
                               ATTR_PERMISSION_NONE,
                               GATT_DONT_NOTIFY_EVENTS,
                               ENCRY_KEY_SIZE,
                               1,
                               &(ecg_gatt_service_handle.char_measurment_handle));
    if (status != BLE_STATUS_SUCCESS)
    {
        ecg_error("Add measurement characteristic failed, status %d.", status);

        return -EINVAL;
    }
    else
    {
        ecg_info("Add measurement characteristic succeed.");
    }

    ecg_info("Initialize ecg gatt service succeed.");

    return 0;
}

int32_t ecg_gatt_service_update_measurment_value(uint8_t* value, uint8_t len)
{
    tBleStatus status;

    status = aci_gatt_update_char_value(
        ecg_gatt_service_handle.gatt_service_handle,
        ecg_gatt_service_handle.char_measurment_handle,
        0,
        len,
        value);
    if (status != BLE_STATUS_SUCCESS)
    {
        ecg_error("Update measurement characteristic failed, status %d.",
                  status);

        return -EIO;
    }

    return 0;
}
