#include "biz_crud.h"
#include <stdio.h>
#include <string.h>

void crud_init(CrudRepo *repo, const char *entity_name) { memset(repo, 0, sizeof(*repo)); entity_def_init(&repo->def, entity_name); repo->next_id = 1; }

int crud_create(CrudRepo *repo) {
    int id;
    if (repo->deleted_count > 0) { id = repo->deleted[--repo->deleted_count]; } else { if (repo->count >= CRUD_MAX_RECORDS) return -1; id = repo->next_id++; }
    entity_inst_init(&repo->instances[id - 1], &repo->def, id);
    if (id > repo->count) repo->count = id;
    return id;
}

bool crud_read(CrudRepo *repo, int id, EntityInst **out) { if (id < 1 || id > repo->count) return false; *out = &repo->instances[id-1]; return true; }

bool crud_update(CrudRepo *repo, int id, int fi, const char *value) { EntityInst *e; if (!crud_read(repo, id, &e)) return false; return entity_set_field(e, fi, value); }

bool crud_delete(CrudRepo *repo, int id) {
    if (id < 1 || id > repo->count) return false;
    if (repo->deleted_count < CRUD_MAX_RECORDS) repo->deleted[repo->deleted_count++] = id;
    memset(&repo->instances[id-1], 0, sizeof(EntityInst));
    return true;
}

int crud_find(CrudRepo *repo, const char *field_name, const char *value, int *results, int max) {
    int fi = entity_def_get_field(&repo->def, field_name); if (fi < 0) return 0;
    int found = 0;
    for (int i = 1; i <= repo->count && found < max; i++) { const char *fv = entity_get_field(&repo->instances[i-1], fi); if (fv && strcmp(fv, value) == 0) results[found++] = i; }
    return found;
}

int crud_count(CrudRepo *repo) { return repo->count - repo->deleted_count; }

void crud_print_all(CrudRepo *repo) {
    printf("=== %s (%d records) ===\n", repo->def.name, crud_count(repo));
    for (int i = 1; i <= repo->count; i++) { EntityInst *e; if (crud_read(repo, i, &e) && e->def) entity_print(e); }
}
