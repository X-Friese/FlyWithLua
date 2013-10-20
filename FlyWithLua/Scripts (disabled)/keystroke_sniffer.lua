keystroke_debug_string = "No key pressed!"
keystroke_sniffer_active = false

function keystroke_sniffer_event_handler()
	if keystroke_sniffer_active then
		if SHIFT_KEY then
			keystroke_debug_string = "Key no. " .. VKEY .. " was pressed (shifted), KEY_ACTION = " .. KEY_ACTION
		else
			keystroke_debug_string = "Key no. " .. VKEY .. " was pressed (unshifted), KEY_ACTION = " .. KEY_ACTION
		end
		RESUME_KEY = true
	end
end

do_on_keystroke("keystroke_sniffer_event_handler()")

function print_keystroke_sniffer_result()
	if keystroke_sniffer_active then
		draw_string(50, 900, keystroke_debug_string)
	end
end

do_every_draw("print_keystroke_sniffer_result()")
add_macro("Show keystoke numbers", "keystroke_sniffer_active = true", "keystroke_sniffer_active = false", "deactivate")