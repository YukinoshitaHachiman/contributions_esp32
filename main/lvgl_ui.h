#pragma once
#include <stdint.h>
#include "lvgl.h"
#include "esp_log.h"
/*********************** 开机界面 ****************************/



/*********************** 主界面 ****************************/
extern uint8_t current_page;
void lv_main_page(void);
void lv_main_page_2(void);
void screen_click_handler(lv_event_t * e);
