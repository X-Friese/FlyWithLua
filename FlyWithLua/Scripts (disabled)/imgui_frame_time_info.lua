-- Imgui Frame Time Info
-- William R. Good 08-24-26

-- Added custom font
-- V 1.01 
-- V 1.02 (Refactored to change macro to a toggle like the command is to improve VR stability)

if not SUPPORTS_FLOATING_WINDOWS then
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

-----------------------------------Variables go here--------------------------------------------
dataref("frame_rate_period", "sim/operation/misc/frame_rate_period", "readonly")
dataref("framerate_period", "sim/time/framerate_period", "readonly")
dataref("gpu_time_per_frame_sec_approx", "sim/time/gpu_time_per_frame_sec_approx", "readonly")


-- Isolate tracking states securely
local ifti_wnd = nil
local ifti_window_visible = false

-------------------------------------Build Your GUI Here----------------------------------------

local function ifti_on_build(wnd, x, y)
    imgui.PushStyleColor(imgui.constant.Col.WindowBg, 0xCC101112)
    imgui.PushStyleColor(imgui.constant.Col.Button, 0xFF0000FF)
    
    -- Custom Red Button
    if ifti_wnd and imgui.Button("##my_ifti_colored_button", 15, 15) then
        toggle_ifti_window()
        imgui.PopStyleColor()
		imgui.PopStyleColor()
        return
    end 
    imgui.PopStyleColor()
	imgui_push_font(8)
	
	if ifti_wnd then
		imgui.SameLine()
		imgui.TextUnformatted("This button toggles this window")
	end	
    
	local frame_rate_period_formatted = string.format("%.4f", frame_rate_period) 
	imgui.TextUnformatted(frame_rate_period_formatted)
	imgui.SameLine()
	imgui.TextUnformatted("  sim/operation/misc/frame_rate_period")
	local framerate_period_formatted = string.format("%.4f", framerate_period)
	imgui.TextUnformatted(framerate_period_formatted)
	imgui.SameLine()
	imgui.TextUnformatted("  sim/time/framerate_period")
	local gpu_time_per_frame_sec_approx_formatted = string.format("%.4f", gpu_time_per_frame_sec_approx)
	imgui.TextUnformatted(gpu_time_per_frame_sec_approx_formatted)
	imgui.SameLine()	
	imgui.TextUnformatted("  sim/time/gpu_time_per_frame_sec_approx")	


    imgui.PopFont()
	imgui.PopStyleColor()

end

-- Catch if user closes via native X-Plane window frame "X" button decoration
local function ifti_on_native_close(wnd)
    -- Clean up window entirely on frame closure to maintain identical state logic
    if ifti_wnd then
        float_wnd_destroy(ifti_wnd)
        ifti_wnd = nil
    end
    ifti_window_visible = false
end

-------------------Visibility & Lifetime Management Toggling System---------------------------

-- Find the VR dataref safely
local ifti_vr_enabled = XPLMFindDataRef("sim/graphics/VR/enabled")
local ifti_was_vr_enabled = false

-- Dynamic Helper to instantiate window contexts cleanly
local function create_ifti_window_context()
    ifti_wnd = float_wnd_create(400, 100, 1, true)
    float_wnd_set_title(ifti_wnd, "Imgui Frame Time info v1.02")
    float_wnd_set_imgui_builder(ifti_wnd, "ifti_on_build")
    float_wnd_set_onclose(ifti_wnd, "ifti_on_native_close")
end


local function ifti_show_wnd()
    -- Rebuild the context from scratch if it was destroyed by closing or exiting VR
    if not ifti_wnd then
        create_ifti_window_context()
    end
    
    -- REMOVED float_wnd_set_positioning_mode forcing to prevent lens-pinning.
    -- X-Plane will now create the window as a native floating panel.
    float_wnd_set_visible(ifti_wnd, 1)
    ifti_window_visible = true
end


local function ifti_hide_wnd()
    if ifti_wnd then
        -- Completely purge context memory to prevent the VR graphics engine from throwing a CTD
        float_wnd_destroy(ifti_wnd)
        ifti_wnd = nil
    end
    ifti_window_visible = false
end

local function toggle_ifti_window()
    if ifti_window_visible then
        ifti_hide_wnd()
    else
        ifti_show_wnd()
    end
end

-- ----------------- Continuous Frame Monitoring Loop -----------------
local function monitor_vr_state()
    if ifti_vr_enabled then
        local current_vr_state = XPLMGetDatai(ifti_vr_enabled) == 1
        
        -- Detect exact frame transition out of VR
        if ifti_was_vr_enabled and not current_vr_state then
            ifti_hide_wnd() -- Nukes the window frame cleanly
            ifti_was_vr_enabled = false
            logMsg("Imgui Sound Settings: Safely closed window on VR exit to prevent crash.")
            
        -- Sync state tracking when going into VR
        elseif not ifti_was_vr_enabled and current_vr_state then
            ifti_was_vr_enabled = true
        end
    end
end

-- Register tracking frame loops
do_every_frame("monitor_vr_state()")

-- Global Exposure Mappings for X-Plane Callback Resolution
_G["ifti_on_build"] = ifti_on_build
_G["ifti_on_native_close"] = ifti_on_native_close
_G["ifti_show_wnd"] = ifti_show_wnd
_G["ifti_hide_wnd"] = ifti_hide_wnd
_G["toggle_ifti_window"] = toggle_ifti_window
_G["monitor_vr_state"] = monitor_vr_state

------------------------------------------------------------------------------------------------
add_macro("Toggle Imgui Frame Time Inf", "toggle_ifti_window()")
create_command("fwl_imgui/frame_time_info/show_toggle", "Toggle Imgui Frame Time Info Window", "toggle_ifti_window()", "", "")