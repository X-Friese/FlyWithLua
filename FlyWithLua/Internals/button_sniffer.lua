button_debug_string = "No button pressed!"
button_sniffer_active = false

function button_sniffer_event_handler()
    if button_sniffer_active then
        print_message = false
        for i = 0, 3199, 1 do
            if not last_button(i) and button(i) then
                print_message = true
                button_debug_string = "Button no. " .. i .. " was pressed."
            end
            if last_button(i) and not button(i) then
                print_message = true
                button_debug_string = "Button no. " .. i .. " was released."
            end
        end
        if print_message then print(button_debug_string) end
	end
end

do_every_frame("button_sniffer_event_handler()")

function print_button_sniffer_result()
    if button_sniffer_active then
        glColor4f(0, 0, 0, 255)
        draw_string_Helvetica_18(50-1, SCREEN_HIGHT-100-1, button_debug_string)
        draw_string_Helvetica_18(50+1, SCREEN_HIGHT-100+1, button_debug_string)
        draw_string_Helvetica_18(50+1, SCREEN_HIGHT-100-1, button_debug_string)
        draw_string_Helvetica_18(50-1, SCREEN_HIGHT-100+1, button_debug_string)
        glColor4f(255, 255, 255, 255)
        draw_string_Helvetica_18(50, SCREEN_HIGHT-100, button_debug_string)
	end
end

do_every_draw("print_button_sniffer_result()")
add_macro("Show joystick button numbers", "button_sniffer_active = true", "button_sniffer_active = false", "deactivate")
