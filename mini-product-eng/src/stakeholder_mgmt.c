#include "stakeholder_mgmt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void stakeholder_init(Stakeholder *s, const char *name, const char *title, double power, double interest) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    if (name) {
        strncpy(s->name, name, sizeof(s->name) - 1);
        s->name[sizeof(s->name) - 1] = '\0';
    }
    if (title) {
        strncpy(s->title, title, sizeof(s->title) - 1);
        s->title[sizeof(s->title) - 1] = '\0';
    }
    s->power = power;
    s->interest = interest;
    stakeholder_set_strategy(s);
}

const char *stakeholder_quadrant(double power, double interest) {
    if (power >= 0.5 && interest >= 0.5) return "Key Players";
    if (power >= 0.5 && interest < 0.5)  return "Meet Needs";
    if (power < 0.5 && interest >= 0.5)  return "Show Consideration";
    return "Monitor";
}

void stakeholder_set_strategy(Stakeholder *s) {
    if (!s) return;
    const char *quadrant = stakeholder_quadrant(s->power, s->interest);
    strncpy(s->quadrant, quadrant, sizeof(s->quadrant) - 1);
    s->quadrant[sizeof(s->quadrant) - 1] = '\0';

    if (strcmp(quadrant, "Key Players") == 0) {
        strncpy(s->engagement_strategy, "Manage closely — active collaboration, regular 1:1s, early input",
                sizeof(s->engagement_strategy) - 1);
    } else if (strcmp(quadrant, "Meet Needs") == 0) {
        strncpy(s->engagement_strategy, "Keep satisfied — formal updates, address concerns promptly",
                sizeof(s->engagement_strategy) - 1);
    } else if (strcmp(quadrant, "Show Consideration") == 0) {
        strncpy(s->engagement_strategy, "Keep informed — regular newsletters, demos, feedback channels",
                sizeof(s->engagement_strategy) - 1);
    } else {
        strncpy(s->engagement_strategy, "Monitor — minimal effort, periodic check-ins",
                sizeof(s->engagement_strategy) - 1);
    }
    s->engagement_strategy[sizeof(s->engagement_strategy) - 1] = '\0';
}

void stakeholder_map_print(Stakeholder *stakeholders, size_t count, char *buffer, size_t buf_size) {
    if (!stakeholders || !buffer || buf_size == 0) return;
    int w = 0;
    w += snprintf(buffer + w, buf_size - w,
        "┌──────────────────┬──────────────────┐\n"
        "│  Meet Needs      │  Key Players     │\n"
        "│  (High Pwr, Low  │  (High Pwr, High │\n"
        "│   Interest)      │   Interest)      │\n"
        "├──────────────────┼──────────────────┤\n");
    char q1[512] = {0}, q2[512] = {0}, q3[512] = {0}, q4[512] = {0};
    for (size_t i = 0; i < count; i++) {
        const char *q = stakeholder_quadrant(stakeholders[i].power, stakeholders[i].interest);
        char line[128];
        snprintf(line, sizeof(line), "  %s\n", stakeholders[i].name);
        if (strcmp(q, "Key Players") == 0) {
            strncat(q2, line, sizeof(q2) - strlen(q2) - 1);
        } else if (strcmp(q, "Meet Needs") == 0) {
            strncat(q1, line, sizeof(q1) - strlen(q1) - 1);
        } else if (strcmp(q, "Show Consideration") == 0) {
            strncat(q3, line, sizeof(q3) - strlen(q3) - 1);
        } else {
            strncat(q4, line, sizeof(q4) - strlen(q4) - 1);
        }
    }
    w += snprintf(buffer + w, buf_size - w, "│%s│%s│\n", q1, q2);
    w += snprintf(buffer + w, buf_size - w,
        "├──────────────────┼──────────────────┤\n"
        "│  Monitor         │  Show Consid.    │\n"
        "│  (Low Pwr, Low   │  (Low Pwr, High  │\n"
        "│   Interest)      │   Interest)      │\n"
        "├──────────────────┼──────────────────┤\n");
    w += snprintf(buffer + w, buf_size - w, "│%s│%s│\n", q4, q3);
    w += snprintf(buffer + w, buf_size - w, "└──────────────────┴──────────────────┘\n");
}

void raci_matrix_init(RaciMatrix *rm) {
    if (!rm) return;
    memset(rm, 0, sizeof(*rm));
}

void raci_add_item(RaciMatrix *rm, const char *task, const char *responsible, const char *accountable) {
    if (!rm || !task || !responsible || !accountable) return;
    rm->item_count++;
    RacItem *tmp = realloc(rm->items, rm->item_count * sizeof(RacItem));
    if (!tmp) return;
    rm->items = tmp;
    RacItem *item = &rm->items[rm->item_count - 1];
    memset(item, 0, sizeof(*item));
    strncpy(item->task, task, sizeof(item->task) - 1);
    item->task[sizeof(item->task) - 1] = '\0';
    strncpy(item->responsible, responsible, sizeof(item->responsible) - 1);
    item->responsible[sizeof(item->responsible) - 1] = '\0';
    strncpy(item->accountable, accountable, sizeof(item->accountable) - 1);
    item->accountable[sizeof(item->accountable) - 1] = '\0';
}

void raci_add_consulted(RaciMatrix *rm, size_t index, const char *person) {
    if (!rm || !person || index >= rm->item_count || rm->items[index].consulted_count >= 10) return;
    strncpy(rm->items[index].consulted[rm->items[index].consulted_count], person,
            sizeof(rm->items[index].consulted[0]) - 1);
    rm->items[index].consulted[rm->items[index].consulted_count]
        [sizeof(rm->items[index].consulted[0]) - 1] = '\0';
    rm->items[index].consulted_count++;
}

void raci_add_informed(RaciMatrix *rm, size_t index, const char *person) {
    if (!rm || !person || index >= rm->item_count || rm->items[index].informed_count >= 10) return;
    strncpy(rm->items[index].informed[rm->items[index].informed_count], person,
            sizeof(rm->items[index].informed[0]) - 1);
    rm->items[index].informed[rm->items[index].informed_count]
        [sizeof(rm->items[index].informed[0]) - 1] = '\0';
    rm->items[index].informed_count++;
}

bool raci_has_gaps(const RaciMatrix *rm, char *buffer, size_t buf_size) {
    if (!rm) return false;
    int gaps = 0;
    int w = 0;
    for (size_t i = 0; i < rm->item_count; i++) {
        if (rm->items[i].consulted_count == 0 && rm->items[i].informed_count == 0) {
            w += snprintf(buffer + w, buf_size - w,
                "Gap on '%s': no C or I assigned\n", rm->items[i].task);
            gaps++;
        }
    }
    return gaps > 0;
}

void raci_free(RaciMatrix *rm) {
    if (!rm) return;
    free(rm->items);
    memset(rm, 0, sizeof(*rm));
}

void comm_plan_init(CommunicationPlan *cp, const char *stakeholder, const char *channel, const char *freq) {
    if (!cp) return;
    memset(cp, 0, sizeof(*cp));
    if (stakeholder) {
        strncpy(cp->stakeholder_name, stakeholder, sizeof(cp->stakeholder_name) - 1);
        cp->stakeholder_name[sizeof(cp->stakeholder_name) - 1] = '\0';
    }
    if (channel) {
        strncpy(cp->channel, channel, sizeof(cp->channel) - 1);
        cp->channel[sizeof(cp->channel) - 1] = '\0';
    }
    if (freq) {
        strncpy(cp->frequency, freq, sizeof(cp->frequency) - 1);
        cp->frequency[sizeof(cp->frequency) - 1] = '\0';
    }
}

void comm_plan_generate(Stakeholder *stakeholders, size_t count, CommunicationPlan *plans) {
    if (!stakeholders || !plans) return;
    for (size_t i = 0; i < count; i++) {
        Stakeholder *s = &stakeholders[i];
        comm_plan_init(&plans[i], s->name, "", "");
        const char *q = stakeholder_quadrant(s->power, s->interest);
        if (strcmp(q, "Key Players") == 0) {
            strncpy(plans[i].channel, "Weekly 1:1", sizeof(plans[i].channel) - 1);
            strncpy(plans[i].frequency, "Weekly", sizeof(plans[i].frequency) - 1);
            strncpy(plans[i].content_summary, "Detailed status, decisions needed, roadmap review",
                    sizeof(plans[i].content_summary) - 1);
            strncpy(plans[i].owner, "Product Manager", sizeof(plans[i].owner) - 1);
        } else if (strcmp(q, "Meet Needs") == 0) {
            strncpy(plans[i].channel, "Email + Monthly", sizeof(plans[i].channel) - 1);
            strncpy(plans[i].frequency, "Monthly", sizeof(plans[i].frequency) - 1);
            strncpy(plans[i].content_summary, "High-level progress, risks, upcoming milestones",
                    sizeof(plans[i].content_summary) - 1);
            strncpy(plans[i].owner, "Product Manager", sizeof(plans[i].owner) - 1);
        } else if (strcmp(q, "Show Consideration") == 0) {
            strncpy(plans[i].channel, "Newsletter", sizeof(plans[i].channel) - 1);
            strncpy(plans[i].frequency, "Bi-weekly", sizeof(plans[i].frequency) - 1);
            strncpy(plans[i].content_summary, "Feature highlights, tips, community updates",
                    sizeof(plans[i].content_summary) - 1);
            strncpy(plans[i].owner, "Product Marketing", sizeof(plans[i].owner) - 1);
        } else {
            strncpy(plans[i].channel, "Wiki page", sizeof(plans[i].channel) - 1);
            strncpy(plans[i].frequency, "Quarterly", sizeof(plans[i].frequency) - 1);
            strncpy(plans[i].content_summary, "Release notes, roadmap summary",
                    sizeof(plans[i].content_summary) - 1);
            strncpy(plans[i].owner, "Engineering Manager", sizeof(plans[i].owner) - 1);
        }
    }
}

void comm_plan_print(const CommunicationPlan *plans, size_t count) {
    if (!plans) return;
    for (size_t i = 0; i < count; i++) {
        printf("%-20s channel=%-12s freq=%-10s owner=%s\n",
               plans[i].stakeholder_name, plans[i].channel, plans[i].frequency, plans[i].owner);
    }
}

void escalation_init(Escalation *e, const char *issue, int severity) {
    if (!e) return;
    memset(e, 0, sizeof(*e));
    if (issue) {
        strncpy(e->issue, issue, sizeof(e->issue) - 1);
        e->issue[sizeof(e->issue) - 1] = '\0';
    }
    e->severity = severity;
}

bool escalation_resolve(Escalation *e, const char *decision) {
    if (!e || !decision) return false;
    strncpy(e->escalation_path, decision, sizeof(e->escalation_path) - 1);
    e->escalation_path[sizeof(e->escalation_path) - 1] = '\0';
    e->resolved = true;
    return true;
}

bool escalation_requires_attention(const Escalation *e, int threshold) {
    if (!e) return false;
    return !e->resolved && e->severity >= threshold;
}

void escalation_print(const Escalation *e) {
    if (!e) return;
    printf("[Sev %d] %s\n  Resolved: %s\n  Path: %s\n",
           e->severity, e->issue,
           e->resolved ? "Yes" : "No",
           e->resolved ? e->escalation_path : "PENDING");
}
