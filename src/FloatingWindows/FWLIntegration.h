/*
 *   Floating Windows with imgui integration for FlyWithLua
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *   Released as public domain code.
 *
 */
#ifndef FLOATINGWINDOWS_FWLINTEGRATION_H_
#define FLOATINGWINDOWS_FWLINTEGRATION_H_

#include <memory>
#include "lua.hpp"

// globals for imgui lua bindings
extern lua_State* lState;
void LoadImguiBindings();

namespace flwnd {

class FloatingWindow;

void initFloatingWindowSupport();
void deinitFloatingWindowSupport();
void onFlightLoop();
bool FindAndQuarantine (lua_State *L);
FloatingWindow* GetValidFloatingWindow(void* ptr);
void RegisterManagedFloatingWindow(const std::shared_ptr<FloatingWindow>& wnd);

}

#endif /* FLOATINGWINDOWS_FWLINTEGRATION_H_ */
