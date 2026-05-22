/* ============================================================
 * Example 1: Domain Model — Bounded Context & Context Mapping
 * ============================================================ */

#include "domain_model.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct {
    char    customer_id[32];
    char    name[64];
    int64_t credit_limit;
    int64_t balance;
} customer_data;

typedef struct {
    char    order_id[32];
    int64_t total_amount;
    char    status[16];
} order_data;

static int customer_equals(const void *a, const void *b, size_t size) {
    (void)size;
    const customer_data *ca = (const customer_data *)a;
    const customer_data *cb = (const customer_data *)b;
    return strcmp(ca->customer_id, cb->customer_id) == 0
        && ca->credit_limit == cb->credit_limit;
}

int main(void) {
    printf("=== Example 1: Domain Modeling ===\n\n");

    /* 1. Ubiquitous Language Glossary */
    printf("--- Ubiquitous Language Glossary ---\n");
    dm_glossary glossary;
    dm_glossary_init(&glossary);
    dm_glossary_add(&glossary, "Customer", "A person or organization that purchases products");
    dm_glossary_add(&glossary, "Order", "A request to purchase one or more products");
    dm_glossary_add(&glossary, "Credit Limit", "Maximum amount a customer can owe");
    dm_glossary_add(&glossary, "Aggregate", "A cluster of domain objects treated as a single unit");

    for (size_t i = 0; i < 4; i++) {
        printf("  Term: %-15s -> %s\n", glossary.entries[i].term, glossary.entries[i].definition);
    }

    /* 2. Value Objects */
    printf("\n--- Value Objects ---\n");
    int amount1 = 5000, amount2 = 5000, amount3 = 10000;
    dm_value_object vo1, vo2, vo3;
    dm_value_object_init(&vo1, "Money", &amount1, sizeof(int), NULL);
    dm_value_object_init(&vo2, "Money", &amount2, sizeof(int), NULL);
    dm_value_object_init(&vo3, "Money", &amount3, sizeof(int), NULL);

    printf("  vo1 == vo2: %s\n", dm_value_object_equals(&vo1, &vo2) ? "true" : "false");
    printf("  vo1 == vo3: %s\n", dm_value_object_equals(&vo1, &vo3) ? "true" : "false");

    /* 3. Entities */
    printf("\n--- Entities ---\n");
    customer_data cust_a = {"C001", "Alice Corp", 100000, 25000};
    customer_data cust_b = {"C002", "Bob Ltd", 500000, 120000};
    dm_entity e1, e2, e3;
    dm_entity_init(&e1, 1001, "Customer", &cust_a, sizeof(cust_a));
    dm_entity_init(&e2, 1002, "Customer", &cust_b, sizeof(cust_b));
    dm_entity_init(&e3, 1001, "Customer", &cust_a, sizeof(cust_a));

    printf("  Entity %llu: %s (%lld credit)\n",
           (unsigned long long)e1.id, cust_a.name, (long long)cust_a.credit_limit);
    printf("  Entity %llu: %s (%lld credit)\n",
           (unsigned long long)e2.id, cust_b.name, (long long)cust_b.credit_limit);
    printf("  e1 same identity as e3: %s\n", dm_entity_equals(&e1, &e3) ? "true" : "false");

    /* 4. Aggregate Root with Domain Events */
    printf("\n--- Aggregate Root & Domain Events ---\n");
    order_data order = {"ORD-1001", 75000, "CREATED"};
    dm_aggregate_root order_ar;
    dm_aggregate_root_init(&order_ar, 2001, "Order", &order, sizeof(order));

    char payload_buf[DM_MAX_EVENT_PAYLOAD];
    snprintf(payload_buf, sizeof(payload_buf), "Order amount: %lld", (long long)order.total_amount);

    dm_domain_event ev1, ev2;
    dm_domain_event_init(&ev1, 1, "OrderCreated", 2001, "Order", payload_buf, strlen(payload_buf));
    dm_domain_event_init(&ev2, 2, "OrderApproved", 2001, "Order", "approved", 8);

    dm_aggregate_root_add_event(&order_ar, &ev1);
    dm_aggregate_root_add_event(&order_ar, &ev2);

    size_t ev_count = 0;
    const dm_domain_event *events = dm_aggregate_root_get_events(&order_ar, &ev_count);
    printf("  Aggregate %llu has %zu pending events:\n", (unsigned long long)order_ar.id, ev_count);
    for (size_t i = 0; i < ev_count; i++) {
        printf("    Event %llu: %s (aggregate=%llu, type=%s)\n",
               (unsigned long long)events[i].event_id, events[i].event_name,
               (unsigned long long)events[i].aggregate_id, events[i].aggregate_type);
    }

    /* 5. Context Mapping */
    printf("\n--- Context Mapping ---\n");
    dm_bounded_context sales_ctx, billing_ctx, shipping_ctx;
    dm_bounded_context_init(&sales_ctx, "Sales");
    dm_bounded_context_init(&billing_ctx, "Billing");
    dm_bounded_context_init(&shipping_ctx, "Shipping");

    dm_context_map cm1, cm2, cm3;
    dm_context_map_init(&cm1, "Sales-Billing", "Sales", "Billing", DM_MAP_CUSTOMER_SUPPLIER);
    dm_context_map_init(&cm2, "Billing-Shipping", "Billing", "Shipping", DM_MAP_PARTNERSHIP);
    dm_context_map_init(&cm3, "Sales-Shipping", "Sales", "Shipping", DM_MAP_SHARED_KERNEL);

    sales_ctx.glossary = glossary;
    dm_bounded_context_add_aggregate(&sales_ctx, &order_ar);
    dm_bounded_context_add_map(&sales_ctx, &cm1);
    dm_bounded_context_add_map(&sales_ctx, &cm3);

    printf("  Bounded Context: %s (aggregates=%zu, maps=%zu)\n",
           sales_ctx.name, sales_ctx.aggregate_count, sales_ctx.mapping_count);
    printf("    Glossary terms: %zu\n", sales_ctx.glossary.count);
    for (size_t i = 0; i < sales_ctx.mapping_count; i++) {
        printf("    Map: %s [%s] upstream=%s -> downstream=%s\n",
               sales_ctx.mappings[i].name,
               dm_context_map_type_name(sales_ctx.mappings[i].map_type),
               sales_ctx.mappings[i].upstream,
               sales_ctx.mappings[i].downstream);
    }

    printf("\n=== Example 1 Complete ===\n");
    return 0;
}
