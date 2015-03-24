-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Script:  Keep monitor alive on Windows                --
-- Version: 1.0                                          --
-- Build:   2015-03-24                                   --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Description:                                          --
-- Will send alive messages to Windows periodically.     --
-- This should keep the monitor staying on.              --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

-- this only makes sense if we are running on Windows
if SYSTEM == "IBM" then
    ffi = require( "ffi" )
        
    -- define Microsoft's API C-function to be used from Lua
    -- see: https://msdn.microsoft.com/en-us/library/windows/desktop/aa373208(v=vs.85).aspx
    ffi.cdef( "int SetThreadExecutionState(int esFlags)" )

    -- define a global function (macros can only access global functions)
    function keep_monitor_on()
        ffi.C.SetThreadExecutionState(3)
    end
    
    do_sometimes("keep_monitor_on()")
end