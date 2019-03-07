/*
 * Copyright (c) 2006, Ben Supnik and Chris Serio.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#ifndef _Fonts_h_
#define _Fonts_h_

#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>
#include <string>

namespace PPL {

using std::multimap;
using std::string;

/*

    A BRIEF DIGRESSION ON FONT METRICS BY BEN....

    We have to get our terms consistent (if not straight) if we are to have any hope of drawing fonts in the right
    location.  Some terms:

    - A "point" is a real-world physical unit...about 1/72th of an inch, more or less...see WikiPedia for more info
        than you'd ever want to know.  Since the old CRTs ran at about 72 DPI, a point and a pixel have become
        somewhat interchangeable to old-school Mac font programmers.

    - A "pixel" is a single phosphor on your screen...one of the "dots" you can see.  The DPI of the monitor tells
        how big they are.  Most monitors are around 90 DPI, more or less, I think...they used to be more like 72 DPI.

    - A "texel" is a pixel in a texture.  Because OpenGL will draw our fonts almost anywhere, in any shape, including
        in 3-d, it's not really proper to talk about how m any pixels tall a letter is unless we've very carefully
        controlled a number of aspects of the OpenGL drawing environment.  However the texture is built by the font
        manager, so we can speak with total certainty about how many "texels" tall a letter is.

    - The "baseline" of a font is a line of reference that letters "sit on"...if you are in 1st grade and are learning
        to right, you try to write on the baseline.

    - Letters that go below the baseline (like 'g' and 'p') are "descenders".  The font "descent" is the distance from
        the baseline to the lowest of the descenders.

    - The "ascent" is the distance from the baseline to the tallest letter.  So the height of an A is the ascent
        because it's real tall and sits on the baseline.

    - The line height is the distance from one baseline to the next when spacing multiple lines.

    - The point size of the font is the height from the lowest descender to tallest ascender (descent + ascent) in
        points.

    Now that was a lot of fun but who cares?  Well here's the KEY to understanding the fontmgr's layout:

    The font manager always renders fonts into their textures at a scale of 1 point = 1 texel!

    So...the "point size" you request should be based on roughly how many screen pixels you plan to cover.  Want
    your 'A' to be about 13 screen pixels?  Well you have to pick the rectangle, but ask for a 13 point font so
    that your 13 texel A looks good.

    Please note that this doesn't take into account the monitor resolution at all!  If you draw at 1 texel = 1 pixel
    and draw a 13 point font on a modern monitor, it will look SMALLER than a real-world 13 point font because
    the font manager never takes the monitor DPI into account.  Heck - the font manager doesn't KNOW the DPI of
    your monitor!

    Point sizes are passed into the font manager as unsigned integers - no fractional point sizes for now.

    Line heights and other measurements are given in float point fractional texels.  While a texel is either there
    or not, we preserve the floating point metrics to provide the most accurate layout of the font.

    OPEN ISSUE: Hrm...the glyphs are rendered with rounding...can we capture that so the metrics match the real
    texels?  The FreeType manual isn't terribly clear on this.

    A WARNING ABOUT MULTIPLE OPENGL CONTEXTS:

    FontMgr attempts to only build up each font once.  But it cannot tell if you have multiple OpenGL windows.  Therefore
    if you hvae multiple OpenGL windows you should share your texture contexts between them using the appropriate
    windowing-system specific stuff.

 */

/*
 * Opaque Font pointer...
 * Declaraction is in the CPP
 */
typedef struct FontInfo_t* FontHandle;

/*
 * Enums to allow the user to specify the alignment of the
 * text string that we're going to draw.
 */
enum {
    ALIGN_LEFT = 0,
    ALIGN_CENTER,
    ALIGN_RIGHT
};
typedef int FontAlign_t;

/*
 * Class FontMgr
 *
 * This class is essentially a wrapper around the FreeType 2.x font library.
 * It uses FreeType to generate a pixmap texture of all possible characters in
 * a given true-type font. Once the texture is generated for a particular font,
 * FreeType resources are released and we then use the texture to draw text ourselves.
 *
 * The class enforces a 10px font interval to save on resources. That is, any texture
 * generated must be on a 10px interval. If the client asks for a 13px texture and we
 * already have a 10px one, he'll share that with the other user. If there is no 10px
 * texture, we generate one for the client.
 */
class FontMgr {
public:

    FontMgr();
    ~FontMgr();

    /*
     * Load font takes the path to a TTF as well as a font size. This size specifies
     * how big the font on the TEXTURE will be, NOT how big the font will be when
     * it's drawn! If a size is specified that's not on an interval of 10px, it will
     * be rounded to a 10px size, unless inRequireExactSize is true, in which case
     * you get the precise requested size.
     * This flag also affects filtering: nearest neighbor when exact sizes are requested,
     * for a sharp look, otherwise linear filtering for a scalable look.
     * This function returns a FontHandle to be used as a reference for draw and unload
     * requests.
     * NOTE: Texture sharing is implemented for multiple requests for the same font.
     *		 Each call to LoadFont increments a reference count for the texture whether
     *		 it is created or shared.
     */
    FontHandle loadFont(
                const char*						inFontPath,
                const char *					inStartMem,		// null if we open file
                const char *					inEndMem,		// null if we open file
                unsigned int					inSizePx,
                bool							inRequireExactSize);

    /*
     * Unload font takes a font handle and will decrement the reference count of a
     * particular font. When no one is using the font anymore, it will be unloaded
     * from memory and the texture will be deleted.
     */
    void	unloadFont(
                FontHandle						inFont);

    /*
     * This is DEBUGGING function that will display the pixmap texture on the screen
     * so that you can see what the font looks like.
     */
    void	displayTexture(
                FontHandle						inFont);

    /*
     * Simple Drawing: draws a left-justified string starting at a point.  The first
     * letter will sit on a line starting at X, and Y (e.g. Y is the basline).  Font is
     * drawn such that one texel = one modelview unit.  So this is only appropriate if
     * you are using OpenGL with "pixels" as a drawing unit.
     *
     */
    void	drawString(
                FontHandle						inFont,
                float							color[4],	//	4-part color, featuring alpha.
                float							inX,
                float							inY,
                const char *					inString);

    /*
     * Simple Drawing: draws a left- or right justified string starting at a point.  The first
     * letter will sit on a line starting at X, and Y (e.g. Y is the basline).  Font is
     * drawn such that one texel = one modelview unit.  So this is only appropriate if
     * you are using OpenGL with "pixels" as a drawing unit.
     *
     */
    void	drawString(
                FontHandle						inFont,
                float							color[4],	//	4-part color, featuring alpha.
                float							inX,
                float							inY,
                FontAlign_t                     align,
                const char *					inString);

    /*
     * Advanced drawing: draws a string justified within a box.  The bottom of the box is
     * the lowest descender, the top is the highest ascender plus any built-in leading.
     * Justification: inRight is ignored for left-justified strings.  The whole string is
     * drawn - even parts that exceed the passed in box.
     *
     */
    void	drawRange(
                FontHandle						inFont,
                float							color[4],	//	4-part color, featuring alpha.
                float							inLeft,
                float							inBottom,
                float							inRight,
                float							inTop,
                const char *					inStart,
                const char *					inEnd,
                FontAlign_t						fontAlign);

    /***************** FONT MEASUREMENT ROUTINES ******************
     *
     * These routines measure in "pixels" - that is, screen units.  So you have to provide
     * how tall the line will end up and they return how wide it will be.
     *
     */

    /*
     * Given a font and a height in px, this function returns the length necessary
     * to draw the string in px.
     *
     * OPEN ISSUE: change to start, end + 1 syntax?
     *
     */
    float	measureString(
                FontHandle						inFont,
                float							inHeightPx,
                const char *					inString);

    float	measureRange(
                FontHandle						inFont,
                float							inHeightPx,
                const char *					inStart,
                const char *					inEnd);

    /*
     * Given a string, return how many characters will fit in a given space, either
     * forward or backward.  Strings are specified as a ptr to the start and a ptr to
     * the end plus 1.
     *
     */
    int		fitForward(
                FontHandle						inFont,
                float							inHeightPx,
                float							inWidthPx,
                const char *					inStringStart,
                const char *					inStringEnd);

    int		fitReverse(
                FontHandle						inFont,
                float							inHeightPx,
                float							inWidthPx,
                const char *					inStringStart,
                const char *					inStringEnd);

    /******************** FONT METRICS *************************
     *
     * These routines return the measurements of the font in TEXELS.
     * You pass in the point size of the font.  Why?  Well, if you are
     * using a scalable font, the point size you asked for may not be
     * the one in the font handle.  These routines take this into
     * consideration.
     *
     * OPEN ISSUE perhaps the font handle should know the actual font size
     * and the requested font size?  Passing the size in pixels is weird.
     *
     */

    /*
     * Returns the line height - that is the number of texels from
     * one baseline to another.
     *
     */
    float	getLineHeight(
                FontHandle						inFont,
                unsigned int					inFontSizePx);

    /*
     * Returns the number of texels from the baseline to the lowest
     * descender (this is like the bottom of o to the bottom of g).
     *
     */
    float	getLineDescent(
                FontHandle						inFont,
                unsigned int					inFontSizePx);

    /*
     * Returns the number of texels from the baseline to the top of
     * the tallest ascender.  This is like the height of the letter A.
     *
     */
    float	getLineAscent(
                FontHandle						inFont,
                unsigned int					inFontSizePx);

private:

    void	CalcTexSize(
                FT_Face*						inFace,
                int*							outHeight,
                int*							outWidth);

    void    CopyBitmapSection(
                unsigned int					inSrcHeight,
                unsigned int					inSrcWidth,
                unsigned int					inSrcLeft,
                unsigned int					inSrcTop,
                unsigned char*					inSrcData,
                unsigned int					inDstHeight,
                unsigned int					inDstWidth,
                unsigned int					inDstLeft,
                unsigned int					inDstTop,
                unsigned char*					inDstData);

    typedef multimap<string, FontInfo_t*> TextureMap_t;

    FT_Library		library_;
    TextureMap_t	tex_map_;
};

}

#endif
