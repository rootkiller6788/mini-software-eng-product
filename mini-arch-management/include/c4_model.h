#ifndef C4_MODEL_H
#define C4_MODEL_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>
#include "adr_system.h"

#define C4_MAX_NAME_LENGTH       256
#define C4_MAX_DESCRIPTION_LENGTH 2048
#define C4_MAX_TECHNOLOGY_LENGTH  128
#define C4_MAX_TAG_LENGTH         64
#define C4_MAX_SYSTEMS            32
#define C4_MAX_USERS              32
#define C4_MAX_CONTAINERS         64
#define C4_MAX_COMPONENTS        128
#define C4_MAX_CODE_ELEMENTS     256
#define C4_MAX_RELATIONSHIPS     512
#define C4_MAX_RESPONSIBILITIES   16

typedef enum C4Level {
    C4_LEVEL_CONTEXT,
    C4_LEVEL_CONTAINER,
    C4_LEVEL_COMPONENT,
    C4_LEVEL_CODE
} C4Level;

typedef enum C4RelationshipType {
    C4_REL_USES,
    C4_REL_DEPENDS_ON,
    C4_REL_CALLS,
    C4_REL_PUBLISHES_TO,
    C4_REL_SUBSCRIBES_TO,
    C4_REL_SENDS_TO,
    C4_REL_STORED_IN,
    C4_REL_DEPLOYED_ON,
    C4_REL_OWNED_BY
} C4RelationshipType;

typedef struct C4Tag {
    char name[C4_MAX_TAG_LENGTH];
} C4Tag;

typedef struct C4Responsibility {
    char description[C4_MAX_DESCRIPTION_LENGTH];
} C4Responsibility;

typedef struct C4AdrLink {
    unsigned int adr_id;
    char reason[C4_MAX_DESCRIPTION_LENGTH];
} C4AdrLink;

typedef struct C4Person {
    unsigned int id;
    char name[C4_MAX_NAME_LENGTH];
    char role[C4_MAX_DESCRIPTION_LENGTH];
    bool is_external;
} C4Person;

typedef struct C4System {
    unsigned int id;
    char name[C4_MAX_NAME_LENGTH];
    char description[C4_MAX_DESCRIPTION_LENGTH];
    char boundary[C4_MAX_NAME_LENGTH];
    C4Tag tags[C4_MAX_RESPONSIBILITIES];
    size_t tag_count;
    C4AdrLink adr_links[C4_MAX_RESPONSIBILITIES];
    size_t adr_link_count;
    bool is_external;
} C4System;

typedef struct C4Container {
    unsigned int id;
    char name[C4_MAX_NAME_LENGTH];
    char description[C4_MAX_DESCRIPTION_LENGTH];
    char technology[C4_MAX_TECHNOLOGY_LENGTH];
    unsigned int system_id;
    C4Tag tags[C4_MAX_RESPONSIBILITIES];
    size_t tag_count;
    C4AdrLink adr_links[C4_MAX_RESPONSIBILITIES];
    size_t adr_link_count;
} C4Container;

typedef struct C4Component {
    unsigned int id;
    char name[C4_MAX_NAME_LENGTH];
    char description[C4_MAX_DESCRIPTION_LENGTH];
    char technology[C4_MAX_TECHNOLOGY_LENGTH];
    unsigned int container_id;
    C4Responsibility responsibilities[C4_MAX_RESPONSIBILITIES];
    size_t responsibility_count;
    C4Tag tags[C4_MAX_RESPONSIBILITIES];
    size_t tag_count;
    C4AdrLink adr_links[C4_MAX_RESPONSIBILITIES];
    size_t adr_link_count;
} C4Component;

typedef struct C4CodeElement {
    unsigned int id;
    char name[C4_MAX_NAME_LENGTH];
    char description[C4_MAX_DESCRIPTION_LENGTH];
    char language[C4_MAX_TECHNOLOGY_LENGTH];
    unsigned int component_id;
    C4Tag tags[C4_MAX_RESPONSIBILITIES];
    size_t tag_count;
} C4CodeElement;

typedef struct C4Relationship {
    unsigned int id;
    char from_label[C4_MAX_NAME_LENGTH];
    char to_label[C4_MAX_NAME_LENGTH];
    C4RelationshipType type;
    char description[C4_MAX_DESCRIPTION_LENGTH];
    char technology[C4_MAX_TECHNOLOGY_LENGTH];
    C4AdrLink adr_links[C4_MAX_RESPONSIBILITIES];
    size_t adr_link_count;
} C4Relationship;

typedef struct C4Model {
    char workspace_name[C4_MAX_NAME_LENGTH];
    char description[C4_MAX_DESCRIPTION_LENGTH];
    time_t created_at;
    time_t updated_at;
    C4Person people[C4_MAX_USERS];
    size_t person_count;
    C4System systems[C4_MAX_SYSTEMS];
    size_t system_count;
    C4Container containers[C4_MAX_CONTAINERS];
    size_t container_count;
    C4Component components[C4_MAX_COMPONENTS];
    size_t component_count;
    C4CodeElement code_elements[C4_MAX_CODE_ELEMENTS];
    size_t code_element_count;
    C4Relationship relationships[C4_MAX_RELATIONSHIPS];
    size_t relationship_count;
    unsigned int next_id;
} C4Model;

void c4_model_init(C4Model *model, const char *workspace_name, const char *description);
C4Model *c4_model_create(const char *workspace_name, const char *description);
void c4_model_destroy(C4Model *model);

unsigned int c4_model_add_person(C4Model *model, const char *name,
                                  const char *role, bool is_external);
unsigned int c4_model_add_system(C4Model *model, const char *name,
                                  const char *description, const char *boundary,
                                  bool is_external);
unsigned int c4_model_add_container(C4Model *model, const char *name,
                                     const char *description,
                                     const char *technology,
                                     unsigned int system_id);
unsigned int c4_model_add_component(C4Model *model, const char *name,
                                     const char *description,
                                     const char *technology,
                                     unsigned int container_id);
unsigned int c4_model_add_code_element(C4Model *model, const char *name,
                                        const char *description,
                                        const char *language,
                                        unsigned int component_id);
unsigned int c4_model_add_relationship(C4Model *model,
                                        const char *from_label,
                                        const char *to_label,
                                        C4RelationshipType type,
                                        const char *description,
                                        const char *technology);

bool c4_model_link_adr_to_system(C4Model *model, unsigned int system_id,
                                  unsigned int adr_id, const char *reason);
bool c4_model_link_adr_to_container(C4Model *model, unsigned int container_id,
                                     unsigned int adr_id, const char *reason);
bool c4_model_link_adr_to_component(C4Model *model, unsigned int component_id,
                                     unsigned int adr_id, const char *reason);
bool c4_model_link_adr_to_relationship(C4Model *model,
                                        unsigned int relationship_id,
                                        unsigned int adr_id,
                                        const char *reason);

C4System *c4_model_find_system(C4Model *model, unsigned int id);
C4Container *c4_model_find_container(C4Model *model, unsigned int id);
C4Component *c4_model_find_component(C4Model *model, unsigned int id);
C4CodeElement *c4_model_find_code_element(C4Model *model, unsigned int id);
C4Relationship *c4_model_find_relationship(C4Model *model, unsigned int id);

const char *c4_level_to_string(C4Level level);
const char *c4_relationship_type_to_string(C4RelationshipType type);

void c4_model_print_context(const C4Model *model);
void c4_model_print_container(const C4Model *model);
void c4_model_print_component(const C4Model *model, unsigned int container_id);
void c4_model_print_code(const C4Model *model, unsigned int component_id);

bool c4_model_export_plantuml(const C4Model *model, const char *filename,
                               C4Level level);
size_t c4_model_get_container_count_for_system(const C4Model *model,
                                                unsigned int system_id);
size_t c4_model_get_component_count_for_container(const C4Model *model,
                                                   unsigned int container_id);
bool c4_model_validate(const C4Model *model);

#endif /* C4_MODEL_H */
