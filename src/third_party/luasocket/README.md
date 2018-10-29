# LuaSocket for FlyWithLua

This directory contains build scripts necessary to embed LuaSocket into FlyWithLua
plugin. Directory `./upstream/` contains unmodified sources downloaded from
[LuaSocket](https://github.com/diegonehab/luasocket) GitHub page. 

## Upgrading to a new version of LuaSocket

 1. Remove everything from `./upstream/` subdirectory: `rm -r ./upstream/*`.
 1. Download the latest master branch
    [snapshot](https://github.com/diegonehab/luasocket/archive/master.zip).
 1. Extract contents of `luasocket-master` directory from the archive into
    `./upstream/` directory. Verify that extracted files __do not__ contain
    `luasocket-master` in their paths.
 1. Copy update Lua scripts in `FlyWithLua/Modules` directory:
    ```
       $ FWL_DIR="$(git rev-parse --show-toplevel)"
       $ MODULES_DIR="$FWL_DIR/FlyWithLua/Modules"
       $ mkdir "$MODULES_DIR/socket"
       $ cp upstream/src/{ltn12.lua,socket.lua,mime.lua} "$MODULES_DIR"
       $ cp upstream/src/{http.lua,url.lua,tp.lua,ftp.lua,headers.lua,smtp.lua} "$MODULES_DIR/socket"
    ```

Assuming there are no breaking changes, the new version should "just work".
