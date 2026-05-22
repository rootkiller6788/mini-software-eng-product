#include "biz_entity.h"
#include <stdio.h>
#include <string.h>

void entity_def_init(EntityDef *ed, const char *name) { memset(ed, 0, sizeof(*ed)); strncpy(ed->name, name, ENT_NAME_LEN-1); ed->name[ENT_NAME_LEN-1]='\0'; }

int entity_def_add_field(EntityDef *ed, const char *fn, FieldType type, bool req) {
    if (ed->field_count >= ENT_MAX_FIELDS) return -1;
    EntityField *f = &ed->fields[ed->field_count]; f->type = type; f->required = req; f->max_len = 128;
    strncpy(f->name, fn, 31); f->name[31] = '\0';
    return ed->field_count++;
}

int entity_def_get_field(EntityDef *ed, const char *name) { for (int i = 0; i < ed->field_count; i++) if (strcmp(ed->fields[i].name, name) == 0) return i; return -1; }

void entity_inst_init(EntityInst *ei, EntityDef *def, int id) { memset(ei, 0, sizeof(*ei)); ei->def = def; ei->id = id; }

bool entity_set_field(EntityInst *ei, int fi, const char *value) {
    if (!ei->def || fi < 0 || fi >= ei->def->field_count) return false;
    strncpy(ei->data[fi], value ? value : "", 127); ei->data[fi][127] = '\0';
    return true;
}

const char *entity_get_field(EntityInst *ei, int fi) {
    if (!ei->def || fi < 0 || fi >= ei->def->field_count) return NULL;
    return ei->data[fi];
}

bool entity_validate(EntityInst *ei) {
    if (!ei->def) return false;
    for (int i = 0; i < ei->def->field_count; i++) if (ei->def->fields[i].required && ei->data[i][0] == '\0') return false;
    return true;
}

void entity_print(EntityInst *ei) {
    if (!ei->def) return;
    printf("  [%s #%d]\n", ei->def->name, ei->id);
    for (int i = 0; i < ei->def->field_count; i++) printf("    %s: %s\n", ei->def->fields[i].name, ei->data[i]);
}
