/* Essential utilities */

#pragma once

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define unreachable() assert(0)
/* For "unsigned INT64_" to be valid, INT64_ must not be a typedef name. */
#define INT64_ long long
#define bputc(b, c) (b)->putc(b, c)
#define bputs(b, s) (b)->puts(b, s, strlen(s))
#define NELEM(x) (sizeof (x) / sizeof *(x))

typedef struct {
    char *cur, *limit;
    struct chunk *head;
} Region;

typedef struct buf {
    void (*putc)(struct buf *, char);
    void (*puts)(struct buf *, const char *, int);
} Buf;

typedef struct {
    Buf buf;
    char *start, *cur, *end;
} HeapBuf;

/* WARNING: FixedBuf does not check for buffer overflow! */
typedef struct {
    Buf buf;
    char *cur;
} FixedBuf;

typedef struct {
    Buf buf;
    FILE *fp;
} FileBuf;

typedef struct {
    Buf buf;
    Region *r;
    char *start;
} RegionBuf;

#ifdef __cplusplus
extern "C" {
#endif

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);

void *ralloc(Region *r, int size, int align);
void rinit(Region *r);
void rfreeall(Region *r);
void rfree(Region *r, void *p);
void ralign(Region *r, int align);

void init_heapbuf(HeapBuf *);
void init_heapbuf_size(HeapBuf *, int);
char *finish_heapbuf(HeapBuf *);
void init_fixedbuf(FixedBuf *, char *);
void init_filebuf(FileBuf *, FILE *);
void init_regionbuf(RegionBuf *, Region *);
char *finish_regionbuf(RegionBuf *);

void vbprintf(Buf *b, const char *fmt, va_list va);
void bprintf(Buf *b, const char *fmt, ...);
//char *asprintf(const char *fmt, ...);
char *rsprintf(Region *r, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <cstddef>
#include <utility>
void *operator new(std::size_t, Region &r);
void *operator new(std::size_t, std::align_val_t, Region &r);
void *operator new[](std::size_t, Region &r);
void *operator new[](std::size_t, std::align_val_t, Region &r);
#else
#define NEW(p, r) p = ralloc(r, sizeof *p, alignof(*p))
#define NEWARRAY(p, n, r) p = ralloc(r, (n) * sizeof *p, alignof(*p))
#endif
