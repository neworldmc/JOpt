#include "u.h"

void *
xmalloc(size_t size)
{
    void *p = malloc(size);
    if (!p) {
        fputs("out of memory\n", stderr);
        abort();
    }
    return p;
}

void *
xrealloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size);
    if (!p) {
        fputs("out of memory\n", stderr);
        abort();
    }
    return p;
}

/* region-based memory management */

#define CHUNK_SIZE 0x1000
#define ALIGN(x,a) (((x)+(a)-1)&(-(a)))

typedef struct chunk {
    struct chunk *next;
    void *limit;
    char data[];
} Chunk;

static void
new_chunk(Region *r, int data_size)
{
    int minsize = sizeof(Chunk) + data_size;
    int malloc_size = minsize < CHUNK_SIZE ? CHUNK_SIZE : minsize;
    Chunk *c;
    assert(malloc_size >= 0);
    c = xmalloc(malloc_size);
    c->next = r->head;
    c->limit = (char *) c + malloc_size;
    r->head = c;
    r->cur = c->data;
    r->limit = c->limit;
}

void *
ralloc(Region *r, int size, int align)
{
    void *ret;
    assert(size >= 0);
    size = ALIGN(size, align);
    if (r->cur + size > r->limit) {
        new_chunk(r, size);
    }
    ret = r->cur;
    r->cur += size;
    return ret;
}

void
rinit(Region *r)
{
    r->head = 0;
    new_chunk(r, 0);
}

void
rfreeall(Region *r)
{
    Chunk *c = r->head;
    while (c) {
        Chunk *nextc = c->next;
        free(c);
        c = nextc;
    }
    r->cur = 0;
    r->limit = 0;
    r->head = 0;
}

void
rfree(Region *r, void *p)
{
    Chunk *c = r->head;
    while (!(p >= (void *) c && p <= c->limit)) {
        Chunk *nextc = c->next;
        assert(nextc);
        free(c);
        c = nextc;
    }
    r->cur = p;
    r->limit = c->limit;
    r->head = c;
}

/* printf */

#define INIT_BUFSIZE 32

typedef void (*Formatter)();

enum {
    F_SIGNED = 1,
    F_UPPERCASE = 2,
    F_PRINTSIGN = 4,
    F_PLUS_SIGN = 8,
    F_HAVE_PREC = 16,
};

union u {
    int i;
    long l;
    INT64_ L;
};

/* buffer */

static void
heapbuf_grow(HeapBuf *hb, int newsize)
{
    char *newbuf = xrealloc(hb->start, newsize * sizeof *newbuf);
    int len;
    len = hb->cur - hb->start;
    hb->start = newbuf;
    hb->cur = newbuf + len;
    hb->end = newbuf + newsize;
}

static void
heapbuf_ensure_avail(HeapBuf *hb, int n)
{
    if (hb->end - hb->cur < n) {
        int oldsize = hb->end - hb->start;
        int newsize = oldsize*2+n;
        heapbuf_grow(hb, newsize);
    }
}

static void
heapbuf_putc(Buf *b, char c)
{
    HeapBuf *hb = (HeapBuf *) b;
    heapbuf_ensure_avail(hb, 1);
    *hb->cur++ = c;
}

static void
heapbuf_puts(Buf *b, const char *s, int n)
{
    HeapBuf *hb = (HeapBuf *) b;
    heapbuf_ensure_avail(hb, n);
    memcpy(hb->cur, s, n * sizeof *s);
    hb->cur += n;
}

void
init_heapbuf_size(HeapBuf *hb, int size)
{
    char *buf;
    assert(size >= 0);
    buf = xmalloc(INIT_BUFSIZE * sizeof *buf);
    hb->buf.putc = heapbuf_putc;
    hb->buf.puts = heapbuf_puts;
    hb->start = buf;
    hb->cur = buf;
    hb->end = buf + size;
}

void
init_heapbuf(HeapBuf *hb)
{
    init_heapbuf_size(hb, INIT_BUFSIZE);
}

char *
finish_heapbuf(HeapBuf *hb)
{
    if (hb->cur == hb->end) {
        heapbuf_grow(hb, hb->cur - hb->start + 1);
    }
    *hb->cur = 0;
    return hb->start;
}

static void
filebuf_putc(Buf *b, char c)
{
    FileBuf *fb = (FileBuf *) b;
    fputc(c, fb->fp);
}

static void
filebuf_puts(Buf *b, const char *s, int n)
{
    while (n--) filebuf_putc(b, *s++);
}

void
init_filebuf(FileBuf *fb, FILE *fp)
{
    fb->buf.putc = filebuf_putc;
    fb->buf.puts = filebuf_puts;
    fb->fp = fp;
}

static void
fixedbuf_putc(Buf *b, char c)
{
    FixedBuf *fb = (FixedBuf *) b;
    *fb->cur++ = c;
    *fb->cur = 0;
}

static void
fixedbuf_puts(Buf *b, const char *s, int n)
{
    FixedBuf *fb = (FixedBuf *) b;
    memcpy(fb->cur, s, n * sizeof *s);
    fb->cur += n;
    *fb->cur = 0;
}

void
init_fixedbuf(FixedBuf *fb, char *start)
{
    fb->buf.putc = fixedbuf_putc;
    fb->buf.puts = fixedbuf_puts;
    fb->cur = start;
}

static void
regionbuf_grow(RegionBuf *rb, int newsize)
{
    Region *r = rb->r;
    char *oldcur = r->cur;
    int oldlen = oldcur - (char *) rb->start; // this many bytes need to be copied
    new_chunk(r, newsize);
    /* move data in (rb->start -- r->cur) to new area */
    memcpy(r->cur, rb->start, oldlen);
    rb->start = (char *) r->cur;
    r->cur += oldlen;
}

static void
regionbuf_ensure_avail(RegionBuf *rb, int n)
{
    Region *r = rb->r;
    if ((char *) r->limit - (char *) r->cur < n) {
        int oldsize = (char *) r->limit - rb->start;
        int newsize = oldsize*2+n;
        regionbuf_grow(rb, newsize);
    }
}

static void
regionbuf_putc(Buf *b, char c)
{
    RegionBuf *rb = (RegionBuf *) b;
    Region *r = rb->r;
    regionbuf_ensure_avail(rb, 1);
    *(char*)r->cur = c;
    r->cur++;
}

static void
regionbuf_puts(Buf *b, const char *s, int n)
{
    RegionBuf *rb = (RegionBuf *) b;
    Region *r = rb->r;
    regionbuf_ensure_avail(rb, n);
    memcpy(r->cur, s, n * sizeof *s);
    r->cur += n * sizeof *s;
}

void
init_regionbuf(RegionBuf *rb, Region *r)
{
    rb->buf.putc = regionbuf_putc;
    rb->buf.puts = regionbuf_puts;
    rb->r = r;
    rb->start = (char *) r->cur;
}

char *
finish_regionbuf(RegionBuf *rb)
{
    Region *r = rb->r;
    if (r->cur == r->limit) {
        regionbuf_grow(rb, (char *) r->cur - rb->start + 1);
    }
    *(char*)r->cur = 0;
    r->cur++;
    return rb->start;
}


/*
 * printf, based on Michal Ludvig's minimal snprintf() implementation
 *
 * Copyright (c) 2013,2014 Michal Ludvig <michal@logix.cz>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the author nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

typedef unsigned char uchar;
typedef unsigned int uint;

#define PUTNUM10(NAME, FIELD, TYPE)\
    static char *\
    NAME(union u *value, char *buf, uint f)\
{\
    TYPE v = value->FIELD;\
    uchar neg = (f & F_SIGNED) && v < 0;\
    char *p = buf;\
    if (neg) v = -v;\
    if (v || !(f & F_HAVE_PREC)) do {\
        int d = (int)((unsigned TYPE) v % 10);\
        *p++ = '0'+d;\
        v = (unsigned TYPE) v / 10;\
    } while (v);\
    if (neg) *p++ = '-';\
    else if (f & F_PRINTSIGN) *p++ = f & F_PLUS_SIGN ? '+' : ' ';\
    return p;\
}

#define PUTNUM16(NAME, FIELD, TYPE)\
    static char *\
    NAME(union u *value, char *buf, uint f)\
{\
    TYPE v = value->FIELD;\
    uchar upcase = (f & F_UPPERCASE) != 0;\
    char *p = buf;\
    if (v || !(f & F_HAVE_PREC)) do {\
        int d = (int) v & 15;\
        *p++ = d < 10 ? '0'+d : (upcase?'A':'a')+(d-10);\
        v = (unsigned TYPE) v >> 4;\
    } while (v);\
    return p;\
}

PUTNUM10(puti10, i, int)
PUTNUM10(putl10, l, long)
PUTNUM10(putL10, L, INT64_)
PUTNUM16(puti16, i, int)
PUTNUM16(putl16, l, long)
PUTNUM16(putL16, L, INT64_)

typedef char *(*conv_fn)(union u *, char *, uint);

static void
putn(void *value, Buf *b, conv_fn conv, int prec, uint flags)
{
    char buf[20]; /* NO TRAILING NUL */

    /* This builds the string back to front ... */
    int len = conv(value, buf, flags) - buf;
    int i;
    /* ... now we reverse it (could do it recursively but will
       conserve the stack space) */
    for (i = 0; i < len/2; i++) {
        char tmp = buf[i];
        buf[i] = buf[len-i-1];
        buf[len-i-1] = tmp;
    }

    if (flags & F_HAVE_PREC) {
        for (i=len; i<prec; i++) b->putc(b, '0');
    }
    b->puts(b, buf, len);
}

void
vbprintf(Buf *b, const char *fmt, va_list va)
{
    char ch;

    while ((ch = *fmt++)) {
        uint flags;
        char *s;
        int n;
        int prec;
        int l;
        conv_fn conv;
        union u val;
        Formatter f;
        void *d;

        if (ch!='%') {
            b->putc(b, ch);
            continue;
        }

        flags = 0;
        prec = 0;
        l = 0;

        ch = *fmt++;
flags:
        switch (ch) {
        case ' ':
            ch = *fmt++;
            flags |= F_PRINTSIGN;
            goto flags;
        case '+':
            ch = *fmt++;
            flags |= F_PRINTSIGN | F_PLUS_SIGN;
            goto flags;
        }

        /* precision */
        if (ch == '.') {
            ch = *fmt++;
            flags |= F_HAVE_PREC;
            if (ch == '*') {
                ch = *fmt++;
                prec = va_arg(va, int);
                if (prec < 0) prec = 0;
            } else {
                prec = 0;
                while (ch >= '0' && ch <= '9') {
                    prec = prec*10 + (ch - '0');
                    ch = *fmt++;
                }
            }
        }

spec:
        switch (ch) {
        case 0:
            return;

        case 'l':
            l++;
            ch = *fmt++;
            goto spec;

        case 'd':
            flags |= F_SIGNED;
            /* fallthrough */
        case 'u':
            switch (l) {
            case 0:
                conv = puti10;
                val.i = va_arg(va, int);
                break;
            case 1:
                conv = putl10;
                val.l = va_arg(va, long);
                break;
            default:
                conv = putL10;
                val.L = va_arg(va, INT64_);
            }
            putn(&val, b, conv, prec, flags);
            break;

        case 'X':
            flags |= F_UPPERCASE;
            /* fallthrough */
        case 'x':
            switch (l) {
            case 0:
                conv = puti16;
                val.i = va_arg(va, int);
                break;
            case 1:
                conv = putl16;
                val.l = va_arg(va, long);
                break;
            default:
                conv = putL16;
                val.L = va_arg(va, INT64_);
            }
            putn(&val, b, conv, prec, flags);
            break;

        case 'c':
            val.i = va_arg(va, int);
            b->putc(b, val.i);
            break;

        case 's':
            s = va_arg(va, char *);
            if (flags & F_HAVE_PREC) {
                /* be careful not to read past (s+prec) */
                n = 0;
                while (n < prec && s[n]) n++;
            } else {
                n = strlen(s);
            }
            b->puts(b, s, n);
            break;

        case 'a':
            f = va_arg(va, Formatter);
            d = va_arg(va, void *);
            f(b, d);
            break;

        default:
            b->putc(b, ch);
        }
    }
}

void
bprintf(Buf *b, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    vbprintf(b, fmt, va);
    va_end(va);
}

#if 0
void
_vsprintf(char *s, const char *fmt, va_list va)
{
    FixedBuf fb;
    init_fixedbuf(&fb, s);
    vbprintf(&fb.buf, fmt, va);
}

void
_sprintf(char *buf, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    _vsprintf(buf, fmt, va);
    va_end(va);
}

char *
asprintf(const char *fmt, ...)
{
    HeapBuf hb;
    va_list va;
    init_heapbuf(&hb);
    va_start(va, fmt);
    vbprintf(&hb.buf, fmt, va);
    va_end(va);
    return finish_heapbuf(&hb);
}
#endif

char *
rsprintf(Region *r, const char *fmt, ...)
{
    RegionBuf rb;
    va_list va;
    init_regionbuf(&rb, r);
    va_start(va, fmt);
    vbprintf(&rb.buf, fmt, va);
    va_end(va);
    return finish_regionbuf(&rb);
}

#if 0
void
_vfprintf(FILE *fp, const char *fmt, va_list va)
{
    FileBuf fb;
    init_filebuf(&fb, fp);
    vbprintf(&fb.buf, fmt, va);
}

void
_fprintf(FILE *fp, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    _vfprintf(fp, fmt, va);
    va_end(va);
}

void
_printf(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    _vfprintf(stdout, fmt, va);
    va_end(va);
}

void
eprintf(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    _vfprintf(stderr, fmt, va);
    va_end(va);
}
#endif
