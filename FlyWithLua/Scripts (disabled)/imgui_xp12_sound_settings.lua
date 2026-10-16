-- Imgui xp12 Sound Settings
-- William R. Good 08-18-26
-- Fixed slider Id issue
-- Added custom font
-- V 1.22 (Refactored to completely resolve VR exit CTD via dynamic window allocation)

if not SUPPORTS_FLOATING_WINDOWS then
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

-----------------------------------Variables go here--------------------------------------------
dataref("master_volume", "sim/operation/sound/master_volume_ratio", "writable")
dataref("aircraft_exterior_volume", "sim/operation/sound/exterior_volume_ratio", "writable")
dataref("aircraft_interior_volume", "sim/operation/sound/interior_volume_ratio", "writable")
dataref("pilot_volume", "sim/operation/sound/pilot_volume_ratio", "writable")
dataref("copilot_volume", "sim/operation/sound/copilot_volume_ratio", "writable")
dataref("radio_volume", "sim/operation/sound/radio_volume_ratio", "writable")
dataref("enviro_volume", "sim/operation/sound/enviro_volume_ratio", "writable")
dataref("ui_volume", "sim/operation/sound/ui_volume_ratio", "writable")

local rep_wind_exists = false
if XPLMFindDataRef("simcoders/rep/settings/wind_volume") ~= nil then
    dataref("wind_volume", "simcoders/rep/settings/wind_volume", "writable")
    rep_wind_exists = true
end

-- Isolate tracking states securely
local ixss_wnd = nil
local ixss_window_visible = false

-------------------------------------Build Your GUI Here----------------------------------------

local function ixss_on_build(wnd, x, y)
    imgui.PushStyleColor(imgui.constant.Col.Button, 0xFF0000FF)
    
    -- Custom Red Button
    if imgui.Button("##my_ixss_colored_button", 15, 15) then
        -- Indirectly calls the global hide to completely unregister contexts safely
        ixss_hide_wnd()
        imgui.PopStyleColor()
        return
    end 
    imgui.PopStyleColor()
    
    imgui.SameLine()
    imgui.TextUnformatted("This button hides this window")

    imgui_push_font(8)
    imgui.SetNextItemWidth(-1)
    local changed, newVal = imgui.SliderFloat("##mv", master_volume, 0, 1, "Master Volume Value: %.2f")
    if changed then master_volume = newVal end

    imgui.SetNextItemWidth(-1)
    local changed, newVal = imgui.SliderFloat("##aev", aircraft_exterior_volume, 0, 1, "Aircraft Exterior Volume Value: %.2f")
    if changed then aircraft_exterior_volume = newVal end
    
    imgui.SetNextItemWidth(-1)
    local changed, newVal = imgui.SliderFloat("##aiv", aircraft_interior_volume, 0, 1, "Aircraft Interior Volume Value: %.2f")
    if changed then aircraft_interior_volume = newVal end

    imgui.SetNextItemWidth(-1)
    local changed, newVal = imgui.SliderFloat("##pv", pilot_volume, 0, 1, "Pilot Volume Value: %.2f")
    if changed then pilot_volume = newVal end

    imgui.SetNextItemWidth(-1)
    local changed, newVal = imgui.SliderFloat("##cv", copilot_volume, 0, 1, "Copilot Volume Value: %.2f")
    if changed then copilot_volume = newVal end

    imgui.SetNextItemWidth(-1)
    local changed, newVal = imgui.SliderFloat("##rv", radio_volume, 0, 1, "Radio Volume Value: %.2f")
    if changed then radio_volume = newVal end

    imgui.SetNextItemWidth(-1)
    local changed, newVal = imgui.SliderFloat("##ev", enviro_volume, 0, 1, "Enviro Volume Value: %.2f")
    if changed then enviro_volume = newVal end

    imgui.SetNextItemWidth(-1)
    local changed, newVal = imgui.SliderFloat("##uv", ui_volume, 0, 1, "Ui Volume Value: %.2f")
    if changed then ui_volume = newVal end

    imgui.PopFont()

    if rep_wind_exists then
        imgui.SetNextItemWidth(-1)
        local changed, newVal = imgui.SliderInt("##wv", wind_volume, 0, 100, "REP Wind sound level: %.0f")
        if changed then
            wind_volume = newVal
        end
    end
end

-- Catch if user closes via native X-Plane window frame "X" button decoration
local function ixss_on_native_close(wnd)
    -- Clean up window entirely on frame closure to maintain identical state logic
    if ixss_wnd then
        float_wnd_destroy(ixss_wnd)
        ixss_wnd = nil
    end
    ixss_window_visible = false
end

-------------------Visibility & Lifetime Management Toggling System---------------------------

-- Find the VR dataref safely
local ixss_vr_enabled = XPLMFindDataRef("sim/graphics/VR/enabled")
local ixss_was_vr_enabled = false

-- Dynamic Helper to instantiate window contexts cleanly
local function create_ixss_window_context()
    ixss_wnd = float_wnd_create(275, 250, 1, true)
    float_wnd_set_title(ixss_wnd, "Imgui Sound Settings v1.22")
    float_wnd_set_imgui_builder(ixss_wnd, "ixss_on_build")
    float_wnd_set_onclose(ixss_wnd, "ixss_on_native_close")
end


local function ixss_show_wnd()
    -- Rebuild the context from scratch if it was destroyed by closing or exiting VR
    if not ixss_wnd then
        create_ixss_window_context()
    end
    
    -- REMOVED float_wnd_set_positioning_mode forcing to prevent lens-pinning.
    -- X-Plane will now create the window as a native floating panel.
    float_wnd_set_visible(ixss_wnd, 1)
    ixss_window_visible = true
end


local function ixss_hide_wnd()
    if ixss_wnd then
        -- Completely purge context memory to prevent the VR graphics engine from throwing a CTD
        float_wnd_destroy(ixss_wnd)
        ixss_wnd = nil
    end
    ixss_window_visible = false
end

local function toggle_ixss_window()
    if ixss_window_visible then
        ixss_hide_wnd()
    else
        ixss_show_wnd()
    end
end

-- ----------------- Continuous Frame Monitoring Loop -----------------
local function monitor_vr_state()
    if ixss_vr_enabled then
        local current_vr_state = XPLMGetDatai(ixss_vr_enabled) == 1
        
        -- Detect exact frame transition out of VR
        if ixss_was_vr_enabled and not current_vr_state then
            ixss_hide_wnd() -- Nukes the window frame cleanly
            ixss_was_vr_enabled = false
            logMsg("Imgui Sound Settings: Safely closed window on VR exit to prevent crash.")
            
        -- Sync state tracking when going into VR
        elseif not ixss_was_vr_enabled and current_vr_state then
            ixss_was_vr_enabled = true
        end
    end
end

-- Register tracking frame loops
do_every_frame("monitor_vr_state()")

-- Global Exposure Mappings for X-Plane Callback Resolution
_G["ixss_on_build"] = ixss_on_build
_G["ixss_on_native_close"] = ixss_on_native_close
_G["ixss_show_wnd"] = ixss_show_wnd
_G["ixss_hide_wnd"] = ixss_hide_wnd
_G["toggle_ixss_window"] = toggle_ixss_window
_G["monitor_vr_state"] = monitor_vr_state

------------------------------------------------------------------------------------------------
add_macro("Imgui Sound Settings: show/hide", "ixss_show_wnd()", "ixss_hide_wnd()", "deactivate")
create_command("fwl_imgui/sound_settings/show_toggle", "open/close Imgui Sound Settings Window", "toggle_ixss_window()", "", "")