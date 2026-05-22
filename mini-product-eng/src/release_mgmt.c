#include "release_mgmt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool semver_parse(const char *str, SemVer *sv) {
    if (!str || !sv) return false;
    memset(sv, 0, sizeof(*sv));
    char prerelease_buf[64] = {0};
    char build_buf[64] = {0};
    int n = sscanf(str, "%d.%d.%d-%63[^+]%63s",
                   &sv->major, &sv->minor, &sv->patch, prerelease_buf, build_buf);
    if (n >= 3) {
        if (n >= 4 && prerelease_buf[0]) {
            strncpy(sv->prerelease, prerelease_buf, sizeof(sv->prerelease) - 1);
        }
        if (n >= 5 && build_buf[0]) {
            strncpy(sv->build_meta, build_buf, sizeof(sv->build_meta) - 1);
        }
        return true;
    }
    n = sscanf(str, "%d.%d.%d", &sv->major, &sv->minor, &sv->patch);
    return n == 3;
}

int semver_compare(const SemVer *a, const SemVer *b) {
    if (!a || !b) return 0;
    if (a->major != b->major) return (a->major > b->major) ? 1 : -1;
    if (a->minor != b->minor) return (a->minor > b->minor) ? 1 : -1;
    if (a->patch != b->patch) return (a->patch > b->patch) ? 1 : -1;
    bool a_pre = (a->prerelease[0] != '\0');
    bool b_pre = (b->prerelease[0] != '\0');
    if (a_pre && !b_pre) return -1;
    if (!a_pre && b_pre) return 1;
    if (a_pre && b_pre) return strcmp(a->prerelease, b->prerelease);
    return 0;
}

void semver_bump_major(SemVer *sv) {
    if (!sv) return;
    sv->major++;
    sv->minor = 0;
    sv->patch = 0;
    sv->prerelease[0] = '\0';
    sv->build_meta[0] = '\0';
}

void semver_bump_minor(SemVer *sv) {
    if (!sv) return;
    sv->minor++;
    sv->patch = 0;
    sv->prerelease[0] = '\0';
    sv->build_meta[0] = '\0';
}

void semver_bump_patch(SemVer *sv) {
    if (!sv) return;
    sv->patch++;
    sv->prerelease[0] = '\0';
    sv->build_meta[0] = '\0';
}

void semver_to_string(const SemVer *sv, char *buffer, size_t buf_size) {
    if (!sv || !buffer || buf_size == 0) return;
    int w = snprintf(buffer, buf_size, "%d.%d.%d", sv->major, sv->minor, sv->patch);
    if (sv->prerelease[0]) {
        w += snprintf(buffer + w, buf_size - w, "-%s", sv->prerelease);
    }
    if (sv->build_meta[0]) {
        snprintf(buffer + w, buf_size - w, "+%s", sv->build_meta);
    }
}

bool semver_is_prerelease(const SemVer *sv) {
    return sv && sv->prerelease[0] != '\0';
}

bool semver_is_stable(const SemVer *sv) {
    return sv && sv->major > 0 && sv->prerelease[0] == '\0';
}

void release_train_init(ReleaseTrain *rt, const char *name, int major, int minor, int patch, int sprints) {
    if (!rt) return;
    memset(rt, 0, sizeof(*rt));
    if (name) {
        strncpy(rt->name, name, sizeof(rt->name) - 1);
        rt->name[sizeof(rt->name) - 1] = '\0';
    }
    rt->version.major = major;
    rt->version.minor = minor;
    rt->version.patch = patch;
    rt->sprint_count = sprints;
    rt->locked = false;
}

bool release_train_is_ready(const ReleaseTrain *rt) {
    if (!rt) return false;
    return rt->locked && rt->sprint_count > 0;
}

void release_train_lock(ReleaseTrain *rt) {
    if (rt) rt->locked = true;
}

void release_train_print(const ReleaseTrain *rt) {
    if (!rt) return;
    char ver[64];
    semver_to_string(&rt->version, ver, sizeof(ver));
    printf("Release Train: %s v%s  sprints=%d  locked=%s\n",
           rt->name, ver, rt->sprint_count, rt->locked ? "yes" : "no");
}

void feature_flag_init(FeatureFlag *ff, const char *name, const char *owner, const char *desc) {
    if (!ff) return;
    memset(ff, 0, sizeof(*ff));
    if (name) {
        strncpy(ff->flag_name, name, sizeof(ff->flag_name) - 1);
        ff->flag_name[sizeof(ff->flag_name) - 1] = '\0';
    }
    if (owner) {
        strncpy(ff->owner, owner, sizeof(ff->owner) - 1);
        ff->owner[sizeof(ff->owner) - 1] = '\0';
    }
    if (desc) {
        strncpy(ff->description, desc, sizeof(ff->description) - 1);
        ff->description[sizeof(ff->description) - 1] = '\0';
    }
    ff->enabled = false;
    ff->rollout_percentage = 0.0;
    ff->kill_switch_active = false;
}

void feature_flag_enable(FeatureFlag *ff) {
    if (ff) ff->enabled = true;
}

void feature_flag_disable(FeatureFlag *ff) {
    if (ff) { ff->enabled = false; ff->rollout_percentage = 0.0; }
}

void feature_flag_set_rollout(FeatureFlag *ff, double percentage) {
    if (!ff) return;
    if (percentage < 0.0) percentage = 0.0;
    if (percentage > 100.0) percentage = 100.0;
    ff->rollout_percentage = percentage;
}

void feature_flag_kill_switch(FeatureFlag *ff) {
    if (!ff) return;
    ff->kill_switch_active = true;
    ff->enabled = false;
}

bool feature_flag_is_active_for_user(const FeatureFlag *ff, int user_id) {
    if (!ff) return false;
    if (ff->kill_switch_active) return false;
    if (!ff->enabled) return false;
    if (ff->rollout_percentage >= 100.0) return true;
    int bucket = user_id % 100;
    return (double)bucket < ff->rollout_percentage;
}

void release_checklist_init(ReleaseChecklistItem *items, size_t count) {
    if (items && count > 0) {
        memset(items, 0, count * sizeof(ReleaseChecklistItem));
    }
}

bool release_checklist_all_done(const ReleaseChecklistItem *items, size_t count) {
    if (!items || count == 0) return false;
    for (size_t i = 0; i < count; i++) {
        if (!items[i].completed) return false;
    }
    return true;
}

double release_checklist_progress(const ReleaseChecklistItem *items, size_t count) {
    if (!items || count == 0) return 0.0;
    int done = 0;
    for (size_t i = 0; i < count; i++) {
        if (items[i].completed) done++;
    }
    return (double)done / (double)count;
}

void rollback_plan_init(RollbackPlan *rp, const char *trigger, const char *steps, int minutes) {
    if (!rp) return;
    memset(rp, 0, sizeof(*rp));
    if (trigger) {
        strncpy(rp->trigger, trigger, sizeof(rp->trigger) - 1);
        rp->trigger[sizeof(rp->trigger) - 1] = '\0';
    }
    if (steps) {
        strncpy(rp->steps, steps, sizeof(rp->steps) - 1);
        rp->steps[sizeof(rp->steps) - 1] = '\0';
    }
    rp->estimated_minutes = minutes;
}

bool rollback_plan_validate(const RollbackPlan *rp) {
    if (!rp) return false;
    return rp->trigger[0] != '\0' && rp->steps[0] != '\0' && rp->estimated_minutes > 0;
}

const char *release_stage_name(ReleaseStage stage) {
    switch (stage) {
        case STAGE_PRE_ALPHA: return "Pre-Alpha";
        case STAGE_ALPHA:     return "Alpha";
        case STAGE_BETA:      return "Beta";
        case STAGE_RC:        return "RC";
        case STAGE_GA:        return "GA";
        default:              return "Unknown";
    }
}

bool release_stage_can_advance(ReleaseStage current, ReleaseStage next) {
    if (current >= STAGE_GA) return false;
    return (int)next == (int)current + 1;
}

void announcement_init(Announcement *a, const char *title, const char *date) {
    if (!a) return;
    memset(a, 0, sizeof(*a));
    if (title) {
        strncpy(a->title, title, sizeof(a->title) - 1);
        a->title[sizeof(a->title) - 1] = '\0';
    }
    if (date) {
        strncpy(a->date, date, sizeof(a->date) - 1);
        a->date[sizeof(a->date) - 1] = '\0';
    }
}

void announcement_add_highlight(Announcement *a, const char *highlight) {
    if (!a || !highlight || a->highlight_count >= 10) return;
    strncpy(a->highlights[a->highlight_count], highlight, sizeof(a->highlights[0]) - 1);
    a->highlights[a->highlight_count][sizeof(a->highlights[0]) - 1] = '\0';
    a->highlight_count++;
}

void announcement_generate(const Announcement *a, char *buffer, size_t buf_size) {
    if (!a || !buffer || buf_size == 0) return;
    int w = 0;
    w = snprintf(buffer, buf_size,
        "========================================\n"
        "  %s\n"
        "  Release Date: %s\n"
        "========================================\n\n", a->title, a->date);

    if (a->highlight_count > 0) {
        w += snprintf(buffer + w, buf_size - w, "What's New:\n");
        for (int i = 0; i < a->highlight_count; i++) {
            w += snprintf(buffer + w, buf_size - w, "  * %s\n", a->highlights[i]);
        }
        w += snprintf(buffer + w, buf_size - w, "\n");
    }

    if (a->known_issues[0]) {
        w += snprintf(buffer + w, buf_size - w, "Known Issues:\n  %s\n\n", a->known_issues);
    }

    if (a->upgrade_guide[0]) {
        w += snprintf(buffer + w, buf_size - w, "Upgrade Guide:\n  %s\n\n", a->upgrade_guide);
    }

    if (a->thank_you[0]) {
        w += snprintf(buffer + w, buf_size - w, "%s\n", a->thank_you);
    } else {
        w += snprintf(buffer + w, buf_size - w,
            "Thank you for using our product!\n");
    }
}
