/*
 *   Floating Windows with imgui integration for FlyWithLua
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *   Released as public domain code.
 *
 */

/*
 * portions copied with permission from https://github.com/kuroneko/xsb_public
 *
 * ImgWindow.cpp
 *
 * Integration for dear imgui into X-Plane.
 *
 * Copyright (C) 2018, Christopher Collins
 */
#include <XPLMGraphics.h>
#include <XPLMUtilities.h>
#include <cstdint>
#include <cctype>
#include "ImGUIIntegration.h"

// Mirrors source_code/core/ui/imgui_impl_xsystem.cpp:127.
ImGuiKey XPLM_VK_to_ImGuiKey(int vkey) {
    switch (vkey) {
        case XPLM_VK_TAB:       return ImGuiKey_Tab;
        case XPLM_VK_LEFT:      return ImGuiKey_LeftArrow;
        case XPLM_VK_RIGHT:     return ImGuiKey_RightArrow;
        case XPLM_VK_UP:        return ImGuiKey_UpArrow;
        case XPLM_VK_DOWN:      return ImGuiKey_DownArrow;
        case XPLM_VK_HOME:      return ImGuiKey_Home;
        case XPLM_VK_END:       return ImGuiKey_End;
        case XPLM_VK_INSERT:    return ImGuiKey_Insert;
        case XPLM_VK_DELETE:    return ImGuiKey_Delete;
        case XPLM_VK_BACK:      return ImGuiKey_Backspace;
        case XPLM_VK_SPACE:     return ImGuiKey_Space;
        case XPLM_VK_ENTER:     return ImGuiKey_Enter;
        case XPLM_VK_ESCAPE:    return ImGuiKey_Escape;
        case XPLM_VK_0:         return ImGuiKey_0;
        case XPLM_VK_1:         return ImGuiKey_1;
        case XPLM_VK_2:         return ImGuiKey_2;
        case XPLM_VK_3:         return ImGuiKey_3;
        case XPLM_VK_4:         return ImGuiKey_4;
        case XPLM_VK_5:         return ImGuiKey_5;
        case XPLM_VK_6:         return ImGuiKey_6;
        case XPLM_VK_7:         return ImGuiKey_7;
        case XPLM_VK_8:         return ImGuiKey_8;
        case XPLM_VK_9:         return ImGuiKey_9;
        case XPLM_VK_A: return ImGuiKey_A; case XPLM_VK_B: return ImGuiKey_B;
        case XPLM_VK_C: return ImGuiKey_C; case XPLM_VK_D: return ImGuiKey_D;
        case XPLM_VK_E: return ImGuiKey_E; case XPLM_VK_F: return ImGuiKey_F;
        case XPLM_VK_G: return ImGuiKey_G; case XPLM_VK_H: return ImGuiKey_H;
        case XPLM_VK_I: return ImGuiKey_I; case XPLM_VK_J: return ImGuiKey_J;
        case XPLM_VK_K: return ImGuiKey_K; case XPLM_VK_L: return ImGuiKey_L;
        case XPLM_VK_M: return ImGuiKey_M; case XPLM_VK_N: return ImGuiKey_N;
        case XPLM_VK_O: return ImGuiKey_O; case XPLM_VK_P: return ImGuiKey_P;
        case XPLM_VK_Q: return ImGuiKey_Q; case XPLM_VK_R: return ImGuiKey_R;
        case XPLM_VK_S: return ImGuiKey_S; case XPLM_VK_T: return ImGuiKey_T;
        case XPLM_VK_U: return ImGuiKey_U; case XPLM_VK_V: return ImGuiKey_V;
        case XPLM_VK_W: return ImGuiKey_W; case XPLM_VK_X: return ImGuiKey_X;
        case XPLM_VK_Y: return ImGuiKey_Y; case XPLM_VK_Z: return ImGuiKey_Z;
        case XPLM_VK_F1:  return ImGuiKey_F1;  case XPLM_VK_F2:  return ImGuiKey_F2;
        case XPLM_VK_F3:  return ImGuiKey_F3;  case XPLM_VK_F4:  return ImGuiKey_F4;
        case XPLM_VK_F5:  return ImGuiKey_F5;  case XPLM_VK_F6:  return ImGuiKey_F6;
        case XPLM_VK_F7:  return ImGuiKey_F7;  case XPLM_VK_F8:  return ImGuiKey_F8;
        case XPLM_VK_F9:  return ImGuiKey_F9;  case XPLM_VK_F10: return ImGuiKey_F10;
        case XPLM_VK_F11: return ImGuiKey_F11; case XPLM_VK_F12: return ImGuiKey_F12;
        case XPLM_VK_F13: return ImGuiKey_F13; case XPLM_VK_F14: return ImGuiKey_F14;
        case XPLM_VK_F15: return ImGuiKey_F15; case XPLM_VK_F16: return ImGuiKey_F16;
        case XPLM_VK_F17: return ImGuiKey_F17; case XPLM_VK_F18: return ImGuiKey_F18;
        case XPLM_VK_F19: return ImGuiKey_F19; case XPLM_VK_F20: return ImGuiKey_F20;
        case XPLM_VK_F21: return ImGuiKey_F21; case XPLM_VK_F22: return ImGuiKey_F22;
        case XPLM_VK_F23: return ImGuiKey_F23; case XPLM_VK_F24: return ImGuiKey_F24;
        case XPLM_VK_QUOTE:     return ImGuiKey_Apostrophe;
        case XPLM_VK_COMMA:     return ImGuiKey_Comma;
        case XPLM_VK_MINUS:     return ImGuiKey_Minus;
        case XPLM_VK_PERIOD:    return ImGuiKey_Period;
        case XPLM_VK_SLASH:     return ImGuiKey_Slash;
        case XPLM_VK_SEMICOLON: return ImGuiKey_Semicolon;
        case XPLM_VK_EQUAL:     return ImGuiKey_Equal;
        case XPLM_VK_LBRACE:    return ImGuiKey_LeftBracket;
        case XPLM_VK_RBRACE:    return ImGuiKey_RightBracket;
        case XPLM_VK_BACKSLASH: return ImGuiKey_Backslash;
    }
    return ImGuiKey_None;
}

namespace flwnd {

ImGUIWindow::ImGUIWindow(int width, int height, int decoration):
    FloatingWindow(width, height, decoration)
{
    imGuiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(imGuiContext);

    auto &style = ImGui::GetStyle();
    style.WindowRounding = 0;

    auto &io = ImGui::GetIO();
    // io.RenderDrawListsFn = nullptr;

    io.Fonts->AddFontDefaultVector();  // Load embedded scalable font.
    io.Fonts->AddFontDefaultBitmap();  // Load embedded bitmap font (legacy).
    io.Fonts->AddFontDefault();        // Load embedded font (legacy: auto-selected between the two above).

    // Tried to set flag here to improve font scaling but broke it
    // For this to work need to stop using legacy backend in part
    // means removing Glut helper.
    // io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

    // Here we load some custom fonts in FlyWithLua folder Custom_Fonts. This should allow you to pick the font you want to use.
    customFont1 = io.Fonts->AddFontFromFileTTF("./Resources/plugins/FlyWithLua/Custom_Fonts/ProFontWindows.ttf", 13);
    IM_ASSERT(customFont1 != NULL);
    customFont2 = io.Fonts->AddFontFromFileTTF("./Resources/plugins/FlyWithLua/Custom_Fonts/ProFontWindows.ttf", 16);
    IM_ASSERT(customFont2 != NULL);
    customFont3 = io.Fonts->AddFontFromFileTTF("./Resources/plugins/FlyWithLua/Custom_Fonts/ProFontWindows.ttf", 20);
    IM_ASSERT(customFont3 != NULL);

    customFont4 = io.Fonts->AddFontFromFileTTF("./Resources/plugins/FlyWithLua/Custom_Fonts/Roboto-Light.ttf", 13);
    IM_ASSERT(customFont4 != NULL);
    customFont5 = io.Fonts->AddFontFromFileTTF("./Resources/plugins/FlyWithLua/Custom_Fonts/Roboto-Light.ttf", 16);
    IM_ASSERT(customFont5 != NULL);
    customFont6 = io.Fonts->AddFontFromFileTTF("./Resources/plugins/FlyWithLua/Custom_Fonts/Roboto-Light.ttf", 20);
    IM_ASSERT(customFont6 != NULL);

    customFont7 = io.Fonts->AddFontFromFileTTF("./Resources/plugins/FlyWithLua/Custom_Fonts/Roboto-Regular.ttf", 13);
    IM_ASSERT(customFont7 != NULL);
    customFont8 = io.Fonts->AddFontFromFileTTF("./Resources/plugins/FlyWithLua/Custom_Fonts/Roboto-Regular.ttf", 16);
    IM_ASSERT(customFont8 != NULL);
    customFont9 = io.Fonts->AddFontFromFileTTF("./Resources/plugins/FlyWithLua/Custom_Fonts/Roboto-Regular.ttf", 20);
    IM_ASSERT(customFont9 != NULL);

    io.IniFilename = nullptr;
    // io.OptMacOSXBehaviors = false;
    // disable OSX-like keyboard behaviours always - we don't have the keymapping for it.
    io.ConfigMacOSXBehaviors = false; // This is the new version above Imgui 1.60
    io.ConfigFlags = ImGuiConfigFlags_NavNoCaptureKeyboard;

    uint8_t *pixels;
    int fontTexWidth, fontTexHeight;
    io.Fonts->GetTexDataAsAlpha8(&pixels, &fontTexWidth, &fontTexHeight);

    int textureId;
    XPLMGenerateTextureNumbers(&textureId, 1);
    fontTextureId = (GLuint) textureId;

    XPLMBindTexture2d(fontTextureId, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, fontTexWidth, fontTexHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);
    io.Fonts->TexID = (ImTextureID)(intptr_t)(fontTextureId);
}

void ImGUIWindow::setBuildCallback(BuildCallback cb) {
    doBuild = cb;
}

void ImGUIWindow::setErrorHandler(ErrorHandler eh) {
    onError = eh;
}

void ImGUIWindow::onDraw() {
    if (stopped) {
        return;
    }

    updateMatrices();
    try {
        buildGUI();
        showGUI();
    } catch (const std::exception &e) {
        if (onError) {
            onError(e.what());
        }
        stopped = true;
    }

    ImGui::SetCurrentContext(imGuiContext);
    auto &io = ImGui::GetIO();
    bool hasKeyboardFocus = hasInputFocus();
    if (io.WantTextInput && !hasKeyboardFocus) {
        requestInputFocus(true);
    } else if (!io.WantTextInput && hasKeyboardFocus) {
        requestInputFocus(false);
    }

    FloatingWindow::onDraw();
}

void ImGUIWindow::buildGUI() {
    ImGui::SetCurrentContext(imGuiContext);
    auto &io = ImGui::GetIO();

    // transfer the window geometry to ImGui
    XPLMGetWindowGeometry(getXWindow(), &mLeft, &mTop, &mRight, &mBottom);

    float win_width = static_cast<float>(mRight - mLeft);
    float win_height = static_cast<float>(mTop - mBottom);

    io.DisplaySize = ImVec2(win_width, win_height);
    // in boxels, we're always scale 1, 1.
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2((float) 0.0, (float) 0.0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(win_width, win_height), ImGuiCond_Always);

    // and construct the window
    // This is the original Begin that does not have horizontal scrollbars.
    // I am keeping it here to make sure adding them does not cause and issuew
    // ImGui::Begin("FlyWithLua", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    // and construct the window with horizontal scrollbar
    // Still not sure if this will cause any issues with tables or collums.
    ImGuiWindowFlags fwl_imgui_wnd_flags = ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_HorizontalScrollbar;
    ImGui::Begin("FlyWithLua", nullptr, fwl_imgui_wnd_flags);

    if (doBuild) {
        doBuild(*this);
    }
    ImGui::End();

    ImGui::Render();
}

void ImGUIWindow::showGUI() {
    ImGui::SetCurrentContext(imGuiContext);
    auto &io = ImGui::GetIO();

    ImDrawData *drawData = ImGui::GetDrawData();

    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    drawData->ScaleClipRects(io.DisplayFramebufferScale);

    // We are using the OpenGL fixed pipeline because messing with the
    // shader-state in X-Plane is not very well documented, but using the fixed
    // function pipeline is.

    // 1TU + Alpha settings, no depth, no fog.
    XPLMSetGraphicsState(0, 1, 0, 1, 1, 0, 0);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
    glDisable(GL_CULL_FACE);
    glEnable(GL_SCISSOR_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_TEXTURE_2D);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glScalef(1.0f, -1.0f, 1.0f);
    glTranslatef(static_cast<GLfloat>(mLeft), static_cast<GLfloat>(-mTop), 0.0f);

    // Render command lists
    for (int n = 0; n < drawData->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = drawData->CmdLists[n];
        const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
        const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;
        // pre v190
        // glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, pos)));
        // glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, uv)));
        // glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, col)));
        glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + offsetof(ImDrawVert, pos)));
        glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + offsetof(ImDrawVert, uv)));
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + offsetof(ImDrawVert, col)));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback) {
                pcmd->UserCallback(cmd_list, pcmd);
            } else {
                // pre v190
                // glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                XPLMBindTexture2d((int)(intptr_t)pcmd->GetTexID(), 0);

                // Scissors work in viewport space - must translate the coordinates from ImGui -> Boxels, then Boxels -> Native.
                //FIXME: it must be possible to apply the scale+transform manually to the projection matrix so we don't need to doublestep.
                int bTop, bLeft, bRight, bBottom;
                translateImguiToBoxel(pcmd->ClipRect.x, pcmd->ClipRect.y, bLeft, bTop);
                translateImguiToBoxel(pcmd->ClipRect.z, pcmd->ClipRect.w, bRight, bBottom);
                int nTop, nLeft, nRight, nBottom;
                boxelsToNative(bLeft, bTop, nLeft, nTop);
                boxelsToNative(bRight, bBottom, nRight, nBottom);
                glScissor(nLeft, nBottom, nRight-nLeft, nTop-nBottom);
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
            }
            idx_buffer += pcmd->ElemCount;
        }
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    // Restore modified state
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindTexture(GL_TEXTURE_2D, 0);
    glPopAttrib();
    glPopClientAttrib();
}

bool ImGUIWindow::onClick(int x, int y, XPLMMouseStatus status) {
    ImGui::SetCurrentContext(imGuiContext);
    ImGuiIO& io = ImGui::GetIO();

    float outX, outY;
    translateToImguiSpace(x, y, outX, outY);
    io.MousePos = ImVec2(outX, outY);

    switch (status) {
    case xplm_MouseDown:
    case xplm_MouseDrag:
        io.MouseDown[0] = true;
        break;
    case xplm_MouseUp:
        io.MouseDown[0] = false;
        break;
    }

    return FloatingWindow::onClick(x, y, status);
}

bool ImGUIWindow::onMouseWheel(int x, int y, int wheel, int clicks) {
    ImGui::SetCurrentContext(imGuiContext);
    ImGuiIO& io = ImGui::GetIO();

    float outX, outY;
    translateToImguiSpace(x, y, outX, outY);

    io.MousePos = ImVec2(outX, outY);
    switch (wheel) {
    case 0:
        io.MouseWheel = static_cast<float>(clicks);
        break;
    case 1:
        io.MouseWheelH = static_cast<float>(clicks);
        break;
    }

    return FloatingWindow::onMouseWheel(x, y, wheel, clicks);
}

XPLMCursorStatus ImGUIWindow::getCursor(int x, int y) {
    ImGui::SetCurrentContext(imGuiContext);
    ImGuiIO& io = ImGui::GetIO();

    float outX, outY;
    translateToImguiSpace(x, y, outX, outY);
    io.MousePos = ImVec2(outX, outY);

    return xplm_CursorDefault;
}

// This is the original I will be keeping till sure working fine
#if 0
void ImGUIWindow::onKey(char key, XPLMKeyFlags flags, char virtualKey, bool losingFocus) {
    if (losingFocus) {
        return;
    }

    ImGui::SetCurrentContext(imGuiContext);
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantTextInput) {
        // If you press and hold a key, the flags will actually be down, 0, 0, ..., up
        // So the key always has to be considered as pressed unless the up flag is set
        auto vk = static_cast<ImGuiKey>(virtualKey);
        io.AddKeyEvent(vk, (flags & xplm_UpFlag) != xplm_UpFlag);
        io.KeyShift = (flags & xplm_ShiftFlag) == xplm_ShiftFlag;
        io.KeyAlt = (flags & xplm_OptionAltFlag) == xplm_OptionAltFlag;
        io.KeyCtrl = (flags & xplm_ControlFlag) == xplm_ControlFlag;

        if ((flags & xplm_UpFlag) != xplm_UpFlag
            && !io.KeyCtrl
            && !io.KeyAlt
            && std::isprint(key)) {
            char smallStr[] = { key, 0 };
            io.AddInputCharactersUTF8(smallStr);
        }
    }

    buildGUI();

    FloatingWindow::onKey(key, flags, virtualKey, losingFocus);
}
#endif

// This is from xlua and just trying to find a way to make it work
void ImGUIWindow::onKey(char key, XPLMKeyFlags flags, char vkey, bool losing_focus) {
    ImGui::SetCurrentContext(imGuiContext);
    auto& io = ImGui::GetIO();

    // Losing-focus notification: drop pressed-key state so a key held while
    // focus left this window doesn't stay "down" inside ImGui forever.
    if (losing_focus) {
        io.ClearInputKeys();
        io.AddKeyEvent(ImGuiMod_Shift, false);
        io.AddKeyEvent(ImGuiMod_Ctrl,  false);
        io.AddKeyEvent(ImGuiMod_Alt,   false);
        has_focus_ = false;
        return;
    }

    io.AddKeyEvent(ImGuiMod_Shift, (flags & xplm_ShiftFlag) != 0);
    io.AddKeyEvent(ImGuiMod_Ctrl,  (flags & xplm_ControlFlag) != 0);
    io.AddKeyEvent(ImGuiMod_Alt,   (flags & xplm_OptionAltFlag) != 0);

    // XPLM dispatches three flavours of key callback:
    //   Down   = initial press        (xplm_DownFlag)
    //   Up     = release              (xplm_UpFlag)
    //   Repeat = OS auto-repeat       (neither flag set; gfx_window_cocoa.mm:452
    //            deliberately strips xplm_DownFlag for isARepeat events)
    const bool is_down   = (flags & xplm_DownFlag) != 0;
    const bool is_up     = (flags & xplm_UpFlag)   != 0;
    const bool is_repeat = !is_down && !is_up;

    const ImGuiKey ik = XPLM_VK_to_ImGuiKey(static_cast<unsigned char>(vkey));
    if (ik != ImGuiKey_None) {
        if (is_down)        io.AddKeyEvent(ik, true);
        else if (is_up)     io.AddKeyEvent(ik, false);
        // Repeat: leave the key state alone. The initial Down already told
        // ImGui the key is held; its own KeyRepeatDelay/Rate timers drive
        // non-text repeat (Backspace, arrows, Delete in InputText).
    }
    // Forward typed characters on Down AND Repeat so InputText sees the
    // OS-auto-repeated 'aaaaa…' stream when a printable key is held.
    const unsigned char ukey = static_cast<unsigned char>(key);
    if ((is_down || is_repeat) && ukey >= 0x20 && ukey < 0x7f) {
        io.AddInputCharacter(static_cast<unsigned int>(ukey));
    }
}


void ImGUIWindow::translateImguiToBoxel(float inX, float inY, int& outX, int& outY) {
    outX = (int)(mLeft + inX);
    outY = (int)(mTop - inY);
}

void ImGUIWindow::translateToImguiSpace(int inX, int inY, float& outX, float& outY) {
    outX = static_cast<float>(inX - mLeft);
    if (outX < 0.0f || outX > (float)(mRight - mLeft)) {
        outX = -FLT_MAX;
        outY = -FLT_MAX;
        return;
    }
    outY = static_cast<float>(mTop - inY);
    if (outY < 0.0f || outY > (float)(mTop - mBottom)) {
        outX = -FLT_MAX;
        outY = -FLT_MAX;
        return;
    }
}

ImGUIWindow::~ImGUIWindow() {
    ImGui::DestroyContext(imGuiContext);
    glDeleteTextures(1, &fontTextureId);
}

} /* namespace flwnd */
