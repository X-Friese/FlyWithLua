#pragma once

#ifndef FWL_IMGEE_BINDINGS_H
#define FWL_IMGEE_BINDINGS_H

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

namespace flywithlua {

void InitImGeeSupport(lua_State* L);
void UpdateImGeeMarkdownFonts();

} // namespace flywithlua

#endif // FWL_IMGEE_BINDINGS_H
