#ifndef GIT_CONFLICT_H
#define GIT_CONFLICT_H
#include <stdbool.h>

typedef enum { CONF_NONE, CONF_CONTENT, CONF_RENAME, CONF_DELETE_MODIFY, CONF_BINARY } ConflictType;

typedef struct {
    ConflictType type; char file_path[128];
    char ours_content[512]; char theirs_content[512];
    int conflict_marker_start; int conflict_marker_end;
} Conflict;

typedef struct { Conflict conflicts[32]; int conflict_count; bool has_conflicts; } ConflictSet;

void conflict_detect(const char *ours, const char *theirs, const char *base, Conflict *c);
int  conflict_resolve_ours(Conflict *c, char *resolved, int sz);
int  conflict_resolve_theirs(Conflict *c, char *resolved, int sz);
int  conflict_resolve_union(Conflict *c, char *resolved, int sz);
void conflict_analyze(ConflictSet *cs);
void conflict_print(ConflictSet *cs);
#endif
