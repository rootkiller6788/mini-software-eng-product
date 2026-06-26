#include "test_coverage.h"
#include <stdio.h>
#include <string.h>

void coverage_init(CoverageData *cd) { memset(cd, 0, sizeof(*cd)); }

int coverage_add_file(CoverageData *cd, const char *file_path) {
    if (cd->file_count >= MAX_COV_FILES) return -1;
    CoverageFile *cf = &cd->files[cd->file_count];
    strncpy(cf->file_path, file_path, 127); cf->file_path[127] = '\0';
    cf->line_count = 0; cf->total_lines = 0; cf->covered_lines = 0;
    cf->branch_points = 0; cf->covered_branches = 0;
    return cd->file_count++;
}

int coverage_add_line(CoverageData *cd, int file_idx, int line_number, bool has_branch) {
    if (file_idx < 0 || file_idx >= cd->file_count) return -1;
    CoverageFile *cf = &cd->files[file_idx];
    if (cf->line_count >= MAX_COV_LINES_PER_FILE) return -1;
    CoverageLine *cl = &cf->lines[cf->line_count];
    cl->line_number = line_number; cl->status = CLINE_NEVER; cl->hit_count = 0;
    cl->has_branch = has_branch; cl->branch_true_hit = false; cl->branch_false_hit = false;
    cf->total_lines++;
    if (has_branch) cf->branch_points++;
    return cf->line_count++;
}

void coverage_hit(CoverageData *cd, int file_idx, int line_number) {
    if (file_idx < 0 || file_idx >= cd->file_count) return;
    CoverageFile *cf = &cd->files[file_idx];
    for (int i = 0; i < cf->line_count; i++)
        if (cf->lines[i].line_number == line_number) {
            cf->lines[i].hit_count++;
            if (cf->lines[i].status == CLINE_NEVER) {
                cf->lines[i].status = CLINE_HIT; cf->covered_lines++;
            }
            return;
        }
}

void coverage_branch_hit(CoverageData *cd, int file_idx, int line_number, bool branch_true_hit, bool branch_false_hit) {
    if (file_idx < 0 || file_idx >= cd->file_count) return;
    CoverageFile *cf = &cd->files[file_idx];
    for (int i = 0; i < cf->line_count; i++)
        if (cf->lines[i].line_number == line_number && cf->lines[i].has_branch) {
            if (branch_true_hit && !cf->lines[i].branch_true_hit) {
                cf->lines[i].branch_true_hit = true;
            }
            if (branch_false_hit && !cf->lines[i].branch_false_hit) {
                cf->lines[i].branch_false_hit = true;
            }
            if (cf->lines[i].branch_true_hit && cf->lines[i].branch_false_hit) {
                cf->lines[i].status = CLINE_BRANCH_BOTH; cf->covered_branches++;
            } else if (cf->lines[i].branch_true_hit) {
                cf->lines[i].status = CLINE_BRANCH_TRUE;
            } else if (cf->lines[i].branch_false_hit) {
                cf->lines[i].status = CLINE_BRANCH_FALSE;
            }
            return;
        }
}

void coverage_calculate(CoverageData *cd) {
    cd->mcdc_conditions = 0; cd->mcdc_covered = 0;
    for (int i = 0; i < cd->file_count; i++) {
        CoverageFile *cf = &cd->files[i];
        cf->covered_lines = 0; cf->covered_branches = 0;
        for (int j = 0; j < cf->line_count; j++) {
            if (cf->lines[j].status != CLINE_NEVER) cf->covered_lines++;
            if (cf->lines[j].has_branch && cf->lines[j].status == CLINE_BRANCH_BOTH) cf->covered_branches++;
        }
    }
}

double coverage_line_pct(CoverageData *cd) {
    int total = 0, covered = 0;
    for (int i = 0; i < cd->file_count; i++) { total += cd->files[i].total_lines; covered += cd->files[i].covered_lines; }
    return total > 0 ? 100.0 * covered / total : 0;
}

double coverage_branch_pct(CoverageData *cd) {
    int total = 0, covered = 0;
    for (int i = 0; i < cd->file_count; i++) { total += cd->files[i].branch_points; covered += cd->files[i].covered_branches; }
    return total > 0 ? 100.0 * covered / total : 0;
}

double coverage_mcdc_pct(CoverageData *cd) {
    return cd->mcdc_conditions > 0 ? 100.0 * cd->mcdc_covered / cd->mcdc_conditions : 0;
}

bool coverage_meets_threshold(CoverageData *cd, double line_pct, double branch_pct) {
    return coverage_line_pct(cd) >= line_pct && coverage_branch_pct(cd) >= branch_pct;
}

void coverage_print_file(CoverageData *cd, int file_idx) {
    if (file_idx < 0 || file_idx >= cd->file_count) return;
    CoverageFile *cf = &cd->files[file_idx];
    printf("--- %s: %.0f%% lines, %.0f%% branches ---\n", cf->file_path, coverage_line_pct(cd), coverage_branch_pct(cd));
    for (int i = 0; i < cf->line_count; i++) {
        const char *ss[] = {"  ","H ","P ","T ","F ","B "};
        printf("  %s L%-4d hits=%d\n", ss[cf->lines[i].status], cf->lines[i].line_number, cf->lines[i].hit_count);
    }
}

void coverage_print_summary(CoverageData *cd) {
    printf("=== Coverage Summary ===\n");
    coverage_calculate(cd);
    for (int i = 0; i < cd->file_count; i++)
        printf("  %s: lines=%.0f%% branches=%.0f%%\n",
               cd->files[i].file_path,
               cd->files[i].total_lines > 0 ? 100.0*cd->files[i].covered_lines/cd->files[i].total_lines : 0,
               cd->files[i].branch_points > 0 ? 100.0*cd->files[i].covered_branches/cd->files[i].branch_points : 0);
    printf("  TOTAL: lines=%.1f%% branches=%.1f%% MC/DC=%.1f%%\n",
           coverage_line_pct(cd), coverage_branch_pct(cd), coverage_mcdc_pct(cd));
}
