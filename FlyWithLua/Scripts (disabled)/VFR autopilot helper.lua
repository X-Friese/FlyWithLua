-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Script:  VFR autopilot helper
-- Version: 1.1
-- Build:   2012-10-15
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Description:
-- This script provides a command to start the autopilot and cofigure the autopilot to fly
-- with the actual heading and altitude. While the autopilot is running, all axis from the
-- joystick controlling pitch, roll and yaw are disabled.
-- Pitch axis will control altitude and roll axis will control heading while autopilot is on.
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

-- define a global variable to be used in other scripts
autopilot_helper_vvi = 600

-- define the connection to the datarefs
local plane_PSI =  dataref_table("sim/flightmodel/position/psi")
local autopilot_mode = dataref_table("sim/cockpit/autopilot/autopilot_mode")
local HDG = dataref_table("sim/cockpit/autopilot/heading")
local ALT = dataref_table("sim/cockpit/autopilot/altitude")
local VVI = dataref_table("sim/cockpit2/autopilot/vvi_dial_fpm")
local override_joystick = dataref_table("sim/operation/override/override_joystick")
local frame_time = dataref_table("sim/operation/misc/frame_rate_period")
local axis = dataref_table("sim/joystick/joystick_axis_values")
local axis_assignment = dataref_table("sim/joystick/joystick_axis_assignments")
local pitch = 0
local roll = 0
local tweak_is_working = false

-- find the pitch and roll axis
for i = 0, 99 do
    if axis_assignment[i] == 1 then pitch = i end
    if axis_assignment[i] == 2 then roll = i end
end

function activate_autopilot_tweak()
    HDG[0] = plane_PSI[0]
    command_once( "sim/autopilot/reentry" )
    command_once( "sim/autopilot/heading" )
    command_once( "sim/autopilot/altitude_hold" )
    autopilot_mode[0] = 2
    override_joystick[0] = 1
    tweak_is_working = true
end

function stop_autopilot_tweak()
    autopilot_mode[0] = 0
    override_joystick[0] = 0
    tweak_is_working = false
end

-- provide a custom command to activate it
create_command( "FlyWithLua/autopilot/activate_autopilot", "activate autopilot and set actual heading and altitude",
                "activate_autopilot_tweak()", "", "" )

create_command( "FlyWithLua/autopilot/set_autopilot_off", "deactivate autopilot tweak and get back normal steeringe",
                "stop_autopilot_tweak()", "", "" )

-- use a switch
autopilot_helper_active = true

-- check the autopilot state
function autopilot_helper_callback()
    if tweak_is_working == false then return end
    if (autopilot_mode[0] == 2) and (autopilot_helper_active == true) then
        ALT[0] = ALT[0] + (axis[pitch] - 0.5) * frame_time[0] * 1000
        HDG[0] = HDG[0] + (axis[roll]  - 0.5) * frame_time[0] * 100
    end
    if (ALT[0] > ELEVATION*3.2808399 + 100) and (VVI[0] ~= autopilot_helper_vvi) then
        command_once( "sim/autopilot/vertical_speed" )
        VVI[0] = autopilot_helper_vvi
    end
    if (ALT[0] < ELEVATION*3.2808399 - 100) and (VVI[0] ~= -autopilot_helper_vvi) then
        command_once( "sim/autopilot/vertical_speed" )
        VVI[0] = -autopilot_helper_vvi
    end
end

do_every_frame("autopilot_helper_callback()")

-- airplanes D-Axxx, D-Bxx and D-Cxxx will fly IFR
if XPLMGetDataf(XPLMFindDataRef("sim/aircraft/weight/acf_m_max")) < 5701 then
    add_macro("use VFR autopilot tweak", "autopilot_helper_active = true", "autopilot_helper_active = false", "activate")
else
    add_macro("use VFR autopilot tweak", "autopilot_helper_active = true", "autopilot_helper_active = false", "deactivate")
end