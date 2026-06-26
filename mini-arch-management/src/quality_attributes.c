#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "quality_attributes.h"

void quality_model_init(QualityModel *model,
                         const char *system_name,
                         const char *description)
{
    if (!model) return;
    memset(model, 0, sizeof(*model));
    if (system_name) {
        strncpy(model->system_name, system_name, QA_MAX_NAME_LENGTH - 1);
        model->system_name[QA_MAX_NAME_LENGTH - 1] = '\0';
    }
    if (description) {
        strncpy(model->description, description, QA_MAX_DESC_LENGTH - 1);
        model->description[QA_MAX_DESC_LENGTH - 1] = '\0';
    }
    model->next_id = 1;
    model->created_at = time(NULL);
    model->updated_at = model->created_at;
}

QualityModel *quality_model_create(const char *system_name,
                                    const char *description)
{
    QualityModel *model = (QualityModel *)malloc(sizeof(QualityModel));
    if (!model) return NULL;
    quality_model_init(model, system_name, description);
    return model;
}

void quality_model_destroy(QualityModel *model)
{
    if (model) free(model);
}

bool quality_model_add_profile(QualityModel *model,
                                QualityAttributeType type,
                                double target_score)
{
    if (!model || model->profile_count >= QA_MAX_ATTRIBUTES) return false;
    for (size_t i = 0; i < model->profile_count; i++) {
        if (model->profiles[i].type == type) return false;
    }
    QualityAttributeProfile *p = &model->profiles[model->profile_count];
    memset(p, 0, sizeof(*p));
    p->type = type;
    strncpy(p->name, qa_type_to_string(type), QA_MAX_NAME_LENGTH - 1);
    p->name[QA_MAX_NAME_LENGTH - 1] = '\0';
    p->target_score = target_score;
    p->current_score = 0.0;
    p->scenario_count = 0;
    p->sla_count = 0;
    strncpy(p->measurement_approach, "To be defined",
            QA_MAX_DESC_LENGTH - 1);
    model->profile_count++;
    model->updated_at = time(NULL);
    return true;
}

static QualityAttributeProfile *find_profile(QualityModel *model,
                                              QualityAttributeType type)
{
    if (!model) return NULL;
    for (size_t i = 0; i < model->profile_count; i++) {
        if (model->profiles[i].type == type) return &model->profiles[i];
    }
    return NULL;
}

unsigned int quality_model_add_scenario(QualityModel *model,
                                         QualityAttributeType attribute,
                                         const char *stimulus,
                                         const char *source,
                                         const char *environment,
                                         const char *artifact,
                                         const char *response,
                                         const char *response_measure,
                                         QaPriority priority)
{
    if (!model) return 0;
    QualityAttributeProfile *p = find_profile(model, attribute);
    if (!p || p->scenario_count >= QA_MAX_SCENARIOS) return 0;
    QualityScenario *s = &p->scenarios[p->scenario_count];
    s->id = model->next_id++;
    strncpy(s->stimulus, stimulus ? stimulus : "", QA_MAX_DESC_LENGTH - 1);
    s->stimulus[QA_MAX_DESC_LENGTH - 1] = '\0';
    strncpy(s->source, source ? source : "", QA_MAX_NAME_LENGTH - 1);
    s->source[QA_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(s->environment, environment ? environment : "",
            QA_MAX_NAME_LENGTH - 1);
    s->environment[QA_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(s->artifact, artifact ? artifact : "", QA_MAX_NAME_LENGTH - 1);
    s->artifact[QA_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(s->response, response ? response : "", QA_MAX_DESC_LENGTH - 1);
    s->response[QA_MAX_DESC_LENGTH - 1] = '\0';
    strncpy(s->response_measure, response_measure ? response_measure : "",
            QA_MAX_DESC_LENGTH - 1);
    s->response_measure[QA_MAX_DESC_LENGTH - 1] = '\0';
    s->attribute = attribute;
    s->priority = priority;
    s->is_tested = false;
    s->is_met = false;
    p->scenario_count++;
    model->updated_at = time(NULL);
    return s->id;
}

bool quality_model_test_scenario(QualityModel *model,
                                  unsigned int scenario_id, bool is_met)
{
    if (!model) return false;
    for (size_t i = 0; i < model->profile_count; i++) {
        for (size_t j = 0; j < model->profiles[i].scenario_count; j++) {
            if (model->profiles[i].scenarios[j].id == scenario_id) {
                model->profiles[i].scenarios[j].is_tested = true;
                model->profiles[i].scenarios[j].is_met = is_met;
                size_t met = 0;
                for (size_t k = 0; k < model->profiles[i].scenario_count; k++) {
                    if (model->profiles[i].scenarios[k].is_tested
                        && model->profiles[i].scenarios[k].is_met)
                        met++;
                }
                if (model->profiles[i].scenario_count > 0) {
                    model->profiles[i].current_score =
                        (double)met
                        / (double)model->profiles[i].scenario_count * 100.0;
                }
                model->updated_at = time(NULL);
                return true;
            }
        }
    }
    return false;
}

unsigned int quality_model_add_trade_off(QualityModel *model,
                                          const char *title,
                                          const char *description,
                                          QualityAttributeType attribute_a,
                                          QualityAttributeType attribute_b,
                                          double weight_a, double weight_b)
{
    if (!model || model->trade_off_count >= QA_MAX_TRADE_OFFS) return 0;
    TradeOffAnalysis *t = &model->trade_offs[model->trade_off_count];
    t->id = model->next_id++;
    strncpy(t->title, title ? title : "", QA_MAX_NAME_LENGTH - 1);
    t->title[QA_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(t->description, description ? description : "",
            QA_MAX_DESC_LENGTH - 1);
    t->description[QA_MAX_DESC_LENGTH - 1] = '\0';
    t->attribute_a = attribute_a;
    t->attribute_b = attribute_b;
    t->score_a = 0.0;
    t->score_b = 0.0;
    t->weight_a = weight_a;
    t->weight_b = weight_b;
    t->decision_rationale[0] = '\0';
    t->linked_adr_id = 0;
    t->is_resolved = false;
    model->trade_off_count++;
    model->updated_at = time(NULL);
    return t->id;
}

bool quality_model_resolve_trade_off(QualityModel *model,
                                      unsigned int trade_off_id,
                                      double score_a, double score_b,
                                      const char *decision_rationale)
{
    if (!model) return false;
    for (size_t i = 0; i < model->trade_off_count; i++) {
        if (model->trade_offs[i].id == trade_off_id) {
            model->trade_offs[i].score_a = score_a;
            model->trade_offs[i].score_b = score_b;
            model->trade_offs[i].is_resolved = true;
            if (decision_rationale) {
                strncpy(model->trade_offs[i].decision_rationale,
                        decision_rationale, QA_MAX_DESC_LENGTH - 1);
                model->trade_offs[i]
                    .decision_rationale[QA_MAX_DESC_LENGTH - 1] = '\0';
            }
            model->updated_at = time(NULL);
            return true;
        }
    }
    return false;
}

unsigned int quality_model_add_sla(QualityModel *model,
                                    const char *metric_name,
                                    const char *description,
                                    double target_value,
                                    const char *unit,
                                    double warning_threshold,
                                    double breach_threshold,
                                    SlaSeverity severity,
                                    QualityAttributeType attribute)
{
    if (!model) return 0;
    QualityAttributeProfile *p = find_profile(model, attribute);
    if (!p || p->sla_count >= QA_MAX_SLA_ITEMS) return 0;
    SlaItem *s = &p->sla_items[p->sla_count];
    s->id = model->next_id++;
    strncpy(s->metric_name, metric_name ? metric_name : "",
            QA_MAX_NAME_LENGTH - 1);
    s->metric_name[QA_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(s->description, description ? description : "",
            QA_MAX_DESC_LENGTH - 1);
    s->description[QA_MAX_DESC_LENGTH - 1] = '\0';
    s->target_value = target_value;
    strncpy(s->unit, unit ? unit : "", QA_MAX_NAME_LENGTH - 1);
    s->unit[QA_MAX_NAME_LENGTH - 1] = '\0';
    s->warning_threshold = warning_threshold;
    s->breach_threshold = breach_threshold;
    s->severity = severity;
    s->attribute = attribute;
    strncpy(s->measurement_window, "1 month", QA_MAX_NAME_LENGTH - 1);
    s->measurement_window[QA_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(s->remediation, "Notify on-call", QA_MAX_DESC_LENGTH - 1);
    s->remediation[QA_MAX_DESC_LENGTH - 1] = '\0';
    p->sla_count++;
    model->updated_at = time(NULL);
    return s->id;
}

unsigned int quality_model_add_slo(QualityModel *model,
                                    const char *name,
                                    const char *description,
                                    double target_value,
                                    const char *unit,
                                    QualityAttributeType attribute,
                                    unsigned int sla_id)
{
    if (!model || model->slo_count >= QA_MAX_SLO_ITEMS) return 0;
    SloItem *s = &model->slo_items[model->slo_count];
    s->id = model->next_id++;
    strncpy(s->name, name ? name : "", QA_MAX_NAME_LENGTH - 1);
    s->name[QA_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(s->description, description ? description : "",
            QA_MAX_DESC_LENGTH - 1);
    s->description[QA_MAX_DESC_LENGTH - 1] = '\0';
    s->target_value = target_value;
    s->achieved_value = 0.0;
    strncpy(s->unit, unit ? unit : "", QA_MAX_NAME_LENGTH - 1);
    s->unit[QA_MAX_NAME_LENGTH - 1] = '\0';
    s->measurement_period_start = time(NULL);
    s->measurement_period_end = s->measurement_period_start + 2592000;
    s->is_met = false;
    s->attribute = attribute;
    s->sla_id = sla_id;
    model->slo_count++;
    model->updated_at = time(NULL);
    return s->id;
}

unsigned int quality_model_add_sli(QualityModel *model,
                                    const char *name,
                                    const char *description,
                                    const char *unit,
                                    QualityAttributeType attribute)
{
    if (!model || model->sli_count >= QA_MAX_SLI_ITEMS) return 0;
    SliItem *s = &model->sli_items[model->sli_count];
    s->id = model->next_id++;
    strncpy(s->name, name ? name : "", QA_MAX_NAME_LENGTH - 1);
    s->name[QA_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(s->description, description ? description : "",
            QA_MAX_DESC_LENGTH - 1);
    s->description[QA_MAX_DESC_LENGTH - 1] = '\0';
    s->current_value = 0.0;
    strncpy(s->unit, unit ? unit : "", QA_MAX_NAME_LENGTH - 1);
    s->unit[QA_MAX_NAME_LENGTH - 1] = '\0';
    s->attribute = attribute;
    s->last_measured = time(NULL);
    s->good_events = 0.0;
    s->valid_events = 0.0;
    model->sli_count++;
    model->updated_at = time(NULL);
    return s->id;
}

bool quality_model_update_sli(QualityModel *model,
                               unsigned int sli_id,
                               double current_value,
                               double good_events,
                               double valid_events)
{
    SliItem *s = quality_model_find_sli(model, sli_id);
    if (!s) return false;
    s->current_value = current_value;
    s->good_events = good_events;
    s->valid_events = valid_events;
    s->last_measured = time(NULL);
    model->updated_at = time(NULL);
    return true;
}

bool quality_model_evaluate_slo(QualityModel *model, unsigned int slo_id)
{
    SloItem *s = quality_model_find_slo(model, slo_id);
    if (!s) return false;
    s->is_met = s->achieved_value >= s->target_value;
    return true;
}

QualityAttributeProfile *quality_model_find_profile(QualityModel *model,
                                                     QualityAttributeType type)
{
    return find_profile(model, type);
}

SlaItem *quality_model_find_sla(QualityModel *model, unsigned int id)
{
    if (!model) return NULL;
    for (size_t i = 0; i < model->profile_count; i++) {
        for (size_t j = 0; j < model->profiles[i].sla_count; j++) {
            if (model->profiles[i].sla_items[j].id == id)
                return &model->profiles[i].sla_items[j];
        }
    }
    return NULL;
}

SloItem *quality_model_find_slo(QualityModel *model, unsigned int id)
{
    if (!model) return NULL;
    for (size_t i = 0; i < model->slo_count; i++) {
        if (model->slo_items[i].id == id) return &model->slo_items[i];
    }
    return NULL;
}

SliItem *quality_model_find_sli(QualityModel *model, unsigned int id)
{
    if (!model) return NULL;
    for (size_t i = 0; i < model->sli_count; i++) {
        if (model->sli_items[i].id == id) return &model->sli_items[i];
    }
    return NULL;
}

const char *qa_type_to_string(QualityAttributeType type)
{
    switch (type) {
    case QA_PERFORMANCE:      return "Performance";
    case QA_SCALABILITY:      return "Scalability";
    case QA_AVAILABILITY:     return "Availability";
    case QA_SECURITY:         return "Security";
    case QA_MAINTAINABILITY:  return "Maintainability";
    case QA_TESTABILITY:      return "Testability";
    case QA_USABILITY:        return "Usability";
    case QA_RELIABILITY:      return "Reliability";
    case QA_PORTABILITY:      return "Portability";
    case QA_INTEROPERABILITY: return "Interoperability";
    case QA_MODIFIABILITY:    return "Modifiability";
    case QA_MONITORABILITY:   return "Monitorability";
    case QA_DEPLOYABILITY:    return "Deployability";
    case QA_OBSERVABILITY:    return "Observability";
    default:                  return "Unknown";
    }
}

const char *qa_priority_to_string(QaPriority priority)
{
    switch (priority) {
    case QA_PRIORITY_LOW:      return "Low";
    case QA_PRIORITY_MEDIUM:   return "Medium";
    case QA_PRIORITY_HIGH:     return "High";
    case QA_PRIORITY_CRITICAL: return "Critical";
    default:                   return "Unknown";
    }
}

const char *sla_severity_to_string(SlaSeverity severity)
{
    switch (severity) {
    case SLA_SEVERITY_MINOR:    return "Minor";
    case SLA_SEVERITY_MAJOR:    return "Major";
    case SLA_SEVERITY_CRITICAL: return "Critical";
    default:                    return "Unknown";
    }
}

QualityAttributeType qa_type_from_string(const char *str)
{
    if (!str) return QA_PERFORMANCE;
    if (strcmp(str, "Scalability") == 0)       return QA_SCALABILITY;
    if (strcmp(str, "Availability") == 0)      return QA_AVAILABILITY;
    if (strcmp(str, "Security") == 0)          return QA_SECURITY;
    if (strcmp(str, "Maintainability") == 0)   return QA_MAINTAINABILITY;
    if (strcmp(str, "Testability") == 0)       return QA_TESTABILITY;
    if (strcmp(str, "Usability") == 0)         return QA_USABILITY;
    if (strcmp(str, "Reliability") == 0)       return QA_RELIABILITY;
    if (strcmp(str, "Portability") == 0)       return QA_PORTABILITY;
    if (strcmp(str, "Interoperability") == 0)  return QA_INTEROPERABILITY;
    if (strcmp(str, "Modifiability") == 0)     return QA_MODIFIABILITY;
    if (strcmp(str, "Monitorability") == 0)    return QA_MONITORABILITY;
    if (strcmp(str, "Deployability") == 0)     return QA_DEPLOYABILITY;
    if (strcmp(str, "Observability") == 0)     return QA_OBSERVABILITY;
    return QA_PERFORMANCE;
}

double quality_model_get_score(const QualityModel *model,
                                QualityAttributeType type)
{
    QualityAttributeProfile *p = find_profile((QualityModel *)model, type);
    return p ? p->current_score : 0.0;
}

double quality_model_get_overall_score(const QualityModel *model)
{
    if (!model || model->profile_count == 0) return 0.0;
    double total = 0.0;
    for (size_t i = 0; i < model->profile_count; i++) {
        total += model->profiles[i].current_score;
    }
    return total / (double)model->profile_count;
}

double quality_model_get_sla_compliance(const QualityModel *model)
{
    if (!model) return 0.0;
    size_t total = 0;
    for (size_t i = 0; i < model->profile_count; i++) {
        total += model->profiles[i].sla_count;
    }
    return total > 0 ? 100.0 : 0.0;
}

double quality_model_get_slo_compliance(const QualityModel *model)
{
    if (!model || model->slo_count == 0) return 0.0;
    size_t met = 0;
    for (size_t i = 0; i < model->slo_count; i++) {
        if (model->slo_items[i].is_met) met++;
    }
    return (double)met / (double)model->slo_count * 100.0;
}

void quality_model_print_atam_summary(const QualityModel *model)
{
    if (!model) return;
    printf("==================================================\n");
    printf("ATAM Summary: %s\n", model->system_name);
    printf("==================================================\n");
    printf("Overall Score: %.1f%%\n", quality_model_get_overall_score(model));
    printf("\n--- Quality Attributes ---\n");
    for (size_t i = 0; i < model->profile_count; i++) {
        printf("  %-20s: %.1f%% / %.1f%% (%zu scenarios)\n",
               model->profiles[i].name,
               model->profiles[i].current_score,
               model->profiles[i].target_score,
               model->profiles[i].scenario_count);
    }
    printf("\n--- Trade-off Analysis (%zu items) ---\n",
           model->trade_off_count);
    for (size_t i = 0; i < model->trade_off_count; i++) {
        const TradeOffAnalysis *t = &model->trade_offs[i];
        printf("  [%02u] %s\n", t->id, t->title);
        printf("        %s (w=%.2f) vs %s (w=%.2f)\n",
               qa_type_to_string(t->attribute_a), t->weight_a,
               qa_type_to_string(t->attribute_b), t->weight_b);
        if (t->is_resolved) {
            printf("        Scores: A=%.1f, B=%.1f | %s\n",
                   t->score_a, t->score_b, t->decision_rationale);
        } else {
            printf("        [Unresolved]\n");
        }
    }
    printf("==================================================\n");
}

void quality_model_print_sla_slo_summary(const QualityModel *model)
{
    if (!model) return;
    printf("==================================================\n");
    printf("SLA/SLO/SLI Summary: %s\n", model->system_name);
    printf("==================================================\n");
    printf("SLA Compliance: %.1f%%\n",
           quality_model_get_sla_compliance(model));
    printf("SLO Compliance: %.1f%%\n",
           quality_model_get_slo_compliance(model));
    printf("\n--- SLI ---\n");
    for (size_t i = 0; i < model->sli_count; i++) {
        printf("  [%02u] %s: %.2f %s (%s)\n",
               model->sli_items[i].id,
               model->sli_items[i].name,
               model->sli_items[i].current_value,
               model->sli_items[i].unit,
               qa_type_to_string(model->sli_items[i].attribute));
    }
    printf("\n--- SLO ---\n");
    for (size_t i = 0; i < model->slo_count; i++) {
        printf("  [%02u] %s: %.2f/%.2f %s [%s]\n",
               model->slo_items[i].id,
               model->slo_items[i].name,
               model->slo_items[i].achieved_value,
               model->slo_items[i].target_value,
               model->slo_items[i].unit,
               model->slo_items[i].is_met ? "MET" : "FAIL");
    }
    printf("==================================================\n");
}

bool quality_model_export_report(const QualityModel *model,
                                  const char *filename)
{
    if (!model || !filename) return false;
    FILE *fp = fopen(filename, "w");
    if (!fp) return false;
    fprintf(fp, "# Quality Model Report: %s\n\n", model->system_name);
    fprintf(fp, "## Overview\n\n");
    fprintf(fp, "**Overall Score:** %.1f%%\n\n",
            quality_model_get_overall_score(model));
    fprintf(fp, "## Quality Attributes\n\n");
    fprintf(fp, "| Attribute | Current | Target |\n");
    fprintf(fp, "|-----------|---------|--------|\n");
    for (size_t i = 0; i < model->profile_count; i++) {
        fprintf(fp, "| %s | %.1f%% | %.1f%% |\n",
                model->profiles[i].name,
                model->profiles[i].current_score,
                model->profiles[i].target_score);
    }
    fprintf(fp, "\n## Trade-off Analysis\n\n");
    for (size_t i = 0; i < model->trade_off_count; i++) {
        fprintf(fp, "### %s\n\n", model->trade_offs[i].title);
        fprintf(fp, "%s vs %s (weights: %.2f / %.2f)\n\n",
                qa_type_to_string(model->trade_offs[i].attribute_a),
                qa_type_to_string(model->trade_offs[i].attribute_b),
                model->trade_offs[i].weight_a,
                model->trade_offs[i].weight_b);
        if (model->trade_offs[i].is_resolved) {
            fprintf(fp, "**Decision:** %s\n\n",
                    model->trade_offs[i].decision_rationale);
        }
    }
    fclose(fp);
    return true;
}

bool quality_model_validate(const QualityModel *model)
{
    if (!model) return false;
    if (model->profile_count > QA_MAX_ATTRIBUTES) return false;
    if (model->trade_off_count > QA_MAX_TRADE_OFFS) return false;
    if (model->slo_count > QA_MAX_SLO_ITEMS) return false;
    if (model->sli_count > QA_MAX_SLI_ITEMS) return false;
    return true;
}
