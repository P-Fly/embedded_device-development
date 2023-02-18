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

#ifndef __ADC_MANAGER_H__
#define __ADC_MANAGER_H__

typedef enum
{
    ADC_ID_1 = 0,
    ADC_ID_2,
    ADC_ID_3,

    ADC_ID_BUTT,
} adc_id_e;

typedef void (* adc_user_clbk_t)(adc_id_e id, uint16_t data,
                                 const void* user_ctx);

extern int32_t adc_manager_register_user_clbk(adc_id_e          id,
                                              adc_user_clbk_t   user_clbk,
                                              const void*       user_ctx);
extern void adc_manager_unregister_user_clbk(adc_id_e id);
extern void adc_manager_driver_conv_cplt_clbk(adc_id_e id, uint16_t data);
extern void adc_manager_driver_error_clbk(void);

#endif /* __ADC_MANAGER_H__ */
