-- Imugi Show Hide Window With Command
-- William R. Good 03-22-19

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

function ishwwc_on_build(ishwwc_wnd, x, y)
	imgui.TextUnformatted("This is a test window")
end

ishwwc_wnd = nil

function ishwwc_show_wnd()
    ishwwc_wnd = float_wnd_create(640, 480, 1, true)
    float_wnd_set_title(ishwwc_wnd, "Imgui Show Hide Window With Command")
    float_wnd_set_imgui_builder(ishwwc_wnd, "ishwwc_on_build")
end

function ishwwc_hide_wnd()
    if ishwwc_wnd then
        float_wnd_destroy(ishwwc_wnd)
    end
end

ishwwc_show_only_once = 0
ishwwc_hide_only_once = 0

function toggle_imgui_show_hide_window_with_command()
	ishwwc_show_window = not ishwwc_show_window
	if ishwwc_show_window then
		if ishwwc_show_only_once == 0 then
			ishwwc_show_wnd()
			ishwwc_show_only_once = 1
			ishwwc_hide_only_once = 0
		end
	else
		if ishwwc_hide_only_once == 0 then
			ishwwc_hide_wnd()
			ishwwc_hide_only_once = 1
			ishwwc_show_only_once = 0
		end
	end
end

add_macro("Imgui Show Hide Window With Command: open/close", "ishwwc_show_wnd()", "ishwwc_hide_wnd()", "deactivate")
create_command("FlyWithLua/imgui-show-hide-window-with-command/show_toggle", "open/close umgui show hide window", "toggle_imgui_show_hide_window_with_command()", "", "")
