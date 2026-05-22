#include "domain_model.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/* ============================================================
 * Domain Model — implementation
 * ============================================================ */

static int _default_equals(const void *a, const void *b, size_t size) {
    return memcmp(a, b, size) == 0;
}

/* ---------- Glossary ---------- */
void dm_glossary_init(dm_glossary *g) {
    g->count = 0;
}

int dm_glossary_add(dm_glossary *g, const char *term, const char *definition) {
    if (g->count >= DM_MAX_EVENTS) return -1;
    dm_glossary_entry *e = &g->entries[g->count];
    strncpy(e->term, term, DM_MAX_TERM - 1);
    strncpy(e->definition, definition, DM_MAX_TERM - 1);
    g->count++;
    return 0;
}

const dm_glossary_entry *dm_glossary_find(const dm_glossary *g, const char *term) {
    for (size_t i = 0; i < g->count; i++) {
        if (strcmp(g->entries[i].term, term) == 0) return &g->entries[i];
    }
    return NULL;
}

/* ---------- Value Object ---------- */
void dm_value_object_init(dm_value_object *vo, const char *name, void *data, size_t size,
                          int (*equals)(const void *, const void *, size_t)) {
    strncpy(vo->name, name, DM_MAX_NAME - 1);
    vo->data = data;
    vo->size = size;
    vo->equals = equals ? equals : _default_equals;
}

int dm_value_object_equals(const dm_value_object *a, const dm_value_object *b) {
    if (!a || !b) return 0;
    if (strcmp(a->name, b->name) != 0) return 0;
    if (a->size != b->size) return 0;
    return a->equals(a->data, b->data, a->size);
}

/* ---------- Entity ---------- */
void dm_entity_init(dm_entity *e, uint64_t id, const char *type, void *data, size_t size) {
    e->id = id;
    strncpy(e->type, type, DM_MAX_NAME - 1);
    e->data = data;
    e->size = size;
}

int dm_entity_equals(const dm_entity *a, const dm_entity *b) {
    if (!a || !b) return 0;
    return a->id == b->id && strcmp(a->type, b->type) == 0;
}

/* ---------- Domain Event ---------- */
void dm_domain_event_init(dm_domain_event *ev, uint64_t id, const char *name,
                          uint64_t aggregate_id, const char *agg_type,
                          void *payload, size_t size) {
    ev->event_id = id;
    strncpy(ev->event_name, name, DM_MAX_EVENT_NAME - 1);
    ev->aggregate_id = aggregate_id;
    strncpy(ev->aggregate_type, agg_type, DM_MAX_NAME - 1);
    ev->occurred_at = (int64_t)time(NULL);
    ev->payload = payload;
    ev->payload_size = size;
}

/* ---------- Aggregate Root ---------- */
void dm_aggregate_root_init(dm_aggregate_root *ar, uint64_t id, const char *type,
                            void *data, size_t size) {
    ar->id = id;
    strncpy(ar->type, type, DM_MAX_NAME - 1);
    ar->data = data;
    ar->data_size = size;
    ar->pending_count = 0;
}

void dm_aggregate_root_add_event(dm_aggregate_root *ar, const dm_domain_event *ev) {
    if (ar->pending_count >= DM_MAX_EVENTS) return;
    ar->pending_events[ar->pending_count++] = *ev;
}

void dm_aggregate_root_clear_events(dm_aggregate_root *ar) {
    ar->pending_count = 0;
}

const dm_domain_event *dm_aggregate_root_get_events(const dm_aggregate_root *ar, size_t *count) {
    if (count) *count = ar->pending_count;
    return ar->pending_events;
}

/* ---------- Context Mapping ---------- */
void dm_context_map_init(dm_context_map *cm, const char *name,
                         const char *upstream, const char *downstream,
                         dm_context_map_type type) {
    strncpy(cm->name, name, DM_MAX_NAME - 1);
    strncpy(cm->upstream, upstream, DM_MAX_CONTEXT - 1);
    strncpy(cm->downstream, downstream, DM_MAX_CONTEXT - 1);
    cm->map_type = type;
}

const char *dm_context_map_type_name(dm_context_map_type type) {
    switch (type) {
    case DM_MAP_PARTNERSHIP:         return "Partnership";
    case DM_MAP_SHARED_KERNEL:       return "Shared Kernel";
    case DM_MAP_CUSTOMER_SUPPLIER:   return "Customer-Supplier";
    case DM_MAP_CONFORMIST:          return "Conformist";
    case DM_MAP_ANTICORRUPTION_LAYER:return "Anti-corruption Layer";
    case DM_MAP_OPEN_HOST_SERVICE:   return "Open Host Service";
    case DM_MAP_PUBLISHED_LANGUAGE:  return "Published Language";
    default:                         return "Unknown";
    }
}

/* ---------- Bounded Context ---------- */
void dm_bounded_context_init(dm_bounded_context *ctx, const char *name) {
    strncpy(ctx->name, name, DM_MAX_NAME - 1);
    ctx->aggregate_count = 0;
    ctx->mapping_count = 0;
    ctx->service_count = 0;
    dm_glossary_init(&ctx->glossary);
}

void dm_bounded_context_add_aggregate(dm_bounded_context *ctx, const dm_aggregate_root *ar) {
    if (ctx->aggregate_count >= DM_MAX_MAPS) return;
    ctx->aggregates[ctx->aggregate_count++] = *ar;
}

void dm_bounded_context_add_map(dm_bounded_context *ctx, const dm_context_map *cm) {
    if (ctx->mapping_count >= DM_MAX_MAPS) return;
    ctx->mappings[ctx->mapping_count++] = *cm;
}

const dm_aggregate_root *dm_bounded_context_find_aggregate(const dm_bounded_context *ctx, uint64_t id) {
    for (size_t i = 0; i < ctx->aggregate_count; i++) {
        if (ctx->aggregates[i].id == id) return &ctx->aggregates[i];
    }
    return NULL;
}
