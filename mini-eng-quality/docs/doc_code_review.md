# Code Review Module

## Overview

The code review module (`code_review.h`) provides a lightweight C99 implementation of core code review workflows, simulating features found in tools like Gerrit and GitHub pull requests.

## Diff Parsing

Unified diff format parsing extracts file paths, hunks, and line counts:

```c
const char *diff = "diff --git a/main.c b/main.c\n...";
cr_review_t review;
cr_parse_diff(&review, diff);
```

### Supported Format
- `diff --git a/path b/path` headers
- `@@ -start,count +start,count @@` hunk headers
- `+` / `-` line tracking for addition/deletion counts

## Comment → Resolve Flow

Comments follow a threaded conversation model:

| State | Description |
|-------|-------------|
| `CR_COMMENT_OPEN` | New, unresolved comment |
| `CR_COMMENT_RESOLVED` | Marked resolved by reviewer |
| `CR_COMMENT_WONT_FIX` | Acknowledged but not actionable |
| `CR_COMMENT_DUPLICATE` | Marked as duplicate |

### Resolution Flow
1. Reviewer adds comment with severity and file/line context
2. Author replies via `cr_reply_to_comment()`
3. Comment thread resolved via `cr_resolve_comment()`
4. Can be reopened with `cr_reopen_comment()`

## Reviewer Assignment

Ownership-based suggestion using file path pattern matching:

```c
cr_ownership_t owners[] = {
    { "src/auth/", {"alice", "bob"}, 2 },
    { "include/",  {"charlie"}, 1 }
};
cr_suggest_reviewers(&review, owners, 2);
```

## Review Checklist

Predefined checklist items ensure review completeness:
- Code compiles without warnings
- Unit tests pass
- No memory leaks
- Documentation updated

## Approval Rules

Configurable gates including minimum approvals and owner requirements.
Statistics tracking: total reviews, resolution time, comment density.

## Module Dependencies

- Standard C99 library only (`<stdlib.h>`, `<string.h>`, `<stdio.h>`)
- No external dependencies
