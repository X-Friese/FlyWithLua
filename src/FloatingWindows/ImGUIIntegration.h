/*
 *   Floating Windows with imgui integration for FlyWithLua
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *   Released as public domain code.
 *
 */

/*
 * portions copied with permission from https://github.com/kuroneko/xsb_public
 *
 * ImgWindow.h
 *
 * Integration for dear imgui into X-Plane.
 *
 * Copyright (C) 2018, Christopher Collins
 */
#ifndef FLOATINGWINDOWS_IMGUIINTEGRATION_H_
#define FLOATINGWINDOWS_IMGUIINTEGRATION_H_


#include <memory>
#include <string>
#include "FloatingWindow.h"
#include "FlyWithLua.h"
#include "imgui/imgui.h"
#include "lua.hpp"

// Not sure why this will not work but will leave it here
// For some visability
// I had to add inline to get rid of error
inline ImFont* customFont1 = nullptr;
inline ImFont* customFont2 = nullptr;
inline ImFont* customFont3 = nullptr;


namespace flwnd {

class ImGUIWindow: public FloatingWindow {
public:
    using BuildCallback = std::function<void(ImGUIWindow &)>;
    using ErrorHandler = std::function<void(const std::string &)>;

    ImGUIWindow(int width, int height, int decoration);
    void setErrorHandler(ErrorHandler eh);
    void setBuildCallback(BuildCallback cb);
    ~ImGUIWindow();
protected:
    void onDraw() override;
    bool onClick(int x, int y, XPLMMouseStatus status) override;
    bool onMouseWheel(int x, int y, int wheel, int clicks) override;
    XPLMCursorStatus getCursor(int x, int y) override;
    void onKey(char key, XPLMKeyFlags flags, char virtualKey, bool losingFocus) override;
private:
    GLuint fontTextureId{};
    ImGuiContext *imGuiContext{};
    int mLeft{}, mTop{}, mRight{}, mBottom{};
    ErrorHandler onError;
    BuildCallback doBuild;
    bool stopped = false;
    bool has_focus_ = false;     // last-known result of XPLMTakeKeyboardFocus

    void buildGUI();
    void showGUI();

    void translateImguiToBoxel(float inX, float inY, int &outX, int &outY);
    void translateToImguiSpace(int inX, int inY, float &outX, float &outY);
};

} /* namespace flwnd */

#endif /* FLOATINGWINDOWS_IMGUIINTEGRATION_H_ */
