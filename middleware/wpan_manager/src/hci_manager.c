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
#include "cmsis_os.h"
#include "framework.h"
#include "hci_manager.h"

#define hci_manager_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define hci_manager_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define hci_manager_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define hci_manager_debug(str, ...)   pr_debug(str, ## __VA_ARGS__)

/**
 * @brief   HCI manager handle definition.
 */
typedef struct
{
    uint32_t reserved;
} hci_manager_handle_t;

static hci_manager_handle_t hci_manager_handle;

/**
 * @brief   Probe the HCI manager.
 *
 * @param   obj Pointer to the HCI manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t hci_manager_probe(const object* obj)
{
    hci_manager_handle_t* handle = (hci_manager_handle_t*)obj->object_data;

    (void)memset(handle, 0, sizeof(hci_manager_handle_t));

    hci_manager_info("Manager <%s> probe succeed.", obj->name);

    return 0;
}

/**
 * @brief   Remove the HCI manager.
 *
 * @param   obj Pointer to the HCI manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t hci_manager_shutdown(const object* obj)
{
    hci_manager_handle_t* handle = (hci_manager_handle_t*)obj->object_data;

    (void)handle;

    hci_manager_info("Manager <%s> shutdown succeed.", obj->name);

    return 0;
}

module_middleware(CONFIG_HCI_MANAGER_NAME,
                  CONFIG_HCI_MANAGER_LABEL,
                  hci_manager_probe,
                  hci_manager_shutdown,
                  NULL, &hci_manager_handle, NULL);
