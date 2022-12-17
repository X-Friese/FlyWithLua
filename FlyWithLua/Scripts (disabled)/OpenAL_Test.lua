--- OpenAL Test
--- by William Good (SparkerInVR)
--- Ver 0.2 
--- 09-11-2022

if not SUPPORTS_FLOATING_WINDOWS then
	logMsg("Upgrade your FlyWithLua! to NG, need Floating Windows")
	return
end

local Ten = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/10ft.wav")
local Fire_Bell = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/fire_bell.wav")
local SeatBelt = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/seatbelt.wav")
local Transonic = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/transonic.wav")
local Boom1 = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/contact/boom1.wav")
local Boom2 = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/contact/boom2.wav")
local Boom3 = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/contact/boom3.wav")
local Boom4 = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/contact/boom4.wav")


oalt_wnd = float_wnd_create(640, 480, 1, true)
float_wnd_set_position(oalt_wnd, 100, 150)
float_wnd_set_title(oalt_wnd, "Imgui OpenAL Demo")
float_wnd_set_imgui_builder(oalt_wnd, "oalt_on_build")
float_wnd_set_onclose(oalt_wnd, "closed_openal_demo")


function oalt_on_build(ihd_wnd, x, y)
    imgui.TextUnformatted(string.format("OpenAL Demo:\n  wnd=%s\n  x=%d\n  y=%d", ihd_wnd, x, y))
end


function closed_openal_demo(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end


function oalt_on_build(oalt_wnd, x, y)
--	imgui.PushItemWidth(50)
	if imgui.Button("Button One", 100, 50) then
		play_sound(Ten)
	end
	imgui.SameLine()
	imgui.SetCursorPosX(150)

	if imgui.Button("Button Two", 100, 50) then
		play_sound(Fire_Bell)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 2)

	if imgui.Button("Button Three", 100, 50) then
		play_sound(SeatBelt)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 3)

	if imgui.Button("Button Four", 100, 50) then
		play_sound(Transonic)
	end

	imgui.SetCursorPosY(100)


	if imgui.Button("Button Five", 100, 50) then
		play_sound(Boom1)
	end
	imgui.SameLine()
	imgui.SetCursorPosX(150)

	if imgui.Button("Button Six", 100, 50) then
		play_sound(Boom2)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 2)

	if imgui.Button("Button Seven", 100, 50) then
		play_sound(Boom3)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 3)
	
	if imgui.Button("Button Eight", 100, 50) then
		play_sound(Boom4)
	end

end
