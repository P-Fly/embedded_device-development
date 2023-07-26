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
#include "tunit_manager.h"

#define tunit_error(str, ...)   pr_error(str, ## __VA_ARGS__)
#define tunit_warning(str, ...) pr_warning(str, ## __VA_ARGS__)
#define tunit_info(str, ...)    pr_info(str, ## __VA_ARGS__)
#define tunit_debug(str, ...)   pr_debug(str, ## __VA_ARGS__)

/**
 * @brief   Tunit manager handle definition.
 */
typedef struct
{
    uint32_t reserved;
} tunit_manager_handle_t;

static tunit_manager_handle_t tunit_manager_handle;

/**
 * @brief   Register new case sets and add to the specified suite.
 *
 * @param   suite Test suite to which to add new case.
 * @param   suite_name Name for the new test suite.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t tunit_manager_register_case(const CU_pSuite  suite,
                                           const char*      suite_name)
{
    extern tunit_manager_case_t tunit_case$$Base[];
    extern tunit_manager_case_t tunit_case$$Limit[];

    const tunit_manager_case_t* start = tunit_case$$Base;
    const tunit_manager_case_t* end = tunit_case$$Limit;
    const tunit_manager_case_t* test;
    CU_pTest new_test;

    for (test = start; test < end; test++)
    {
        if (test && !strcmp(suite_name, test->suite_name))
        {
            new_test = CU_add_test(suite, test->case_name, test->case_func);
            if (!new_test)
            {
                return -EINVAL;
            }
        }
    }

    return 0;
}

/**
 * @brief   Register all suites and case sets.
 *
 * @param   None.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t tunit_manager_register_suite_and_case(void)
{
    extern tunit_manager_suite_t tunit_suite$$Base[];
    extern tunit_manager_suite_t tunit_suite$$Limit[];

    const tunit_manager_suite_t* start = tunit_suite$$Base;
    const tunit_manager_suite_t* end = tunit_suite$$Limit;
    const tunit_manager_suite_t* suite;
    CU_pSuite new_suite;
    int32_t ret;

    for (suite = start; suite < end; suite++)
    {
        new_suite = CU_add_suite(suite->suite_name,
                                 suite->initialize,
                                 suite->cleanup);
        if (!new_suite)
        {
            return -EINVAL;
        }

        ret = tunit_manager_register_case(new_suite, suite->suite_name);
        if (ret)
        {
            return ret;
        }
    }

    return 0;
}

/**
 * @brief   A unit testing application, depend on CUnit module.
 *
 * @param   None.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
int32_t tunit_manager_run_tests(void)
{
    CU_ErrorCode error_code;

    error_code = CU_basic_run_tests();
    if (error_code != CUE_SUCCESS)
    {
        return -EINVAL;
    }

    return 0;
}

/**
 * @brief   Probe the tunit manager.
 *
 * @param   obj Pointer to the tunit manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t tunit_manager_probe(const object* obj)
{
    tunit_manager_handle_t* handle = (tunit_manager_handle_t*)obj->object_data;
    CU_ErrorCode error_code;
    int32_t ret;

    (void)handle;

    error_code = CU_initialize_registry();
    if (error_code != CUE_SUCCESS)
    {
        return -EINVAL;
    }

    ret = tunit_manager_register_suite_and_case();
    if (ret)
    {
        return ret;
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);

    tunit_info("Manager <%s> probe succeed.", obj->name);

    return 0;
}

/**
 * @brief   Remove the tunit manager.
 *
 * @param   obj Pointer to the tunit manager object handle.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
static int32_t tunit_manager_shutdown(const object* obj)
{
    tunit_manager_handle_t* handle = (tunit_manager_handle_t*)obj->object_data;

    (void)handle;

    CU_cleanup_registry();

    tunit_info("Manager <%s> shutdown succeed.", obj->name);

    return 0;
}

module_middleware(CONFIG_TUNIT_MANAGER_NAME,
                  CONFIG_TUNIT_MANAGER_LABEL,
                  tunit_manager_probe,
                  tunit_manager_shutdown,
                  NULL, &tunit_manager_handle, NULL);

#ifdef CONFIG_TUNIT_MANAGER_INTERNAL_CASE_ENABLE
static int tunit_bist_initialize(void)
{
    return 0;
}

static int tunit_bist_cleanup(void)
{
    return 0;
}

static void tunit_bist_case_1(void)
{
    TUNIT_TEST(1 == 1);
    TUNIT_TEST('0' == 0x30);
    TUNIT_TEST('a' == 0x61);
}

static void tunit_bist_case_2(void)
{
    CU_ASSERT_EQUAL(1, 1);
    CU_ASSERT_EQUAL('0', 0x30);
    CU_ASSERT_EQUAL('a', 0x61);
}

DECLARE_TUNIT_SUITE("Built-in self-test",
                    bist,
                    tunit_bist_initialize,
                    tunit_bist_cleanup);

DECLARE_TUNIT_CASE("Built-in self-test",
                   "Test case 1",
                   test_case_1,
                   tunit_bist_case_1);

DECLARE_TUNIT_CASE("Built-in self-test",
                   "Test case 2",
                   test_case_2,
                   tunit_bist_case_2);
#endif
