-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- Lua module "SMA_smoother.lua" v1.0  -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
module(..., package.seeall);

function create_SMA(axis_number, samples)
	-- make samples an optional argument
	samples = samples or 10
	
	-- make sure that axis_number is a string
	axis_number = tostring(axis_number)
	
	-- create the code
	local code = 'dataref("real_axis_' .. axis_number
	code = code .. '", "sim/joystick/joystick_axis_values", "readonly", ' .. axis_number .. ')\n'
	code = code .. "local values_axis_" .. axis_number .. " = { }\n"
	code = code .. "for i = 1, " .. samples .. " do\n"
	code = code .. "    values_axis_" .. axis_number .. "[i] = real_axis_" .. axis_number .. "\n"
	code = code .. "end\n"
	code = code .. "axis_" .. axis_number .. " = real_axis_" .. axis_number .. "\n\n"
	code = code .. "function calculate_axis_" .. axis_number .. "()\n"
    code = code .. "    axis_" .. axis_number .. " = real_axis_" .. axis_number .. "\n"
    code = code .. "    for i = 2, " .. samples .. " do\n"
    code = code .. "        axis_" .. axis_number .. " = axis_" .. axis_number .. " + values_axis_" .. axis_number .. "[i]\n"
    code = code .. "        values_axis_" .. axis_number .. "[i-1] = values_axis_" .. axis_number .. "[i]\n"
	code = code .. "    end\n"
	code = code .. "    values_axis_" .. axis_number .. "[" .. samples .. "] = real_axis_" .. axis_number .. "\n"
    code = code .. "    axis_" .. axis_number .. " = axis_" .. axis_number .. " / " .. samples .. "\n"
    code = code .. "end\n\n"
    code = code .. 'do_every_frame("calculate_axis_' .. axis_number .. '()")\n'
	
	-- execute the code
	assert(loadstring(code))()
end