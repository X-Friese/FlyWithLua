/*
 *   FWLDocWindow: Turnkey X-Plane Markdown Manual & Documentation Window for FlyWithLua.
 *
 *   Subclasses flwnd::FloatingWindow and ImgDocWindow (ImgWindow/contrib/ImgDocWindow.h)
 *   so Lua scripts can instantiate full-featured in-sim documentation windows with:
 *   - Two-row 1.0x header chrome + Nx zoomed ([50%..250%]) Markdown viewport
 *   - Automatic ATX Table-of-Contents section jumper & 1-click scroll return
 *   - Floated Retina figures, captions, and zoom/pan click-to-enlarge lightboxes
 *   - Glass HUD transparency + opacity slider & 5 curated color themes
 *   - Proportional zoom/[Fit] window resizing, Pop-Out <-> In-Sim <-> VR transitions,
 *     and automatic mtime disk hot-reloading
 *   - Full compatibility with standard float_wnd_* lifecycle & Lua reload cleanup
 */
#ifndef FLOATINGWINDOWS_FWLDOCWINDOW_H_
#define FLOATINGWINDOWS_FWLDOCWINDOW_H_

#include "FloatingWindow.h"
#include "../FlyWithLua.h"
#include "../FlyWithLuaImGui.h"
#include "contrib/ImgDocWindow.h"
#include <XPLMDisplay.h>

namespace flwnd {

#if defined(IMGUI_DOC_VIEWER_H)

void SyncBuiltInDocMenuCheckmarks();

class FWLDocWindow : public FloatingWindow, public ImgDocWindow {
public:
    FWLDocWindow(
        const std::string& windowTitle,
        int width = kDefaultWidth,
        int height = kDefaultHeight,
        int decoration = xplm_WindowDecorationSelfDecoratedResizable,
        bool cursors = (flywithlua::imgui_mouse_cursors != 0))
        : ImgDocWindow(
              windowTitle,
              MakeDefaultDocGeometry(width, height),
              static_cast<XPLMWindowDecoration>(decoration),
              xplm_WindowLayerFloatingWindows,
              cursors)
    {
        SetVisible(true);
        SetCaptureKeyboardOnMouseFocus(true);
    }

    ~FWLDocWindow() override = default;

    // Resolve paths relative to calling Lua script directory first, then FlyWithLua plugin root
    bool LoadMarkdownFileWithScriptDir(const std::string& pathOrRel, const std::string& scriptDir)
    {
        if (!scriptDir.empty() && !pathOrRel.empty()) {
            std::string cand = JoinScriptPath(scriptDir, pathOrRel);
            if (LoadMarkdownFile(cand)) {
                return true;
            }
        }
        return LoadMarkdownFile(pathOrRel);
    }

    void SetCompanionPdfWithScriptDir(const std::string& pdfPathOrUrl, const std::string& scriptDir)
    {
        if (pdfPathOrUrl.empty()) {
            SetCompanionPdfPath("");
            return;
        }
        if (!scriptDir.empty() &&
            pdfPathOrUrl.compare(0, 7, "http://") != 0 &&
            pdfPathOrUrl.compare(0, 8, "https://") != 0 &&
            pdfPathOrUrl.compare(0, 7, "file://") != 0)
        {
            std::string cand = JoinScriptPath(scriptDir, pdfPathOrUrl);
            struct stat st;
            if (stat(cand.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {
                SetCompanionPdfPath(cand);
                return;
            }
        }
        SetCompanionPdfPath(pdfPathOrUrl);
    }

    // FloatingWindow interface overrides
    void setTitle(const char* title) override {
        std::string t = title ? title : "";
        SetWindowTitle(t);
        GetViewerConfig().title = nullptr;
    }

    void setPosition(int posx, int posy) override {
        if (!IsInVR()) {
            int winLeft = 0, winTop = 0, winRight = 0, winBot = 0;
            XPLMGetScreenBoundsGlobal(&winLeft, &winTop, &winRight, &winBot);
            int curLeft = 0, curTop = 0, curRight = 0, curBottom = 0;
            GetWindowGeometry(curLeft, curTop, curRight, curBottom);
            int w = curRight - curLeft;
            int h = curTop - curBottom;
            SetWindowGeometry(winLeft + posx, winBot + posy + h, winLeft + posx + w, winBot + posy);
        }
    }

    bool isVisible() const override { return GetVisible(); }
    void setVisible(bool visible) override { SetVisible(visible); }
    void moveFromOrToVR() override { moveForVR(); }
    void requestInputFocus(bool req) override {
        SetExplicitKeyboardFocus(req);
        XPLMTakeKeyboardFocus(req ? GetWindowId() : nullptr);
    }
    bool hasInputFocus() override { return XPLMHasKeyboardFocus(GetWindowId()) != 0; }

    void reportClose() override {
        if (mOnCloseCB) {
            mOnCloseCB(*this);
        }
    }

    void boxelsToNative(int x, int y, int& outX, int& outY) override {
        outX = x;
        outY = y;
    }

    bool isPopped() override { return IsPoppedOut(); }
    bool isFront() override { return IsWindowInFront(); }
    void bringToFront() override {
        if (!IsInVR()) {
            BringWindowToFront();
        }
    }

    void setResizingLimits(int minwidth, int minheight, int maxwidth, int maxheight) override {
        SetWindowResizingLimits(minwidth, minheight, maxwidth, maxheight);
    }

    void setPositioningMode(int pmode, int monindex) override {
        SetWindowPositioningMode(static_cast<XPLMWindowPositioningMode>(pmode), monindex);
    }

    void setGravity(float inLeftGravity, float inTopGravity, float inRightGravity, float inBottomGravity) override {
        if (!IsInVR()) {
            XPLMSetWindowGravity(GetWindowId(), inLeftGravity, inTopGravity, inRightGravity, inBottomGravity);
        }
    }

    void setWindowGeometry(int mleft, int mtop, int mright, int mbot) override {
        if (!IsInVR()) {
            SetWindowGeometry(mleft, mtop, mright, mbot);
        }
    }

    void setWindowGeometryOS(int mleft, int mtop, int mright, int mbot) override {
        if (!IsInVR() && IsPoppedOut()) {
            SetWindowGeometryOS(mleft, mtop, mright, mbot);
        }
    }

    void setWindowGeometryVR(int bWidth, int bHeight) override {
        if (IsInVR()) {
            SetWindowGeometryVR(bWidth, bHeight);
        }
    }

    bool isVR() override { return IsInVR(); }

    void setIsCmdVisible(int fCondition) override {
        if (!IsInVR()) {
            mIsCmdVisible = (fCondition == 0);
        }
    }

    bool getIsCmdVisible() override { return mIsCmdVisible; }

    XPLMWindowID getXWindow() override { return GetWindowId(); }

    void setCloseCallback(CloseCallback cb) override { mOnCloseCB = cb; }

    void setImguiCursorHandling(bool enabled) {
        GetViewerState().dynamic_cursors_enabled = enabled;
        SetImgCursorHandling(enabled);
    }

protected:
    ImGuiWindowFlags_ beforeBegin() override {
        ImGuiIO& io = ImGui::GetIO();
        if (!io.FontDefault) {
            ImFont* reg = flywithlua::GetFontByName("Roboto-Regular-16");
            if (!reg) reg = flywithlua::GetFontByName("Roboto-Regular");
            if (reg) io.FontDefault = reg;
        }
        io.ConfigDebugHighlightIdConflicts = (flywithlua::imgui_debug_popup == 1);
        ImGuiWindowFlags_ flags = ImgDocWindow::beforeBegin();
        enforceSolidOpaqueWhenNotTransparent();
        return flags;
    }

    void onDocumentThemeChanged(int /*themeIndex*/, bool /*transparentHud*/, float /*opacity*/) override {
        enforceSolidOpaqueWhenNotTransparent();
    }

    void onDocumentClosed() override {
        reportClose();
        SyncBuiltInDocMenuCheckmarks();
    }

private:
    void enforceSolidOpaqueWhenNotTransparent() {
        // Upstream ApplyDocumentViewerTheme leaves stock Dear ImGui theme WindowBg.w (< 1.0f,
        // e.g. 0.94f in Dark, 0.85f in Classic, 0.96f in Charcoal/Rusty) when transparent_hud
        // is false. Force 100% solid WindowBg alpha whenever Transparent HUD is OFF while
        // preserving GetViewerState().window_opacity untouched for the slider.
        if (!GetTransparentHud() || !IsSelfDecorated()) {
            ImGuiStyle& st = ImGui::GetStyle();
            st.Colors[ImGuiCol_WindowBg].w = 1.0f;
            st.Colors[ImGuiCol_Border].w   = 1.0f;
        }
    }
    static WindowGeometry MakeDefaultDocGeometry(int width, int height) {
        int sl = 0, st = 0;
        XPLMGetScreenBoundsGlobal(&sl, &st, nullptr, nullptr);
        return WindowGeometry::FromTopLeft(sl + 120, st - 90, width, height);
    }

    static std::string JoinScriptPath(const std::string& dir, const std::string& rel) {
        if (dir.empty()) return rel;
        char back = dir[dir.size() - 1];
        if (back == '/' || back == '\\') return dir + rel;
        return dir + "/" + rel;
    }

    CloseCallback mOnCloseCB;
    bool mIsCmdVisible = false;
};

#endif // defined(IMGUI_DOC_VIEWER_H)

} // namespace flwnd

#endif // FLOATINGWINDOWS_FWLDOCWINDOW_H_
