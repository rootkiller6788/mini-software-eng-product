#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "adr_system.h"

int main(void)
{
    printf("=== ADR System Example ===\n\n");

    AdrLog log;
    adr_log_init(&log, "mini-arch-management");

    unsigned int adr1 = adr_log_add_record(&log,
        "Use C99 for implementation",
        "The project needs a systems language with broad compiler support and "
        "deterministic resource management. Options considered: C, Rust, Go.",
        "Use C99 as the implementation language. C99 provides struct "
        "designated initializers, stdbool.h, and portable compilation across "
        "all target platforms including embedded systems.",
        "Positive: Wide compiler support, minimal runtime overhead, direct "
        "memory control. Negative: Manual memory management requires "
        "discipline, no built-in testing framework.",
        "Architecture Team");

    unsigned int adr2 = adr_log_add_record(&log,
        "Use layered architecture with include/ and src/ separation",
        "Code organisation must support clean compilation units and "
        "reusable headers for downstream consumers.",
        "Separate headers in include/ and implementation in src/. Each "
        "module has a corresponding .h and .c pair with matching names.",
        "Positive: Clear API boundary, easy to integrate into other "
        "projects via -I include. Negative: Requires discipline to keep "
        "headers minimal.",
        "Architecture Team");

    unsigned int adr3 = adr_log_add_record(&log,
        "Adopt ADR process for all architectural decisions",
        "The team needs a systematic way to document why architectural "
        "choices were made, not just what was chosen.",
        "Use ADR (Architecture Decision Records) with status lifecycle: "
        "proposed -> accepted -> deprecated -> superseded. Each ADR "
        "documents context, decision, and consequences.",
        "Positive: Complete audit trail of decisions, enables future "
        "contributors to understand rationale. Negative: Requires "
        "discipline to maintain.",
        "Architecture Team");

    adr_log_accept(&log, adr1);
    adr_log_accept(&log, adr2);
    adr_log_accept(&log, adr3);

    adr_record_add_tag(
        adr_log_find(&log, adr1), "language-choice");
    adr_record_add_tag(
        adr_log_find(&log, adr1), "portability");
    adr_record_add_tag(
        adr_log_find(&log, adr2), "project-structure");
    adr_record_add_tag(
        adr_log_find(&log, adr3), "process");

    unsigned int adr4 = adr_log_add_record(&log,
        "Use C11 atomics for concurrency support",
        "The threading module needs atomic operations. Options: POSIX "
        "atomics, C11 stdatomic.h, compiler intrinsics.",
        "Adopt C11 stdatomic.h for atomic operations. Upgrade from C99 "
        "to C11 for the stdlib threading features.",
        "Positive: Standardised atomic model, portable across compilers. "
        "Negative: Requires C11 (breaks C99-only tool chains), atomics "
        "not available on all embedded targets.",
        "Architecture Team");

    adr_log_supersede(&log, adr1, adr4);

    printf("Total ADRs: %zu\n", adr_log_count_total(&log));
    printf("Accepted: %zu\n",
           adr_log_count_by_status(&log, ADR_STATUS_ACCEPTED));
    printf("Superseded: %zu\n",
           adr_log_count_by_status(&log, ADR_STATUS_SUPERSEDED));

    printf("\n--- ADR Log ---\n");
    adr_log_print(&log);

    printf("\n--- Supersede Chain for ADR #0001 ---\n");
    AdrSupersedeChain chain;
    if (adr_log_get_supersede_chain(&log, 1, &chain)) {
        printf("Chain: ");
        for (size_t i = 0; i < chain.length; i++) {
            printf("ADR #%04u ", chain.chain[i]);
            if (i < chain.length - 1) printf("-> ");
        }
        printf("\n");
    }

    printf("\n--- Detailed Entry ---\n");
    adr_log_print_entry(adr_log_find(&log, adr4));

    printf("\nExporting to adr_log.md...\n");
    adr_log_export_markdown(&log, "adr_log.md");
    printf("Done.\n");

    if (adr_log_validate(&log)) {
        printf("ADR log validation: PASSED\n");
    }

    return 0;
}
