#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "tech_debt.h"
#include "arch_review.h"
#include "quality_attributes.h"

int main(void)
{
    printf("=== Technical Debt, Architecture Review & Quality Attributes "
           "Example ===\n\n");

    /* ---- Technical Debt ---- */
    printf("--- Technical Debt Register ---\n\n");

    TechDebtRegister register_;
    tech_debt_register_init(&register_, "Online Banking Platform", 5.0);

    unsigned int debt1 = tech_debt_register_add_item(&register_,
        "Duplicated validation logic",
        "Customer input validation is duplicated across the Web "
        "Application (JavaScript) and the Account Service (Java). "
        "Changes to validation rules must be made in two places.",
        TECH_DEBT_QUADRANT_INADVERTENT_PRUDENT,
        80.0, 12.0, "backend-team");

    tech_debt_item_add_code_smell(&register_, debt1,
        CODE_SMELL_DUPLICATED_CODE,
        "src/account_service/validation/", 45, 230,
        "Validation logic mirrors web-app/src/validators/", 7.0);

    tech_debt_item_add_payback_step(&register_, debt1,
        "Extract shared validation rules to a specification document",
        8.0, 30.0, time(NULL) + 86400 * 14);
    tech_debt_item_add_payback_step(&register_, debt1,
        "Implement validation library and integrate into both services",
        40.0, 50.0, time(NULL) + 86400 * 30);
    tech_debt_item_add_payback_step(&register_, debt1,
        "Remove old validation code from both codebases",
        24.0, 20.0, time(NULL) + 86400 * 45);

    unsigned int debt2 = tech_debt_register_add_item(&register_,
        "Monolithic account service",
        "The Account Service has grown into a monolith handling "
        "authentication, accounts, reporting, and admin functions. "
        "Deployment coupling is increasing risk.",
        TECH_DEBT_QUADRANT_DELIBERATE_RECKLESS,
        320.0, 22.0, "architecture-team");

    tech_debt_item_add_code_smell(&register_, debt2,
        CODE_SMELL_LARGE_CLASS,
        "src/account_service/AccountService.java", 1, 4500,
        "Single class exceeding 4500 lines with 120+ public methods", 9.0);
    tech_debt_item_add_code_smell(&register_, debt2,
        CODE_SMELL_DIVERGENT_CHANGE,
        "src/account_service/", 1, 1,
        "Multiple teams modifying the same module for unrelated reasons",
        8.0);

    tech_debt_item_add_payback_step(&register_, debt2,
        "Identify bounded contexts and define service boundaries",
        40.0, 10.0, time(NULL) + 86400 * 21);
    tech_debt_item_add_payback_step(&register_, debt2,
        "Extract authentication into a separate identity service",
        100.0, 30.0, time(NULL) + 86400 * 60);
    tech_debt_item_add_payback_step(&register_, debt2,
        "Extract reporting into a separate reporting service",
        80.0, 30.0, time(NULL) + 86400 * 90);
    tech_debt_item_add_payback_step(&register_, debt2,
        "Decommission monolith after migration verified",
        40.0, 30.0, time(NULL) + 86400 * 120);

    unsigned int debt3 = tech_debt_register_add_item(&register_,
        "Missing test coverage in payment service",
        "The Go payment service was deployed rapidly and lacks unit "
        "and integration tests. Regression risk is high.",
        TECH_DEBT_QUADRANT_INADVERTENT_RECKLESS,
        60.0, 18.0, "payments-team");

    tech_debt_item_add_code_smell(&register_, debt3,
        CODE_SMELL_COMMENTS_CODE_SMELL,
        "src/payment_service/", 1, 1,
        "Large blocks of commented-out code from debugging sessions", 5.0);

    tech_debt_item_acknowledge(&register_, debt1);
    tech_debt_item_quantify(&register_, debt1);
    tech_debt_item_plan(&register_, debt1);
    tech_debt_item_start_repayment(&register_, debt1);

    tech_debt_item_acknowledge(&register_, debt2);
    tech_debt_item_quantify(&register_, debt2);
    tech_debt_item_plan(&register_, debt2);

    tech_debt_item_acknowledge(&register_, debt3);
    tech_debt_item_quantify(&register_, debt3);

    tech_debt_register_update_interest(&register_, time(NULL));

    tech_debt_register_print_summary(&register_);

    printf("\n--- Detailed Item ---\n");
    tech_debt_register_print_item(
        tech_debt_register_find(&register_, debt2));

    printf("\nQuadrant analysis:\n");
    printf("  Debt #1 deliberate? %s\n",
           tech_debt_item_is_deliberate(
               tech_debt_register_find(&register_, debt1))
           ? "Yes" : "No");
    printf("  Debt #1 prudent? %s\n",
           tech_debt_item_is_prudent(
               tech_debt_register_find(&register_, debt1))
           ? "Yes" : "No");
    printf("  Debt #2 deliberate? %s\n",
           tech_debt_item_is_deliberate(
               tech_debt_register_find(&register_, debt2))
           ? "Yes" : "No");
    printf("  Debt #2 prudent? %s\n",
           tech_debt_item_is_prudent(
               tech_debt_register_find(&register_, debt2))
           ? "Yes" : "No");

    tech_debt_register_export_csv(&register_, "tech_debt_report.csv");
    printf("\nExported tech_debt_report.csv\n");

    /* ---- Architecture Review ---- */
    printf("\n--- Architecture Review ---\n\n");

    ArchReview review;
    arch_review_init(&review, 1,
        "Q2 2025 Architecture Review - Online Banking Platform",
        "Comprehensive review of the online banking platform architecture "
        "with focus on scalability, security, and technical debt "
        "management.");

    arch_review_add_stakeholder(&review,
        "Chief Architect", "Review Lead",
        "Overall architectural integrity and adherence to standards",
        true, true);
    arch_review_add_stakeholder(&review,
        "Security Lead", "Reviewer",
        "Security posture of all system components and data flows",
        true, false);
    arch_review_add_stakeholder(&review,
        "SRE Lead", "Reviewer",
        "Operational readiness, observability, and reliability targets",
        true, false);
    arch_review_add_stakeholder(&review,
        "Product Manager", "Observer",
        "Feature roadmap alignment with architectural capabilities",
        false, false);

    arch_review_add_checklist_item(&review,
        "All services have defined SLA objectives",
        ARCH_CHECKLIST_AVAILABILITY);
    arch_review_add_checklist_item(&review,
        "Database connections use connection pooling",
        ARCH_CHECKLIST_PERFORMANCE);
    arch_review_add_checklist_item(&review,
        "All external API calls have circuit breakers",
        ARCH_CHECKLIST_SCALABILITY);
    arch_review_add_checklist_item(&review,
        "Authentication tokens have appropriate expiry",
        ARCH_CHECKLIST_SECURITY);
    arch_review_add_checklist_item(&review,
        "All services expose health check endpoints",
        ARCH_CHECKLIST_MAINTAINABILITY);
    arch_review_add_checklist_item(&review,
        "GDPR data retention policies are documented and enforced",
        ARCH_CHECKLIST_COMPLIANCE);
    arch_review_add_checklist_item(&review,
        "Infrastructure costs are within quarterly budget",
        ARCH_CHECKLIST_COST);
    arch_review_add_checklist_item(&review,
        "Data backup and restore procedures are tested monthly",
        ARCH_CHECKLIST_DATA_MANAGEMENT);

    arch_review_check_item(&review, 0, true, "SLAs defined for all "
        "customer-facing services at 99.9% availability");
    arch_review_check_item(&review, 1, true, "HikariCP configured "
        "with min=5, max=20 connections per pod");
    arch_review_check_item(&review, 2, false,
        "Circuit breaker missing on Payment Service -> Mainframe calls");
    arch_review_check_item(&review, 3, true, "JWT tokens expire "
        "after 15 minutes, refresh tokens after 7 days");

    arch_review_add_finding(&review,
        "Missing circuit breaker on critical payment path",
        "The Payment Service makes synchronous MQ calls to the Mainframe "
        "Banking System without a circuit breaker. A mainframe outage "
        "could cascade and exhaust payment service resources.",
        ARCH_RISK_HIGH,
        "Implement Hystrix/Resilience4j circuit breaker with fallback "
        "to queued retry mechanism within 2 sprints.",
        "payments-team",
        time(NULL) + 86400 * 14);

    arch_review_add_finding(&review,
        "Monolithic account service is a scaling bottleneck",
        "Account Service (4500+ lines) cannot scale components "
        "independently. Authentication spikes during market opening "
        "degrade reporting and admin functions.",
        ARCH_RISK_CRITICAL,
        "Prioritise decomposition of Account Service. Extract "
        "authentication as separate service in Q3.",
        "architecture-team",
        time(NULL) + 86400 * 90);

    arch_review_add_finding(&review,
        "Observability coverage gaps in notification service",
        "Notification service logs to stdout only; no metrics exported "
        "to Prometheus. Cannot measure delivery latency percentiles.",
        ARCH_RISK_MEDIUM,
        "Instrument notification service with Prometheus client library "
        "and expose /metrics endpoint.",
        "notifications-team",
        time(NULL) + 86400 * 21);

    arch_review_link_adr(&review, 1);
    arch_review_link_adr(&review, 3);

    arch_review_start(&review);
    arch_review_schedule_meeting(&review,
        time(NULL) + 86400 * 7, 90,
        "Conference Room B / Zoom",
        "Agenda: Review open findings, discuss account service "
        "decomposition timeline, approve circuit breaker implementation "
        "plan.");

    arch_review_complete(&review);

    arch_review_print_summary(&review);
    printf("\nReview progress: %.1f%% checklist complete\n",
           arch_review_progress(&review));
    printf("Open findings: %zu\n",
           arch_review_count_open_findings(&review));
    printf("High+Critical findings: %zu\n",
           arch_review_count_findings_by_risk(&review, ARCH_RISK_HIGH)
           + arch_review_count_findings_by_risk(&review,
                 ARCH_RISK_CRITICAL));

    arch_review_export_report(&review, "arch_review_report.md");
    printf("Exported arch_review_report.md\n");

    /* ---- Quality Attributes ---- */
    printf("\n--- Quality Attributes & ATAM ---\n\n");

    QualityModel qmodel;
    quality_model_init(&qmodel, "Online Banking Platform",
                       "Quality attribute analysis for the core banking "
                       "platform");

    quality_model_add_profile(&qmodel, QA_PERFORMANCE, 95.0);
    quality_model_add_profile(&qmodel, QA_SCALABILITY, 90.0);
    quality_model_add_profile(&qmodel, QA_AVAILABILITY, 99.9);
    quality_model_add_profile(&qmodel, QA_SECURITY, 100.0);
    quality_model_add_profile(&qmodel, QA_MAINTAINABILITY, 85.0);
    quality_model_add_profile(&qmodel, QA_TESTABILITY, 80.0);

    quality_model_add_scenario(&qmodel, QA_PERFORMANCE,
        "User requests account balance",
        "Mobile App User",
        "Normal operation, peak hours",
        "Account Service",
        "Returns balance within 200ms",
        "P95 latency measured via Prometheus histogram",
        QA_PRIORITY_HIGH);

    quality_model_add_scenario(&qmodel, QA_PERFORMANCE,
        "User initiates fund transfer",
        "Web App User",
        "Normal operation",
        "Payment Service",
        "Transfer completes within 2 seconds end-to-end",
        "P99 latency from API Gateway logs",
        QA_PRIORITY_CRITICAL);

    quality_model_add_scenario(&qmodel, QA_AVAILABILITY,
        "Mainframe banking system becomes unavailable",
        "Mainframe Banking System",
        "Unplanned outage during business hours",
        "Online Banking System",
        "Degraded mode: balance display from cache, payments queued",
        "System continues serving read requests with stale data",
        QA_PRIORITY_CRITICAL);

    quality_model_add_scenario(&qmodel, QA_SECURITY,
        "Brute-force login attempt detected",
        "External attacker",
        "Production environment",
        "API Gateway / Auth Module",
        "Account locked after 5 failed attempts, alert sent to SOC",
        "Incident response time < 5 minutes",
        QA_PRIORITY_CRITICAL);

    quality_model_add_scenario(&qmodel, QA_SCALABILITY,
        "Black Friday traffic spike (10x normal)",
        "Customer base",
        "Peak shopping event",
        "Online Banking System",
        "Auto-scale to 10x pods, no degradation beyond 500ms added "
        "latency",
        "HPA scales based on CPU/memory, measured by K6 load test",
        QA_PRIORITY_HIGH);

    quality_model_add_scenario(&qmodel, QA_MAINTAINABILITY,
        "New developer joins the team",
        "New backend engineer",
        "Development environment setup",
        "Account Service codebase",
        "Developer can run full test suite within 1 hour of onboarding",
        "Time to first commit measured during onboarding",
        QA_PRIORITY_MEDIUM);

    quality_model_test_scenario(&qmodel, 1, true);
    quality_model_test_scenario(&qmodel, 2, true);
    quality_model_test_scenario(&qmodel, 3, false);
    quality_model_test_scenario(&qmodel, 4, true);
    quality_model_test_scenario(&qmodel, 5, true);
    quality_model_test_scenario(&qmodel, 6, true);

    unsigned int trade_off_1 = quality_model_add_trade_off(&qmodel,
        "Performance vs Security: SSL termination at API gateway",
        "Terminating SSL at the API gateway improves performance by "
        "offloading crypto from application containers, but internal "
        "traffic between gateway and services is unencrypted.",
        QA_PERFORMANCE, QA_SECURITY, 0.6, 0.4);

    unsigned int trade_off_2 = quality_model_add_trade_off(&qmodel,
        "Scalability vs Availability: Session state storage",
        "Sticky sessions simplify state management but reduce "
        "availability (cannot route around failed instances). "
        "Externalising session state improves availability but adds "
        "latency.",
        QA_SCALABILITY, QA_AVAILABILITY, 0.5, 0.5);

    quality_model_resolve_trade_off(&qmodel, trade_off_1,
        8.5, 6.0,
        "Proceed with SSL termination at gateway, but implement mTLS "
        "for internal service-to-service communication in next quarter. "
        "Risk accepted for 3-month window.");
    quality_model_resolve_trade_off(&qmodel, trade_off_2,
        7.0, 8.5,
        "Externalise session state to Redis cluster. Accept 50ms added "
        "latency for improved availability and horizontal scaling.");

    quality_model_add_sla(&qmodel,
        "API availability", "Availability of customer-facing API endpoints",
        99.9, "%", 99.5, 99.0, SLA_SEVERITY_CRITICAL,
        QA_AVAILABILITY);

    quality_model_add_sla(&qmodel,
        "API response time", "P95 latency for account balance endpoint",
        200.0, "ms", 400.0, 800.0, SLA_SEVERITY_MAJOR,
        QA_PERFORMANCE);

    quality_model_add_sla(&qmodel,
        "Failed login rate", "Rate of failed login attempts per minute",
        5.0, "count/min", 10.0, 20.0, SLA_SEVERITY_CRITICAL,
        QA_SECURITY);

    unsigned int slo1 = quality_model_add_slo(&qmodel,
        "API availability SLO", "Monthly uptime target for API gateway",
        99.9, "%", QA_AVAILABILITY, 1);
    unsigned int slo2 = quality_model_add_slo(&qmodel,
        "P95 latency SLO", "Monthly P95 latency for account queries",
        200.0, "ms", QA_PERFORMANCE, 2);

    unsigned int sli1 = quality_model_add_sli(&qmodel,
        "API uptime", "Measured uptime percentage", "%",
        QA_AVAILABILITY);
    unsigned int sli2 = quality_model_add_sli(&qmodel,
        "Account query P95", "P95 latency of /accounts/{id}/balance",
        "ms", QA_PERFORMANCE);

    quality_model_update_sli(&qmodel, sli1, 99.95, 99.95, 100.0);
    quality_model_update_sli(&qmodel, sli2, 178.0, 950.0, 1000.0);

    for (size_t i = 0; i < qmodel.slo_count; i++) {
        qmodel.slo_items[i].achieved_value = (i == 0) ? 99.95 : 178.0;
        quality_model_evaluate_slo(&qmodel, qmodel.slo_items[i].id);
    }

    quality_model_print_atam_summary(&qmodel);
    printf("\n");
    quality_model_print_sla_slo_summary(&qmodel);

    printf("\nOverall quality score: %.1f%%\n",
           quality_model_get_overall_score(&qmodel));
    printf("SLO compliance: %.1f%%\n",
           quality_model_get_slo_compliance(&qmodel));

    quality_model_export_report(&qmodel, "quality_model_report.md");
    printf("Exported quality_model_report.md\n");

    if (quality_model_validate(&qmodel)) {
        printf("Quality model validation: PASSED\n");
    }

    printf("\nAll examples completed successfully.\n");
    return 0;
}
