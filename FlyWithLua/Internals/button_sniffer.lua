button_debug_string = "No button pressed!"
button_sniffer_active = false

function button_sniffer_event_handler()
	if button_sniffer_active then
        for i = 0, 3199, 1 do
            if not last_button(i) and button(i) then
                button_debug_string = "Button no. " .. i .. " was pressed."
            end
            if last_button(i) and not button(i) then
                button_debug_string = "Button no. " .. i .. " was released."
            end
        end
	end
end

do_every_frame("button_sniffer_event_handler()")

function print_button_sniffer_result()
	if button_sniffer_active then
		draw_string(50, 850, button_debug_string)
	end
end

do_every_draw("print_button_sniffer_result()")
add_macro("Show joystick button numbers", "button_sniffer_active = true", "button_sniffer_active = false", "deactivate")
