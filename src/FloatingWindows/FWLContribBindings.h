#pragma once

#ifndef FWL_CONTRIB_BINDINGS_H
#define FWL_CONTRIB_BINDINGS_H

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

namespace flywithlua {

void InitContribSupport(lua_State* L);
void UpdateContribMarkdownFonts();

} // namespace flywithlua

#endif // FWL_CONTRIB_BINDINGS_H
