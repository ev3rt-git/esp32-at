/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_at.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"

#include "time.h"
#include "sys/time.h"


#if !defined(CONFIG_BT_ENABLED) || \
    !defined(CONFIG_BTDM_CONTROLLER_MODE_BR_EDR_ONLY) || \
    !defined(CONFIG_AT_BT_COMMAND_SUPPORT) || \
    defined(CONFIG_AT_BLE_COMMAND_SUPPORT)
#error "unsupported config"
#endif

#define SPP_TAG "SPP_ACCEPTOR"

static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;

static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;

static uint32_t connection_handle = 0; // TODO: protect this?
static uint8_t *data_buf = NULL;
static uint32_t data_len = 0;
static xSemaphoreHandle data_received_semaphore;

int32_t bt_spp_read_data(uint8_t* buf, int32_t length) {
    ESP_LOGI(SPP_TAG, "%s, len=%d, data_len=%d",
        __FUNCTION__, length, data_len);

    /*
     * data_len WILL be modified after giving the semaphore,
     * copy it to local.
     */
    assert(length >= data_len);
    if ((length = data_len) == 0) {
        return 0;
    }

    assert(buf != NULL);
    memcpy(buf, data_buf, length);

    int res;
    res = xSemaphoreGive(data_received_semaphore);
    assert(res == pdTRUE);

    return length;
}

int32_t bt_spp_write_data(uint8_t*data,int32_t len)
{
    ESP_LOGI(SPP_TAG, "%s, len %d" , __FUNCTION__, len);

    if (connection_handle == 0) {
        return 0;
    }

    esp_err_t ret = esp_spp_write(connection_handle, len, data);
    if (ret != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s spp write failed: %s\n", __func__, esp_err_to_name(ret));
        return 0;
    }

    return len;
}

static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    switch (event) {
    case ESP_SPP_INIT_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_INIT_EVT");
        esp_bt_dev_set_device_name(CONFIG_AT_BT_SPP_DEVICE_NAME);
        esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
        esp_spp_start_srv(sec_mask,role_slave, 0, CONFIG_AT_BT_SPP_SERVICE_NAME);
        break;
    case ESP_SPP_DISCOVERY_COMP_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_DISCOVERY_COMP_EVT");
        break;
    case ESP_SPP_OPEN_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_OPEN_EVT");
        break;
    case ESP_SPP_CLOSE_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_CLOSE_EVT");
        connection_handle = 0;
        esp_restart();
        break;
    case ESP_SPP_START_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_START_EVT");
        break;
    case ESP_SPP_CL_INIT_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_CL_INIT_EVT");
        break;
    case ESP_SPP_DATA_IND_EVT: {
        data_buf = param->data_ind.data;
        data_len = param->data_ind.len;
        ESP_LOGI(SPP_TAG, "ESP_SPP_DATA_IND_EVT len=%d handle=%d",
                 param->data_ind.len, param->data_ind.handle);
        int ret = esp_at_port_recv_data_notify(param->data_ind.len, portMAX_DELAY);
        assert(ret);
        ret = xSemaphoreTake(data_received_semaphore, portMAX_DELAY);
        assert(ret == pdTRUE);
        data_len = 0;
        }
        break;
    case ESP_SPP_CONG_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_CONG_EVT");
        break;
    case ESP_SPP_WRITE_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_WRITE_EVT");
        break;
    case ESP_SPP_SRV_OPEN_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_SRV_OPEN_EVT");
        connection_handle = param->srv_open.handle;
        break;
    default:
        break;
    }
}

void spp_acceptor_init()
{
    data_received_semaphore = xSemaphoreCreateBinary();
    assert(data_received_semaphore != NULL);

    esp_err_t ret;

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_spp_register_callback(esp_spp_cb)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s spp register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_spp_init(esp_spp_mode)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s spp init failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    /* Set default parameters for Secure Simple Pairing */
    esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
    esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));

    /*
     * Set default parameters for Legacy Pairing
     * Use fixed PIN code
     */
    uint8_t *pin_code = (uint8_t*)CONFIG_AT_BT_SPP_PIN_CODE;
    uint8_t pin_code_len = strlen(CONFIG_AT_BT_SPP_PIN_CODE);
    if ((ret = esp_bt_gap_set_pin(ESP_BT_PIN_TYPE_FIXED, pin_code_len, pin_code)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s gap set pin failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
}

