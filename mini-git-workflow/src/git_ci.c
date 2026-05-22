#include "git_ci.h"
#include <stdio.h>
#include <string.h>

void ci_pipeline_init(CiPipeline *p) { memset(p,0,sizeof(*p)); p->all_passed=true; }

int ci_add_job(CiPipeline *p, const char *name, const char *cmd) { if (p->job_count>=16) return -1; CiJob *j=&p->jobs[p->job_count]; strncpy(j->name,name,47); strncpy(j->command,cmd,255); j->status=CI_PENDING; return p->job_count++; }

bool ci_run_job(CiPipeline *p, int jid) { if (jid>=p->job_count) return false; CiJob *j=&p->jobs[jid]; j->status=CI_RUNNING; j->duration_ms=100; /* simulated */ j->status=CI_PASSED; p->total_duration_ms+=j->duration_ms; return true; }

bool ci_run_all(CiPipeline *p) { p->all_passed=true; for (int i=0; i<p->job_count; i++) { ci_run_job(p,i); if (p->jobs[i].status==CI_FAILED) p->all_passed=false; } return p->all_passed; }

bool ci_pre_commit_check(const char *changed_files, BranchProtection *bp) { (void)changed_files; (void)bp; return true; }

bool ci_check_branch_protection(BranchProtection *bp, bool ci_pass, bool reviewed, bool linear, bool signed_) {
    if (bp->require_ci_pass && !ci_pass) return false;
    if (bp->require_review && !reviewed) return false;
    if (bp->require_linear_history && !linear) return false;
    if (bp->require_signed_commits && !signed_) return false;
    return true;
}

void ci_print_pipeline(CiPipeline *p) {
    printf("=== CI Pipeline ===\n");
    for (int i=0; i<p->job_count; i++) { const char *ss[]={"PEND","RUN","PASS","FAIL","SKIP"}; printf("  %s: %s (%dms)\n",p->jobs[i].name,ss[p->jobs[i].status],p->jobs[i].duration_ms); }
}

void ci_print_status(CiPipeline *p) { printf("  CI: %s (%dms total)\n", p->all_passed?"PASSED":"FAILED", p->total_duration_ms); }
