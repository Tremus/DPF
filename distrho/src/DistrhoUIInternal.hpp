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

#ifndef DISTRHO_UI_INTERNAL_HPP_INCLUDED
#define DISTRHO_UI_INTERNAL_HPP_INCLUDED

#include "DistrhoUIPrivateData.hpp"


// -----------------------------------------------------------------------
// Static data, see DistrhoUI.cpp

extern const char* g_nextBundlePath;
#if DISTRHO_PLUGIN_HAS_EXTERNAL_UI
extern uintptr_t   g_nextWindowId;
extern double      g_nextScaleFactor;
#endif

// -----------------------------------------------------------------------
// UI exporter class

class UIExporter
{
    // -------------------------------------------------------------------
    // UI Widget and its private data

    UI* ui;
    UI::PrivateData* uiData;

    // -------------------------------------------------------------------

public:
    UIExporter(void* const callbacksPtr,
               const uintptr_t winId,
               const double sampleRate,
               const editParamFunc editParamCall,
               const setParamFunc setParamCall,
               const sendNoteFunc sendNoteCall,
               const setSizeFunc setSizeCall,
               const fileRequestFunc fileRequestCall,
               const char* const bundlePath = nullptr,
               void* const dspPtr = nullptr,
               const double scaleFactor = 0.0,
               const uint32_t bgColor = 0,
               const uint32_t fgColor = 0xffffffff,
               const char* const appClassName = nullptr)
        : ui(nullptr),
          uiData(new UI::PrivateData(appClassName))
    {
        uiData->sampleRate = sampleRate;
        uiData->bundlePath = bundlePath != nullptr ? strdup(bundlePath) : nullptr;
        uiData->dspPtr = dspPtr;

        uiData->bgColor = bgColor;
        uiData->fgColor = fgColor;
        uiData->scaleFactor = scaleFactor;
        uiData->winId = winId;

        uiData->callbacksPtr            = callbacksPtr;
        uiData->editParamCallbackFunc   = editParamCall;
        uiData->setParamCallbackFunc    = setParamCall;
        uiData->sendNoteCallbackFunc    = sendNoteCall;
        uiData->setSizeCallbackFunc     = setSizeCall;
        uiData->fileRequestCallbackFunc = fileRequestCall;

        g_nextBundlePath  = bundlePath;
#if DISTRHO_PLUGIN_HAS_EXTERNAL_UI
        g_nextWindowId    = winId;
        g_nextScaleFactor = scaleFactor;
#endif
        UI::PrivateData::s_nextPrivateData = uiData;

        UI* const uiPtr = createUI();

        g_nextBundlePath  = nullptr;
#if DISTRHO_PLUGIN_HAS_EXTERNAL_UI
        g_nextWindowId    = 0;
        g_nextScaleFactor = 0.0;
#else
        // enter context called in the PluginWindow constructor, see DistrhoUIPrivateData.hpp
        uiData->window->leaveContext();
#endif
        UI::PrivateData::s_nextPrivateData = nullptr;

        DISTRHO_SAFE_ASSERT_RETURN(uiPtr != nullptr,);
        ui = uiPtr;
        uiData->initializing = false;

#if !DISTRHO_PLUGIN_HAS_EXTERNAL_UI
        // unused
        (void)bundlePath;
#endif
    }

    ~UIExporter()
    {
        quit();
#if !DISTRHO_PLUGIN_HAS_EXTERNAL_UI
        uiData->window->enterContextForDeletion();
#endif
        delete ui;
        delete uiData;
    }

    // -------------------------------------------------------------------

    uint32_t getWidth() const noexcept
    {
        return uiData->window->getWidth();
    }

    uint32_t getHeight() const noexcept
    {
        return uiData->window->getHeight();
    }

    double getScaleFactor() const noexcept
    {
        return uiData->window->getScaleFactor();
    }

    bool getGeometryConstraints(uint32_t& minimumWidth, uint32_t& minimumHeight, bool& keepAspectRatio) const noexcept
    {
#if DISTRHO_PLUGIN_HAS_EXTERNAL_UI
        uiData->window->getGeometryConstraints(minimumWidth, minimumHeight, keepAspectRatio);
#else
        const Size<uint32_t> size(uiData->window->getGeometryConstraints(keepAspectRatio));
        minimumWidth = size.getWidth();
        minimumHeight = size.getHeight();
#endif
        return true;
    }

    bool isResizable() const noexcept
    {
        return uiData->window->isResizable();
    }

    bool isVisible() const noexcept
    {
        return uiData->window->isVisible();
    }

    uintptr_t getNativeWindowHandle() const noexcept
    {
        return uiData->window->getNativeWindowHandle();
    }

    uint32_t getBackgroundColor() const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(uiData != nullptr, 0);

        return uiData->bgColor;
    }

    uint32_t getForegroundColor() const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(uiData != nullptr, 0xffffffff);

        return uiData->fgColor;
    }

    // -------------------------------------------------------------------

    uint32_t getParameterOffset() const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(uiData != nullptr, 0);

        return uiData->parameterOffset;
    }

    // -------------------------------------------------------------------

    void parameterChanged(const uint32_t index, const float value)
    {
        DISTRHO_SAFE_ASSERT_RETURN(ui != nullptr,);

        ui->parameterChanged(index, value);
    }

    // -------------------------------------------------------------------

   #if DISTRHO_UI_IS_STANDALONE
    void exec(IdleCallback* const cb)
    {
        DISTRHO_SAFE_ASSERT_RETURN(cb != nullptr,);

        uiData->window->show();
        uiData->window->focus();
        uiData->app.addIdleCallback(cb);
        uiData->app.exec();
    }

    void exec_idle()
    {
        DISTRHO_SAFE_ASSERT_RETURN(ui != nullptr, );

        ui->uiIdle();
    }

    void showAndFocus()
    {
        uiData->window->show();
        uiData->window->focus();
    }
   #endif

    bool plugin_idle()
    {
        DISTRHO_SAFE_ASSERT_RETURN(ui != nullptr, false);

        uiData->app.idle();
        ui->uiIdle();
        return ! uiData->app.isQuitting();
    }

    void focus()
    {
        uiData->window->focus();
    }

    void quit()
    {
        uiData->window->close();
        uiData->app.quit();
    }

   #if !DISTRHO_PLUGIN_HAS_EXTERNAL_UI
    void repaint()
    {
        uiData->window->repaint();
    }
   #endif

    // -------------------------------------------------------------------

  #if defined(DISTRHO_OS_MAC) || defined(DISTRHO_OS_WINDOWS)
    void idleFromNativeIdle()
    {
        DISTRHO_SAFE_ASSERT_RETURN(ui != nullptr,);

        uiData->app.triggerIdleCallbacks();
        ui->uiIdle();
    }

   #if !DISTRHO_PLUGIN_HAS_EXTERNAL_UI
    void addIdleCallbackForNativeIdle(IdleCallback* const cb, const uint32_t timerFrequencyInMs)
    {
        uiData->window->addIdleCallback(cb, timerFrequencyInMs);
    }

    void removeIdleCallbackForNativeIdle(IdleCallback* const cb)
    {
        uiData->window->removeIdleCallback(cb);
    }
   #endif
  #endif

    // -------------------------------------------------------------------

    void setWindowOffset(const int x, const int y)
    {
       #if DISTRHO_PLUGIN_HAS_EXTERNAL_UI
        // TODO
        (void)x; (void)y;
       #else
        uiData->window->setOffset(x, y);
       #endif
    }

   #if defined(DISTRHO_PLUGIN_TARGET_VST3) || defined(DISTRHO_PLUGIN_TARGET_CLAP)
    void setWindowSizeFromHost(const uint32_t width, const uint32_t height)
    {
       #if DISTRHO_PLUGIN_HAS_EXTERNAL_UI
        ui->setSize(width, height);
       #else
        uiData->window->setSizeFromHost(width, height);
       #endif
    }
   #endif

    void setWindowTitle(const char* const uiTitle)
    {
        uiData->window->setTitle(uiTitle);
    }

    void setWindowTransientWinId(const uintptr_t transientParentWindowHandle)
    {
#if DISTRHO_PLUGIN_HAS_EXTERNAL_UI
        ui->setTransientWindowId(transientParentWindowHandle);
#else
        uiData->window->setTransientParent(transientParentWindowHandle);
#endif
    }

    bool setWindowVisible(const bool yesNo)
    {
        uiData->window->setVisible(yesNo);

        return ! uiData->app.isQuitting();
    }

#if !DISTRHO_PLUGIN_HAS_EXTERNAL_UI
    bool handlePluginKeyboardVST(const bool press, const bool special, const uint32_t keychar, const uint32_t keycode, const uint16_t mods)
    {
        Widget::KeyboardEvent ev;
        ev.mod     = mods;
        ev.press   = press;
        ev.key     = keychar;
        ev.keycode = keycode;

        // keyboard events must always be lowercase
        if (ev.key >= 'A' && ev.key <= 'Z')
            ev.key += 'a' - 'A'; // A-Z -> a-z

        const bool ret = ui->onKeyboard(ev);

        if (press && !special && (mods & (kModifierControl|kModifierAlt|kModifierSuper)) == 0)
        {
            Widget::CharacterInputEvent cev;
            cev.mod       = mods;
            cev.character = keychar;
            cev.keycode   = keycode;

            // if shift modifier is on, convert a-z -> A-Z for character input
            if (cev.character >= 'a' && cev.character <= 'z' && (mods & kModifierShift) != 0)
                cev.character -= 'a' - 'A';

            ui->onCharacterInput(cev);
        }

        return ret;
    }
#endif

    // -------------------------------------------------------------------

    void notifyScaleFactorChanged(const double scaleFactor)
    {
        DISTRHO_SAFE_ASSERT_RETURN(ui != nullptr,);

        ui->uiScaleFactorChanged(scaleFactor);
    }

#if !DISTRHO_PLUGIN_HAS_EXTERNAL_UI
    void notifyFocusChanged(const bool focus)
    {
        DISTRHO_SAFE_ASSERT_RETURN(ui != nullptr,);

        ui->uiFocus(focus, kCrossingNormal);
    }
#endif

    void setSampleRate(const double sampleRate, const bool doCallback = false)
    {
        DISTRHO_SAFE_ASSERT_RETURN(ui != nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(uiData != nullptr,);
        DISTRHO_SAFE_ASSERT(sampleRate > 0.0);

        if (d_isEqual(uiData->sampleRate, sampleRate))
            return;

        uiData->sampleRate = sampleRate;

        if (doCallback)
            ui->sampleRateChanged(sampleRate);
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UIExporter)
};

// -----------------------------------------------------------------------


#endif // DISTRHO_UI_INTERNAL_HPP_INCLUDED
