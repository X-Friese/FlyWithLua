--- XPLMSpeakString Test
--- by William Good (SparkerInVR)
--- Ver 0.2 
--- 09-11-2022

if not SUPPORTS_FLOATING_WINDOWS then
	logMsg("Upgrade your FlyWithLua! to NG, need Floating Windows")
	return
end


xsst_wnd = float_wnd_create(600, 180, 1, true)
float_wnd_set_position(xsst_wnd, 100, 150)
float_wnd_set_title(xsst_wnd, "XPLMSpeakString Test")
float_wnd_set_imgui_builder(xsst_wnd, "xsst_on_build")
float_wnd_set_onclose(xsst_wnd, "closed_openal_demo")


function closed_openal_demo(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end


function xsst_on_build(xsst_wnd, x, y)
--	imgui.PushItemWidth(50)
	if imgui.Button("Button One", 100, 50) then
		XPLMSpeakString("Button One")
	end
	imgui.SameLine()
	imgui.SetCursorPosX(150)

	if imgui.Button("Button Two", 100, 50) then
		XPLMSpeakString("Button Two")
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 2)

	if imgui.Button("Button Three", 100, 50) then
		XPLMSpeakString("Button Three")
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 3)

	if imgui.Button("Button Four", 100, 50) then
		XPLMSpeakString("Button Four")
	end

	imgui.SetCursorPosY(100)


	if imgui.Button("Button Five", 100, 50) then
		XPLMSpeakString("Button Five")
	end
	imgui.SameLine()
	imgui.SetCursorPosX(150)

	if imgui.Button("Button Six", 100, 50) then
		XPLMSpeakString("Button Six")
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 2)

	if imgui.Button("Button Seven", 100, 50) then
		XPLMSpeakString("Button Seven")
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 3)
	
	if imgui.Button("Button Eight", 100, 50) then
		XPLMSpeakString("Button Eight")
	end

end
