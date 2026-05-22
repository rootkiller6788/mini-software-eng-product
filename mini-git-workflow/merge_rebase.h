#ifndef MERGE_REBASE_H
#define MERGE_REBASE_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

/* ── OID (lightweight, avoids circular dependency) ───────────────── */

#define MERGE_SHA1_RAWSZ 20

typedef struct {
    unsigned char id[MERGE_SHA1_RAWSZ];
} MergeOid;

/* ── Commit stub ────────────────────────────────────────────────── */

typedef struct {
    MergeOid    oid;
    MergeOid   *parents;
    size_t      parent_count;
    char       *message;
    time_t      authored_at;
} MergeCommit;

/* ── Merge strategies ───────────────────────────────────────────── */

typedef enum {
    MERGE_FF,          /* fast-forward: move branch pointer forward */
    MERGE_FF_ONLY,     /* only if fast-forward possible */
    MERGE_THREE_WAY,   /* standard recursive three-way merge */
    MERGE_OCTOPUS,     /* merge 3+ heads */
    MERGE_SUBTREE,     /* subtree merge (adjust path prefixes) */
    MERGE_OURS         /* ignore other branch, keep ours */
} MergeStrategy;

/* Three-way merge context */
typedef struct {
    MergeCommit base;     /* common ancestor */
    MergeCommit ours;     /* current branch tip */
    MergeCommit theirs;   /* branch being merged */
} MergeBase;

/* Resolved conflict chunk */
typedef struct {
    size_t  ours_start, ours_end;
    size_t  theirs_start, theirs_end;
    int     resolved;       /* 1 after user picks resolution */
    char   *resolved_text;
} MergeConflict;

typedef struct {
    MergeConflict *conflicts;
    size_t         conflict_count;
    char          *merged_content;   /* result after resolution */
} MergeResult;

/* ── Rebase ─────────────────────────────────────────────────────── */
/*   Replay your commits on top of another branch                    */

typedef struct {
    MergeCommit  *commits;       /* sequence to replay */
    size_t        commit_count;
    MergeOid      onto_oid;      /* target base */
    MergeOid      upstream_oid;  /* original upstream we diverged from */
    int           preserve_merges; /* --preserve-merges / --rebase-merges */
} RebasePlan;

/* Interactive rebase options */
typedef enum {
    REBASE_PICK,       /* use commit as-is */
    REBASE_REWORD,     /* change commit message */
    REBASE_EDIT,       /* stop to amend */
    REBASE_SQUASH,     /* meld into previous, keep message */
    REBASE_FIXUP,      /* meld into previous, discard message */
    REBASE_BREAK,      /* stop here for interaction */
    REBASE_DROP,       /* remove commit entirely */
    REBASE_EXEC        /* execute a shell command */
} RebaseCommand;

typedef struct {
    MergeCommit    commit;
    RebaseCommand  command;
    char          *new_message;  /* for REWORD */
    char          *exec_cmd;     /* for EXEC */
} RebaseStep;

typedef struct {
    RebaseStep *steps;
    size_t      step_count;
    MergeOid    onto;

    /* conflict handling during rebase */
    int         in_progress;
    size_t      current_step;
} InteractiveRebase;

/* ── Cherry-pick ────────────────────────────────────────────────── */

typedef struct {
    MergeCommit source_commit;   /* commit to apply */
    int         allow_empty;     /* keep empty commits */
    int         record_origin;   /* append (cherry-picked from ...) */
    MergeOid    parent_number;   /* which parent in merge commits */
} CherryPick;

/* ── Rerere (Reuse Recorded Resolution) ─────────────────────────── */

typedef struct {
    char  *conflict_hash;       /* sha1 of conflict markers */
    char  *patch;               /* resolved patch */
    size_t patch_size;
    time_t recorded_at;
} RerereRecord;

typedef struct {
    RerereRecord *records;
    size_t        record_count;
    char         *rerere_dir;  /* .git/rr-cache */
} RerereCache;

/* ── Squash merge ────────────────────────────────────────────────── */
/*   Collapse all branch commits into one before merging              */

typedef struct {
    MergeOid    base_oid;
    MergeOid    tip_oid;
    char       *new_message;    /* if NULL, concatenate all messages */
    int         discard_messages; /* 1 = provide single message */
} SquashMerge;

/* ── API ────────────────────────────────────────────────────────── */

/* Merge */
MergeBase  merge_find_base(const MergeCommit *ours,
                           const MergeCommit *theirs);
MergeResult merge_three_way(const MergeCommit *base,
                             const MergeCommit *ours,
                             const MergeCommit *theirs,
                             const char *ours_content,
                             const char *theirs_content);
int         merge_is_fast_forward(const MergeCommit *ours,
                                  const MergeCommit *theirs,
                                  int *ours_is_ancestor,
                                  int *theirs_is_ancestor);
MergeResult merge_octopus(const MergeCommit **branches, size_t count);
void        merge_result_free(MergeResult *mr);

/* Rebase */
void        rebase_plan_build(RebasePlan *plan,
                              const MergeCommit *commits, size_t count,
                              const MergeOid *onto,
                              const MergeOid *upstream);
MergeCommit rebase_apply(const RebasePlan *plan,
                         int (*conflict_handler)(MergeConflict *, void *),
                         void *userdata);
void        rebase_plan_free(RebasePlan *plan);

/* Interactive rebase */
void        interactive_rebase_init(InteractiveRebase *ir,
                                     const MergeCommit *commits,
                                     size_t count,
                                     const MergeOid *onto);
void        interactive_rebase_set_command(InteractiveRebase *ir,
                                            size_t idx,
                                            RebaseCommand cmd,
                                            const char *extra);
int         interactive_rebase_execute(InteractiveRebase *ir);
void        interactive_rebase_free(InteractiveRebase *ir);

/* Cherry-pick */
int         cherry_pick_apply(const CherryPick *cp,
                               const char *ours_content,
                               char **out);

/* Rerere */
void        rerere_cache_init(RerereCache *rc, const char *git_dir);
int         rerere_record(RerereCache *rc, const char *hash,
                          const char *patch, size_t len);
int         rerere_reuse(RerereCache *rc, const char *hash,
                         char **patch, size_t *len);
void        rerere_cache_free(RerereCache *rc);

/* Squash */
int         squash_merge_apply(const SquashMerge *sm,
                                const MergeCommit *source_commits,
                                size_t count,
                                const char *target_content,
                                char **out);

void        merge_commit_free(MergeCommit *mc);

#endif /* MERGE_REBASE_H */
