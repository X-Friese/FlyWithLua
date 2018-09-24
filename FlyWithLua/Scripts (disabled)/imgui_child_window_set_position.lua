-- Imgui Child Window Set Position
-- William R. Good 09-23-18

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end


icwsp_wnd = float_wnd_create(640, 480, 1, true)

float_wnd_set_title(icwsp_wnd, "Imgui Child Window Set Position")

float_wnd_set_position(icwsp_wnd, 775, 650)

float_wnd_set_imgui_builder(icwsp_wnd, "icwsp_on_build")

float_wnd_set_onclose(icwsp_wnd, "closed_imgui_child_window_set_position")

function icwsp_on_build(icwsp_wnd, x, y)
--	imgui.PushStyleColor(imgui.constant.Col.ChildBg, 0xFF000000) -- Black Background
--	imgui.PushStyleColor(imgui.constant.Col.ChildBg, 0xFFFFFFFF) -- White Background
--	imgui.PushStyleColor(imgui.constant.Col.ChildBg, 0xFF0000FF) -- Red Background
--	imgui.PushStyleColor(imgui.constant.Col.ChildBg, 0xFF00FF00) -- Green Background
--	imgui.PushStyleColor(imgui.constant.Col.ChildBg, 0xFFFF0000) -- Blue Background
	imgui.PushStyleColor(imgui.constant.Col.ChildBg, 0xFFA89300) -- Strong Artic Blue Background
--	imgui.PushStyleColor(imgui.constant.Col.ChildBg, 0xFFA8A800) -- Strong Cyan Background

	imgui.BeginChild("test", 640, 480)
	imgui.PopStyleColor()
	if imgui.Button("Button One") then  -- Standard size button

	end
	imgui.SameLine()
	imgui.SetCursorPosX(150)  -- Control where next button will be placed

	if imgui.Button("Button Two", 100, 50) then  -- Bigger than normal sized button

	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 2)

	if imgui.Button("Button Three", 100, 50) then

	end

	imgui.EndChild()	

end


function closed_imgui_child_window_set_position(wnd)
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end
