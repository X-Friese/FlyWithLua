-- Imgui Buttons Demo using imgui
-- William R. Good 08-29-18

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end



img_id = float_wnd_load_image(SCRIPT_DIRECTORY .. "/Radio_Entry_Window.jpg")

img_id2 = float_wnd_load_image("./Resources/bitmaps/cockpit/buttons/starter/but_starter_BC.png")


button_pressed = 0

function ibd_on_build(ibd_wnd, x, y)
--	imgui.PushItemWidth(50)
	if imgui.Button("Button One", 100, 50) then

	end
	imgui.SameLine()
	imgui.SetCursorPosX(150)

	if imgui.Button("Button Two", 100, 50) then

	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 2)

	if imgui.Button("Button Three", 100, 50) then

	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 3)

	if imgui.Button("Button Four", 100, 50) then

	end

	imgui.SetCursorPosY(100)


	if imgui.Button("Button Five", 100, 50) then

	end
	imgui.SameLine()
	imgui.SetCursorPosX(150)

	if imgui.Button("Button Six", 100, 50) then

	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 2)

	if imgui.Button("Button Seven", 100, 50) then

	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 3)
	
	if imgui.Button("Button Eight", 100, 50) then

	end

	imgui.SetCursorPosY(100 * 2 )

	if imgui.ImageButton(img_id, 170, 128) then
		button_pressed = button_pressed + 1
	end

	imgui.SameLine()

	if imgui.ImageButton(img_id2, 68, 68) then
--		button_pressed = button_pressed + 1
	end
--	imgui.TextUnformatted("ImageButton Pressed  " .. button_pressed .. "  Times")


	imgui.TextUnformatted("ImageButton Pressed  " .. button_pressed .. "  Times")




end

ibd_wnd = nil

function ibd_show_wnd()
    ibd_wnd = float_wnd_create(640, 480, 1, true)
    float_wnd_set_title(ibd_wnd, "Imgui Button Demo")
    float_wnd_set_imgui_builder(ibd_wnd, "ibd_on_build")
end

function ibd_hide_wnd()
    if ibd_wnd then
        float_wnd_destroy(ibd_wnd)
    end
end

ibd_show_only_once = 0
ibd_hide_only_once = 0

function toggle_imgui_button_demo()
	ibd_show_window = not ibd_show_window
	if ibd_show_window then
		if ibd_show_only_once == 0 then
			ibd_show_wnd()
			ibd_show_only_once = 1
			ibd_hide_only_once = 0
		end
	else
		if ibd_hide_only_once == 0 then
			ibd_hide_wnd()
			ibd_hide_only_once = 1
			ibd_show_only_once = 0
		end
	end
end


add_macro("Imgui Button Demo: open/close", "ibd_show_wnd()", "ibd_hide_wnd()", "deactivate")
create_command("FlyWithLua/imgui-button-demo/show_toggle", "open/close imgui button demo", "toggle_imgui_button_demo()", "", "")

