/*
 *   Floating Windows with imgui integration for FlyWithLua
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *   Released as public domain code.
 *
 */
#ifndef _FLOATINGWINDOW_H_
#define _FLOATINGWINDOW_H_

#include <XPLMGraphics.h>
#include <XPLMDisplay.h>
#include <XPLMDataAccess.h>
#include <cstdint>
#include <vector>
#include <string>
#include <functional>

namespace flwnd {

class FloatingWindow {
public:
    using DrawCallback = std::function<void(FloatingWindow &)>;
    using ClickCallback = std::function<void(FloatingWindow &, int, int, XPLMMouseStatus)>;
    using CloseCallback = std::function<void(FloatingWindow &)>;
    using KeyCallback = std::function<void(FloatingWindow &, char, char, XPLMKeyFlags)>;

    virtual ~FloatingWindow() = default;

    virtual void setTitle(const char *title) = 0;
    virtual void setPosition(int posx, int posy) = 0;
    virtual bool isVisible() const = 0;
    virtual void setVisible(bool visible) = 0;
    virtual void moveFromOrToVR() = 0;
    virtual void requestInputFocus(bool req) = 0;
    virtual bool hasInputFocus() = 0;
    virtual void reportClose() = 0;
    virtual void boxelsToNative(int x, int y, int &outX, int &outY) = 0;

    virtual bool isPopped() = 0;
    virtual bool isFront() = 0;
    virtual void bringToFront() = 0;
    virtual void setResizingLimits(int minwidth, int minheight, int maxwidth, int maxheight) = 0;
    virtual void setPositioningMode(int pmode, int monindex) = 0;
    virtual void setGravity(float inLeftGravity, float inTopGravity, float inRightGravity, float inBottomGravity) = 0;
    virtual void setWindowGeometry(int mleft, int mtop, int mright, int mbot) = 0;
    virtual void setWindowGeometryOS(int mleft, int mtop, int mright, int mbot) = 0;
    virtual void setWindowGeometryVR(int bWidth, int bHeight) = 0;
    virtual bool isVR() = 0;
    virtual void setIsCmdVisible(int fCondition) = 0;
    virtual bool getIsCmdVisible() = 0;

    virtual XPLMWindowID getXWindow() = 0;

    virtual void setDrawCallback(DrawCallback cb) {}
    virtual void setClickCallback(ClickCallback cb) {}
    virtual void setCloseCallback(CloseCallback cb) {}
    virtual void setKeyCallback(KeyCallback cb) {}
};

class LegacyFloatingWindow : public FloatingWindow {
public:
    LegacyFloatingWindow(int winWidth, int winHeight, int winDecoration);
    virtual ~LegacyFloatingWindow();

    void setDrawCallback(DrawCallback cb) override;
    void setClickCallback(ClickCallback cb) override;
    void setCloseCallback(CloseCallback cb) override;
    void setKeyCallback(KeyCallback cb) override;

    void setTitle(const char *title) override;
    void setPosition(int posx, int posy) override;
    bool isVisible() const override;
    void setVisible(bool visible) override;
    void moveFromOrToVR() override;
    void requestInputFocus(bool req) override;
    bool hasInputFocus() override;
    void reportClose() override;
    void boxelsToNative(int x, int y, int &outX, int &outY) override;

    bool isPopped() override;
    bool isFront() override;
    void bringToFront() override;
    void setResizingLimits(int minwidth, int minheight, int maxwidth, int maxheight) override;
    void setPositioningMode(int pmode, int monindex) override;
    void setGravity(float inLeftGravity, float inTopGravity, float inRightGravity, float inBottomGravity) override;
    void setWindowGeometry(int mleft, int mtop, int mright, int mbot) override;
    void setWindowGeometryOS(int mleft, int mtop, int mright, int mbot) override;
    void setWindowGeometryVR(int bWidth, int bHeight) override;
    bool isVR() override;
    void setIsCmdVisible(int fCondition) override;
    bool getIsCmdVisible() override;

    XPLMWindowID getXWindow() override;

protected:
    void updateMatrices();
    virtual void onDraw();
    virtual bool onClick(int x, int y, XPLMMouseStatus status);
    virtual bool onRightClick(int x, int y, XPLMMouseStatus status);
    virtual void onKey(char key, XPLMKeyFlags flags, char virtualKey, bool losingFocus);
    virtual XPLMCursorStatus getCursor(int x, int y);
    virtual bool onMouseWheel(int x, int y, int wheel, int clicks);

private:
    XPLMWindowID window{};
    int width, height, decoration;
    bool isInVR = false;

    bool isCmdVisible = false;

    XPLMDataRef vrEnabledRef{};
    XPLMDataRef modeliewMatrixRef{};
    XPLMDataRef viewportRef{};
    XPLMDataRef projectionMatrixRef{};

    float mModelView[16]{}, mProjection[16]{};
    int mViewport[4]{};

    DrawCallback onDrawCB;
    ClickCallback onClickCB;
    CloseCallback onCloseCB;
    KeyCallback onKeyCB;

    void createWindow();
};

} // namespace flwnd

#endif
