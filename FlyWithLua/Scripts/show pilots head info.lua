display_PH_info = false

function show_pilots_head_parameters()
	local x, y, z, heading, pitch = get_pilots_head()
	local info_to_print = string.format("Pilot's head info:   x = %f, y = %f, z = %f, heading = %f, pitch = %f", x, y, z, heading, pitch)
	if display_PH_info then
		XPLMSetGraphicsState(0,0,0,1,1,0,0)
		draw_string_Helvetica_18(200, 300, info_to_print)
	end
end

do_every_draw("show_pilots_head_parameters()")

add_macro("Show pilot's head info", "display_PH_info = true", "display_PH_info = false", "deactivate")