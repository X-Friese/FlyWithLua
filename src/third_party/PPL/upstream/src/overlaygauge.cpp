// Copyright (c) 2017, Philipp Ringler philipp@x-plane.com
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation are those
// of the authors and should not be interpreted as representing official policies,
// either expressed or implied, of the FreeBSD Project.

#include "overlaygauge.h"
#include "basics.h"
#include <cstdio>
#include <cstring>
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"

using namespace PPL;

OverlayGauge::OverlayGauge(int left2d, int top2d, int width2d, int height2d, int left3d, int top3d, int width3d, int height3d,
                           int frameOffX, int frameOffY, int, bool , bool is_visible3d, bool is_visible2d,
                           bool, bool , float scale_3d, bool, int panel_render_pass):
    left_3d_(left3d),
    top_3d_(top3d),
    left_2d_(left2d),
    top_2d_(top2d),
    width_3d_(width3d),
    height_3d_(height3d),
    width_2d_(width2d),
    height_2d_(height2d),
    frame_off_x_(frameOffX),
    frame_off_y_(frameOffY),
    visible_2d_(is_visible2d),
    visible_3d_(is_visible3d),
    always_draw_3d_(false),
    scale_3d_(scale_3d),
    width_view_3d_(width3d),
    height_view_3d_(height3d),
    panel_render_pass_(panel_render_pass),
    screen_width_("sim/graphics/view/window_width"),
    screen_height_("sim/graphics/view/window_height"),
    view_type_("sim/graphics/view/view_type"),
    panel_render_type_("sim/graphics/view/panel_render_type"),
    vr_enabled_("sim/graphics/VR/enabled"),
    instrument_brightness_("sim/cockpit2/switches/instrument_brightness_ratio"),
    lit_level_r_("sim/graphics/misc/cockpit_light_level_r"),
    lit_level_g_("sim/graphics/misc/cockpit_light_level_g"),
    lit_level_b_("sim/graphics/misc/cockpit_light_level_b"),
    window_is_dragging_(false),
    window_has_keyboard_focus_(false),
    copy_left_3d_(-1),
    copy_top_3d_(-1)
{
    // sim/graphics/view/panel_render_type
    // debug/texture_browser
    // panel texture
    // 0 = Attr_cockpit
    // 1 = attr_cockpit_Region albedo
    // 2 = attr_cockpit_Region emissive

    XPLMRegisterDrawCallback(draw3dCallback, xplm_Phase_Gauges, 0, this);
    XPLMRegisterFlightLoopCallback(frameCallback, -1, this);

    XPLMCreateWindow_t win;
    memset(&win, 0, sizeof(win));

    win.structSize = sizeof(win);
    win.left = left2d;
    win.top = top2d;
    win.right = left2d+width2d;
    win.bottom = top2d-height2d;
    win.visible = is_visible2d;
    win.drawWindowFunc = draw2dWindowCallback;
    win.handleKeyFunc = handle2dKeyCallback;
    win.handleMouseClickFunc = handle2dClickCallback;
    win.handleCursorFunc = handle2dCursorCallback;
    win.handleMouseWheelFunc = handle2dWheelCallback;
    win.refcon = this;
    win.decorateAsFloatingWindow = xplm_WindowDecorationSelfDecoratedResizable;
    win.layer = xplm_WindowLayerFloatingWindows;
    win.handleRightClickFunc = handle2dRightClickCallback;
    window2d_id_ = XPLMCreateWindowEx(&win);
    XPLMSetWindowIsVisible(window2d_id_, visible_2d_);

    generateTex((int*)(&gauge_texture_), 1);
    bindTex(gauge_texture_, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_3d_, height_3d_, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // store whatever rbo and fbo X-Plane is using right now
    GLint xp_rbo, xp_fbo;
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &xp_rbo);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &xp_fbo);

    // create a renderbuffer object to store depth info
    glGenRenderbuffersEXT(1, &rbo_);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rbo_);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_STENCIL,
                             width_3d_, height_3d_);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, xp_rbo);

    // create a framebuffer object
    glGenFramebuffersEXT(1, &fbo_);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);

    // attach the texture to FBO color attachment point
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                              GL_TEXTURE_2D, gauge_texture_, 0);

    // attach the renderbuffer to depth attachment point
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_STENCIL_ATTACHMENT,
                                 GL_RENDERBUFFER_EXT, rbo_);

    // check FBO status
    GLenum status_ = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status_ != GL_FRAMEBUFFER_COMPLETE_EXT)
        throw std::runtime_error("No possible FBO, sorry");

    // switch back to window-system-provided framebuffer
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, xp_fbo);
}

OverlayGauge::~OverlayGauge()
{
    XPLMUnregisterDrawCallback(draw3dCallback, xplm_Phase_Gauges, 0, this);
    XPLMDestroyWindow(window2d_id_);
    XPLMUnregisterFlightLoopCallback(frameCallback, this);
}

void OverlayGauge::set3d(int left3d, int top3d, int width3d, int height3d, int, bool always_draw_3d)
{
    always_draw_3d_ = always_draw_3d;

    left_3d_ = left3d;
    top_3d_ = top3d;

    width_3d_ = width3d;
    height_3d_ = height3d;
    width_view_3d_ = width3d;
    height_view_3d_ = height3d;

    visible_3d_ = true;
}

void OverlayGauge::setCopy3d(int left3dcopy, int top3dcopy)
{
    copy_left_3d_ = left3dcopy;
    copy_top_3d_ = top3dcopy;
}

void OverlayGauge::disable3d()
{
    visible_3d_ = false;
}

void OverlayGauge::setVisible(bool b)
{
    if (!b)
    {
        XPLMTakeKeyboardFocus(nullptr);
        window_has_keyboard_focus_ = false;
    }
    visible_2d_ = b;
    XPLMSetWindowIsVisible(window2d_id_, visible_2d_);
    if (b && wantVRifAvailable())
    {
        XPLMSetWindowPositioningMode(window2d_id_, (vr_enabled_==1) ? xplm_WindowVR : xplm_WindowPositionFree, -1);
        vr_enabled_.save();
    }
}

bool OverlayGauge::isVisible() const
{
    return visible_2d_;
}

void OverlayGauge::frame()
{
    update();
    visible_2d_ = XPLMGetWindowIsVisible(window2d_id_);
    if (!wantVRifAvailable())
        return;
    if (visible_2d_)
    if (vr_enabled_.hasChanged())
    {
        XPLMSetWindowPositioningMode(window2d_id_, (vr_enabled_==1) ? xplm_WindowVR : xplm_WindowPositionFree, -1);
        if (vr_enabled_ == 0)
            XPLMSetWindowGeometry(window2d_id_, left_2d_, top_2d_, left_2d_+width_2d_, top_2d_-height_2d_);
        vr_enabled_.save();
    }
}

void OverlayGauge::drawTexture(int tex_id, int left, int top, int right, int bottom, bool vflip)
{
    bindTex(tex_id, 0);
    glBegin(GL_QUADS);
    glTexCoord2f(0,!vflip); glVertex2f(left, top);
    glTexCoord2f(1,!vflip); glVertex2f(right, top);
    glTexCoord2f(1, vflip); glVertex2f(right, bottom);
    glTexCoord2f(0, vflip); glVertex2f(left, bottom);
    glEnd();
}

int OverlayGauge::draw3dCallback(XPLMDrawingPhase, int)
{
    if (!visible_2d_)
        updateFBO();
    if (panel_render_type_ == 0 || panel_render_type_ == panel_render_pass_)
    {
        if (view_type_ == 1026 || always_draw_3d_)
        {
            if (visible_3d_)
            {
                glColor4f(1,1,1, instrumentBrightness());
                drawTexture(gauge_texture_, left_3d_, top_3d_, left_3d_+width_view_3d_ * scale_3d_, top_3d_-height_view_3d_*scale_3d_,wantVFlip());

                if (copy_top_3d_ > -1 && copy_left_3d_ > -1)
                {
                    drawTexture(gauge_texture_, copy_left_3d_, copy_top_3d_, copy_left_3d_+width_view_3d_ * scale_3d_, copy_top_3d_-height_view_3d_*scale_3d_, wantVFlip());
                }
            }
        }
    }
    return 1;
}

void OverlayGauge::toggleKeyboardFocus()
{
    if (window_has_keyboard_focus_)
    {
        XPLMTakeKeyboardFocus(nullptr);
        window_has_keyboard_focus_ = false;
    }
    else
    {
        XPLMTakeKeyboardFocus(window2d_id_);
        window_has_keyboard_focus_ = true;
    }
}

float OverlayGauge::instrumentBrightness() const
{
    return instrument_brightness_[0];
}

bool OverlayGauge::wantVRifAvailable() const
{
    return true;
}

void OverlayGauge::update()
{
}

void OverlayGauge::draw2dWindowCallback(XPLMWindowID)
{
    if (visible_2d_)
    {
        updateFBO();
        int left, top, right, bottom;
        XPLMGetWindowGeometry(window2d_id_, &left, &top, &right, &bottom);
        float sX = (right-left)/float(width_2d_);
        float sY = (top-bottom)/float(height_2d_);

        setDrawState(0,1,0,0,1,0,0);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);  // normal colors

        glColor4f(0,0,0,1);
        drawFrameTexture(left, top, right, bottom);

        if (wantClearTexture())
        {
            setDrawState(0,0,0,0,1,0,0);
            glColor4f(0,0,0,1);
            glBegin(GL_QUADS);
            glVertex2f(left+ frame_off_x_*sX                , top-frame_off_y_*sY);
            glVertex2f(left+(frame_off_x_+width_view_3d_)*sX, top-frame_off_y_*sY);
            glVertex2f(left+(frame_off_x_+width_view_3d_)*sX, top-(frame_off_y_+height_view_3d_)*sY);
            glVertex2f(left+ frame_off_x_*sX                , top-(frame_off_y_+height_view_3d_)*sY);
            glEnd();
            setDrawState(0,1,0,0,1,0,0);
        }

        glColor4f(1,1,1,instrumentBrightness());
        drawTexture(gauge_texture_, left+frame_off_x_*sX,
                                    top-frame_off_y_*sY,
                                    left+(frame_off_x_+width_view_3d_)*sX,
                                    top-(frame_off_y_+height_view_3d_)*sY,
                    wantVFlip());

        drawFrameTextureLit(left, top, right, bottom);

        glBlendFunc(GL_SRC_ALPHA,GL_ONE);   // glow colors
        glColor4f(lit_level_r_, lit_level_g_, lit_level_b_, 1);
        drawFrameTexture(left, top, right, bottom);

        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);  // normal colors
        glColor4f(1,1,1,1);


        if (window_has_keyboard_focus_)
        {
            static float color[] = { 1.f, 0.5f, 0.f};
            static char str[] = "K";
            XPLMDrawString(color, left + 20, top - 25, str, nullptr, xplmFont_Proportional);
        }
    }
}

void OverlayGauge::updateFBO()
{
    if (wantRedraw())
    {
        GLint xp_fbo;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &xp_fbo);
        // set rendering destination to FBO
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);
        glPushAttrib(GL_VIEWPORT_BIT);
        glViewport(0,0,width_3d_, height_3d_);
        glMatrixMode (GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0,width_3d_, 0, height_3d_, 0, 1);
        glMatrixMode (GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        // clear buffers
        if (wantClearTexture())
            glClearColor(0,0,0,0);
        else
            glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw a scene to a texture directly
        draw(0, height_3d_, width_3d_, 0);

        // unbind FBO
        glPopMatrix();
        glMatrixMode (GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopAttrib();
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, xp_fbo);
    }
}

void OverlayGauge::handle2dKeyCallback(XPLMWindowID, char key, XPLMKeyFlags flags, unsigned char virtual_key, int losing_focus)
{
    if (losing_focus)
        window_has_keyboard_focus_ = false;
    else
        handleKeyPress(key, flags, virtual_key);
}

int OverlayGauge::handle2dRightClickCallback(XPLMWindowID window_id, int x, int y, XPLMMouseStatus mouse)
{
    int Left, Top, Right, Bottom;

    if (!visible_2d_)
        return 0;

    XPLMGetWindowGeometry(window_id, &Left, &Top, &Right, &Bottom);
    int x_rel = x - Left;
    int y_rel = y - Bottom;
    if (mouse == xplm_MouseDown)
        handleNonDragClick(x_rel, y_rel, true);
    else if (mouse == xplm_MouseUp)
        handleNonDragClickRelease(x_rel, y_rel, true);
    return 1;
}

int OverlayGauge::handle2dClickCallback(XPLMWindowID window_id, int x, int y, XPLMMouseStatus mouse)
{
    int Left, Top, Right, Bottom;

    if (!visible_2d_)
        return 0;

    /// Get the windows current position
    XPLMGetWindowGeometry(window_id, &Left, &Top, &Right, &Bottom);
    int x_rel = x - Left;
    int y_rel = y - Bottom;
    if (!XPLMWindowIsPoppedOut(window_id))
    {
        switch(mouse)
        {
        case xplm_MouseDown:
            /// Test for the mouse in the window
            if (vr_enabled_ == 0)
            {
                if (coordInRect(x, y, Left, Top, Left+40, Top-40))
                {
                    XPLMTakeKeyboardFocus(nullptr);
                    window_has_keyboard_focus_ = false;
                    setVisible(false);
                }
                else if (coordInRect(x, y, Right-40, Top, Right, Top-40))
                {
                    XPLMSetWindowPositioningMode(window2d_id_, xplm_WindowPopOut, -1);
                }
                else if (!handleNonDragClick(x_rel, y_rel, false))
                {
                    dX = x - Left;
                    dY = y - Top;
                    window_is_dragging_ = true;
                }
            }
            else
            {
                handleNonDragClick(x_rel, y_rel, false);
            }
            break;
        case xplm_MouseDrag:
            /// We are dragging so update the window position
            if (vr_enabled_ == 0)
            if (window_is_dragging_)
            {
                left_2d_ = Left = (x - dX);
                Right = Left + width_2d_;
                top_2d_ = Top = (y - dY);
                Bottom = Top - height_2d_;
                XPLMSetWindowGeometry(window_id, Left, Top, Right, Bottom);
            }
            break;
        case xplm_MouseUp:
            window_is_dragging_ = false;
            handleNonDragClickRelease(x_rel, y_rel, false);
            break;
        }
    }
    else
    {
        switch(mouse)
        {
        case xplm_MouseDown:
            /// Test for the mouse in the window
            if (coordInRect(x, y, Left, Top, Left+40, Top-40) || coordInRect(x, y, Right-40, Top, Right, Top-40))
            {
                XPLMSetWindowPositioningMode(window2d_id_, xplm_WindowPositionFree, -1);
                XPLMSetWindowGeometry(window_id, left_2d_, top_2d_, left_2d_+width_2d_, top_2d_-height_2d_);
            }
            else
                handleNonDragClick(x_rel, y_rel, false);
            break;
        case xplm_MouseUp:
            window_is_dragging_ = false;
            handleNonDragClickRelease(x_rel, y_rel, false);
            break;
        }
    }
    return 1;
}

void OverlayGauge::handleNonDragClickRelease(int, int, bool)
{
}

XPLMCursorStatus OverlayGauge::handle2dCursorCallback(XPLMWindowID, int, int)
{
    if (visible_2d_)
        return xplm_CursorArrow;
    else
        return xplm_CursorDefault;
}

int OverlayGauge::handle2dWheelCallback(XPLMWindowID inWindowID, int x, int y, int wheel, int clicks)
{
    if (visible_2d_)
    {
        int left, right, top, bottom;
        XPLMGetWindowGeometry(inWindowID, &left, &top, &right, &bottom);
        handleMouseWheel(x-left, y-bottom, wheel, clicks);
        return 1;
    }
    return 0;
}

bool OverlayGauge::wantRedraw()
{
    return true;
}

void OverlayGauge::drawFrameTexture(int left, int top, int right, int bottom)
{
    int tex_id = frameTextureId();
    if (tex_id > 0)
    {
        drawTexture(tex_id, left, top, right, bottom);
    }
}

void OverlayGauge::drawFrameTextureLit(int left, int top, int right, int bottom)
{
    int tex_id = frameTextureLitId();
    if (tex_id > 0)
    {
        drawTexture(tex_id, left, top, right, bottom);
    }
}

bool OverlayGauge::wantClearTexture() const
{
    return false;
}

bool OverlayGauge::wantVFlip() const
{
    return false;
}

int OverlayGauge::handleMouseWheel(int, int, int, int)
{
    return 1;
}

int OverlayGauge::draw3dCallback(XPLMDrawingPhase phase, int is_before, void* refcon)
{
    OverlayGauge* window = static_cast<OverlayGauge*>(refcon);
    return window->draw3dCallback(phase, is_before);
}

void OverlayGauge::draw2dWindowCallback(XPLMWindowID window_id, void* refcon)
{
    OverlayGauge* window = static_cast<OverlayGauge*>(refcon);
    window->draw2dWindowCallback(window_id);
}

void OverlayGauge::handle2dKeyCallback(XPLMWindowID window_id, char key, XPLMKeyFlags flags, char virtual_key, void* refcon, int losing_focus)
{
    OverlayGauge* window = static_cast<OverlayGauge*>(refcon);
    window->handle2dKeyCallback(window_id, key, flags, static_cast<unsigned char>(virtual_key), losing_focus);
}

int OverlayGauge::handle2dClickCallback(XPLMWindowID window_id, int x, int y, XPLMMouseStatus mouse, void* refcon)
{
    OverlayGauge* window = static_cast<OverlayGauge*>(refcon);
    return window->handle2dClickCallback(window_id, x, y, mouse);
}

int OverlayGauge::handle2dRightClickCallback(XPLMWindowID window_id, int x, int y, XPLMMouseStatus mouse, void* refcon)
{
    OverlayGauge* window = static_cast<OverlayGauge*>(refcon);
    return window->handle2dRightClickCallback(window_id, x, y, mouse);
}

XPLMCursorStatus OverlayGauge::handle2dCursorCallback(XPLMWindowID window_id, int x, int y, void* refcon)
{
    OverlayGauge* window = static_cast<OverlayGauge*>(refcon);
    return window->handle2dCursorCallback(window_id, x, y);
}

int OverlayGauge::handle2dWheelCallback(XPLMWindowID window_id, int x, int y, int wheel, int clicks, void *refcon)
{
    OverlayGauge* window = static_cast<OverlayGauge*>(refcon);
    return window->handle2dWheelCallback(window_id, x, y, wheel, clicks);
}

float OverlayGauge::frameCallback(float, float, int, void* refcon)
{
    OverlayGauge* window = static_cast<OverlayGauge*>(refcon);
    window->frame();
    return -1;
}

void OverlayGauge::setDrawState(int inEnableFog, int inNumberTexUnits, int inEnableLighting, int inEnableAlphaTesting, int inEnableAlphaBlending, int inEnableDepthTesting, int inEnableDepthWriting)
{
    XPLMSetGraphicsState(inEnableFog, inNumberTexUnits, inEnableLighting, inEnableAlphaTesting, inEnableAlphaBlending, inEnableDepthTesting, inEnableDepthWriting);
}

void OverlayGauge::bindTex(int tex_id, int texture_unit)
{
    XPLMBindTexture2d(tex_id, texture_unit);
}

void OverlayGauge::generateTex(int* tex_ids, int number_of_textures)
{
    XPLMGenerateTextureNumbers(tex_ids, number_of_textures);
}

bool OverlayGauge::coordInRect(float x, float y, float l, float t, float r, float b)
{
    return ((x >= l) && (x < r) && (y < t) && (y >= b));
}
