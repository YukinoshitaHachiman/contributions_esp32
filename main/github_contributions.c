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

static char response_buffer[MAX_RESPONSE_SIZE];
static int response_len = 0;

int contributions_last_7_days[CONTRIBUTION_DAYS] = {0}; // 最近7天提交数

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

// static void parse_contributions(const char *json) {
//     cJSON *root = cJSON_Parse(json);
//     if (!root) {
//         ESP_LOGE(TAG, "JSON解析失败");
//         return;
//     }

//     cJSON *errors = cJSON_GetObjectItem(root, "errors");
//     if (errors) {
//         cJSON *message = cJSON_GetObjectItem(cJSON_GetArrayItem(errors, 0), "message");
//         if (message && message->valuestring) {
//             ESP_LOGE(TAG, "GraphQL错误: %s", message->valuestring);
//         }
//         cJSON_Delete(root);
//         return;
//     }

//     cJSON *weeks = cJSON_GetObjectItemCaseSensitive(
//         cJSON_GetObjectItemCaseSensitive(
//             cJSON_GetObjectItemCaseSensitive(
//                 cJSON_GetObjectItemCaseSensitive(
//                     cJSON_GetObjectItemCaseSensitive(root, "data"), "user"), "contributionsCollection"), "contributionCalendar"), "weeks");

//     if (!weeks) {
//         ESP_LOGE(TAG, "没有weeks数据");
//         cJSON_Delete(root);
//         return;
//     }

//     int total_days = 0;
//     for (int w = cJSON_GetArraySize(weeks) - 1; w >= 0 && total_days < CONTRIBUTION_DAYS; w--) {
//         cJSON *week = cJSON_GetArrayItem(weeks, w);
//         cJSON *days = cJSON_GetObjectItem(week, "contributionDays");
//         for (int d = cJSON_GetArraySize(days) - 1; d >= 0 && total_days < CONTRIBUTION_DAYS; d--) {
//             cJSON *day = cJSON_GetArrayItem(days, d);
//             cJSON *count = cJSON_GetObjectItem(day, "contributionCount");
//             if (count) {
//                 contributions_last_7_days[CONTRIBUTION_DAYS - 1 - total_days] = count->valueint;
//                 total_days++;
//             }
//         }
//     }
//     cJSON_Delete(root);
// }


static void parse_contributions(const char *json) {
    ESP_LOGI(TAG, "Received JSON: %s", json); // 可注释掉，太长了的话
    cJSON *root = cJSON_Parse(json);
    if (!root) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return;
    }
    
    memset(contributions_last_7_days, 0, sizeof(contributions_last_7_days)); // 清零

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

    // 获取当前时间
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    int today_weekday = tm_now->tm_wday; // 0=Sunday, 1=Monday, ..., 6=Saturday

    // 计算本周周一的时间戳（0点）
    time_t monday = now - ((today_weekday == 0 ? 6 : today_weekday - 1) * 24 * 3600);
    struct tm tm_monday = *localtime(&monday);
    tm_monday.tm_hour = 0;
    tm_monday.tm_min = 0;
    tm_monday.tm_sec = 0;
    monday = mktime(&tm_monday);

    int week_count = cJSON_GetArraySize(weeks);
    for (int w = 0; w < week_count; w++) {
        cJSON *week = cJSON_GetArrayItem(weeks, w);
        cJSON *days = cJSON_GetObjectItem(week, "contributionDays");
        int day_count = cJSON_GetArraySize(days);

        for (int d = 0; d < day_count; d++) {
            cJSON *day = cJSON_GetArrayItem(days, d);
            cJSON *count = cJSON_GetObjectItem(day, "contributionCount");
            cJSON *date = cJSON_GetObjectItem(day, "date");

            if (count && date && date->valuestring) {
                // 解析日期字符串
                struct tm tm_day = {0};
                strptime(date->valuestring, "%Y-%m-%d", &tm_day);
                time_t day_time = mktime(&tm_day);

                // 如果在本周范围内
                double diff_days = difftime(day_time, monday) / (24 * 3600);
                if (diff_days >= 0 && diff_days < 7) {
                    contributions_last_7_days[(int)diff_days] = count->valueint;
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
