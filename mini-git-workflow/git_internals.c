#include "git_internals.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* ── Simple SHA-1 (educational, not FIPS-compliant) ─────────────── */

static uint32_t sha1_rol(uint32_t v, unsigned int n) {
    return (v << n) | (v >> (32 - n));
}

static void sha1_transform(uint32_t state[5], const unsigned char block[64]) {
    uint32_t w[80], a, b, c, d, e, t;
    int i;

    for (i = 0; i < 16; i++)
        w[i] = ((uint32_t)block[i * 4] << 24) |
               ((uint32_t)block[i * 4 + 1] << 16) |
               ((uint32_t)block[i * 4 + 2] << 8) |
               ((uint32_t)block[i * 4 + 3]);
    for (i = 16; i < 80; i++)
        w[i] = sha1_rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

    a = state[0]; b = state[1]; c = state[2]; d = state[3]; e = state[4];
    for (i = 0; i < 80; i++) {
        if (i < 20)
            t = sha1_rol(a, 5) + ((b & c) | ((~b) & d)) + e + w[i] + 0x5A827999U;
        else if (i < 40)
            t = sha1_rol(a, 5) + (b ^ c ^ d) + e + w[i] + 0x6ED9EBA1U;
        else if (i < 60)
            t = sha1_rol(a, 5) + ((b & c) | (b & d) | (c & d)) + e + w[i] + 0x8F1BBCDCU;
        else
            t = sha1_rol(a, 5) + (b ^ c ^ d) + e + w[i] + 0xCA62C1D6U;
        e = d; d = c; c = sha1_rol(b, 30); b = a; a = t;
    }
    state[0] += a; state[1] += b; state[2] += c; state[3] += d; state[4] += e;
}

static void sha1_compute(const unsigned char *data, size_t len, unsigned char hash[20]) {
    uint32_t state[5] = { 0x67452301U, 0xEFCDAB89U, 0x98BADCFEU, 0x10325476U, 0xC3D2E1F0U };
    unsigned char block[64];
    size_t i = 0, blen = 0;
    uint64_t bits = (uint64_t)len * 8;

    while (i < len) {
        block[blen++] = data[i++];
        if (blen == 64) { sha1_transform(state, block); blen = 0; }
    }
    block[blen++] = 0x80;
    if (blen > 56) { while (blen < 64) block[blen++] = 0; sha1_transform(state, block); blen = 0; }
    while (blen < 56) block[blen++] = 0;
    for (i = 0; i < 8; i++) block[56 + i] = (unsigned char)(bits >> (56 - 8 * i));
    sha1_transform(state, block);
    for (i = 0; i < 20; i++) hash[i] = (unsigned char)(state[i / 4] >> (24 - 8 * (i % 4)));
}

/* ── Object construction ────────────────────────────────────────── */

static const char *obj_type_names[] = { "none", "blob", "tree", "commit", "tag" };

void git_hash_object(const unsigned char *data, size_t len,
                     GitObjectType type, GitOid *out) {
    char header[128];
    int hlen = snprintf(header, sizeof(header), "%s %zu", obj_type_names[type], len);
    unsigned char *buf = (unsigned char *)malloc(hlen + 1 + len);
    memcpy(buf, header, hlen);
    buf[hlen] = 0;
    memcpy(buf + hlen + 1, data, len);
    sha1_compute(buf, hlen + 1 + len, out->id);
    free(buf);
}

int git_odb_write(GitOdb *odb, GitObjectType type,
                  const unsigned char *data, size_t len, GitOid *out) {
    git_hash_object(data, len, type, out);
    char hex[GIT_SHA1_HEXSZ + 1];
    int i;
    for (i = 0; i < GIT_SHA1_RAWSZ; i++)
        sprintf(hex + i * 2, "%02x", out->id[i]);
    (void)odb;
    FILE *f = fopen(hex, "wb");
    if (!f) return -1;
    fwrite(data, 1, len, f);
    fclose(f);
    return 0;
}

int git_odb_read(GitOdb *odb, const GitOid *oid, GitRawObject *out) {
    char hex[GIT_SHA1_HEXSZ + 1];
    int i;
    for (i = 0; i < GIT_SHA1_RAWSZ; i++)
        sprintf(hex + i * 2, "%02x", oid->id[i]);
    (void)odb;
    FILE *f = fopen(hex, "rb");
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    out->raw_data = (unsigned char *)malloc((size_t)sz);
    out->raw_size = (size_t)sz;
    fread(out->raw_data, 1, (size_t)sz, f);
    fclose(f);
    out->type = GIT_OBJ_BLOB;
    return 0;
}

int git_odb_exists(GitOdb *odb, const GitOid *oid) {
    char hex[GIT_SHA1_HEXSZ + 1];
    int i;
    for (i = 0; i < GIT_SHA1_RAWSZ; i++)
        sprintf(hex + i * 2, "%02x", oid->id[i]);
    (void)odb;
    FILE *f = fopen(hex, "rb");
    if (f) { fclose(f); return 1; }
    return 0;
}

void git_raw_object_free(GitRawObject *obj) {
    if (!obj) return;
    free(obj->raw_data);
    obj->raw_data = NULL;
    obj->raw_size = 0;
}

/* ── Delta compression ──────────────────────────────────────────── */

int git_delta_create(const unsigned char *src, size_t src_len,
                     const unsigned char *dst, size_t dst_len,
                     unsigned char **out, size_t *out_len) {
    *out = (unsigned char *)malloc(dst_len * 3 / 2 + 64);
    size_t pos = 0, sp = 0;
    (void)src_len;
    while (sp < dst_len) {
        size_t best_off = 0, best_len = 0;
        size_t ss;
        for (ss = 0; ss < sp && ss < src_len; ss++) {
            size_t l = 0;
            while (sp + l < dst_len && ss + l < src_len && src[ss + l] == dst[sp + l]) l++;
            if (l > best_len) { best_off = ss; best_len = l; }
        }
        if (best_len >= 4) {
            (*out)[pos++] = 0x80;
            (*out)[pos++] = (unsigned char)(best_off & 0xFF);
            (*out)[pos++] = (unsigned char)((best_off >> 8) & 0xFF);
            (*out)[pos++] = (unsigned char)(best_len & 0xFF);
            sp += best_len;
        } else {
            (*out)[pos++] = dst[sp++];
        }
    }
    *out_len = pos;
    return 0;
}

int git_delta_apply(const unsigned char *base, size_t base_len,
                    const GitDeltaObject *delta,
                    unsigned char **out, size_t *out_len) {
    *out = (unsigned char *)malloc(delta->result_size + 1024);
    size_t pos = 0;
    size_t i;
    for (i = 0; i < delta->inst_count; i++) {
        if (delta->instructions[i].opcode == 0x80) {
            size_t off = delta->instructions[i].src_offset;
            size_t sz = delta->instructions[i].src_size;
            if (off + sz <= base_len) {
                memcpy(*out + pos, base + off, sz);
                pos += sz;
            }
        } else {
            memcpy(*out + pos, delta->instructions[i].data, delta->instructions[i].data_len);
            pos += delta->instructions[i].data_len;
        }
    }
    *out_len = pos;
    return 0;
}

/* ── Packfile stubs ─────────────────────────────────────────────── */

int git_packfile_write(const char *path, const GitPackfile *pack) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    fwrite("PACK", 1, 4, f);
    fwrite(&pack->version, sizeof(pack->version), 1, f);
    fwrite(&pack->object_count, sizeof(pack->object_count), 1, f);
    fclose(f);
    return 0;
}

int git_packfile_read(const char *path, GitPackfile *out) {
    (void)path;
    out->version = 2;
    out->object_count = 0;
    return 0;
}

void git_packfile_free(GitPackfile *pack) {
    if (!pack) return;
    free(pack->objects);
    pack->objects = NULL;
}

/* ── Ref helpers ────────────────────────────────────────────────── */

void git_ref_free(GitRef *ref) {
    if (!ref) return;
    free(ref->name);
    free(ref->target);
}

int git_ref_resolve(GitRef **refs, size_t count, const char *name, GitOid *out_oid) {
    size_t i;
    for (i = 0; i < count; i++) {
        if (refs[i]->name && strcmp(refs[i]->name, name) == 0) {
            if (refs[i]->type == GIT_REF_DIRECT) {
                memcpy(out_oid->id, refs[i]->oid.id, GIT_SHA1_RAWSZ);
                return 0;
            }
            if (refs[i]->type == GIT_REF_SYMBOLIC && refs[i]->target)
                return git_ref_resolve(refs, count, refs[i]->target, out_oid);
        }
    }
    return -1;
}

/* ── Index stubs ────────────────────────────────────────────────── */

int git_index_read(GitIndex *idx, const char *path) {
    (void)idx; (void)path;
    return 0;
}

int git_index_write(const GitIndex *idx, const char *path) {
    (void)idx; (void)path;
    return 0;
}

void git_index_free(GitIndex *idx) {
    if (!idx) return;
    size_t i;
    for (i = 0; i < idx->entry_count; i++)
        free(idx->entries[i].path);
    free(idx->entries);
    idx->entries = NULL;
    idx->entry_count = 0;
}

/* ── Commit / tree free ─────────────────────────────────────────── */

void git_commit_free(GitCommit *c) {
    if (!c) return;
    free(c->parents);
    free(c->author.name); free(c->author.email);
    free(c->committer.name); free(c->committer.email);
    free(c->message);
    size_t i;
    for (i = 0; i < c->extra_count; i++) { free(c->extra_keys[i]); free(c->extra_values[i]); }
    free(c->extra_keys); free(c->extra_values);
}

void git_tree_free(GitTree *t) {
    if (!t) return;
    size_t i;
    for (i = 0; i < t->entry_count; i++) free(t->entries[i].name);
    free(t->entries);
}
