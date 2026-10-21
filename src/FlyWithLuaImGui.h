/*
 *   ImGui lifecycle (shared ImgFontAtlas) for FlyWithLua.
 *
 *   InitializeImGui() builds the single, plugin-wide font atlas that every
 *   ImgWindow shares (ImgWindow::sFontAtlas); TeardownImGui() destroys it
 *   following the mandatory ImgWindow shutdown sequence. See
 *   ImgWindow/docs/Basic-Usage-Guide.md.
 */
#ifndef FLYWITHLUA_IMGUI_H_
#define FLYWITHLUA_IMGUI_H_

#include <string>
#include <vector>
#include "imgui/imgui.h"

// Legacy global font pointers preserved for backward compatibility
extern ImFont* customFont1;
extern ImFont* customFont2;
extern ImFont* customFont3;
extern ImFont* customFont4;
extern ImFont* customFont5;
extern ImFont* customFont6;
extern ImFont* customFont7;
extern ImFont* customFont8;
extern ImFont* customFont9;

namespace flywithlua {

/**
 * Retrieve a registered font by name (case-insensitive).
 *
 * Supported names include:
 *   - Built-ins: "default", "ProggyClean", "ProggyForever"
 *   - Bundled/Custom: "Roboto-Light-13", "Roboto-Regular-16", "Roboto-Medium-20", etc.
 *   - Base names: "Roboto-Light", "Roboto-Regular", "Roboto-Medium" (resolves to primary size)
 *   - Legacy aliases: "Font1-1", "Font1", "1" .. "9"
 *
 * @param name The font name or alias
 * @return Pointer to ImFont, or nullptr if not registered
 */
ImFont* GetFontByName(const std::string& name);

/**
 * Retrieve a legacy font by 1-based index (1..9).
 *
 * @param index 1 through 9
 * @return Pointer to ImFont, or nullptr if index out of range or unassigned
 */
ImFont* GetFontByIndex(int index);

/**
 * Retrieve the primary default font from the atlas.
 *
 * @return Default ImFont*, or nullptr if atlas not initialized
 */
ImFont* GetDefaultFont();

/**
 * Get a list of all registered canonical font names.
 *
 * @return Vector of registered font names
 */
std::vector<std::string> GetRegisteredFontNames();

/**
 * Clear the font registry map and legacy lookup tables.
 */
void ClearFontRegistry();

} // namespace flywithlua

/**
 * Create the shared font atlas, load all FlyWithLua fonts into it (default
 * fonts plus customFont1..9) and publish it as ImgWindow::sFontAtlas.
 *
 * Must run after fwl_prefs.ini has been read (font sizes come from there) and
 * before the first ImgWindow is created. Safe to call when already initialized.
 *
 * @return false if the atlas could not be created.
 */
bool InitializeImGui();

/**
 * Destroy everything InitializeImGui() created. Closes all floating windows
 * first, then severs ImGui's font pointer, releases the atlas and flushes the
 * deferred texture deletion queue. Safe to call when not initialized.
 */
void TeardownImGui();

#endif /* FLYWITHLUA_IMGUI_H_ */

