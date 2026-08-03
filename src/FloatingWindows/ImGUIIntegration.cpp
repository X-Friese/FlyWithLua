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
    style.FontSizeBase = 13.0f;

    io.Fonts->AddFontDefaultVector();  // Load embedded scalable font.
    io.Fonts->AddFontDefaultBitmap();  // Load embedded bitmap font (legacy).
    io.Fonts->AddFontDefault();        // Load embedded font (legacy: auto-selected between the two above).

    // Tried to set flag here to improve font scaling but broke it
    // io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

    // Here we load some custom fonts in FlyWithLua folder Custom_Fonts. This should allow you to pick the font yu want to use.
    ImFont* customFont = io.Fonts->AddFontFromFileTTF("./Resources/plugins/FlyWithLua/Custom_Fonts/ProFontWindows.ttf", 13);
    IM_ASSERT(customFont != NULL);
    ImFont* customFont2 = io.Fonts->AddFontFromFileTTF("./Resources/plugins/FlyWithLua/Custom_Fonts/Roboto-Light.ttf", 13);
    IM_ASSERT(customFont2 != NULL);
    ImFont* customFont3 = io.Fonts->AddFontFromFileTTF("./Resources/plugins/FlyWithLua/Custom_Fonts/Roboto-Regular.ttf", 13);
    IM_ASSERT(customFont3 != NULL);

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
