#ifndef PROJECT_COMPLIANCE_H
#define PROJECT_COMPLIANCE_H
#include <stdbool.h>

#define MAX_GATES 8
#define MAX_CHECKS_PER_GATE 16
#define MAX_AUDIT_ENTRIES 128

typedef enum { GSTAT_PENDING, GSTAT_IN_REVIEW, GSTAT_PASSED, GSTAT_FAILED } GateStatus;

typedef struct {
    char description[256];
    bool required; /* is this check mandatory? */
    bool passed; char evidence[256];
} ComplianceCheck;

typedef struct {
    char name[64]; char description[256];
    ComplianceCheck checks[MAX_CHECKS_PER_GATE]; int check_count;
    GateStatus status;
    char reviewer[64]; char review_date[16];
} StageGate;

typedef struct {
    char timestamp[24]; char action[128]; char by[64];
    char before_state[128]; char after_state[128];
} AuditEntry;

typedef struct {
    StageGate gates[MAX_GATES]; int gate_count;
    AuditEntry audit_trail[MAX_AUDIT_ENTRIES]; int audit_count;
    char standard[64]; /* e.g. ISO 9001, SOX, SOC2, GDPR */
} CompliancePlan;

void compliance_init(CompliancePlan *cp, const char *standard);
int  compliance_add_gate(CompliancePlan *cp, const char *name, const char *desc);
int  compliance_add_check(CompliancePlan *cp, int gate_idx, const char *desc, bool required);
void compliance_pass_check(CompliancePlan *cp, int gate_idx, int check_idx, const char *evidence);
bool compliance_gate_ready(CompliancePlan *cp, int gate_idx); /* all required checks pass */
void compliance_review_gate(CompliancePlan *cp, int gate_idx, const char *reviewer);
void compliance_audit_log(CompliancePlan *cp, const char *action, const char *by, const char *before, const char *after);
bool compliance_all_gates_passed(CompliancePlan *cp);
void compliance_print_gate(CompliancePlan *cp, int gate_idx);
void compliance_print_all(CompliancePlan *cp);
#endif
