#include "eq_halstead.h"
#include <stdio.h>
#include <math.h>

void halstead_compute(int u_ops, int u_ops_count, int t_ops, int t_ops_count, HalsteadMetrics *hm) {
    hm->n1 = u_ops; hm->n2 = u_ops_count; hm->N1 = t_ops; hm->N2 = t_ops_count;
    hm->vocabulary = (double)(u_ops + u_ops_count);
    hm->length = (double)(t_ops + t_ops_count);
    hm->volume = hm->length > 0 && hm->vocabulary > 0 ? hm->length * log2(hm->vocabulary) : 0;
    hm->difficulty = hm->n2 > 0 ? (hm->n1 / 2.0) * (hm->N2 / (double)hm->n2) : 0;
    hm->effort = hm->difficulty * hm->volume;
    hm->time_seconds = hm->effort / 18.0;
    hm->bugs_delivered = hm->effort > 0 ? pow(hm->effort, 2.0/3.0) / 3000.0 : 0;
}

const char *halstead_grade(HalsteadMetrics *hm) {
    if (hm->volume < 500) return "A (Excellent)";
    if (hm->volume < 1500) return "B (Good)";
    if (hm->volume < 3000) return "C (Adequate)";
    if (hm->volume < 5000) return "D (Poor)";
    return "F (Refactor)";
}

void halstead_print(HalsteadMetrics *hm) {
    printf("  Halstead: V=%.0f D=%.2f E=%.0f T=%.1fs\n", hm->volume, hm->difficulty, hm->effort, hm->time_seconds);
    printf("  Vocabulary: %.0f, Length: %.0f, Bugs: %.4f\n", hm->vocabulary, hm->length, hm->bugs_delivered);
    printf("  Grade: %s\n", halstead_grade(hm));
}
