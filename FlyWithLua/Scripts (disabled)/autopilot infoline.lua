-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Script:  autopilot infoline
-- Version: 1.0
-- Build:   2012-10-15
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Description:
-- Display autopilot information in external view
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
require "bit"

local flight_director_mode = dataref_table("sim/cockpit2/autopilot/flight_director_mode")
local vvi_dial_fpm = dataref_table("sim/cockpit2/autopilot/vvi_dial_fpm")
local altitude_dial_ft = dataref_table("sim/cockpit2/autopilot/altitude_dial_ft")
local heading_dial_deg_mag_pilot = dataref_table("sim/cockpit2/autopilot/heading_dial_deg_mag_pilot")
local airspeed_dial_kts_mach = dataref_table("sim/cockpit2/autopilot/airspeed_dial_kts_mach")
local altitude_hold_armed = dataref_table("sim/cockpit2/autopilot/altitude_hold_armed")
local autopilot_state = dataref_table("sim/cockpit/autopilot/autopilot_state")
local backcourse_status = dataref_table("sim/cockpit2/autopilot/backcourse_status")
local approach_status = dataref_table("sim/cockpit2/autopilot/approach_status")
local xp_view_is_external = dataref_table("sim/graphics/view/view_is_external")
local autopilot_infostring = ""
local autopilot_infostring2 = ""
local infocolor = "blue"

function draw_autopilot_info()
    if xp_view_is_external[0] == 0 then
        return
    end
    if flight_director_mode[0] == 0 then
        draw_string(5, SCREEN_HIGHT - 10, "Autopilot off", "red")
        return
    end
    if flight_director_mode[0] == 1 then
        autopilot_infostring = "Autopilot: Flight Director Mode"
        infocolor = "yellow"
    else
        autopilot_infostring = "Autopilot: on"
        infocolor = "green"
    end
    
    state = autopilot_state[0]
    if bit.band(state, 1) > 0 then
        autopilot_infostring = autopilot_infostring .. ", Autothrottle"
    end
    if bit.band(state, 2) > 0 then
        autopilot_infostring = autopilot_infostring .. ", Heading Hold"
    end
    if bit.band(state, 4) > 0 then
        autopilot_infostring = autopilot_infostring .. ", Wing Leveler"
    end
    if bit.band(state, 8) > 0 then
        autopilot_infostring = autopilot_infostring .. ", Airspeed Hold With Pitch"
    end
    if bit.band(state, 16) > 0 then
        autopilot_infostring = autopilot_infostring .. ", VVI Climb"
    end
    if bit.band(state, 32) > 0 then
        autopilot_infostring = autopilot_infostring .. ", Altitude Hold Armed"
    end
    if bit.band(state, 64) > 0 then
        autopilot_infostring = autopilot_infostring .. ", Flight Level Change"
    end
    if bit.band(state, 128) > 0 then
        autopilot_infostring = autopilot_infostring .. ", Pitch Sync"
    end
    if bit.band(state, 256) > 0 then
        autopilot_infostring = autopilot_infostring .. ", HNAV Armed"
    end
    if bit.band(state, 512) > 0 then
        autopilot_infostring = autopilot_infostring .. ", HNAV Captured"
    end
    if bit.band(state, 1024) > 0 then
        autopilot_infostring = autopilot_infostring .. ", Glideslope Armed"
    end
    if bit.band(state, 2048) > 0 then
        autopilot_infostring = autopilot_infostring .. ", Glideslope"
    end
    if bit.band(state, 4096) > 0 then
        autopilot_infostring = autopilot_infostring .. ", FMS Armed"
    end
    if bit.band(state, 8192) > 0 then
        autopilot_infostring = autopilot_infostring .. ", FMS Captured"
    end
     if bit.band(state, 16384) > 0 then
        autopilot_infostring = autopilot_infostring .. ", Altitude Hold"
    end
    if bit.band(state, 32768) > 0 then
        autopilot_infostring = autopilot_infostring .. ", Horizontal TOGA"
    end
    if bit.band(state, 65536) > 0 then
        autopilot_infostring = autopilot_infostring .. ", Vertical TOGA"
    end
    if bit.band(state, 131072) > 0 then
        autopilot_infostring = autopilot_infostring .. ", VNAV Armed"
    end
    if bit.band(state, 262144) > 0 then
        autopilot_infostring = autopilot_infostring .. ", VNAV Captured"
    end

    if approach_status[0] == 1 then
        autopilot_infostring = autopilot_infostring .. ", Approach Armed"
    end
    if approach_status[0] == 2 then
        autopilot_infostring = autopilot_infostring .. ", Approach Captured"
    end
    if backcourse_status[0] == 1 then
        autopilot_infostring = autopilot_infostring .. ", Backcourse Armed"
    end
    if backcourse_status[0] == 2 then
        autopilot_infostring = autopilot_infostring .. ", Backcourse Captured"
    end
    
    -- add hdg, alt and vvi
    autopilot_infostring2 = string.format("(HDG=%03i, ALT=%05i, VVI=%+04i, SPD=%03i)",
                                         heading_dial_deg_mag_pilot[0],
                                         altitude_dial_ft[0],
                                         vvi_dial_fpm[0],
                                         airspeed_dial_kts_mach[0])
    draw_string(5, SCREEN_HIGHT - 10, autopilot_infostring, infocolor)
    draw_string(5, SCREEN_HIGHT - 22, autopilot_infostring2, infocolor)
end

do_every_draw("draw_autopilot_info()")