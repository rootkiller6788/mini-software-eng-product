#ifndef BIZ_REPORT_H
#define BIZ_REPORT_H
#include <stdint.h>
#include <stdbool.h>

#define REP_MAX_ROWS 128
#define REP_MAX_COLS 8

typedef enum { REP_SALES, REP_INVENTORY, REP_CUSTOMER, REP_REVENUE } ReportType;

typedef struct { char headers[REP_MAX_COLS][32]; int col_count; double data[REP_MAX_ROWS][REP_MAX_COLS]; int row_count; ReportType type; char title[64]; } BizReport;

typedef struct { double total_revenue; double total_cost; double gross_margin; double avg_order_value; int total_orders; int total_customers; } SalesSummary;

typedef struct { int product_id; char name[48]; int stock_level; int reorder_point; double unit_cost; double total_value; } InventoryItem;

void report_init(BizReport *r, ReportType type, const char *title);
void report_set_header(BizReport *r, int col, const char *name);
void report_add_row(BizReport *r, double *values, int n);
void report_summarize_sales(BizReport *r, SalesSummary *ss);
void report_inventory_value(BizReport *r, InventoryItem *items, int count);
void report_print(BizReport *r);
void report_export_csv(BizReport *r, const char *filename);
#endif
