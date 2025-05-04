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
#include <esp_system.h>
#include "wifi_station.h"
#include "nvs_flash.h"
#include "github_contributions.h"
#include "esp32_s3_szp.h"
#include "lvgl_ui.h"
#include "time.h"
#include "esp_sntp.h"



static void main_page_task(void *pvParameters)
{

    lv_main_page();
    vTaskDelete(NULL);

}

static void initialize_sntp(void)
{
    // 设置时区
    setenv("TZ", "CST-8", 1);
    tzset();

    // 初始化 SNTP
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    bsp_i2c_init();  // I2C初始化
    pca9557_init();  // IO扩展芯片初始化
    bsp_lvgl_start(); // 初始化液晶屏lvgl接口


 


    ESP_LOGI("main", "ESP_WIFI_MODE_STA");
    wifi_init_sta();



    vTaskDelay(pdMS_TO_TICKS(1000));

   
    initialize_sntp();

    
    time_t now = 0;
    struct tm timeinfo = { 0 };
    while (now < 1525132800) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        time(&now);
    }
    localtime_r(&now, &timeinfo);
    ESP_LOGI("main", "Current date: %04d-%02d-%02d",
             timeinfo.tm_year + 1900,
             timeinfo.tm_mon + 1,
             timeinfo.tm_mday);

    github_contributions_query();

    

    printf("最近7天贡献数：\n");
    for (int i = 0; i < CONTRIBUTION_DAYS; i++) {
        printf("%s: %d commits\n", contribution_dates[i], contributions_last_7_days[i]);
    }
    
    // printf("\n%d月份的GitHub贡献情况：\n", timeinfo.tm_mon + 1);
    // for (int i = 0; i < MAX_MONTHLY_DAYS; i++) {
    //     printf("%2d日: %d commits\n", i + 1, monthly_contributions[i]);
    // }

    xTaskCreatePinnedToCore(main_page_task, "main_page_task", 4*1024, NULL, 5, NULL, 0); // 主界面任务


    
}
