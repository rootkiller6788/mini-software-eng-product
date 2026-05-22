#include "git_internals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
 * doc_git_internals.c — Educational documentation on Git internals
 *
 * ── Git Object Model ──
 *   Git is a content-addressable filesystem. Everything is stored as an
 *   object identified by a SHA-1 hash of its contents. There are four
 *   object types:
 *
 *   1. BLOB   — raw file content (no filename, no metadata)
 *   2. TREE   — directory listing (maps names to blob/tree OIDs)
 *   3. COMMIT — snapshot reference (points to a tree, lists parents)
 *   4. TAG    — named reference to an object (usually a commit)
 *
 * ── SHA-1 Content Addressing ──
 *   Git prepends the object type and size to the content before hashing:
 *     hash = SHA1(type + " " + len + "\0" + content)
 *   This creates a unique identifier for every object. The same content
 *   always produces the same OID.
 *
 * ── Object Storage ──
 *   Loose objects:  .git/objects/xx/yyyyyyyyyyyyyy...
 *     - First byte of hex OID is directory, remaining 19 bytes are filename
 *     - Stored as deflate-compressed blob
 *
 *   Packfiles:       .git/objects/pack/*.pack + *.idx
 *     - Multiple objects stored in a single file with delta compression
 *     - Pack index (.idx) maps OIDs to pack file offsets
 *     - Delta chain: object = base + series_of_deltas
 *
 * ── Refs ──
 *   HEAD:    symbolic ref → "ref: refs/heads/main" or detached (raw OID)
 *   Branches: .git/refs/heads/<name> → OID
 *   Tags:     .git/refs/tags/<name> → OID
 *
 *   Packed refs: .git/packed-refs contains all packed refs for efficiency
 *
 * ── Index (Staging Area) ──
 *   .git/index is a binary file containing:
 *     - Version (2, 3, or 4)
 *     - Entry count
 *     - For each entry: ctime, mtime, dev, ino, mode, uid, gid, size, OID,
 *       flags, path
 *   The index tracks stat information for fast "git status" comparisons.
 *
 * ── Delta Compression ──
 *   When creating packfiles, Git uses delta compression to store similar
 *   objects compactly. A delta is a sequence of instructions:
 *     COPY <offset,size>: copy bytes from base object
 *     INSERT <data>:      insert new bytes
 *   This minimizes storage for similar files across revisions.
 *
 * ── Thin Packs ──
 *   During network transfer (git push/pull), a "thin pack" is sent where
 *   delta base objects may not be present on the receiver's side. The
 *   receiver is expected to already have those base objects and completes
 *   the pack on their end.
 */

/* ── Object construction walkthrough ────────────────────────────── */

static void doc_object_model(void) {
    printf("═══════════════════════════════════════════════\n");
    printf("  GIT OBJECT MODEL\n");
    printf("═══════════════════════════════════════════════\n\n");

    const char *file_content = "print('Hello, World!')";
    GitOid oid;
    git_hash_object((const unsigned char *)file_content,
                     strlen(file_content), GIT_OBJ_BLOB, &oid);

    printf("Input:  %s\n", file_content);
    printf("Type:   blob\n");
    printf("Size:   %zu bytes\n", strlen(file_content));
    printf("OID:    ");
    int i;
    for (i = 0; i < GIT_SHA1_RAWSZ; i++)
        printf("%02x", oid.id[i]);
    printf("\n\n");

    printf("Storage path: .git/objects/");
    printf("%02x/", oid.id[0]);
    for (i = 1; i < GIT_SHA1_RAWSZ; i++)
        printf("%02x", oid.id[i]);
    printf("\n\n");

    printf("A tree object would reference this blob:\n");
    printf("  100644 blob %02x%02x%02x%02x...\thello.py\n\n",
           oid.id[0], oid.id[1], oid.id[2], oid.id[3]);

    printf("A commit would look like:\n");
    printf("  tree <tree_oid>\n");
    printf("  parent <parent_oid>\n");
    printf("  author John <john@example.com> %lld +0000\n",
           (long long)time(NULL));
    printf("  committer John <john@example.com> %lld +0000\n",
           (long long)time(NULL));
    printf("  \n");
    printf("  Add hello world script\n\n");
}

/* ── SHA-1 walkthrough ──────────────────────────────────────────── */

static void doc_sha1_walkthrough(void) {
    printf("═══════════════════════════════════════════════\n");
    printf("  SHA-1 CONTENT ADDRESSING\n");
    printf("═══════════════════════════════════════════════\n\n");

    printf("Algorithm: FIPS PUB 180-4 (160-bit hash)\n\n");
    printf("Git's usage:\n");
    printf("  1. Construct header:  \"<type> <len>\\0\"\n");
    printf("  2. Concatenate:       header + content\n");
    printf("  3. Compute SHA1:      sha1(header + content)\n");
    printf("  4. Encode hex:        40-character string\n\n");

    printf("Example:\n");
    printf("  Input:   \"blob 16\\0what is up, doc?\"\n");
    printf("  Header:  \"blob 16\"\n");
    printf("  Output:  bd9dbf5aae1a3862dd1526723246b20206e5fc37\n\n");

    printf("Properties:\n");
    printf("  - Deterministic: same content → same OID\n");
    printf("  - Content-addressable: OID identifies content uniquely\n");
    printf("  - Deduplication: identical blobs share the same OID\n");
    printf("  - Integrity: any corruption changes the OID\n\n");
}

/* ── Packfile walkthrough ───────────────────────────────────────── */

static void doc_packfile(void) {
    printf("═══════════════════════════════════════════════\n");
    printf("  PACKFILE FORMAT\n");
    printf("═══════════════════════════════════════════════\n\n");

    printf("Packfile structure:\n");
    printf("  ┌─────────────────────────────────────────┐\n");
    printf("  │ PACK          (4-byte signature)        │\n");
    printf("  │ [version]     (4-byte, always 2 or 3)   │\n");
    printf("  │ [obj_count]   (4-byte, number of objs)  │\n");
    printf("  │                                         │\n");
    printf("  │ Object 1..N  (each: type+size+data)     │\n");
    printf("  │                                         │\n");
    printf("  │ SHA1 checksum (20-byte, of everything)  │\n");
    printf("  └─────────────────────────────────────────┘\n\n");

    printf("Object types in pack (first byte):\n");
    printf("  OBJ_COMMIT  = 1\n");
    printf("  OBJ_TREE    = 2\n");
    printf("  OBJ_BLOB    = 3\n");
    printf("  OBJ_TAG     = 4\n");
    printf("  OBJ_OFS_DELTA = 6  (offset-based delta)\n");
    printf("  OBJ_REF_DELTA = 7  (OID-based delta)\n\n");

    printf("Delta chain example:\n");
    printf("  Blob A  → full content \"line1\\nline2\\nline3\\n\"\n");
    printf("  Blob B  → delta from A: INSERT \"line2.5\\n\" at pos 6\n");
    printf("  To reconstruct B: apply delta instructions to A\n\n");
}

/* ── Index walkthrough ──────────────────────────────────────────── */

static void doc_index(void) {
    printf("═══════════════════════════════════════════════\n");
    printf("  INDEX (STAGING AREA)\n");
    printf("═══════════════════════════════════════════════\n\n");

    printf("Location: .git/index\n\n");

    printf("Header:\n");
    printf("  DIRC  (4-byte signature)\n");
    printf("  [version]  (4-byte: 2, 3, or 4)\n");
    printf("  [entries]  (4-byte: count)\n\n");

    printf("Each entry contains:\n");
    printf("  - stat info: ctime, mtime, dev, ino, mode,\n");
    printf("                uid, gid, file_size\n");
    printf("  - SHA-1 OID of the staged blob\n");
    printf("  - flags (assume-valid, skip-worktree, etc.)\n");
    printf("  - path (null-terminated, variable length)\n\n");

    printf("Version differences:\n");
    printf("  V2: entry names are fixed-length padded\n");
    printf("  V3: support for extended flags\n");
    printf("  V4: path prefix compression to reduce size\n\n");
}

/* ── Ref system walkthrough ─────────────────────────────────────── */

static void doc_refs(void) {
    printf("═══════════════════════════════════════════════\n");
    printf("  REF SYSTEM\n");
    printf("═══════════════════════════════════════════════\n\n");

    printf("Directory layout:\n");
    printf("  .git/\n");
    printf("  ├── HEAD              \"ref: refs/heads/main\"\n");
    printf("  ├── refs/\n");
    printf("  │   ├── heads/main    → commit OID\n");
    printf("  │   ├── heads/develop → commit OID\n");
    printf("  │   ├── remotes/origin/main → commit OID\n");
    printf("  │   └── tags/v1.0.0   → tag OID (or commit OID)\n");
    printf("  └── packed-refs       (all refs in one file)\n\n");

    printf("Symbolic refs:\n");
    printf("  - HEAD usually points to a branch\n");
    printf("  - Can be \"detached\" (points directly to an OID)\n\n");

    printf("Branch lifecycle:\n");
    printf("  1. Create:  echo <oid> > .git/refs/heads/<name>\n");
    printf("  2. Update:  advance the OID on each commit\n");
    printf("  3. Delete:  rm .git/refs/heads/<name>\n\n");

    printf("Tags:\n");
    printf("  - Lightweight: just a ref pointing to an OID\n");
    printf("  - Annotated:   a full tag object with message,\n");
    printf("                 tagger, date, and PGP signature\n\n");
}

/* ── Main ───────────────────────────────────────────────────────── */

int main(void) {
    printf("GIT INTERNALS — C99 Educational Documentation\n");
    printf("==============================================\n\n");

    doc_object_model();
    doc_sha1_walkthrough();
    doc_packfile();
    doc_index();
    doc_refs();

    printf("═══════════════════════════════════════════════\n");
    printf("  FURTHER READING\n");
    printf("═══════════════════════════════════════════════\n\n");
    printf("  - Git source:    https://github.com/git/git\n");
    printf("  - Pro Git book:  https://git-scm.com/book\n");
    printf("  - Git internals: https://git-scm.com/book/en/v2/Git-Internals\n");
    printf("  - Pack format:   https://git-scm.com/docs/pack-format\n");
    printf("  - Index format:  https://git-scm.com/docs/index-format\n\n");

    return 0;
}
