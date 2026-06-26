#include "git_pr.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ================================================================
 * L2-L6: Pull Request & Code Review Implementation
 *
 * L2: PR lifecycle state machine (draft -> open -> review -> merged)
 * L4: Four-Eyes Principle — minimum reviewer approval requirements
 * L6: Canonical Problem — code review pipeline
 *
 * Courses: Cambridge Part II Software Engineering
 *          Georgia Tech CS 6300 Software Development Process
 * ================================================================ */

void pr_manager_init(PRManager *pm, const PRReviewPolicy *policy) {
    if (!pm) return;
    memset(pm, 0, sizeof(*pm));
    pm->next_pr_id = 1;
    if (policy) {
        pm->policy = *policy;
    } else {
        /* Default policy: 1 approval, CI required, stale review dismissal */
        pm->policy.min_approvals = 1;
        pm->policy.require_ci_pass = true;
        pm->policy.require_up_to_date = true;
        pm->policy.dismiss_stale_reviews = true;
        pm->policy.allow_self_approve = false;
        pm->policy.require_resolve_all = false;
    }
}

int pr_create(PRManager *pm, const char *title, const char *desc,
              int author, int source_branch, int target_branch) {
    if (!pm || !title || pm->pr_count >= 64) return -1;

    int id = pm->next_pr_id++;
    /* Use sequential index for array storage */
    int idx = pm->pr_count;
    PullRequest *pr = &pm->prs[idx];
    memset(pr, 0, sizeof(*pr));
    pr->id = id;
    strncpy(pr->title, title, PR_TITLE_MAX_LEN - 1);
    pr->title[PR_TITLE_MAX_LEN - 1] = 0;
    if (desc) {
        strncpy(pr->description, desc, PR_DESC_MAX_LEN - 1);
        pr->description[PR_DESC_MAX_LEN - 1] = 0;
    }
    pr->author_id = author;
    pr->source_branch_id = source_branch;
    pr->target_branch_id = target_branch;
    pr->state = PR_STATE_DRAFT;
    pr->draft = true;
    pr->created_at = time(NULL);
    pr->updated_at = pr->created_at;
    pr->mergeable = true;
    pr->ci_passed = false;
    pm->pr_count++;
    return id;
}

/* L2: Submit PR for review — transitions from draft to open */
bool pr_submit_for_review(PRManager *pm, int pr_id) {
    if (!pm) return false;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) {
            if (pm->prs[i].state != PR_STATE_DRAFT) return false;
            pm->prs[i].state = PR_STATE_OPEN;
            pm->prs[i].draft = false;
            pm->prs[i].updated_at = time(NULL);
            return true;
        }
    }
    return false;
}

bool pr_close(PRManager *pm, int pr_id) {
    if (!pm) return false;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) {
            if (pm->prs[i].state == PR_STATE_MERGED) return false;
            pm->prs[i].state = PR_STATE_CLOSED;
            pm->prs[i].updated_at = time(NULL);
            return true;
        }
    }
    return false;
}

bool pr_reopen(PRManager *pm, int pr_id) {
    if (!pm) return false;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) {
            if (pm->prs[i].state != PR_STATE_CLOSED) return false;
            pm->prs[i].state = PR_STATE_OPEN;
            pm->prs[i].updated_at = time(NULL);
            return true;
        }
    }
    return false;
}

/* L4: Merge PR — must pass all policy checks first.
 * Gate checks: approved, CI green, no conflicts, up-to-date. */
bool pr_merge(PRManager *pm, int pr_id) {
    if (!pm) return false;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) {
            PullRequest *pr = &pm->prs[i];
            if (pr->state != PR_STATE_APPROVED && pr->state != PR_STATE_OPEN)
                return false;
            if (!pr_is_mergeable(pm, pr_id)) return false;
            pr->state = PR_STATE_MERGED;
            pr->merged_at = time(NULL);
            pr->updated_at = pr->merged_at;
            return true;
        }
    }
    return false;
}

int pr_assign_reviewer(PRManager *pm, int pr_id, int reviewer_id) {
    if (!pm) return -1;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) {
            PullRequest *pr = &pm->prs[i];
            if (pr->assigned_count >= PR_MAX_REVIEWERS) return -1;
            /* Don't allow self-review */
            if (reviewer_id == pr->author_id && !pm->policy.allow_self_approve)
                return -1;
            /* Check not already assigned */
            for (int j = 0; j < pr->assigned_count; j++) {
                if (pr->assigned_reviewers[j] == reviewer_id) return -1;
            }
            pr->assigned_reviewers[pr->assigned_count++] = reviewer_id;
            pr->updated_at = time(NULL);
            return pr->assigned_count;
        }
    }
    return -1;
}

/* L4: Submit a review with verdict.
 * Updates PR state based on review outcome:
 *   - APPROVED: check if min_approvals met -> PR_STATE_APPROVED
 *   - CHANGES: PR_STATE_CHANGES
 *   - COMMENTED: PR_STATE_REVIEW */
bool pr_submit_review(PRManager *pm, int pr_id, int reviewer_id,
                      ReviewVerdict verdict) {
    if (!pm) return false;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) {
            PullRequest *pr = &pm->prs[i];
            if (pr->state != PR_STATE_OPEN && pr->state != PR_STATE_REVIEW)
                return false;

            /* Check if reviewer already submitted a review — update it */
            int review_idx = -1;
            for (int j = 0; j < pr->review_count; j++) {
                if (pr->reviews[j].reviewer_id == reviewer_id) {
                    review_idx = j; break;
                }
            }

            Review *review;
            if (review_idx >= 0) {
                review = &pr->reviews[review_idx];
            } else if (pr->review_count < PR_MAX_REVIEWERS) {
                review = &pr->reviews[pr->review_count];
                review->id = pr->review_count;
                review->reviewer_id = reviewer_id;
                pr->review_count++;
            } else {
                return false;
            }

            ReviewVerdict old_verdict = review->verdict;
            review->verdict = verdict;
            review->submitted_at = time(NULL);
            review->stale = false;

            /* Update PR approval/blocking counts */
            if (old_verdict == REVIEW_APPROVED && verdict != REVIEW_APPROVED)
                pr->approval_count--;
            if (verdict == REVIEW_APPROVED && old_verdict != REVIEW_APPROVED)
                pr->approval_count++;
            if (old_verdict == REVIEW_CHANGES && verdict != REVIEW_CHANGES)
                pr->blocking_review_count--;
            if (verdict == REVIEW_CHANGES)
                pr->blocking_review_count++;

            /* Update PR state */
            if (verdict == REVIEW_CHANGES) {
                pr->state = PR_STATE_CHANGES;
            } else if (verdict == REVIEW_APPROVED &&
                       pr->approval_count >= pm->policy.min_approvals) {
                pr->state = PR_STATE_APPROVED;
            } else if (verdict == REVIEW_COMMENTED || verdict == REVIEW_PENDING) {
                pr->state = PR_STATE_REVIEW;
            }

            pr->updated_at = time(NULL);
            return true;
        }
    }
    return false;
}

int pr_add_comment(PRManager *pm, int pr_id, int reviewer_id,
                   const char *file_path, int line,
                   const char *content, CommentSeverity severity) {
    if (!pm || !file_path || !content) return -1;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) {
            PullRequest *pr = &pm->prs[i];

            /* Find or create review for this reviewer */
            int review_idx = -1;
            for (int j = 0; j < pr->review_count; j++) {
                if (pr->reviews[j].reviewer_id == reviewer_id) {
                    review_idx = j; break;
                }
            }

            Review *review;
            if (review_idx >= 0) {
                review = &pr->reviews[review_idx];
            } else if (pr->review_count < PR_MAX_REVIEWERS) {
                review = &pr->reviews[pr->review_count];
                review->id = pr->review_count;
                review->reviewer_id = reviewer_id;
                review->verdict = REVIEW_PENDING;
                pr->review_count++;
            } else {
                return -1;
            }

            if (review->comment_count >= PR_MAX_COMMENTS) return -1;

            ReviewComment *cmt = &review->comments[review->comment_count];
            cmt->id = review->comment_count;
            cmt->reviewer_id = reviewer_id;
            strncpy(cmt->file_path, file_path, PR_FILE_MAX - 1);
            cmt->file_path[PR_FILE_MAX - 1] = 0;
            cmt->line_number = line;
            strncpy(cmt->content, content, PR_COMMENT_MAX_LEN - 1);
            cmt->content[PR_COMMENT_MAX_LEN - 1] = 0;
            cmt->severity = severity;
            cmt->resolved = false;
            cmt->created_at = time(NULL);
            review->comment_count++;
            pr->updated_at = time(NULL);
            return cmt->id;
        }
    }
    return -1;
}

bool pr_resolve_comment(PRManager *pm, int pr_id, int comment_id) {
    if (!pm) return false;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) {
            for (int j = 0; j < pm->prs[i].review_count; j++) {
                Review *review = &pm->prs[i].reviews[j];
                for (int k = 0; k < review->comment_count; k++) {
                    if (review->comments[k].id == comment_id) {
                        review->comments[k].resolved = true;
                        pm->prs[i].updated_at = time(NULL);
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

/* L4: Dismiss stale review — when new commits are pushed,
 * previous reviews become stale and must be re-done. */
bool pr_dismiss_review(PRManager *pm, int pr_id, int review_id) {
    if (!pm) return false;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) {
            for (int j = 0; j < pm->prs[i].review_count; j++) {
                if (pm->prs[i].reviews[j].id == review_id) {
                    Review *r = &pm->prs[i].reviews[j];
                    if (r->verdict == REVIEW_APPROVED) pm->prs[i].approval_count--;
                    if (r->verdict == REVIEW_CHANGES) pm->prs[i].blocking_review_count--;
                    r->verdict = REVIEW_DISMISSED;
                    r->stale = true;
                    pm->prs[i].updated_at = time(NULL);
                    if (pm->prs[i].state == PR_STATE_APPROVED)
                        pm->prs[i].state = PR_STATE_REVIEW;
                    return true;
                }
            }
        }
    }
    return false;
}

/* L4: Check if PR is mergeable — all gates pass. */
bool pr_is_mergeable(PRManager *pm, int pr_id) {
    if (!pm) return false;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) {
            PullRequest *pr = &pm->prs[i];
            if (pr->state == PR_STATE_MERGED || pr->state == PR_STATE_CLOSED)
                return false;
            if (!pr->mergeable) return false;
            if (pm->policy.require_ci_pass && !pr->ci_passed) return false;
            if (pr->blocking_review_count > 0) return false;
            if (pr->approval_count < pm->policy.min_approvals) return false;
            if (pm->policy.require_resolve_all && !pr_all_comments_resolved(pm, pr_id))
                return false;
            return true;
        }
    }
    return false;
}

bool pr_is_approved(PRManager *pm, int pr_id) {
    if (!pm) return false;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id)
            return pm->prs[i].approval_count >= pm->policy.min_approvals;
    }
    return false;
}

bool pr_needs_changes(PRManager *pm, int pr_id) {
    if (!pm) return true;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id)
            return pm->prs[i].blocking_review_count > 0;
    }
    return true;
}

int pr_pending_review_count(PRManager *pm, int pr_id) {
    if (!pm) return 0;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) {
            int count = 0;
            for (int j = 0; j < pm->prs[i].assigned_count; j++) {
                bool reviewed = false;
                for (int k = 0; k < pm->prs[i].review_count; k++) {
                    if (pm->prs[i].reviews[k].reviewer_id ==
                        pm->prs[i].assigned_reviewers[j] &&
                        pm->prs[i].reviews[k].verdict != REVIEW_PENDING) {
                        reviewed = true; break;
                    }
                }
                if (!reviewed) count++;
            }
            return count;
        }
    }
    return 0;
}

bool pr_all_comments_resolved(PRManager *pm, int pr_id) {
    if (!pm) return false;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) {
            for (int j = 0; j < pm->prs[i].review_count; j++) {
                for (int k = 0; k < pm->prs[i].reviews[j].comment_count; k++) {
                    if (!pm->prs[i].reviews[j].comments[k].resolved)
                        return false;
                }
            }
            return true;
        }
    }
    return false;
}

bool pr_add_label(PRManager *pm, int pr_id, const char *label) {
    if (!pm || !label) return false;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) {
            if (pm->prs[i].label_count >= PR_MAX_LABELS) return false;
            strncpy(pm->prs[i].labels[pm->prs[i].label_count],
                    label, PR_LABEL_MAX_LEN - 1);
            pm->prs[i].labels[pm->prs[i].label_count][PR_LABEL_MAX_LEN - 1] = 0;
            pm->prs[i].label_count++;
            pm->prs[i].updated_at = time(NULL);
            return true;
        }
    }
    return false;
}

bool pr_remove_label(PRManager *pm, int pr_id, const char *label) {
    if (!pm || !label) return false;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) {
            for (int j = 0; j < pm->prs[i].label_count; j++) {
                if (strcmp(pm->prs[i].labels[j], label) == 0) {
                    for (int k = j; k < pm->prs[i].label_count - 1; k++) {
                        strncpy(pm->prs[i].labels[k], pm->prs[i].labels[k+1],
                                PR_LABEL_MAX_LEN - 1);
                    }
                    pm->prs[i].label_count--;
                    return true;
                }
            }
        }
    }
    return false;
}

bool pr_has_label(PRManager *pm, int pr_id, const char *label) {
    if (!pm || !label) return false;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) {
            for (int j = 0; j < pm->prs[i].label_count; j++) {
                if (strcmp(pm->prs[i].labels[j], label) == 0) return true;
            }
        }
    }
    return false;
}

void pr_set_ci_status(PRManager *pm, int pr_id, bool passed) {
    if (!pm) return;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) {
            pm->prs[i].ci_passed = passed;
            pm->prs[i].updated_at = time(NULL);
            return;
        }
    }
}

bool pr_ci_check(PRManager *pm, int pr_id) {
    if (!pm) return false;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) return pm->prs[i].ci_passed;
    }
    return false;
}

void pr_configure_policy(PRManager *pm, const PRReviewPolicy *policy) {
    if (pm && policy) pm->policy = *policy;
}

bool pr_check_policy(PRManager *pm, int pr_id) {
    return pr_is_mergeable(pm, pr_id);
}

int pr_find_open(PRManager *pm, int *results, int max_results) {
    if (!pm || !results) return 0;
    int count = 0;
    for (int i = 0; i < pm->pr_count && count < max_results; i++) {
        if (pm->prs[i].state == PR_STATE_OPEN ||
            pm->prs[i].state == PR_STATE_REVIEW) {
            results[count++] = pm->prs[i].id;
        }
    }
    return count;
}

int pr_find_by_author(PRManager *pm, int author_id, int *results, int max_results) {
    if (!pm || !results) return 0;
    int count = 0;
    for (int i = 0; i < pm->pr_count && count < max_results; i++) {
        if (pm->prs[i].author_id == author_id) {
            results[count++] = pm->prs[i].id;
        }
    }
    return count;
}

int pr_find_awaiting_review(PRManager *pm, int *results, int max_results) {
    if (!pm || !results) return 0;
    int count = 0;
    for (int i = 0; i < pm->pr_count && count < max_results; i++) {
        if (pm->prs[i].state == PR_STATE_OPEN &&
            pr_pending_review_count(pm, pm->prs[i].id) > 0) {
            results[count++] = pm->prs[i].id;
        }
    }
    return count;
}

int pr_find_approved(PRManager *pm, int *results, int max_results) {
    if (!pm || !results) return 0;
    int count = 0;
    for (int i = 0; i < pm->pr_count && count < max_results; i++) {
        if (pr_is_approved(pm, pm->prs[i].id)) {
            results[count++] = pm->prs[i].id;
        }
    }
    return count;
}

void pr_print(const PRManager *pm, int pr_id) {
    if (!pm) return;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) {
            const PullRequest *pr = &pm->prs[i];
            printf("=== PR #%d: %s ===\n", pr->id, pr->title);
            printf("  State: %s, Draft: %s\n",
                   pr_state_name(pr->state), pr->draft ? "yes" : "no");
            printf("  Author: %d, Source: %d -> Target: %d\n",
                   pr->author_id, pr->source_branch_id, pr->target_branch_id);
            printf("  Approvals: %d/%d, Blocking: %d, CI: %s\n",
                   pr->approval_count, pm->policy.min_approvals,
                   pr->blocking_review_count,
                   pr->ci_passed ? "green" : "red");
            printf("  Mergeable: %s\n",
                   pr_is_mergeable((PRManager*)pm, pr_id) ? "yes" : "no");
            printf("  Labels: ");
            for (int j = 0; j < pr->label_count; j++) printf("%s ", pr->labels[j]);
            printf("\n  Reviews: %d\n", pr->review_count);
            for (int j = 0; j < pr->review_count; j++) {
                printf("    [%d] reviewer=%d verdict=%s comments=%d%s\n",
                       pr->reviews[j].id, pr->reviews[j].reviewer_id,
                       review_verdict_name(pr->reviews[j].verdict),
                       pr->reviews[j].comment_count,
                       pr->reviews[j].stale ? " STALE" : "");
            }
            return;
        }
    }
    printf("PR #%d not found\n", pr_id);
}

void pr_print_summary(const PRManager *pm, int pr_id) {
    if (!pm) return;
    for (int i = 0; i < pm->pr_count; i++) {
        if (pm->prs[i].id == pr_id) {
            const PullRequest *pr = &pm->prs[i];
            printf("PR #%d: %s [%s] A:%d/%d CI:%s\n",
                   pr->id, pr->title, pr_state_name(pr->state),
                   pr->approval_count, pm->policy.min_approvals,
                   pr->ci_passed ? "PASS" : "FAIL");
            return;
        }
    }
}

void pr_print_all(const PRManager *pm) {
    if (!pm) return;
    printf("=== PR Manager ===\n");
    printf("Policy: min_approvals=%d ci=%s dismiss_stale=%s\n",
           pm->policy.min_approvals,
           pm->policy.require_ci_pass ? "required" : "not-required",
           pm->policy.dismiss_stale_reviews ? "yes" : "no");
    for (int i = 0; i < pm->pr_count; i++) {
        pr_print_summary(pm, pm->prs[i].id);
    }
}

const char* pr_state_name(PRState state) {
    switch (state) {
    case PR_STATE_DRAFT:    return "draft";
    case PR_STATE_OPEN:     return "open";
    case PR_STATE_REVIEW:   return "in-review";
    case PR_STATE_APPROVED: return "approved";
    case PR_STATE_CHANGES:  return "changes-requested";
    case PR_STATE_MERGED:   return "merged";
    case PR_STATE_CLOSED:   return "closed";
    case PR_STATE_REOPENED: return "reopened";
    default:                return "unknown";
    }
}

const char* review_verdict_name(ReviewVerdict verdict) {
    switch (verdict) {
    case REVIEW_PENDING:   return "pending";
    case REVIEW_APPROVED:  return "approved";
    case REVIEW_COMMENTED: return "commented";
    case REVIEW_CHANGES:   return "changes-requested";
    case REVIEW_DISMISSED: return "dismissed";
    default:               return "unknown";
    }
}
