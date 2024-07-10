/**
 * @file cfifo.h
 * @brief A circle fifo.
 * @author Peter.Peng <27144363@qq.com>
 * @date 2022
 *
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

#ifndef __CFIFO_H__
#define __CFIFO_H__

/**
 * @defgroup cfifo Circle FIFO
 *
 * @brief A circle fifo implementation.
 *
 * @ingroup utils_group
 *
 * @{
 *
 */

#include <stddef.h>
#include <stdint.h>

#ifndef DOC_HIDDEN
typedef struct
{
    uint8_t*    base;
    uint32_t    size;
    uint32_t    write;
    uint32_t    read;
} cfifo;
#endif

/**
 * @brief   Initialize the cfifo.
 *
 * @param   fifo Pointer to the fifo handle.
 * @param   buffer The buffer space.
 * @param   buf_len The buffer size.
 */
extern void cfifo_init(cfifo* fifo, uint8_t* buffer, uint32_t buf_len);

/**
 * @brief   Put the data to the fifo.
 *
 * @param   fifo Pointer to the fifo handle.
 * @param   buffer The buffer space.
 * @param   buf_len The buffer size.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
extern int32_t cfifo_put(cfifo* fifo, uint8_t* buffer, uint32_t buf_len);

/**
 * @brief   Pop the data from the fifo.
 *
 * @param   fifo Pointer to the fifo handle.
 * @param   buffer The buffer space.
 * @param   buf_len The buffer size.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
extern int32_t cfifo_pop(cfifo* fifo, uint8_t* buffer, uint32_t buf_len);

/**
 * @brief   Peek the data from the fifo.
 *
 * @param   fifo Pointer to the fifo handle.
 * @param   len_want Want to get the data length.
 * @param   buf1 Pointer to the buffer 1.
 * @param   len1 The buffer 1 size.
 * @param   buf2 Pointer to the buffer 2.
 * @param   len2 The buffer 2 size.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
extern int32_t cfifo_peek(cfifo*    fifo,
                          uint32_t  len_want,
                          uint8_t** buf1,
                          uint32_t* len1,
                          uint8_t** buf2,
                          uint32_t* len2);

/**
 * @brief   Peek the data from the fifo.
 *
 * @param   fifo Pointer to the fifo handle.
 * @param   buffer The buffer space.
 * @param   buf_len The buffer size.
 *
 * @retval  Returns 0 on success, negative error code otherwise.
 */
extern int32_t cfifo_peek_to_buf(cfifo* fifo, uint8_t* buffer,
                                 uint32_t buf_len);

/**
 * @brief   Get the length from the fifo.
 *
 * @param   fifo Pointer to the fifo handle.
 *
 * @retval  Returns the fifo length.
 */
extern uint32_t cfifo_len(cfifo* fifo);

/**
 * @brief   Get the free space from the fifo.
 *
 * @param   fifo Pointer to the fifo handle.
 *
 * @retval  Returns the free space.
 */
extern uint32_t cfifo_free_space(cfifo* fifo);

/**
 * @}
 */

#endif /* __CFIFO_H__ */
