#include "project_charter.h"
#include "project_risk.h"
#include "project_decision.h"
#include "project_stakeholder.h"
#include "project_compliance.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("===== Project Governance Demo =====\n\n");

    /* Project Charter */
    printf("--- Project Charter ---\n");
    ProjectCharter pc;
    charter_init(&pc, "DevOps Migration", "CTO Office", "Alice Chen");
    pc.status = CS_ACTIVE;
    pc.budget_total = 500000; pc.budget_spent = 180000;
    pc.timeline_days = 180; pc.days_elapsed = 60;
    charter_add_objective(&pc, "Reduce deploy time to <5 min", 5.0);
    charter_add_objective(&pc, "Achieve 99.9% uptime", 99.9);
    charter_add_objective(&pc, "Migrate 50 services", 50.0);
    charter_update_objective(&pc, 0, 3.5);
    charter_update_objective(&pc, 1, 99.8);
    charter_update_objective(&pc, 2, 20.0);
    charter_add_constraint(&pc, "No downtime during business hours", "time");
    charter_add_constraint(&pc, "Budget capped at $500K", "budget");
    charter_add_scope(&pc, "CI/CD pipeline setup", true);
    charter_add_scope(&pc, "Monitoring & alerting", true);
    charter_add_scope(&pc, "Rewrite all applications", false);
    charter_print(&pc);
    printf("\n");

    /* Risk Registry */
    printf("--- Risk Registry ---\n");
    RiskRegistry rr;
    risk_registry_init(&rr, 50);
    risk_add(&rr, "R01", "Vendor lock-in with cloud provider", RPROB_POSSIBLE, RSEV_HIGH);
    risk_add(&rr, "R02", "Key engineer attrition", RPROB_UNLIKELY, RSEV_CRITICAL);
    risk_add(&rr, "R03", "Data migration failure", RPROB_RARE, RSEV_CRITICAL);
    risk_add(&rr, "R04", "Performance regression post-migration", RPROB_LIKELY, RSEV_MEDIUM);
    risk_set_mitigation(&rr, "R01", "Use cloud-agnostic IaC tools", "Infra Team");
    risk_set_mitigation(&rr, "R02", "Cross-training and documentation", "EM");
    risk_set_mitigation(&rr, "R04", "Load testing before cutover", "QA Lead");
    risk_update_status(&rr, "R04", RSTAT_MITIGATED);
    risk_print(&rr);
    risk_print_matrix(&rr);
    printf("Exposure: %d, Within appetite: %s\n\n",
           risk_exposure(&rr), risk_within_appetite(&rr) ? "YES" : "NO");

    /* Decision Log */
    printf("--- Decision Log ---\n");
    DecisionLog dl;
    decision_log_init(&dl);
    decision_add(&dl, "D-001", "Choose cloud provider", "Must select primary cloud for migration");
    decision_add_option(&dl, 0, "AWS", "Largest market share, best tooling", "Higher cost, vendor lock-in");
    decision_add_option(&dl, 0, "GCP", "Strong Kubernetes, good pricing", "Smaller market share");
    decision_add_option(&dl, 0, "Azure", "Best MSFT integration", "Weaker container orchestration");
    decision_make(&dl, 0, 0, "CTO", "2026-03-15");
    decision_add(&dl, "D-002", "Monolith vs microservices", "Target architecture for migrated services");
    decision_add_option(&dl, 1, "Microservices", "Independent deploy, scale", "Operational complexity");
    decision_add_option(&dl, 1, "Modular monolith", "Simpler operations", "Less flexible scaling");
    decision_print(&dl, 0);
    decision_print_all(&dl);
    printf("\n");

    /* Stakeholder Map */
    printf("--- Stakeholder Map ---\n");
    StakeholderMap sm;
    stakeholder_map_init(&sm);
    stakeholder_add(&sm, "CTO", "Executive Sponsor", SINFL_HIGH, SINTEREST_HIGH);
    stakeholder_add(&sm, "VP Eng", "Business Owner", SINFL_HIGH, SINTEREST_HIGH);
    stakeholder_add(&sm, "Infra Lead", "Technical Lead", SINFL_MEDIUM, SINTEREST_HIGH);
    stakeholder_add(&sm, "QA Manager", "Quality Assurance", SINFL_LOW, SINTEREST_MEDIUM);
    stakeholder_add(&sm, "End Users", "Users", SINFL_LOW, SINTEREST_LOW);
    stakeholder_set_key(&sm, 0, true);
    stakeholder_set_key(&sm, 1, true);
    stakeholder_set_key(&sm, 2, true);
    stakeholder_print_matrix(&sm);
    raci_add(&sm, "Cloud architecture design");
    raci_assign(&sm, 0, 2, RACI_R);
    raci_assign(&sm, 0, 1, RACI_A);
    raci_assign(&sm, 0, 0, RACI_I);
    raci_assign(&sm, 0, 3, RACI_C);
    raci_add(&sm, "Security review");
    raci_assign(&sm, 1, 3, RACI_R);
    raci_assign(&sm, 1, 0, RACI_A);
    stakeholder_print_raci(&sm);
    printf("\n");

    /* Compliance */
    printf("--- Compliance Plan ---\n");
    CompliancePlan cp;
    compliance_init(&cp, "SOC2 Type II");
    compliance_add_gate(&cp, "Design Review", "Architecture and security design must be reviewed");
    compliance_add_check(&cp, 0, "Threat model completed", true);
    compliance_add_check(&cp, 0, "Architecture diagram documented", true);
    compliance_add_check(&cp, 0, "Data flow mapped with PII locations", true);
    compliance_add_gate(&cp, "Pre-Deployment", "All checks must pass before production deployment");
    compliance_add_check(&cp, 1, "Penetration test passed", true);
    compliance_add_check(&cp, 1, "Load test results within SLA", false);
    compliance_add_check(&cp, 1, "Rollback plan tested", true);
    compliance_pass_check(&cp, 0, 0, "threat_model_v2.pdf");
    compliance_pass_check(&cp, 0, 1, "arch_diagram.png");
    compliance_pass_check(&cp, 0, 2, "data_flow_map.xlsx");
    compliance_review_gate(&cp, 0, "Security Architect");
    compliance_audit_log(&cp, "Design review gate passed", "Security Architect", "PENDING", "PASSED");
    compliance_audit_log(&cp, "Pre-deployment gate opened", "Release Manager", "N/A", "IN REVIEW");
    compliance_print_gate(&cp, 0);
    compliance_print_gate(&cp, 1);
    compliance_print_all(&cp);

    return 0;
}
