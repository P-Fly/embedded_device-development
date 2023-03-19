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

#ifndef __ECG_GATT_SERVICE_H__
#define __ECG_GATT_SERVICE_H__

#include "wpan_conf.h"

int32_t ecg_gatt_service_init(void);
int32_t ecg_gatt_service_update_measurment_value(uint8_t* value, uint8_t len);

#endif /* __ECG_GATT_SERVICE_H__ */
