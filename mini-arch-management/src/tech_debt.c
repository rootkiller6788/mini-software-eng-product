#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "tech_debt.h"

static const double SECONDS_IN_YEAR = 31536000.0;

void tech_debt_register_init(TechDebtRegister *register_ptr,
                              const char *project_name,
                              double base_interest_rate)
{
    if (!register_ptr) return;
    memset(register_ptr, 0, sizeof(*register_ptr));
    if (project_name) {
        strncpy(register_ptr->project_name, project_name,
                TECH_DEBT_MAX_NAME_LENGTH - 1);
        register_ptr->project_name[TECH_DEBT_MAX_NAME_LENGTH - 1] = '\0';
    }
    register_ptr->base_interest_rate = base_interest_rate;
    register_ptr->next_id = 1;
    register_ptr->count = 0;
    register_ptr->last_updated = time(NULL);
}

TechDebtRegister *tech_debt_register_create(const char *project_name,
                                             double base_interest_rate)
{
    TechDebtRegister *reg = (TechDebtRegister *)malloc(sizeof(TechDebtRegister));
    if (!reg) return NULL;
    tech_debt_register_init(reg, project_name, base_interest_rate);
    return reg;
}

void tech_debt_register_destroy(TechDebtRegister *register_ptr)
{
    if (register_ptr) free(register_ptr);
}

unsigned int tech_debt_register_add_item(TechDebtRegister *register_ptr,
                                          const char *name,
                                          const char *description,
                                          TechDebtQuadrant quadrant,
                                          double principal_hours,
                                          double interest_rate_percent,
                                          const char *owner)
{
    if (!register_ptr || register_ptr->count >= TECH_DEBT_MAX_REGISTER_ITEMS)
        return 0;

    TechDebtItem *item = &register_ptr->items[register_ptr->count];
    memset(item, 0, sizeof(*item));
    item->id = register_ptr->next_id++;
    strncpy(item->name, name ? name : "", TECH_DEBT_MAX_NAME_LENGTH - 1);
    item->name[TECH_DEBT_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(item->description, description ? description : "",
            TECH_DEBT_MAX_DESC_LENGTH - 1);
    item->description[TECH_DEBT_MAX_DESC_LENGTH - 1] = '\0';
    item->quadrant = quadrant;
    item->status = TECH_DEBT_STATUS_IDENTIFIED;
    item->principal_hours = principal_hours;
    item->interest_rate_percent = interest_rate_percent;
    item->annual_interest_hours = principal_hours * interest_rate_percent / 100.0;
    item->identified_date = time(NULL);
    item->target_resolution_date = 0;
    item->actual_resolution_date = 0;
    strncpy(item->owner, owner ? owner : "", TECH_DEBT_MAX_OWNER_LENGTH - 1);
    item->owner[TECH_DEBT_MAX_OWNER_LENGTH - 1] = '\0';
    item->code_smell_count = 0;
    item->affected_location_count = 0;
    item->payback_step_count = 0;
    item->current_accumulated_interest = 0;
    item->is_critical = false;
    item->linked_adr_id = 0;
    item->rationale[0] = '\0';

    register_ptr->count++;
    register_ptr->last_updated = time(NULL);
    return item->id;
}

bool tech_debt_item_add_code_smell(TechDebtRegister *register_ptr,
                                    unsigned int item_id,
                                    CodeSmellType type,
                                    const char *file_path,
                                    unsigned int line_start,
                                    unsigned int line_end,
                                    const char *description,
                                    double severity)
{
    TechDebtItem *item = tech_debt_register_find(register_ptr, item_id);
    if (!item || item->code_smell_count >= TECH_DEBT_MAX_CODE_SMELLS)
        return false;
    CodeSmell *smell = &item->code_smells[item->code_smell_count];
    smell->type = type;
    strncpy(smell->name, code_smell_type_to_string(type),
            TECH_DEBT_MAX_SMELL_NAME_LENGTH - 1);
    smell->name[TECH_DEBT_MAX_SMELL_NAME_LENGTH - 1] = '\0';
    strncpy(smell->location.file_path, file_path ? file_path : "",
            TECH_DEBT_MAX_LOCATION_LENGTH - 1);
    smell->location.file_path[TECH_DEBT_MAX_LOCATION_LENGTH - 1] = '\0';
    smell->location.line_start = line_start;
    smell->location.line_end = line_end;
    smell->location.module_name[0] = '\0';
    strncpy(smell->description, description ? description : "",
            TECH_DEBT_MAX_DESC_LENGTH - 1);
    smell->description[TECH_DEBT_MAX_DESC_LENGTH - 1] = '\0';
    smell->severity = severity;
    item->code_smell_count++;
    return true;
}

bool tech_debt_item_add_payback_step(TechDebtRegister *register_ptr,
                                      unsigned int item_id,
                                      const char *description,
                                      double estimated_effort_hours,
                                      double interest_reduction_percent,
                                      time_t target_date)
{
    TechDebtItem *item = tech_debt_register_find(register_ptr, item_id);
    if (!item || item->payback_step_count >= TECH_DEBT_MAX_PAYBACK_STEPS)
        return false;
    PaybackStep *step = &item->payback_plan[item->payback_step_count];
    strncpy(step->description, description ? description : "",
            TECH_DEBT_MAX_DESC_LENGTH - 1);
    step->description[TECH_DEBT_MAX_DESC_LENGTH - 1] = '\0';
    step->estimated_effort_hours = estimated_effort_hours;
    step->interest_reduction_percent = interest_reduction_percent;
    step->target_date = target_date;
    step->is_completed = false;
    item->payback_step_count++;
    return true;
}

bool tech_debt_item_acknowledge(TechDebtRegister *register_ptr,
                                 unsigned int item_id)
{
    TechDebtItem *item = tech_debt_register_find(register_ptr, item_id);
    if (!item || item->status != TECH_DEBT_STATUS_IDENTIFIED) return false;
    item->status = TECH_DEBT_STATUS_ACKNOWLEDGED;
    return true;
}

bool tech_debt_item_quantify(TechDebtRegister *register_ptr,
                              unsigned int item_id)
{
    TechDebtItem *item = tech_debt_register_find(register_ptr, item_id);
    if (!item || item->status != TECH_DEBT_STATUS_ACKNOWLEDGED) return false;
    item->status = TECH_DEBT_STATUS_QUANTIFIED;
    return true;
}

bool tech_debt_item_plan(TechDebtRegister *register_ptr,
                          unsigned int item_id)
{
    TechDebtItem *item = tech_debt_register_find(register_ptr, item_id);
    if (!item || item->status != TECH_DEBT_STATUS_QUANTIFIED) return false;
    item->status = TECH_DEBT_STATUS_PLANNED;
    return true;
}

bool tech_debt_item_start_repayment(TechDebtRegister *register_ptr,
                                     unsigned int item_id)
{
    TechDebtItem *item = tech_debt_register_find(register_ptr, item_id);
    if (!item || item->status != TECH_DEBT_STATUS_PLANNED) return false;
    item->status = TECH_DEBT_STATUS_IN_REPAYMENT;
    return true;
}

bool tech_debt_item_resolve(TechDebtRegister *register_ptr,
                             unsigned int item_id)
{
    TechDebtItem *item = tech_debt_register_find(register_ptr, item_id);
    if (!item) return false;
    item->status = TECH_DEBT_STATUS_RESOLVED;
    item->actual_resolution_date = time(NULL);
    return true;
}

bool tech_debt_item_accept(TechDebtRegister *register_ptr,
                            unsigned int item_id)
{
    TechDebtItem *item = tech_debt_register_find(register_ptr, item_id);
    if (!item) return false;
    item->status = TECH_DEBT_STATUS_ACCEPTED;
    return true;
}

double tech_debt_calculate_accumulated_interest(const TechDebtItem *item,
                                                 time_t current_date)
{
    if (!item || item->interest_rate_percent <= 0) return 0.0;
    double elapsed_seconds = difftime(current_date, item->identified_date);
    double years = elapsed_seconds / SECONDS_IN_YEAR;
    if (years <= 0) return 0.0;
    double interest = item->principal_hours
                      * item->interest_rate_percent / 100.0
                      * years;
    return interest;
}

double tech_debt_register_total_principal(
    const TechDebtRegister *register_ptr)
{
    if (!register_ptr) return 0.0;
    double total = 0.0;
    for (size_t i = 0; i < register_ptr->count; i++) {
        if (register_ptr->items[i].status != TECH_DEBT_STATUS_RESOLVED) {
            total += register_ptr->items[i].principal_hours;
        }
    }
    return total;
}

double tech_debt_register_total_interest(
    const TechDebtRegister *register_ptr, time_t current_date)
{
    if (!register_ptr) return 0.0;
    double total = 0.0;
    for (size_t i = 0; i < register_ptr->count; i++) {
        if (register_ptr->items[i].status != TECH_DEBT_STATUS_RESOLVED) {
            total += tech_debt_calculate_accumulated_interest(
                &register_ptr->items[i], current_date);
        }
    }
    return total;
}

double tech_debt_register_roi_payback(
    const TechDebtRegister *register_ptr, unsigned int item_id)
{
    TechDebtItem *item = tech_debt_register_find(
        (TechDebtRegister *)register_ptr, item_id);
    if (!item) return 0.0;
    double total_effort = 0.0;
    for (size_t i = 0; i < item->payback_step_count; i++) {
        total_effort += item->payback_plan[i].estimated_effort_hours;
    }
    if (total_effort <= 0) return 0.0;
    double annual_interest = item->principal_hours
                             * item->interest_rate_percent / 100.0;
    return annual_interest / total_effort;
}

double tech_debt_register_weighted_interest_rate(
    const TechDebtRegister *register_ptr)
{
    if (!register_ptr) return 0.0;
    double total_principal = tech_debt_register_total_principal(register_ptr);
    if (total_principal <= 0) return 0.0;
    double weighted_sum = 0.0;
    for (size_t i = 0; i < register_ptr->count; i++) {
        if (register_ptr->items[i].status != TECH_DEBT_STATUS_RESOLVED) {
            weighted_sum += register_ptr->items[i].principal_hours
                            * register_ptr->items[i].interest_rate_percent;
        }
    }
    return weighted_sum / total_principal;
}

TechDebtItem *tech_debt_register_find(TechDebtRegister *register_ptr,
                                       unsigned int id)
{
    if (!register_ptr) return NULL;
    for (size_t i = 0; i < register_ptr->count; i++) {
        if (register_ptr->items[i].id == id) return &register_ptr->items[i];
    }
    return NULL;
}

TechDebtItem *tech_debt_register_find_by_status(
    TechDebtRegister *register_ptr, TechDebtStatus status, size_t *count)
{
    if (!register_ptr) return NULL;
    size_t found = 0;
    for (size_t i = 0; i < register_ptr->count; i++) {
        if (register_ptr->items[i].status == status) found++;
    }
    if (count) *count = found;
    return (found > 0) ? register_ptr->items : NULL;
}

const char *tech_debt_quadrant_to_string(TechDebtQuadrant quadrant)
{
    switch (quadrant) {
    case TECH_DEBT_QUADRANT_DELIBERATE_PRUDENT:
        return "Deliberate & Prudent";
    case TECH_DEBT_QUADRANT_DELIBERATE_RECKLESS:
        return "Deliberate & Reckless";
    case TECH_DEBT_QUADRANT_INADVERTENT_PRUDENT:
        return "Inadvertent & Prudent";
    case TECH_DEBT_QUADRANT_INADVERTENT_RECKLESS:
        return "Inadvertent & Reckless";
    default:
        return "Unknown";
    }
}

const char *tech_debt_status_to_string(TechDebtStatus status)
{
    switch (status) {
    case TECH_DEBT_STATUS_IDENTIFIED:   return "Identified";
    case TECH_DEBT_STATUS_ACKNOWLEDGED: return "Acknowledged";
    case TECH_DEBT_STATUS_QUANTIFIED:   return "Quantified";
    case TECH_DEBT_STATUS_PLANNED:      return "Planned";
    case TECH_DEBT_STATUS_IN_REPAYMENT: return "In Repayment";
    case TECH_DEBT_STATUS_RESOLVED:     return "Resolved";
    case TECH_DEBT_STATUS_ACCEPTED:     return "Accepted";
    case TECH_DEBT_STATUS_WONT_FIX:     return "Won't Fix";
    default:                            return "Unknown";
    }
}

const char *code_smell_type_to_string(CodeSmellType type)
{
    switch (type) {
    case CODE_SMELL_DUPLICATED_CODE:      return "Duplicated Code";
    case CODE_SMELL_LONG_METHOD:          return "Long Method";
    case CODE_SMELL_LARGE_CLASS:          return "Large Class";
    case CODE_SMELL_LONG_PARAMETER_LIST:  return "Long Parameter List";
    case CODE_SMELL_DIVERGENT_CHANGE:     return "Divergent Change";
    case CODE_SMELL_SHOTGUN_SURGERY:      return "Shotgun Surgery";
    case CODE_SMELL_FEATURE_ENVY:         return "Feature Envy";
    case CODE_SMELL_DATA_CLUMPS:          return "Data Clumps";
    case CODE_SMELL_PRIMITIVE_OBSESSION:  return "Primitive Obsession";
    case CODE_SMELL_SWITCH_STATEMENTS:    return "Switch Statements";
    case CODE_SMELL_PARALLEL_INHERITANCE: return "Parallel Inheritance";
    case CODE_SMELL_LAZY_CLASS:           return "Lazy Class";
    case CODE_SMELL_SPECULATIVE_GENERALITY: return "Speculative Generality";
    case CODE_SMELL_TEMPORARY_FIELD:      return "Temporary Field";
    case CODE_SMELL_MESSAGE_CHAINS:       return "Message Chains";
    case CODE_SMELL_MIDDLE_MAN:           return "Middle Man";
    case CODE_SMELL_INAPPROPRIATE_INTIMACY: return "Inappropriate Intimacy";
    case CODE_SMELL_ALTERNATIVE_CLASSES:  return "Alternative Classes";
    case CODE_SMELL_INCOMPLETE_LIBRARY:   return "Incomplete Library";
    case CODE_SMELL_DATA_CLASS:           return "Data Class";
    case CODE_SMELL_REFUSED_BEQUEST:      return "Refused Bequest";
    case CODE_SMELL_COMMENTS_CODE_SMELL:  return "Comments";
    default:                              return "Unknown";
    }
}

bool tech_debt_item_is_deliberate(const TechDebtItem *item)
{
    if (!item) return false;
    return item->quadrant == TECH_DEBT_QUADRANT_DELIBERATE_PRUDENT
        || item->quadrant == TECH_DEBT_QUADRANT_DELIBERATE_RECKLESS;
}

bool tech_debt_item_is_prudent(const TechDebtItem *item)
{
    if (!item) return false;
    return item->quadrant == TECH_DEBT_QUADRANT_DELIBERATE_PRUDENT
        || item->quadrant == TECH_DEBT_QUADRANT_INADVERTENT_PRUDENT;
}

void tech_debt_register_print_summary(
    const TechDebtRegister *register_ptr)
{
    if (!register_ptr) return;
    printf("==================================================\n");
    printf("Technical Debt Register: %s\n", register_ptr->project_name);
    printf("==================================================\n");
    printf("Total items: %zu\n", register_ptr->count);
    printf("Total principal: %.1f hours\n",
           tech_debt_register_total_principal(register_ptr));
    time_t now = time(NULL);
    printf("Total accumulated interest: %.1f hours\n",
           tech_debt_register_total_interest(register_ptr, now));
    printf("Weighted interest rate: %.2f%%\n",
           tech_debt_register_weighted_interest_rate(register_ptr));
    printf("\n--- Quadrant Breakdown ---\n");
    for (int q = 0; q < 4; q++) {
        size_t count = 0;
        double principal = 0.0;
        for (size_t i = 0; i < register_ptr->count; i++) {
            if ((int)register_ptr->items[i].quadrant == q) {
                count++;
                principal += register_ptr->items[i].principal_hours;
            }
        }
        printf("  %s: %zu items, %.1f hrs\n",
               tech_debt_quadrant_to_string((TechDebtQuadrant)q),
               count, principal);
    }
    printf("\n--- Items ---\n");
    for (size_t i = 0; i < register_ptr->count; i++) {
        printf("  [%04u] %s | %s | %.1f hrs @ %.1f%%\n",
               register_ptr->items[i].id,
               register_ptr->items[i].name,
               tech_debt_status_to_string(register_ptr->items[i].status),
               register_ptr->items[i].principal_hours,
               register_ptr->items[i].interest_rate_percent);
    }
    printf("==================================================\n");
}

void tech_debt_register_print_item(const TechDebtItem *item)
{
    if (!item) return;
    printf("--- Tech Debt Item #%04u ---\n", item->id);
    printf("Name: %s\n", item->name);
    printf("Quadrant: %s\n", tech_debt_quadrant_to_string(item->quadrant));
    printf("Status: %s\n", tech_debt_status_to_string(item->status));
    printf("Principal: %.1f hours\n", item->principal_hours);
    printf("Interest Rate: %.1f%%\n", item->interest_rate_percent);
    printf("Annual Interest: %.1f hours\n", item->annual_interest_hours);
    printf("Accumulated Interest: %.1f hours\n",
           item->current_accumulated_interest);
    printf("Description: %s\n", item->description);
    if (item->code_smell_count > 0) {
        printf("Code Smells:\n");
        for (size_t i = 0; i < item->code_smell_count; i++) {
            printf("  - %s (severity: %.1f) @ %s:%u-%u\n",
                   item->code_smells[i].name,
                   item->code_smells[i].severity,
                   item->code_smells[i].location.file_path,
                   item->code_smells[i].location.line_start,
                   item->code_smells[i].location.line_end);
        }
    }
    if (item->payback_step_count > 0) {
        printf("Payback Plan:\n");
        for (size_t i = 0; i < item->payback_step_count; i++) {
            printf("  %zu. %s (%.1f hrs, -%.0f%% interest)%s\n",
                   i + 1,
                   item->payback_plan[i].description,
                   item->payback_plan[i].estimated_effort_hours,
                   item->payback_plan[i].interest_reduction_percent,
                   item->payback_plan[i].is_completed ? " [DONE]" : "");
        }
    }
    printf("-------------------------------\n");
}

bool tech_debt_register_export_csv(
    const TechDebtRegister *register_ptr, const char *filename)
{
    if (!register_ptr || !filename) return false;
    FILE *fp = fopen(filename, "w");
    if (!fp) return false;
    fprintf(fp, "ID,Name,Quadrant,Status,PrincipalHours,"
            "InterestRate,AnnualInterest,AccumulatedInterest,Owner\n");
    for (size_t i = 0; i < register_ptr->count; i++) {
        fprintf(fp, "%u,%s,%s,%s,%.1f,%.1f,%.1f,%.1f,%s\n",
                register_ptr->items[i].id,
                register_ptr->items[i].name,
                tech_debt_quadrant_to_string(register_ptr->items[i].quadrant),
                tech_debt_status_to_string(register_ptr->items[i].status),
                register_ptr->items[i].principal_hours,
                register_ptr->items[i].interest_rate_percent,
                register_ptr->items[i].annual_interest_hours,
                register_ptr->items[i].current_accumulated_interest,
                register_ptr->items[i].owner);
    }
    fclose(fp);
    return true;
}

void tech_debt_register_update_interest(TechDebtRegister *register_ptr,
                                         time_t current_date)
{
    if (!register_ptr) return;
    for (size_t i = 0; i < register_ptr->count; i++) {
        if (register_ptr->items[i].status != TECH_DEBT_STATUS_RESOLVED) {
            register_ptr->items[i].current_accumulated_interest =
                tech_debt_calculate_accumulated_interest(
                    &register_ptr->items[i], current_date);
        }
    }
    register_ptr->last_updated = current_date;
}
