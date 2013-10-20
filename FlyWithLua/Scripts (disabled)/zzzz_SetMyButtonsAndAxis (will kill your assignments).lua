-- define the joystick offsets (for button assignments)
local Thrustmaster = 0
local Quadrant = 160
local XBox = 320

last_view_type = 0

-- define the DataRef we want to use
DataRef( "view_type", "sim/graphics/view/view_type" )

-- init the view
if get("sim/cockpit/electrical/battery_on") == 1 then
    command_once( "sim/view/3d_cockpit_cmnd_look" )
else
    command_once( "sim/view/chase" )
end

-- do the initial things
clear_all_axis_assignments()
set_axis_assignment(  0, "roll",     "normal" )
set_axis_assignment(  1, "pitch",     "normal" )
set_axis_assignment(  3, "yaw",      "normal" )
set_axis_assignment( 10, "mixture",  "reverse" )
set_axis_assignment( 11, "prop",     "reverse" )
set_axis_assignment( 12, "throttle", "normal" )

clear_all_button_assignments()
set_button_assignment( Thrustmaster + 0,  "FlyWithLua/autopilot/set_autopilot_off" )
set_button_assignment( Thrustmaster + 2,  "sim/view/chase" )
set_button_assignment( Thrustmaster + 3,  "sim/view/chase" )
set_button_assignment( Thrustmaster + 4,  "sim/general/zoom_in_fast" )
set_button_assignment( Thrustmaster + 5,  "sim/general/zoom_out_fast" )
set_button_assignment( Thrustmaster + 6,  "FlyWithLua/autopilot/activate_autopilot" )
set_button_assignment( Thrustmaster + 7,  "FlyWithLua/autopilot/set_autopilot_off" )
set_button_assignment( Thrustmaster + 8,  "sim/view/flashlight_red" )
set_button_assignment( Thrustmaster + 9,  "sim/view/flashlight_wht" )
set_button_assignment( Thrustmaster + 10, "FlyWithLua/debugging/enter_code" )
set_button_assignment( Thrustmaster + 11, "FlyWithLua/debugging/reload_scripts" )
set_button_assignment( Thrustmaster + 13, "FlyWithLua/Ingo/SetQNH" )
set_button_assignment( Thrustmaster + 16, "sim/general/up" )
set_button_assignment( Thrustmaster + 18, "sim/view/night_vision_toggle" )
set_button_assignment( Thrustmaster + 20, "sim/general/down" )
set_button_assignment( Thrustmaster + 22, "FlyWithLua/cockpit/toggle_HASI" )
set_button_assignment( XBox + 0,  "sim/lights/nav_lights_toggle" )
set_button_assignment( XBox + 1,  "sim/lights/beacon_lights_toggle" )
set_button_assignment( XBox + 3,  "sim/lights/strobe_lights_toggle" )
set_button_assignment( XBox + 2,  "sim/lights/landing_lights_toggle" )
set_button_assignment( XBox + 6,  "sim/operation/dev_console" )
set_button_assignment( XBox + 7,  "sim/operation/toggle_full_screen" )
set_button_assignment( XBox + 12, "FlyWithLua/HeliTrim/turn_off_tweak" )
set_button_assignment( XBox + 14, "FlyWithLua/HeliTrim/show_toggle" )
set_button_assignment( XBox + 16, "FlyWithLua/HeliTrim/turn_on_tweak" )
set_button_assignment( Quadrant + 0, "sim/flight_controls/flaps_up" )
set_button_assignment( Quadrant + 1, "sim/flight_controls/flaps_down" )
set_button_assignment( Quadrant + 2, "sim/flight_controls/speed_brakes_up_all" )
set_button_assignment( Quadrant + 3, "sim/flight_controls/speed_brakes_down_all" )
set_button_assignment( Quadrant + 4, "FlyWithLua/HeliTrim/save_settings" )
set_button_assignment( Quadrant + 5, "FlyWithLua/HeliTrim/load_settings" )
set_button_assignment( Quadrant + 6, "FlyWithLua/flight_controls/ARB" )

set( "sim/joystick/joystick_pitch_nullzone",       0.0 )
set( "sim/joystick/joystick_roll_nullzone",        0.0 )
set( "sim/joystick/joystick_heading_nullzone",     0.0 )
set( "sim/joystick/joystick_pitch_augment",        0.2 )
set( "sim/joystick/joystick_roll_augment",         0.2 )
set( "sim/joystick/joystick_heading_augment",      0.2 )
set( "sim/joystick/joystick_pitch_sensitivity",    0.2 )
set( "sim/joystick/joystick_roll_sensitivity",     0.2 )
set( "sim/joystick/joystick_heading_sensitivity",  0.2 )

set( "sim/graphics/view/field_of_view_deg",   72.5 )

-- and do all the every frame stuff
function B2D_update()
	local Thrustmaster = 0
	local Quadrant = 160
	local XBox = 320
	
	-- let button change the view between 3d inside and chase outside
	-- type 1026 is the inside 3d view, type 1023 the HUD
	if (( view_type == 1026 ) and ( last_view_type ~= 1026 )) 
		or (( view_type == 1023 ) and ( last_view_type ~= 1023 )) then
		set_button_assignment( Thrustmaster + 22, "FlyWithLua/cockpit/toggle_HASI" )
		set_button_assignment( Thrustmaster + 18, "sim/view/night_vision_toggle" )
		set_button_assignment( Thrustmaster + 2, "sim/view/chase" )
		set_button_assignment( Thrustmaster + 3, "sim/view/chase" )
	end

	-- type 1017 is the chase view mode
	if ( view_type == 1017 ) and ( last_view_type ~= 1017 ) then
		set_button_assignment( Thrustmaster + 22, "sim/general/left_fast" )
		set_button_assignment( Thrustmaster + 18, "sim/general/right_fast" )
		set_button_assignment( Thrustmaster + 2, "sim/view/forward_with_hud" )
		set_button_assignment( Thrustmaster + 3, "sim/view/3d_cockpit_cmnd_look" )
		set_button_assignment( Thrustmaster + 4, "sim/general/backward" )
		set_button_assignment( Thrustmaster + 5, "sim/general/forward" )
	end

	-- remember some last values for next frame
	last_view_type = view_type
end

-- register the function to be run every frame
do_every_frame( "B2D_update()" )

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- --    All code above represents the dafault setting of the simulator.                           -- --
-- --    The following settings are mede for plane (or helicopter) types, who share a              -- --
-- --    unique setting with all members of this type.                                             -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

-- all helicopter will get a unique setting as default
function set_helicopter_assignments()
    set_axis_assignment(12, "collective", "normal")
    set( "sim/joystick/joystick_pitch_nullzone",       0.0 )
    set( "sim/joystick/joystick_roll_nullzone",        0.0 )
    set( "sim/joystick/joystick_heading_nullzone",     0.0 )
    set( "sim/joystick/joystick_pitch_augment",        0.0 )
    set( "sim/joystick/joystick_roll_augment",         0.0 )
    set( "sim/joystick/joystick_heading_augment",      0.0 )
    set( "sim/joystick/joystick_pitch_sensitivity",    0.0 )
    set( "sim/joystick/joystick_roll_sensitivity",     0.0 )
    set( "sim/joystick/joystick_heading_sensitivity",  0.0 )
    set( "sim/flightmodel/controls/parkbrake", 0.0 )
    set_button_assignment( Thrustmaster + 0, "FlyWithLua/HeliTrim/turn_on_tweak" )
    HeliTrim_sensitivity = 0.65
end

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- --    After the default is set and type defaults are set as functions, we will                  -- --
-- --    define the settings for each of our individual plane or helicopter.                       -- --
-- --    To be plane spezific, we use "if PLANE_ICAO == "xyz" then ... end".                       -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

-- AW139 X-Rotors (A139)
if PLANE_ICAO == "A139" then
    set_helicopter_assignments()
    set( "sim/graphics/view/field_of_view_deg",   85 )
    set( "sim/flightmodel/controls/parkbrake", 1.0 )
end

-- MD902 Explorer (EXPL)
if PLANE_ICAO == "EXPL" then
    set_helicopter_assignments()
end

-- Hudges 300 (Hu30)
if PLANE_ICAO == "Hu30" then
    set_helicopter_assignments()
    set( "sim/graphics/view/field_of_view_deg",   85 )
end

-- Default Robinson R22 (r22)
if PLANE_ICAO == "r22" then
    set_helicopter_assignments()
    set( "sim/graphics/view/field_of_view_deg",   85 )
end

-- Robinson R44 (RH44)
if PLANE_ICAO == "RH44" then
    set_helicopter_assignments()
    set( "sim/graphics/view/field_of_view_deg",   85 )
end

-- Bell 206 Dreamfoil
if PLANE_ICAO == "B06" then
    set_helicopter_assignments()
    set( "sim/graphics/view/field_of_view_deg",   80 )
end

-- EC135 Rotorsim or Jason Chandler
if PLANE_ICAO == "EC35" then
    set_helicopter_assignments()
end

-- EC145 Jason Chandler
if PLANE_ICAO == "EC45" then
    set_helicopter_assignments()
end

-- Hudges 500D
if PLANE_ICAO == "h500" then
    set_helicopter_assignments()
    set_heli_tweak_on(-0.16, -0.08, -0.07)
    set( "sim/graphics/view/field_of_view_deg",   80 )
end

-- Dreamfoil AS350
if PLANE_ICAO == "PT-DHH" then
    set_helicopter_assignments()
    set_heli_tweak_on(-0.16, -0.08, -0.07)
    set( "sim/graphics/view/field_of_view_deg",   80 )
end

-- BO 105
if PLANE_ICAO == "B105" then
    set_helicopter_assignments()
end
-- Carenado C208B
if PLANE_ICAO == "C208" then
    autopilot_helper_vvi = 800
    plane_has_reverser = true
end

-- S-BACH 300
if PLANE_ICAO == "SBACH" then
    plane_has_flaps = false
end

-- Jason Chandler C510
if PLANE_ICAO == "C510" then
    plane_has_speedbrakes = true
end

-- Carenado Bonanza
if PLANE_ICAO == "BE33" then
    set( "sim/graphics/view/field_of_view_deg",   65.0 )
end

-- Cessna C172
if PLANE_ICAO == "C172" then
    set( "sim/graphics/view/field_of_view_deg",   75.0 )
    show_MPPS_instrument = true
end

-- Blackshape Prime
if PLANE_TAILNUMBER == "T7-BLK" then
	set( "sim/flightmodel/weight/m_fixed", 125.0 )
	set( "sim/flightmodel/weight/m_fuel1", 25,0 )
	set( "sim/flightmodel/weight/m_fuel2", 25,0 )
end