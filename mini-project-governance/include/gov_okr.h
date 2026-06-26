#ifndef GOV_OKR_H
#define GOV_OKR_H
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/* ============================================================
 * L1: OKR System — Core Definitions
 * Reference: Measure What Matters (Doerr), Google OKR Playbook
 * OKR = Objective (qualitative goal) + Key Results (quantitative measures)
 * ============================================================ */

#define OKR_MAX_OBJECTIVES     32
#define OKR_MAX_KEY_RESULTS    128
#define OKR_NAME_LEN           128
#define OKR_DESC_LEN           256
#define OKR_ID_LEN             32
#define OKR_MAX_ALIGNMENTS     8
#define OKR_MAX_CHECKINS       16

/* ---- Confidence Level ---- */
typedef enum {
    CONF_ON_TRACK = 0,      /* >= 0.7 completion expected */
    CONF_AT_RISK,           /* 0.4 - 0.7 */
    CONF_OFF_TRACK,         /* < 0.4 */
    CONF_COMPLETED          /* 1.0 */
} OkrConfidence;

/* ---- Key Result Type ---- */
typedef enum {
    KR_PERCENTAGE = 0,      /* 0-100% */
    KR_ABSOLUTE,            /* numeric target (e.g., 1000 users) */
    KR_BINARY,              /* 0 or 1 */
    KR_CURRENCY,            /* monetary value */
    KR_RATIO                /* e.g., 0.01-100.0 */
} KRType;

/* ---- Key Result ---- */
typedef struct {
    char id[OKR_ID_LEN];
    char description[OKR_DESC_LEN];
    KRType type;
    double target;
    double current;
    double start;
    OkrConfidence confidence;
    int objective_id;          /* parent objective */
    int weight_pct;            /* weight within parent objective */
    uint32_t created_at;
    uint32_t last_checkin;
} KeyResult;

/* ---- Objective ---- */
typedef struct {
    char id[OKR_ID_LEN];
    char title[OKR_NAME_LEN];
    char description[OKR_DESC_LEN];
    int key_result_ids[8];
    int kr_count;
    double progress;           /* weighted avg of KRs */
    OkrConfidence confidence;
    int alignments[OKR_MAX_ALIGNMENTS];
    int alignment_count;
    uint32_t start_date;
    uint32_t end_date;
    bool is_company_level;
} Objective;

/* ---- OKR Dashboard ---- */
typedef struct {
    Objective objectives[OKR_MAX_OBJECTIVES];
    int obj_count;
    KeyResult key_results[OKR_MAX_KEY_RESULTS];
    int kr_count;
    uint32_t quarter_start;
    uint32_t quarter_end;
    int quarter_number;
    int year;
} OkrDashboard;

/* ---- OKR Alignment Tree (L3: Engineering Structure) ---- */
typedef struct OkrNode {
    Objective *obj;
    struct OkrNode *children[8];
    int child_count;
    struct OkrNode *parent;
} OkrNode;

typedef struct {
    OkrNode *root;
    int total_nodes;
    int max_depth;
} OkrTree;

/* ---- API ---- */
void okr_init(OkrDashboard *dash, int year, int quarter);
int  okr_add_objective(OkrDashboard *dash, const char *id, const char *title,
                       const char *desc, bool company_level,
                       uint32_t start, uint32_t end);
int  okr_find_objective(OkrDashboard *dash, const char *id);

int  okr_add_key_result(OkrDashboard *dash, const char *id, const char *desc,
                        KRType type, double target, double start,
                        int obj_idx, int weight);
int  okr_find_kr(OkrDashboard *dash, const char *id);

bool okr_update_kr(OkrDashboard *dash, const char *kr_id, double new_value);
void okr_recalc_progress(OkrDashboard *dash, int obj_idx);
void okr_recalc_all(OkrDashboard *dash);
OkrConfidence okr_assess_confidence(double progress, double elapsed_pct);

bool okr_align(OkrDashboard *dash, const char *parent_id, const char *child_id);
double okr_company_score(OkrDashboard *dash);

/* L8: Advanced — OKR Tree with cascading alignment */
OkrTree *okr_build_tree(OkrDashboard *dash);
void okr_tree_score(OkrTree *tree);
void okr_tree_free(OkrTree *tree);
void okr_print_tree(OkrTree *tree, int depth);
void okr_print_dashboard(OkrDashboard *dash);

#endif
