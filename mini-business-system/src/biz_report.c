#include "biz_report.h"
#include <stdio.h>
#include <string.h>

void report_init(BizReport *r, ReportType type, const char *title) { memset(r, 0, sizeof(*r)); r->type = type; strncpy(r->title, title, 63); r->title[63]='\0'; }

void report_set_header(BizReport *r, int col, const char *name) { if (col < REP_MAX_COLS) { strncpy(r->headers[col], name, 31); r->headers[col][31]='\0'; if (col >= r->col_count) r->col_count = col + 1; } }

void report_add_row(BizReport *r, double *values, int n) {
    if (r->row_count >= REP_MAX_ROWS) return;
    for (int i = 0; i < n && i < REP_MAX_COLS; i++) r->data[r->row_count][i] = values[i];
    r->row_count++;
}

void report_summarize_sales(BizReport *r, SalesSummary *ss) {
    memset(ss, 0, sizeof(*ss));
    if (r->type != REP_SALES) return;
    for (int i = 0; i < r->row_count; i++) { ss->total_revenue += r->data[i][0]; ss->total_cost += r->data[i][1]; if (r->col_count > 3) ss->total_orders++; }
    ss->gross_margin = ss->total_revenue > 0 ? (ss->total_revenue - ss->total_cost) / ss->total_revenue * 100 : 0;
    ss->avg_order_value = ss->total_orders > 0 ? ss->total_revenue / ss->total_orders : 0;
}

void report_inventory_value(BizReport *r, InventoryItem *items, int count) {
    report_init(r, REP_INVENTORY, "Inventory Valuation");
    report_set_header(r, 0, "Product"); report_set_header(r, 1, "Stock"); report_set_header(r, 2, "UnitCost"); report_set_header(r, 3, "TotalValue");
    for (int i = 0; i < count && r->row_count < REP_MAX_ROWS; i++) { double row[4] = {(double)items[i].product_id, (double)items[i].stock_level, items[i].unit_cost, items[i].stock_level * items[i].unit_cost}; strncpy(r->headers[0], items[i].name, 31); report_add_row(r, row+1, 3); }
}

void report_print(BizReport *r) {
    printf("=== %s ===\n", r->title);
    for (int c = 0; c < r->col_count; c++) printf("%-12s ", r->headers[c]); printf("\n");
    for (int i = 0; i < r->row_count; i++) { for (int c = 0; c < r->col_count; c++) printf("%-12.2f ", r->data[i][c]); printf("\n"); }
}

void report_export_csv(BizReport *r, const char *filename) {
    FILE *f = fopen(filename, "w"); if (!f) return;
    for (int c = 0; c < r->col_count; c++) { if (c > 0) fputc(',', f); fprintf(f, "%s", r->headers[c]); }
    fputc('\n', f);
    for (int i = 0; i < r->row_count; i++) { for (int c = 0; c < r->col_count; c++) { if (c > 0) fputc(',', f); fprintf(f, "%.2f", r->data[i][c]); } fputc('\n', f); }
    fclose(f);
}
