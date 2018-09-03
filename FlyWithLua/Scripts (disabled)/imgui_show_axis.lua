-- Imgui Show Axis
-- William R. Good 09-03-18

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

local Axis_Values = dataref_table("sim/joystick/joystick_axis_values")

local starting_axis = 0

isav_wnd = float_wnd_create(640, 480, 1, true)

float_wnd_set_title(isav_wnd, "Imgui Show Axis Values")

float_wnd_set_imgui_builder(isav_wnd, "isav_on_build")

float_wnd_set_onclose(isav_wnd, "closed_show_axis")

function isav_on_build(isav_wnd, x, y)
	imgui.TextUnformatted("To find the Axis Start Number look in")
	imgui.TextUnformatted("Resources/plugins/FlyWithLua/initial_assignments.txt")

	imgui.TextUnformatted("")
	local changed, newAxisStart = imgui.SliderInt("##E", starting_axis, 0, 490, "Change Axis Start Number: %.0f")
	if changed then
		starting_axis = newAxisStart
	end

	imgui.TextUnformatted("")
	imgui.TextUnformatted("Axis" .. starting_axis .. "  " .. Axis_Values[starting_axis])
	imgui.TextUnformatted("Axis" .. starting_axis + 1 .. "  " .. Axis_Values[starting_axis + 1])
	imgui.TextUnformatted("Axis" .. starting_axis + 2 .. "  " .. Axis_Values[starting_axis + 2])
	imgui.TextUnformatted("Axis" .. starting_axis + 3 .. "  " .. Axis_Values[starting_axis + 3])
	imgui.TextUnformatted("Axis" .. starting_axis + 4 .. "  " .. Axis_Values[starting_axis + 4])
	imgui.TextUnformatted("Axis" .. starting_axis + 5 .. "  " .. Axis_Values[starting_axis + 5])
	imgui.TextUnformatted("Axis" .. starting_axis + 6 .. "  " .. Axis_Values[starting_axis + 6])
	imgui.TextUnformatted("Axis" .. starting_axis + 7 .. "  " .. Axis_Values[starting_axis + 7])
	imgui.TextUnformatted("Axis" .. starting_axis + 8 .. "  " .. Axis_Values[starting_axis + 8])
	imgui.TextUnformatted("Axis" .. starting_axis + 9 .. "  " .. Axis_Values[starting_axis + 9])
	imgui.TextUnformatted("Axis" .. starting_axis + 10 .. "  " .. Axis_Values[starting_axis + 10])

end

function closed_show_axis(wnd)
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end
