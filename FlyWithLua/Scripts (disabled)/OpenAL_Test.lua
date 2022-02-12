--- OpenAL Test
--- by William Good (SparkerInVR)
--- Ver 0.1 
--- 02-08-2022
if not SUPPORTS_FLOATING_WINDOWS then
	logMsg("Upgrade your FlyWithLua! to NG, need Floating Windows")
	return
end

local Ten = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/10ft.wav")
local Twenty = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/20ft.wav")
local Thirty = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/30ft.wav")
local AutoPilotFail = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/autopilot_fail.wav")
local SinkRate = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/sink.wav")
local SeatBelt = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/seatbelt.wav")
local PullUp = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/pull.wav")
local AutopilotDisco = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/alert/autopilot_disco.wav")



button_pressed = 0

function oalt_on_build(oalt_wnd, x, y)
--	imgui.PushItemWidth(50)
	if imgui.Button("Button One", 100, 50) then
		play_sound(Ten)
	end
	imgui.SameLine()
	imgui.SetCursorPosX(150)

	if imgui.Button("Button Two", 100, 50) then
		play_sound(Twenty)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 2)

	if imgui.Button("Button Three", 100, 50) then
		play_sound(Thirty)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 3)

	if imgui.Button("Button Four", 100, 50) then
		play_sound(AutoPilotFail)
	end

	imgui.SetCursorPosY(100)


	if imgui.Button("Button Five", 100, 50) then
		play_sound(SinkRate)
	end
	imgui.SameLine()
	imgui.SetCursorPosX(150)

	if imgui.Button("Button Six", 100, 50) then
		play_sound(SeatBelt)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 2)

	if imgui.Button("Button Seven", 100, 50) then
		play_sound(PullUp)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 3)
	
	if imgui.Button("Button Eight", 100, 50) then
		play_sound(AutopilotDisco)
	end

end

oalt_wnd = nil

function oalt_show_wnd()
    oalt_wnd = float_wnd_create(640, 480, 1, true)
    float_wnd_set_title(oalt_wnd, "Imgui OpenAL Demo")
    float_wnd_set_imgui_builder(oalt_wnd, "oalt_on_build")
end

function oalt_hide_wnd()
    if oalt_wnd then
        float_wnd_destroy(oalt_wnd)
    end
end

oalt_show_only_once = 0
oalt_hide_only_once = 0

function toggle_imgui_openal_demo()
	oalt_show_window = not oalt_show_window
	if oalt_show_window then
		if oalt_show_only_once == 0 then
			oalt_show_wnd()
			oalt_show_only_once = 1
			oalt_hide_only_once = 0
		end
	else
		if oalt_hide_only_once == 0 then
			oalt_hide_wnd()
			oalt_hide_only_once = 1
			oalt_show_only_once = 0
		end
	end
end


add_macro("Imgui OpenAL Demo: open/close", "oalt_show_wnd()", "oalt_hide_wnd()", "deactivate")
create_command("FlyWithLua/imgui-OpenAL-demo/show_toggle", "open/close imgui OpenAL demo", "toggle_imgui_openal_demo()", "", "")
