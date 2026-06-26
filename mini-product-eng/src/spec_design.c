#include "spec_design.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void user_story_init(UserStory *us, const char *role, const char *want, const char *so_that) {
    if (!us) return;
    memset(us, 0, sizeof(*us));
    if (role) {
        strncpy(us->role, role, sizeof(us->role) - 1);
        us->role[sizeof(us->role) - 1] = '\0';
    }
    if (want) {
        strncpy(us->want, want, sizeof(us->want) - 1);
        us->want[sizeof(us->want) - 1] = '\0';
    }
    if (so_that) {
        strncpy(us->so_that, so_that, sizeof(us->so_that) - 1);
        us->so_that[sizeof(us->so_that) - 1] = '\0';
    }
}

void user_story_add_acceptance(UserStory *us, const char *given, const char *when, const char *then) {
    if (!us || us->ac_count >= 10) return;
    char buf[384];
    if (given && when && then) {
        snprintf(buf, sizeof(buf), "Given %s, When %s, Then %s", given, when, then);
    } else if (given) {
        snprintf(buf, sizeof(buf), "Given %s", given);
    } else {
        return;
    }
    strncpy(us->acceptance_criteria[us->ac_count], buf, sizeof(us->acceptance_criteria[0]) - 1);
    us->acceptance_criteria[us->ac_count][sizeof(us->acceptance_criteria[0]) - 1] = '\0';
    us->ac_count++;
}

void user_story_format(const UserStory *us, char *buffer, size_t buf_size) {
    if (!us || !buffer) return;
    int w = snprintf(buffer, buf_size,
        "As a %s,\nI want %s,\nso that %s.\n\nAcceptance Criteria (%d):\n",
        us->role, us->want, us->so_that, us->ac_count);
    for (int i = 0; i < us->ac_count; i++) {
        w += snprintf(buffer + w, buf_size - w, "  %d. %s\n", i + 1, us->acceptance_criteria[i]);
    }
}

void prd_init(PRD *p, const char *title, const char *problem) {
    if (!p) return;
    memset(p, 0, sizeof(*p));
    if (title) {
        strncpy(p->title, title, sizeof(p->title) - 1);
        p->title[sizeof(p->title) - 1] = '\0';
    }
    if (problem) {
        strncpy(p->problem_statement, problem, sizeof(p->problem_statement) - 1);
        p->problem_statement[sizeof(p->problem_statement) - 1] = '\0';
    }
}

void prd_add_story(PRD *p, const UserStory *story) {
    if (!p || !story) return;
    p->story_count++;
    UserStory *tmp = realloc(p->stories, p->story_count * sizeof(UserStory));
    if (!tmp) return;
    p->stories = tmp;
    memcpy(&p->stories[p->story_count - 1], story, sizeof(UserStory));
}

int prd_total_story_points(const PRD *p) {
    if (!p) return 0;
    int total = 0;
    for (size_t i = 0; i < p->story_count; i++) {
        total += p->stories[i].story_points;
    }
    return total;
}

void prd_print_summary(const PRD *p) {
    if (!p) return;
    printf("PRD: %s\n", p->title);
    printf("Problem: %s\n", p->problem_statement);
    if (p->target_user[0]) printf("Target: %s\n", p->target_user);
    if (p->scope[0]) printf("Scope: %s\n", p->scope);
    if (p->out_of_scope[0]) printf("Out of scope: %s\n", p->out_of_scope);
    printf("Stories: %zu (%d story points)\n", p->story_count, prd_total_story_points(p));
    if (p->risks[0]) printf("Risks: %s\n", p->risks);
}

void tech_spec_add_endpoint(TechnicalSpec *ts, const char *endpoint, const char *method, int status) {
    if (!ts || !endpoint || !method) return;
    ts->endpoint_count++;
    ApiEndpoint *tmp = realloc(ts->endpoints, ts->endpoint_count * sizeof(ApiEndpoint));
    if (!tmp) return;
    ts->endpoints = tmp;
    ApiEndpoint *ep = &ts->endpoints[ts->endpoint_count - 1];
    memset(ep, 0, sizeof(*ep));
    strncpy(ep->endpoint, endpoint, sizeof(ep->endpoint) - 1);
    ep->endpoint[sizeof(ep->endpoint) - 1] = '\0';
    strncpy(ep->method, method, sizeof(ep->method) - 1);
    ep->method[sizeof(ep->method) - 1] = '\0';
    ep->status_code = status;
}

void tech_spec_add_model(TechnicalSpec *ts, const char *entity) {
    if (!ts || !entity) return;
    ts->model_count++;
    DataModel *tmp = realloc(ts->models, ts->model_count * sizeof(DataModel));
    if (!tmp) return;
    ts->models = tmp;
    DataModel *dm = &ts->models[ts->model_count - 1];
    memset(dm, 0, sizeof(*dm));
    strncpy(dm->entity_name, entity, sizeof(dm->entity_name) - 1);
    dm->entity_name[sizeof(dm->entity_name) - 1] = '\0';
}

void tech_spec_add_mockup(TechnicalSpec *ts, const char *screen, const char *layout) {
    if (!ts || !screen || !layout) return;
    ts->mockup_count++;
    UiMockup *tmp = realloc(ts->mockups, ts->mockup_count * sizeof(UiMockup));
    if (!tmp) return;
    ts->mockups = tmp;
    UiMockup *um = &ts->mockups[ts->mockup_count - 1];
    memset(um, 0, sizeof(*um));
    strncpy(um->screen_name, screen, sizeof(um->screen_name) - 1);
    um->screen_name[sizeof(um->screen_name) - 1] = '\0';
    strncpy(um->layout_description, layout, sizeof(um->layout_description) - 1);
    um->layout_description[sizeof(um->layout_description) - 1] = '\0';
}

void tech_spec_print_summary(const TechnicalSpec *ts) {
    if (!ts) return;
    printf("Technical Spec Summary:\n");
    printf("  Endpoints: %zu\n", ts->endpoint_count);
    for (size_t i = 0; i < ts->endpoint_count; i++) {
        printf("    %s %s -> %d\n", ts->endpoints[i].method, ts->endpoints[i].endpoint, ts->endpoints[i].status_code);
    }
    printf("  Data Models: %zu\n", ts->model_count);
    for (size_t i = 0; i < ts->model_count; i++) {
        printf("    %s\n", ts->models[i].entity_name);
    }
    printf("  UI Mockups: %zu\n", ts->mockup_count);
    for (size_t i = 0; i < ts->mockup_count; i++) {
        printf("    %s: %s\n", ts->mockups[i].screen_name, ts->mockups[i].layout_description);
    }
}

void design_review_init(DesignReview *dr) {
    if (!dr) return;
    memset(dr, 0, sizeof(*dr));
}

bool design_review_all_passed(const DesignReview *dr) {
    if (!dr) return false;
    return dr->requirements_clarity &&
           dr->edge_cases_covered &&
           dr->error_handling_documented &&
           dr->security_review &&
           dr->accessibility_review &&
           dr->i18n_considerations &&
           dr->performance_constraints &&
           dr->dependency_analysis &&
           dr->backward_compatibility &&
           dr->monitoring_plan;
}

double design_review_score(const DesignReview *dr) {
    if (!dr) return 0.0;
    int total = 10;
    int passed = 0;
    if (dr->requirements_clarity) passed++;
    if (dr->edge_cases_covered) passed++;
    if (dr->error_handling_documented) passed++;
    if (dr->security_review) passed++;
    if (dr->accessibility_review) passed++;
    if (dr->i18n_considerations) passed++;
    if (dr->performance_constraints) passed++;
    if (dr->dependency_analysis) passed++;
    if (dr->backward_compatibility) passed++;
    if (dr->monitoring_plan) passed++;
    return (double)passed / (double)total;
}

void design_review_list_pending(const DesignReview *dr, char *buffer, size_t buf_size) {
    if (!dr || !buffer) return;
    const char *checks[] = {
        "Requirements clarity", "Edge cases covered",
        "Error handling documented", "Security review",
        "Accessibility review", "i18n considerations",
        "Performance constraints", "Dependency analysis",
        "Backward compatibility", "Monitoring plan"
    };
    const bool *results[] = {
        &dr->requirements_clarity, &dr->edge_cases_covered,
        &dr->error_handling_documented, &dr->security_review,
        &dr->accessibility_review, &dr->i18n_considerations,
        &dr->performance_constraints, &dr->dependency_analysis,
        &dr->backward_compatibility, &dr->monitoring_plan
    };
    int w = 0;
    for (int i = 0; i < 10; i++) {
        if (!*results[i]) {
            w += snprintf(buffer + w, buf_size - w, "[ ] %s\n", checks[i]);
        }
    }
}

void spec_free_all(PRD *prd, TechnicalSpec *ts) {
    if (prd) {
        free(prd->stories);
        memset(prd, 0, sizeof(*prd));
    }
    if (ts) {
        free(ts->endpoints);
        free(ts->models);
        free(ts->mockups);
        memset(ts, 0, sizeof(*ts));
    }
}
