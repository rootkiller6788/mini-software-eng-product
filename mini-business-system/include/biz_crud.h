#ifndef BIZ_CRUD_H
#define BIZ_CRUD_H
#include "biz_entity.h"
#include <stdbool.h>

#define CRUD_MAX_RECORDS 256

typedef struct {
    EntityDef def;
    EntityInst instances[CRUD_MAX_RECORDS];
    int count;
    int next_id;
    int deleted[CRUD_MAX_RECORDS]; int deleted_count;
} CrudRepo;

void crud_init(CrudRepo *repo, const char *entity_name);
int  crud_create(CrudRepo *repo);
bool crud_read(CrudRepo *repo, int id, EntityInst **out);
bool crud_update(CrudRepo *repo, int id, int field_idx, const char *value);
bool crud_delete(CrudRepo *repo, int id);
int  crud_find(CrudRepo *repo, const char *field_name, const char *value, int *results, int max_results);
int  crud_count(CrudRepo *repo);
void crud_print_all(CrudRepo *repo);
#endif
