#ifndef SPEC_DESIGN_H
#define SPEC_DESIGN_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    char given[256];
    char when[256];
    char then[256];
} AcceptanceCriteria;

typedef struct {
    char role[128];
    char want[256];
    char so_that[256];
    int priority;
    int story_points;
    char acceptance_criteria[10][256];
    int ac_count;
} UserStory;

typedef struct {
    char title[256];
    char problem_statement[512];
    char target_user[128];
    char success_metrics[512];
    UserStory *stories;
    size_t story_count;
    char scope[256];
    char out_of_scope[256];
    char assumptions[512];
    char risks[512];
} PRD;

typedef struct {
    char endpoint[256];
    char method[16];
    char request_body[512];
    char response_body[512];
    int status_code;
} ApiEndpoint;

typedef struct {
    char entity_name[128];
    char fields[20][128];
    int field_count;
    char relationships[256];
} DataModel;

typedef struct {
    char screen_name[128];
    char layout_description[512];
    char interaction_flow[512];
} UiMockup;

typedef struct {
    ApiEndpoint *endpoints;
    size_t endpoint_count;
    DataModel *models;
    size_t model_count;
    UiMockup *mockups;
    size_t mockup_count;
} TechnicalSpec;

typedef struct {
    bool requirements_clarity;
    bool edge_cases_covered;
    bool error_handling_documented;
    bool security_review;
    bool accessibility_review;
    bool i18n_considerations;
    bool performance_constraints;
    bool dependency_analysis;
    bool backward_compatibility;
    bool monitoring_plan;
    char notes[1024];
} DesignReview;

void   user_story_init(UserStory *us, const char *role, const char *want, const char *so_that);
void   user_story_add_acceptance(UserStory *us, const char *given, const char *when, const char *then);
void   user_story_format(const UserStory *us, char *buffer, size_t buf_size);
void   prd_init(PRD *p, const char *title, const char *problem);
void   prd_add_story(PRD *p, const UserStory *story);
int    prd_total_story_points(const PRD *p);
void   prd_print_summary(const PRD *p);
void   tech_spec_add_endpoint(TechnicalSpec *ts, const char *endpoint, const char *method, int status);
void   tech_spec_add_model(TechnicalSpec *ts, const char *entity);
void   tech_spec_add_mockup(TechnicalSpec *ts, const char *screen, const char *layout);
void   tech_spec_print_summary(const TechnicalSpec *ts);
void   design_review_init(DesignReview *dr);
bool   design_review_all_passed(const DesignReview *dr);
double design_review_score(const DesignReview *dr);
void   design_review_list_pending(const DesignReview *dr, char *buffer, size_t buf_size);
void   spec_free_all(PRD *prd, TechnicalSpec *ts);

#endif
