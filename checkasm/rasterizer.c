/*
 * Copyright (C) 2021 Vabishchevich Nikolay <vabnick@gmail.com>
 *
 * This file is part of libass.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "checkasm.h"
#include "ass_rasterizer.h"

#define HEIGHT 34
#define STRIDE 96
#define BORD_X 32
#define BORD_Y 1
#define REP_COUNT 8
#define MAX_SEG 8


static inline int ilog2(uint32_t n)
{
#ifdef __GNUC__
    return __builtin_clz(n) ^ 31;
#elif defined(_MSC_VER)
    int res;
    _BitScanReverse(&res, n);
    return res;
#else
    int res = 0;
    for (int ord = 16; ord; ord /= 2)
        if (n >= ((uint32_t) 1 << ord)) {
            res += ord;
            n >>= ord;
        }
    return res;
#endif
}

static inline int segment_check_top(const struct segment *line, int32_t y)
{
    if (line->flags & SEGFLAG_EXACT_TOP)
        return line->y_min >= y;
    int64_t cc = line->c - line->b * (int64_t) y -
        line->a * (int64_t) (line->flags & SEGFLAG_UL_DR ? line->x_min : line->x_max);
    if (line->b < 0)
        cc = -cc;
    return cc >= 0;
}

static inline int segment_check_bottom(const struct segment *line, int32_t y)
{
    if (line->flags & SEGFLAG_EXACT_BOTTOM)
        return line->y_max <= y;
    int64_t cc = line->c - line->b * (int64_t) y -
        line->a * (int64_t) (line->flags & SEGFLAG_UL_DR ? line->x_max : line->x_min);
    if (line->b > 0)
        cc = -cc;
    return cc >= 0;
}

static bool generate_segment(struct segment *line, int tile_size)
{
    tile_size *= 64;
    const int mask = 2 * tile_size - 1, offs = tile_size >> 1;
    ASS_Vector pt0 = {(rnd() & mask) - offs, (rnd() & mask) - offs};
    ASS_Vector pt1 = {(rnd() & mask) - offs, (rnd() & mask) - offs};
    int32_t x = pt1.x - pt0.x;
    int32_t y = pt1.y - pt0.y;
    if (!x && !y)
        return false;

    line->flags = SEGFLAG_EXACT_LEFT | SEGFLAG_EXACT_RIGHT |
                  SEGFLAG_EXACT_TOP | SEGFLAG_EXACT_BOTTOM;
    if (x < 0)
        line->flags ^= SEGFLAG_UL_DR;
    if (y >= 0)
        line->flags ^= SEGFLAG_DN | SEGFLAG_UL_DR;

    line->x_min = FFMIN(pt0.x, pt1.x);
    line->x_max = FFMAX(pt0.x, pt1.x);
    line->y_min = FFMIN(pt0.y, pt1.y);
    line->y_max = FFMAX(pt0.y, pt1.y);
    if (line->x_min >= tile_size || line->x_max <= 0)
        return false;
    if (line->y_min >= tile_size || line->y_max <= 0)
        return false;

    line->a = y;
    line->b = -x;
    line->c = y * (int64_t) pt0.x - x * (int64_t) pt0.y;
    int32_t min_corner = FFMIN(0, line->a) + FFMIN(0, line->b);
    int32_t max_corner = FFMAX(0, line->a) + FFMAX(0, line->b);
    if (line->c - min_corner * (int64_t) tile_size <= 0)
        return false;
    if (line->c - max_corner * (int64_t) tile_size >= 0)
        return false;

    // halfplane normalization
    int32_t abs_x = x < 0 ? -x : x;
    int32_t abs_y = y < 0 ? -y : y;
    uint32_t max_ab = (abs_x > abs_y ? abs_x : abs_y);
    int shift = 30 - ilog2(max_ab);
    max_ab <<= shift + 1;
    line->a *= 1 << shift;
    line->b *= 1 << shift;
    line->c *= 1 << shift;
    line->scale = ((uint64_t) 1 << 61) / max_ab;

    if (line->x_min < 0) {
        line->x_min = 0;
        if (line->flags & SEGFLAG_UL_DR)
            line->flags &= ~SEGFLAG_EXACT_TOP;
        else
            line->flags &= ~SEGFLAG_EXACT_BOTTOM;
    }
    if (line->x_max > tile_size) {
        line->x_max = tile_size;
        if (line->flags & SEGFLAG_UL_DR)
            line->flags &= ~SEGFLAG_EXACT_BOTTOM;
        else
            line->flags &= ~SEGFLAG_EXACT_TOP;
    }
    if (segment_check_top(line, 0)) {
        line->y_min = FFMAX(line->y_min, 0);
    } else {
        line->y_min = 0;
        if (line->flags & SEGFLAG_UL_DR)
            line->flags &= ~SEGFLAG_EXACT_LEFT;
        else
            line->flags &= ~SEGFLAG_EXACT_RIGHT;
        line->flags |= SEGFLAG_EXACT_TOP;
    }
    if (segment_check_bottom(line, tile_size)) {
        line->y_max = FFMIN(line->y_max, tile_size);
    } else {
        line->y_max = tile_size;
        if (line->flags & SEGFLAG_UL_DR)
            line->flags &= ~SEGFLAG_EXACT_RIGHT;
        else
            line->flags &= ~SEGFLAG_EXACT_LEFT;
        line->flags |= SEGFLAG_EXACT_BOTTOM;
    }
    return true;
}


static void check_fill_solid(FillSolidTileFunc func, const char *name, int tile_size)
{
    if (check_func(func, name, tile_size)) {
        ALIGN(uint8_t buf_ref[STRIDE * HEIGHT], 32);
        ALIGN(uint8_t buf_new[STRIDE * HEIGHT], 32);
        declare_func(void,
                     uint8_t *buf, ptrdiff_t stride, int set);

        for(int set = 0; set <= 1; set++) {
            for (int i = 0; i < sizeof(buf_ref); i++)
                buf_ref[i] = buf_new[i] = rnd();

            call_ref(buf_ref + BORD_Y * STRIDE + BORD_X, STRIDE, set);
            call_new(buf_new + BORD_Y * STRIDE + BORD_X, STRIDE, set);

            if (memcmp(buf_ref, buf_new, sizeof(buf_ref))) {
                fail();
                break;
            }
        }

        bench_new(buf_new + BORD_Y * STRIDE + BORD_X, STRIDE, 1);
    }

    report(name, tile_size);
}

static void check_fill_halfplane(FillHalfplaneTileFunc func, const char *name, int tile_size)
{
    if (check_func(func, name, tile_size)) {
        ALIGN(uint8_t buf_ref[STRIDE * HEIGHT], 32);
        ALIGN(uint8_t buf_new[STRIDE * HEIGHT], 32);
        struct segment line;
        declare_func(void,
                     uint8_t *buf, ptrdiff_t stride,
                     int32_t a, int32_t b, int64_t c, int32_t scale);

        for(int rep = 0; rep < REP_COUNT; rep++) {
            for (int i = 0; i < sizeof(buf_ref); i++)
                buf_ref[i] = buf_new[i] = rnd();

            while (!generate_segment(&line, tile_size));

            call_ref(buf_ref + BORD_Y * STRIDE + BORD_X, STRIDE, line.a, line.b, line.c, line.scale);
            call_new(buf_new + BORD_Y * STRIDE + BORD_X, STRIDE, line.a, line.b, line.c, line.scale);

            if (memcmp(buf_ref, buf_new, sizeof(buf_ref))) {
                fail();
                break;
            }
        }

        bench_new(buf_new + BORD_Y * STRIDE + BORD_X, STRIDE, line.a, line.b, line.c, line.scale);
    }

    report(name, tile_size);
}

static void check_fill_generic(FillGenericTileFunc func, const char *name, int tile_size)
{
    if (check_func(func, name, tile_size)) {
        ALIGN(uint8_t buf_ref[STRIDE * HEIGHT], 32);
        ALIGN(uint8_t buf_new[STRIDE * HEIGHT], 32);
        struct segment line[MAX_SEG];
        int n;
        declare_func(void,
                     uint8_t *buf, ptrdiff_t stride,
                     const struct segment *line, size_t n_lines,
                     int winding);

        for(int rep = 0; rep < REP_COUNT; rep++) {
            for (int i = 0; i < sizeof(buf_ref); i++)
                buf_ref[i] = buf_new[i] = rnd();

            n = rnd() % MAX_SEG + 1;
            for (int i = 0; i < n; )
                if (generate_segment(line + i, tile_size))
                    i++;

            int winding = rnd() % 5 - 2;

            call_ref(buf_ref + BORD_Y * STRIDE + BORD_X, STRIDE, line, n, winding);
            call_new(buf_new + BORD_Y * STRIDE + BORD_X, STRIDE, line, n, winding);

            if (memcmp(buf_ref, buf_new, sizeof(buf_ref))) {
                fail();
                break;
            }
        }

        // unstable due to segment difference
        bench_new(buf_new + BORD_Y * STRIDE + BORD_X, STRIDE, line, n, 0);
    }

    report(name, tile_size);
}

void checkasm_check_rasterizer(const BitmapEngine *engine)
{
    int tile_size = 1 << engine->tile_order;
    check_fill_solid(engine->fill_solid, "fill_solid%d", tile_size);
    check_fill_halfplane(engine->fill_halfplane, "fill_halfplane%d", tile_size);
    check_fill_generic(engine->fill_generic, "fill_generic%d", tile_size);
}
