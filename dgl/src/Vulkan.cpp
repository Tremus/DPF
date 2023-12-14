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

#include "../Vulkan.hpp"
#include "../Color.hpp"

#include "SubWidgetPrivateData.hpp"
#include "TopLevelWidgetPrivateData.hpp"
#include "WidgetPrivateData.hpp"
#include "WindowPrivateData.hpp"


// -----------------------------------------------------------------------

static void notImplemented(const char* const name)
{
    d_stderr2("vulkan function not implemented: %s", name);
}

// -----------------------------------------------------------------------
// Color

void Color::setFor(const GraphicsContext&, bool)
{
    notImplemented("Color::setFor");
}

// -----------------------------------------------------------------------
// Line

template<typename T>
void Line<T>::draw(const GraphicsContext&, T)
{
    notImplemented("Line::draw");
}

template<typename T>
void Line<T>::draw()
{
    notImplemented("Line::draw");
}

template class Line<double>;
template class Line<float>;
template class Line<int>;
template class Line<uint32_t>;
template class Line<short>;
template class Line<uint16_t>;

// -----------------------------------------------------------------------
// Circle

template<typename T>
void Circle<T>::draw(const GraphicsContext&)
{
    notImplemented("Circle::draw");
}

template<typename T>
void Circle<T>::drawOutline(const GraphicsContext&, T)
{
    notImplemented("Circle::drawOutline");
}

template<typename T>
void Circle<T>::draw()
{
    notImplemented("Circle::draw");
}

template<typename T>
void Circle<T>::drawOutline()
{
    notImplemented("Circle::drawOutline");
}

template class Circle<double>;
template class Circle<float>;
template class Circle<int>;
template class Circle<uint32_t>;
template class Circle<short>;
template class Circle<uint16_t>;

// -----------------------------------------------------------------------
// Triangle

template<typename T>
void Triangle<T>::draw(const GraphicsContext&)
{
    notImplemented("Triangle::draw");
}

template<typename T>
void Triangle<T>::drawOutline(const GraphicsContext&, T)
{
    notImplemented("Triangle::drawOutline");
}

template<typename T>
void Triangle<T>::draw()
{
    notImplemented("Triangle::draw");
}

template<typename T>
void Triangle<T>::drawOutline()
{
    notImplemented("Triangle::drawOutline");
}

template class Triangle<double>;
template class Triangle<float>;
template class Triangle<int>;
template class Triangle<uint32_t>;
template class Triangle<short>;
template class Triangle<uint16_t>;


// -----------------------------------------------------------------------
// Rectangle

template<typename T>
void Rectangle<T>::draw(const GraphicsContext&)
{
    notImplemented("Rectangle::draw");
}

template<typename T>
void Rectangle<T>::drawOutline(const GraphicsContext&, T)
{
    notImplemented("Rectangle::drawOutline");
}

template<typename T>
void Rectangle<T>::draw()
{
    notImplemented("Rectangle::draw");
}

template<typename T>
void Rectangle<T>::drawOutline()
{
    notImplemented("Rectangle::drawOutline");
}

template class Rectangle<double>;
template class Rectangle<float>;
template class Rectangle<int>;
template class Rectangle<uint32_t>;
template class Rectangle<short>;
template class Rectangle<uint16_t>;

// -----------------------------------------------------------------------
// VulkanImage

VulkanImage::VulkanImage()
    : ImageBase() {}

VulkanImage::VulkanImage(const char* const rdata, const uint32_t w, const uint32_t h, const ImageFormat fmt)
    : ImageBase(rdata, w, h, fmt) {}

VulkanImage::VulkanImage(const char* const rdata, const Size<uint32_t>& s, const ImageFormat fmt)
    : ImageBase(rdata, s, fmt) {}

VulkanImage::VulkanImage(const VulkanImage& image)
    : ImageBase(image.rawData, image.size, image.format) {}

VulkanImage::~VulkanImage() {}

void VulkanImage::loadFromMemory(const char* const rdata, const Size<uint32_t>& s, const ImageFormat fmt) noexcept
{
    ImageBase::loadFromMemory(rdata, s, fmt);
}

void VulkanImage::drawAt(const GraphicsContext&, const Point<int>&)
{
}

VulkanImage& VulkanImage::operator=(const VulkanImage& image) noexcept
{
    rawData = image.rawData;
    size    = image.size;
    format  = image.format;
    return *this;
}

// -----------------------------------------------------------------------

void SubWidget::PrivateData::display(const uint32_t width, const uint32_t height, const double autoScaleFactor)
{
    // TODO

    selfw->pData->displaySubWidgets(width, height, autoScaleFactor);
}

// -----------------------------------------------------------------------

void TopLevelWidget::PrivateData::display()
{
    if (! selfw->pData->visible)
        return;

    const Size<uint32_t> size(window.getSize());
    const uint32_t width  = size.getWidth();
    const uint32_t height = size.getHeight();

    const double autoScaleFactor = window.pData->autoScaleFactor;

    // TODO

    // main widget drawing
    self->onDisplay();

    // now draw subwidgets if there are any
    selfw->pData->displaySubWidgets(width, height, autoScaleFactor);
}

// -----------------------------------------------------------------------

void Window::PrivateData::renderToPicture(const char*, const GraphicsContext&, uint32_t, uint32_t)
{
    notImplemented("Window::PrivateData::renderToPicture");
}

// -----------------------------------------------------------------------

const GraphicsContext& Window::PrivateData::getGraphicsContext() const noexcept
{
    return (const GraphicsContext&)graphicsContext;
}

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------
