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
void ClearImGeeMarkdownTextures();

class ScopedMarkdownWindowBaseDir {
public:
    ScopedMarkdownWindowBaseDir(const char* auto_base_dir, const char* explicit_base_dir);
    ~ScopedMarkdownWindowBaseDir();
    ScopedMarkdownWindowBaseDir(const ScopedMarkdownWindowBaseDir&) = delete;
    ScopedMarkdownWindowBaseDir& operator=(const ScopedMarkdownWindowBaseDir&) = delete;
private:
    const char* m_prev_base_dir = nullptr;
};

} // namespace flywithlua

#endif // FWL_IMGEE_BINDINGS_H
