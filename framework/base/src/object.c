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

#include <stddef.h>
#include <string.h>

#include "object.h"
#include "err.h"

extern object module_object_0$$Base[];
extern object module_object_0$$Limit[];
extern object module_object_1$$Base[];
extern object module_object_1$$Limit[];
extern object module_object_2$$Base[];
extern object module_object_2$$Limit[];
extern object module_object_3$$Base[];
extern object module_object_3$$Limit[];

/**
 * @brief   Define object levels.
 */
static object* object_levels[] =
{
    module_object_0$$Base,
    module_object_0$$Limit,
    module_object_1$$Base,
    module_object_1$$Limit,
    module_object_2$$Base,
    module_object_2$$Limit,
    module_object_3$$Base,
    module_object_3$$Limit,
};

#define OBJECT_LEVELS_NUM (sizeof(object_levels) / sizeof(object_levels[0]))

/**
 * @brief   Execute all the object initialization functions at a given level.
 *
 * @param   level Object level.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t object_do_one_initcall(int32_t level)
{
    const object* obj;
    int32_t ret;

    for (obj = object_levels[level]; obj < object_levels[level + 1];
         obj++)
    {
        if (obj && obj->probe)
        {
            ret = obj->probe(obj);
            if (ret)
            {
                return ret;
            }
        }
    }

    return 0;
}

/**
 * @brief   Execute all the object initialization functions.
 *
 * @param   None.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t object_init(void)
{
    uint32_t level;
    int32_t ret;

    for (level = 0; level < OBJECT_LEVELS_NUM; level += 2)
    {
        ret = object_do_one_initcall(level);
        if (ret)
        {
            return ret;
        }
    }

    return 0;
}

/**
 * @brief   Execute all the object de-initialization functions at a given level.
 *
 * @param   level Object level.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t object_do_one_deinitcall(int32_t level)
{
    const object* obj;
    int32_t ret;

    for (obj = object_levels[level]; obj < object_levels[level + 1];
         obj++)
    {
        if (obj && obj->shutdown)
        {
            ret = obj->shutdown(obj);
            if (ret)
            {
                return ret;
            }
        }
    }

    return 0;
}

/**
 * @brief   Execute all the object de-initialization functions.
 *
 * @param   None.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t object_deinit(void)
{
    uint32_t level;
    int32_t ret;

    for (level = OBJECT_LEVELS_NUM - 2; level == 0; level -= 2)
    {
        ret = object_do_one_deinitcall(level);
        if (ret)
        {
            return ret;
        }
    }

    return 0;
}

/**
 * @brief   Execute all the object suspend functions at a given level.
 *
 * @param   level Object level.
 * @param   suspend_level Suspend level.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t object_do_one_suspendcall(int32_t level, int32_t suspend_level)
{
    const object* obj;
    int32_t ret;

    for (obj = object_levels[level]; obj < object_levels[level + 1];
         obj++)
    {
        if (obj && obj->suspend)
        {
            ret = obj->suspend(obj, suspend_level);
            if (ret)
            {
                return ret;
            }
        }
    }

    return 0;
}

/**
 * @brief   Execute all the object suspend functions.
 *
 * @param   suspend_level Suspend level.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t object_suspend(int32_t suspend_level)
{
    uint32_t level;
    int32_t ret;

    for (level = OBJECT_LEVELS_NUM - 2; level == 0; level -= 2)
    {
        ret = object_do_one_suspendcall(level, suspend_level);
        if (ret)
        {
            return ret;
        }
    }

    return 0;
}

/**
 * @brief   Execute all the object resume functions at a given level.
 *
 * @param   level Object level.
 * @param   resume_level Resume level.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t object_do_one_resumecall(int32_t level, int32_t resume_level)
{
    const object* obj;
    int32_t ret;

    for (obj = object_levels[level]; obj < object_levels[level + 1];
         obj++)
    {
        if (obj && obj->resume)
        {
            ret = obj->resume(obj, resume_level);
            if (ret)
            {
                return ret;
            }
        }
    }

    return 0;
}

/**
 * @brief   Execute all the object resume functions.
 *
 * @param   resume_level Resume level.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t object_resume(int32_t resume_level)
{
    uint32_t level;
    int32_t ret;

    for (level = 0; level < OBJECT_LEVELS_NUM; level += 2)
    {
        ret = object_do_one_resumecall(level, resume_level);
        if (ret)
        {
            return ret;
        }
    }

    return 0;
}

/**
 * @brief   Get the object handle.
 *
 * @param   name Object name.
 *
 * @retval  Object handle for reference or NULL in case of error.
 */
const object* object_get_binding(const char* const name)
{
    uint32_t level;
    const object* obj;
    const object* start;
    const object* end;

    for (level = 0; level < OBJECT_LEVELS_NUM; level += 2)
    {
        start = object_levels[level];
        end = object_levels[level + 1];

        for (obj = start; obj < end; obj++)
        {
            if (obj && obj->object_intf && !strcmp(name, obj->name))
            {
                return obj;
            }
        }
    }

    return NULL;
}
