/*
 *   Floating Windows with imgui integration for FlyWithLua
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *   Released as public domain code.
 *
 */
#ifndef FLOATINGWINDOWS_IMGUIINTEGRATION_H_
#define FLOATINGWINDOWS_IMGUIINTEGRATION_H_

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include <memory>
#include <string>
#include "FloatingWindow.h"
#include "imgui/imgui.h"
#include "lua.hpp"

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

    void buildGUI();
    void showGUI();

    void translateImguiToBoxel(float inX, float inY, int &outX, int &outY);
    void translateToImguiSpace(int inX, int inY, float &outX, float &outY);
};

} /* namespace flwnd */

#endif /* FLOATINGWINDOWS_IMGUIINTEGRATION_H_ */
