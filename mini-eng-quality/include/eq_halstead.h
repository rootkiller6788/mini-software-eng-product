#ifndef EQ_HALSTEAD_H
#define EQ_HALSTEAD_H

typedef struct {
    int n1, n2;           /* unique operators, operands */
    int N1, N2;           /* total operators, operands */
    double vocabulary;    /* n = n1 + n2 */
    double length;        /* N = N1 + N2 */
    double volume;        /* V = N * log2(n) */
    double difficulty;    /* D = (n1/2) * (N2/n2) */
    double effort;        /* E = D * V */
    double time_seconds;  /* T = E / 18 */
    double bugs_delivered;/* B = E^(2/3) / 3000 */
} HalsteadMetrics;

void halstead_compute(int unique_ops, int unique_ops_count, int total_ops, int total_ops_count, HalsteadMetrics *hm);
void halstead_print(HalsteadMetrics *hm);
const char *halstead_grade(HalsteadMetrics *hm);
#endif
