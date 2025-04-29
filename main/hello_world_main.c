/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "wifi_station.h"
#include "nvs_flash.h"
#include "github_contributions.h"

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);



    ESP_LOGI("main", "ESP_WIFI_MODE_STA");
    wifi_init_sta();



    vTaskDelay(pdMS_TO_TICKS(5000));

    github_contributions_query();

    

    printf("最近7天贡献数：\n");
    for (int i = 0; i < CONTRIBUTION_DAYS; i++) {
        printf("Day %d: %d commits\n", i + 1, contributions_last_7_days[i]);
    }

}
