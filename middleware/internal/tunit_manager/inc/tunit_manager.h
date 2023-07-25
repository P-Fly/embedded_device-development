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

#ifndef __TUNIT_MANAGER_H__
#define __TUNIT_MANAGER_H__

#include <string.h>
#include "CUnit.h"
#include "Basic.h"

typedef int (* tunit_suite_initialize)(void);
typedef int (* tunit_suite_cleanup)(void);

typedef struct
{
    char*                   suite_name;
    tunit_suite_initialize  initialize;
    tunit_suite_cleanup     cleanup;
} tunit_manager_suite_t;

#define DECLARE_TUNIT_SUITE(_suite_name, \
                            _suite_label, \
                            _suite_initialize, \
                            _suite_cleanup) \
    static tunit_manager_suite_t __tunit_manager_suite_def_ ## _suite_label \
    __attribute__((used, section("tunit_suite"))) = { \
        .suite_name = (_suite_name), \
        .initialize = (_suite_initialize), \
        .cleanup    = (_suite_cleanup) }

typedef void (* tunit_case_func)(void);

typedef struct
{
    char*           suite_name;
    char*           case_name;
    tunit_case_func case_func;
} tunit_manager_case_t;

#define DECLARE_TUNIT_CASE(_suite_name, \
                           _case_name, \
                           _case_label, \
                           _case_func) \
    static tunit_manager_case_t __tunit_manager_case_def_ ## _case_label \
    __attribute__((used, section("tunit_case"))) = { \
        .suite_name = (_suite_name), \
        .case_name  = (_case_name), \
        .case_func  = (_case_func) }

#define TUNIT_PASS(msg) \
    CU_PASS(msg)
#define TUNIT_ASSERT(value) \
    CU_ASSERT(value)
#define TUNIT_ASSERT_FATAL(value) \
    CU_ASSERT_FATAL(value)
#define TUNIT_TEST(value) \
    CU_TEST(value)
#define TUNIT_TEST_FATAL(value) \
    CU_TEST_FATAL(value)
#define TUNIT_FAIL(msg) \
    CU_FAIL(msg)
#define TUNIT_FAIL_FATAL(msg) \
    CU_FAIL_FATAL(msg)
#define TUNIT_ASSERT_TRUE(value) \
    CU_ASSERT_TRUE(value)
#define TUNIT_ASSERT_TRUE_FATAL(value) \
    CU_ASSERT_TRUE_FATAL(value)
#define TUNIT_ASSERT_FALSE(value) \
    CU_ASSERT_FALSE(value)
#define TUNIT_ASSERT_FALSE_FATAL(value) \
    CU_ASSERT_FALSE_FATAL(value)
#define TUNIT_ASSERT_EQUAL(actual, expected) \
    CU_ASSERT_EQUAL(actual, expected)
#define TUNIT_ASSERT_EQUAL_FATAL(actual, expected) \
    CU_ASSERT_EQUAL_FATAL(actual, expected)
#define TUNIT_ASSERT_NOT_EQUAL(actual, expected) \
    CU_ASSERT_NOT_EQUAL(actual, expected)
#define TUNIT_ASSERT_NOT_EQUAL_FATAL(actual, expected) \
    CU_ASSERT_NOT_EQUAL_FATAL(actual, expected)
#define TUNIT_ASSERT_PTR_EQUAL(actual, expected) \
    CU_ASSERT_PTR_EQUAL(actual, expected)
#define TUNIT_ASSERT_PTR_EQUAL_FATAL(actual, expected) \
    CU_ASSERT_PTR_EQUAL_FATAL(actual, expected)
#define TUNIT_ASSERT_PTR_NOT_EQUAL(actual, expected) \
    CU_ASSERT_PTR_NOT_EQUAL(actual, expected)
#define TUNIT_ASSERT_PTR_NOT_EQUAL_FATAL(actual, expected) \
    CU_ASSERT_PTR_NOT_EQUAL_FATAL(actual, expected)
#define TUNIT_ASSERT_PTR_NULL(value) \
    CU_ASSERT_PTR_NULL(value)
#define TUNIT_ASSERT_PTR_NULL_FATAL(value) \
    CU_ASSERT_PTR_NULL_FATAL(value)
#define TUNIT_ASSERT_PTR_NOT_NULL(value) \
    CU_ASSERT_PTR_NOT_NULL(value)
#define TUNIT_ASSERT_PTR_NOT_NULL_FATAL(value) \
    CU_ASSERT_PTR_NOT_NULL_FATAL(value)
#define TUNIT_ASSERT_STRING_EQUAL(actual, expected) \
    CU_ASSERT_STRING_EQUAL(actual, expected)
#define TUNIT_ASSERT_STRING_EQUAL_FATAL(actual, expected) \
    CU_ASSERT_STRING_EQUAL_FATAL(actual, expected)
#define TUNIT_ASSERT_STRING_NOT_EQUAL(actual, expected) \
    CU_ASSERT_STRING_NOT_EQUAL(actual, expected)
#define TUNIT_ASSERT_STRING_NOT_EQUAL_FATAL(actual, expected) \
    CU_ASSERT_STRING_NOT_EQUAL_FATAL(actual, expected)
#define TUNIT_ASSERT_NSTRING_EQUAL(actual, expected, count) \
    CU_ASSERT_NSTRING_EQUAL(actual, expected, count)
#define TUNIT_ASSERT_NSTRING_EQUAL_FATAL(actual, expected, count) \
    CU_ASSERT_NSTRING_EQUAL_FATAL(actual, expected, count)
#define TUNIT_ASSERT_NSTRING_NOT_EQUAL(actual, expected, count) \
    CU_ASSERT_NSTRING_NOT_EQUAL(actual, expected, count)
#define TUNIT_ASSERT_NSTRING_NOT_EQUAL_FATAL(actual, expected, count) \
    CU_ASSERT_NSTRING_NOT_EQUAL_FATAL(actual, expected, count)
#define TUNIT_ASSERT_DOUBLE_EQUAL(actual, expected, granularity) \
    CU_ASSERT_DOUBLE_EQUAL(actual, expected, granularity)
#define TUNIT_ASSERT_DOUBLE_EQUAL_FATAL(actual, expected, granularity) \
    CU_ASSERT_DOUBLE_EQUAL_FATAL(actual, expected, granularity)
#define TUNIT_ASSERT_DOUBLE_NOT_EQUAL(actual, expected, granularity) \
    CU_ASSERT_DOUBLE_NOT_EQUAL(actual, expected, granularity)
#define TUNIT_ASSERT_DOUBLE_NOT_EQUAL_FATAL(actual, expected, granularity) \
    CU_ASSERT_DOUBLE_NOT_EQUAL_FATAL(actual, expected, granularity)

extern int32_t tunit_manager_run_tests(void);

#endif /* __TUNIT_MANAGER_H__ */
