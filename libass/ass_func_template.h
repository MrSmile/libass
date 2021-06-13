/*
 * Copyright (C) 2015 Vabishchevich Nikolay <vabnick@gmail.com>
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

#ifdef DECORATE_ALIGN
#define DEC_A DECORATE_ALIGN
#else
#define DEC_A DECORATE
#endif

#ifdef DECORATE_NAME
#define DEC_N DECORATE_NAME
#else
#define DEC_N DEC_A
#endif

void DECORATE(fill_solid_tile16)(uint8_t *buf, ptrdiff_t stride, int set);
void DECORATE(fill_solid_tile32)(uint8_t *buf, ptrdiff_t stride, int set);
void DECORATE(fill_halfplane_tile16)(uint8_t *buf, ptrdiff_t stride,
                                     int32_t a, int32_t b, int64_t c, int32_t scale);
void DECORATE(fill_halfplane_tile32)(uint8_t *buf, ptrdiff_t stride,
                                     int32_t a, int32_t b, int64_t c, int32_t scale);
void DECORATE(fill_generic_tile16)(uint8_t *buf, ptrdiff_t stride,
                                   const struct segment *line, size_t n_lines,
                                   int winding);
void DECORATE(fill_generic_tile32)(uint8_t *buf, ptrdiff_t stride,
                                   const struct segment *line, size_t n_lines,
                                   int winding);

void DECORATE(add_bitmaps)(uint8_t *dst, intptr_t dst_stride,
                           uint8_t *src, intptr_t src_stride,
                           intptr_t width, intptr_t height);
void DECORATE(sub_bitmaps)(uint8_t *dst, intptr_t dst_stride,
                           uint8_t *src, intptr_t src_stride,
                           intptr_t width, intptr_t height);
void DECORATE(mul_bitmaps)(uint8_t *dst, intptr_t dst_stride,
                           uint8_t *src1, intptr_t src1_stride,
                           uint8_t *src2, intptr_t src2_stride,
                           intptr_t width, intptr_t height);

void DECORATE(be_blur)(uint8_t *buf, intptr_t stride,
                       intptr_t width, intptr_t height, uint16_t *tmp);

void DEC_A(stripe_unpack)(int16_t *dst, const uint8_t *src, ptrdiff_t src_stride,
                          uintptr_t width, uintptr_t height);
void DEC_A(stripe_pack)(uint8_t *dst, ptrdiff_t dst_stride, const int16_t *src,
                        uintptr_t width, uintptr_t height);
void DEC_A(shrink_horz)(int16_t *dst, const int16_t *src,
                        uintptr_t src_width, uintptr_t src_height);
void DEC_A(shrink_vert)(int16_t *dst, const int16_t *src,
                        uintptr_t src_width, uintptr_t src_height);
void DEC_A(expand_horz)(int16_t *dst, const int16_t *src,
                        uintptr_t src_width, uintptr_t src_height);
void DEC_A(expand_vert)(int16_t *dst, const int16_t *src,
                        uintptr_t src_width, uintptr_t src_height);
void DEC_A(blur4_horz)(int16_t *dst, const int16_t *src,
                       uintptr_t src_width, uintptr_t src_height,
                       const int16_t *param);
void DEC_A(blur4_vert)(int16_t *dst, const int16_t *src,
                       uintptr_t src_width, uintptr_t src_height,
                       const int16_t *param);
void DEC_A(blur5_horz)(int16_t *dst, const int16_t *src,
                       uintptr_t src_width, uintptr_t src_height,
                       const int16_t *param);
void DEC_A(blur5_vert)(int16_t *dst, const int16_t *src,
                       uintptr_t src_width, uintptr_t src_height,
                       const int16_t *param);
void DEC_A(blur6_horz)(int16_t *dst, const int16_t *src,
                       uintptr_t src_width, uintptr_t src_height,
                       const int16_t *param);
void DEC_A(blur6_vert)(int16_t *dst, const int16_t *src,
                       uintptr_t src_width, uintptr_t src_height,
                       const int16_t *param);
void DEC_A(blur7_horz)(int16_t *dst, const int16_t *src,
                       uintptr_t src_width, uintptr_t src_height,
                       const int16_t *param);
void DEC_A(blur7_vert)(int16_t *dst, const int16_t *src,
                       uintptr_t src_width, uintptr_t src_height,
                       const int16_t *param);
void DEC_A(blur8_horz)(int16_t *dst, const int16_t *src,
                       uintptr_t src_width, uintptr_t src_height,
                       const int16_t *param);
void DEC_A(blur8_vert)(int16_t *dst, const int16_t *src,
                       uintptr_t src_width, uintptr_t src_height,
                       const int16_t *param);

const BitmapEngine DEC_N(bitmap_engine) = {
    .align_order = ALIGN,

#if LARGE_TILES
    .tile_order = 5,
    .fill_solid = DECORATE(fill_solid_tile32),
    .fill_halfplane = DECORATE(fill_halfplane_tile32),
    .fill_generic = DECORATE(fill_generic_tile32),
#else
    .tile_order = 4,
    .fill_solid = DECORATE(fill_solid_tile16),
    .fill_halfplane = DECORATE(fill_halfplane_tile16),
    .fill_generic = DECORATE(fill_generic_tile16),
#endif

    .add_bitmaps = DECORATE(add_bitmaps),
    .sub_bitmaps = DECORATE(sub_bitmaps),
    .mul_bitmaps = DECORATE(mul_bitmaps),

    .be_blur = DECORATE(be_blur),

    .stripe_unpack = DEC_A(stripe_unpack),
    .stripe_pack = DEC_A(stripe_pack),
    .shrink_horz = DEC_A(shrink_horz),
    .shrink_vert = DEC_A(shrink_vert),
    .expand_horz = DEC_A(expand_horz),
    .expand_vert = DEC_A(expand_vert),
    .blur_horz = { DEC_A(blur4_horz), DEC_A(blur5_horz), DEC_A(blur6_horz), DEC_A(blur7_horz), DEC_A(blur8_horz) },
    .blur_vert = { DEC_A(blur4_vert), DEC_A(blur5_vert), DEC_A(blur6_vert), DEC_A(blur7_vert), DEC_A(blur8_vert) },
};

#undef DEC_A
#undef DEC_N
