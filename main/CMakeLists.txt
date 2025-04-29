idf_component_register(SRCS "esp32_s3_szp.c" "lvgl_ui.c" "github_contributions.c" "wifi_station.c" "hello_world_main.c"
                       
                    INCLUDE_DIRS ".")
                    target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-const-variable)