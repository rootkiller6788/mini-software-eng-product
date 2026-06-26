#ifndef GIT_CI_H
#define GIT_CI_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "git_commit.h"

/* ================================================================
 * L1-L7: CI/CD Pipeline — Continuous Integration & Delivery
 *
 * L6: Canonical Problem — CI pipeline for automated testing
 * L7: Applications — GitHub Actions / GitLab CI simulation
 *
 * Pipeline stages: build → test → analyze → stage → deploy
 *
 * Courses: Stanford CS 144, Georgia Tech CS 6300
 * ================================================================ */

#define CI_MAX_PIPELINES     8
#define CI_MAX_STAGES        8
#define CI_MAX_JOBS          16
#define CI_MAX_ARTIFACTS     16
#define CI_ARTIFACT_PATH_MAX 128
#define CI_JOB_NAME_MAX      64
#define CI_STAGE_NAME_MAX    32
#define CI_LOG_LINE_MAX      128
#define CI_MAX_LOG_LINES     64

/* L1: Pipeline status */
typedef enum {
    CI_STATUS_PENDING     = 0,
    CI_STATUS_RUNNING     = 1,
    CI_STATUS_SUCCESS     = 2,
    CI_STATUS_FAILED      = 3,
    CI_STATUS_CANCELED    = 4,
    CI_STATUS_SKIPPED     = 5,
    CI_STATUS_TIMED_OUT   = 6,
    CI_STATUS_MANUAL      = 7   /* waiting for manual trigger */
} CIStatus;

/* L1: Stage types */
typedef enum {
    CI_STAGE_BUILD        = 0,
    CI_STAGE_TEST         = 1,
    CI_STAGE_LINT         = 2,
    CI_STAGE_SAST         = 3,  /* static analysis */
    CI_STAGE_SCA          = 4,  /* software composition analysis */
    CI_STAGE_CONTAINER    = 5,
    CI_STAGE_STAGING      = 6,
    CI_STAGE_DEPLOY       = 7,
    CI_STAGE_E2E_TEST     = 8,  /* end-to-end test */
    CI_STAGE_PERF         = 9   /* performance test */
} CIStageType;

/* L1: A single job within a stage */
typedef struct {
    int             id;
    char            name[CI_JOB_NAME_MAX];
    CIStageType     stage;
    CIStatus        status;
    time_t          started_at;
    time_t          finished_at;
    int             duration_seconds;
    int             exit_code;
    char            log[CI_MAX_LOG_LINES][CI_LOG_LINE_MAX];
    int             log_line_count;
    bool            allow_failure;    /* job can fail without failing pipeline */
    int             retry_count;
    int             max_retries;
    int             dependencies[16]; /* job ids this depends on */
    int             dep_count;
} CIJob;

/* L1: A stage in the pipeline */
typedef struct {
    int             id;
    char            name[CI_STAGE_NAME_MAX];
    CIStageType     type;
    CIStatus        status;
    CIJob           jobs[CI_MAX_JOBS];
    int             job_count;
    bool            required;         /* must pass to continue */
    int             order;            /* execution order */
} CIStage;

/* L1: A build artifact */
typedef struct {
    int             id;
    char            name[CI_ARTIFACT_PATH_MAX];
    char            path[CI_ARTIFACT_PATH_MAX];
    int             size_bytes;
    int             job_id;           /* which job produced it */
    time_t          created_at;
    int             expire_days;      /* auto-delete after N days */
} CIArtifact;

/* L1: Complete pipeline */
typedef struct {
    int             id;
    int             pr_id;            /* associated PR */
    int             branch_id;
    char            commit_sha[COMMIT_SHA_MAX_LEN];
    CIStatus        status;
    CIStage         stages[CI_MAX_STAGES];
    int             stage_count;
    CIArtifact      artifacts[CI_MAX_ARTIFACTS];
    int             artifact_count;
    time_t          created_at;
    time_t          updated_at;
    int             total_duration_seconds;
    bool            is_automatic;     /* triggered by push/PR */
    int             trigger_user_id;  /* if manual trigger */
} CIPipeline;

/* L3: CI Manager */
typedef struct {
    CIPipeline      pipelines[CI_MAX_PIPELINES];
    int             pipeline_count;
    int             next_pipeline_id;
    int             next_job_id;
    int             next_artifact_id;
} CIManager;

/* === API Declarations === */

/* Pipeline Lifecycle */
void        ci_manager_init(CIManager *cm);
int         ci_create_pipeline(CIManager *cm, int pr_id, int branch_id,
                               const char *commit_sha);
bool        ci_trigger_pipeline(CIManager *cm, int pipeline_id);
bool        ci_cancel_pipeline(CIManager *cm, int pipeline_id);
bool        ci_retry_pipeline(CIManager *cm, int pipeline_id);
bool        ci_retry_job(CIManager *cm, int pipeline_id, int job_id);

/* Stage & Job Management */
int         ci_add_stage(CIManager *cm, int pipeline_id, const char *name,
                         CIStageType type, int order, bool required);
int         ci_add_job(CIManager *cm, int pipeline_id, int stage_id,
                       const char *name, bool allow_failure, int max_retries);
bool        ci_set_job_status(CIManager *cm, int pipeline_id, int job_id,
                              CIStatus status, int exit_code);
bool        ci_add_job_log(CIManager *cm, int pipeline_id, int job_id,
                           const char *line);
bool        ci_set_job_dependency(CIManager *cm, int pipeline_id,
                                  int job_id, int depends_on_job_id);

/* Status Queries */
CIStatus    ci_get_pipeline_status(CIManager *cm, int pipeline_id);
CIStatus    ci_get_stage_status(CIManager *cm, int pipeline_id, int stage_id);
CIStatus    ci_get_job_status(CIManager *cm, int pipeline_id, int job_id);
bool        ci_is_pipeline_green(CIManager *cm, int pipeline_id);
bool        ci_is_stage_green(CIManager *cm, int pipeline_id, int stage_id);
int         ci_count_failed_jobs(CIManager *cm, int pipeline_id);
int         ci_count_passed_jobs(CIManager *cm, int pipeline_id);

/* Pipeline Stage Progression */
bool        ci_advance_stage(CIManager *cm, int pipeline_id);
bool        ci_stage_can_proceed(CIManager *cm, int pipeline_id, int stage_id);

/* Artifact Management */
int         ci_add_artifact(CIManager *cm, int pipeline_id, const char *name,
                            const char *path, int size_bytes, int job_id);
int         ci_find_artifacts(CIManager *cm, int pipeline_id, const char *name_pattern,
                              int *results, int max_results);

/* Gate Checks (L7: deployment gate) */
bool        ci_deploy_gate_check(CIManager *cm, int pipeline_id);
bool        ci_rollback_check(CIManager *cm, int pipeline_id, int *prev_green_pipeline);

/* Pipeline Config Generation */
bool        ci_generate_default_pipeline(CIManager *cm, int pr_id, int branch_id,
                                         const char *commit_sha);

/* Print & Debug */
void        ci_print_pipeline_status(const CIManager *cm, int pipeline_id);
void        ci_print_job_log(const CIManager *cm, int pipeline_id, int job_id);
void        ci_print_summary(const CIManager *cm, int pipeline_id);
const char* ci_status_name(CIStatus status);
const char* ci_stage_type_name(CIStageType type);

/* L8: Advanced — Parallel job scheduling simulation */
bool        ci_schedule_parallel_jobs(CIManager *cm, int pipeline_id);
int         ci_get_critical_path_duration(CIManager *cm, int pipeline_id);

#endif
