#include "project_charter.h"
#include "project_risk.h"
#include "project_decision.h"
#include "project_stakeholder.h"
#include "project_compliance.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static int tests = 0, passed = 0;
#define TEST(name) do { tests++; printf("  TEST %s... ", name); } while(0)
#define OK() do { passed++; printf("OK\n"); } while(0)

int main(void) {
    printf("=== Project Governance Tests ===\n\n");

    /* Charter */
    printf("charter:\n");
    TEST("init"); { ProjectCharter pc; charter_init(&pc,"P","S","M");
      assert(strcmp(pc.project_name,"P")==0); assert(pc.status == CS_PROPOSED); OK(); }
    TEST("add objective"); { ProjectCharter pc; charter_init(&pc,"P","S","M");
      assert(charter_add_objective(&pc,"Reduce time",50.0) == 0); OK(); }
    TEST("progress"); { ProjectCharter pc; charter_init(&pc,"P","S","M");
      charter_add_objective(&pc,"A",100.0); charter_add_objective(&pc,"B",100.0);
      charter_update_objective(&pc,0,50.0); charter_update_objective(&pc,1,30.0);
      assert(charter_progress_pct(&pc) == 40.0); OK(); }
    TEST("on budget"); { ProjectCharter pc; charter_init(&pc,"P","S","M");
      pc.budget_total=100; pc.budget_spent=80; assert(charter_is_on_budget(&pc)); OK(); }
    TEST("on schedule"); { ProjectCharter pc; charter_init(&pc,"P","S","M");
      pc.timeline_days=100; pc.days_elapsed=90; assert(charter_is_on_schedule(&pc)); OK(); }
    TEST("max objectives"); { ProjectCharter pc; charter_init(&pc,"P","S","M");
      for (int i=0;i<MAX_OBJECTIVES;i++) charter_add_objective(&pc,"X",1.0);
      assert(charter_add_objective(&pc,"overflow",1.0) == -1); OK(); }
    TEST("scope in/out"); { ProjectCharter pc; charter_init(&pc,"P","S","M");
      charter_add_scope(&pc,"Feature A",true); charter_add_scope(&pc,"Feature B",false);
      assert(pc.scope[0].in_scope); assert(!pc.scope[1].in_scope); OK(); }

    /* Risk */
    printf("risk:\n");
    TEST("add risk"); { RiskRegistry rr; risk_registry_init(&rr,50);
      assert(risk_add(&rr,"R01","Test",RPROB_LIKELY,RSEV_HIGH) == 0);
      assert(rr.risks[0].risk_score == 12); OK(); }
    TEST("exposure"); { RiskRegistry rr; risk_registry_init(&rr,50);
      risk_add(&rr,"R01","A",RPROB_POSSIBLE,RSEV_HIGH); /* 9 */
      risk_add(&rr,"R02","B",RPROB_LIKELY,RSEV_MEDIUM); /* 8 */
      risk_update_status(&rr,"R02",RSTAT_MITIGATED);
      assert(risk_exposure(&rr) == 9); OK(); }
    TEST("within appetite"); { RiskRegistry rr; risk_registry_init(&rr,20);
      risk_add(&rr,"R01","A",RPROB_POSSIBLE,RSEV_HIGH);
      assert(risk_within_appetite(&rr)); OK(); }
    TEST("over appetite"); { RiskRegistry rr; risk_registry_init(&rr,3);
      risk_add(&rr,"R01","A",RPROB_LIKELY,RSEV_CRITICAL); /* 16 */
      assert(!risk_within_appetite(&rr)); OK(); }
    TEST("top N"); { RiskRegistry rr; risk_registry_init(&rr,50);
      risk_add(&rr,"R01","Low",RPROB_RARE,RSEV_LOW);
      risk_add(&rr,"R02","High",RPROB_ALMOST_CERTAIN,RSEV_CRITICAL);
      int indices[5];
      assert(risk_top_n(&rr,1,indices) == 1);
      assert(indices[0] == 1); OK(); }

    /* Decision */
    printf("decision:\n");
    TEST("add decision"); { DecisionLog dl; decision_log_init(&dl);
      assert(decision_add(&dl,"D1","Title","Context") == 0); OK(); }
    TEST("add option"); { DecisionLog dl; decision_log_init(&dl);
      decision_add(&dl,"D1","T","C");
      assert(decision_add_option(&dl,0,"Opt1","pros","cons") == 0); OK(); }
    TEST("make decision"); { DecisionLog dl; decision_log_init(&dl);
      decision_add(&dl,"D1","T","C"); decision_add_option(&dl,0,"O1","p","c");
      decision_make(&dl,0,0,"Alice","2026-01-01");
      assert(dl.decisions[0].chosen_option == 0);
      assert(dl.decisions[0].status == DSTAT_ACCEPTED); OK(); }
    TEST("pending count"); { DecisionLog dl; decision_log_init(&dl);
      decision_add(&dl,"D1","T","C"); decision_add(&dl,"D2","T2","C");
      assert(decision_pending_count(&dl) == 2); OK(); }

    /* Stakeholder */
    printf("stakeholder:\n");
    TEST("add stakeholder"); { StakeholderMap sm; stakeholder_map_init(&sm);
      assert(stakeholder_add(&sm,"Alice","CTO",SINFL_HIGH,SINTEREST_HIGH) == 0); OK(); }
    TEST("key stakeholder"); { StakeholderMap sm; stakeholder_map_init(&sm);
      stakeholder_add(&sm,"Alice","CTO",SINFL_HIGH,SINTEREST_HIGH);
      stakeholder_set_key(&sm,0,true); assert(sm.stakeholders[0].is_key); OK(); }
    TEST("RACI"); { StakeholderMap sm; stakeholder_map_init(&sm);
      stakeholder_add(&sm,"Alice","CTO",SINFL_HIGH,SINTEREST_HIGH);
      stakeholder_add(&sm,"Bob","TL",SINFL_MEDIUM,SINTEREST_HIGH);
      raci_add(&sm,"Design");
      raci_assign(&sm,0,0,RACI_A); raci_assign(&sm,0,1,RACI_R);
      assert(sm.raci[0].roles[0] == RACI_A);
      assert(sm.raci[0].responsible_idx == 1); OK(); }
    TEST("count by influence"); { StakeholderMap sm; stakeholder_map_init(&sm);
      stakeholder_add(&sm,"A","",SINFL_HIGH,SINTEREST_LOW);
      stakeholder_add(&sm,"B","",SINFL_HIGH,SINTEREST_LOW);
      stakeholder_add(&sm,"C","",SINFL_LOW,SINTEREST_LOW);
      assert(stakeholder_count_by_influence(&sm,SINFL_HIGH) == 2); OK(); }

    /* Compliance */
    printf("compliance:\n");
    TEST("init"); { CompliancePlan cp; compliance_init(&cp,"SOC2");
      assert(strcmp(cp.standard,"SOC2")==0); OK(); }
    TEST("add gate"); { CompliancePlan cp; compliance_init(&cp,"ISO");
      assert(compliance_add_gate(&cp,"Design","Review") == 0); OK(); }
    TEST("add check"); { CompliancePlan cp; compliance_init(&cp,"ISO");
      compliance_add_gate(&cp,"Design",""); assert(compliance_add_check(&cp,0,"Check1",true) == 0); OK(); }
    TEST("pass check"); { CompliancePlan cp; compliance_init(&cp,"ISO");
      compliance_add_gate(&cp,"Design",""); compliance_add_check(&cp,0,"Check1",true);
      compliance_pass_check(&cp,0,0,"evidence.txt");
      assert(cp.gates[0].checks[0].passed); OK(); }
    TEST("gate ready"); { CompliancePlan cp; compliance_init(&cp,"ISO");
      compliance_add_gate(&cp,"G",""); compliance_add_check(&cp,0,"C1",true);
      assert(!compliance_gate_ready(&cp,0));
      compliance_pass_check(&cp,0,0,"e"); assert(compliance_gate_ready(&cp,0)); OK(); }
    TEST("all gates passed"); { CompliancePlan cp; compliance_init(&cp,"ISO");
      compliance_add_gate(&cp,"G1",""); compliance_add_gate(&cp,"G2","");
      compliance_add_check(&cp,0,"C1",true); compliance_pass_check(&cp,0,0,"e");
      compliance_review_gate(&cp,0,"Rev");
      assert(!compliance_all_gates_passed(&cp)); OK(); }
    TEST("audit log"); { CompliancePlan cp; compliance_init(&cp,"ISO");
      compliance_audit_log(&cp,"Design approved","Alice","DRAFT","APPROVED");
      assert(cp.audit_count == 1); OK(); }

    printf("\n=== Results: %d/%d passed ===\n", passed, tests);
    return passed == tests ? 0 : 1;
}
