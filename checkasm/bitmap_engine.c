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

#include "config.h"
#include "ass_compat.h"

#include "ass_bitmap_engine.h"


#define DECORATE(func)        ass_##func##_c

#define ALIGN                 4

#define LARGE_TILES           0
#include "ass_func_template.h"
#undef LARGE_TILES

#define LARGE_TILES           1
#define DECORATE_NAME(func)   ass_##func##_cl
#include "ass_func_template.h"
#undef DECORATE_NAME
#undef LARGE_TILES

#undef ALIGN

#define ALIGN                 5
#define DECORATE_ALIGN(func)  ass_##func##_c32

#define LARGE_TILES           0
#include "ass_func_template.h"
#undef LARGE_TILES

#define LARGE_TILES           1
#define DECORATE_NAME(func)   ass_##func##_c32l
#include "ass_func_template.h"
#undef DECORATE_NAME
#undef LARGE_TILES

#undef DECORATE_ALIGN
#undef ALIGN

#undef DECORATE


#if (defined(__i386__) || defined(__x86_64__)) && CONFIG_ASM

#define DECORATE(func)        ass_##func##_sse2
#define ALIGN                 4

#define LARGE_TILES           0
#include "ass_func_template.h"
#undef LARGE_TILES

#define LARGE_TILES           1
#define DECORATE_NAME(func)   ass_##func##_sse2l
#include "ass_func_template.h"
#undef DECORATE_NAME
#undef LARGE_TILES

#undef ALIGN
#undef DECORATE

#define DECORATE(func)        ass_##func##_avx2
#define ALIGN                 5

#define LARGE_TILES           0
#include "ass_func_template.h"
#undef LARGE_TILES

#define LARGE_TILES           1
#define DECORATE_NAME(func)   ass_##func##_avx2l
#include "ass_func_template.h"
#undef DECORATE_NAME
#undef LARGE_TILES

#undef ALIGN
#undef DECORATE

#endif


static const BitmapEngine *engines_c[] = {
    &ass_bitmap_engine_c,
    &ass_bitmap_engine_cl,
    &ass_bitmap_engine_c32,
    &ass_bitmap_engine_c32l,
    NULL
};

#if (defined(__i386__) || defined(__x86_64__)) && CONFIG_ASM

static const BitmapEngine *engines_sse2[] = {
    &ass_bitmap_engine_sse2,
    &ass_bitmap_engine_sse2l,
    NULL
};

static const BitmapEngine *engines_avx2[] = {
    &ass_bitmap_engine_avx2,
    &ass_bitmap_engine_avx2l,
    NULL
};

#endif


const BitmapEngine **bitmap_engine_init(ASS_CPUFlags mask)
{
#if CONFIG_ASM
    ASS_CPUFlags flags = ass_get_cpu_flags(mask);
#if (defined(__i386__) || defined(__x86_64__))
    if (flags & ASS_CPU_FLAG_X86_AVX2)
        return engines_avx2;
    if (flags & ASS_CPU_FLAG_X86_SSE2)
        return engines_sse2;
#endif
#endif
    return engines_c;
}
