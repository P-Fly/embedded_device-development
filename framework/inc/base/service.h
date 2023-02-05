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

#ifndef __SERVICE_H__
#define __SERVICE_H__

#include <stddef.h>
#include <stdint.h>
#include "cmsis_os.h"
#include "object.h"

struct _service_t;
typedef struct _service_t service_t;

/**
 * @brief   Service handle definitions.
 */
typedef struct _service_t
{
    const object*       owner;
    osThreadId_t        thread_id;
    osMessageQueueId_t  queue_id;
    void*               priv;

    int32_t (* init)(const object* obj);
    int32_t (* deinit)(const object* obj);
    void (* message_handler)(const object* obj, const message_t* const message);
} service_t;

/**
 * @brief   Service configuration structure.
 */
typedef struct
{
    osThreadAttr_t          thread_attr;
    osMessageQueueAttr_t    queue_attr;
    uint32_t                msg_count;
} service_config_t;

/**
 * @brief   Service interface definitions.
 */
typedef struct
{
    int32_t (* init)(const object* obj, const service_config_t* const config);
    int32_t (* deinit)(const object* obj);
    void (* message_handler)(const object* obj, const message_t* const message);
} service_intf_t;

extern const service_intf_t service_intf;

extern int32_t service_probe(const object* obj);
extern int32_t service_shutdown(const object* obj);
extern osThreadId_t service_get_thread_id(const object* obj);
extern osMessageQueueId_t service_get_queue_id(const object* obj);
extern void* service_get_priv_data(const object* obj);
extern int32_t service_broadcast_message(const message_t* message);

#define DECLARE_SERVICE(service_name, \
                        service_label, \
                        priv_data, \
                        service_config, \
                        init_fn, \
                        deinit_fn, \
                        message_handler_fn) \
    __define_service(service_name, \
                     service_label, \
                     priv_data, \
                     &service_intf, \
                     service_config, \
                     init_fn, \
                     deinit_fn, \
                     message_handler_fn)

#define __define_service(service_name, \
                         service_label, \
                         priv_data, \
                         intf, \
                         service_config, \
                         init_fn, \
                         deinit_fn, \
                         message_handler_fn) \
    static service_t __service_def_ ## service_label \
    __attribute__((used, section("module_service"))) = { \
        .owner              = NULL, \
        .thread_id          = NULL, \
        .queue_id           = NULL, \
        .init               = (init_fn), \
        .deinit             = (deinit_fn), \
        .message_handler    = (message_handler_fn), \
        .priv               = (priv_data) }; \
    module_service(service_name, \
                   service_label, \
                   service_probe, \
                   service_shutdown, \
                   ((void*)intf), \
                   (&__service_def_ ## service_label), \
                   ((void*)service_config))

#endif /* __SERVICE_H__ */
