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
#include "log.h"

/**
 * When the program is asserted, we stop all tasks and wait indefinitely.
 * We didn't disable the IRQ due to need to print out the debug information.
 */

/**
 * @brief   Get file name from full path.
 *
 * @param   fullpath Pointer to the full path.
 *
 * @retval  Pointer to the file name.
 */
static const char* get_file_name(const char* fullpath)
{
    const char* ret = fullpath;

    if (strrchr(fullpath, '\\') != NULL)
    {
        ret = strrchr(fullpath, '\\') + 1;
    }
    else if (strrchr(fullpath, '/') != NULL)
    {
        ret = strrchr(fullpath, '/') + 1;
    }

    return ret;
}

/**
 * @brief   Stop all tasks and wait indefinitely.
 *
 * @retval  None.
 */
static void abort(void)
{
    osKernelLock();

    for (;;)
    {
        ;
    }
}

/**
 * @brief   Prints information on the failing diagnostic and calls abort()
 *
 * @param   expr Assert expression that was not TRUE.
 * @param   file Source file of the assertion.
 * @param   line Source line of the assertion.
 *
 * @retval  None.
 *
 * @note:
 *  If the NDEBUG macro is defined, the assert macro has no effect.
 *  If the assert expression evaluates to FALSE, the assert macro calls
 *      the __aeabi_assert() function if any of the following are true:
 *      You are compiling with --strict.
 *      You are using -O0 or -O1.
 *      __OPT_SMALL_ASSERT is defined.
 *      __ASSERT_MSG is defined.
 *      _AEABI_PORTABILITY_LEVEL is defined and not 0.
 */
void __aeabi_assert(const char* expr, const char* file, int line)
{
    char str[12], * p;

    p = str + sizeof(str);
    *--p = '\0';
    *--p = '\n';

    while (line > 0)
    {
        *--p = '0' + (line % 10);
        line /= 10;
    }

    (void)trace_output("assertion failed: %s, file %s, line %s\r\n",
                       expr,
                       get_file_name(file),
                       p);

    abort();
}
