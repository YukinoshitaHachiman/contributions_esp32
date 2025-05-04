#ifndef GITHUB_CONTRIBUTIONS_H
#define GITHUB_CONTRIBUTIONS_H


#define GITHUB_USERNAME "你自己的用户名"
#define GITHUB_TOKEN "你自己的token"
//#define GITHUB_TOKEN ""
#define CONTRIBUTION_DAYS 7
#define MAX_MONTHLY_DAYS 31  
extern int monthly_contributions[MAX_MONTHLY_DAYS];
extern char monthly_dates[MAX_MONTHLY_DAYS][11];  // YYYY-MM-DD格式


extern int contributions_last_7_days[CONTRIBUTION_DAYS];

extern char contribution_dates[CONTRIBUTION_DAYS][11];
void test_graphql_query(void);
;
// --- API函数 ---
void github_contributions_query(void);

#endif // GITHUB_CONTRIBUTIONS_H

