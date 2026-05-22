#include "project_decision.h"
#include <stdio.h>
#include <string.h>

void decision_log_init(DecisionLog *dl) { memset(dl, 0, sizeof(*dl)); }

int decision_add(DecisionLog *dl, const char *id, const char *title, const char *context) {
    if (dl->decision_count >= MAX_DECISIONS) return -1;
    Decision *d = &dl->decisions[dl->decision_count];
    strncpy(d->id, id, 15); d->id[15] = '\0';
    strncpy(d->title, title, 127); d->title[127] = '\0';
    strncpy(d->context, context, 511); d->context[511] = '\0';
    d->option_count = 0; d->chosen_option = -1; d->status = DSTAT_PROPOSED;
    d->decided_by[0] = '\0'; d->decided_date[0] = '\0'; d->consequences[0] = '\0';
    return dl->decision_count++;
}

int decision_add_option(DecisionLog *dl, int dec_idx, const char *desc, const char *pros, const char *cons) {
    if (dec_idx < 0 || dec_idx >= dl->decision_count) return -1;
    Decision *d = &dl->decisions[dec_idx];
    if (d->option_count >= MAX_OPTIONS) return -1;
    DecisionOption *o = &d->options[d->option_count];
    strncpy(o->description, desc, 255); o->description[255] = '\0';
    strncpy(o->pros, pros, 255); o->pros[255] = '\0';
    strncpy(o->cons, cons, 255); o->cons[255] = '\0';
    return d->option_count++;
}

void decision_make(DecisionLog *dl, int dec_idx, int option, const char *by, const char *date) {
    if (dec_idx < 0 || dec_idx >= dl->decision_count) return;
    Decision *d = &dl->decisions[dec_idx];
    if (option < 0 || option >= d->option_count) return;
    d->chosen_option = option; d->status = DSTAT_ACCEPTED;
    strncpy(d->decided_by, by, 63); d->decided_by[63] = '\0';
    strncpy(d->decided_date, date, 15); d->decided_date[15] = '\0';
}

int decision_pending_count(DecisionLog *dl) {
    int count = 0;
    for (int i = 0; i < dl->decision_count; i++)
        if (dl->decisions[i].chosen_option == -1) count++;
    return count;
}

void decision_print(DecisionLog *dl, int dec_idx) {
    if (dec_idx < 0 || dec_idx >= dl->decision_count) return;
    Decision *d = &dl->decisions[dec_idx];
    const char *ss[] = {"PROPOSED","ACCEPTED","DEPRECATED","SUPERSEDED"};
    printf("=== Decision %s: %s [%s] ===\n", d->id, d->title, ss[d->status]);
    printf("  Context: %s\n", d->context);
    for (int i = 0; i < d->option_count; i++) {
        printf("  Option %d: %s%s\n", i+1, d->options[i].description,
               i == d->chosen_option ? " *** CHOSEN ***" : "");
        printf("    Pros: %s\n    Cons: %s\n", d->options[i].pros, d->options[i].cons);
    }
    if (d->chosen_option >= 0) printf("  Decided by: %s on %s\n", d->decided_by, d->decided_date);
}

void decision_print_all(DecisionLog *dl) {
    printf("=== Decision Log: %d decisions (%d pending) ===\n", dl->decision_count, decision_pending_count(dl));
    for (int i = 0; i < dl->decision_count; i++)
        printf("  %s: %s [%s]\n", dl->decisions[i].id, dl->decisions[i].title,
               dl->decisions[i].chosen_option >= 0 ? "DECIDED" : "PENDING");
}
