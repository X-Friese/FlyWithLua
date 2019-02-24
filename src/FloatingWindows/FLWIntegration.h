/*
 *   Floating Windows with imgui integration for FlyWithLua
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *   Released as public domain code.
 *
 */
#ifndef FLOATINGWINDOWS_FLWINTEGRATION_H_
#define FLOATINGWINDOWS_FLWINTEGRATION_H_

#include "lua.hpp"

// globals for imgui lua bindings
extern lua_State* lState;
void LoadImguiBindings();

namespace flwnd {

void initFloatingWindowSupport();
void deinitFloatingWindowSupport();
void onFlightLoop();
bool FindAndQuarantine (lua_State *L);

}

#endif /* FLOATINGWINDOWS_FLWINTEGRATION_H_ */
