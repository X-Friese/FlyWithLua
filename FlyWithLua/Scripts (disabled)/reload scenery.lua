-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Script:  reload scenery                               --
-- Version: 1.0                                          --
-- Build:   2012-10-16                                   --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Description:                                          --
-- Will add a macro to reload the scenery.               --
-- During reload the simulator freezes a few seconds     --
-- (depending on the complexity of your scenery).        --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

-- load the XPLM library
ffi = require( "ffi" )
if SYSTEM_ARCHITECTURE == 64 then
    XPLM = ffi.load( "XPLM_64" )
else
    XPLM = ffi.load( "XPLM" )
end
    
-- define the XPLMReloadScenery() C-function to be used from Lua
ffi.cdef( "void XPLMReloadScenery(void)" )

-- define a global function (macros can only access global functions)
function let_XPLM_reload_the_scenery()
    XPLMSpeakString("Please wait, X-Plane is reloading the scenery. Don't panic! The simulator will freeze for a moment.")
    XPLM.XPLMReloadScenery()
end

-- create the macro
add_macro( "Reload the scenery", "let_XPLM_reload_the_scenery()" )