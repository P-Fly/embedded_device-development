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

#ifndef __FRAMEWORK_CONF__
#define __FRAMEWORK_CONF__

#define CONFIG_INIT_THREAD_NAME "init"
#define CONFIG_INIT_THREAD_STACK_SIZE 2048
#define CONFIG_INIT_THREAD_PRIORITY osPriorityRealtime

#define CONFIG_DBG_TRACE_NAME "dbg trace module"
#define CONFIG_DBG_TRACE_LABEL dbg_trace_module
#define CONFIG_DBG_TRACE_MAX_LEN 256

#endif /* __FRAMEWORK_CONF__ */
