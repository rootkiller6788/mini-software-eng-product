#include "product_growth.h"
#include "product_economics.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

static int tests = 0, passed = 0;
#define TEST(n) do { tests++; printf("  TEST %s... ", n); } while(0)
#define OK() do { passed++; printf("OK\n"); } while(0)

int main(void) {
    printf("=== Growth + Economics Tests ===\n\n");

    printf("viral:\n");
    TEST("init"); { ViralModel vm; viral_model_init(&vm,10.0,0.05,2.0,100); assert(fabs(vm.k_factor-0.5)<0.01); OK(); }
    TEST("non-viral"); { ViralModel vm; viral_model_init(&vm,10.0,0.05,2.0,100); viral_model_simulate(&vm,5); assert(!viral_model_is_viral(&vm)); OK(); }
    TEST("viral K>1"); { ViralModel vm; viral_model_init(&vm,10.0,0.12,2.0,100); assert(viral_model_is_viral(&vm)); OK(); }
    TEST("null safety"); { viral_model_simulate(NULL,5); assert(!viral_model_is_viral(NULL)); OK(); }

    printf("ltv:\n");
    TEST("calc"); { CustomerLTV c; customer_ltv_init(&c,100,80,5,600); customer_ltv_calculate(&c); assert(fabs(c.ltv-1600)<1); OK(); }
    TEST("healthy"); { CustomerLTV c; customer_ltv_init(&c,100,80,5,400); customer_ltv_calculate(&c); assert(c.ltv_cac_ratio>=3.0); OK(); }
    TEST("unhealthy"); { CustomerLTV c; customer_ltv_init(&c,50,60,10,500); customer_ltv_calculate(&c); assert(!customer_ltv_is_healthy(&c)); OK(); }

    printf("cohort_ltv:\n");
    TEST("project"); { CohortLTVProjection cl; cohort_ltv_init(&cl,1000,0.01); cohort_ltv_record(&cl,0,100,50); cohort_ltv_record(&cl,1,90,50); cohort_ltv_record(&cl,2,80,50); cohort_ltv_project(&cl,3); assert(cohort_ltv_total(&cl)>100); OK(); }

    printf("nrr:\n");
    TEST(">100"); { NetRevenueRetention nr; nrr_init(&nr,100000); nrr_add_expansion(&nr,15000); nrr_add_contraction(&nr,3000); nrr_add_churn(&nr,5000); nrr_calculate(&nr); assert(fabs(nr.nrr_pct-107)<0.1); OK(); }
    TEST("<100"); { NetRevenueRetention nr; nrr_init(&nr,100000); nrr_add_churn(&nr,20000); nrr_calculate(&nr); assert(nr.nrr_pct<100); OK(); }

    printf("aarrr:\n");
    TEST("conv"); { AARRRMetrics am; aarrr_init(&am); aarrr_set_acquisition(&am,10000,2000,500); aarrr_calculate_conversions(&am); assert(fabs(am.visitor_to_lead_pct-20)<0.1); OK(); }
    TEST("report"); { AARRRMetrics am; aarrr_init(&am); aarrr_set_acquisition(&am,1000,200,100); aarrr_set_activation(&am,50,30); aarrr_set_retention(&am,70,50,40,30); aarrr_set_revenue(&am,29,50000); char b[1024]; aarrr_health_report(&am,b,1024); assert(strlen(b)>50); OK(); }

    printf("heart:\n");
    TEST("sticky"); { HEARTMetrics hm; heart_init(&hm); heart_set_happiness(&hm,4.2,5.5); heart_set_engagement(&hm,5000,20000,2.5); heart_calculate_stickiness(&hm); assert(fabs(hm.stickiness-0.25)<0.01); OK(); }
    TEST("report"); { HEARTMetrics hm; heart_init(&hm); heart_set_engagement(&hm,15000,25000,3); heart_calculate_stickiness(&hm); char b[1024]; heart_overall_health(&hm,b,1024); assert(strlen(b)>50); OK(); }

    printf("pmf:\n");
    TEST("achieved"); { ProductMarketFit pmf; pmf_init(&pmf); int i; for(i=0;i<50;i++) pmf_add_response(&pmf,0); for(i=0;i<50;i++) pmf_add_response(&pmf,1); pmf_evaluate(&pmf); assert(pmf.pmf_achieved); OK(); }
    TEST("not achieved"); { ProductMarketFit pmf; pmf_init(&pmf); int i; for(i=0;i<20;i++) pmf_add_response(&pmf,0); for(i=0;i<80;i++) pmf_add_response(&pmf,1); pmf_evaluate(&pmf); assert(!pmf.pmf_achieved); OK(); }

    printf("bass:\n");
    TEST("peak"); { BassDiffusion bd; bass_init(&bd,0.03,0.38,1000000); double p=bass_peak_time(&bd); assert(p>3&&p<10); OK(); }
    TEST("sim"); { BassDiffusion bd; bass_init(&bd,0.03,0.38,1000000); bass_simulate(&bd,20); assert(bd.adopters[5]>0); OK(); }

    printf("pricing:\n");
    TEST("cost+"); { PricingModel pm; pricing_init_cost_plus(&pm,50,30); pricing_calculate(&pm); assert(fabs(pm.calculated_price-65)<0.01); OK(); }
    TEST("value"); { PricingModel pm; pricing_init_value_based(&pm,200); pricing_calculate(&pm); assert(fabs(pm.calculated_price-170)<0.01); OK(); }
    TEST("compete"); { PricingModel pm; pricing_init_competitive(&pm,100,40); pricing_calculate(&pm); assert(fabs(pm.calculated_price-95)<0.01); OK(); }

    printf("unit_econ:\n");
    TEST("sust"); { UnitEconomics ue; unit_economics_init(&ue); ue.cac=300;ue.arpu=50;ue.monthly_churn_pct=3;ue.gross_margin_pct=80;ue.paid_users=800;ue.total_users=1000; unit_economics_calculate(&ue); assert(unit_economics_is_sustainable(&ue)); OK(); }

    printf("breakeven:\n");
    TEST("calc"); { BreakevenAnalysis ba; breakeven_init(&ba,10000,10,60); breakeven_calculate(&ba); assert(ba.breakeven_units==200); OK(); }
    TEST("profit"); { BreakevenAnalysis ba; breakeven_init(&ba,10000,10,60); assert(breakeven_units_for_profit(&ba,5000)==300); OK(); }
    TEST("impossible"); { BreakevenAnalysis ba; breakeven_init(&ba,10000,10,5); breakeven_calculate(&ba); assert(ba.breakeven_units==-1); OK(); }

    printf("forecast:\n");
    TEST("linear"); { RevenueForecast rf; revenue_forecast_init(&rf,FORECAST_LINEAR); int i; for(i=0;i<10;i++) revenue_forecast_add_historical(&rf,100+i*10); revenue_forecast_predict(&rf,3); assert(rf.r_squared>0.9); OK(); }

    printf("demand:\n");
    TEST("elastic"); { DemandCurve dc; demand_curve_init(&dc); demand_curve_add_point(&dc,10,1000); demand_curve_add_point(&dc,20,500); demand_curve_calculate_elasticity(&dc); assert(dc.elasticity<0); OK(); }
    TEST("optimal"); { DemandCurve dc; demand_curve_init(&dc); demand_curve_add_point(&dc,10,100); demand_curve_add_point(&dc,15,80); demand_curve_add_point(&dc,20,50); demand_curve_find_optimal(&dc); assert(fabs(dc.optimal_price-15)<0.01); OK(); }

    printf("pareto:\n");
    TEST("strong"); { double v[]={100,95,90,85,10,5,4,3,2,1,1,1,1,1,1,1,1,1,1,1}; ParetoAnalysis pa; pareto_analyze(&pa,v,20); assert(pareto_is_strong(&pa)); OK(); }

    printf("network:\n");
    TEST("metcalfe"); { NetworkEffect ne; network_effect_calculate(&ne,10,1); assert(fabs(ne.metcalfe_value-45)<0.01); OK(); }
    TEST("marginal"); { NetworkEffect ne; network_effect_calculate(&ne,10,1); assert(fabs(network_effect_marginal_value(&ne,1)-10)<0.01); OK(); }

    printf("\n=== All %d/%d tests passed ===\n", passed, tests);
    return passed == tests ? 0 : 1;
}
