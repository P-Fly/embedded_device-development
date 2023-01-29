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

#include "cmsis_os.h"
#include "framework.h"

/**
 * @brief   Startup hardware early.
 *
 * @retval  None.
 *
 * @note    This process is called before the OS start.
 */
__weak void hardware_early_startup(void)
{
    /**
     * This function should not be modified, when the callback is needed,
     * please override it on product layer.
     */
}

/**
 * @brief   Startup hardware later.
 *
 * @retval  None.
 *
 * @note    This process is called after initialization thread is completed.
 */
__weak void hardware_later_startup(void)
{
    /**
     * This function should not be modified, when the callback is needed,
     * please override it on product layer.
     */
}

/**
 * @brief   Attributes structure for init thread.
 */
const osThreadAttr_t init_attr = {
    .name       = CONFIG_INIT_THREAD_NAME,
    .attr_bits  = osThreadDetached,
    .cb_mem     = NULL,
    .cb_size    = 0,
    .stack_mem  = NULL,
    .stack_size = CONFIG_INIT_THREAD_STACK_SIZE,
    .priority   = CONFIG_INIT_THREAD_PRIORITY,
};

/**
 * @brief   Initialize thread, include all drivers, services, applications etc.
 *
 * @param   argument None.
 *
 * @retval  None.
 */
static void init_thread(void* argument)
{
    int32_t ret;

    (void)argument;

	ret = object_init();
	if (ret)
    {   
		//pr_error("Some objects initialize failed.");
    }
	else
    {   
		//pr_info("All objects initialized.");
    }

    while (1)
    {
        ;
    }
}

/**
 * @brief   The entry function of this project.
 */
__attribute__((noreturn))
int main(int argc, char* argv[])
{
    osThreadId_t thread_id;
    osStatus_t stat;

    hardware_early_startup();

    stat = osKernelInitialize();
    if (stat != osOK)
    {
    }

    thread_id = osThreadNew(init_thread, NULL, &init_attr);
    if (!thread_id)
    {
    }

    stat = osKernelStart();
    if (stat != osOK)
    {
    }

    /* The program is undefined, if the code reaches this point. */
    while (1)
    {
        ;
    }
}
