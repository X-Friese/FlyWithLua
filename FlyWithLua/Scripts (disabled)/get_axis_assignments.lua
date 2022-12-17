-- This script gets all your axis assignments from X-Plane an prints them into the Log.txt file
-- Set up your axis with X-Plane's joystick config menu and run this macro after that
-- You can copy&paste the code written in Log.txt into a Lua script file, to save your axis setup

function get_axis_assignments()
	-- X-Plane only stores an integer to represent the function, we use a table to give it a name
	local axis_functions = { "Pitch", "Roll", "Yaw", "Throttle", "Collective", "Left toe brake", "Right toe brake", "Prop",
	"Mixture", "Carb heat", "Flaps", "Thrust vector", "Wing sweep", "Speedbrakes", "Displacement",
	"Reverse", "Elevator trim", "Aileron trim", "Rudder trim", "Throttle 1", "Throttle 2", "Throttle 3",
	"Throttle 4", "Prop 1", "Prop 2", "Prop 3", "Prop 4", "Mixture 1", "Mixture 2",
	"Mixture 3", "Mixture 4", "Reverse 1", "Reverse 2", "Reverse 3", "Reverse 4", "Landing gear",
	"Nosewheel tiller", "Backup throttle", "Cowl flaps", "none", "View left/right", "View up/down", "View zoom",
	"Camera left/right", "Camera up/down", "Camera zoom", "Gun/bomb left/right", "Gun/bomb up/down",
	"VR Touchpad X", "VR Touchpad Y", "VR Trigger", "Custom command(s)", "Throttle 5", "Throttle 6", "Throttle 7",
	"Throttle 8", "Cowl flaps 1", "Cowl flaps 2", "Cowl flaps 3", "Cowl flaps 4", "Cowl flaps 5", "Cowl flaps 6",
	"Cowl flaps 7", "Cowl flaps 8", "Throttle Vertical", "Throttle Horizontal", "Copilot Pitch", "Copilot Roll",
	"Copilot Yaw", "Copilot Left toe brake", "Copilot Right toe brake" }

	-- now we can write the axis config to the Log.txt file
	logMsg("FlyWithLua debug: the following code sets your axis to there actual setup")
	logMsg("-- -- 8< -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- ")
	logMsg("clear_all_axis_assignments()")

	-- make the variables local to keep Lua clean
	local i
	local axis_function_index
	local axis_reverse
	
	-- look into all 499 axis
	for i = 0, 499 do
		-- pull the index to the function name from X-Plane
		axis_function_index = get( "sim/joystick/joystick_axis_assignments", i )
		-- pull the reverse info from X-Plane
		-- the value is 0, if the axis is normal, but is the number of the axis, if the axis is reverse
		if get( "sim/joystick/joystick_axis_reverse", i ) > 0 then
			axis_reverse = "reverse"
		else
			axis_reverse = "normal"
		end
		-- we do not need to kill axis twice, this was already done by clear_all_axis_assignments()
		if axis_function_index > 0 then
			logMsg( "set_axis_assignment( " .. i .. ', "' .. axis_functions[axis_function_index] .. '", "' .. axis_reverse .. '" )' )
		end
	end
	logMsg("-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- >8 -- -- ")
end

add_macro( "write axis assignments to Log", "get_axis_assignments()" )
