#ifndef QUALITY_ATTRIBUTES_H
#define QUALITY_ATTRIBUTES_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#define QA_MAX_NAME_LENGTH           256
#define QA_MAX_DESC_LENGTH          2048
#define QA_MAX_SCENARIOS              64
#define QA_MAX_ATTRIBUTES             16
#define QA_MAX_TRADE_OFFS            64
#define QA_MAX_SLA_ITEMS             32
#define QA_MAX_SLO_ITEMS             64
#define QA_MAX_SLI_ITEMS             64

typedef enum QualityAttributeType {
    QA_PERFORMANCE,
    QA_SCALABILITY,
    QA_AVAILABILITY,
    QA_SECURITY,
    QA_MAINTAINABILITY,
    QA_TESTABILITY,
    QA_USABILITY,
    QA_RELIABILITY,
    QA_PORTABILITY,
    QA_INTEROPERABILITY,
    QA_MODIFIABILITY,
    QA_MONITORABILITY,
    QA_DEPLOYABILITY,
    QA_OBSERVABILITY
} QualityAttributeType;

typedef enum QaPriority {
    QA_PRIORITY_LOW = 1,
    QA_PRIORITY_MEDIUM = 2,
    QA_PRIORITY_HIGH = 3,
    QA_PRIORITY_CRITICAL = 4
} QaPriority;

typedef enum SlaSeverity {
    SLA_SEVERITY_MINOR,
    SLA_SEVERITY_MAJOR,
    SLA_SEVERITY_CRITICAL
} SlaSeverity;

typedef struct SlaItem {
    unsigned int id;
    char metric_name[QA_MAX_NAME_LENGTH];
    char description[QA_MAX_DESC_LENGTH];
    double target_value;
    char unit[QA_MAX_NAME_LENGTH];
    double warning_threshold;
    double breach_threshold;
    SlaSeverity severity;
    QualityAttributeType attribute;
    char measurement_window[QA_MAX_NAME_LENGTH];
    char remediation[QA_MAX_DESC_LENGTH];
} SlaItem;

typedef struct SloItem {
    unsigned int id;
    char name[QA_MAX_NAME_LENGTH];
    char description[QA_MAX_DESC_LENGTH];
    double target_value;
    double achieved_value;
    char unit[QA_MAX_NAME_LENGTH];
    time_t measurement_period_start;
    time_t measurement_period_end;
    bool is_met;
    QualityAttributeType attribute;
    unsigned int sla_id;
} SloItem;

typedef struct SliItem {
    unsigned int id;
    char name[QA_MAX_NAME_LENGTH];
    char description[QA_MAX_DESC_LENGTH];
    double current_value;
    char unit[QA_MAX_NAME_LENGTH];
    QualityAttributeType attribute;
    time_t last_measured;
    double good_events;
    double valid_events;
} SliItem;

typedef struct QualityScenario {
    unsigned int id;
    char stimulus[QA_MAX_DESC_LENGTH];
    char source[QA_MAX_NAME_LENGTH];
    char environment[QA_MAX_NAME_LENGTH];
    char artifact[QA_MAX_NAME_LENGTH];
    char response[QA_MAX_DESC_LENGTH];
    char response_measure[QA_MAX_DESC_LENGTH];
    QualityAttributeType attribute;
    QaPriority priority;
    bool is_tested;
    bool is_met;
} QualityScenario;

typedef struct TradeOffAnalysis {
    unsigned int id;
    char title[QA_MAX_NAME_LENGTH];
    char description[QA_MAX_DESC_LENGTH];
    QualityAttributeType attribute_a;
    QualityAttributeType attribute_b;
    double score_a;
    double score_b;
    double weight_a;
    double weight_b;
    char decision_rationale[QA_MAX_DESC_LENGTH];
    unsigned int linked_adr_id;
    bool is_resolved;
} TradeOffAnalysis;

typedef struct QualityAttributeProfile {
    QualityAttributeType type;
    char name[QA_MAX_NAME_LENGTH];
    double target_score;
    double current_score;
    char measurement_approach[QA_MAX_DESC_LENGTH];
    QualityScenario scenarios[QA_MAX_SCENARIOS];
    size_t scenario_count;
    SlaItem sla_items[QA_MAX_SLA_ITEMS];
    size_t sla_count;
} QualityAttributeProfile;

typedef struct QualityModel {
    char system_name[QA_MAX_NAME_LENGTH];
    char description[QA_MAX_DESC_LENGTH];
    time_t created_at;
    time_t updated_at;
    QualityAttributeProfile profiles[QA_MAX_ATTRIBUTES];
    size_t profile_count;
    TradeOffAnalysis trade_offs[QA_MAX_TRADE_OFFS];
    size_t trade_off_count;
    SloItem slo_items[QA_MAX_SLO_ITEMS];
    size_t slo_count;
    SliItem sli_items[QA_MAX_SLI_ITEMS];
    size_t sli_count;
    unsigned int next_id;
} QualityModel;

void quality_model_init(QualityModel *model,
                         const char *system_name,
                         const char *description);
QualityModel *quality_model_create(const char *system_name,
                                    const char *description);
void quality_model_destroy(QualityModel *model);

bool quality_model_add_profile(QualityModel *model,
                                QualityAttributeType type,
                                double target_score);

unsigned int quality_model_add_scenario(QualityModel *model,
                                         QualityAttributeType attribute,
                                         const char *stimulus,
                                         const char *source,
                                         const char *environment,
                                         const char *artifact,
                                         const char *response,
                                         const char *response_measure,
                                         QaPriority priority);

bool quality_model_test_scenario(QualityModel *model,
                                  unsigned int scenario_id, bool is_met);

unsigned int quality_model_add_trade_off(QualityModel *model,
                                          const char *title,
                                          const char *description,
                                          QualityAttributeType attribute_a,
                                          QualityAttributeType attribute_b,
                                          double weight_a, double weight_b);
bool quality_model_resolve_trade_off(QualityModel *model,
                                      unsigned int trade_off_id,
                                      double score_a, double score_b,
                                      const char *decision_rationale);

unsigned int quality_model_add_sla(QualityModel *model,
                                    const char *metric_name,
                                    const char *description,
                                    double target_value,
                                    const char *unit,
                                    double warning_threshold,
                                    double breach_threshold,
                                    SlaSeverity severity,
                                    QualityAttributeType attribute);

unsigned int quality_model_add_slo(QualityModel *model,
                                    const char *name,
                                    const char *description,
                                    double target_value,
                                    const char *unit,
                                    QualityAttributeType attribute,
                                    unsigned int sla_id);

unsigned int quality_model_add_sli(QualityModel *model,
                                    const char *name,
                                    const char *description,
                                    const char *unit,
                                    QualityAttributeType attribute);

bool quality_model_update_sli(QualityModel *model,
                               unsigned int sli_id,
                               double current_value,
                               double good_events,
                               double valid_events);

bool quality_model_evaluate_slo(QualityModel *model, unsigned int slo_id);

QualityAttributeProfile *quality_model_find_profile(QualityModel *model,
                                                     QualityAttributeType type);
SlaItem *quality_model_find_sla(QualityModel *model, unsigned int id);
SloItem *quality_model_find_slo(QualityModel *model, unsigned int id);
SliItem *quality_model_find_sli(QualityModel *model, unsigned int id);

const char *qa_type_to_string(QualityAttributeType type);
const char *qa_priority_to_string(QaPriority priority);
const char *sla_severity_to_string(SlaSeverity severity);
QualityAttributeType qa_type_from_string(const char *str);

double quality_model_get_score(const QualityModel *model,
                                QualityAttributeType type);
double quality_model_get_overall_score(const QualityModel *model);
double quality_model_get_sla_compliance(const QualityModel *model);
double quality_model_get_slo_compliance(const QualityModel *model);

void quality_model_print_atam_summary(const QualityModel *model);
void quality_model_print_sla_slo_summary(const QualityModel *model);
bool quality_model_export_report(const QualityModel *model,
                                  const char *filename);

bool quality_model_validate(const QualityModel *model);

#endif /* QUALITY_ATTRIBUTES_H */
