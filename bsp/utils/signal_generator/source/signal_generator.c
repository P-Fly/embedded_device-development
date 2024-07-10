/**
 * @file signal_generator.c
 * @brief A simple signal generator.
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
#include <math.h>
#include "err.h"
#include "signal_generator.h"

#define DEF_PI   3.14159265358979f
#define DEF_GAIN 0.977237f // -0.2dB

typedef struct
{
    sg_type_e           type;
    sg_sample_rate_e    sample_rate;

    uint32_t            period_cnt;
    uint32_t            period_samples;
} sg_config_list_t;

const sg_config_list_t sg_config_list[] =
{
    { SG_TYPE_SIN_1K,  SG_SR_16K, 1,  16 },
    { SG_TYPE_SIN_1K,  SG_SR_48K, 1,  48 },
    { SG_TYPE_SIN_10K, SG_SR_16K, 10, 16 },
    { SG_TYPE_SIN_10K, SG_SR_48K, 10, 48 },
};

int32_t signal_generator_get_data_size(sg_type_e        type,
                                       sg_sample_rate_e sample_rate,
                                       sg_bits_e        bits)
{
    uint32_t i;

    for (i = 0; i < sizeof(sg_config_list) / sizeof(sg_config_list[0]); i++)
    {
        if (sg_config_list[i].type == type &&
            sg_config_list[i].sample_rate == sample_rate)
        {
            if (bits == SG_BIT_16)
            {
                return sg_config_list[i].period_samples * sizeof(int16_t);
            }
            else if (bits == SG_BIT_24 || bits == SG_BIT_32)
            {
                return sg_config_list[i].period_samples * sizeof(int32_t);
            }
        }
    }

    return -ENOSUPPORT;
}

int32_t signal_generator_get_data(sg_type_e         type,
                                  sg_sample_rate_e  sample_rate,
                                  sg_bits_e         bits,
                                  void*             signal_buff)
{
    const sg_config_list_t* config = NULL;
    int32_t max_bits = ((0x00000001 << (bits - 1)) - 1);
    int16_t* const signal_buff_int16 = (int16_t*)signal_buff;
    int32_t* const signal_buff_int32 = (int32_t*)signal_buff;
    uint32_t i;

    if (!signal_buff)
    {
        return -EINVAL;
    }

    for (i = 0; i < sizeof(sg_config_list) / sizeof(sg_config_list[0]); i++)
    {
        if (sg_config_list[i].type == type &&
            sg_config_list[i].sample_rate == sample_rate)
        {
            config = &sg_config_list[i];
        }
    }

    if (!config)
    {
        return -EINVAL;
    }

    for (i = 0; i < config->period_samples; i++)
    {
        if (bits == SG_BIT_16)
        {
            signal_buff_int16[i] =
                (int16_t)(sin(i * 2 * DEF_PI * config->period_cnt /
                              config->period_samples) * DEF_GAIN * max_bits);
        }
        else if (bits == SG_BIT_24 || bits == SG_BIT_32)
        {
            signal_buff_int32[i] =
                (int32_t)(sin(i * 2 * DEF_PI * config->period_cnt /
                              config->period_samples) * DEF_GAIN * max_bits);
        }
        else
        {
            return -EINVAL;
        }
    }

    return 0;
}
