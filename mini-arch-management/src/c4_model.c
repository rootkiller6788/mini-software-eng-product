#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "c4_model.h"

void c4_model_init(C4Model *model, const char *workspace_name,
                    const char *description)
{
    if (!model) return;
    memset(model, 0, sizeof(*model));
    if (workspace_name) {
        strncpy(model->workspace_name, workspace_name, C4_MAX_NAME_LENGTH - 1);
        model->workspace_name[C4_MAX_NAME_LENGTH - 1] = '\0';
    }
    if (description) {
        strncpy(model->description, description, C4_MAX_DESCRIPTION_LENGTH - 1);
        model->description[C4_MAX_DESCRIPTION_LENGTH - 1] = '\0';
    }
    model->next_id = 1;
    model->created_at = time(NULL);
    model->updated_at = model->created_at;
}

C4Model *c4_model_create(const char *workspace_name, const char *description)
{
    C4Model *model = (C4Model *)malloc(sizeof(C4Model));
    if (!model) return NULL;
    c4_model_init(model, workspace_name, description);
    return model;
}

void c4_model_destroy(C4Model *model)
{
    if (model) free(model);
}

unsigned int c4_model_add_person(C4Model *model, const char *name,
                                  const char *role, bool is_external)
{
    if (!model || model->person_count >= C4_MAX_USERS) return 0;
    C4Person *p = &model->people[model->person_count];
    p->id = model->next_id++;
    strncpy(p->name, name ? name : "", C4_MAX_NAME_LENGTH - 1);
    p->name[C4_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(p->role, role ? role : "", C4_MAX_DESCRIPTION_LENGTH - 1);
    p->role[C4_MAX_DESCRIPTION_LENGTH - 1] = '\0';
    p->is_external = is_external;
    model->person_count++;
    model->updated_at = time(NULL);
    return p->id;
}

unsigned int c4_model_add_system(C4Model *model, const char *name,
                                  const char *description,
                                  const char *boundary, bool is_external)
{
    if (!model || model->system_count >= C4_MAX_SYSTEMS) return 0;
    C4System *s = &model->systems[model->system_count];
    s->id = model->next_id++;
    strncpy(s->name, name ? name : "", C4_MAX_NAME_LENGTH - 1);
    s->name[C4_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(s->description, description ? description : "",
            C4_MAX_DESCRIPTION_LENGTH - 1);
    s->description[C4_MAX_DESCRIPTION_LENGTH - 1] = '\0';
    strncpy(s->boundary, boundary ? boundary : "", C4_MAX_NAME_LENGTH - 1);
    s->boundary[C4_MAX_NAME_LENGTH - 1] = '\0';
    s->is_external = is_external;
    s->tag_count = 0;
    s->adr_link_count = 0;
    model->system_count++;
    model->updated_at = time(NULL);
    return s->id;
}

unsigned int c4_model_add_container(C4Model *model, const char *name,
                                     const char *description,
                                     const char *technology,
                                     unsigned int system_id)
{
    if (!model || model->container_count >= C4_MAX_CONTAINERS) return 0;
    C4Container *c = &model->containers[model->container_count];
    c->id = model->next_id++;
    strncpy(c->name, name ? name : "", C4_MAX_NAME_LENGTH - 1);
    c->name[C4_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(c->description, description ? description : "",
            C4_MAX_DESCRIPTION_LENGTH - 1);
    c->description[C4_MAX_DESCRIPTION_LENGTH - 1] = '\0';
    strncpy(c->technology, technology ? technology : "",
            C4_MAX_TECHNOLOGY_LENGTH - 1);
    c->technology[C4_MAX_TECHNOLOGY_LENGTH - 1] = '\0';
    c->system_id = system_id;
    c->tag_count = 0;
    c->adr_link_count = 0;
    model->container_count++;
    model->updated_at = time(NULL);
    return c->id;
}

unsigned int c4_model_add_component(C4Model *model, const char *name,
                                     const char *description,
                                     const char *technology,
                                     unsigned int container_id)
{
    if (!model || model->component_count >= C4_MAX_COMPONENTS) return 0;
    C4Component *comp = &model->components[model->component_count];
    comp->id = model->next_id++;
    strncpy(comp->name, name ? name : "", C4_MAX_NAME_LENGTH - 1);
    comp->name[C4_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(comp->description, description ? description : "",
            C4_MAX_DESCRIPTION_LENGTH - 1);
    comp->description[C4_MAX_DESCRIPTION_LENGTH - 1] = '\0';
    strncpy(comp->technology, technology ? technology : "",
            C4_MAX_TECHNOLOGY_LENGTH - 1);
    comp->technology[C4_MAX_TECHNOLOGY_LENGTH - 1] = '\0';
    comp->container_id = container_id;
    comp->responsibility_count = 0;
    comp->tag_count = 0;
    comp->adr_link_count = 0;
    model->component_count++;
    model->updated_at = time(NULL);
    return comp->id;
}

unsigned int c4_model_add_code_element(C4Model *model, const char *name,
                                        const char *description,
                                        const char *language,
                                        unsigned int component_id)
{
    if (!model || model->code_element_count >= C4_MAX_CODE_ELEMENTS) return 0;
    C4CodeElement *ce = &model->code_elements[model->code_element_count];
    ce->id = model->next_id++;
    strncpy(ce->name, name ? name : "", C4_MAX_NAME_LENGTH - 1);
    ce->name[C4_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(ce->description, description ? description : "",
            C4_MAX_DESCRIPTION_LENGTH - 1);
    ce->description[C4_MAX_DESCRIPTION_LENGTH - 1] = '\0';
    strncpy(ce->language, language ? language : "",
            C4_MAX_TECHNOLOGY_LENGTH - 1);
    ce->language[C4_MAX_TECHNOLOGY_LENGTH - 1] = '\0';
    ce->component_id = component_id;
    ce->tag_count = 0;
    model->code_element_count++;
    model->updated_at = time(NULL);
    return ce->id;
}

unsigned int c4_model_add_relationship(C4Model *model,
                                        const char *from_label,
                                        const char *to_label,
                                        C4RelationshipType type,
                                        const char *description,
                                        const char *technology)
{
    if (!model || model->relationship_count >= C4_MAX_RELATIONSHIPS) return 0;
    C4Relationship *r = &model->relationships[model->relationship_count];
    r->id = model->next_id++;
    strncpy(r->from_label, from_label ? from_label : "",
            C4_MAX_NAME_LENGTH - 1);
    r->from_label[C4_MAX_NAME_LENGTH - 1] = '\0';
    strncpy(r->to_label, to_label ? to_label : "", C4_MAX_NAME_LENGTH - 1);
    r->to_label[C4_MAX_NAME_LENGTH - 1] = '\0';
    r->type = type;
    strncpy(r->description, description ? description : "",
            C4_MAX_DESCRIPTION_LENGTH - 1);
    r->description[C4_MAX_DESCRIPTION_LENGTH - 1] = '\0';
    strncpy(r->technology, technology ? technology : "",
            C4_MAX_TECHNOLOGY_LENGTH - 1);
    r->technology[C4_MAX_TECHNOLOGY_LENGTH - 1] = '\0';
    r->adr_link_count = 0;
    model->relationship_count++;
    model->updated_at = time(NULL);
    return r->id;
}

static bool c4_add_adr_link(C4AdrLink *links, size_t *count, size_t max,
                             unsigned int adr_id, const char *reason)
{
    if (*count >= max) return false;
    links[*count].adr_id = adr_id;
    strncpy(links[*count].reason, reason ? reason : "",
            C4_MAX_DESCRIPTION_LENGTH - 1);
    links[*count].reason[C4_MAX_DESCRIPTION_LENGTH - 1] = '\0';
    (*count)++;
    return true;
}

bool c4_model_link_adr_to_system(C4Model *model, unsigned int system_id,
                                  unsigned int adr_id, const char *reason)
{
    C4System *s = c4_model_find_system(model, system_id);
    if (!s) return false;
    return c4_add_adr_link(s->adr_links, &s->adr_link_count,
                            C4_MAX_RESPONSIBILITIES, adr_id, reason);
}

bool c4_model_link_adr_to_container(C4Model *model, unsigned int container_id,
                                     unsigned int adr_id, const char *reason)
{
    C4Container *c = c4_model_find_container(model, container_id);
    if (!c) return false;
    return c4_add_adr_link(c->adr_links, &c->adr_link_count,
                            C4_MAX_RESPONSIBILITIES, adr_id, reason);
}

bool c4_model_link_adr_to_component(C4Model *model, unsigned int component_id,
                                     unsigned int adr_id, const char *reason)
{
    C4Component *c = c4_model_find_component(model, component_id);
    if (!c) return false;
    return c4_add_adr_link(c->adr_links, &c->adr_link_count,
                            C4_MAX_RESPONSIBILITIES, adr_id, reason);
}

bool c4_model_link_adr_to_relationship(C4Model *model,
                                        unsigned int relationship_id,
                                        unsigned int adr_id,
                                        const char *reason)
{
    C4Relationship *r = c4_model_find_relationship(model, relationship_id);
    if (!r) return false;
    return c4_add_adr_link(r->adr_links, &r->adr_link_count,
                            C4_MAX_RESPONSIBILITIES, adr_id, reason);
}

C4System *c4_model_find_system(C4Model *model, unsigned int id)
{
    if (!model) return NULL;
    for (size_t i = 0; i < model->system_count; i++) {
        if (model->systems[i].id == id) return &model->systems[i];
    }
    return NULL;
}

C4Container *c4_model_find_container(C4Model *model, unsigned int id)
{
    if (!model) return NULL;
    for (size_t i = 0; i < model->container_count; i++) {
        if (model->containers[i].id == id) return &model->containers[i];
    }
    return NULL;
}

C4Component *c4_model_find_component(C4Model *model, unsigned int id)
{
    if (!model) return NULL;
    for (size_t i = 0; i < model->component_count; i++) {
        if (model->components[i].id == id) return &model->components[i];
    }
    return NULL;
}

C4CodeElement *c4_model_find_code_element(C4Model *model, unsigned int id)
{
    if (!model) return NULL;
    for (size_t i = 0; i < model->code_element_count; i++) {
        if (model->code_elements[i].id == id) return &model->code_elements[i];
    }
    return NULL;
}

C4Relationship *c4_model_find_relationship(C4Model *model, unsigned int id)
{
    if (!model) return NULL;
    for (size_t i = 0; i < model->relationship_count; i++) {
        if (model->relationships[i].id == id) return &model->relationships[i];
    }
    return NULL;
}

const char *c4_level_to_string(C4Level level)
{
    switch (level) {
    case C4_LEVEL_CONTEXT:   return "Context";
    case C4_LEVEL_CONTAINER: return "Container";
    case C4_LEVEL_COMPONENT: return "Component";
    case C4_LEVEL_CODE:      return "Code";
    default:                 return "Unknown";
    }
}

const char *c4_relationship_type_to_string(C4RelationshipType type)
{
    switch (type) {
    case C4_REL_USES:          return "Uses";
    case C4_REL_DEPENDS_ON:    return "Depends on";
    case C4_REL_CALLS:         return "Calls";
    case C4_REL_PUBLISHES_TO:  return "Publishes to";
    case C4_REL_SUBSCRIBES_TO: return "Subscribes to";
    case C4_REL_SENDS_TO:      return "Sends to";
    case C4_REL_STORED_IN:     return "Stored in";
    case C4_REL_DEPLOYED_ON:   return "Deployed on";
    case C4_REL_OWNED_BY:      return "Owned by";
    default:                   return "Unknown";
    }
}

void c4_model_print_context(const C4Model *model)
{
    if (!model) return;
    printf("=== C4 Context Diagram: %s ===\n", model->workspace_name);
    printf("Users:\n");
    for (size_t i = 0; i < model->person_count; i++) {
        printf("  [%02u] %s (%s) %s\n",
               model->people[i].id,
               model->people[i].name,
               model->people[i].role,
               model->people[i].is_external ? "[External]" : "");
    }
    printf("Systems:\n");
    for (size_t i = 0; i < model->system_count; i++) {
        printf("  [%02u] %s -- %s %s\n",
               model->systems[i].id,
               model->systems[i].name,
               model->systems[i].description,
               model->systems[i].is_external ? "[External]" : "");
    }
    printf("Relationships:\n");
    for (size_t i = 0; i < model->relationship_count; i++) {
        printf("  %s --[%s]--> %s\n",
               model->relationships[i].from_label,
               c4_relationship_type_to_string(model->relationships[i].type),
               model->relationships[i].to_label);
    }
}

void c4_model_print_container(const C4Model *model)
{
    if (!model) return;
    printf("=== C4 Container Diagram: %s ===\n", model->workspace_name);
    for (size_t i = 0; i < model->system_count; i++) {
        printf("System [%02u]: %s\n", model->systems[i].id,
               model->systems[i].name);
        for (size_t j = 0; j < model->container_count; j++) {
            if (model->containers[j].system_id == model->systems[i].id) {
                printf("  Container [%02u]: %s [%s] -- %s\n",
                       model->containers[j].id,
                       model->containers[j].name,
                       model->containers[j].technology,
                       model->containers[j].description);
            }
        }
    }
}

void c4_model_print_component(const C4Model *model, unsigned int container_id)
{
    if (!model) return;
    C4Container *c = c4_model_find_container((C4Model *)model, container_id);
    if (!c) return;
    printf("=== Components in Container [%02u] %s ===\n", c->id, c->name);
    for (size_t i = 0; i < model->component_count; i++) {
        if (model->components[i].container_id == container_id) {
            printf("  [%02u] %s [%s] -- %s\n",
                   model->components[i].id,
                   model->components[i].name,
                   model->components[i].technology,
                   model->components[i].description);
        }
    }
}

void c4_model_print_code(const C4Model *model, unsigned int component_id)
{
    if (!model) return;
    C4Component *comp = c4_model_find_component((C4Model *)model, component_id);
    if (!comp) return;
    printf("=== Code Elements in Component [%02u] %s ===\n",
           comp->id, comp->name);
    for (size_t i = 0; i < model->code_element_count; i++) {
        if (model->code_elements[i].component_id == component_id) {
            printf("  [%02u] %s [%s]\n",
                   model->code_elements[i].id,
                   model->code_elements[i].name,
                   model->code_elements[i].language);
        }
    }
}

bool c4_model_export_plantuml(const C4Model *model, const char *filename,
                               C4Level level)
{
    if (!model || !filename) return false;
    FILE *fp = fopen(filename, "w");
    if (!fp) return false;
    fprintf(fp, "@startuml\n");
    fprintf(fp, "title %s - %s\n\n", model->workspace_name,
            c4_level_to_string(level));
    if (level == C4_LEVEL_CONTEXT) {
        for (size_t i = 0; i < model->person_count; i++) {
            fprintf(fp, "Person(%s, \"%s\", \"%s\")\n",
                    model->people[i].name,
                    model->people[i].name,
                    model->people[i].role);
        }
        for (size_t i = 0; i < model->system_count; i++) {
            fprintf(fp, "%s_System(%s, \"%s\", \"%s\")\n",
                    model->systems[i].is_external ? "System_Ext" : "System",
                    model->systems[i].name,
                    model->systems[i].name,
                    model->systems[i].description);
        }
    }
    if (level == C4_LEVEL_CONTAINER) {
        for (size_t i = 0; i < model->container_count; i++) {
            fprintf(fp, "Container(%s, \"%s\", \"%s\", \"%s\")\n",
                    model->containers[i].name,
                    model->containers[i].name,
                    model->containers[i].technology,
                    model->containers[i].description);
        }
    }
    for (size_t i = 0; i < model->relationship_count; i++) {
        fprintf(fp, "Rel(%s, %s, \"%s\", \"%s\")\n",
                model->relationships[i].from_label,
                model->relationships[i].to_label,
                c4_relationship_type_to_string(model->relationships[i].type),
                model->relationships[i].description);
    }
    fprintf(fp, "@enduml\n");
    fclose(fp);
    return true;
}

size_t c4_model_get_container_count_for_system(const C4Model *model,
                                                unsigned int system_id)
{
    if (!model) return 0;
    size_t count = 0;
    for (size_t i = 0; i < model->container_count; i++) {
        if (model->containers[i].system_id == system_id) count++;
    }
    return count;
}

size_t c4_model_get_component_count_for_container(const C4Model *model,
                                                   unsigned int container_id)
{
    if (!model) return 0;
    size_t count = 0;
    for (size_t i = 0; i < model->component_count; i++) {
        if (model->components[i].container_id == container_id) count++;
    }
    return count;
}

bool c4_model_validate(const C4Model *model)
{
    if (!model) return false;
    if (model->next_id == 0) return false;
    if (model->system_count > C4_MAX_SYSTEMS) return false;
    if (model->container_count > C4_MAX_CONTAINERS) return false;
    if (model->component_count > C4_MAX_COMPONENTS) return false;
    if (model->code_element_count > C4_MAX_CODE_ELEMENTS) return false;
    if (model->relationship_count > C4_MAX_RELATIONSHIPS) return false;
    for (size_t i = 0; i < model->container_count; i++) {
        if (!c4_model_find_system((C4Model *)model,
                                   model->containers[i].system_id))
            return false;
    }
    for (size_t i = 0; i < model->component_count; i++) {
        if (!c4_model_find_container((C4Model *)model,
                                      model->components[i].container_id))
            return false;
    }
    return true;
}
