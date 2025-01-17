/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2021 Filipe Coelho <falktx@falktx.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef DGL_LAYOUT_HPP_INCLUDED
#define DGL_LAYOUT_HPP_INCLUDED

#include "Geometry.hpp"

#include <list>


class SubWidget;

// --------------------------------------------------------------------------------------------------------------------

// NOTE: under development, API to be finalized and documented soon

enum SizeHint {
    Expanding,
    Fixed
};

struct SubWidgetWithSizeHint {
    SubWidget* widget;
    SizeHint sizeHint;
};

template<bool horizontal>
struct Layout
{
    std::list<SubWidgetWithSizeHint> widgets;
    uint32_t setAbsolutePos(int x, int y, uint32_t padding);
    void setSize(uint32_t size, uint32_t padding);
};

typedef Layout<true> HorizontalLayout;
typedef Layout<false> VerticalLayout;

struct HorizontallyStackedVerticalLayout
{
    std::list<VerticalLayout*> items;
    Size<uint32_t> adjustSize(uint32_t padding); // TODO
    void setAbsolutePos(int x, int y, uint32_t padding);
};

struct VerticallyStackedHorizontalLayout
{
    std::list<HorizontalLayout*> items;
    Size<uint32_t> adjustSize(uint32_t padding);
    void setAbsolutePos(int x, int y, uint32_t padding);
};

// --------------------------------------------------------------------------------------------------------------------


#endif // DGL_LAYOUT_HPP_INCLUDED
