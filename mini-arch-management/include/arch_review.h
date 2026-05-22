#ifndef ARCH_REVIEW_H
#define ARCH_REVIEW_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>
#include "adr_system.h"

#define ARCH_REVIEW_MAX_NAME_LENGTH       256
#define ARCH_REVIEW_MAX_DESC_LENGTH      2048
#define ARCH_REVIEW_MAX_FINDINGS          64
#define ARCH_REVIEW_MAX_CHECKLIST_ITEMS   64
#define ARCH_REVIEW_MAX_STAKEHOLDERS      32
#define ARCH_REVIEW_MAX_MEETING_NOTES    4096
#define ARCH_REVIEW_MAX_RECOMMENDATION    1024
#define ARCH_REVIEW_MAX_ACTION_ITEMS      32

typedef enum ArchReviewStatus {
    ARCH_REVIEW_STATUS_SCHEDULED,
    ARCH_REVIEW_STATUS_IN_PROGRESS,
    ARCH_REVIEW_STATUS_COMPLETED,
    ARCH_REVIEW_STATUS_REJECTED,
    ARCH_REVIEW_STATUS_CLOSED
} ArchReviewStatus;

typedef enum ArchRiskLevel {
    ARCH_RISK_LOW,
    ARCH_RISK_MEDIUM,
    ARCH_RISK_HIGH,
    ARCH_RISK_CRITICAL
} ArchRiskLevel;

typedef enum ArchChecklistCategory {
    ARCH_CHECKLIST_PERFORMANCE,
    ARCH_CHECKLIST_SCALABILITY,
    ARCH_CHECKLIST_AVAILABILITY,
    ARCH_CHECKLIST_SECURITY,
    ARCH_CHECKLIST_MAINTAINABILITY,
    ARCH_CHECKLIST_COMPLIANCE,
    ARCH_CHECKLIST_COST,
    ARCH_CHECKLIST_DATA_MANAGEMENT,
    ARCH_CHECKLIST_INTEGRATION,
    ARCH_CHECKLIST_GENERAL
} ArchChecklistCategory;

typedef struct ArchStakeholder {
    char name[ARCH_REVIEW_MAX_NAME_LENGTH];
    char role[ARCH_REVIEW_MAX_NAME_LENGTH];
    char concerns[ARCH_REVIEW_MAX_DESC_LENGTH];
    bool is_decision_maker;
    bool has_veto_power;
} ArchStakeholder;

typedef struct ArchChecklistItem {
    char description[ARCH_REVIEW_MAX_DESC_LENGTH];
    ArchChecklistCategory category;
    bool is_checked;
    bool is_met;
    char comment[ARCH_REVIEW_MAX_DESC_LENGTH];
} ArchChecklistItem;

typedef struct ArchFinding {
    unsigned int id;
    char title[ARCH_REVIEW_MAX_NAME_LENGTH];
    char description[ARCH_REVIEW_MAX_DESC_LENGTH];
    ArchRiskLevel risk_level;
    bool is_action_required;
    char recommendation[ARCH_REVIEW_MAX_RECOMMENDATION];
    char action_owner[ARCH_REVIEW_MAX_NAME_LENGTH];
    time_t action_deadline;
    bool is_resolved;
    unsigned int linked_adr_id;
} ArchFinding;

typedef struct ArchReviewMeeting {
    time_t meeting_date;
    time_t duration_minutes;
    char location[ARCH_REVIEW_MAX_NAME_LENGTH];
    char notes[ARCH_REVIEW_MAX_MEETING_NOTES];
    ArchStakeholder attendees[ARCH_REVIEW_MAX_STAKEHOLDERS];
    size_t attendee_count;
    ArchFinding meeting_findings[ARCH_REVIEW_MAX_FINDINGS];
    size_t finding_count;
} ArchReviewMeeting;

typedef struct ArchReview {
    unsigned int id;
    char title[ARCH_REVIEW_MAX_NAME_LENGTH];
    char description[ARCH_REVIEW_MAX_DESC_LENGTH];
    ArchReviewStatus status;
    time_t scheduled_date;
    time_t completed_date;
    ArchReviewMeeting meetings[8];
    size_t meeting_count;
    ArchStakeholder stakeholders[ARCH_REVIEW_MAX_STAKEHOLDERS];
    size_t stakeholder_count;
    ArchChecklistItem checklist[ARCH_REVIEW_MAX_CHECKLIST_ITEMS];
    size_t checklist_count;
    ArchFinding findings[ARCH_REVIEW_MAX_FINDINGS];
    size_t finding_count;
    unsigned int linked_adr_ids[ADR_LOG_MAX_ENTRIES];
    size_t linked_adr_count;
    ArchFinding action_items[ARCH_REVIEW_MAX_ACTION_ITEMS];
    size_t action_item_count;
} ArchReview;

void arch_review_init(ArchReview *review, unsigned int id,
                       const char *title, const char *description);
ArchReview *arch_review_create(unsigned int id, const char *title,
                                const char *description);
void arch_review_destroy(ArchReview *review);

bool arch_review_add_stakeholder(ArchReview *review,
                                  const char *name, const char *role,
                                  const char *concerns,
                                  bool is_decision_maker, bool has_veto_power);
bool arch_review_remove_stakeholder(ArchReview *review, const char *name);

bool arch_review_add_checklist_item(ArchReview *review,
                                     const char *description,
                                     ArchChecklistCategory category);
bool arch_review_check_item(ArchReview *review, size_t index,
                             bool is_met, const char *comment);

unsigned int arch_review_add_finding(ArchReview *review,
                                      const char *title,
                                      const char *description,
                                      ArchRiskLevel risk_level,
                                      const char *recommendation,
                                      const char *action_owner,
                                      time_t action_deadline);
bool arch_review_resolve_finding(ArchReview *review, unsigned int finding_id);
bool arch_review_link_adr(ArchReview *review, unsigned int adr_id);

bool arch_review_schedule_meeting(ArchReview *review,
                                   time_t meeting_date,
                                   time_t duration_minutes,
                                   const char *location,
                                   const char *notes);

bool arch_review_start(ArchReview *review);
bool arch_review_complete(ArchReview *review);
bool arch_review_reject(ArchReview *review);
bool arch_review_close(ArchReview *review);

const char *arch_review_status_to_string(ArchReviewStatus status);
const char *arch_risk_level_to_string(ArchRiskLevel level);
const char *arch_checklist_category_to_string(ArchChecklistCategory category);

ArchFinding *arch_review_find_finding(ArchReview *review, unsigned int id);

size_t arch_review_count_open_findings(const ArchReview *review);
size_t arch_review_count_findings_by_risk(const ArchReview *review,
                                           ArchRiskLevel level);

double arch_review_progress(const ArchReview *review);

void arch_review_print_summary(const ArchReview *review);
bool arch_review_export_report(const ArchReview *review,
                                const char *filename);

#endif /* ARCH_REVIEW_H */
