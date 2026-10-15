/*
 *   ImGui lifecycle (shared ImgFontAtlas) for FlyWithLua.
 *
 *   See FlyWithLuaImGui.h and ImgWindow/docs/Basic-Usage-Guide.md.
 */
#include <fstream>
#include <string>

#include "FlyWithLua.h"
#include "FlyWithLuaImGui.h"
#include "FloatingWindows/FLWIntegration.h"
#include "ImgWindow.h"

ImFont* customFont1 = nullptr;
ImFont* customFont2 = nullptr;
ImFont* customFont3 = nullptr;
ImFont* customFont4 = nullptr;
ImFont* customFont5 = nullptr;
ImFont* customFont6 = nullptr;
ImFont* customFont7 = nullptr;
ImFont* customFont8 = nullptr;
ImFont* customFont9 = nullptr;

namespace {

const char* const kCustomFontDir = "./Resources/plugins/FlyWithLua/Custom_Fonts/";

// Load one TTF from the FlyWithLua Custom_Fonts folder into the shared atlas.
// A missing/unreadable font yields nullptr (imgui_push_font() already ignores
// null fonts) instead of asserting inside ImGui.
ImFont* LoadCustomFont(const char* fileName, float sizePixels)
{
    const std::string path = std::string(kCustomFontDir) + fileName;
    if (!std::ifstream(path))
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Custom font not found: " + path);
        return nullptr;
    }
    ImFont* font = ImgWindow::sFontAtlas->AddFontFromFileTTF(path.c_str(), sizePixels);
    if (font == nullptr)
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Could not load custom font: " + path);
    }
    return font;
}

} // namespace

bool InitializeImGui()
{
    // Exactly one atlas for the whole plugin. If one is already published
    // (e.g. Enable without a matching Disable) leave it alone: adding the fonts
    // a second time would only duplicate them.
    if (ImgWindow::sFontAtlas)
    {
        return true;
    }

    ImgWindow::sFontAtlas = std::make_shared<ImgFontAtlas>();
    if (!ImgWindow::sFontAtlas || !ImgWindow::sFontAtlas->getAtlas())
    {
        flywithlua::logMsg(logToDevCon, "FlyWithLua Error: Could not create the shared ImGui font atlas.");
        ImgWindow::sFontAtlas.reset();
        return false;
    }

    ImFontAtlas* atlas = ImgWindow::sFontAtlas->getAtlas();

    // Same default fonts, in the same order, as the legacy per-window setup.
    atlas->AddFontDefaultVector();                // Load embedded scalable font.
    atlas->AddFontDefaultBitmap();                // Load embedded bitmap font (legacy).
    ImgWindow::sFontAtlas->AddFontDefault();      // Load embedded font (legacy: auto-selected between the two above).

    const float font_size1 = static_cast<float>(flywithlua::imgui_font_size1);
    const float font_size2 = static_cast<float>(flywithlua::imgui_font_size2);
    const float font_size3 = static_cast<float>(flywithlua::imgui_font_size3);

    // Here we load some custom fonts in FlyWithLua folder Custom_Fonts. This should allow you to pick the font you want to use.
    customFont1 = LoadCustomFont("ProFontWindows.ttf", font_size1);
    customFont2 = LoadCustomFont("ProFontWindows.ttf", font_size2);
    customFont3 = LoadCustomFont("ProFontWindows.ttf", font_size3);

    customFont4 = LoadCustomFont("Roboto-Light.ttf", font_size1);
    customFont5 = LoadCustomFont("Roboto-Light.ttf", font_size2);
    customFont6 = LoadCustomFont("Roboto-Light.ttf", font_size3);

    customFont7 = LoadCustomFont("Roboto-Regular.ttf", font_size1);
    customFont8 = LoadCustomFont("Roboto-Regular.ttf", font_size2);
    customFont9 = LoadCustomFont("Roboto-Regular.ttf", font_size3);

    // Note: the GPU upload happens lazily, when the first window draws.
    return true;
}

void TeardownImGui()
{
    // Step 0: close every floating window first, so none can draw against a dead atlas.
    flwnd::deinitFloatingWindowSupport();

    // Step 1: delete custom loaded textures (ImgWindow::DeleteTexture()).
    // flwnd::deinitFloatingWindowSupport() in Step 0 queues all tracked ImTextureIDs
    // for deletion via ImgWindow::DeleteTexture(), which is flushed to VRAM in Step 4.

    // The custom font pointers belong to the atlas that is about to go away.
    customFont1 = customFont2 = customFont3 = nullptr;
    customFont4 = customFont5 = customFont6 = nullptr;
    customFont7 = customFont8 = customFont9 = nullptr;

#if defined(IMGUI_VERSION_NUM) && (IMGUI_VERSION_NUM >= 19200)
    // Step 2: CRITICAL - sever ImGui's internal reference FIRST, while the atlas is still
    // alive. Reversing steps 2 and 3 leaves io.Fonts dangling (use-after-free).
    if (ImGui::GetCurrentContext() != nullptr)
    {
        ImGui::GetIO().Fonts = NULL;
    }
#endif

    // Step 3: release the shared pointer, which deletes the atlas.
    if (ImgWindow::sFontAtlas)
    {
        ImgWindow::sFontAtlas.reset();
    }

    // Step 4: flush the deferred deletion queue so the atlas (and any custom
    // textures) are purged from VRAM now; no further flight loop will do it.
    ImgWindow::Shutdown();
}

