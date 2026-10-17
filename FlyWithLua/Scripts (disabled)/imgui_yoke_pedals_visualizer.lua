-- Imgui Yoke Pedals Visualizer
-- William R. Good 08-12-26
-- Added custom font
-- Fixed slider Id issue
-- V 1.04 (Refactored to completely resolve VR exit CTD via dynamic window allocation)

if not SUPPORTS_FLOATING_WINDOWS then
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

-- Datarefs
dataref("rudder_pedal_position", "sim/flightmodel/controls/ldruddef")
dataref("left_brake_pedal_position", "sim/cockpit2/controls/left_brake_ratio")
dataref("right_brake_pedal_position", "sim/cockpit2/controls/right_brake_ratio")
dataref("yoke_pitch_position", "sim/cockpit2/controls/yoke_pitch_ratio")
dataref("yoke_roll_position", "sim/cockpit2/controls/yoke_roll_ratio")
dataref("elevator1_deg", "sim/flightmodel2/wing/elevator1_deg")
dataref("elevator_trim", "sim/flightmodel2/controls/elevator_trim")

-- Isolate local tracking variables across frames
local iypv_wnd = nil
local iypv_window_visible = false


-- Define drawing calculations
local function iypv_on_build(wnd, x, y)
    imgui.PushStyleColor(imgui.constant.Col.WindowBg, 0xCC101112)
    imgui.PushStyleColor(imgui.constant.Col.Button, 0xFF0000FF)
    
    if iypv_wnd and imgui.Button("##my_iypv_colored_button", 15, 15) then
        float_wnd_set_visible(iypv_wnd, 0) 
        imgui.PopStyleColor() 
        imgui.PopStyleColor() 
        return 
    end 
    imgui.PopStyleColor()
    
    if iypv_wnd then
        imgui.SameLine()
        imgui.TextUnformatted("This button hides wnd")
    end
    
    imgui_push_font(7)    
    imgui.SetCursorPosX(20)
    imgui.VSliderFloat("##slider1", 17, 100, rudder_pedal_position, -19.0, 19.0, "")
    imgui.SameLine()
    imgui.SetCursorPosX(55)
    imgui.VSliderFloat("##slider2", 17, 100, left_brake_pedal_position, 0.0, 1.0, "")
    imgui.SameLine()    
    imgui.SetCursorPosX(90)
    imgui.VSliderFloat("##slider3", 17, 100, yoke_pitch_position, 0.99, -0.99, "")    
    imgui.SameLine()
    imgui.SetCursorPosX(125)
    imgui.VSliderFloat("##slider4", 17, 100, right_brake_pedal_position, 0.0, 1.0, "")
    imgui.SameLine()      
    imgui.SetCursorPosX(160)
    imgui.VSliderFloat("##slider5", 17, 100, rudder_pedal_position, 19.0, -19.0, "")
    imgui.SetCursorPosX(20)
    imgui.PushItemWidth(160)
    imgui.SliderFloat("##slider6", yoke_roll_position, -0.99, 0.99, "")

    imgui.TextUnformatted(string.format("yoke_position  %f", yoke_pitch_position))
    imgui.TextUnformatted(string.format("elevator1_deg  %f", elevator1_deg))
    imgui.TextUnformatted(string.format("elevator_trim  %f", elevator_trim))

    imgui.SetCursorPosX(50)    
    if imgui.Button("Takeoff Trim", 100, 20) then
        command_once("sim/flight_controls/pitch_trim_takeoff")
    end    
    imgui.PopFont()    
    
    imgui.PopStyleColor()
end


-- Catch if user closes via native X-Plane window frame "X" button decoration
local function iypv_on_native_close(wnd)
    -- Clean up window entirely on frame closure to maintain identical state logic
    if iypv_wnd then
        float_wnd_destroy(iypv_wnd)
        iypv_wnd = nil
    end
    iypv_window_visible = false
end


-------------------Visibility & Lifetime Management Toggling System---------------------------

-- Find the VR dataref safely
local iypv_vr_enabled = XPLMFindDataRef("sim/graphics/VR/enabled")
local iypv_was_vr_enabled = false

-- Dynamic Helper to instantiate window contexts cleanly
local function create_iypv_window_context()
    iypv_wnd = float_wnd_create(205, 235, 1, true)
    float_wnd_set_title(iypv_wnd, "Imgui Visualizer v1.04")
    float_wnd_set_imgui_builder(iypv_wnd, "iypv_on_build")
    float_wnd_set_onclose(iypv_wnd, "iypv_on_native_close")
end


local function iypv_show_wnd()
    -- Rebuild the context from scratch if it was destroyed by closing or exiting VR
    if not iypv_wnd then
        create_iypv_window_context()
    end
    
    -- REMOVED float_wnd_set_positioning_mode forcing to prevent lens-pinning.
    -- X-Plane will now create the window as a native floating panel.
    float_wnd_set_visible(iypv_wnd, 1)
    iypv_window_visible = true
end


local function iypv_hide_wnd()
    if iypv_wnd then
        -- Completely purge context memory to prevent the VR graphics engine from throwing a CTD
        float_wnd_destroy(iypv_wnd)
        iypv_wnd = nil
    end
    iypv_window_visible = false
end


local function toggle_iypv_window()
    if iypv_window_visible then
        iypv_hide_wnd()
    else
        iypv_show_wnd()
    end
end


-- ----------------- Continuous Frame Monitoring Loop -----------------
local function monitor_iypv_vr_state()
    if iypv_vr_enabled then
        local current_iypv_vr_state = XPLMGetDatai(iypv_vr_enabled) == 1
        
        -- Detect exact frame transition out of VR
        if iypv_was_vr_enabled and not current_iypv_vr_state then
            iypv_hide_wnd() -- Nukes the window frame cleanly
            iypv_was_vr_enabled = false
            logMsg("Imgui Sound Settings: Safely closed window on VR exit to prevent crash.")
            
        -- Sync state tracking when going into VR
        elseif not iypv_was_vr_enabled and current_iypv_vr_state then
            iypv_was_vr_enabled = true
        end
    end
end


-- Register tracking frame loops
do_every_frame("monitor_iypv_vr_state()")


-- Global Exposure Mappings for X-Plane Callback Resolution
_G["iypv_on_build"] = iypv_on_build
_G["iypv_on_native_close"] = iypv_on_native_close
_G["iypv_show_wnd"] = iypv_show_wnd
_G["iypv_hide_wnd"] = iypv_hide_wnd
_G["toggle_iypv_window"] = toggle_iypv_window
_G["monitor_iypv_vr_state"] = monitor_iypv_vr_state


------------------------------------------------------------------------------------------------
-- Register macros and command hooks cleanly
add_macro("Imgui Visualizer Toggle: show/hide", "iypv_show_wnd()", "iypv_hide_wnd()", "deactivate")
create_command("fwl_imgui/visualizer/show_toggle", "open/close Imgui Visualizer Toggle", "toggle_iypv_window()", "", "")