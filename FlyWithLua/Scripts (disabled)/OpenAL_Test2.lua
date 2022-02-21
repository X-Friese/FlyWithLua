--- OpenAL Test2
--- by William Good (SparkerInVR)
--- Ver 0.1 
--- 02-14-2022
if not SUPPORTS_FLOATING_WINDOWS then
	logMsg("Upgrade your FlyWithLua! to NG, need Floating Windows")
	return
end

local Forty = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/40ft.wav")
local Fifty = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/50ft.wav")
local OneHundred = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/100ft.wav")
local GlideSlope = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/glideslope.wav")
local LandingGear = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/gear_warn_1.wav")
local Minumans = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/mini.wav")
local Stall = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/stall.wav")
local WindShear = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/wshr.wav")


oalt2_wnd = float_wnd_create(640, 480, 1, true)
float_wnd_set_position(oalt2_wnd, 400, 150)
float_wnd_set_title(oalt2_wnd, "Imgui OpenAL Demo2")
float_wnd_set_imgui_builder(oalt2_wnd, "oalt2_on_build")
float_wnd_set_onclose(oalt2_wnd, "closed_openal_demo")


function oalt2_on_build(ihd_wnd, x, y)
    imgui.TextUnformatted(string.format("OpenAL Demo:\n  wnd=%s\n  x=%d\n  y=%d", ihd_wnd, x, y))
end


function closed_openal_demo(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end


function oalt2_on_build(oalt2_wnd, x, y)
--	imgui.PushItemWidth(50)
	if imgui.Button("Button One", 100, 50) then
		play_sound(Forty)
	end
	imgui.SameLine()
	imgui.SetCursorPosX(150)

	if imgui.Button("Button Two", 100, 50) then
		play_sound(Fifty)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 2)

	if imgui.Button("Button Three", 100, 50) then
		play_sound(OneHundred)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 3)

	if imgui.Button("Button Four", 100, 50) then
		play_sound(GlideSlope)
	end

	imgui.SetCursorPosY(100)


	if imgui.Button("Button Five", 100, 50) then
		play_sound(LandingGear)
	end
	imgui.SameLine()
	imgui.SetCursorPosX(150)

	if imgui.Button("Button Six", 100, 50) then
		play_sound(Minumans)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 2)

	if imgui.Button("Button Seven", 100, 50) then
		play_sound(Stall)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 3)
	
	if imgui.Button("Button Eight", 100, 50) then
		play_sound(WindShear)
	end

end
