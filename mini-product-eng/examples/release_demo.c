#include "release_mgmt.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("=== Release Management Demo ===\n\n");

    printf("--- Semantic Versioning ---\n");
    SemVer v1, v2, v3;
    semver_parse("2.3.1", &v1);
    semver_parse("2.4.0-beta.1", &v2);
    semver_parse("3.0.0-alpha+exp.sha.5114f85", &v3);

    char s1[64], s2[64], s3[64];
    semver_to_string(&v1, s1, sizeof(s1));
    semver_to_string(&v2, s2, sizeof(s2));
    semver_to_string(&v3, s3, sizeof(s3));
    printf("  v1 = %s  stable=%s  prerelease=%s\n", s1,
           semver_is_stable(&v1) ? "yes" : "no",
           semver_is_prerelease(&v1) ? "yes" : "no");
    printf("  v2 = %s  stable=%s  prerelease=%s\n", s2,
           semver_is_stable(&v2) ? "yes" : "no",
           semver_is_prerelease(&v2) ? "yes" : "no");
    printf("  v3 = %s  stable=%s  prerelease=%s\n", s3,
           semver_is_stable(&v3) ? "yes" : "no",
           semver_is_prerelease(&v3) ? "yes" : "no");
    printf("  v1 vs v2: %d  v2 vs v3: %d\n", semver_compare(&v1, &v2), semver_compare(&v2, &v3));

    SemVer bump = v1;
    semver_bump_minor(&bump);
    char bumpstr[64];
    semver_to_string(&bump, bumpstr, sizeof(bumpstr));
    printf("  After minor bump on v1: %s\n", bumpstr);
    semver_bump_major(&bump);
    semver_to_string(&bump, bumpstr, sizeof(bumpstr));
    printf("  After major bump: %s\n", bumpstr);

    printf("\n--- Release Train ---\n");
    ReleaseTrain rt;
    release_train_init(&rt, "Sprint 42 — Platform", 1, 8, 0, 2);
    release_train_print(&rt);
    printf("  Ready: %s\n", release_train_is_ready(&rt) ? "yes" : "no");
    release_train_lock(&rt);
    printf("  After lock — Ready: %s\n", release_train_is_ready(&rt) ? "yes" : "no");

    printf("\n--- Feature Flags ---\n");
    FeatureFlag ff1, ff2;
    feature_flag_init(&ff1, "new_dashboard", "alice@team", "Next-gen analytics dashboard");
    feature_flag_init(&ff2, "dark_mode", "bob@team", "System dark theme support");

    feature_flag_enable(&ff1);
    feature_flag_set_rollout(&ff1, 25.0);
    feature_flag_enable(&ff2);
    feature_flag_set_rollout(&ff2, 100.0);

    printf("  %-20s enabled=%s rollout=%.0f%% active(uid=10)=%s active(uid=50)=%s\n",
           ff1.flag_name, ff1.enabled ? "yes" : "no", ff1.rollout_percentage,
           feature_flag_is_active_for_user(&ff1, 10) ? "yes" : "no",
           feature_flag_is_active_for_user(&ff1, 50) ? "yes" : "no");
    printf("  %-20s enabled=%s rollout=%.0f%% active(uid=10)=%s\n",
           ff2.flag_name, ff2.enabled ? "yes" : "no", ff2.rollout_percentage,
           feature_flag_is_active_for_user(&ff2, 10) ? "yes" : "no");

    feature_flag_kill_switch(&ff1);
    printf("  After kill-switch on %s: active(uid=10)=%s\n",
           ff1.flag_name,
           feature_flag_is_active_for_user(&ff1, 10) ? "yes" : "no");

    printf("\n--- Release Checklist ---\n");
    ReleaseChecklistItem cl[] = {
        {"Code review completed", true, "bob", "All modules reviewed"},
        {"QA sign-off", true, "qa-team", "247 tests pass"},
        {"Performance benchmarks met", true, "devops", "P99 < 200ms"},
        {"Security audit passed", false, "security", "Pending pentest report"},
        {"Documentation updated", true, "tech-writer", "API ref + user guide"},
        {"Rollback plan tested", true, "sre", "2 rollback drills executed"},
    };
    size_t cl_count = sizeof(cl) / sizeof(cl[0]);
    printf("  Progress: %.0f%%  All done: %s\n",
           release_checklist_progress(cl, cl_count) * 100,
           release_checklist_all_done(cl, cl_count) ? "yes" : "no (1 item)");

    printf("\n--- Rollback Plan ---\n");
    RollbackPlan rp;
    rollback_plan_init(&rp, "Error rate > 5% OR P99 latency > 2s",
                       "1. Disable feature flag\n2. Revert DB migration v14\n3. Deploy v1.7.9\n4. Smoke test\n5. Notify on-call",
                       15);
    printf("  Valid: %s\n", rollback_plan_validate(&rp) ? "yes" : "no");
    printf("  Trigger: %s\n", rp.trigger);
    printf("  Estimated: %d min\n", rp.estimated_minutes);

    printf("\n--- Release Stages ---\n");
    for (ReleaseStage s = STAGE_PRE_ALPHA; s <= STAGE_GA; s++) {
        printf("  %-12s -> can advance: %s\n",
               release_stage_name(s),
               release_stage_can_advance(s, (ReleaseStage)(s + 1)) ? "yes" : "no");
    }

    printf("\n--- Release Announcement ---\n");
    Announcement ann;
    announcement_init(&ann, "SuperApp v2.0 — GA Release", "2026-06-15");
    announcement_add_highlight(&ann, "New AI-powered search with semantic understanding");
    announcement_add_highlight(&ann, "Redesigned dashboard with real-time analytics");
    announcement_add_highlight(&ann, "SSO integration (SAML/OIDC support)");
    strncpy(ann.known_issues, "CSV export: large files (>1M rows) may time out. Fix in v2.0.1.",
            sizeof(ann.known_issues) - 1);
    strncpy(ann.upgrade_guide, "Run 'superapp update' or pull latest Docker image. DB migration runs automatically.",
            sizeof(ann.upgrade_guide) - 1);
    strncpy(ann.thank_you, "Thanks to our 500+ beta testers for their invaluable feedback!",
            sizeof(ann.thank_you) - 1);

    char ann_buf[2048];
    announcement_generate(&ann, ann_buf, sizeof(ann_buf));
    printf("%s\n", ann_buf);

    return 0;
}
