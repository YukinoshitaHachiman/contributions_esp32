#include <string.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"
#include "github_contributions.h"
#include <time.h>
#define TAG "GITHUB"
#define GITHUB_API_URL "https://api.github.com/graphql"
#define MAX_RESPONSE_SIZE 32768
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define DATE_STR_LEN 11 // "YYYY-MM-DD" + '\0'


static char response_buffer[MAX_RESPONSE_SIZE];
static int response_len = 0;

int contributions_last_7_days[CONTRIBUTION_DAYS] = {0}; // 最近7天提交数
char contribution_dates[CONTRIBUTION_DAYS][DATE_STR_LEN] = {0};

int monthly_contributions[MAX_MONTHLY_DAYS] = {0};
char monthly_dates[MAX_MONTHLY_DAYS][11] = {0};
static int current_month_count = 0;  // 当月数据计数

static const char *graphql_template =
    "{"
    "\"query\": \"query UserContributions {"
    "  user(login: \\\"%s\\\") {"
    "    contributionsCollection {"
    "      contributionCalendar {"
    "        totalContributions"
    "        weeks {"
    "          contributionDays {"
    "            contributionCount"
    "            date"
    "          }"
    "        }"
    "      }"
    "    }"
    "  }"
    "}\""
    "}";


static esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    if (evt->event_id == HTTP_EVENT_ON_DATA && !esp_http_client_is_chunked_response(evt->client)) {
        int copy_len = MIN(evt->data_len, MAX_RESPONSE_SIZE - response_len - 1);
        memcpy(response_buffer + response_len, evt->data, copy_len);
        response_len += copy_len;
        response_buffer[response_len] = '\0';
    }
    return ESP_OK;
}





static void parse_contributions(const char *json) {
    //ESP_LOGI(TAG, "Received JSON: %s", json);
    cJSON *root = cJSON_Parse(json);
    if (!root) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return;
    }

    cJSON *errors = cJSON_GetObjectItem(root, "errors");
    if (errors) {
        int error_count = cJSON_GetArraySize(errors);
        for (int i = 0; i < error_count; i++) {
            cJSON *error = cJSON_GetArrayItem(errors, i);
            cJSON *message = cJSON_GetObjectItem(error, "message");
            if (message && message->valuestring) {
                ESP_LOGE(TAG, "GraphQL Error: %s", message->valuestring);
            }
        }
        cJSON_Delete(root);
        return;
    }

    cJSON *data = cJSON_GetObjectItem(root, "data");
    cJSON *user = cJSON_GetObjectItem(data, "user");
    cJSON *contributionsCollection = cJSON_GetObjectItem(user, "contributionsCollection");
    cJSON *contributionCalendar = cJSON_GetObjectItem(contributionsCollection, "contributionCalendar");
    cJSON *weeks = cJSON_GetObjectItem(contributionCalendar, "weeks");

    if (!weeks) {
        ESP_LOGE(TAG, "No weeks data found");
        cJSON_Delete(root);
        return;
    }

    int total_days = 0;
    int week_count = cJSON_GetArraySize(weeks);

    for (int w = week_count - 1; w >= 0 && total_days < CONTRIBUTION_DAYS; w--) {
        cJSON *week = cJSON_GetArrayItem(weeks, w);
        cJSON *days = cJSON_GetObjectItem(week, "contributionDays");

        int day_count = cJSON_GetArraySize(days);
        for (int d = day_count - 1; d >= 0 && total_days < CONTRIBUTION_DAYS; d--) {
            cJSON *day = cJSON_GetArrayItem(days, d);
            cJSON *count = cJSON_GetObjectItem(day, "contributionCount");
            cJSON *date = cJSON_GetObjectItem(day, "date");

            if (count && date) {
                contributions_last_7_days[CONTRIBUTION_DAYS - 1 - total_days] = count->valueint;
                snprintf(contribution_dates[CONTRIBUTION_DAYS - 1 - total_days], 11, "%s", date->valuestring);
                total_days++;
            }
        }
    }

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    int current_month = timeinfo.tm_mon + 1;
    
    current_month_count = 0;  // 重置计数器
    
    for (int w = 0; w < week_count; w++) {
        cJSON *week = cJSON_GetArrayItem(weeks, w);
        cJSON *days = cJSON_GetObjectItem(week, "contributionDays");
        
        int day_count = cJSON_GetArraySize(days);
        for (int d = 0; d < day_count; d++) {
            cJSON *day = cJSON_GetArrayItem(days, d);
            cJSON *count = cJSON_GetObjectItem(day, "contributionCount");
            cJSON *date = cJSON_GetObjectItem(day, "date");
            
            if (count && date) {
                // int year, month, day;
                // sscanf(date->valuestring, "%d-%d-%d", &year, &month, &day);
                
                // // 只保存当月的数据
                // if (month == current_month) {
                //     monthly_contributions[day-1] = count->valueint;  // 数组索引从0开始
                //     strncpy(monthly_dates[day-1], date->valuestring, 11);
                //     current_month_count++;
                // }

                int year, month, mday;
                sscanf(date->valuestring, "%d-%d-%d", &year, &month, &mday);
                
                if (year == timeinfo.tm_year + 1900 && month == current_month) {
                    monthly_contributions[mday - 1] = count->valueint;
                    strncpy(monthly_dates[mday - 1], date->valuestring, DATE_STR_LEN);
                    current_month_count++;
                }
                    
            }
        }
    }

    cJSON_Delete(root);
}



void github_contributions_query(void) {



    char graphql_query[512];
    snprintf(graphql_query, sizeof(graphql_query), graphql_template, GITHUB_USERNAME);

    esp_http_client_config_t config = {
        .url = GITHUB_API_URL,
        .method = HTTP_METHOD_POST,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .cert_pem = NULL,  // 简化：禁用证书验证
        .skip_cert_common_name_check = true,
        .event_handler = _http_event_handler,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "HTTP客户端初始化失败");
        return;
    }

    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Bearer %s", GITHUB_TOKEN);
    esp_http_client_set_header(client, "Authorization", auth_header);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_header(client, "User-Agent", "ESP32-GraphQL-Client");

    esp_http_client_set_post_field(client, graphql_query, strlen(graphql_query));
    response_len = 0;  // 每次请求前清空

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP状态码: %d", esp_http_client_get_status_code(client));
        parse_contributions(response_buffer);
    } else {
        ESP_LOGE(TAG, "请求失败: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}
