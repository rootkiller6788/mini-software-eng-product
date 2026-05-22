#include "project_compliance.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

void compliance_init(CompliancePlan *cp, const char *standard) {
    memset(cp, 0, sizeof(*cp));
    strncpy(cp->standard, standard, 63); cp->standard[63] = '\0';
}

int compliance_add_gate(CompliancePlan *cp, const char *name, const char *desc) {
    if (cp->gate_count >= MAX_GATES) return -1;
    StageGate *g = &cp->gates[cp->gate_count];
    strncpy(g->name, name, 63); g->name[63] = '\0';
    strncpy(g->description, desc, 255); g->description[255] = '\0';
    g->check_count = 0; g->status = GSTAT_PENDING;
    g->reviewer[0] = '\0'; g->review_date[0] = '\0';
    return cp->gate_count++;
}

int compliance_add_check(CompliancePlan *cp, int gate_idx, const char *desc, bool required) {
    if (gate_idx < 0 || gate_idx >= cp->gate_count) return -1;
    StageGate *g = &cp->gates[gate_idx];
    if (g->check_count >= MAX_CHECKS_PER_GATE) return -1;
    ComplianceCheck *cc = &g->checks[g->check_count];
    strncpy(cc->description, desc, 255); cc->description[255] = '\0';
    cc->required = required; cc->passed = false; cc->evidence[0] = '\0';
    return g->check_count++;
}

void compliance_pass_check(CompliancePlan *cp, int gate_idx, int check_idx, const char *evidence) {
    if (gate_idx < 0 || gate_idx >= cp->gate_count) return;
    StageGate *g = &cp->gates[gate_idx];
    if (check_idx < 0 || check_idx >= g->check_count) return;
    g->checks[check_idx].passed = true;
    strncpy(g->checks[check_idx].evidence, evidence, 255);
}

bool compliance_gate_ready(CompliancePlan *cp, int gate_idx) {
    if (gate_idx < 0 || gate_idx >= cp->gate_count) return false;
    StageGate *g = &cp->gates[gate_idx];
    for (int i = 0; i < g->check_count; i++)
        if (g->checks[i].required && !g->checks[i].passed) return false;
    return true;
}

void compliance_review_gate(CompliancePlan *cp, int gate_idx, const char *reviewer) {
    if (gate_idx < 0 || gate_idx >= cp->gate_count) return;
    StageGate *g = &cp->gates[gate_idx];
    g->status = GSTAT_IN_REVIEW;
    strncpy(g->reviewer, reviewer, 63); g->reviewer[63] = '\0';
    g->status = compliance_gate_ready(cp, gate_idx) ? GSTAT_PASSED : GSTAT_FAILED;
}

void compliance_audit_log(CompliancePlan *cp, const char *action, const char *by,
                          const char *before, const char *after) {
    if (cp->audit_count >= MAX_AUDIT_ENTRIES) return;
    AuditEntry *ae = &cp->audit_trail[cp->audit_count];
    strncpy(ae->action, action, 127); ae->action[127] = '\0';
    strncpy(ae->by, by, 63); ae->by[63] = '\0';
    strncpy(ae->before_state, before, 127); ae->before_state[127] = '\0';
    strncpy(ae->after_state, after, 127); ae->after_state[127] = '\0';
    /* simple timestamp */
    snprintf(ae->timestamp, sizeof(ae->timestamp), "%u", (unsigned)time(NULL));
    cp->audit_count++;
}

bool compliance_all_gates_passed(CompliancePlan *cp) {
    for (int i = 0; i < cp->gate_count; i++)
        if (cp->gates[i].status != GSTAT_PASSED) return false;
    return true;
}

void compliance_print_gate(CompliancePlan *cp, int gate_idx) {
    if (gate_idx < 0 || gate_idx >= cp->gate_count) return;
    StageGate *g = &cp->gates[gate_idx];
    const char *ss[] = {"PENDING","IN REVIEW","PASSED","FAILED"};
    printf("=== Gate: %s [%s] ===\n%s\n", g->name, ss[g->status], g->description);
    for (int i = 0; i < g->check_count; i++)
        printf("  [%s%s] %s  %s\n",
               g->checks[i].required ? "R" : "O",
               g->checks[i].passed ? " PASS" : " FAIL",
               g->checks[i].description,
               g->checks[i].evidence[0] ? g->checks[i].evidence : "");
    if (g->reviewer[0]) printf("  Reviewed by: %s\n", g->reviewer);
}

void compliance_print_all(CompliancePlan *cp) {
    printf("=== Compliance Plan: %s (%d gates) ===\n", cp->standard, cp->gate_count);
    for (int i = 0; i < cp->gate_count; i++) {
        const char *ss[] = {"PEND","REV","PASS","FAIL"};
        int passed = 0, required = 0;
        for (int j = 0; j < cp->gates[i].check_count; j++) {
            if (cp->gates[i].checks[j].required) required++;
            if (cp->gates[i].checks[j].passed) passed++;
        }
        printf("  %s [%s] %d/%d checks passed\n", cp->gates[i].name, ss[cp->gates[i].status], passed, cp->gates[i].check_count);
    }
    printf("  All gates passed: %s\n", compliance_all_gates_passed(cp) ? "YES" : "NO");
    printf("  Audit trail: %d entries\n", cp->audit_count);
}
