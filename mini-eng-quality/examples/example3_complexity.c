#include "complexity_metrics.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    cm_analysis_t analysis;
    cm_analysis_init(&analysis);

    const char *func1 =
        "int compute(int n) {\n"
        "    int result = 0;\n"
        "    for (int i = 0; i < n; i++) {\n"
        "        if (i % 2 == 0) {\n"
        "            result += i;\n"
        "        } else if (i % 3 == 0) {\n"
        "            result -= i;\n"
        "        } else {\n"
        "            result *= 2;\n"
        "        }\n"
        "    }\n"
        "    if (result < 0) return 0;\n"
        "    return result;\n"
        "}\n";

    const char *func2 =
        "int factorial(int n) {\n"
        "    if (n <= 1) return 1;\n"
        "    return n * factorial(n - 1);\n"
        "}\n";

    const char *func3 =
        "// Process input data\n"
        "/* This function validates and transforms\n"
        "   the input buffer before processing */\n"
        "void process_buffer(char *buf, size_t len) {\n"
        "    if (!buf || len == 0) return;\n"
        "    for (size_t i = 0; i < len; i++) {\n"
        "        if (buf[i] >= 'a' && buf[i] <= 'z') {\n"
        "            buf[i] -= 32;\n"
        "        } else {\n"
        "            buf[i] = '_';\n"
        "        }\n"
        "    }\n"
        "    return;\n"
        "}\n";

    printf("=== Complexity Metrics Analysis ===\n\n");

    cm_analyze_function(&analysis, "compute", "src/math.c", 1, 14,
                        func1, (int)strlen(func1));
    cm_analyze_function(&analysis, "factorial", "src/math.c", 16, 19,
                        func2, (int)strlen(func2));
    cm_analyze_function(&analysis, "process_buffer", "src/buffer.c", 1, 14,
                        func3, (int)strlen(func3));

    for (int i = 0; i < analysis.function_count; i++)
        cm_print_function_metrics(&analysis.functions[i]);

    const char *file_source =
        "// Header comment\n"
        "#include <stdio.h>\n"
        "\n"
        "int add(int a, int b) {\n"
        "    return a + b;\n"
        "}\n"
        "\n"
        "int subtract(int a, int b) {\n"
        "    return a - b;\n"
        "}\n"
        "\n"
        "int main(void) {\n"
        "    int x = add(3, 4);\n"
        "    printf(\"Result: %d\\n\", x);\n"
        "    return 0;\n"
        "}\n";

    cm_analyze_file(&analysis, "src/main.c", file_source,
                    (int)strlen(file_source));
    if (analysis.file_count > 0)
        cm_print_file_metrics(&analysis.files[0]);

    int comments = 0, blanks = 0;
    int loc = cm_count_loc("int x;\n// comment\n\nint y;\n", 23, &comments, &blanks);
    printf("\nLOC counting: total=%d comments=%d blanks=%d\n",
           loc, comments, blanks);

    double hv = cm_calc_halstead_volume(12, 15, 6, 8);
    printf("Halstead Volume: %.2f\n", hv);

    double mi = cm_calc_maintainability(hv, 5.0, 200.0, 15.0);
    printf("Maintainability Index: %.2f\n", mi);

    printf("\n=== Hotspot Analysis ===\n");
    cm_hotspot_analysis_t hotspots;
    cm_hotspot_init(&hotspots);
    cm_add_hotspot(&hotspots, "src/auth.c", 45, 120, 75, 42);
    cm_add_hotspot(&hotspots, "src/parser.c", 200, 350, 150, 65);
    cm_add_hotspot(&hotspots, "src/util.c", 10, 30, 20, 8);
    cm_add_hotspot(&hotspots, "src/net.c", 500, 700, 200, 28);
    cm_sort_hotspots(&hotspots);
    cm_print_hotspots(&hotspots);

    cm_cognitive_profile_t profile;
    cm_cognitive_profile(func1, (int)strlen(func1), &profile);
    printf("\nCognitive Profile for compute():\n");
    printf("  Branches: %d, Loops: %d, Nesting: %d, Exception: %d\n",
           profile.functional_branches, profile.functional_loops,
           profile.nesting_depth, profile.exceptional_paths);

    int nesting = cm_nesting_depth("{{ if (x) { { } } }}", 20);
    printf("Max nesting depth: %d\n", nesting);

    return 0;
}
