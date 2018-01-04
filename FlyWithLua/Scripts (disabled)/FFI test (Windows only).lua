-- This script is uses native Windows code
if SYSTEM == "IBM" then

-- This script will provide a Lua function to display a Windows Message Box
-- As the message box will stop the Lua enging until the user clicks "ok",
-- it can be used to pause X-Plane
--
-- usage: wait( "Message" )

-- we need the FFI library to access Windows C/C++ functions
local ffi = require("ffi")
local C = ffi.C

-- then we have to define all C stuff we want to use
ffi.cdef[[
int MessageBoxA(void *w, const char *txt, const char *cap, int type);
]]

-- providing the function
function wait( message )
    message = message or "X-Plane is waiting for your OK!"
    C.MessageBoxA(nil, message, "FlyWithLua", 0)
end

wait()
wait("You will have to confirm twice!")

-- the XPLM libraries are all included to X-Plane, but they are all C libraries
-- we can access all XPLM stuff via FFI, but we have to define everything we need first
ffi.cdef[[
int                   XPLMGetDirectoryContents(
                                   const char *         inDirectoryPath,    
                                   int                  inFirstReturn,    
                                   char *               outFileNames,    
                                   int                  inFileNameBufSize,    
                                   char **              outIndices,    /* Can be NULL */
                                   int                  inIndexCount,    
                                   int *                outTotalFiles,    /* Can be NULL */
                                   int *                outReturnedFiles);    /* Can be NULL */
]]

-- and we have to load the DLL
local XPLM = ffi.load(ffi.os == "Windows" and "XPLM_64")

-- as Lua doesn't know char*, we have to declare a C-variable via FFI
local filestring = ffi.new("char[1024]")

-- call the C function
XPLM.XPLMGetDirectoryContents(SCRIPT_DIRECTORY, 0, filestring, 1024, nil, 0, nil, nil)

-- re-convert the C-variable into Lua string
print(ffi.string(filestring))

-- Why does it only show the first file? Because the XPLM function sets null between the filenames.
-- But this is only a demonstration how to use the FFI library in FlyWithLua.

end  -- if system == "IBM"