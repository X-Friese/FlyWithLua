-- Enter the button number, you want to use for simple trim here 
simple_trim_button = 330

-- edit the factor, to define simple_trim's sensibility
stg_trim_factor = 1.0

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- leave the rest of the file as it is, if you are not a Lua programmer -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

-- global variables we need
-- all global variables start with 'stg_' to avoid conflicts with other scripts
stg_elv_axis = 1
stg_rud_axis = 2
stg_ail_axis = 3

-- defining the datarefs we need
DataRef("xp_override_joystick", "sim/operation/override/override_joystick", "writable")
DataRef("xp_ail_trim", "sim/flightmodel/controls/ail_trim", "writable")
DataRef("xp_elv_trim", "sim/flightmodel/controls/elv_trim", "writable")
DataRef("xp_rud_trim", "sim/flightmodel/controls/rud_trim", "writable")

-- as we do not know where the axis are located to, we have to find it out
-- datarefs' names starts with std_
function simple_trim_init()
	local axis_function_type_is = 0
	local i
	
	-- take a look where the right axis are
	for i = 0, 499 do
		axis_function_type_is = get("sim/joystick/joystick_axis_assignments", i)
		if axis_function_type_is == 1 then
			DataRef("std_elv_value", "sim/joystick/joystick_axis_values", "readonly", i)
		end
		if axis_function_type_is == 2 then
			DataRef("std_ail_value", "sim/joystick/joystick_axis_values", "readonly", i)
		end
		if axis_function_type_is == 3 then
			DataRef("std_rud_value", "sim/joystick/joystick_axis_values", "readonly", i)
		end
	end
end

function simple_trim()
	-- we will only work when the button is down
	if not button(simple_trim_button) and not last_button(simple_trim_button) then
		return
	end

	-- we start overriding on first press
	if button(simple_trim_button) and not last_button(simple_trim_button) then
		xp_override_joystick = 1
	end

	-- don't forget to reset the override when finished
	if not button(simple_trim_button) and last_button(simple_trim_button) then
		xp_override_joystick = 0
	end
	
	-- while trimming we will write the axis values into the trim datarefs
	-- the axis values run from 0.0 to 1.0, but the trim values run from -1.0 to 1.0
	if button(simple_trim_button) and last_button(simple_trim_button) then
		xp_ail_trim = (std_ail_value * 2.0 - 1.0) * stg_trim_factor
		xp_rud_trim = (std_rud_value * 2.0 - 1.0) * stg_trim_factor
		xp_elv_trim = (std_elv_value * 2.0 - 1.0) * stg_trim_factor
	end
	
	-- store the last value to check out state
	stg_last_button = simple_trim_button
end

simple_trim_init()
do_every_frame("simple_trim()")
