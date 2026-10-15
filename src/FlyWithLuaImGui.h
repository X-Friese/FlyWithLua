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

#include "imgui/imgui.h"

extern ImFont* customFont1;
extern ImFont* customFont2;
extern ImFont* customFont3;
extern ImFont* customFont4;
extern ImFont* customFont5;
extern ImFont* customFont6;
extern ImFont* customFont7;
extern ImFont* customFont8;
extern ImFont* customFont9;

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

