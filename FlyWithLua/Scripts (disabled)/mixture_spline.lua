if PLANE_ICAO == "C172"
or PLANE_ICAO == "M20J"
or PLANE_ICAO == "BE33"
or PLANE_ICAO == "PA28"
or PLANE_TAILNUMBER == "N399CK"
then
	dataref("xp_mixture_ratio_all", "sim/cockpit2/engine/actuators/mixture_ratio_all", "writable")
	
	local axis_function_type_is = 0
	local i
	
	-- take a look where the mixture axis is
	for i = 0, 99 do
		axis_function_type_is = get("sim/joystick/joystick_axis_assignments", i)
		if axis_function_type_is == 9 then
			dataref("mixture_axis", "sim/joystick/joystick_axis_values", "readonly", i)
		end
	end
	
	-- get control over the mixture
	set("sim/operation/override/override_mixture", 1)
	
	--   *** the mixture formula ***
	-- when the mixture axis is full down, it's value is 1.0
	-- when the mixture axis is full up, it's value is 0.0
	-- we are searching for a function through these points: (0.0, 1.0), (0.5, 0.75), (1.0, 0.0)
	-- if you are not familiar with mathematics, use this website to calculate the function:
	-- http://www.arndt-bruenner.de/mathe/10/parabeldurchdreipunkte.htm#rechner
	function mixture_spline()
		xp_mixture_ratio_all = 1.0 - mixture_axis * mixture_axis
	end
	
	do_every_frame("mixture_spline()")
else
	set("sim/operation/override/override_mixture", 0)
end