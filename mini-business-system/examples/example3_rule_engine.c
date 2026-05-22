/* ============================================================
 * Example 3: Rule Engine — Forward Chaining & Decision Table
 * ============================================================ */

#include "rule_engine.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("=== Example 3: Rule Engine ===\n\n");

    /* ---- Part A: Production Rules (Forward Chaining) ---- */
    printf("--- Production Rules ---\n");

    re_engine eng;
    re_engine_init(&eng);

    /* Rule 1: High-value customer discount */
    re_rule r1;
    re_rule_init(&r1, "HighValueDiscount", 10);
    re_condition c1a;
    re_condition_init(&c1a, "total_orders", RE_OP_GT, 100000);
    re_rule_add_condition(&r1, &c1a);
    re_condition c1b;
    re_condition_init(&c1b, "loyalty_years", RE_OP_GTE, 3);
    re_rule_add_condition(&r1, &c1b);
    re_action a1;
    re_action_init_print(&a1, "Applying 15% VIP discount!");
    re_rule_add_action(&r1, &a1);
    re_action a1m;
    re_action_init_modify(&a1m, "discount_rate", 15);
    re_rule_add_action(&r1, &a1m);
    re_engine_add_rule(&eng, &r1);

    /* Rule 2: New customer welcome discount */
    re_rule r2;
    re_rule_init(&r2, "NewCustomerDiscount", 5);
    re_condition c2;
    re_condition_init(&c2, "loyalty_years", RE_OP_LT, 1);
    re_rule_add_condition(&r2, &c2);
    re_action a2;
    re_action_init_print(&a2, "Applying 5% new customer discount!");
    re_rule_add_action(&r2, &a2);
    re_action a2m;
    re_action_init_modify(&a2m, "discount_rate", 5);
    re_rule_add_action(&r2, &a2m);
    re_engine_add_rule(&eng, &r2);

    /* Rule 3: Regular customer */
    re_rule r3;
    re_rule_init(&r3, "RegularDiscount", 3);
    re_condition c3;
    re_condition_init(&c3, "total_orders", RE_OP_GT, 10000);
    re_rule_add_condition(&r3, &c3);
    re_action a3;
    re_action_init_print(&a3, "Applying 8% regular discount!");
    re_rule_add_action(&r3, &a3);
    re_action a3m;
    re_action_init_modify(&a3m, "discount_rate", 8);
    re_rule_add_action(&r3, &a3m);
    re_engine_add_rule(&eng, &r3);

    /* Insert facts */
    re_fact f1;
    re_fact_init(&f1, 1, "CustomerProfile");
    re_fact_set_field(&f1, "total_orders", 150000);
    re_fact_set_field(&f1, "loyalty_years", 5);
    re_engine_add_fact(&eng, &f1);

    /* Run engine */
    printf("Running rules...\n");
    re_engine_run(&eng);

    printf("Discount rate: %lld%%\n",
           (long long)re_fact_get_field(&f1, "discount_rate", 0));

    /* Audit trail */
    size_t audit_count = 0;
    const re_audit_entry *audit = re_engine_get_audit(&eng, &audit_count);
    printf("\nAudit trail (%zu entries):\n", audit_count);
    for (size_t i = 0; i < audit_count; i++) {
        printf("  [%lld] %s -> %s (fired=%d)\n",
               (long long)audit[i].timestamp, audit[i].rule_name,
               audit[i].detail, audit[i].fired);
    }

    /* ---- Part B: Decision Table ---- */
    printf("\n--- Decision Table ---\n");

    re_decision_table dt;
    re_decision_table_init(&dt, "ShippingMethod");
    re_decision_table_add_condition_col(&dt, "order_amount");
    re_decision_table_add_condition_col(&dt, "is_premium");

    /* Rules:
     *   amount < 500, not premium  -> Standard (1)
     *   amount < 500, premium      -> Express (2)
     *   amount >= 500, not premium -> Express (2)
     *   amount >= 500, premium     -> NextDay (3)
     */
    int64_t row1[] = {0, 0};    re_decision_table_add_row(&dt, row1, 2, 1);
    int64_t row2[] = {0, 1};    re_decision_table_add_row(&dt, row2, 2, 2);
    int64_t row3[] = {500, 0};  re_decision_table_add_row(&dt, row3, 2, 2);
    int64_t row4[] = {500, 1};  re_decision_table_add_row(&dt, row4, 2, 3);

    const char *methods[] = {"", "Standard Shipping", "Express Shipping", "NextDay Delivery"};

    re_fact cart1, cart2, cart3;
    re_fact_init(&cart1, 100, "Cart");
    re_fact_set_field(&cart1, "order_amount", 200);
    re_fact_set_field(&cart1, "is_premium", 0);
    printf("  Cart $200, not premium -> %s\n", methods[re_decision_table_evaluate(&dt, &cart1)]);

    re_fact_init(&cart2, 101, "Cart");
    re_fact_set_field(&cart2, "order_amount", 200);
    re_fact_set_field(&cart2, "is_premium", 1);
    printf("  Cart $200, premium     -> %s\n", methods[re_decision_table_evaluate(&dt, &cart2)]);

    re_fact_init(&cart3, 102, "Cart");
    re_fact_set_field(&cart3, "order_amount", 600);
    re_fact_set_field(&cart3, "is_premium", 1);
    printf("  Cart $600, premium     -> %s\n", methods[re_decision_table_evaluate(&dt, &cart3)]);

    /* ---- Part C: Rule Versioning ---- */
    printf("\n--- Rule Versioning ---\n");
    re_rule_versioning rv;
    re_rule_versioning_init(&rv, "DiscountRule");
    re_rule_versioning_add_version(&rv, 1);
    re_rule_versioning_add_version(&rv, 2);
    re_rule_versioning_add_version(&rv, 3);
    rv.active_version = 2;
    printf("  Rule '%s': %zu versions, active=v%d\n",
           rv.rule_name, rv.version_count, rv.active_version);

    printf("\n=== Example 3 Complete ===\n");
    return 0;
}
