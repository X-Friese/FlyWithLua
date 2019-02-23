#include "runtime.h"
#include "FlyWithLua.h"

namespace flywithlua {
namespace runtime {

namespace {
typedef std::tuple<std::string, int> _FileLine;

sol::optional<_FileLine> _GetLuaCaller(lua_State *L) {
    sol::optional<_FileLine> caller;

    lua_Debug debug;
    for (int frame = 1; lua_getstack(L, frame, &debug); frame++) {
        if (!lua_getinfo(L, "Sl", &debug)) {
            // Can't get stack frame info, maybe will have better luck with the next frame?
            continue;
        }

        std::string source(debug.source);
        if (source.length() == 0 || source[0] != '@') {
            // Stack frame doesn't belong to a Lua file. Might be C code or an evaluated string.
            // Let's try the frame above.
            continue;
        }

        // Looks like a Lua script, remember it.
        caller = _FileLine(source.substr(1), debug.currentline);
    }

    // Reached top of the stack, return whatever we found.
    // Normally this would contain the topmost Lua script in the call stack, which is almost always a top-level script
    // inside "Scripts" directory. This script would be our prime suspect for quarantining.
    return caller;
}

sol::object _LuaErrorHandler(sol::this_state L) {
    auto msg = sol::stack::check_get<std::string>(L, 1).value_or("unknown Lua error");

    // Attempt enhance error message with the call stack.
    luaL_traceback(L, L, msg.c_str(), 1);
    if (auto maybe_traceback = sol::stack::check_get<sol::string_view>(L, -1)) {
        const sol::string_view &traceback = maybe_traceback.value();
        msg.assign(traceback.data(), traceback.size());
    }

    if (auto maybe_caller = _GetLuaCaller(L)) {
        // We were able to obtain caller information, return BasScript exception as an error.
        auto caller = maybe_caller.value();
        return sol::make_object(L, BadScript(msg, std::get<0>(caller), std::get<1>(caller)));
    }

    // Fall back to canonical Lua approach of returning a string an an error object.
    return sol::make_object(L, msg);
}
}

void Quarantine(BadScript const &e) {
    std::string from = e.filename();
    std::string base_name = from.substr(from.find_last_of("/\\"));
    std::string to = quarantineDir + base_name;

    if (rename(from.c_str(), to.c_str()) == 0) {
        flywithlua::AutoRestartLua = true;
        logMsg(logToDevCon, "FlyWithLua Info: Moved Bad Script to " + to);
    } else {
        logMsg(logToDevCon, "FlyWithLua Info: Could not move bad script to " + to);
    }
}

sol::protected_function LoadFile(lua_State *L, std::string const &filename) {
    return sol::state_view(L).load_file(filename);
}

sol::protected_function LoadString(lua_State *L, std::string const &code, std::string const &chunk_name) {
    return sol::state_view(L).load(code, chunk_name);
}

sol::protected_function_result Exec(lua_State *L, sol::protected_function const &func, ExecOptions opts) {
    auto safe_func = sol::protected_function(
        func,
        (opts & NO_QUARANTINE) ? sol::make_object(L, sol::default_traceback_error_handler)
                               : sol::make_object(L, _LuaErrorHandler));

    if (!LuaIsRunning && !(opts & IGNORE_STOPPED_LUA)) {
        throw std::runtime_error("Lua is not running, can't execute Lua script or snippet!");
    }

    auto result = safe_func();
    if (!result.valid()) {
        flywithlua::LuaIsRunning = false;
        sol::stack_object maybe_bad_script = result;

        if (maybe_bad_script.is<BadScript>()) {
            auto bad_script = maybe_bad_script.as<BadScript>();
            Quarantine(bad_script);
            throw BadScript(bad_script);
        } else {
            sol::error e = result;
            throw sol::error(e);
        }
    }
    return result;
}

sol::protected_function_result ExecFile(lua_State *L, std::string const &filename, ExecOptions opts) {
    return Exec(L, LoadFile(L, filename), opts);
}

sol::protected_function_result ExecString(lua_State *L,
                                          std::string const &code,
                                          std::string const &chunk_name,
                                          ExecOptions opts) {
    return Exec(L, LoadString(L, code, chunk_name), opts);
}

sol::protected_function_result ExecChunk(lua_State *L,
                                         std::string const &chunk_name,
                                         std::string const &chunk_code,
                                         ExecOptions opts) {
    if (chunk_code.empty()) {
        return sol::protected_function_result();
    }

    sol::optional<sol::protected_function> chunk = sol::state_view(L)[chunk_name];
    if (!chunk) {
        return sol::protected_function_result();
    }
    return Exec(L, chunk.value(), opts);
}

BadScript::BadScript(std::string const &msg, std::string const &filename, int line)
    : runtime_error([&]() -> auto {
    std::stringstream full_msg;
    full_msg << "Bad script detected: " << filename << " (line " << line << ")." << std::endl;
    full_msg << "Error: " << msg << std::endl;
    return full_msg.str();
}()),
      _filename(filename),
      _line(line) {}

}
}