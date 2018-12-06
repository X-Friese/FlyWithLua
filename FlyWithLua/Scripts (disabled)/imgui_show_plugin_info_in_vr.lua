-- Imugi Show Plugin Information In VR Ver. 1.02
-- Using Imgui and ffi display plugin info and enable/disable in VR
-- William R. Good 12-05-18

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

-- ======================================================================================
-- Replicate Plugin Admin > Enable/Disable and Plugin Admin > Plugin Information for VR
-- using FlyWithLua, ffi and imgui. 
--
-- Thanks to jörn-jören jörensön on the FlyWithLUA forum for this enlightening
-- post on the FFI library, and how to use it to provide access to the XPLM
-- library in FlyWIthLUA. and to Carsten Lynker (X-Friese) for the FlyWthLUA
-- plugin itself! A masterpiece of innovative thinking and implementation.
--
-- ------------------- BOILER PLATE CODE -------------------
-- See http://forums.x-plane.org/index.php?/forums/topic/123390-xplm-library-how-to-load-and-use/
--
local ffi = require("ffi")

local XPLMlib = ""
if SYSTEM == "IBM" then
    -- Windows OS (no path and file extension needed)
    if SYSTEM_ARCHITECTURE == 64 then
        XPLMlib = "XPLM_64"                                 -- 64bit
    end
elseif SYSTEM == "LIN" then
    -- Linux OS (we need the path "Resources/plugins/" here for some reason)
    if SYSTEM_ARCHITECTURE == 64 then
        XPLMlib = "Resources/plugins/XPLM_64.so"            -- 64bit
    end
elseif SYSTEM == "APL" then
    -- Mac OS (we need the path "Resources/plugins/" here for some reason)
    XPLMlib = "Resources/plugins/XPLM.framework/XPLM"       -- 64bit and 32 bit
else
    logMsg("IMPOSSIBLE ERROR: Cannot determine the OS")
    return
end

-- load the lib and store in local variable
local XPLM = ffi.load(XPLMlib)
-- ---------------------------------------------------------
--
-- The FFI Library is part of the LuaJIT engine that X-Plane uses for LUA. For 
-- info and its API see http://luajit.org/ext_ffi.html. FFI provides low level
-- access to the X-Plane API XPLM. For info on that see
--     http://www.xsquawkbox.net/xpsdk/mediawiki/Category:Documentation
-- and specifically, for dataref methods
--     http://www.xsquawkbox.net/xpsdk/mediawiki/DataAccess
--

ffi.cdef[[
typedef void *  XPLMDataRef;                                // For clarity
typedef int     XPLMPluginID;
typedef int     XPLMDataTypeID;                             // Skip the enum
typedef int     (* XPLMGetDatavf_f)(                        // Scenery object calls to get an indexed value
                void *      inRefcon,
                float *     outValues,                      // Watch it: can be NULL
                int         inOffset,
                int         inMax);
                
typedef void    (* XPLMSetDatavf_f)(
                void *      inRefcon,    
                float *     inValues,    
                int         inOffset,    
                int         inCount);  
                
XPLMPluginID    XPLMGetMyID(void);

XPLMPluginID    XPLMFindPluginBySignature(
                const char *    inSignature);

XPLMPluginID    XPLMGetNthPlugin(
                int inIndex);   

void            XPLMReloadScenery(void);

void            XPLMSendMessageToPlugin(
                XPLMPluginID    inPlugin,    
                int             inMessage,    
                const char *    inParam);                   // Actually void * in API but we send string so cheat

XPLMDataRef XPLMRegisterDataAccessor(
                const char *    inDataName,                 // dc3dreamer/morse_flash and 
                XPLMDataTypeID  inDataType,                 // 8=xplmFloatArray
                int             inIsWritable,               // 0 = read-only for Morse, 1 = writable for blinkers
                void *          inReadInt,                  // Avoid definintions for unused callbacks (typ.)
                void *          inWriteInt,
                void *          inReadFloat,
                void *          inWriteFloat,
                void *          inReadDouble,
                void *          inWriteDouble,
                void *          inReadIntArray,
                void *          inWriteIntArray,
                XPLMGetDatavf_f inReadFloatArray,           // We must implement this, others can be NULL/nil   
                XPLMSetDatavf_f inWriteFloatArray,          // Maybe needed for scenery light datarefs?
                void *          inReadData,
                void *          inWriteData,  
                void *          inReadRefcon,       
                void *          inWriteRefcon);
                
void      XPLMUnregisterDataAccessor(
                XPLMDataRef     inDataRef);

int       XPLMCountPlugins(void);

void      XPLMGetPluginInfo(
                XPLMPluginID         inPlugin,    
                char *               outName,    /* Can be NULL */
                char *               outFilePath,    /* Can be NULL */
                char *               outSignature,    /* Can be NULL */
                char *               outDescription);    /* Can be NULL */

int       XPLMIsPluginEnabled(
               XPLMPluginID         inPluginID);

int       XPLMEnablePlugin(
               XPLMPluginID         inPluginID);

void      XPLMDisablePlugin(
               XPLMPluginID         inPluginID);			
                
]]

-- as Lua doesn't know char*, we have to declare a C-variable via FFI
local Name = ffi.new("char[1024]")
local FilePath = ffi.new("char[1024]")
local Signature = ffi.new("char[1024]")
local Description = ffi.new("char[1024]")

-- Not sure if this is needed but seemed unhappy when I removed it.
local newVal = {}
local changed = {}
local plugin_enabled = {}

function ispiiv_on_build(ispiiv_wnd, x, y)
	local xx = XPLM.XPLMCountPlugins()
	local id = XPLM.XPLMGetNthPlugin(xx - 1)
	for i = 1, id do
		XPLM.XPLMGetPluginInfo(i, Name, FilePath, Signature, Description)
		local en = XPLM.XPLMIsPluginEnabled(i)
		if (en == 1) then 
			en_string = "Enabeled"
			plugin_enabled[i] = true
		else
			en_string = "Disabled"
			plugin_enabled[i] = false
		end
		imgui.TextUnformatted(string.format("(%d) %s (%s)", i, ffi.string(Name), en_string))
		imgui.PushID(ffi.string(Name)) 
		changed[i], newVal[i] = imgui.Checkbox(" ", plugin_enabled[i] == true)
        if changed[i] then
			if newVal[i] == true then
				plugin_enabled[i] = true
				test = XPLM.XPLMEnablePlugin(i)
			else
				plugin_enabled[i] = false
				XPLM.XPLMDisablePlugin(i)
			end
		end
		imgui.PopID()
		imgui.SameLine()	
		imgui.TextUnformatted(string.format("%s", ffi.string(FilePath)))
		imgui.TextUnformatted(string.format("     Signature: %s", ffi.string(Signature)))
		imgui.TextUnformatted(string.format("     %s", ffi.string(Description)))
		imgui.TextUnformatted("")
	end
end

function closed_imgui_show_plugin_info_in_vr(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end

ispiiv_wnd = nil

function ispiiv_show_wnd()
	ispiiv_wnd = float_wnd_create(650, 550, 1, true)
	float_wnd_set_position(ispiiv_wnd, 100, 100)
	float_wnd_set_title(ispiiv_wnd, "Imgui Show Plugin Info In VR")
	float_wnd_set_imgui_builder(ispiiv_wnd, "ispiiv_on_build")
	float_wnd_set_onclose(ispiiv_wnd, "closed_imgui_show_plugin_info_in_vr")

end

function ispiiv_hide_wnd()
    if ispiiv_wnd then
        float_wnd_destroy(ispiiv_wnd)
    end
end

ispiiv_show_only_once = 0
ispiiv_hide_only_once = 0

function toggle_imgui_show_plugin_info()
	ispiiv_show_window = not ispiiv_show_window
	if ispiiv_show_window then
		if ispiiv_show_only_once == 0 then
			ispiiv_show_wnd()
			ispiiv_show_only_once = 1
			ispiiv_hide_only_once = 0
		end
	else
		if ispiiv_hide_only_once == 0 then
			ispiiv_hide_wnd()
			ispiiv_hide_only_once = 1
			ispiiv_show_only_once = 0
		end
	end
end

add_macro("Imgui Show Plugin Info In VR: open/close", "ispiiv_show_wnd()", "ispiiv_hide_wnd()", "deactivate")
create_command("FlyWithLua/imgui-show-plugin-info-in-vr/show_toggle", "open/close imgui show plugin info", "toggle_imgui_show_plugin_info()", "", "")
