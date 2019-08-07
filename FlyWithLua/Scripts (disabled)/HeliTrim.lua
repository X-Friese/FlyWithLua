-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Script:  HeliTrim
-- Author:  Carsten Lynker
-- Version: 1.2
-- Build:   2019-07-19
-- Licence: Same as FlyWithLua (MIT licence)
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Description:
-- Display steering information and provide a logical center for joystick axis.
-- Please read the manual, you find it inside the Documentation subfolder.
-- Updated for X-Plane 11 by Daikan from x-plane.org
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
require "graphics"

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Position of the dignostic display elements (center point in screen coordinates).
-- Edit the values to replace the disgnostic display and it's alpha blending.
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

-- local variable result in a better performance than global variables
local HFXO, HFYO, HFXS, HFYS, HFXH, HFYH, HFXT, HFYT

-- When the simulator starts or the user changes the window size (if X-Plane isn't fullscreen),
-- it uses wrong screen coordinates. As a solution, we use a repaeted function to set the values.
function set_helitrim_display_position()
    -- View of the helicopter from above
    HFXO = 100
    HFYO = SCREEN_HIGHT - 100
    -- View of the helicopter from the right side
    HFXS = 250
    HFYS = SCREEN_HIGHT - 100
    -- View of the helicopter from behind
    HFXH = 100
    HFYH = SCREEN_HIGHT - 250
    -- Trim and stick info area
    HFXT = 250
    HFYT = SCREEN_HIGHT - 250
    -- Alpha value for the diagnostic display (OpenGL alpha blending, range 0.0 = invisible to 1.0 = full solid)
    HFALPHA = 0.25
end

-- Fly a helicopter like a precision engineer (value should be 0.5 or less) or like a rodeo braver (value = 1.0)
HeliTrim_sensitivity = 1.0

-- The diagnostic display is off by default, set it from "false" to "true" if you want it on by default
show_HF = false

-- -- -- 8< -- -- -- -- end of area a normal user should edit -- -- -- -- -- -- -- -- -- -- -- -- -- -- --


-- init the values
set_helitrim_display_position()
-- repeat init every 10 seconds
do_sometimes("set_helitrim_display_position()")

-- create a custom command to toggle diagnostic display view
function toggle_trim_info()
    if show_HF then
        show_HF = false
    else
        show_HF = true
    end
end

create_command("FlyWithLua/HeliTrim/show_toggle", "Toggle showing of trim info", "toggle_trim_info()", "", "")

-- all DataRefs we need (this is a mordern style script)
dataref("force_P", "sim/flightmodel/position/P")
dataref("force_Q", "sim/flightmodel/position/Q")
dataref("force_R", "sim/flightmodel/position/R")
dataref("xp_groundspeed", "sim/flightmodel/position/groundspeed")
dataref("xp_hpath", "sim/flightmodel/position/hpath")
dataref("xp_psi", "sim/flightmodel/position/psi")
dataref("xp_vh_ind_fpm2", "sim/flightmodel/position/vh_ind_fpm2")
dataref("aileron_trim", "sim/cockpit2/controls/aileron_trim")
dataref("elevator_trim", "sim/cockpit2/controls/elevator_trim")
dataref("rudder_trim", "sim/cockpit2/controls/rudder_trim")
dataref("acf_min_trim_elev", "sim/aircraft/controls/acf_min_trim_elev", "writable")
dataref("acf_max_trim_elev", "sim/aircraft/controls/acf_max_trim_elev", "writable")
dataref("acf_min_trim_ailn", "sim/aircraft/controls/acf_min_trim_ailn", "writable")
dataref("acf_max_trim_ailn", "sim/aircraft/controls/acf_max_trim_ailn", "writable")
dataref("acf_min_trim_rudd", "sim/aircraft/controls/acf_min_trim_rudd", "writable")
dataref("acf_max_trim_rudd", "sim/aircraft/controls/acf_max_trim_rudd", "writable")
dataref("yoke_pitch_ratio", "sim/joystick/yoke_pitch_ratio", "writable")
dataref("yoke_roll_ratio", "sim/joystick/yoke_roll_ratio", "writable")
dataref("yoke_heading_ratio", "sim/joystick/yoke_heading_ratio", "writable")
dataref("xp_override_joystick", "sim/operation/override/override_joystick", "writable")
dataref("frame_rate_period","sim/operation/misc/frame_rate_period")

-- the next function is copied from SimpleTrim
-- we use the same name as SimpleTrim, to avoid multiple access to a dataref, even if it's allowed when readonly
function heli_trim_init()
	-- set the DataRefs
	DataRef("std_elv_value", "sim/joystick/joy_mapped_axis_value", "readonly", 1)
	DataRef("std_ail_value", "sim/joystick/joy_mapped_axis_value", "readonly", 2)
	DataRef("std_rud_value", "sim/joystick/joy_mapped_axis_value", "readonly", 3)
end

-- and we have to run the init function one time
heli_trim_init()

-- init the tweaked logical center to the physical center of the stick
heli_trim_tweak = false
HT_overlay_elv = 0
HT_overlay_ail = 0
HT_overlay_rud = 0

-- copy the actual position of the stick into a new logical center and turn HeliTrim on
function turn_on_heli_tweak()
    HT_overlay_elv = yoke_pitch_ratio
    HT_overlay_ail = yoke_roll_ratio
    HT_overlay_rud = yoke_heading_ratio
    heli_trim_tweak = true
end

-- turn of and reset stick values
function turn_off_heli_tweak()
    heli_trim_tweak = false
    HT_overlay_elv = 0
    HT_overlay_ail = 0
    HT_overlay_rud = 0
    yoke_pitch_ratio = 0
    yoke_roll_ratio = 0
    yoke_heading_ratio = 0
end

-- HeliTrim is off by default
turn_off_heli_tweak()

-- we need a global variable to indicate that the user holds down the HeliTrim activation button
-- as long as holds the button down, HeliTrim won't add the stick onto the logical center
pause_heli_tweak = false

-- providing on/off custom commands
create_command("FlyWithLua/HeliTrim/turn_on_tweak", "turn on adding the joystick overlay", "turn_on_heli_tweak()", "pause_heli_tweak = true", "pause_heli_tweak = false")
create_command("FlyWithLua/HeliTrim/turn_off_tweak", "turn off adding the joystick overlay", "turn_off_heli_tweak()", "", "")

-- now follows the hear of HeliTrim, the every frame loop faking the center position
function heli_overlay_tweak()
    -- does the HeliTrim magic has to be done?
    if heli_trim_tweak and not pause_heli_tweak then
        -- intercept direct connection between X-Plane and the stick(s)
        xp_override_joystick = 1
        -- the values from std_..._value range from 0.0 to 1.0, but we need -1.0 to 1.0
        -- these inputs from the stick(s) are reduced by the sesitivity factor and added to the logical center values
        yoke_pitch_ratio   = std_elv_value * HeliTrim_sensitivity + HT_overlay_elv
        yoke_roll_ratio    = std_ail_value * HeliTrim_sensitivity + HT_overlay_ail
        yoke_heading_ratio = std_rud_value * HeliTrim_sensitivity + HT_overlay_rud
        -- are we beyond the range -1.0 to 1.0, the limits of X-Plane?
        if yoke_pitch_ratio   < -1 then yoke_pitch_ratio   = -1 end
        if yoke_pitch_ratio   >  1 then yoke_pitch_ratio   =  1 end
        if yoke_roll_ratio    < -1 then yoke_roll_ratio    = -1 end
        if yoke_roll_ratio    >  1 then yoke_roll_ratio    =  1 end
        if yoke_heading_ratio < -1 then yoke_heading_ratio = -1 end
        if yoke_heading_ratio >  1 then yoke_heading_ratio =  1 end
    -- does we have to freeze to the logical center?
    elseif pause_heli_tweak then
        xp_override_joystick = 1
        yoke_pitch_ratio   = HT_overlay_elv
        yoke_roll_ratio    = HT_overlay_ail
        yoke_heading_ratio = HT_overlay_rud
    -- connect X-Plane direct to the stick(s) if HeliTrim is off
    else
        xp_override_joystick = 0
    end
end

do_every_frame("heli_overlay_tweak()")

-- blow up the range of the autopilot (X-Plane's autopilot uses the trim system a heli doesn't have)
function maximize_all_trim()
    acf_min_trim_elev = 1
    acf_max_trim_elev = 1
    acf_min_trim_ailn = 1
    acf_max_trim_ailn = 1
    acf_min_trim_rudd = 1
    acf_max_trim_rudd = 1
end

-- provide these nonsense as a simple macro
add_macro("Maximize all trim axis", "maximize_all_trim()")

-- the function to show the diagnostic display
function show_heli_forces()
    -- init OpenGL
    XPLMSetGraphicsState(0, 0, 0, 1, 1, 0, 0)
    
    -- draw the helicopter
    graphics.set_color(1,1,1,HFALPHA)
    -- view from above
    graphics.draw_circle(HFXO, HFYO, 19, 2)
    graphics.draw_line(HFXO, HFYO-20, HFXO, HFYO-60, 2)
    graphics.draw_line(HFXO-5, HFYO-60, HFXO, HFYO-60, 2)
    graphics.draw_line(HFXO-5, HFYO-50, HFXO-5, HFYO-68, 2)
    graphics.draw_line(HFXO-30, HFYO-30, HFXO+30, HFYO+30, 2)
    graphics.draw_line(HFXO-30, HFYO+30, HFXO+30, HFYO-30, 2)
    -- side view
    graphics.draw_circle(HFXS, HFYS, 18, 2)
    graphics.draw_line(HFXS-19, HFYS, HFXS-58.5, HFYS, 2)
    graphics.draw_line(HFXS-40, HFYS+20, HFXS+40, HFYS+20, 2)
    graphics.draw_line(HFXS-65, HFYS-5, HFXS-55, HFYS+5, 2)
    graphics.draw_line(HFXS-65, HFYS+5, HFXS-55, HFYS-5, 2)
    -- view from behind
    graphics.draw_circle(HFXH, HFYH, 18, 2)
    graphics.draw_filled_circle(HFXH, HFYH, 4, 2)
    graphics.draw_line(HFXH-5, HFYH+5, HFXH-5, HFYH-5, 2)
    graphics.draw_line(HFXH-40, HFYH+20, HFXH+40, HFYH+20, 2)
    
    -- draw spinning speeds
    graphics.set_color(1,0,0,HFALPHA)
    if math.abs(force_P) > 0.1 then
        if force_P < 0 then
            graphics.draw_angle_arrow(HFXH, HFYH+20, 270, force_P * -5, 8, 3)
        else
            graphics.draw_angle_arrow(HFXH, HFYH+20, 90, force_P * 5, 8, 3)
        end
    end
    if math.abs(force_P) > 0.1 then
        if force_Q < 0 then
            graphics.draw_angle_arrow(HFXS-60, HFYS, 0, force_Q * -5, 8, 3)
        else
            graphics.draw_angle_arrow(HFXS-60, HFYS, 180, force_Q * 5, 8, 3)
        end
    end
    if math.abs(force_P) > 0.05 then
        if force_R < 0 then
            graphics.draw_angle_arrow(HFXO-5, HFYO-60, 90, force_R * -10, 8, 3)
        else
            graphics.draw_angle_arrow(HFXO-5, HFYO-60, 270, force_R * 10, 8, 3)
        end
    end
    
    -- limit movement speeds (as they are datarefs, we copy them into local variables)
    local lgs = xp_groundspeed * 5
    if lgs > 80 then lgs = 80 end
    local gs_angle = xp_hpath-xp_psi
    local lvz = xp_vh_ind_fpm2
    if lvz < -600 then lvz = -600 end
    if lvz >  600 then lvz =  600 end
    

    -- draw relative movement speeds
    graphics.set_color(0,1,0,HFALPHA)
    graphics.draw_angle_arrow(HFXO, HFYO, gs_angle, lgs)
    if math.abs(lvz) > 15 then
        graphics.set_color(1,1,0,HFALPHA)
        if lvz < 0 then
            graphics.draw_angle_arrow(HFXS, HFYS, 180, lvz * -0.1)
            graphics.draw_angle_arrow(HFXH, HFYH, 180, lvz * -0.1)
        else
            graphics.draw_angle_arrow(HFXS, HFYS, 0, lvz * 0.1)
            graphics.draw_angle_arrow(HFXH, HFYH, 0, lvz * 0.1)
        end
    end
    
    -- draw the trim info frame
    graphics.set_color(1,1,1,HFALPHA/4)
    graphics.draw_rectangle(HFXT-100*acf_max_trim_ailn, HFYT-100*acf_max_trim_elev, HFXT+100*acf_min_trim_ailn, HFYT+100*acf_min_trim_elev)
    graphics.draw_filled_arc(HFXT, HFYT, -90*acf_min_trim_rudd, 90*acf_max_trim_rudd, 100)
    graphics.set_color(1,1,1,HFALPHA)
    graphics.draw_line(HFXT-100, HFYT, HFXT+100, HFYT)
    graphics.draw_line(HFXT, HFYT-100, HFXT, HFYT+100)
    
    -- checkout the limits
    local acf_trim_elev
    local acf_trim_aileron
    local acf_trim_rudd
    if elevator_trim < 0 then
        acf_trim_elev = acf_min_trim_elev
    else
        acf_trim_elev = acf_max_trim_elev
    end
    if aileron_trim < 0 then
        acf_trim_aileron = acf_min_trim_ailn
    else
        acf_trim_aileron = acf_max_trim_ailn
    end
    if rudder_trim < 0 then
        acf_trim_rudd = acf_min_trim_rudd
    else
        acf_trim_rudd = acf_max_trim_rudd
    end
    
    -- draw trim info cross and arrow
    graphics.set_color(1,0,1,HFALPHA)
    graphics.set_width(2)
    graphics.draw_line(HFXT+100*aileron_trim*acf_trim_aileron-10, HFYT-100*elevator_trim*acf_trim_elev, HFXT+100*aileron_trim*acf_trim_aileron+10, HFYT-100*elevator_trim*acf_trim_elev)
    graphics.draw_line(HFXT+100*aileron_trim*acf_trim_aileron, HFYT-100*elevator_trim*acf_trim_elev-10, HFXT+100*aileron_trim*acf_trim_aileron, HFYT-100*elevator_trim*acf_trim_elev+10)
    graphics.draw_angle_arrow(HFXT, HFYT, rudder_trim*90*acf_trim_rudd, 100, 20, 2)
    
    -- draw stick info
    graphics.set_color(0,1,0,HFALPHA)
    graphics.set_width(2)
    graphics.draw_line(HFXT+100*yoke_roll_ratio-10, HFYT-100*yoke_pitch_ratio, HFXT+100*yoke_roll_ratio+10, HFYT-100*yoke_pitch_ratio)
    graphics.draw_line(HFXT+100*yoke_roll_ratio, HFYT-100*yoke_pitch_ratio-10, HFXT+100*yoke_roll_ratio, HFYT-100*yoke_pitch_ratio+10)
    graphics.draw_angle_arrow(HFXT, HFYT, yoke_heading_ratio*90, 100, 20, 2)
end

do_every_draw("if show_HF then show_heli_forces() end")

function show_heli_tweak_text_info()
    -- draw tweaking info test
    if heli_trim_tweak then
        draw_string(SCREEN_WIDTH/3, SCREEN_HIGHT-10, "HeliTrim is tweaking axis!", "grey")
    end
end

do_every_draw("show_heli_tweak_text_info()")

-- provide custom commands to save and load the logical center

local HT_saved_elv = 0
local HT_saved_ail = 0
local HT_saved_rud = 0

function save_heli_tweaks()
    HT_saved_elv = HT_overlay_elv
    HT_saved_ail = HT_overlay_ail
    HT_saved_rud = HT_overlay_rud
    XPLMSpeakString("Heli axis tweak saved.")
    logMsg(string.format("FlyWithLua info: HeliTweak axis positions saved (elv = %f, ail = %f, rud = %f)", HT_saved_elv, HT_saved_ail, HT_saved_rud))
end

create_command("FlyWithLua/HeliTrim/save_settings", "save heli tweak settings", "save_heli_tweaks()", "", "")

function load_heli_tweaks()
    HT_overlay_elv = HT_saved_elv
    HT_overlay_ail = HT_saved_ail
    HT_overlay_rud = HT_saved_rud
    heli_trim_tweak = true
    XPLMSpeakString("Heli axis tweak loaded.")
end

create_command("FlyWithLua/HeliTrim/load_settings", "save heli tweak settings", "load_heli_tweaks()", "", "")

-- provide a Lua function for advanced scripting

function set_heli_tweak_on(elv, ail, rud)
    HT_saved_elv = elv
    HT_saved_ail = ail
    HT_saved_rud = rud
    HT_overlay_elv = elv
    HT_overlay_ail = ail
    HT_overlay_rud = rud
    heli_trim_tweak = true
end
