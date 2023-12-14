/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2022 Filipe Coelho <falktx@falktx.com>
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

#include "WindowPrivateData.hpp"
#include "../TopLevelWidget.hpp"

#include "pugl.hpp"


// -----------------------------------------------------------------------
// ScopedGraphicsContext

Window::ScopedGraphicsContext::ScopedGraphicsContext(Window& win)
    : window(win),
      ppData(nullptr),
      active(window.pData->view != nullptr && puglBackendEnter(window.pData->view)),
      reenter(false) {}

Window::ScopedGraphicsContext::ScopedGraphicsContext(Window& win, Window& transientWin)
    : window(win),
      ppData(transientWin.pData),
      active(false),
      reenter(window.pData->view != nullptr)
{
    if (reenter)
    {
        puglBackendLeave(ppData->view);
        active = puglBackendEnter(window.pData->view);
    }
}

Window::ScopedGraphicsContext::~ScopedGraphicsContext()
{
    done();
}

void Window::ScopedGraphicsContext::done()
{
    if (active)
    {
        puglBackendLeave(window.pData->view);
        active = false;
    }

    if (reenter)
    {
        reenter = false;
        DISTRHO_SAFE_ASSERT_RETURN(ppData != nullptr,);

        puglBackendEnter(ppData->view);
    }
}

void Window::ScopedGraphicsContext::reinit()
{
    DISTRHO_SAFE_ASSERT_RETURN(!active,);
    DISTRHO_SAFE_ASSERT_RETURN(!reenter,);
    DISTRHO_SAFE_ASSERT_RETURN(ppData != nullptr,);

    reenter = true;
    puglBackendLeave(ppData->view);
    active = puglBackendEnter(window.pData->view);
}

// -----------------------------------------------------------------------
// Window

Window::Window(Application& app)
    : pData(new PrivateData(app, this))
{
    pData->initPost();
}

Window::Window(Application& app, Window& transientParentWindow)
    : pData(new PrivateData(app, this, transientParentWindow.pData))
{
    pData->initPost();
}

Window::Window(Application& app,
               const uintptr_t parentWindowHandle,
               const double scaleFactor,
               const bool resizable)
    : pData(new PrivateData(app, this, parentWindowHandle, scaleFactor, resizable))
{
    pData->initPost();
}

Window::Window(Application& app,
               const uintptr_t parentWindowHandle,
               const uint32_t width,
               const uint32_t height,
               const double scaleFactor,
               const bool resizable)
    : pData(new PrivateData(app, this, parentWindowHandle, width, height, scaleFactor, resizable, false))
{
    pData->initPost();
}

Window::Window(Application& app,
               const uintptr_t parentWindowHandle,
               const uint32_t width,
               const uint32_t height,
               const double scaleFactor,
               const bool resizable,
               const bool isVST3,
               const bool doPostInit)
    : pData(new PrivateData(app, this, parentWindowHandle, width, height, scaleFactor, resizable, isVST3))
{
    if (doPostInit)
        pData->initPost();
}

Window::~Window()
{
    delete pData;
}

bool Window::isEmbed() const noexcept
{
    return pData->isEmbed;
}

bool Window::isVisible() const noexcept
{
    return pData->isVisible;
}

void Window::setVisible(const bool visible)
{
    if (visible)
        pData->show();
    else
        pData->hide();
}

void Window::show()
{
    pData->show();
}

void Window::hide()
{
    pData->hide();
}

void Window::close()
{
    pData->close();
}

bool Window::isResizable() const noexcept
{
    return pData->view != nullptr
        && puglGetViewHint(pData->view, PUGL_RESIZABLE) == PUGL_TRUE;
}

void Window::setResizable(const bool resizable)
{
    pData->setResizable(resizable);
}

int Window::getOffsetX() const noexcept
{
    DISTRHO_SAFE_ASSERT_RETURN(pData->view != nullptr, 0);

    return puglGetFrame(pData->view).x;
}

int Window::getOffsetY() const noexcept
{
    DISTRHO_SAFE_ASSERT_RETURN(pData->view != nullptr, 0);

    return puglGetFrame(pData->view).y;
}

Point<int> Window::getOffset() const noexcept
{
    DISTRHO_SAFE_ASSERT_RETURN(pData->view != nullptr, Point<int>());

    const PuglRect rect = puglGetFrame(pData->view);
    return Point<int>(rect.x, rect.y);
}

void Window::setOffsetX(const int x)
{
    setOffset(x, getOffsetY());
}

void Window::setOffsetY(const int y)
{
    setOffset(getOffsetX(), y);
}

void Window::setOffset(const int x, const int y)
{
    if (pData->view != nullptr)
        puglSetPosition(pData->view, x, y);
}

void Window::setOffset(const Point<int>& offset)
{
    setOffset(offset.getX(), offset.getY());
}

uint32_t Window::getWidth() const noexcept
{
    DISTRHO_SAFE_ASSERT_RETURN(pData->view != nullptr, 0);

    const double width = puglGetFrame(pData->view).width;
    DISTRHO_SAFE_ASSERT_RETURN(width >= 0.0, 0);
    return static_cast<uint32_t>(width + 0.5);
}

uint32_t Window::getHeight() const noexcept
{
    DISTRHO_SAFE_ASSERT_RETURN(pData->view != nullptr, 0);

    const double height = puglGetFrame(pData->view).height;
    DISTRHO_SAFE_ASSERT_RETURN(height >= 0.0, 0);
    return static_cast<uint32_t>(height + 0.5);
}

Size<uint32_t> Window::getSize() const noexcept
{
    DISTRHO_SAFE_ASSERT_RETURN(pData->view != nullptr, Size<uint32_t>());

    const PuglRect rect = puglGetFrame(pData->view);
    DISTRHO_SAFE_ASSERT_RETURN(rect.width >= 0.0, Size<uint32_t>());
    DISTRHO_SAFE_ASSERT_RETURN(rect.height >= 0.0, Size<uint32_t>());
    return Size<uint32_t>(static_cast<uint32_t>(rect.width + 0.5),
                      static_cast<uint32_t>(rect.height + 0.5));
}

void Window::setWidth(const uint32_t width)
{
    setSize(width, getHeight());
}

void Window::setHeight(const uint32_t height)
{
    setSize(getWidth(), height);
}

void Window::setSize(uint32_t width, uint32_t height)
{
    DISTRHO_SAFE_ASSERT_UINT2_RETURN(width > 1 && height > 1, width, height,);

    if (pData->isEmbed)
    {
        const double scaleFactor = pData->scaleFactor;
        uint32_t minWidth = pData->minWidth;
        uint32_t minHeight = pData->minHeight;

        if (pData->autoScaling && scaleFactor != 1.0)
        {
            minWidth *= scaleFactor;
            minHeight *= scaleFactor;
        }

        // handle geometry constraints here
        if (width < minWidth)
            width = minWidth;

        if (height < minHeight)
            height = minHeight;

        if (pData->keepAspectRatio)
        {
            const double ratio = static_cast<double>(pData->minWidth)
                               / static_cast<double>(pData->minHeight);
            const double reqRatio = static_cast<double>(width)
                                  / static_cast<double>(height);

            if (d_isNotEqual(ratio, reqRatio))
            {
                // fix width
                if (reqRatio > ratio)
                    width = static_cast<uint32_t>(height * ratio + 0.5);
                // fix height
                else
                    height = static_cast<uint32_t>(static_cast<double>(width) / ratio + 0.5);
            }
        }
    }

    if (pData->usesSizeRequest)
    {
        DISTRHO_SAFE_ASSERT_RETURN(pData->topLevelWidgets.size() != 0,);

        TopLevelWidget* const topLevelWidget = pData->topLevelWidgets.front();
        DISTRHO_SAFE_ASSERT_RETURN(topLevelWidget != nullptr,);

        topLevelWidget->requestSizeChange(width, height);
    }
    else if (pData->view != nullptr)
    {
        puglSetSizeAndDefault(pData->view, width, height);
    }
}

void Window::setSize(const Size<uint32_t>& size)
{
    setSize(size.getWidth(), size.getHeight());
}

const char* Window::getTitle() const noexcept
{
    return pData->view != nullptr ? puglGetWindowTitle(pData->view) : "";
}

void Window::setTitle(const char* const title)
{
    if (pData->view != nullptr)
        puglSetWindowTitle(pData->view, title);
}

bool Window::isIgnoringKeyRepeat() const noexcept
{
    return pData->view != nullptr
        && puglGetViewHint(pData->view, PUGL_IGNORE_KEY_REPEAT) == PUGL_TRUE;
}

void Window::setIgnoringKeyRepeat(const bool ignore) noexcept
{
    if (pData->view != nullptr)
        puglSetViewHint(pData->view, PUGL_IGNORE_KEY_REPEAT, ignore);
}

const void* Window::getClipboard(size_t& dataSize)
{
    return pData->getClipboard(dataSize);
}

bool Window::setClipboard(const char* const mimeType, const void* const data, const size_t dataSize)
{
    return pData->view != nullptr
        && puglSetClipboard(pData->view, mimeType != nullptr ? mimeType : "text/plain", data, dataSize) == PUGL_SUCCESS;
}

bool Window::setCursor(const MouseCursor cursor)
{
    return pData->view != nullptr
        && puglSetCursor(pData->view, static_cast<PuglCursor>(cursor)) == PUGL_SUCCESS;
}

bool Window::addIdleCallback(IdleCallback* const callback, const uint32_t timerFrequencyInMs)
{
    DISTRHO_SAFE_ASSERT_RETURN(callback != nullptr, false)

    return pData->addIdleCallback(callback, timerFrequencyInMs);
}

bool Window::removeIdleCallback(IdleCallback* const callback)
{
    DISTRHO_SAFE_ASSERT_RETURN(callback != nullptr, false)

    return pData->removeIdleCallback(callback);
}

Application& Window::getApp() const noexcept
{
    return pData->app;
}

#ifndef DPF_TEST_WINDOW_CPP
const GraphicsContext& Window::getGraphicsContext() const noexcept
{
    return pData->getGraphicsContext();
}
#endif

uintptr_t Window::getNativeWindowHandle() const noexcept
{
    return pData->view != nullptr ? puglGetNativeView(pData->view) : 0;
}

double Window::getScaleFactor() const noexcept
{
    return pData->scaleFactor;
}

void Window::focus()
{
    pData->focus();
}

#ifndef DGL_FILE_BROWSER_DISABLED
bool Window::openFileBrowser(const FileBrowserOptions& options)
{
    return pData->openFileBrowser(options);
}
#endif

void Window::repaint() noexcept
{
    if (pData->view != nullptr)
        puglPostRedisplay(pData->view);
}

void Window::repaint(const Rectangle<uint32_t>& rect) noexcept
{
    if (pData->view == nullptr)
        return;

    PuglRect prect = {
        static_cast<PuglCoord>(rect.getX()),
        static_cast<PuglCoord>(rect.getY()),
        static_cast<PuglSpan>(rect.getWidth()),
        static_cast<PuglSpan>(rect.getHeight()),
    };
    if (pData->autoScaling)
    {
        const double autoScaleFactor = pData->autoScaleFactor;

        prect.x *= autoScaleFactor;
        prect.y *= autoScaleFactor;
        prect.width *= autoScaleFactor;
        prect.height *= autoScaleFactor;
    }
    puglPostRedisplayRect(pData->view, prect);
}

void Window::renderToPicture(const char* const filename)
{
    pData->filenameToRenderInto = strdup(filename);
}

void Window::runAsModal(bool blockWait)
{
    pData->runAsModal(blockWait);
}

Size<uint32_t> Window::getGeometryConstraints(bool& keepAspectRatio)
{
    keepAspectRatio = pData->keepAspectRatio;
    return Size<uint32_t>(pData->minWidth, pData->minHeight);
}

void Window::setGeometryConstraints(uint32_t minimumWidth,
                                    uint32_t minimumHeight,
                                    const bool keepAspectRatio,
                                    const bool automaticallyScale,
                                    const bool resizeNowIfAutoScaling)
{
    DISTRHO_SAFE_ASSERT_RETURN(minimumWidth > 0,);
    DISTRHO_SAFE_ASSERT_RETURN(minimumHeight > 0,);

    pData->minWidth = minimumWidth;
    pData->minHeight = minimumHeight;
    pData->autoScaling = automaticallyScale;
    pData->keepAspectRatio = keepAspectRatio;

    if (pData->view == nullptr)
        return;

    const double scaleFactor = pData->scaleFactor;

    if (automaticallyScale && scaleFactor != 1.0)
    {
        minimumWidth *= scaleFactor;
        minimumHeight *= scaleFactor;
    }

    puglSetGeometryConstraints(pData->view, minimumWidth, minimumHeight, keepAspectRatio);

    if (scaleFactor != 1.0 && automaticallyScale && resizeNowIfAutoScaling)
    {
        const Size<uint32_t> size(getSize());

        setSize(static_cast<uint32_t>(size.getWidth() * scaleFactor + 0.5),
                static_cast<uint32_t>(size.getHeight() * scaleFactor + 0.5));
    }
}

void Window::setTransientParent(const uintptr_t transientParentWindowHandle)
{
    if (pData->view != nullptr)
        puglSetTransientParent(pData->view, transientParentWindowHandle);
}

std::vector<ClipboardDataOffer> Window::getClipboardDataOfferTypes()
{
    std::vector<ClipboardDataOffer> offerTypes;

    if (pData->view != nullptr)
        return offerTypes;

    if (const uint32_t numTypes = puglGetNumClipboardTypes(pData->view))
    {
        offerTypes.reserve(numTypes);

        for (uint32_t i=0; i<numTypes; ++i)
        {
            const ClipboardDataOffer offer = { i + 1, puglGetClipboardType(pData->view, i) };
            offerTypes.push_back(offer);
        }
    }

    return offerTypes;
}

uint32_t Window::onClipboardDataOffer()
{
    std::vector<ClipboardDataOffer> offers(getClipboardDataOfferTypes());

    for (std::vector<ClipboardDataOffer>::iterator it=offers.begin(), end=offers.end(); it != end;++it)
    {
        const ClipboardDataOffer offer = *it;
        if (std::strcmp(offer.type, "text/plain") == 0)
            return offer.id;
    }

    return 0;
}

bool Window::onClose()
{
    return true;
}

void Window::onFocus(bool, CrossingMode)
{
}

void Window::onReshape(uint32_t, uint32_t)
{
    if (pData->view != nullptr)
        puglFallbackOnResize(pData->view);
}

void Window::onScaleFactorChanged(double)
{
}

#ifndef DGL_FILE_BROWSER_DISABLED
void Window::onFileSelected(const char*)
{
}
#endif

// -----------------------------------------------------------------------

