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

#ifndef DGL_OPENGL_HPP_INCLUDED
#define DGL_OPENGL_HPP_INCLUDED

#include "ImageBase.hpp"
#include "ImageBaseWidgets.hpp"

#include "OpenGL-include.hpp"


// -----------------------------------------------------------------------

/**
   OpenGL Graphics context.
 */
struct OpenGLGraphicsContext : GraphicsContext
{
#ifdef DGL_USE_OPENGL3
#endif
};

// -----------------------------------------------------------------------

static inline
ImageFormat asDISTRHOImageFormat(const GLenum format)
{
    switch (format)
    {
#ifdef DGL_USE_OPENGL3
    case GL_RED:
#else
    case GL_LUMINANCE:
#endif
        return kImageFormatGrayscale;
    case GL_BGR:
        return kImageFormatBGR;
    case GL_BGRA:
        return kImageFormatBGRA;
    case GL_RGB:
        return kImageFormatRGB;
    case GL_RGBA:
        return kImageFormatRGBA;
    }

    return kImageFormatNull;
}

static inline
GLenum asOpenGLImageFormat(const ImageFormat format)
{
    switch (format)
    {
    case kImageFormatNull:
        break;
    case kImageFormatGrayscale:
#ifdef DGL_USE_OPENGL3
        return GL_RED;
#else
        return GL_LUMINANCE;
#endif
    case kImageFormatBGR:
        return GL_BGR;
    case kImageFormatBGRA:
        return GL_BGRA;
    case kImageFormatRGB:
        return GL_RGB;
    case kImageFormatRGBA:
        return GL_RGBA;
    }

    return 0x0;
}

// -----------------------------------------------------------------------

/**
   OpenGL Image class.

   This is an Image class that handles raw image data in pixels.
   You can init the image data on the contructor or later on by calling loadFromMemory().

   To generate raw data useful for this class see the utils/png2rgba.py script.
   Be careful when using a PNG without alpha channel, for those the format is 'GL_BGR'
   instead of the default 'GL_BGRA'.

   Images are drawn on screen via 2D textures.
 */
class OpenGLImage : public ImageBase
{
public:
   /**
      Constructor for a null Image.
    */
    OpenGLImage();

   /**
      Constructor using raw image data.
      @note @a rawData must remain valid for the lifetime of this Image.
    */
    OpenGLImage(const char* rawData, uint32_t width, uint32_t height, ImageFormat format = kImageFormatBGRA);

   /**
      Constructor using raw image data.
      @note @a rawData must remain valid for the lifetime of this Image.
    */
    OpenGLImage(const char* rawData, const Size<uint32_t>& size, ImageFormat format = kImageFormatBGRA);

   /**
      Constructor using another image data.
    */
    OpenGLImage(const OpenGLImage& image);

   /**
      Destructor.
    */
    ~OpenGLImage() override;

   /**
      Load image data from memory.
      @note @a rawData must remain valid for the lifetime of this Image.
    */
    void loadFromMemory(const char* rawData,
                        const Size<uint32_t>& size,
                        ImageFormat format = kImageFormatBGRA) noexcept override;

   /**
      Draw this image at position @a pos using the graphics context @a context.
    */
    void drawAt(const GraphicsContext& context, const Point<int>& pos) override;

   /**
      TODO document this.
    */
    OpenGLImage& operator=(const OpenGLImage& image) noexcept;

    // FIXME this should not be needed
    inline void loadFromMemory(const char* rdata, uint32_t w, uint32_t h, ImageFormat fmt = kImageFormatBGRA)
    { loadFromMemory(rdata, Size<uint32_t>(w, h), fmt); }
    inline void draw(const GraphicsContext& context)
    { drawAt(context, Point<int>(0, 0)); }
    inline void drawAt(const GraphicsContext& context, int x, int y)
    { drawAt(context, Point<int>(x, y)); }

private:
    bool setupCalled;
    bool textureInit;
    GLuint textureId;
};

// -----------------------------------------------------------------------

typedef ImageBaseAboutWindow<OpenGLImage> OpenGLImageAboutWindow;
typedef ImageBaseButton<OpenGLImage> OpenGLImageButton;
typedef ImageBaseKnob<OpenGLImage> OpenGLImageKnob;
typedef ImageBaseSlider<OpenGLImage> OpenGLImageSlider;
typedef ImageBaseSwitch<OpenGLImage> OpenGLImageSwitch;

// -----------------------------------------------------------------------


#endif // DGL_OPENGL_HPP_INCLUDED
