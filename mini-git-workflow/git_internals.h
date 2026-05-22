#ifndef GIT_INTERNALS_H
#define GIT_INTERNALS_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>

/* ── Git Object Types ───────────────────────────────────────────── */

typedef enum {
    GIT_OBJ_NONE = 0,
    GIT_OBJ_BLOB,
    GIT_OBJ_TREE,
    GIT_OBJ_COMMIT,
    GIT_OBJ_TAG
} GitObjectType;

/* ── SHA-1 ──────────────────────────────────────────────────────── */

#define GIT_SHA1_RAWSZ  20
#define GIT_SHA1_HEXSZ  40

typedef struct {
    unsigned char id[GIT_SHA1_RAWSZ];
} GitOid;

typedef struct {
    char hex[GIT_SHA1_HEXSZ + 1];
} GitOidHex;

/* ── Blob (raw file content, no metadata) ───────────────────────── */

typedef struct {
    GitOid   oid;
    size_t   size;              /* raw content size in bytes */
    char    *content;           /* raw content (may be binary) */
} GitBlob;

/* ── Tree entry ─────────────────────────────────────────────────── */

typedef enum {
    GIT_FILEMODE_NORMAL = 0100644,
    GIT_FILEMODE_EXEC   = 0100755,
    GIT_FILEMODE_SYMLINK = 0120000,
    GIT_FILEMODE_DIR    = 0040000,
    GIT_FILEMODE_GITLINK = 0160000
} GitFilemode;

typedef struct {
    GitFilemode  mode;
    GitObjectType type;          /* blob or tree */
    GitOid       oid;
    char        *name;           /* entry name */
} GitTreeEntry;

typedef struct {
    GitOid         oid;
    GitTreeEntry  *entries;
    size_t         entry_count;
} GitTree;

/* ── Commit (parent, message, snapshot) ─────────────────────────── */

typedef struct {
    char *name;
    char *email;
} GitSignature;

typedef struct {
    GitOid        oid;

    GitOid        tree_oid;      /* root tree snapshot */
    GitOid       *parents;       /* 1+ parent oids */
    size_t        parent_count;

    GitSignature  author;
    GitSignature  committer;
    time_t        author_time;
    time_t        committer_time;

    char         *message;

    /* extra headers (e.g. "gpgsig") */
    char        **extra_keys;
    char        **extra_values;
    size_t        extra_count;
} GitCommit;

/* ── Tag ────────────────────────────────────────────────────────── */

typedef enum {
    GIT_TAG_LIGHTWEIGHT = 0,  /* ref only */
    GIT_TAG_ANNOTATED          /* full object */
} GitTagType;

typedef struct {
    GitOid        oid;
    GitTagType    type;
    GitOid        target_oid;   /* oid being tagged */
    GitObjectType target_type;
    char         *name;
    char         *message;
    GitSignature  tagger;
    time_t        tag_time;
} GitTag;

/* ── Object Storage ─────────────────────────────────────────────── */

/* loose object: .git/objects/xx/yyyy...   (deflate-compressed) */
typedef struct {
    GitObjectType type;
    size_t        raw_size;    /* uncompressed content size */
    unsigned char *raw_data;
} GitRawObject;

/* Packfile delta instruction */
typedef struct {
    unsigned char  opcode;     /* COPY or INSERT */
    size_t         src_offset; /* for COPY */
    size_t         src_size;
    unsigned char *data;       /* for INSERT */
    size_t         data_len;
} GitDeltaInst;

typedef struct {
    GitObjectType type;
    size_t        result_size;
    GitOid        base_oid;    /* base object for delta */
    GitDeltaInst *instructions;
    size_t        inst_count;
} GitDeltaObject;

/* Thin pack = packfile sent over network without base objects */
typedef struct {
    unsigned int  version;      /* 2 */
    unsigned int  object_count;
    GitDeltaObject *objects;
    int            is_thin;    /* missing base objects */
} GitPackfile;

/* ── Refs ───────────────────────────────────────────────────────── */

typedef enum {
    GIT_REF_SYMBOLIC,  /* ref: refs/heads/main */
    GIT_REF_DIRECT     /* points to an oid */
} GitRefType;

typedef struct {
    char        *name;       /* full ref name, e.g. "refs/heads/main" */
    GitRefType   type;
    GitOid       oid;        /* valid for DIRECT */
    char        *target;     /* valid for SYMBOLIC, e.g. "refs/heads/main" */
} GitRef;

/* HEAD – special symbolic ref pointing to current branch */
typedef struct {
    GitRefType   type;
    GitOid       oid;        /* detached HEAD */
    char        *target;     /* e.g. "refs/heads/main" */
} GitHead;

/* Branch metadata */
typedef struct {
    char   *name;             /* short name, e.g. "main" */
    GitOid  tip_oid;          /* latest commit */
    char   *upstream;         /* e.g. "refs/remotes/origin/main" or NULL */
} GitBranch;

/* ── Index (staging area) ───────────────────────────────────────── */

typedef struct {
    unsigned int ctime_sec, ctime_nsec;
    unsigned int mtime_sec, mtime_nsec;
    unsigned int dev;
    unsigned int ino;
    unsigned int mode;
    unsigned int uid;
    unsigned int gid;
    unsigned int file_size;
    GitOid       oid;
    unsigned short flags;
    unsigned short flags_extended;
    char         *path;
} GitIndexEntry;

typedef struct {
    unsigned int   version;    /* 2, 3, or 4 */
    GitIndexEntry *entries;
    size_t         entry_count;
} GitIndex;

/* ── Reference Storage Backend ──────────────────────────────────── */

typedef struct {
    /* pack-refs format on disk */
    GitRef  **refs;
    size_t    ref_count;
    int       peeled;        /* fully peeled */
} GitPackedRefs;

/* ── Object Database ────────────────────────────────────────────── */

typedef struct {
    char       *objects_dir;   /* .git/objects */
    GitPackfile *packs;
    size_t       pack_count;
} GitOdb;  /* object database */

/* ── Core API ───────────────────────────────────────────────────── */

/* Hash content → OID */
void    git_hash_object(const unsigned char *data, size_t len,
                        GitObjectType type, GitOid *out);

/* Write a loose object to disk */
int     git_odb_write(GitOdb *odb, GitObjectType type,
                      const unsigned char *data, size_t len, GitOid *out);

/* Read a loose object by OID */
int     git_odb_read(GitOdb *odb, const GitOid *oid,
                     GitRawObject *out);

/* Check if an object exists in the odb */
int     git_odb_exists(GitOdb *odb, const GitOid *oid);

/* Free raw object memory */
void    git_raw_object_free(GitRawObject *obj);

/* Pack file helpers */
int     git_packfile_write(const char *path, const GitPackfile *pack);
int     git_packfile_read(const char *path, GitPackfile *out);
void    git_packfile_free(GitPackfile *pack);

/* Delta compress src → dst; caller owns output */
int     git_delta_create(const unsigned char *src, size_t src_len,
                         const unsigned char *dst, size_t dst_len,
                         unsigned char **out, size_t *out_len);

/* Apply delta instructions to base → result */
int     git_delta_apply(const unsigned char *base, size_t base_len,
                        const GitDeltaObject *delta,
                        unsigned char **out, size_t *out_len);

/* Ref helpers */
void    git_ref_free(GitRef *ref);
int     git_ref_resolve(GitRef **refs, size_t count,
                        const char *name, GitOid *out_oid);

/* Index read/write */
int     git_index_read(GitIndex *idx, const char *path);
int     git_index_write(const GitIndex *idx, const char *path);
void    git_index_free(GitIndex *idx);

/* Commit helpers */
void    git_commit_free(GitCommit *c);
void    git_tree_free(GitTree *t);

#endif /* GIT_INTERNALS_H */
