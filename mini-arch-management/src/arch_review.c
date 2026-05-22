#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "arch_review.h"

void arch_review_init(ArchReview *review, unsigned int id,
                       const char *title, const char *description)
{
    if (!review) return;
    memset(review, 0, sizeof(*review));
    review->id = id;
    if (title) {
        strncpy(review->title, title, ARCH_REVIEW_MAX_NAME_LENGTH - 1);
        review->title[ARCH_REVIEW_MAX_NAME_LENGTH - 1] = '\0';
    }
    if (description) {
        strncpy(review->description, description,
                ARCH_REVIEW_MAX_DESC_LENGTH - 1);
        review->description[ARCH_REVIEW_MAX_DESC_LENGTH - 1] = '\0';
    }
    review->status = ARCH_REVIEW_STATUS_SCHEDULED;
    review->stakeholder_count = 0;
    review->checklist_count = 0;
    review->finding_count = 0;
    review->meeting_count = 0;
    review->linked_adr_count = 0;
    review->action_item_count = 0;
    review->scheduled_date = time(NULL);
    review->completed_date = 0;
}

ArchReview *arch_review_create(unsigned int id, const char *title,
                                const char *description)
{
    ArchReview *review = (ArchReview *)malloc(sizeof(ArchReview));
    if (!review) return NULL;
    arch_review_init(review, id, title, description);
    return review;
}

void arch_review_destroy(ArchReview *review)
{
    if (review) free(review);
}

bool arch_review_add_stakeholder(ArchReview *review,
                                  const char *name, const char *role,
                                  const char *concerns,
                                  bool is_decision_maker, bool has_veto_power)
{
    if (!review || !name) return false;
    if (review->stakeholder_count >= ARCH_REVIEW_MAX_STAKEHOLDERS) return false;
    ArchStakeholder *s = &review->stakeholders[review->stakeholder_count];
    strncpy(s->name, name, ARCH_REVIEW_MAX_NAME_LENGTH - 1);
    s->name[ARCH_REVIEW_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(s->role, role ? role : "", ARCH_REVIEW_MAX_NAME_LENGTH - 1);
    s->role[ARCH_REVIEW_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(s->concerns, concerns ? concerns : "",
            ARCH_REVIEW_MAX_DESC_LENGTH - 1);
    s->concerns[ARCH_REVIEW_MAX_DESC_LENGTH - 1] = '\0';
    s->is_decision_maker = is_decision_maker;
    s->has_veto_power = has_veto_power;
    review->stakeholder_count++;
    return true;
}

bool arch_review_remove_stakeholder(ArchReview *review, const char *name)
{
    if (!review || !name) return false;
    for (size_t i = 0; i < review->stakeholder_count; i++) {
        if (strcmp(review->stakeholders[i].name, name) == 0) {
            for (size_t j = i; j < review->stakeholder_count - 1; j++) {
                review->stakeholders[j] = review->stakeholders[j + 1];
            }
            review->stakeholder_count--;
            return true;
        }
    }
    return false;
}

bool arch_review_add_checklist_item(ArchReview *review,
                                     const char *description,
                                     ArchChecklistCategory category)
{
    if (!review || !description) return false;
    if (review->checklist_count >= ARCH_REVIEW_MAX_CHECKLIST_ITEMS) return false;
    ArchChecklistItem *item = &review->checklist[review->checklist_count];
    strncpy(item->description, description, ARCH_REVIEW_MAX_DESC_LENGTH - 1);
    item->description[ARCH_REVIEW_MAX_DESC_LENGTH - 1] = '\0';
    item->category = category;
    item->is_checked = false;
    item->is_met = false;
    item->comment[0] = '\0';
    review->checklist_count++;
    return true;
}

bool arch_review_check_item(ArchReview *review, size_t index,
                             bool is_met, const char *comment)
{
    if (!review || index >= review->checklist_count) return false;
    review->checklist[index].is_checked = true;
    review->checklist[index].is_met = is_met;
    if (comment) {
        strncpy(review->checklist[index].comment, comment,
                ARCH_REVIEW_MAX_DESC_LENGTH - 1);
        review->checklist[index].comment[ARCH_REVIEW_MAX_DESC_LENGTH - 1]
            = '\0';
    }
    return true;
}

unsigned int arch_review_add_finding(ArchReview *review,
                                      const char *title,
                                      const char *description,
                                      ArchRiskLevel risk_level,
                                      const char *recommendation,
                                      const char *action_owner,
                                      time_t action_deadline)
{
    if (!review || !title) return 0;
    if (review->finding_count >= ARCH_REVIEW_MAX_FINDINGS) return 0;
    static unsigned int finding_id_counter = 1;
    ArchFinding *f = &review->findings[review->finding_count];
    f->id = finding_id_counter++;
    strncpy(f->title, title, ARCH_REVIEW_MAX_NAME_LENGTH - 1);
    f->title[ARCH_REVIEW_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(f->description, description ? description : "",
            ARCH_REVIEW_MAX_DESC_LENGTH - 1);
    f->description[ARCH_REVIEW_MAX_DESC_LENGTH - 1] = '\0';
    f->risk_level = risk_level;
    f->is_action_required = (risk_level >= ARCH_RISK_HIGH);
    strncpy(f->recommendation, recommendation ? recommendation : "",
            ARCH_REVIEW_MAX_RECOMMENDATION - 1);
    f->recommendation[ARCH_REVIEW_MAX_RECOMMENDATION - 1] = '\0';
    strncpy(f->action_owner, action_owner ? action_owner : "",
            ARCH_REVIEW_MAX_NAME_LENGTH - 1);
    f->action_owner[ARCH_REVIEW_MAX_NAME_LENGTH - 1] = '\0';
    f->action_deadline = action_deadline;
    f->is_resolved = false;
    f->linked_adr_id = 0;
    review->finding_count++;
    return f->id;
}

bool arch_review_resolve_finding(ArchReview *review, unsigned int finding_id)
{
    ArchFinding *f = arch_review_find_finding(review, finding_id);
    if (!f) return false;
    f->is_resolved = true;
    return true;
}

bool arch_review_link_adr(ArchReview *review, unsigned int adr_id)
{
    if (!review) return false;
    if (review->linked_adr_count >= ADR_LOG_MAX_ENTRIES) return false;
    review->linked_adr_ids[review->linked_adr_count++] = adr_id;
    return true;
}

bool arch_review_schedule_meeting(ArchReview *review,
                                   time_t meeting_date,
                                   time_t duration_minutes,
                                   const char *location,
                                   const char *notes)
{
    if (!review || review->meeting_count >= 8) return false;
    ArchReviewMeeting *m = &review->meetings[review->meeting_count];
    m->meeting_date = meeting_date;
    m->duration_minutes = duration_minutes;
    strncpy(m->location, location ? location : "",
            ARCH_REVIEW_MAX_NAME_LENGTH - 1);
    m->location[ARCH_REVIEW_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(m->notes, notes ? notes : "",
            ARCH_REVIEW_MAX_MEETING_NOTES - 1);
    m->notes[ARCH_REVIEW_MAX_MEETING_NOTES - 1] = '\0';
    m->attendee_count = 0;
    m->finding_count = 0;
    review->meeting_count++;
    return true;
}

bool arch_review_start(ArchReview *review)
{
    if (!review) return false;
    if (review->status != ARCH_REVIEW_STATUS_SCHEDULED) return false;
    review->status = ARCH_REVIEW_STATUS_IN_PROGRESS;
    return true;
}

bool arch_review_complete(ArchReview *review)
{
    if (!review) return false;
    if (review->status != ARCH_REVIEW_STATUS_IN_PROGRESS) return false;
    review->status = ARCH_REVIEW_STATUS_COMPLETED;
    review->completed_date = time(NULL);
    return true;
}

bool arch_review_reject(ArchReview *review)
{
    if (!review) return false;
    review->status = ARCH_REVIEW_STATUS_REJECTED;
    return true;
}

bool arch_review_close(ArchReview *review)
{
    if (!review) return false;
    review->status = ARCH_REVIEW_STATUS_CLOSED;
    return true;
}

const char *arch_review_status_to_string(ArchReviewStatus status)
{
    switch (status) {
    case ARCH_REVIEW_STATUS_SCHEDULED:   return "Scheduled";
    case ARCH_REVIEW_STATUS_IN_PROGRESS: return "In Progress";
    case ARCH_REVIEW_STATUS_COMPLETED:   return "Completed";
    case ARCH_REVIEW_STATUS_REJECTED:    return "Rejected";
    case ARCH_REVIEW_STATUS_CLOSED:      return "Closed";
    default:                             return "Unknown";
    }
}

const char *arch_risk_level_to_string(ArchRiskLevel level)
{
    switch (level) {
    case ARCH_RISK_LOW:      return "Low";
    case ARCH_RISK_MEDIUM:   return "Medium";
    case ARCH_RISK_HIGH:     return "High";
    case ARCH_RISK_CRITICAL: return "Critical";
    default:                 return "Unknown";
    }
}

const char *arch_checklist_category_to_string(ArchChecklistCategory category)
{
    switch (category) {
    case ARCH_CHECKLIST_PERFORMANCE:    return "Performance";
    case ARCH_CHECKLIST_SCALABILITY:    return "Scalability";
    case ARCH_CHECKLIST_AVAILABILITY:   return "Availability";
    case ARCH_CHECKLIST_SECURITY:       return "Security";
    case ARCH_CHECKLIST_MAINTAINABILITY:return "Maintainability";
    case ARCH_CHECKLIST_COMPLIANCE:     return "Compliance";
    case ARCH_CHECKLIST_COST:           return "Cost";
    case ARCH_CHECKLIST_DATA_MANAGEMENT:return "Data Management";
    case ARCH_CHECKLIST_INTEGRATION:    return "Integration";
    case ARCH_CHECKLIST_GENERAL:        return "General";
    default:                            return "Unknown";
    }
}

ArchFinding *arch_review_find_finding(ArchReview *review, unsigned int id)
{
    if (!review) return NULL;
    for (size_t i = 0; i < review->finding_count; i++) {
        if (review->findings[i].id == id) return &review->findings[i];
    }
    return NULL;
}

size_t arch_review_count_open_findings(const ArchReview *review)
{
    if (!review) return 0;
    size_t count = 0;
    for (size_t i = 0; i < review->finding_count; i++) {
        if (!review->findings[i].is_resolved) count++;
    }
    return count;
}

size_t arch_review_count_findings_by_risk(const ArchReview *review,
                                           ArchRiskLevel level)
{
    if (!review) return 0;
    size_t count = 0;
    for (size_t i = 0; i < review->finding_count; i++) {
        if (review->findings[i].risk_level == level) count++;
    }
    return count;
}

double arch_review_progress(const ArchReview *review)
{
    if (!review || review->checklist_count == 0) return 0.0;
    size_t checked = 0;
    for (size_t i = 0; i < review->checklist_count; i++) {
        if (review->checklist[i].is_checked) checked++;
    }
    return (double)checked / (double)review->checklist_count * 100.0;
}

void arch_review_print_summary(const ArchReview *review)
{
    if (!review) return;
    printf("==================================================\n");
    printf("Architecture Review Report\n");
    printf("==================================================\n");
    printf("Review ID: %u\n", review->id);
    printf("Title: %s\n", review->title);
    printf("Status: %s\n", arch_review_status_to_string(review->status));
    printf("Scheduled: %s", ctime(&review->scheduled_date));
    if (review->completed_date) {
        printf("Completed: %s", ctime(&review->completed_date));
    }
    printf("\n--- Stakeholders (%zu) ---\n", review->stakeholder_count);
    for (size_t i = 0; i < review->stakeholder_count; i++) {
        printf("  %s (%s)%s%s\n",
               review->stakeholders[i].name,
               review->stakeholders[i].role,
               review->stakeholders[i].is_decision_maker ? " [DM]" : "",
               review->stakeholders[i].has_veto_power ? " [VETO]" : "");
    }
    printf("\n--- Checklist Progress: %.1f%% ---\n",
           arch_review_progress(review));
    for (size_t i = 0; i < review->checklist_count; i++) {
        printf("  [%s] [%s] %s\n",
               review->checklist[i].is_checked ? "X" : " ",
               review->checklist[i].is_met ? "PASS" : "FAIL",
               review->checklist[i].description);
    }
    printf("\n--- Findings (%zu open) ---\n",
           arch_review_count_open_findings(review));
    for (size_t i = 0; i < review->finding_count; i++) {
        printf("  [F%02u] %s | Risk: %s | Resolved: %s\n",
               review->findings[i].id,
               review->findings[i].title,
               arch_risk_level_to_string(review->findings[i].risk_level),
               review->findings[i].is_resolved ? "Yes" : "No");
    }
    printf("==================================================\n");
}

bool arch_review_export_report(const ArchReview *review,
                                const char *filename)
{
    if (!review || !filename) return false;
    FILE *fp = fopen(filename, "w");
    if (!fp) return false;
    fprintf(fp, "# Architecture Review Report\n\n");
    fprintf(fp, "## %s\n\n", review->title);
    fprintf(fp, "**Status:** %s\n\n", arch_review_status_to_string(review->status));
    fprintf(fp, "## Stakeholders\n\n");
    fprintf(fp, "| Name | Role | Decision Maker | Veto |\n");
    fprintf(fp, "|------|------|---------------|------|\n");
    for (size_t i = 0; i < review->stakeholder_count; i++) {
        fprintf(fp, "| %s | %s | %s | %s |\n",
                review->stakeholders[i].name,
                review->stakeholders[i].role,
                review->stakeholders[i].is_decision_maker ? "Yes" : "No",
                review->stakeholders[i].has_veto_power ? "Yes" : "No");
    }
    fprintf(fp, "\n## Findings\n\n");
    fprintf(fp, "| ID | Title | Risk | Resolved |\n");
    fprintf(fp, "|----|-------|------|----------|\n");
    for (size_t i = 0; i < review->finding_count; i++) {
        fprintf(fp, "| %u | %s | %s | %s |\n",
                review->findings[i].id,
                review->findings[i].title,
                arch_risk_level_to_string(review->findings[i].risk_level),
                review->findings[i].is_resolved ? "Yes" : "No");
    }
    fprintf(fp, "\n## Checklist\n\n");
    fprintf(fp, "| Item | Category | Met |\n");
    fprintf(fp, "|------|----------|-----|\n");
    for (size_t i = 0; i < review->checklist_count; i++) {
        fprintf(fp, "| %s | %s | %s |\n",
                review->checklist[i].description,
                arch_checklist_category_to_string(review->checklist[i].category),
                review->checklist[i].is_met ? "Yes" : "No");
    }
    fclose(fp);
    return true;
}
