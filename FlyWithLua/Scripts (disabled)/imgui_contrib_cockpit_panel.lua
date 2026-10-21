-- Avionics & Cockpit Console Demo for FlyWithLua
-- Demonstrates practical cockpit instrumentation using:
--   1. ImGui::Markdown:: (Annunciator pills, badges, status colors, bold/italic telemetry)
--   2. ImGui::TimedTooltip:: (Stationary, non-jittering hover tooltips with Markdown)
--   3. Tactile Widgets (Animated ToggleButton, SliderWithReset, PercentSlider, Mouse-Wheel Chasers)

if not SUPPORTS_FLOATING_WINDOWS then
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

local panel_wnd = float_wnd_create(580, 520, 1, true)
float_wnd_set_position(panel_wnd, 120, 120)
float_wnd_set_title(panel_wnd, "Avionics Contrib Console")
float_wnd_set_imgui_builder(panel_wnd, "panel_on_build")
float_wnd_set_onclose(panel_wnd, "panel_on_close")

-- Avionics simulation state
local ap_master          = false
local flight_director    = true
local autothrottle       = false
local nav_source_idx     = 2 -- 1: NAV1, 2: GPS/FMS, 3: VOR2
local nav_sources        = { "NAV 1 (ILS)", "GPS / FMS", "VOR 2" }
local target_altitude    = 14000
local target_heading     = 270
local baro_inHg          = 29.92
local flaps_pct          = 0.0
local trim_setting       = 0.0

function panel_on_build(wnd, x, y)
    -- Top Annunciator Banner
    imgui.MD.Text("# Primary Flight Guidance <badge blue>XP12.4.4+</badge>")
    
    if ap_master then
        imgui.MD.Text("System Status: <pill green>AUTOPILOT ENGAGED</pill>  |  Nav: <badge purple>%s</badge>", nav_sources[nav_source_idx])
    else
        imgui.MD.Text("System Status: <pill yellow>MANUAL FLIGHT</pill>  |  Nav: <badge purple>%s</badge>", nav_sources[nav_source_idx])
    end
    imgui.Separator()

    -- Section 1: Tactile Animated Switches
    imgui.Spacing()
    imgui.MD.Text("### Mode Selectors")

    local ap_changed, new_ap = imgui.ToggleButton("Autopilot Master##ap", ap_master)
    if ap_changed then ap_master = new_ap end
    imgui.TimedTooltip.MD.Text("### Autopilot Master Control\n\n" ..
                               "• Coupled lateral & vertical guidance.\n" ..
                               "• <color #00bc8c>Disengages on pilot yoke force override.</color>")

    imgui.SameLine()
    imgui.SetCursorPosX(300)
    local fd_changed, new_fd = imgui.ToggleButton("Flight Director##fd", flight_director)
    if fd_changed then flight_director = new_fd end
    imgui.TimedTooltip.Text("Toggles PFD flight director command bars.")

    local at_changed, new_at = imgui.ToggleButton("Auto-Throttle##at", autothrottle)
    if at_changed then autothrottle = new_at end
    imgui.TimedTooltip.MD.Text("### Auto-Throttle Servo Engagement\n\n" ..
                               "<pill error>WARNING</pill> Confirm speed bug set before arming!")

    imgui.Spacing()
    imgui.Separator()

    -- Section 2: Mouse-Wheel Tuning Knobs
    imgui.MD.Text("### Guidance Rotary Adjusters (Scroll Mouse Wheel to Tune)")

    -- Target Altitude (Step 500 ft)
    imgui.Button(string.format("Altitude Target: %5d ft (Scroll Wheel)##alt", target_altitude), 340, 32)
    local alt_changed, new_alt = imgui.AdjustItemOnMouseWheel(target_altitude, 0, 45000, 500)
    if alt_changed then target_altitude = math.floor(new_alt) end
    imgui.TimedTooltip.Text("Hover and scroll mouse wheel up/down to adjust altitude in 500 ft increments.")

    -- Target Heading (Step 1 deg, wrap 1..360)
    imgui.Button(string.format("Heading Bug: %03d° (Scroll Wheel)##hdg", target_heading), 340, 32)
    local hdg_changed, new_hdg = imgui.AdjustItemOnMouseWheel(target_heading, 1, 360, 1)
    if hdg_changed then
        target_heading = math.floor(new_hdg)
        if target_heading > 360 then target_heading = 1 end
        if target_heading < 1 then target_heading = 360 end
    end
    imgui.TimedTooltip.Text("Hover and scroll mouse wheel to swing heading bug in 1° steps.")

    -- Navigation Source Cycler
    imgui.Button(string.format("Active Nav Source: [%s] (Scroll to Cycle)##nav", nav_sources[nav_source_idx]), 340, 32)
    local nav_changed, new_nav = imgui.CycleItemOnMouseWheel(nav_source_idx, #nav_sources, true)
    if nav_changed then nav_source_idx = new_nav end
    imgui.TimedTooltip.MD.Text("### Active Navigation Coupling\n\n" ..
                               "Scroll mouse wheel over button to cycle through: `NAV1`, `GPS`, or `VOR2`.")

    imgui.Spacing()
    imgui.Separator()

    -- Section 3: Sliders with Reset and Percent Sliders
    imgui.MD.Text("### Flight Control Surfaces & Calibration")

    -- Flaps percent slider
    local flaps_changed, new_flaps = imgui.PercentSlider("Flaps Deflection", flaps_pct)
    if flaps_changed then flaps_pct = new_flaps end
    imgui.TimedTooltip.Text("Displays 0.0 to 1.0 normalized value formatted as clean 0%..100%.")

    -- Elevator trim with reset button (↺)
    local trim_changed, new_trim = imgui.SliderWithReset("Elevator Trim", trim_setting, -1.0, 1.0, 0.0)
    if trim_changed then trim_setting = new_trim end
    imgui.TimedTooltip.MD.Text("### Elevator Pitch Trim\n\n" ..
                               "• Current: `%.2f`\n" ..
                               "• Click the adjacent **↺** reset icon to snap back to neutral `0.00`.", trim_setting)

    imgui.Spacing()
    imgui.Separator()

    -- Section 4: External Web Link
    imgui.TextLink("FlyWithLua GitHub Project", "https://github.com/sparker256/FlyWithLua")
    imgui.SameLine()
    imgui.TextUnformatted(" | ")
    imgui.SameLine()
    imgui.TextLink("X-Plane Scenery & Aircraft Gateway", "https://gateway.x-plane.com")
end

function panel_on_close(wnd)
end
