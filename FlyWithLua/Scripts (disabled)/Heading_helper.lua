DataRef( "HH_heading", "sim/cockpit/autopilot/heading", "readonly" )
last_heading = HH_heading
heading_display_time = os.clock()

function heading_helper()
	-- if the heading has changed, we want to display the info for 3 sec
	if math.abs(HH_heading - last_heading) > 1.0 then
		heading_display_time = os.clock() + 3.0
	end
	-- we place the info above the mouse pointer
	if os.clock() < heading_display_time then
		glColor4f(0,0,0,0.75)
		glRectf(MOUSE_X - 100, MOUSE_Y + 10, MOUSE_X + 90, MOUSE_Y + 50)
		glColor4f(1,1,1,1)
		draw_string_Helvetica_18(MOUSE_X - 90, MOUSE_Y + 20, "Your Heading is " .. math.floor(HH_heading + 0.5) )
		last_heading = HH_heading
	end
end

do_every_draw("heading_helper()")