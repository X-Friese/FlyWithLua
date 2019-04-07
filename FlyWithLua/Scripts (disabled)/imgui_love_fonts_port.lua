-- Imgui Love Fonts Port
-- William R. Good 04-05-19

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

-- local DejaVuSansMono = AddFontFromFileTTF(DejaVuSansMono.ttf", 13)

ilfp_wnd = float_wnd_create(400, 400, 1, true)
float_wnd_set_position(ilfp_wnd, 100, 100)
float_wnd_set_title(ilfp_wnd, "Imgui Love Fonts Port")
float_wnd_set_imgui_builder(ilfp_wnd, "ilfp_on_build")
float_wnd_set_onclose(ilfp_wnd, "closed_hello_world")

function ilfp_on_build(ilfp_wnd, x, y)
--	imgui.PushFont(DejaVuSansMono)
	imgui.SetWindowFontScale(1.0)
    imgui.TextUnformatted(string.format("SetWindowFontScale(1.0):\n  wnd=%s\n  x=%d\n  y=%d", ilfp_wnd, x, y))
--	imgui.PopFont()
	imgui.SetWindowFontScale(2.0)
	imgui.TextUnformatted(string.format("SetWindowFontScale(2.0):\n  wnd=%s\n  x=%d\n  y=%d", ilfp_wnd, x, y))
	imgui.SetWindowFontScale(1.0)
	imgui.ShowFontSelector("font selector")
	
end

function closed_hello_world(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end
