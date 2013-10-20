-- Joystick Sensitivity
-- (c) 2013 Ingo Alm, Carsten Lynker
-- build 2013-01-31

DataRef( "pitch_sensitivity", "sim/joystick/joystick_pitch_sensitivity", "writable", 0 )
DataRef( "roll_sensitivity", "sim/joystick/joystick_roll_sensitivity", "writable", 0 )
DataRef( "heading_sensitivity", "sim/joystick/joystick_heading_sensitivity", "writable", 0 )
DataRef( "pitch_augment", "sim/joystick/joystick_pitch_augment", "writable", 0 )
DataRef( "roll_augment", "sim/joystick/joystick_roll_augment", "writable", 0 )
DataRef( "heading_augment", "sim/joystick/joystick_heading_augment", "writable", 0 )
DataRef( "indicated_airspeed", "sim/flightmodel/position/indicated_airspeed", "readonly", 0 )

-- turn off this service by default (we turn it on later with a menu item)
simple_smooth_stick = false

local min_percent = 0.50
local max_percent = 0.80
local percent = 0

-- set max speed as Vne - 20%
local max_speed = get("sim/aircraft/view/acf_Vne") * 0.8

function sensitivity()
	-- should we work?
	if not simple_smooth_stick then return end
	
	percent = indicated_airspeed / max_speed
	
	if ((percent > min_percent) and (percent < max_percent)) then
		pitch_sensitivity = percent
		roll_sensitivity = percent
		heading_sensitivity = percent
		-- augment should be 20% lower than sensitivity (to start with 30%)
		pitch_augment = percent - 0.20
		roll_augment = percent - 0.20
		heading_augment = percent -0.20
	elseif percent < min_percent then
		pitch_sensitivity = min_percent
		roll_sensitivity = min_percent
		heading_sensitivity = min_percent
		pitch_augment = min_percent - 0.20
		roll_augment = min_percent - 0.20
		heading_augment = min_percent -0.20
	else
		pitch_sensitivity = max_percent
		roll_sensitivity = max_percent
		heading_sensitivity = max_percent
		pitch_augment = max_percent - 0.20
		roll_augment = max_percent - 0.20
		heading_augment = max_percent -0.20
	end
end

do_often("sensitivity()")

-- turn it on by default using a macro menu item
add_macro("smooth the stick", "simple_smooth_stick = true", "simple_smooth_stick = false", "activate")