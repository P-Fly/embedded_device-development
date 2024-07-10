#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "err.h"
#include "cfifo.h"
#ifdef USE_CMSIS
#include "cmsis_os.h"
#else
#ifndef   __memory_changed
  #define __memory_changed() __sync_synchronize()
#endif
#endif

void cfifo_init(cfifo* fifo, uint8_t* buffer, uint32_t buf_len)
{
    assert(fifo);
    assert(buffer);
    assert(buf_len != 0);

    fifo->size = buf_len;
    fifo->base = buffer;
    fifo->write = 0;
    fifo->read = 0;
}

int32_t cfifo_put(cfifo* fifo, uint8_t* buffer, uint32_t buf_len)
{
    assert(fifo);
    assert(buffer);
    assert(buf_len != 0);

    if (buf_len > cfifo_free_space(fifo))
    {
        return -EFULL;
    }

    __memory_changed();

    uint32_t write = fifo->write % fifo->size;
    uint32_t bytesToTheEnd = fifo->size - write;

    if (bytesToTheEnd >= buf_len)
    {
        (void)memcpy(fifo->base + write, buffer, buf_len);
    }
    else
    {
        (void)memcpy(fifo->base + write, buffer, bytesToTheEnd);
        (void)memcpy(fifo->base, buffer + bytesToTheEnd,
                     buf_len - bytesToTheEnd);
    }

    __memory_changed();

    fifo->write += buf_len;

    return 0;
}

int32_t cfifo_pop(cfifo* fifo, uint8_t* buffer, uint32_t buf_len)
{
    assert(fifo);
    assert(buffer);
    assert(buf_len != 0);

    if (buf_len > cfifo_len(fifo))
    {
        return -EEMPTY;
    }

    __memory_changed();

    uint32_t read = fifo->read % fifo->size;
    uint32_t bytesToTheEnd = fifo->size - read;

    if (bytesToTheEnd >= buf_len)
    {
        (void)memcpy(buffer, fifo->base + read, buf_len);
    }
    else
    {
        (void)memcpy(buffer, fifo->base + read, bytesToTheEnd);
        (void)memcpy(buffer + bytesToTheEnd, fifo->base,
                     buf_len - bytesToTheEnd);
    }

    __memory_changed();

    fifo->read += buf_len;

    return 0;
}

int32_t cfifo_peek(cfifo*       fifo,
                   uint32_t     len_want,
                   uint8_t**    buf1,
                   uint32_t*    len1,
                   uint8_t**    buf2,
                   uint32_t*    len2)
{
    assert(fifo);
    assert(len_want != 0);

    if (len_want > cfifo_len(fifo))
    {
        return -EEMPTY;
    }

    __memory_changed();

    uint32_t read = fifo->read % fifo->size;

    *buf1 = fifo->base + read;
    if (fifo->size - read >= len_want)
    {
        *len1 = len_want;
        *buf2 = NULL;
        *len2 = 0;
    }
    else
    {
        *len1 = fifo->size - read;
        *buf2 = fifo->base;
        *len2 = len_want - *len1;
    }

    return 0;
}

int32_t cfifo_peek_to_buf(cfifo* fifo, uint8_t* buffer, uint32_t buf_len)
{
    assert(fifo);
    assert(buffer);
    assert(buf_len != 0);

    if (buf_len > cfifo_len(fifo))
    {
        return -EEMPTY;
    }

    __memory_changed();

    uint32_t read = fifo->read % fifo->size;
    uint32_t bytesToTheEnd = fifo->size - read;

    if (bytesToTheEnd >= buf_len)
    {
        (void)memcpy(buffer, fifo->base + read, buf_len);
    }
    else
    {
        (void)memcpy(buffer, fifo->base + read, bytesToTheEnd);
        (void)memcpy(buffer + bytesToTheEnd, fifo->base,
                     buf_len - bytesToTheEnd);
    }

    return 0;
}

uint32_t cfifo_len(cfifo* fifo)
{
    assert(fifo);

    return fifo->write - fifo->read;
}

uint32_t cfifo_free_space(cfifo* fifo)
{
    assert(fifo);

    return fifo->size - cfifo_len(fifo);
}
