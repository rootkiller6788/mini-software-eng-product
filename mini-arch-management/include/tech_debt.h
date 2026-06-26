#ifndef TECH_DEBT_H
#define TECH_DEBT_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#define TECH_DEBT_MAX_NAME_LENGTH       256
#define TECH_DEBT_MAX_DESC_LENGTH      2048
#define TECH_DEBT_MAX_REGISTER_ITEMS    256
#define TECH_DEBT_MAX_PAYBACK_STEPS      16
#define TECH_DEBT_MAX_CODE_SMELLS        8
#define TECH_DEBT_MAX_LOCATION_LENGTH   512
#define TECH_DEBT_MAX_OWNER_LENGTH      128
#define TECH_DEBT_MAX_SMELL_NAME_LENGTH  64

typedef enum TechDebtQuadrant {
    TECH_DEBT_QUADRANT_DELIBERATE_PRUDENT,
    TECH_DEBT_QUADRANT_DELIBERATE_RECKLESS,
    TECH_DEBT_QUADRANT_INADVERTENT_PRUDENT,
    TECH_DEBT_QUADRANT_INADVERTENT_RECKLESS
} TechDebtQuadrant;

typedef enum TechDebtStatus {
    TECH_DEBT_STATUS_IDENTIFIED,
    TECH_DEBT_STATUS_ACKNOWLEDGED,
    TECH_DEBT_STATUS_QUANTIFIED,
    TECH_DEBT_STATUS_PLANNED,
    TECH_DEBT_STATUS_IN_REPAYMENT,
    TECH_DEBT_STATUS_RESOLVED,
    TECH_DEBT_STATUS_ACCEPTED,
    TECH_DEBT_STATUS_WONT_FIX
} TechDebtStatus;

typedef enum CodeSmellType {
    CODE_SMELL_DUPLICATED_CODE,
    CODE_SMELL_LONG_METHOD,
    CODE_SMELL_LARGE_CLASS,
    CODE_SMELL_LONG_PARAMETER_LIST,
    CODE_SMELL_DIVERGENT_CHANGE,
    CODE_SMELL_SHOTGUN_SURGERY,
    CODE_SMELL_FEATURE_ENVY,
    CODE_SMELL_DATA_CLUMPS,
    CODE_SMELL_PRIMITIVE_OBSESSION,
    CODE_SMELL_SWITCH_STATEMENTS,
    CODE_SMELL_PARALLEL_INHERITANCE,
    CODE_SMELL_LAZY_CLASS,
    CODE_SMELL_SPECULATIVE_GENERALITY,
    CODE_SMELL_TEMPORARY_FIELD,
    CODE_SMELL_MESSAGE_CHAINS,
    CODE_SMELL_MIDDLE_MAN,
    CODE_SMELL_INAPPROPRIATE_INTIMACY,
    CODE_SMELL_ALTERNATIVE_CLASSES,
    CODE_SMELL_INCOMPLETE_LIBRARY,
    CODE_SMELL_DATA_CLASS,
    CODE_SMELL_REFUSED_BEQUEST,
    CODE_SMELL_COMMENTS_CODE_SMELL
} CodeSmellType;

typedef struct CodeLocation {
    char file_path[TECH_DEBT_MAX_LOCATION_LENGTH];
    unsigned int line_start;
    unsigned int line_end;
    char module_name[TECH_DEBT_MAX_NAME_LENGTH];
} CodeLocation;

typedef struct CodeSmell {
    CodeSmellType type;
    char name[TECH_DEBT_MAX_SMELL_NAME_LENGTH];
    CodeLocation location;
    char description[TECH_DEBT_MAX_DESC_LENGTH];
    double severity;
} CodeSmell;

typedef struct PaybackStep {
    char description[TECH_DEBT_MAX_DESC_LENGTH];
    double estimated_effort_hours;
    double interest_reduction_percent;
    time_t target_date;
    bool is_completed;
} PaybackStep;

typedef struct TechDebtItem {
    unsigned int id;
    char name[TECH_DEBT_MAX_NAME_LENGTH];
    char description[TECH_DEBT_MAX_DESC_LENGTH];
    TechDebtQuadrant quadrant;
    TechDebtStatus status;
    double principal_hours;
    double interest_rate_percent;
    double annual_interest_hours;
    time_t identified_date;
    time_t target_resolution_date;
    time_t actual_resolution_date;
    char owner[TECH_DEBT_MAX_OWNER_LENGTH];
    CodeSmell code_smells[TECH_DEBT_MAX_CODE_SMELLS];
    size_t code_smell_count;
    CodeLocation affected_locations[TECH_DEBT_MAX_PAYBACK_STEPS];
    size_t affected_location_count;
    PaybackStep payback_plan[TECH_DEBT_MAX_PAYBACK_STEPS];
    size_t payback_step_count;
    double current_accumulated_interest;
    bool is_critical;
    unsigned int linked_adr_id;
    char rationale[TECH_DEBT_MAX_DESC_LENGTH];
} TechDebtItem;

typedef struct TechDebtRegister {
    TechDebtItem items[TECH_DEBT_MAX_REGISTER_ITEMS];
    size_t count;
    unsigned int next_id;
    char project_name[TECH_DEBT_MAX_NAME_LENGTH];
    time_t last_updated;
    double base_interest_rate;
} TechDebtRegister;

void tech_debt_register_init(TechDebtRegister *register_ptr,
                              const char *project_name,
                              double base_interest_rate);
TechDebtRegister *tech_debt_register_create(const char *project_name,
                                             double base_interest_rate);
void tech_debt_register_destroy(TechDebtRegister *register_ptr);

unsigned int tech_debt_register_add_item(TechDebtRegister *register_ptr,
                                          const char *name,
                                          const char *description,
                                          TechDebtQuadrant quadrant,
                                          double principal_hours,
                                          double interest_rate_percent,
                                          const char *owner);

bool tech_debt_item_add_code_smell(TechDebtRegister *register_ptr,
                                    unsigned int item_id,
                                    CodeSmellType type,
                                    const char *file_path,
                                    unsigned int line_start,
                                    unsigned int line_end,
                                    const char *description,
                                    double severity);

bool tech_debt_item_add_payback_step(TechDebtRegister *register_ptr,
                                      unsigned int item_id,
                                      const char *description,
                                      double estimated_effort_hours,
                                      double interest_reduction_percent,
                                      time_t target_date);

bool tech_debt_item_acknowledge(TechDebtRegister *register_ptr,
                                 unsigned int item_id);
bool tech_debt_item_quantify(TechDebtRegister *register_ptr,
                              unsigned int item_id);
bool tech_debt_item_plan(TechDebtRegister *register_ptr,
                          unsigned int item_id);
bool tech_debt_item_start_repayment(TechDebtRegister *register_ptr,
                                     unsigned int item_id);
bool tech_debt_item_resolve(TechDebtRegister *register_ptr,
                             unsigned int item_id);
bool tech_debt_item_accept(TechDebtRegister *register_ptr,
                            unsigned int item_id);

double tech_debt_calculate_accumulated_interest(const TechDebtItem *item,
                                                 time_t current_date);
double tech_debt_register_total_principal(const TechDebtRegister *register_ptr);
double tech_debt_register_total_interest(const TechDebtRegister *register_ptr,
                                          time_t current_date);
double tech_debt_register_roi_payback(const TechDebtRegister *register_ptr,
                                       unsigned int item_id);
double tech_debt_register_weighted_interest_rate(
    const TechDebtRegister *register_ptr);

TechDebtItem *tech_debt_register_find(TechDebtRegister *register_ptr,
                                       unsigned int id);
TechDebtItem *tech_debt_register_find_by_status(
    TechDebtRegister *register_ptr, TechDebtStatus status, size_t *count);

const char *tech_debt_quadrant_to_string(TechDebtQuadrant quadrant);
const char *tech_debt_status_to_string(TechDebtStatus status);
const char *code_smell_type_to_string(CodeSmellType type);

bool tech_debt_item_is_deliberate(const TechDebtItem *item);
bool tech_debt_item_is_prudent(const TechDebtItem *item);

void tech_debt_register_print_summary(const TechDebtRegister *register_ptr);
void tech_debt_register_print_item(const TechDebtItem *item);
bool tech_debt_register_export_csv(const TechDebtRegister *register_ptr,
                                    const char *filename);
void tech_debt_register_update_interest(TechDebtRegister *register_ptr,
                                         time_t current_date);

#endif /* TECH_DEBT_H */
