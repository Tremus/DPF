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

#ifndef DGL_STANDALONE_WINDOW_HPP_INCLUDED
#define DGL_STANDALONE_WINDOW_HPP_INCLUDED

#include "TopLevelWidget.hpp"
#include "Window.hpp"

START_NAMESPACE_DGL

// -----------------------------------------------------------------------

class StandaloneWindow : public Window,
                         public TopLevelWidget
{
public:
   /**
      Constructor.
    */
    StandaloneWindow(Application& app)
      : Window(app),
        TopLevelWidget((Window&)*this) {}

   /**
      Overloaded functions to ensure they apply to the Window class.
    */
    bool isVisible() const noexcept { return Window::isVisible(); }
    void setVisible(bool yesNo) { Window::setVisible(yesNo); }
    void hide() { Window::hide(); }
    void show() { Window::show(); }
    uint getWidth() const noexcept { return Window::getWidth(); }
    uint getHeight() const noexcept { return Window::getHeight(); }
    const Size<uint> getSize() const noexcept { return Window::getSize(); }

   /**
      Overloaded functions to ensure size changes apply on both TopLevelWidget and Window classes.
    */
    void setWidth(uint width)
    {
        TopLevelWidget::setWidth(width);
        Window::setWidth(width);
    }

    void setHeight(uint height)
    {
        TopLevelWidget::setHeight(height);
        Window::setHeight(height);
    }

    void setSize(uint width, uint height)
    {
        TopLevelWidget::setSize(width, height);
        Window::setSize(width, height);
    }

    void setSize(const Size<uint>& size)
    {
        TopLevelWidget::setSize(size);
        Window::setSize(size);
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandaloneWindow)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DGL

#endif // DGL_STANDALONE_WINDOW_HPP_INCLUDED
