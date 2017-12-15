-- This script gets all your axis assignments from X-Plane an prints them into the Log.txt file
-- Set up your axis with X-Plane's joystick config menu and run this macro after that
-- You can copy&paste the code written in Log.txt into a Lua script file, to save your axis setup

function get_axis_assignments()
	-- X-Plane only stores an integer to represent the function, we use a table to give it a name
	local axis_functions = { "pitch", "roll", "yaw", "throttle", "collective", "left toe brake", "right toe brake", "prop",
	"mixture", "carb heat", "flaps", "thrust vector", "wing sweep", "speedbrakes", "displacement",
	"reverse", "elev trim", "ailn trim", "rudd trim", "throttle 1", "throttle 2", "throttle 3",
	"throttle 4", "prop 1", "prop 2", "prop 3", "prop 4", "mixture 1", "mixture 2",
	"mixture 3", "mixture 4", "reverse 1", "reverse 2", "reverse 3", "reverse 4", "landing gear",
	"nosewheel tiller", "backup throttle", "auto roll", "auto pitch", "view left/right", "view up/down", "view zoom" }

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
