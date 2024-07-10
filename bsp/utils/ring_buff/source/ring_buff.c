/**
 * @file ring_buff.c
 * @brief A ring buffer.
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

#include <assert.h>
#include "err.h"
#include "ring_buff.h"

#define is_empty(ring) ((ring)->cnt == 0)
#define is_full(ring)  ((ring)->cnt == (ring)->size)

int32_t ring_buffer_read(ring_buff_t* ring, uint8_t* value)
{
    if (!ring)
    {
        return -EINVAL;
    }

    if (!value)
    {
        return -EINVAL;
    }

    assert(ring->buffer);
    assert(ring->size);

    if (is_empty(ring))
    {
        return -EEMPTY;
    }

    *value = ring->buffer[ring->front];
    ring->front = (ring->front + 1) % ring->size;
    ring->cnt--;

    return 0;
}

int32_t ring_buffer_write(ring_buff_t* ring, uint8_t value)
{
    if (!ring)
    {
        return -EINVAL;
    }

    assert(ring->buffer);
    assert(ring->size);

    if (is_full(ring))
    {
        return -EFULL;
    }

    ring->buffer[ring->rear] = value;
    ring->rear = (ring->rear + 1) % ring->size;
    ring->cnt++;

    return 0;
}

int32_t ring_buffer_init(ring_buff_t* ring, uint8_t* buffer, int32_t size)
{
    if (!ring)
    {
        return -EINVAL;
    }

    if (!buffer)
    {
        return -EINVAL;
    }

    if (!size)
    {
        return -EINVAL;
    }

    ring->front = 0;
    ring->rear = 0;
    ring->buffer = buffer;
    ring->size = size;
    ring->cnt = 0;

    return 0;
}
