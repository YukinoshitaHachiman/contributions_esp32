#ifndef GITHUB_CONTRIBUTIONS_H
#define GITHUB_CONTRIBUTIONS_H

// --- 用户需要配置自己的信息 ---
#define GITHUB_USERNAME "YukinoshitaHachiman"
#define GITHUB_TOKEN "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"
//#define GITHUB_TOKEN ""
#define CONTRIBUTION_DAYS 7
#define MAX_MONTHLY_DAYS 31  // 一个月最多31天
extern int monthly_contributions[MAX_MONTHLY_DAYS];
extern char monthly_dates[MAX_MONTHLY_DAYS][11];  // YYYY-MM-DD格式


extern int contributions_last_7_days[CONTRIBUTION_DAYS];

extern char contribution_dates[CONTRIBUTION_DAYS][11];
void test_graphql_query(void);
;
// --- API函数 ---
void github_contributions_query(void);

#endif // GITHUB_CONTRIBUTIONS_H

