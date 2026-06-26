#ifndef RELEASE_MGMT_H
#define RELEASE_MGMT_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int major;
    int minor;
    int patch;
    char prerelease[64];
    char build_meta[64];
} SemVer;

typedef struct {
    char name[128];
    SemVer version;
    char target_date[32];
    bool locked;
    int sprint_count;
} ReleaseTrain;

typedef struct {
    char flag_name[128];
    bool enabled;
    double rollout_percentage;
    char owner[128];
    char description[256];
    bool kill_switch_active;
} FeatureFlag;

typedef struct {
    char item[256];
    bool completed;
    char owner[128];
    char notes[256];
} ReleaseChecklistItem;

typedef struct {
    char trigger[256];
    char steps[512];
    char rollback_version[32];
    int estimated_minutes;
} RollbackPlan;

typedef enum {
    STAGE_PRE_ALPHA,
    STAGE_ALPHA,
    STAGE_BETA,
    STAGE_RC,
    STAGE_GA
} ReleaseStage;

typedef struct {
    char title[256];
    char date[32];
    char highlights[10][256];
    int highlight_count;
    char known_issues[512];
    char upgrade_guide[512];
    char thank_you[256];
} Announcement;

bool        semver_parse(const char *str, SemVer *sv);
int         semver_compare(const SemVer *a, const SemVer *b);
void        semver_bump_major(SemVer *sv);
void        semver_bump_minor(SemVer *sv);
void        semver_bump_patch(SemVer *sv);
void        semver_to_string(const SemVer *sv, char *buffer, size_t buf_size);
bool        semver_is_prerelease(const SemVer *sv);
bool        semver_is_stable(const SemVer *sv);

void        release_train_init(ReleaseTrain *rt, const char *name, int major, int minor, int patch, int sprints);
bool        release_train_is_ready(const ReleaseTrain *rt);
void        release_train_lock(ReleaseTrain *rt);
void        release_train_print(const ReleaseTrain *rt);

void        feature_flag_init(FeatureFlag *ff, const char *name, const char *owner, const char *desc);
void        feature_flag_enable(FeatureFlag *ff);
void        feature_flag_disable(FeatureFlag *ff);
void        feature_flag_set_rollout(FeatureFlag *ff, double percentage);
void        feature_flag_kill_switch(FeatureFlag *ff);
bool        feature_flag_is_active_for_user(const FeatureFlag *ff, int user_id);

void        release_checklist_init(ReleaseChecklistItem *items, size_t count);
bool        release_checklist_all_done(const ReleaseChecklistItem *items, size_t count);
double      release_checklist_progress(const ReleaseChecklistItem *items, size_t count);

void        rollback_plan_init(RollbackPlan *rp, const char *trigger, const char *steps, int minutes);
bool        rollback_plan_validate(const RollbackPlan *rp);

const char *release_stage_name(ReleaseStage stage);
bool        release_stage_can_advance(ReleaseStage current, ReleaseStage next);

void        announcement_init(Announcement *a, const char *title, const char *date);
void        announcement_add_highlight(Announcement *a, const char *highlight);
void        announcement_generate(const Announcement *a, char *buffer, size_t buf_size);

#endif
