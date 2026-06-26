#include "biz_pricing.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

void pricing_init(PricingEngine *pe) { memset(pe, 0, sizeof(*pe)); }

int pricing_add_discount(PricingEngine *pe, const char *name, DiscountType type, double value) {
    if (pe->rule_count >= PRICE_MAX_RULES) return -1;
    DiscountRule *r = &pe->rules[pe->rule_count]; r->id = pe->rule_count; r->type = type; r->value = value; r->active = true; r->min_quantity = 1; r->min_order_amount = 0; r->tier_count = 0;
    strncpy(r->name, name, 47); r->name[47]='\0';
    return pe->rule_count++;
}

int pricing_add_tier(PricingEngine *pe, int rid, int min_qty, double price) {
    if (rid >= pe->rule_count || pe->rules[rid].tier_count >= PRICE_MAX_TIERS) return -1;
    PriceTier *t = &pe->rules[rid].tiers[pe->rules[rid].tier_count++]; t->min_qty = min_qty; t->price_per_unit = price;
    return pe->rules[rid].tier_count;
}

double pricing_apply_rule(DiscountRule *r, double base_price, int qty) {
    double total = base_price * qty;
    switch (r->type) {
        case DISC_PERCENT: return total * (1.0 - r->value / 100.0);
        case DISC_FIXED: return fmax(total - r->value, 0);
        case DISC_BUY_ONE_GET_ONE: return base_price * ((qty + 1) / 2);
        case DISC_TIERED: {
            double result = 0; int remaining = qty;
            for (int i = r->tier_count - 1; i >= 0; i--) { if (qty >= r->tiers[i].min_qty) { result = qty * r->tiers[i].price_per_unit; break; } }
            if (result == 0 && r->tier_count > 0) result = qty * r->tiers[0].price_per_unit;
            return result;
        }
    }
    return total;
}

double pricing_calculate(PricingEngine *pe, double base_price, int qty) {
    double best = base_price * qty;
    for (int i = 0; i < pe->rule_count; i++) { if (!pe->rules[i].active) continue; double p = pricing_apply_rule(&pe->rules[i], base_price, qty); if (p < best) best = p; }
    return best;
}

int pricing_best_discount(PricingEngine *pe, double base_price, int qty) {
    int best_idx = -1; double best_price = base_price * qty;
    for (int i = 0; i < pe->rule_count; i++) { if (!pe->rules[i].active) continue; double p = pricing_apply_rule(&pe->rules[i], base_price, qty); if (p < best_price) { best_price = p; best_idx = i; } }
    return best_idx;
}

void pricing_print(PricingEngine *pe) {
    printf("=== Pricing Engine (%d rules) ===\n", pe->rule_count);
    for (int i = 0; i < pe->rule_count; i++) { const char *ts[]={"%","Fixed","BOGO","Tiered"}; printf("  %s: %s %.2f\n", pe->rules[i].name, ts[pe->rules[i].type], pe->rules[i].value); }
}
