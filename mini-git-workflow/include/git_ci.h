#ifndef GIT_CI_H
#define GIT_CI_H
#include <stdbool.h>
#include <stdint.h>

typedef enum { CI_PENDING, CI_RUNNING, CI_PASSED, CI_FAILED, CI_SKIPPED } CiStatus;

typedef struct { char name[48]; char command[256]; CiStatus status; int duration_ms; } CiJob;

typedef struct { CiJob jobs[16]; int job_count; int total_duration_ms; bool all_passed; } CiPipeline;

typedef struct { bool require_ci_pass; bool require_review; bool require_linear_history; bool require_signed_commits; } BranchProtection;

void ci_pipeline_init(CiPipeline *p);
int  ci_add_job(CiPipeline *p, const char *name, const char *command);
bool ci_run_job(CiPipeline *p, int job_id);
bool ci_run_all(CiPipeline *p);
bool ci_pre_commit_check(const char *changed_files, BranchProtection *bp);
bool ci_check_branch_protection(BranchProtection *bp, bool ci_passed, bool reviewed, bool is_linear, bool is_signed);
void ci_print_pipeline(CiPipeline *p);
void ci_print_status(CiPipeline *p);
#endif
