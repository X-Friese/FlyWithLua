# LuaXML for FlyWithLua

This directory contains build scripts necessary to embed LuaXML into FlyWithLua
plugin. Directory `./upstream/` contains unmodified sources downloaded from
[LuaRocks](https://luarocks.org/modules/djerius/luaxml) website. 

## Upgrading to a new version of LuaXML

 1. Download the latest `.rock` package from
    [LuaRocks](https://luarocks.org/modules/djerius/luaxml) website.
 2. In the package find `LuaXML_XXXXX.zip` archive and extract its contents
    into `./upstream/` subdirectory.
 3. Copy `./upstream/LuaXML.lua` to `FlyWithLua/Modules/`.
 4. Clean out pre-compiled binaries, which we don't use: `(cd upstream; rm
    *.exe *.dll *.so)`

Assuming there are no breaking changes, the new version should "just work".
