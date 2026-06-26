#ifndef STAKEHOLDER_MGMT_H
#define STAKEHOLDER_MGMT_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    RACI_RESPONSIBLE,
    RACI_ACCOUNTABLE,
    RACI_CONSULTED,
    RACI_INFORMED
} RaciRole;

typedef struct {
    char name[128];
    char title[128];
    double power;
    double interest;
    char quadrant[32];
    char engagement_strategy[256];
} Stakeholder;

typedef struct {
    char task[256];
    char responsible[128];
    char accountable[128];
    char consulted[10][128];
    int consulted_count;
    char informed[10][128];
    int informed_count;
} RacItem;

typedef struct {
    RacItem *items;
    size_t item_count;
} RaciMatrix;

typedef struct {
    char stakeholder_name[128];
    char channel[64];
    char frequency[64];
    char content_summary[256];
    char owner[128];
} CommunicationPlan;

typedef struct {
    char issue[256];
    int severity;
    char escalation_path[512];
    char decision_maker[128];
    char deadline[32];
    bool resolved;
} Escalation;

void        stakeholder_init(Stakeholder *s, const char *name, const char *title, double power, double interest);
const char *stakeholder_quadrant(double power, double interest);
void        stakeholder_set_strategy(Stakeholder *s);
void        stakeholder_map_print(Stakeholder *stakeholders, size_t count, char *buffer, size_t buf_size);

void        raci_matrix_init(RaciMatrix *rm);
void        raci_add_item(RaciMatrix *rm, const char *task, const char *responsible, const char *accountable);
void        raci_add_consulted(RaciMatrix *rm, size_t index, const char *person);
void        raci_add_informed(RaciMatrix *rm, size_t index, const char *person);
bool        raci_has_gaps(const RaciMatrix *rm, char *buffer, size_t buf_size);
void        raci_free(RaciMatrix *rm);

void        comm_plan_init(CommunicationPlan *cp, const char *stakeholder, const char *channel, const char *freq);
void        comm_plan_generate(Stakeholder *stakeholders, size_t count, CommunicationPlan *plans);
void        comm_plan_print(const CommunicationPlan *plans, size_t count);

void        escalation_init(Escalation *e, const char *issue, int severity);
bool        escalation_resolve(Escalation *e, const char *decision);
bool        escalation_requires_attention(const Escalation *e, int threshold);
void        escalation_print(const Escalation *e);

#endif
