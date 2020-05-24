keystroke_debug_string = "No key pressed!"
keystroke_sniffer_active = false

function keystroke_sniffer_event_handler()
	if keystroke_sniffer_active then
		if SHIFT_KEY then
			keystroke_debug_string = "Key no. " .. VKEY .. " was pressed (shifted, "
		else
			keystroke_debug_string = "Key no. " .. VKEY .. " was pressed (unshifted, "
		end
		if OPTION_KEY then
			keystroke_debug_string = keystroke_debug_string .. "with option key down and "
		else
			keystroke_debug_string = keystroke_debug_string .. "with option key up and "
		end
		if CONTROL_KEY then
			keystroke_debug_string = keystroke_debug_string .. "control key down), KEY_ACTION = " .. KEY_ACTION
		else
			keystroke_debug_string = keystroke_debug_string .. "control key up), KEY_ACTION = " .. KEY_ACTION
		end
		RESUME_KEY = true
	end
end

do_on_keystroke("keystroke_sniffer_event_handler()")

function print_keystroke_sniffer_result()
	if keystroke_sniffer_active then
		glColor4f(0, 0, 0, 255)
        draw_string_Helvetica_18(50-1, SCREEN_HIGHT-130-1, keystroke_debug_string)
        draw_string_Helvetica_18(50+1, SCREEN_HIGHT-130+1, keystroke_debug_string)
        draw_string_Helvetica_18(50+1, SCREEN_HIGHT-130-1, keystroke_debug_string)
        draw_string_Helvetica_18(50-1, SCREEN_HIGHT-130+1, keystroke_debug_string)
        glColor4f(255, 255, 255, 255)
		draw_string_Helvetica_18(50, SCREEN_HIGHT-130, keystroke_debug_string)
	end
end

do_every_draw("print_keystroke_sniffer_result()")
add_macro("Show keystoke numbers", "keystroke_sniffer_active = true", "keystroke_sniffer_active = false", "deactivate")
