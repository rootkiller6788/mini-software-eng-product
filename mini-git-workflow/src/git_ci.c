#include "git_ci.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ================================================================
 * L2-L7: CI/CD Pipeline Implementation
 *
 * L2: Pipeline stage concepts — build, test, deploy
 * L6: Canonical Problem — automated CI pipeline
 * L7: Applications — GitHub Actions / GitLab CI simulation
 * L8: Advanced — parallel job scheduling, critical path analysis
 *
 * Courses: Stanford CS 144, Georgia Tech CS 6300
 * ================================================================ */

void ci_manager_init(CIManager *cm) {
    if (!cm) return;
    memset(cm, 0, sizeof(*cm));
    cm->next_pipeline_id = 1;
    cm->next_job_id = 1;
    cm->next_artifact_id = 1;
}

int ci_create_pipeline(CIManager *cm, int pr_id, int branch_id,
                       const char *commit_sha) {
    if (!cm || cm->pipeline_count >= CI_MAX_PIPELINES) return -1;

    int id = cm->next_pipeline_id++;
    int idx = cm->pipeline_count;
    CIPipeline *p = &cm->pipelines[idx];
    memset(p, 0, sizeof(*p));
    p->id = id;
    p->pr_id = pr_id;
    p->branch_id = branch_id;
    if (commit_sha) {
        strncpy(p->commit_sha, commit_sha, COMMIT_SHA_MAX_LEN - 1);
        p->commit_sha[COMMIT_SHA_MAX_LEN - 1] = 0;
    }
    p->status = CI_STATUS_PENDING;
    p->created_at = time(NULL);
    p->updated_at = p->created_at;
    p->is_automatic = true;
    cm->pipeline_count++;
    return id;
}

bool ci_trigger_pipeline(CIManager *cm, int pipeline_id) {
    if (!cm) return false;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            if (cm->pipelines[i].status != CI_STATUS_PENDING &&
                cm->pipelines[i].status != CI_STATUS_MANUAL) return false;
            cm->pipelines[i].status = CI_STATUS_RUNNING;
            cm->pipelines[i].updated_at = time(NULL);
            return true;
        }
    }
    return false;
}

bool ci_cancel_pipeline(CIManager *cm, int pipeline_id) {
    if (!cm) return false;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            if (cm->pipelines[i].status != CI_STATUS_RUNNING &&
                cm->pipelines[i].status != CI_STATUS_PENDING) return false;
            cm->pipelines[i].status = CI_STATUS_CANCELED;
            cm->pipelines[i].updated_at = time(NULL);
            /* Cancel all running jobs */
            for (int s = 0; s < cm->pipelines[i].stage_count; s++) {
                CIStage *st = &cm->pipelines[i].stages[s];
                for (int j = 0; j < st->job_count; j++) {
                    if (st->jobs[j].status == CI_STATUS_RUNNING ||
                        st->jobs[j].status == CI_STATUS_PENDING) {
                        st->jobs[j].status = CI_STATUS_CANCELED;
                    }
                }
            }
            return true;
        }
    }
    return false;
}

bool ci_retry_pipeline(CIManager *cm, int pipeline_id) {
    if (!cm) return false;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            if (cm->pipelines[i].status != CI_STATUS_FAILED &&
                cm->pipelines[i].status != CI_STATUS_CANCELED) return false;
            cm->pipelines[i].status = CI_STATUS_PENDING;
            /* Reset all jobs */
            for (int s = 0; s < cm->pipelines[i].stage_count; s++) {
                for (int j = 0; j < cm->pipelines[i].stages[s].job_count; j++) {
                    cm->pipelines[i].stages[s].jobs[j].status = CI_STATUS_PENDING;
                    cm->pipelines[i].stages[s].jobs[j].exit_code = -1;
                }
            }
            cm->pipelines[i].updated_at = time(NULL);
            return true;
        }
    }
    return false;
}

bool ci_retry_job(CIManager *cm, int pipeline_id, int job_id) {
    if (!cm) return false;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            for (int s = 0; s < cm->pipelines[i].stage_count; s++) {
                for (int j = 0; j < cm->pipelines[i].stages[s].job_count; j++) {
                    CIJob *job = &cm->pipelines[i].stages[s].jobs[j];
                    if (job->id == job_id) {
                        if (job->status != CI_STATUS_FAILED &&
                            job->status != CI_STATUS_CANCELED &&
                            job->status != CI_STATUS_TIMED_OUT) return false;
                        if (job->retry_count >= job->max_retries) return false;
                        job->status = CI_STATUS_PENDING;
                        job->exit_code = -1;
                        job->retry_count++;
                        cm->pipelines[i].updated_at = time(NULL);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

int ci_add_stage(CIManager *cm, int pipeline_id, const char *name,
                 CIStageType type, int order, bool required) {
    if (!cm || !name) return -1;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            CIPipeline *p = &cm->pipelines[i];
            if (p->stage_count >= CI_MAX_STAGES) return -1;
            CIStage *s = &p->stages[p->stage_count];
            memset(s, 0, sizeof(*s));
            s->id = p->stage_count;
            strncpy(s->name, name, CI_STAGE_NAME_MAX - 1);
            s->name[CI_STAGE_NAME_MAX - 1] = 0;
            s->type = type;
            s->status = CI_STATUS_PENDING;
            s->order = order;
            s->required = required;
            p->stage_count++;
            p->updated_at = time(NULL);
            return s->id;
        }
    }
    return -1;
}

int ci_add_job(CIManager *cm, int pipeline_id, int stage_id,
               const char *name, bool allow_failure, int max_retries) {
    if (!cm || !name) return -1;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            CIPipeline *p = &cm->pipelines[i];
            if (stage_id < 0 || stage_id >= p->stage_count) return -1;
            CIStage *s = &p->stages[stage_id];
            if (s->job_count >= CI_MAX_JOBS) return -1;

            int job_id = cm->next_job_id++;
            CIJob *job = &s->jobs[s->job_count];
            memset(job, 0, sizeof(*job));
            job->id = job_id;
            strncpy(job->name, name, CI_JOB_NAME_MAX - 1);
            job->name[CI_JOB_NAME_MAX - 1] = 0;
            job->stage = s->type;
            job->status = CI_STATUS_PENDING;
            job->allow_failure = allow_failure;
            job->max_retries = max_retries;
            job->retry_count = 0;
            s->job_count++;
            p->updated_at = time(NULL);
            return job_id;
        }
    }
    return -1;
}

bool ci_set_job_status(CIManager *cm, int pipeline_id, int job_id,
                       CIStatus status, int exit_code) {
    if (!cm) return false;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            CIPipeline *p = &cm->pipelines[i];
            for (int s = 0; s < p->stage_count; s++) {
                for (int j = 0; j < p->stages[s].job_count; j++) {
                    CIJob *job = &p->stages[s].jobs[j];
                    if (job->id == job_id) {
                        job->status = status;
                        job->exit_code = exit_code;
                        if (status == CI_STATUS_RUNNING) {
                            job->started_at = time(NULL);
                        } else if (status == CI_STATUS_SUCCESS ||
                                   status == CI_STATUS_FAILED) {
                            job->finished_at = time(NULL);
                            job->duration_seconds = (int)
                                difftime(job->finished_at, job->started_at);
                        }
                        p->updated_at = time(NULL);

                        /* Update stage status */
                        bool all_done = true;
                        bool any_failed = false;
                        for (int k = 0; k < p->stages[s].job_count; k++) {
                            CIJob *jk = &p->stages[s].jobs[k];
                            if (jk->status == CI_STATUS_RUNNING ||
                                jk->status == CI_STATUS_PENDING) {
                                all_done = false;
                            }
                            if (jk->status == CI_STATUS_FAILED && !jk->allow_failure) {
                                any_failed = true;
                            }
                        }
                        if (all_done) {
                            p->stages[s].status = any_failed ?
                                CI_STATUS_FAILED : CI_STATUS_SUCCESS;
                        } else if (any_failed) {
                            p->stages[s].status = CI_STATUS_FAILED;
                        }

                        /* Update pipeline status */
                        bool pipe_all_done = true;
                        bool pipe_any_failed = false;
                        for (int st = 0; st < p->stage_count; st++) {
                            if (p->stages[st].status == CI_STATUS_RUNNING ||
                                p->stages[st].status == CI_STATUS_PENDING) {
                                pipe_all_done = false;
                            }
                            if (p->stages[st].status == CI_STATUS_FAILED &&
                                p->stages[st].required) {
                                pipe_any_failed = true;
                            }
                        }
                        if (pipe_all_done) {
                            p->status = pipe_any_failed ?
                                CI_STATUS_FAILED : CI_STATUS_SUCCESS;
                        } else if (pipe_any_failed) {
                            p->status = CI_STATUS_FAILED;
                        } else {
                            p->status = CI_STATUS_RUNNING;
                        }
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool ci_add_job_log(CIManager *cm, int pipeline_id, int job_id,
                    const char *line) {
    if (!cm || !line) return false;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            for (int s = 0; s < cm->pipelines[i].stage_count; s++) {
                for (int j = 0; j < cm->pipelines[i].stages[s].job_count; j++) {
                    CIJob *job = &cm->pipelines[i].stages[s].jobs[j];
                    if (job->id == job_id) {
                        if (job->log_line_count >= CI_MAX_LOG_LINES) return false;
                        strncpy(job->log[job->log_line_count], line, CI_LOG_LINE_MAX - 1);
                        job->log[job->log_line_count][CI_LOG_LINE_MAX - 1] = 0;
                        job->log_line_count++;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool ci_set_job_dependency(CIManager *cm, int pipeline_id,
                           int job_id, int depends_on_job_id) {
    if (!cm) return false;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            for (int s = 0; s < cm->pipelines[i].stage_count; s++) {
                for (int j = 0; j < cm->pipelines[i].stages[s].job_count; j++) {
                    CIJob *job = &cm->pipelines[i].stages[s].jobs[j];
                    if (job->id == job_id) {
                        if (job->dep_count >= 16) return false;
                        job->dependencies[job->dep_count++] = depends_on_job_id;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

CIStatus ci_get_pipeline_status(CIManager *cm, int pipeline_id) {
    if (!cm) return CI_STATUS_FAILED;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id)
            return cm->pipelines[i].status;
    }
    return CI_STATUS_FAILED;
}

CIStatus ci_get_stage_status(CIManager *cm, int pipeline_id, int stage_id) {
    if (!cm) return CI_STATUS_FAILED;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            if (stage_id >= 0 && stage_id < cm->pipelines[i].stage_count) {
                return cm->pipelines[i].stages[stage_id].status;
            }
        }
    }
    return CI_STATUS_FAILED;
}

CIStatus ci_get_job_status(CIManager *cm, int pipeline_id, int job_id) {
    if (!cm) return CI_STATUS_FAILED;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            for (int s = 0; s < cm->pipelines[i].stage_count; s++) {
                for (int j = 0; j < cm->pipelines[i].stages[s].job_count; j++) {
                    if (cm->pipelines[i].stages[s].jobs[j].id == job_id)
                        return cm->pipelines[i].stages[s].jobs[j].status;
                }
            }
        }
    }
    return CI_STATUS_FAILED;
}

bool ci_is_pipeline_green(CIManager *cm, int pipeline_id) {
    return ci_get_pipeline_status(cm, pipeline_id) == CI_STATUS_SUCCESS;
}

bool ci_is_stage_green(CIManager *cm, int pipeline_id, int stage_id) {
    return ci_get_stage_status(cm, pipeline_id, stage_id) == CI_STATUS_SUCCESS;
}

int ci_count_failed_jobs(CIManager *cm, int pipeline_id) {
    if (!cm) return 0;
    int count = 0;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            for (int s = 0; s < cm->pipelines[i].stage_count; s++) {
                for (int j = 0; j < cm->pipelines[i].stages[s].job_count; j++) {
                    if (cm->pipelines[i].stages[s].jobs[j].status == CI_STATUS_FAILED)
                        count++;
                }
            }
        }
    }
    return count;
}

int ci_count_passed_jobs(CIManager *cm, int pipeline_id) {
    if (!cm) return 0;
    int count = 0;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            for (int s = 0; s < cm->pipelines[i].stage_count; s++) {
                for (int j = 0; j < cm->pipelines[i].stages[s].job_count; j++) {
                    if (cm->pipelines[i].stages[s].jobs[j].status == CI_STATUS_SUCCESS)
                        count++;
                }
            }
        }
    }
    return count;
}

/* L5: Determine if the next stage can proceed.
 * A stage can proceed if the previous stage is complete and successful
 * (or if previous stage had all allow_failure jobs). */
bool ci_stage_can_proceed(CIManager *cm, int pipeline_id, int stage_id) {
    if (!cm) return false;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            CIPipeline *p = &cm->pipelines[i];
            if (stage_id <= 0 || stage_id >= p->stage_count) return false;
            CIStage *prev = &p->stages[stage_id - 1];
            return prev->status == CI_STATUS_SUCCESS ||
                   (prev->status == CI_STATUS_FAILED && !prev->required);
        }
    }
    return false;
}

bool ci_advance_stage(CIManager *cm, int pipeline_id) {
    if (!cm) return false;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            CIPipeline *p = &cm->pipelines[i];
            /* Find current running stage */
            for (int s = 0; s < p->stage_count; s++) {
                if (p->stages[s].status == CI_STATUS_SUCCESS) continue;
                if (p->stages[s].status == CI_STATUS_PENDING) {
                    /* Start this stage */
                    p->stages[s].status = CI_STATUS_RUNNING;
                    for (int j = 0; j < p->stages[s].job_count; j++) {
                        p->stages[s].jobs[j].status = CI_STATUS_PENDING;
                    }
                    p->updated_at = time(NULL);
                    return true;
                }
                if (p->stages[s].status == CI_STATUS_FAILED &&
                    p->stages[s].required) {
                    p->status = CI_STATUS_FAILED;
                    return false;
                }
            }
        }
    }
    return false;
}

int ci_add_artifact(CIManager *cm, int pipeline_id, const char *name,
                    const char *path, int size_bytes, int job_id) {
    if (!cm || !name || !path) return -1;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            CIPipeline *p = &cm->pipelines[i];
            if (p->artifact_count >= CI_MAX_ARTIFACTS) return -1;
            int art_id = cm->next_artifact_id++;
            CIArtifact *a = &p->artifacts[p->artifact_count];
            memset(a, 0, sizeof(*a));
            a->id = art_id;
            strncpy(a->name, name, CI_ARTIFACT_PATH_MAX - 1);
            a->name[CI_ARTIFACT_PATH_MAX - 1] = 0;
            strncpy(a->path, path, CI_ARTIFACT_PATH_MAX - 1);
            a->path[CI_ARTIFACT_PATH_MAX - 1] = 0;
            a->size_bytes = size_bytes;
            a->job_id = job_id;
            a->created_at = time(NULL);
            a->expire_days = 30;
            p->artifact_count++;
            return art_id;
        }
    }
    return -1;
}

int ci_find_artifacts(CIManager *cm, int pipeline_id, const char *name_pattern,
                      int *results, int max_results) {
    if (!cm || !results) return 0;
    int count = 0;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            for (int a = 0; a < cm->pipelines[i].artifact_count && count < max_results; a++) {
                if (!name_pattern || strstr(cm->pipelines[i].artifacts[a].name,
                                            name_pattern)) {
                    results[count++] = cm->pipelines[i].artifacts[a].id;
                }
            }
        }
    }
    return count;
}

/* L7: Deployment gate check.
 * Verifies that all required stages passed before allowing deployment.
 * In production: checks against staging environment metrics,
 * canary analysis results, and change freeze periods. */
bool ci_deploy_gate_check(CIManager *cm, int pipeline_id) {
    if (!cm) return false;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            CIPipeline *p = &cm->pipelines[i];
            if (p->status != CI_STATUS_SUCCESS) return false;
            /* Check that critical stages passed */
            for (int s = 0; s < p->stage_count; s++) {
                if (p->stages[s].required &&
                    p->stages[s].status != CI_STATUS_SUCCESS)
                    return false;
            }
            return true;
        }
    }
    return false;
}

/* L7: Find the last green pipeline for rollback. */
bool ci_rollback_check(CIManager *cm, int pipeline_id, int *prev_green_pipeline) {
    if (!cm || !prev_green_pipeline) return false;
    *prev_green_pipeline = -1;

    int target_idx = -1;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) { target_idx = i; break; }
    }
    if (target_idx < 0) return false;

    /* Search backwards for the last green pipeline */
    for (int i = target_idx - 1; i >= 0; i--) {
        if (cm->pipelines[i].status == CI_STATUS_SUCCESS) {
            *prev_green_pipeline = cm->pipelines[i].id;
            return true;
        }
    }
    return false;
}

/* L6: Generate a default pipeline configuration.
 * Standard pipeline: build -> test -> lint -> deploy */
bool ci_generate_default_pipeline(CIManager *cm, int pr_id, int branch_id,
                                   const char *commit_sha) {
    if (!cm) return false;
    int pid = ci_create_pipeline(cm, pr_id, branch_id, commit_sha);
    if (pid < 0) return false;

    /* Stage 1: Build */
    int build_s = ci_add_stage(cm, pid, "build", CI_STAGE_BUILD, 0, true);
    ci_add_job(cm, pid, build_s, "compile", false, 1);
    ci_add_job(cm, pid, build_s, "link", false, 1);

    /* Stage 2: Test */
    int test_s = ci_add_stage(cm, pid, "test", CI_STAGE_TEST, 1, true);
    ci_add_job(cm, pid, test_s, "unit-tests", false, 2);
    ci_add_job(cm, pid, test_s, "integration-tests", false, 2);

    /* Stage 3: Lint & SAST */
    int lint_s = ci_add_stage(cm, pid, "lint", CI_STAGE_LINT, 2, false);
    ci_add_job(cm, pid, lint_s, "lint-check", true, 0);
    ci_add_job(cm, pid, lint_s, "sast-scan", true, 0);

    /* Stage 4: Deploy */
    int deploy_s = ci_add_stage(cm, pid, "deploy", CI_STAGE_DEPLOY, 3, true);
    ci_add_job(cm, pid, deploy_s, "deploy-staging", false, 1);

    return true;
}

/* L8: Parallel job scheduling simulation.
 * In a CI pipeline, jobs within a stage can run in parallel.
 * This function identifies which jobs can run concurrently
 * based on their dependencies. */
bool ci_schedule_parallel_jobs(CIManager *cm, int pipeline_id) {
    if (!cm) return false;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            CIPipeline *p = &cm->pipelines[i];
            for (int s = 0; s < p->stage_count; s++) {
                CIStage *st = &p->stages[s];
                /* Jobs without dependencies can all start simultaneously */
                int ready_count = 0;
                for (int j = 0; j < st->job_count; j++) {
                    if (st->jobs[j].dep_count == 0) {
                        ready_count++;
                    }
                }
                /* In production: create execution graph, compute
                 * critical path, allocate work to runners */
                (void)ready_count;
            }
            return true;
        }
    }
    return false;
}

/* L8: Compute critical path duration for pipeline optimization.
 * The critical path is the longest chain of dependent jobs
 * through the pipeline, determining minimum execution time. */
int ci_get_critical_path_duration(CIManager *cm, int pipeline_id) {
    if (!cm) return 0;
    int max_duration = 0;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            CIPipeline *p = &cm->pipelines[i];
            /* Sum durations of longest job in each stage
             * (simplified - a full implementation would walk
             * the job dependency DAG) */
            int total = 0;
            for (int s = 0; s < p->stage_count; s++) {
                int stage_max = 0;
                for (int j = 0; j < p->stages[s].job_count; j++) {
                    if (p->stages[s].jobs[j].duration_seconds > stage_max) {
                        stage_max = p->stages[s].jobs[j].duration_seconds;
                    }
                }
                total += stage_max;
            }
            if (total > max_duration) max_duration = total;
        }
    }
    return max_duration;
}

void ci_print_pipeline_status(const CIManager *cm, int pipeline_id) {
    if (!cm) return;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            const CIPipeline *p = &cm->pipelines[i];
            printf("=== Pipeline #%d ===\n", p->id);
            printf("  Status: %s, PR: %d, Branch: %d\n",
                   ci_status_name(p->status), p->pr_id, p->branch_id);
            printf("  Automatic: %s, Duration: %ds\n",
                   p->is_automatic ? "yes" : "no",
                   p->total_duration_seconds);
            printf("  Stages: %d, Artifacts: %d\n",
                   p->stage_count, p->artifact_count);
            for (int s = 0; s < p->stage_count; s++) {
                const CIStage *st = &p->stages[s];
                printf("  [%d] %s (%s) %s required=%s jobs=%d\n",
                       st->order, st->name, ci_stage_type_name(st->type),
                       ci_status_name(st->status),
                       st->required ? "yes" : "no", st->job_count);
                for (int j = 0; j < st->job_count; j++) {
                    const CIJob *job = &st->jobs[j];
                    printf("      job %d: %s [%s] exit=%d retries=%d/%d%s\n",
                           job->id, job->name,
                           ci_status_name(job->status),
                           job->exit_code,
                           job->retry_count, job->max_retries,
                           job->allow_failure ? " allow-failure" : "");
                }
            }
            return;
        }
    }
}

void ci_print_job_log(const CIManager *cm, int pipeline_id, int job_id) {
    if (!cm) return;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            for (int s = 0; s < cm->pipelines[i].stage_count; s++) {
                for (int j = 0; j < cm->pipelines[i].stages[s].job_count; j++) {
                    const CIJob *job = &cm->pipelines[i].stages[s].jobs[j];
                    if (job->id == job_id) {
                        printf("=== Job %s (id=%d) Log ===\n", job->name, job->id);
                        for (int l = 0; l < job->log_line_count; l++) {
                            printf("%s\n", job->log[l]);
                        }
                        return;
                    }
                }
            }
        }
    }
}

void ci_print_summary(const CIManager *cm, int pipeline_id) {
    if (!cm) return;
    for (int i = 0; i < cm->pipeline_count; i++) {
        if (cm->pipelines[i].id == pipeline_id) {
            const CIPipeline *p = &cm->pipelines[i];
            printf("Pipeline #%d: %s | %d/%d stages | %d jobs failed\n",
                   p->id, ci_status_name(p->status),
                   p->stage_count, p->stage_count,
                   ci_count_failed_jobs((CIManager*)cm, pipeline_id));
            return;
        }
    }
}

const char* ci_status_name(CIStatus status) {
    switch (status) {
    case CI_STATUS_PENDING:    return "pending";
    case CI_STATUS_RUNNING:    return "running";
    case CI_STATUS_SUCCESS:    return "success";
    case CI_STATUS_FAILED:     return "failed";
    case CI_STATUS_CANCELED:   return "canceled";
    case CI_STATUS_SKIPPED:    return "skipped";
    case CI_STATUS_TIMED_OUT:  return "timed-out";
    case CI_STATUS_MANUAL:     return "manual";
    default:                   return "unknown";
    }
}

const char* ci_stage_type_name(CIStageType type) {
    switch (type) {
    case CI_STAGE_BUILD:     return "build";
    case CI_STAGE_TEST:      return "test";
    case CI_STAGE_LINT:      return "lint";
    case CI_STAGE_SAST:      return "sast";
    case CI_STAGE_SCA:       return "sca";
    case CI_STAGE_CONTAINER: return "container";
    case CI_STAGE_STAGING:   return "staging";
    case CI_STAGE_DEPLOY:    return "deploy";
    case CI_STAGE_E2E_TEST:  return "e2e-test";
    case CI_STAGE_PERF:      return "perf";
    default:                 return "unknown";
    }
}
