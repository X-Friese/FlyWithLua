/*
 *   FlyWithLuaWindow: Modern ImgWindow (Panel Graphics / OpenGL fallback)
 *   subclass for FlyWithLua.
 */
#include "FlyWithLuaWindow.h"
#include "../FlyWithLua.h"
#include <XPLMDisplay.h>
#include <XPLMGraphics.h>

namespace flwnd {

namespace {
    inline WindowGeometry GetDefaultFlyWithLuaGeometry(int width, int height) {
        int sl = 0, st = 0;
        XPLMGetScreenBoundsGlobal(&sl, &st, nullptr, nullptr);
        return WindowGeometry::FromTopLeft(sl + 100, st - 100, width, height);
    }
}

FlyWithLuaWindow::FlyWithLuaWindow(int width, int height, int decoration) :
    ImgWindow(
        GetDefaultFlyWithLuaGeometry(width, height),
        static_cast<XPLMWindowDecoration>(decoration),
        xplm_WindowLayerFloatingWindows,
        flywithlua::imgui_mouse_cursors != 0),
    mUseImgCursors(flywithlua::imgui_mouse_cursors != 0)
{
    SetWindowTitle("FlyWithLua");
    SetVisible(true);
}

void FlyWithLuaWindow::buildInterface() {
    if (stopped) {
        return;
    }

    try {
        if (doBuild) {
            doBuild(*this);
        }
    } catch (const std::exception &e) {
        if (onError) {
            onError(e.what());
        }
        stopped = true;
    }
}

ImGuiWindowFlags_ FlyWithLuaWindow::beforeBegin() {
    auto &io = ImGui::GetIO();
    io.ConfigDebugHighlightIdConflicts = (flywithlua::imgui_debug_popup == 1);
    return static_cast<ImGuiWindowFlags_>(ImGuiWindowFlags_HorizontalScrollbar);
}

void FlyWithLuaWindow::setBuildCallback(BuildCallback cb) {
    doBuild = cb;
}

void FlyWithLuaWindow::setErrorHandler(ErrorHandler eh) {
    onError = eh;
}

void FlyWithLuaWindow::setCloseCallback(CloseCallback cb) {
    onCloseCB = cb;
}

void FlyWithLuaWindow::reportClose() {
    if (onCloseCB) {
        onCloseCB(*this);
    }
}

void FlyWithLuaWindow::setTitle(const char *title) {
    SetWindowTitle(title ? title : "");
}

void FlyWithLuaWindow::setPosition(int posx, int posy) {
    if (!IsInVR()) {
        int winLeft, winTop, winRight, winBot;
        XPLMGetScreenBoundsGlobal(&winLeft, &winTop, &winRight, &winBot);
        int curLeft, curTop, curRight, curBottom;
        GetWindowGeometry(curLeft, curTop, curRight, curBottom);
        int w = curRight - curLeft;
        int h = curTop - curBottom;
        SetWindowGeometry(winLeft + posx, winBot + posy + h, winLeft + posx + w, winBot + posy);
    }
}

void FlyWithLuaWindow::setWindowGeometry(int mleft, int mtop, int mright, int mbot) {
    if (!IsInVR()) {
        SetWindowGeometry(mleft, mtop, mright, mbot);
    }
}

void FlyWithLuaWindow::setWindowGeometryOS(int mleft, int mtop, int mright, int mbot) {
    if (!IsInVR() && IsPoppedOut()) {
        SetWindowGeometryOS(mleft, mtop, mright, mbot);
    }
}

void FlyWithLuaWindow::setWindowGeometryVR(int bWidth, int bHeight) {
    if (IsInVR()) {
        SetWindowGeometryVR(bWidth, bHeight);
    }
}

void FlyWithLuaWindow::setResizingLimits(int minwidth, int minheight, int maxwidth, int maxheight) {
    SetWindowResizingLimits(minwidth, minheight, maxwidth, maxheight);
}

void FlyWithLuaWindow::setPositioningMode(int pmode, int monindex) {
    SetWindowPositioningMode(static_cast<XPLMWindowPositioningMode>(pmode), monindex);
}

void FlyWithLuaWindow::setGravity(float inLeftGravity, float inTopGravity, float inRightGravity, float inBottomGravity) {
    if (!IsInVR()) {
        XPLMSetWindowGravity(GetWindowId(), inLeftGravity, inTopGravity, inRightGravity, inBottomGravity);
    }
}

bool FlyWithLuaWindow::isVisible() const {
    return GetVisible();
}

void FlyWithLuaWindow::setVisible(bool visible) {
    SetVisible(visible);
}

bool FlyWithLuaWindow::isPopped() {
    return IsPoppedOut();
}

bool FlyWithLuaWindow::isFront() {
    return IsWindowInFront();
}

void FlyWithLuaWindow::bringToFront() {
    if (!IsInVR()) {
        BringWindowToFront();
    }
}

bool FlyWithLuaWindow::isVR() {
    return IsInVR();
}

void FlyWithLuaWindow::moveFromOrToVR() {
    moveForVR();
}

void FlyWithLuaWindow::setIsCmdVisible(int fCondition) {
    if (!IsInVR()) {
        isCmdVisible = (fCondition == 0);
    }
}

bool FlyWithLuaWindow::getIsCmdVisible() {
    return isCmdVisible;
}

XPLMWindowID FlyWithLuaWindow::getXWindow() {
    return GetWindowId();
}

void FlyWithLuaWindow::requestInputFocus(bool req) {
    XPLMTakeKeyboardFocus(req ? GetWindowId() : nullptr);
}

bool FlyWithLuaWindow::hasInputFocus() {
    return XPLMHasKeyboardFocus(GetWindowId()) != 0;
}

void FlyWithLuaWindow::boxelsToNative(int x, int y, int &outX, int &outY) {
    outX = x;
    outY = y;
}

} // namespace flwnd

