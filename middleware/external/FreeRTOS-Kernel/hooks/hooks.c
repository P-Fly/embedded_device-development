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

#include <assert.h>

#include "FreeRTOS.h"
#include "task.h"
#include "stm32wbxx.h"
#include "framework.h"

/**
 * @brief   Function to malloc failed hook.
 */
void vApplicationMallocFailedHook(void)
{
    char* name = pcTaskGetName(xTaskGetCurrentTaskHandle());

    pr_error("Malloc failed at task <%s>.", name);

    assert(0);
}

/**
 * @brief   Function to stack overflow hook.
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
    pr_error("Stack overflow at task <%s>.", pcTaskName);
    pr_error("Water mark: %d.", uxTaskGetStackHighWaterMark(xTask));
    pr_error("Free heap size: %d.", xPortGetFreeHeapSize( ));

    assert(0);
}

/**
 * @brief   Function to tick hook.
 */
void vApplicationTickHook(void)
{
    HAL_IncTick();
}
