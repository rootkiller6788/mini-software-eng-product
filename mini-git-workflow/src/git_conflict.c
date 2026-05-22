#include "git_conflict.h"
#include <stdio.h>
#include <string.h>

void conflict_detect(const char *ours, const char *theirs, const char *base, Conflict *c) {
    memset(c,0,sizeof(*c));
    if (strcmp(ours,theirs)==0) { c->type=CONF_NONE; return; }
    if (strcmp(ours,base)==0) { strncpy(c->ours_content,theirs,511); return; }
    if (strcmp(theirs,base)==0) { strncpy(c->ours_content,ours,511); return; }
    c->type=CONF_CONTENT; strncpy(c->ours_content,ours,511); strncpy(c->theirs_content,theirs,511);
}

int conflict_resolve_ours(Conflict *c, char *resolved, int sz) { snprintf(resolved,sz,"%s",c->ours_content); return 0; }
int conflict_resolve_theirs(Conflict *c, char *resolved, int sz) { snprintf(resolved,sz,"%s",c->theirs_content); return 0; }
int conflict_resolve_union(Conflict *c, char *resolved, int sz) { snprintf(resolved,sz,"%s\n%s",c->ours_content,c->theirs_content); return 0; }

void conflict_analyze(ConflictSet *cs) { cs->has_conflicts=false; for (int i=0; i<cs->conflict_count; i++) if (cs->conflicts[i].type!=CONF_NONE) { cs->has_conflicts=true; return; } }

void conflict_print(ConflictSet *cs) {
    printf("=== Conflicts: %d ===\n", cs->conflict_count);
    for (int i=0; i<cs->conflict_count; i++) if (cs->conflicts[i].type!=CONF_NONE) printf("  %s: %s\n", cs->conflicts[i].file_path, "CONTENT conflict");
    printf("  Has conflicts: %s\n", cs->has_conflicts?"YES":"NO");
}
