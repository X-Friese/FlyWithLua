/*
 *   Floating Windows with imgui integration for FlyWithLua
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *   Released as public domain code.
 *
 */
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include <XPLMGraphics.h>
#include <XPLMDisplay.h>
#include <XPLMDataAccess.h>
#include <XPLMUtilities.h>
#include <memory>
#include <stdexcept>
#include "FloatingWindow.h"
#include "../FlyWithLua.h"

namespace {

void multMatrixVec4f(GLfloat dst[4], const GLfloat m[16], const GLfloat v[4]) {
    dst[0] = v[0] * m[0] + v[1] * m[4] + v[2] * m[8] + v[3] * m[12];
    dst[1] = v[0] * m[1] + v[1] * m[5] + v[2] * m[9] + v[3] * m[13];
    dst[2] = v[0] * m[2] + v[1] * m[6] + v[2] * m[10] + v[3] * m[14];
    dst[3] = v[0] * m[3] + v[1] * m[7] + v[2] * m[11] + v[3] * m[15];
}

} // anonymous helpers namespace

namespace flwnd {

FloatingWindow::FloatingWindow(int winWidth, int winHeight, int winDecoration):
    width(winWidth),
    height(winHeight),
    decoration(winDecoration)
{
    vrEnabledRef = XPLMFindDataRef("sim/graphics/VR/enabled");
    modeliewMatrixRef = XPLMFindDataRef("sim/graphics/view/modelview_matrix");
    viewportRef = XPLMFindDataRef("sim/graphics/view/viewport");
    projectionMatrixRef = XPLMFindDataRef("sim/graphics/view/projection_matrix");

    createWindow();
}

void FloatingWindow::createWindow() {
    int winLeft, winTop, winRight, winBot;
    XPLMGetScreenBoundsGlobal(&winLeft, &winTop, &winRight, &winBot);

    XPLMCreateWindow_t params;
    params.structSize = sizeof(params);
    params.left = winLeft + 100 ;
    params.right = winLeft + 100 + width;
    params.top = winTop - 100 ;
    params.bottom = winTop - 100 - height;
    params.visible = 1;
    params.refcon = this;
    params.drawWindowFunc = [] (XPLMWindowID id, void *ref) {
        try {
            reinterpret_cast<FloatingWindow*>(ref)->onDraw();
        } catch (std::exception& e) {
            flywithlua::logMsg(logToDevCon, std::string("Caught error: ") + e.what());
        }
    };
    params.handleMouseClickFunc = [] (XPLMWindowID id, int x, int y, XPLMMouseStatus status, void *ref) -> int {
        return reinterpret_cast<FloatingWindow *>(ref)->onClick(x, y, status);
    };
    params.handleRightClickFunc = [] (XPLMWindowID id, int x, int y, XPLMMouseStatus status, void *ref) -> int {
        return reinterpret_cast<FloatingWindow *>(ref)->onRightClick(x, y, status);
    };
    params.handleKeyFunc = [] (XPLMWindowID id, char key, XPLMKeyFlags flags, char vKey, void *ref, int losingFocus) {
        reinterpret_cast<FloatingWindow *>(ref)->onKey(key, flags, vKey, losingFocus);
    };
    params.handleCursorFunc = [] (XPLMWindowID id, int x, int y, void *ref) -> XPLMCursorStatus {
        return reinterpret_cast<FloatingWindow *>(ref)->getCursor(x, y);
    };
    params.handleMouseWheelFunc =  [] (XPLMWindowID id, int x, int y, int wheel, int clicks, void *ref) -> int {
        return reinterpret_cast<FloatingWindow *>(ref)->onMouseWheel(x, y, wheel, clicks);
    };
    params.layer = xplm_WindowLayerFloatingWindows;

    params.decorateAsFloatingWindow = decoration;

    window = XPLMCreateWindowEx(&params);

    if (!window) {
        throw std::runtime_error("Couldn't create window");
    }

    moveFromOrToVR();
}

void FloatingWindow::setDrawCallback(DrawCallback cb) {
    onDrawCB = cb;
}

void FloatingWindow::setClickCallback(ClickCallback cb) {
    onClickCB = cb;
}

void FloatingWindow::setCloseCallback(CloseCallback cb) {
    onCloseCB = cb;
}

void FloatingWindow::setKeyCallback(KeyCallback cb) {
    onKeyCB = cb;
}

void FloatingWindow::setTitle(const char *title) {
    XPLMSetWindowTitle(window, title);
}

void FloatingWindow::setPosition(int posx, int posy) {

    bool vrEnabled = XPLMGetDatai(vrEnabledRef);

    if (!vrEnabled) {
        int winLeft, winTop, winRight, winBot;
        XPLMGetScreenBoundsGlobal(&winLeft, &winTop, &winRight, &winBot);

        // XPLMSetWindowGeometry(window, winLeft + posx, winTop - posy, winLeft + posx + width, winTop - posy - height);
        // This is not working corectly and think I should be using bottom
        XPLMSetWindowGeometry(window, winLeft + posx, winBot + posy + height, winLeft + posx + width, winBot + posy);
    }
}

bool FloatingWindow::isPopped() {
    return XPLMWindowIsPoppedOut(window);
}

bool FloatingWindow::isFront() {
    return XPLMIsWindowInFront(window);
}

void FloatingWindow::bringToFront() {

    bool vrEnabled = XPLMGetDatai(vrEnabledRef);

    if (!vrEnabled) {
        XPLMBringWindowToFront(window);
    }
}

void FloatingWindow::setResizingLimits(int minwidth, int minheight, int maxwidth, int maxheight) {
    XPLMSetWindowResizingLimits(window, minwidth, minheight, maxwidth, maxheight);
}

void FloatingWindow::setPositioningMode(int pmode, int monindex) {
    XPLMSetWindowPositioningMode(window, static_cast<XPLMWindowPositioningMode>(pmode), monindex);
}

void FloatingWindow::setGravity(float inLeftGravity, float inTopGravity, float inRightGravity, float inBottomGravity) {

    bool vrEnabled = XPLMGetDatai(vrEnabledRef);

    if (!vrEnabled) {
        XPLMSetWindowGravity(window, inLeftGravity, inTopGravity, inRightGravity, inBottomGravity);
    }
}

void FloatingWindow::setWindowGeometry(int mleft, int mtop, int mright, int mbot) {

    bool vrEnabled = XPLMGetDatai(vrEnabledRef);

    if (!vrEnabled) {
        XPLMSetWindowGeometry(window, mleft, mtop, mright, mbot);
    }
}

void FloatingWindow::setWindowGeometryOS(int mleft, int mtop, int mright, int mbot) {

    bool vrEnabled = XPLMGetDatai(vrEnabledRef);

    if (!vrEnabled && isPopped()) {
        XPLMSetWindowGeometryOS(window, mleft, mtop, mright, mbot);
    }
}

void FloatingWindow::setWindowGeometryVR(int bWidth, int bHeight) {

    bool vrEnabled = XPLMGetDatai(vrEnabledRef);

    if (vrEnabled) {
        XPLMSetWindowGeometryVR(window, bWidth, bHeight);
    }
}

bool FloatingWindow::isVR() {
    return XPLMWindowIsInVR(window);
}

void FloatingWindow::setIsCmdVisible(int fCondition) {

    bool vrEnabled = XPLMGetDatai(vrEnabledRef);

    if (!vrEnabled) {
        fCondition == 0 ? isCmdVisible = true : isCmdVisible = false;
    }
}

bool FloatingWindow::getIsCmdVisible() {
    return isCmdVisible;
}

void FloatingWindow::moveFromOrToVR() {
    bool vrEnabled = XPLMGetDatai(vrEnabledRef);

    if (vrEnabled && !isInVR) {
        // X-Plane switched to VR but our window isn't in VR
        XPLMSetWindowPositioningMode(window, xplm_WindowVR, -1);
        isInVR = true;
    } else if (!vrEnabled && isInVR) {
        // Our window is still in VR but X-Plane switched to 2D
        XPLMSetWindowPositioningMode(window, xplm_WindowPositionFree, -1);
        isInVR = false;
        
        int winLeft, winTop, winRight, winBot;
        XPLMGetScreenBoundsGlobal(&winLeft, &winTop, &winRight, &winBot);

        XPLMSetWindowGeometry(window, winLeft + 100, winTop - 100, winLeft + 100 + width, winTop - 100 - height);
    }
}

void FloatingWindow::requestInputFocus(bool req) {
    XPLMTakeKeyboardFocus(req ? window : nullptr);
}

bool FloatingWindow::hasInputFocus() {
    return XPLMHasKeyboardFocus(window);
}

void FloatingWindow::updateMatrices() {
    // Get the current modelview matrix, viewport, and projection matrix from X-Plane
    XPLMGetDatavf(modeliewMatrixRef, mModelView, 0, 16);
    XPLMGetDatavf(projectionMatrixRef, mProjection, 0, 16);
    XPLMGetDatavi(viewportRef, mViewport, 0, 4);
}

void FloatingWindow::boxelsToNative(int x, int y, int& outX, int& outY) {
    GLfloat boxelPos[4] = { (GLfloat) x, (GLfloat) y, 0, 1 };
    GLfloat eye[4], ndc[4];

    multMatrixVec4f(eye, mModelView, boxelPos);
    multMatrixVec4f(ndc, mProjection, eye);
    ndc[3] = 1.0f / ndc[3];
    ndc[0] *= ndc[3];
    ndc[1] *= ndc[3];

    outX = static_cast<int>((ndc[0] * 0.5f + 0.5f) * mViewport[2] + mViewport[0]);
    outY = static_cast<int>((ndc[1] * 0.5f + 0.5f) * mViewport[3] + mViewport[1]);
}

void FloatingWindow::onDraw() {
    if (onDrawCB) {
        onDrawCB(*this);
    }
}

bool FloatingWindow::onClick(int x, int y, XPLMMouseStatus status) {
    if (onClickCB) {
        onClickCB(*this, x, y, status);
    }
    return true;
}

void FloatingWindow::reportClose() {
    if (onCloseCB) {
        onCloseCB(*this);
    }
}

bool FloatingWindow::isVisible() const {
    return XPLMGetWindowIsVisible(window);
}

void FloatingWindow::setVisible(bool visible) {
    XPLMSetWindowIsVisible(window, visible);
}

bool FloatingWindow::onRightClick(int x, int y, XPLMMouseStatus status) {
    return true;
}

void FloatingWindow::onKey(char key, XPLMKeyFlags flags, char virtualKey, bool losingFocus) {
    if (losingFocus) {
        return;
    }

    if (onKeyCB) {
        onKeyCB(*this, key, virtualKey, flags);
    }
}

XPLMCursorStatus FloatingWindow::getCursor(int x, int y) {
    return xplm_CursorDefault;
}

bool FloatingWindow::onMouseWheel(int x, int y, int wheel, int clicks) {
    return true;
}

XPLMWindowID FloatingWindow::getXWindow() {
    return window;
}

FloatingWindow::~FloatingWindow() {
    XPLMDestroyWindow(window);
}

} // namespace flwnd
