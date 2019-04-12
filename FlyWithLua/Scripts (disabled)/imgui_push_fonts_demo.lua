-- Imgui Push Fonts Demo
-- William R. Good 04-12-19

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

ipfd_wnd = float_wnd_create(400, 400, 1, true)
float_wnd_set_position(ipfd_wnd, 100, 100)
float_wnd_set_title(ipfd_wnd, "Imgui Push Fonts Demo")
float_wnd_set_imgui_builder(ipfd_wnd, "ipfd_on_build")
float_wnd_set_onclose(ipfd_wnd, "closed_hello_world")
DejaVuSansMono = float_wnd_load_font(ipfd_wnd, "./Resources/fonts/DejaVuSansMono.ttf", 13)
DejaVuSans = float_wnd_load_font(ipfd_wnd, "./Resources/fonts/DejaVuSans.ttf", 13)

function ipfd_on_build(ipfd_wnd, x, y)
--	imgui.PushFont(DejaVuSansMono)
    imgui.TextUnformatted("imgui.PushFont(DejaVuSansMono)")
--	imgui.PopFont()
--	imgui.PushFont(DejaVuSans)
	imgui.TextUnformatted("imgui.PushFont(DejaVuSans)")
--	imgui.PopFont()
	imgui.SetWindowFontScale(1.0)
	imgui.ShowFontSelector("font selector")
end

function closed_hello_world(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end
