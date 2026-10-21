/*
 *   FlyWithLuaWindow: Modern ImgWindow (Panel Graphics / OpenGL fallback)
 *   subclass for FlyWithLua.
 *
 *   Replaces legacy ImGUIWindow. Automatically handles:
 *   - Panel Graphics (Vulkan/Metal on XP12.4.4+) and OpenGL fallback
 *   - Unified shared font atlas (ImgWindow::sFontAtlas)
 *   - Multi-monitor / high-DPI scaling & coordinate translations
 *   - Native X-Plane VR integration
 */
#ifndef FLOATINGWINDOWS_FLYWITHLUAWINDOW_H_
#define FLOATINGWINDOWS_FLYWITHLUAWINDOW_H_

#include <functional>
#include <string>
#include <memory>
#include "FloatingWindow.h"
#include "ImgWindow.h"

namespace flwnd {

class FlyWithLuaWindow : public FloatingWindow, public ImgWindow {
public:
    using BuildCallback = std::function<void(FlyWithLuaWindow &)>;
    using ErrorHandler = std::function<void(const std::string &)>;

    FlyWithLuaWindow(int width, int height, int decoration);
    virtual ~FlyWithLuaWindow() = default;

    // FloatingWindow interface overrides
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

    void setCloseCallback(CloseCallback cb) override;

    // ImGui builder callbacks
    void setBuildCallback(BuildCallback cb);
    void setErrorHandler(ErrorHandler eh);

    void setImguiCursorHandling(bool enable) {
        mUseImgCursors = enable;
        SetImgCursorHandling(enable);
    }
    bool getImguiCursorHandling() const { return mUseImgCursors; }

    // ImgWindow overrides
    virtual void buildInterface() override;
    virtual ImGuiWindowFlags_ beforeBegin() override;

private:
    BuildCallback doBuild;
    ErrorHandler onError;
    CloseCallback onCloseCB;

    bool stopped = false;
    bool isCmdVisible = false;
    bool mUseImgCursors = true;
};

} // namespace flwnd

#endif // FLOATINGWINDOWS_FLYWITHLUAWINDOW_H_

