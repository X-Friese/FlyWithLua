# Sol2: C++ <-> Lua bindings

This directory contains build scripts for Sol2, which is used for C++-friendly
interaction with lua, including function binding, stack interaction and more.
Directory `./upstream/` contains single-file version of Sol2 available from
[Sol2 GitHub](https://github.com/ThePhD/sol2). 

## Upgrading to a new version of Sol2

 1. Download sol.hpp from GitHub [the Releases section](https://github.com/ThePhD/sol2/releases/download/v3.2.1/sol.hpp) and
    place it under `./upstream`.
 2. Apply a patch for [issue
    22](https://github.com/X-Friese/FlyWithLua/issues/22) if it hasn't been
    resolved.
    
    ```bash
    patch -p0 < fix_lua_tointeger.patch
    ```

Assuming there are no breaking changes, the new version should "just work".

Note: we could have used their own CMakeLists.txt to build the single-header
version ourselves or use it in multi-header mode, but this only adds build-time
dependencies (e.g. Python) and no practical advantage.
