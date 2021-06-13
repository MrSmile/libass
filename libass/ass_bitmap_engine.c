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

#include "config.h"
#include "ass_compat.h"

#include "ass_bitmap_engine.h"

#if CONFIG_LARGE_TILES
#define LARGE_TILES 1
#else
#define LARGE_TILES 0
#endif

#define ALIGN           4
#define DECORATE(func)  ass_##func##_c
#include "ass_func_template.h"
#undef ALIGN
#undef DECORATE

#if (defined(__i386__) || defined(__x86_64__)) && CONFIG_ASM

#define ALIGN           4
#define DECORATE(func)  ass_##func##_sse2
#include "ass_func_template.h"
#undef ALIGN
#undef DECORATE

#define ALIGN           5
#define DECORATE(func)  ass_##func##_avx2
#include "ass_func_template.h"
#undef ALIGN
#undef DECORATE

#endif


const BitmapEngine *ass_bitmap_engine_init(ASS_CPUFlags mask)
{
#if CONFIG_ASM
    ASS_CPUFlags flags = ass_get_cpu_flags(mask);
#if (defined(__i386__) || defined(__x86_64__))
    if (flags & ASS_CPU_FLAG_X86_AVX2)
        return &ass_bitmap_engine_avx2;
    if (flags & ASS_CPU_FLAG_X86_SSE2)
        return &ass_bitmap_engine_sse2;
#endif
#endif
    return &ass_bitmap_engine_c;
}
