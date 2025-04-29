#include "lvgl_ui.h"
#include "audio_player.h"
#include "esp32_s3_szp.h"
#include "string.h"
#include <dirent.h>

#include "esp_wifi.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include "esp_netif_sntp.h"
#include "esp_sntp.h"
#include "github_contributions.h"

lv_obj_t * main_obj; // 主界面

LV_FONT_DECLARE(font_alipuhui12);


uint8_t current_page = 0;

void screen_click_handler(lv_event_t * e)
{
    lv_obj_t * screen = lv_scr_act();
    
    // 清除当前屏幕上的所有对象
    lv_obj_clean(screen);
    
    // 切换到另一个界面
    if (current_page == 0) {
        current_page = 1;
        lv_main_page_2();
    } else {
        current_page = 0;
        lv_main_page();
    }
    //ESP_LOGI("screen_click_handler", "Screen clicked!");
}



/******************************** 主界面  ******************************/

void lv_main_page(void)
{
    lvgl_port_lock(0);
    

    // 创建主界面基本对象
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0); // 修改背景为黑色

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_radius(&style, 10);  
    lv_style_set_bg_opa( &style, LV_OPA_COVER );
    lv_style_set_bg_color(&style, lv_color_hex(0x000000));
    //lv_style_set_bg_grad_color( &style, lv_color_hex( 0x00BF00 ) );
    lv_style_set_bg_grad_dir( &style, LV_GRAD_DIR_VER );
    lv_style_set_border_width(&style, 0);
    lv_style_set_pad_all(&style, 0);
    lv_style_set_width(&style, 320);  
    lv_style_set_height(&style, 240); 
    
    
    main_obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(main_obj, &style, 0);
    lv_obj_add_event_cb(main_obj, screen_click_handler, LV_EVENT_CLICKED, NULL);
  
    
    
    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);    
    lv_label_set_text(label, GITHUB_USERNAME);      // 设置文本
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);  // 居中上对齐，向下偏移20像素
    
    //这里创建了一个柱形统计图，表示近七天的github贡献，柱形宽度30，间隔15
    static lv_style_t square_style;
    lv_style_init(&square_style);
    lv_style_set_bg_color(&square_style, lv_color_hex(0x00FF00)); // 背景绿色
    lv_style_set_border_color(&square_style, lv_color_black());
    lv_style_set_border_width(&square_style, 2);
    
    
    for(int i = 0; i < CONTRIBUTION_DAYS; i++) 
    {
        lv_obj_t * square = lv_obj_create(lv_scr_act());
        lv_obj_set_size(square, 30,  40 * contributions_last_7_days[i]);
        lv_obj_add_style(square, &square_style, 0);
        lv_obj_set_pos(square, i * 45, 220-(40*contributions_last_7_days[i]));  
    }
    
    
    //模拟了坐标轴
    static lv_point_t line_points_x[] = { {10, 220}, {310, 220} };
    static lv_point_t line_points_y[] = { {20, 10}, {20, 230} };
    
    lv_obj_t * line_x = lv_line_create(lv_scr_act());
    lv_line_set_points(line_x, line_points_x, 2);   // 设置线的两个端点
    lv_obj_set_pos(line_x, 0, 0);                 // 线整体偏移
    lv_obj_set_style_line_width(line_x, 2, LV_PART_MAIN);         // 线宽
    lv_obj_set_style_line_color(line_x, lv_color_hex(0xADD8E6), LV_PART_MAIN); 
    
    lv_obj_t * line_y = lv_line_create(lv_scr_act());
    lv_line_set_points(line_y, line_points_y, 2);   // 设置线的两个端点
    lv_obj_set_pos(line_y, 0, 0);                 // 线整体偏移
    lv_obj_set_style_line_width(line_y, 2, LV_PART_MAIN);         // 线宽
    lv_obj_set_style_line_color(line_y, lv_color_hex(0xADD8E6), LV_PART_MAIN); 
    
    lvgl_port_unlock();
}


void lv_main_page_2(void)
{
    lvgl_port_lock(0);
    //github_contributions_query();

    // 创建主界面基本对象
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0); // 修改背景为黑色

    
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_radius(&style, 10);  
    lv_style_set_bg_opa( &style, LV_OPA_COVER );
    lv_style_set_bg_color(&style, lv_color_hex(0x000000));
    //lv_style_set_bg_grad_color( &style, lv_color_hex( 0x00BF00 ) );
    lv_style_set_bg_grad_dir( &style, LV_GRAD_DIR_VER );
    lv_style_set_border_width(&style, 0);
    lv_style_set_pad_all(&style, 0);
    lv_style_set_width(&style, 320);  
    lv_style_set_height(&style, 240); 

    main_obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(main_obj, &style, 0);
    lv_obj_add_event_cb(main_obj, screen_click_handler, LV_EVENT_CLICKED, NULL);



    int i = 0;
    for (int x = 0; x < 5; x++) {
        for (int y = 0; y < 7; y++) {
            if (i >= MAX_MONTHLY_DAYS) break;
    
            lv_obj_t * rect = lv_obj_create(lv_scr_act());
            lv_obj_remove_style_all(rect);
            lv_obj_set_size(rect, 40, 40);
            lv_obj_set_pos(rect, 16 + y * 44, 24 + x * 44);
            lv_obj_set_style_bg_opa(rect, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_set_style_radius(rect, 12, LV_PART_MAIN);  
            lv_obj_set_style_border_width(rect, 0, LV_PART_MAIN);           // 去边框
            lv_obj_set_style_outline_width(rect, 0, LV_PART_MAIN);

            int count = monthly_contributions[i];
            if (count == 0) {
                lv_obj_set_style_bg_color(rect, lv_color_hex(0xC0C0C0), LV_PART_MAIN);//没有提交灰色
            } else if (count < 3) {
                lv_obj_set_style_bg_color(rect, lv_color_hex(0x90EE90), LV_PART_MAIN);//一到两次提交，浅绿
            } else {
                lv_obj_set_style_bg_color(rect, lv_color_hex(0x006400), LV_PART_MAIN);//三次，深绿
            }
            //ESP_LOGI("lv_main_page_2", "i=%d, count=%d", i, count);
            i++;
        }
    }
    
    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);    
    lv_label_set_text(label, GITHUB_USERNAME);      // 设置文本
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);  // 居中上对齐

    lvgl_port_unlock();
}


