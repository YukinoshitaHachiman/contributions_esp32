#ifndef GITHUB_CONTRIBUTIONS_H
#define GITHUB_CONTRIBUTIONS_H

// --- 用户需要配置自己的信息 ---
#define GITHUB_USERNAME "YukinoshitaHachiman"
#define GITHUB_TOKEN "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"
//#define GITHUB_TOKEN ""
#define CONTRIBUTION_DAYS 7

extern int contributions_last_7_days[CONTRIBUTION_DAYS];

void test_graphql_query(void);

// --- API函数 ---
void github_contributions_query(void);

#endif // GITHUB_CONTRIBUTIONS_H

