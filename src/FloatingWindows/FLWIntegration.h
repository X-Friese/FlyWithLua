/*
 *   Floating Windows with imgui integration for FlyWithLua
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *   Released as public domain code.
 *
 */
#ifndef FLOATINGWINDOWS_FLWINTEGRATION_H_
#define FLOATINGWINDOWS_FLWINTEGRATION_H_

#include "lua.hpp"
#include "imgui/imgui.h"

// globals for imgui lua bindings
extern lua_State* lState;
void LoadImguiBindings();

namespace flwnd {

void initFloatingWindowSupport();
void deinitFloatingWindowSupport();
void onFlightLoop();
bool FindAndQuarantine (lua_State *L);
void SetGlobalFontFromFileTTF(const char *path, float size_pixels,
                              float spacing_x, float spacing_y, float oversample_x, float oversample_y);

}

#endif /* FLOATINGWINDOWS_FLWINTEGRATION_H_ */
