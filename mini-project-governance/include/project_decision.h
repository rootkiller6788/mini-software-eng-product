#ifndef PROJECT_DECISION_H
#define PROJECT_DECISION_H
#include <stdbool.h>

#define MAX_DECISIONS 64
#define MAX_OPTIONS 5

typedef enum { DSTAT_PROPOSED, DSTAT_ACCEPTED, DSTAT_DEPRECATED, DSTAT_SUPERSEDED } DecisionStatus;

typedef struct { char description[256]; char pros[256]; char cons[256]; } DecisionOption;

typedef struct {
    char id[16]; char title[128]; char context[512];
    DecisionOption options[MAX_OPTIONS]; int option_count;
    int chosen_option; /* -1 if undecided */
    DecisionStatus status;
    char decided_by[64]; char decided_date[16];
    char consequences[256]; /* what happened as a result */
} Decision;

typedef struct {
    Decision decisions[MAX_DECISIONS]; int decision_count;
} DecisionLog;

void decision_log_init(DecisionLog *dl);
int  decision_add(DecisionLog *dl, const char *id, const char *title, const char *context);
int  decision_add_option(DecisionLog *dl, int dec_idx, const char *desc, const char *pros, const char *cons);
void decision_make(DecisionLog *dl, int dec_idx, int option, const char *by, const char *date);
int  decision_pending_count(DecisionLog *dl); /* decisions with chosen_option == -1 */
void decision_print(DecisionLog *dl, int dec_idx);
void decision_print_all(DecisionLog *dl);
#endif
