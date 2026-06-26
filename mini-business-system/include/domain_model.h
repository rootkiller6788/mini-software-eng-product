#ifndef DOMAIN_MODEL_H
#define DOMAIN_MODEL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * Domain Modeling — DDD building blocks (C99)
 * ============================================================ */

#define DM_MAX_NAME     64
#define DM_MAX_TERM     128
#define DM_MAX_CONTEXT  16
#define DM_MAX_MAPS     32
#define DM_MAX_EVENTS   64
#define DM_MAX_EVENT_NAME 128
#define DM_MAX_EVENT_PAYLOAD 256

/* ---------- Ubiquitous Language Glossary ---------- */
typedef struct {
    char term[DM_MAX_TERM];
    char definition[DM_MAX_TERM];
} dm_glossary_entry;

typedef struct {
    dm_glossary_entry entries[DM_MAX_EVENTS];
    size_t count;
} dm_glossary;

void dm_glossary_init(dm_glossary *g);
int  dm_glossary_add(dm_glossary *g, const char *term, const char *definition);
const dm_glossary_entry *dm_glossary_find(const dm_glossary *g, const char *term);

/* ---------- Value Object ---------- */
typedef struct {
    char name[DM_MAX_NAME];
    void *data;
    size_t size;
    int (*equals)(const void *a, const void *b, size_t size);
} dm_value_object;

void dm_value_object_init(dm_value_object *vo, const char *name, void *data, size_t size,
                          int (*equals)(const void *, const void *, size_t));
int  dm_value_object_equals(const dm_value_object *a, const dm_value_object *b);

/* ---------- Entity ---------- */
typedef struct {
    uint64_t id;
    char     type[DM_MAX_NAME];
    void    *data;
    size_t   size;
} dm_entity;

void dm_entity_init(dm_entity *e, uint64_t id, const char *type, void *data, size_t size);
int  dm_entity_equals(const dm_entity *a, const dm_entity *b);

/* ---------- Domain Event ---------- */
typedef struct {
    uint64_t event_id;
    char     event_name[DM_MAX_EVENT_NAME];
    uint64_t aggregate_id;
    char     aggregate_type[DM_MAX_NAME];
    int64_t  occurred_at;     /* unix timestamp */
    void    *payload;
    size_t   payload_size;
} dm_domain_event;

void dm_domain_event_init(dm_domain_event *ev, uint64_t id, const char *name,
                          uint64_t aggregate_id, const char *agg_type, void *payload, size_t size);

/* ---------- Aggregate Root ---------- */
typedef struct dm_aggregate_root {
    uint64_t id;
    char     type[DM_MAX_NAME];
    void    *data;
    size_t   data_size;
    dm_domain_event pending_events[DM_MAX_EVENTS];
    size_t   pending_count;
} dm_aggregate_root;

void dm_aggregate_root_init(dm_aggregate_root *ar, uint64_t id, const char *type, void *data, size_t size);
void dm_aggregate_root_add_event(dm_aggregate_root *ar, const dm_domain_event *ev);
void dm_aggregate_root_clear_events(dm_aggregate_root *ar);
const dm_domain_event *dm_aggregate_root_get_events(const dm_aggregate_root *ar, size_t *count);

/* ---------- Context Mapping ---------- */
typedef enum {
    DM_MAP_PARTNERSHIP,
    DM_MAP_SHARED_KERNEL,
    DM_MAP_CUSTOMER_SUPPLIER,
    DM_MAP_CONFORMIST,
    DM_MAP_ANTICORRUPTION_LAYER,
    DM_MAP_OPEN_HOST_SERVICE,
    DM_MAP_PUBLISHED_LANGUAGE
} dm_context_map_type;

typedef struct {
    char                name[DM_MAX_NAME];
    char                upstream[DM_MAX_CONTEXT];
    char                downstream[DM_MAX_CONTEXT];
    dm_context_map_type map_type;
} dm_context_map;

void dm_context_map_init(dm_context_map *cm, const char *name,
                         const char *upstream, const char *downstream,
                         dm_context_map_type type);
const char *dm_context_map_type_name(dm_context_map_type type);

/* ---------- Bounded Context ---------- */
typedef struct {
    char                 name[DM_MAX_NAME];
    dm_aggregate_root    aggregates[DM_MAX_MAPS];
    size_t              aggregate_count;
    dm_glossary          glossary;
    dm_context_map       mappings[DM_MAX_MAPS];
    size_t              mapping_count;
    void                *domain_services[DM_MAX_CONTEXT];
    size_t              service_count;
} dm_bounded_context;

void dm_bounded_context_init(dm_bounded_context *ctx, const char *name);
void dm_bounded_context_add_aggregate(dm_bounded_context *ctx, const dm_aggregate_root *ar);
void dm_bounded_context_add_map(dm_bounded_context *ctx, const dm_context_map *cm);
const dm_aggregate_root *dm_bounded_context_find_aggregate(const dm_bounded_context *ctx, uint64_t id);

#ifdef __cplusplus
}
#endif

#endif /* DOMAIN_MODEL_H */
