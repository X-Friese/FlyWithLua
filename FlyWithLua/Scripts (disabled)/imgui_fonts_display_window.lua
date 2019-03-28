-- Imgui Fonts Display Window
-- William R. Good 03-28-19

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

ifdw_wnd = float_wnd_create(800, 500, 1, true)
float_wnd_set_position(ifdw_wnd, 100, 500)
float_wnd_set_title(ifdw_wnd, "Imgui Fonts Display Window")
float_wnd_set_imgui_builder(ifdw_wnd, "ifdw_on_build")
float_wnd_set_onclose(ifdw_wnd, "closed_hello_world")

function ifdw_on_build(ifdw_wnd, x, y)
	imgui.PushFont(font1)
	imgui.SetWindowFontScale(1.0)
	imgui.TextUnformatted("Font 1 SetWindowFontScale(1.0)\n")
	imgui.PopFont()
	imgui.PushFont(font2)
	imgui.SetWindowFontScale(1.5)
	imgui.TextUnformatted("Font 2 SetWindowFontScale(1.5)\n")
	imgui.PopFont()
	imgui.PushFont(font3)
	imgui.SetWindowFontScale(2.0)
	imgui.TextUnformatted("Font 3 SetWindowFontScale(2.0)\n")
	imgui.PopFont()
	imgui.PushFont(font4)
	imgui.SetWindowFontScale(1.5)
	imgui.TextUnformatted("Font 4 SetWindowFontScale(1.5)\n")
	imgui.PopFont()
	imgui.PushFont(font5)
	imgui.SetWindowFontScale(1.5)
	imgui.TextUnformatted("Font 5 SetWindowFontScale(1.5)\n")
	imgui.PopFont()
	imgui.PushFont(font6)
	imgui.SetWindowFontScale(1.5)
	imgui.TextUnformatted("Font 6 SetWindowFontScale(1.5)\n")
	imgui.PopFont()
	imgui.PushFont(font7)
	imgui.SetWindowFontScale(1.5)
	imgui.TextUnformatted("Font 7 SetWindowFontScale(1.5)\n")
	imgui.PopFont()
	imgui.PushFont(font8)
	imgui.SetWindowFontScale(1.5)
	imgui.TextUnformatted("Font 8 SetWindowFontScale(1.5)\n")
	imgui.PopFont()
	imgui.PushFont(font9)
	imgui.SetWindowFontScale(1.5)
	imgui.TextUnformatted("Font 9 SetWindowFontScale(1.5)\n")
	imgui.PopFont()
	imgui.ShowFontSelector("font selector")
end

function closed_hello_world(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end
