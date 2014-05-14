/*
 * Copyright (C) 2014 Vabishchevich Nikolay <vabnick@gmail.com>
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

#ifndef LIBASS_RASTERIZER_H
#define LIBASS_RASTERIZER_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include "ass_tile.h"


enum {
    SEGFLAG_DN = 1,
    SEGFLAG_UL_DR = 2,
    SEGFLAG_EXACT_LEFT = 4,
    SEGFLAG_EXACT_RIGHT = 8,
    SEGFLAG_EXACT_TOP = 16,
    SEGFLAG_EXACT_BOTTOM = 32,
};

// Polyline segment struct
struct segment {
    int64_t c;
    int32_t a, b, scale, flags;
    int32_t x_min, x_max, y_min, y_max;
};

typedef struct {
    int outline_error;  // acceptable error (in 1/64 pixel units)

    // usable after rasterizer_set_outline
    int32_t x_min, x_max, y_min, y_max;

    // internal buffers
    struct segment *linebuf[2];
    size_t size[2], capacity[2];
} RasterizerData;

void rasterizer_init(RasterizerData *rst, int outline_error);
void rasterizer_done(RasterizerData *rst);
/**
 * \brief Convert FreeType outline to polyline and calculate exact bounds
 */
int rasterizer_set_outline(RasterizerData *rst, const FT_Outline *path);
/**
 * \brief Polyline rasterization function
 * Deletes preprocessed polyline after work.
 */
TileTree *rasterizer_fill(const TileEngine *engine, RasterizerData *rst);


#endif                          /* LIBASS_RASTERIZER_H */
