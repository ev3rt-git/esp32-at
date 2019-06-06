/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2017 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP32 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "string.h"
#include "esp_log.h"
#include "esp_at.h"

#include "esp_system.h"
#include "at_interface.h"

#include "at_default_config.h"

void at_interface_init (void)
{
    extern int32_t bt_spp_read_data(uint8_t* data, int32_t len);
    extern int32_t bt_spp_write_data(uint8_t* data, int32_t len);
    esp_at_device_ops_struct esp_at_device_ops = {
        .read_data = bt_spp_read_data,
        .write_data = bt_spp_write_data,
        .get_data_length = NULL,
        .wait_write_complete = NULL,
    };

    esp_at_device_ops_regist (&esp_at_device_ops);
}


void at_custom_init(void)
{
    extern void spp_acceptor_init();
    spp_acceptor_init();
}
