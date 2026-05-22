#include "roadmap_plan.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    ProductRoadmap roadmap;
    roadmap_init(&roadmap, 2, 2026);

    printf("=== RICE Prioritization & Roadmap Demo ===\n\n");

    roadmap_add_feature(&roadmap, "AI-powered search", "Semantic search with LLM backend", 1000.0, 3.0, 0.8, 2.0);
    roadmap_add_feature(&roadmap, "Dark mode", "System-wide dark theme support", 5000.0, 1.0, 1.0, 1.0);
    roadmap_add_feature(&roadmap, "Export to CSV", "Data export with custom filters", 500.0, 2.0, 0.9, 0.5);
    roadmap_add_feature(&roadmap, "SSO integration", "SAML/OIDC single sign-on", 200.0, 2.5, 0.7, 3.0);
    roadmap_add_feature(&roadmap, "Mobile push notifications", "Real-time push to iOS/Android", 3000.0, 2.0, 0.85, 2.5);
    roadmap_add_feature(&roadmap, "API rate limiting", "Per-user throttling with config", 100.0, 1.5, 0.95, 0.3);
    roadmap_add_feature(&roadmap, "Audit logging", "Compliance-grade audit trail", 50.0, 1.0, 1.0, 4.0);
    roadmap_add_feature(&roadmap, "Dashboard widgets", "Drag-and-drop widget builder", 2000.0, 2.5, 0.8, 3.5);
    roadmap_add_feature(&roadmap, "Multi-language i18n", "Full localization (8 langs)", 800.0, 1.5, 0.6, 5.0);
    roadmap_add_feature(&roadmap, "Real-time collaboration", "Multi-user whiteboard", 500.0, 3.0, 0.5, 8.0);

    printf("Backlog: %zu features\n\n", roadmap.backlog_count);

    printf("Before RICE sort:\n");
    printf("%-30s %8s %8s %8s %8s %8s\n", "Feature", "Reach", "Impact", "Conf", "Effort", "RICE");
    printf("---------------------------------------------------------------\n");
    for (size_t i = 0; i < roadmap.backlog_count; i++) {
        RiceFeature *f = &roadmap.backlog[i];
        printf("%-30s %8.0f %8.1f %8.0f%% %8.1f %8.2f\n",
               f->feature_name, f->reach, f->impact,
               f->confidence * 100, f->effort, f->rice_score);
    }

    roadmap_prioritize_by_rice(&roadmap);

    printf("\nAfter RICE sort + horizon assignment:\n");
    const char *hz[] = {"NOW", "NEXT", "LATER"};
    printf("%-30s %8s %8s\n", "Feature", "RICE", "Horizon");
    printf("-----------------------------------------------\n");
    for (size_t i = 0; i < roadmap.backlog_count; i++) {
        RiceFeature *f = &roadmap.backlog[i];
        printf("%-30s %8.2f %8s\n", f->feature_name, f->rice_score, hz[f->horizon]);
    }

    printf("\n--- Horizon Counts ---\n");
    printf("  NOW:   %d\n", roadmap_count_by_horizon(&roadmap, TIME_HORIZON_NOW));
    printf("  NEXT:  %d\n", roadmap_count_by_horizon(&roadmap, TIME_HORIZON_NEXT));
    printf("  LATER: %d\n", roadmap_count_by_horizon(&roadmap, TIME_HORIZON_LATER));

    roadmap_auto_commit_now(&roadmap, 2);
    printf("\nAuto-committed 2 NOW features:\n");
    for (size_t i = 0; i < roadmap.backlog_count; i++) {
        if (roadmap.backlog[i].committed) {
            printf("  [COMMITTED] %s\n", roadmap.backlog[i].feature_name);
        }
    }

    printf("\n--- Thematic Roadmap with OKRs ---\n");
    roadmap_add_theme(&roadmap, "Platform Foundation", TIME_HORIZON_NOW);
    const char *kr0[] = {"99.9% uptime", "P95 latency < 100ms", "Zero critical vulns"};
    roadmap_set_okr(&roadmap.themes[0], "Build a reliable, secure platform", kr0, 3);

    roadmap_add_theme(&roadmap, "User Growth", TIME_HORIZON_NEXT);
    const char *kr1[] = {"MAU grows 40%", "NPS > 50", "Churn < 3%"};
    roadmap_set_okr(&roadmap.themes[1], "Accelerate user acquisition and retention", kr1, 3);

    roadmap_add_theme(&roadmap, "Intelligence", TIME_HORIZON_LATER);
    const char *kr2[] = {"AI features adopted by 30%", "1000+ datasets/week"};
    roadmap_set_okr(&roadmap.themes[2], "Leverage AI for proactive insights", kr2, 2);

    printf("\n--- Opportunity Scoring ---\n");
    struct { const char *name; double imp; double gap; } opps[] = {
        {"Better onboarding", 4.0, 2.0},
        {"Faster exports", 3.0, 4.0},
        {"Mobile app", 5.0, 3.0},
        {"Integrations", 4.5, 1.5},
    };
    for (int i = 0; i < 4; i++) {
        double score = roadmap_calc_opportunity_score(opps[i].imp, opps[i].gap);
        printf("  %-20s Importance=%.1f  Gap=%.1f  Score=%.2f\n",
               opps[i].name, opps[i].imp, opps[i].gap, score);
    }

    printf("\n--- Stakeholder Message ---\n");
    char msg[4096];
    roadmap_generate_stakeholder_message(&roadmap, msg, sizeof(msg));
    printf("%s\n", msg);

    roadmap_free(&roadmap);
    printf("Roadmap cleaned up.\n");
    return 0;
}
