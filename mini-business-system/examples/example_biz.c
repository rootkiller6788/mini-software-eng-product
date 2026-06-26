#include "biz_entity.h"
#include "biz_crud.h"
#include "biz_workflow.h"
#include "biz_pricing.h"
#include "biz_report.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("=== Business System Demo ===\n\n");
    /* CRUD */ CrudRepo cust; crud_init(&cust, "Customer");
    entity_def_add_field(&cust.def, "name", FT_STRING, true);
    entity_def_add_field(&cust.def, "email", FT_STRING, false);
    int c1 = crud_create(&cust);
    crud_update(&cust, c1, 0, "Alice"); crud_update(&cust, c1, 1, "alice@example.com");
    int c2 = crud_create(&cust);
    crud_update(&cust, c2, 0, "Bob"); crud_update(&cust, c2, 1, "bob@example.com");
    crud_print_all(&cust);
    /* Workflow */ WorkflowEngine wf; wf_init(&wf, "OrderProcess");
    int s1 = wf_add_state(&wf, "Draft", WF_PENDING, true, false);
    int s2 = wf_add_state(&wf, "Review", WF_IN_PROGRESS, false, false);
    int s3 = wf_add_state(&wf, "Approved", WF_COMPLETED, false, true);
    int s4 = wf_add_state(&wf, "Rejected", WF_REJECTED, false, true);
    wf_add_transition(&wf, "Submit", s1, s2, NULL, NULL);
    wf_add_transition(&wf, "Approve", s2, s3, NULL, NULL);
    wf_add_transition(&wf, "Reject", s2, s4, NULL, NULL);
    wf_reset(&wf); wf_execute(&wf, 0, NULL);
    printf("\n"); wf_print(&wf);
    wf_execute(&wf, 1, NULL);
    printf("  After approve: %s\n", wf.states[wf.current_state].name);
    /* Pricing */ PricingEngine pe; pricing_init(&pe);
    pricing_add_discount(&pe, "10% off", DISC_PERCENT, 10);
    pricing_add_discount(&pe, "VIP tier", DISC_TIERED, 0);
    pricing_add_tier(&pe, 1, 10, 8.0); pricing_add_tier(&pe, 1, 50, 6.0);
    printf("\n"); pricing_print(&pe);
    printf("  Best price for 30 items @ $10: $%.2f\n", pricing_calculate(&pe, 10.0, 30));
    /* Report */ BizReport r; report_init(&r, REP_SALES, "Monthly Sales");
    report_set_header(&r, 0, "Revenue"); report_set_header(&r, 1, "Cost");
    double d1[]={5000,2000}; report_add_row(&r,d1,2);
    double d2[]={3000,1200}; report_add_row(&r,d2,2);
    SalesSummary ss; report_summarize_sales(&r, &ss);
    printf("\n"); report_print(&r);
    printf("  Margin: %.1f%%, Avg Order: $%.2f\n", ss.gross_margin, ss.avg_order_value);
    return 0;
}
