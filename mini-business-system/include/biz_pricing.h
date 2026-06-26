#ifndef BIZ_PRICING_H
#define BIZ_PRICING_H
#include <stdbool.h>

#define PRICE_MAX_RULES 32
#define PRICE_MAX_TIERS 8

typedef enum { DISC_PERCENT, DISC_FIXED, DISC_BUY_ONE_GET_ONE, DISC_TIERED } DiscountType;

typedef struct { int min_qty; double price_per_unit; } PriceTier;

typedef struct {
    int id; char name[48]; DiscountType type;
    double value;        /* percentage (0-100) or fixed amount */
    int min_quantity;
    double min_order_amount;
    bool active;
    PriceTier tiers[PRICE_MAX_TIERS]; int tier_count;
} DiscountRule;

typedef struct { DiscountRule rules[PRICE_MAX_RULES]; int rule_count; } PricingEngine;

void pricing_init(PricingEngine *pe);
int  pricing_add_discount(PricingEngine *pe, const char *name, DiscountType type, double value);
int  pricing_add_tier(PricingEngine *pe, int rule_id, int min_qty, double price);
double pricing_calculate(PricingEngine *pe, double base_price, int quantity);
double pricing_apply_rule(DiscountRule *r, double base_price, int qty);
int  pricing_best_discount(PricingEngine *pe, double base_price, int qty); // returns rule index
void pricing_print(PricingEngine *pe);
#endif
