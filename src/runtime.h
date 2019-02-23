#pragma once

#include <stdexcept>
#include <sol.hpp>

namespace flywithlua {
namespace runtime {

class BadScript : public std::runtime_error {
    // A trick to make the exception copy-safe. See: https://stackoverflow.com/a/24520735/759134.
    using string_type = std::runtime_error;
public:
    BadScript(std::string const &msg, std::string const &filename, int line);
    BadScript(const BadScript &) noexcept = default;

    const char *filename() const { return _filename.what(); }

    int line() const { return _line; }

private:
    const string_type _filename;
    const int _line;
};

typedef int8_t ExecOptions;

static const ExecOptions NO_QUARANTINE = 1;
static const ExecOptions IGNORE_STOPPED_LUA = 2;

inline ExecOptions OptIf(bool value, ExecOptions opts) { return value ? opts : ExecOptions(0); }

void Quarantine(BadScript const &e);

sol::protected_function LoadFile(lua_State *L, std::string const &filename);

sol::protected_function LoadString(lua_State *L, std::string const &code, std::string const &chunk_name = "[string]");

sol::protected_function_result Exec(lua_State *L,
                                    sol::protected_function const &func,
                                    bool quarantine_bad_scripts = true);

sol::protected_function_result ExecFile(lua_State *L, std::string const &filename, ExecOptions opts = 0);
sol::protected_function_result ExecString(lua_State *L,
                                          std::string const &code,
                                          std::string const &chunk_name = "[string]",
                                          ExecOptions opts = 0);

sol::protected_function_result ExecChunk(lua_State *L,
                                         std::string const &chunk_name,
                                         std::string const &chunk_code,
                                         ExecOptions opts = 0);
}
}