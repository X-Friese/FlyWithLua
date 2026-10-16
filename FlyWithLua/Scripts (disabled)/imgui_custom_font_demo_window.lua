-- Imgui Custon Font Demo Window
-- William R. Good 10-16-26

-- V 1.00 

if not SUPPORTS_FLOATING_WINDOWS then
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

-----------------------------------Variables go here--------------------------------------------
dataref("frame_rate_period", "sim/operation/misc/frame_rate_period", "readonly")
dataref("framerate_period", "sim/time/framerate_period", "readonly")
dataref("gpu_time_per_frame_sec_approx", "sim/time/gpu_time_per_frame_sec_approx", "readonly")


-- Isolate tracking states securely
local icfdw_wnd = nil
local icfdw_window_visible = false

-------------------------------------Build Your GUI Here----------------------------------------

local function icfdw_on_build(wnd, x, y)
    imgui.PushStyleColor(imgui.constant.Col.Button, 0xFF0000FF)
    
    -- Custom Red Button
    if imgui.Button("##my_icfdw_colored_button", 15, 15) then
        -- Indirectly calls the global hide to completely unregister contexts safely
        icfdw_hide_wnd()
        imgui.PopStyleColor()
        return
    end 
    imgui.PopStyleColor()
    
    imgui.SameLine()
    imgui.TextUnformatted("This button hides this window")
 
    imgui_push_font(1)
	imgui.TextUnformatted("ProFontWindows.ttf Font Size 13")
    imgui.PopFont()	
    imgui_push_font(2)
	imgui.TextUnformatted("ProFontWindows.ttf Font Size 16")
    imgui.PopFont()		
    imgui_push_font(3)
	imgui.TextUnformatted("ProFontWindows.ttf Font Size 20")	
    imgui.PopFont()
    
    imgui_push_font(4)
	imgui.TextUnformatted("Roboto-Light.ttf Font Size 13")
    imgui.PopFont()	
    imgui_push_font(5)
	imgui.TextUnformatted("Roboto-Light.ttf Font Size 16")
    imgui.PopFont()		
    imgui_push_font(6)
	imgui.TextUnformatted("Roboto-Light.ttf Font Size 20")	
    imgui.PopFont()    

    imgui_push_font(7)
	imgui.TextUnformatted("Roboto-Regular.ttf Font Size 13")
    imgui.PopFont()	
    imgui_push_font(8)
	imgui.TextUnformatted("Roboto-Regular.ttf Font Size 16")
    imgui.PopFont()		
    imgui_push_font(8)
	imgui.TextUnformatted("Roboto-Regular.ttf Font Size 20")	
    imgui.PopFont() 

end

-- Catch if user closes via native X-Plane window frame "X" button decoration
local function icfdw_on_native_close(wnd)
    -- Clean up window entirely on frame closure to maintain identical state logic
    if icfdw_wnd then
        float_wnd_destroy(icfdw_wnd)
        icfdw_wnd = nil
    end
    icfdw_window_visible = false
end

-------------------Visibility & Lifetime Management Toggling System---------------------------

-- Find the VR dataref safely
local icfdw_vr_enabled = XPLMFindDataRef("sim/graphics/VR/enabled")
local icfdw_was_vr_enabled = false

-- Dynamic Helper to instantiate window contexts cleanly
local function create_icfdw_window_context()
    icfdw_wnd = float_wnd_create(400, 220, 1, true)
    float_wnd_set_title(icfdw_wnd, "Imgui Custom Font Demo Window v1.00")
    float_wnd_set_imgui_builder(icfdw_wnd, "icfdw_on_build")
    float_wnd_set_onclose(icfdw_wnd, "icfdw_on_native_close")
end


local function icfdw_show_wnd()
    -- Rebuild the context from scratch if it was destroyed by closing or exiting VR
    if not icfdw_wnd then
        create_icfdw_window_context()
    end
    
    -- REMOVED float_wnd_set_positioning_mode forcing to prevent lens-pinning.
    -- X-Plane will now create the window as a native floating panel.
    float_wnd_set_visible(icfdw_wnd, 1)
    icfdw_window_visible = true
end


local function icfdw_hide_wnd()
    if icfdw_wnd then
        -- Completely purge context memory to prevent the VR graphics engine from throwing a CTD
        float_wnd_destroy(icfdw_wnd)
        icfdw_wnd = nil
    end
    icfdw_window_visible = false
end

local function toggle_icfdw_window()
    if icfdw_window_visible then
        icfdw_hide_wnd()
    else
        icfdw_show_wnd()
    end
end

-- ----------------- Continuous Frame Monitoring Loop -----------------
local function monitor_vr_state()
    if icfdw_vr_enabled then
        local current_vr_state = XPLMGetDatai(icfdw_vr_enabled) == 1
        
        -- Detect exact frame transition out of VR
        if icfdw_was_vr_enabled and not current_vr_state then
            icfdw_hide_wnd() -- Nukes the window frame cleanly
            icfdw_was_vr_enabled = false
            logMsg("Imgui Sound Settings: Safely closed window on VR exit to prevent crash.")
            
        -- Sync state tracking when going into VR
        elseif not icfdw_was_vr_enabled and current_vr_state then
            icfdw_was_vr_enabled = true
        end
    end
end

-- Register tracking frame loops
do_every_frame("monitor_vr_state()")

-- Global Exposure Mappings for X-Plane Callback Resolution
_G["icfdw_on_build"] = icfdw_on_build
_G["icfdw_on_native_close"] = icfdw_on_native_close
_G["icfdw_show_wnd"] = icfdw_show_wnd
_G["icfdw_hide_wnd"] = icfdw_hide_wnd
_G["toggle_icfdw_window"] = toggle_icfdw_window
_G["monitor_vr_state"] = monitor_vr_state

------------------------------------------------------------------------------------------------
add_macro("Imgui Custom Font Demo Window: show/hide", "icfdw_show_wnd()", "icfdw_hide_wnd()", "deactivate")
create_command("fwl_imgui/custom_font_demo_window/show_toggle", "open/close Imgui Custom Font Demo Window", "toggle_icfdw_window()", "", "")
