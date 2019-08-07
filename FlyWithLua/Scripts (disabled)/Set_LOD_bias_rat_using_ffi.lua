-- Test FlyWithLua script to set LOD_bias_rat value
-- William R. Good 07-25-2019
-- Thanks to jörn-jören jörensön for his examples that I used as a starting point. 
-- first we need ffi module (variable must be declared local)
local ffi = require("ffi")

-- find the right lib to load
local XPLMlib = ""
if SYSTEM == "IBM" then
	-- Windows OS (no path and file extension needed)
  	if SYSTEM_ARCHITECTURE == 64 then
    		XPLMlib = "XPLM_64"  -- 64bit
  	else
    		XPLMlib = "XPLM"     -- 32bit
  	end
elseif SYSTEM == "LIN" then
  	-- Linux OS (we need the path "Resources/plugins/" here for some reason)
  	if SYSTEM_ARCHITECTURE == 64 then
    		XPLMlib = "Resources/plugins/XPLM_64.so"  -- 64bit
  	else
    		XPLMlib = "Resources/plugins/XPLM.so"     -- 32bit
  	end
elseif SYSTEM == "APL" then
  	-- Mac OS (we need the path "Resources/plugins/" here for some reason)
  	XPLMlib = "Resources/plugins/XPLM.framework/XPLM" -- 64bit and 32 bit
else
  	return -- this should not happen
end

-- load the lib and store in local variable
local XPLM = ffi.load(XPLMlib)


---------------------------------------------------
-- declare all needed XPLM functions
---------------------------------------------------
ffi.cdef("typedef void * XPLMDataRef;")
ffi.cdef("XPLMDataRef XPLMFindDataRef(const char * inDataRefName);")
ffi.cdef("int XPLMGetDatai(XPLMDataRef inDataRef);")
ffi.cdef("void XPLMSetDatai(XPLMDataRef inDataRef, int inValue);")
ffi.cdef("float XPLMGetDataf(XPLMDataRef inDataRef);")
ffi.cdef("void XPLMSetDataf(XPLMDataRef inDataRef, float inValue);")
ffi.cdef("double XPLMGetDatad(XPLMDataRef inDataRef);")
ffi.cdef("void XPLMSetDatad(XPLMDataRef inDataRef, double inValue);")


---------------------------------------------------
-- example how to find, read and write a private DataRef
---------------------------------------------------
-- Caution: use private DataRefs only, if you know what you are doing!

-- get the pointer
-- local dataRef_ptr = findDataRef("sim/private/controls/reno/LOD_bias_rat")

-- check if DataRef exists (always do that when using private DataRefs!)
-- please notice: in this case compare with "dataRef_ptr == nil", a simple "if not dataRef_ptr then" will not work!
-- if dataRef_ptr == nil then
--  	return
--end

-- read DataRef (float)
-- value = XPLM.XPLMGetDataf(dataRef_ptr)

-- write DataRef (float)
-- XPLM.XPLMSetDataf(dataRef_ptr, value)


---------------------------------------------------
-- use functions to encapsulate the XPLM access:
---------------------------------------------------

-- get the pointer to a DataRef
function findDataRef(dataRefName)
	if XPLM then
		return XPLM.XPLMFindDataRef(dataRefName)
  	end
	return nil
end

-- check if DataRef exists
function dataRefExists(dataRefName)
	return (XPLM.XPLMFindDataRef(dataRefName) ~= nil)
end

-- get value (int)
function getDataRef_i(dataRef_ptr)
	if dataRef_ptr ~= nil then
		return XPLM.XPLMGetDatai(dataRef_ptr)
  	end
  	return nil
end
-- get value (float)
function getDataRef_f(dataRef_ptr)
	if dataRef_ptr ~= nil then
		return XPLM.XPLMGetDataf(dataRef_ptr)
  	end
  	return nil
end
-- get value (double)
function getDataRef_d(dataRef_ptr)
	if dataRef_ptr ~= nil then
		return XPLM.XPLMGetDatad(dataRef_ptr)
	end
	return nil
end

-- set value (int)
function setDataRef_i(dataRef_ptr, value)
	if dataRef_ptr ~= nil then
		XPLM.XPLMSetDatai(dataRef_ptr, value)
	end
end
-- set value (float)
function setDataRef_f(dataRef_ptr, value)
	if dataRef_ptr ~= nil then
		XPLM.XPLMSetDataf(dataRef_ptr, value)
  	end
end
-- set value (double)
function setDataRef_d(dataRef_ptr, value)
	if dataRef_ptr ~= nil then
		XPLM.XPLMSetDatad(dataRef_ptr, value)
  	end
end

-- get the pointers to all needed Private DataRefs
local PrivatedataRef1 = findDataRef("sim/private/controls/reno/LOD_bias_rat")
-- local PrivatedataRef1 = findDataRef("sim/private/controls/fog/fog_be_gone")
--local PrivatedataRef2 = findDataRef("sim/private/any_dataref_2")
--local PrivatedataRef3 = findDataRef("sim/private/any_dataref_3")

-- any DataRef does not exist anymore?
-- please notice: You have to check with "dataRef == nil"! "not dataRef" will NOT work!
-- if dataRef1 == nil or dataRef2 == nil or dataRef3 == nil then
if PrivatedataRef1 == nil then
	-- stop the plugin
  	return
end

function main()
	-- read some values
  	local val1 = getDataRef_f(PrivatedataRef1)
--  	local val2 = getDataRef_f(PrivatedataRef2)
  
  	-- do the calculations
	-- Put the value for fog_be_gone in local val1
  	local val1 = 1.0
  
  	-- set the value
  	setDataRef_f(PrivatedataRef1, val1)
end

do_sometimes("main()")
