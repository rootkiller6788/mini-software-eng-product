#ifndef BIZ_ENTITY_H
#define BIZ_ENTITY_H
#include <stdint.h>
#include <stdbool.h>

#define ENT_MAX_FIELDS 16
#define ENT_NAME_LEN 64

typedef enum { FT_INT, FT_DOUBLE, FT_STRING, FT_DATE, FT_BOOL } FieldType;

typedef struct { char name[32]; FieldType type; bool required; int max_len; } EntityField;

typedef struct {
    int id; char name[ENT_NAME_LEN];
    EntityField fields[ENT_MAX_FIELDS]; int field_count;
} EntityDef;

typedef struct {
    EntityDef *def; int id;
    char data[ENT_MAX_FIELDS][128]; /* serialized field values */
} EntityInst;

void entity_def_init(EntityDef *ed, const char *name);
int  entity_def_add_field(EntityDef *ed, const char *field_name, FieldType type, bool required);
int  entity_def_get_field(EntityDef *ed, const char *name);

void entity_inst_init(EntityInst *ei, EntityDef *def, int id);
bool entity_set_field(EntityInst *ei, int field_idx, const char *value);
const char *entity_get_field(EntityInst *ei, int field_idx);
bool entity_validate(EntityInst *ei);
void entity_print(EntityInst *ei);
#endif
