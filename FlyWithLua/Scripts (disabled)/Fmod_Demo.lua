--- Fmod Demo For X-Plane Fmod SDK
--- by William Good (SparkerInVR)
--- Ver 0.4 
--- 02-14-2023


if not SUPPORTS_FLOATING_WINDOWS then
	logMsg("Upgrade your FlyWithLua! to NG, need Floating Windows")
	return
end


local tenft_wav = load_fmod_sound(SYSTEM_DIRECTORY .. "Resources/sounds/alert/10ft.wav")
local twentyft_wav = load_fmod_sound(SYSTEM_DIRECTORY .. "Resources/sounds/alert/20ft.wav")
local thirty_wav = load_fmod_sound(SYSTEM_DIRECTORY .. "Resources/sounds/alert/30ft.wav")
local forthy_wav = load_fmod_sound(SYSTEM_DIRECTORY .. "Resources/sounds/alert/40ft.wav")
local flap_wav = load_fmod_sound(SYSTEM_DIRECTORY .. "Resources/sounds/systems/flap.wav")
local gear_wav = load_fmod_sound(SYSTEM_DIRECTORY .. "Resources/sounds/systems/gear.wav")

dataref("com1_channel_mute", "FlyWithLua_Com1ChannelGroup/Mute", "writable")
dataref("com1_channel_volume", "FlyWithLua_Com1ChannelGroup/Volume", "writable")
dataref("interior_channel_mute", "FlyWithLua_InteriorChannelGroup/Mute", "writable")
dataref("interior_channel_volume", "FlyWithLua_InteriorChannelGroup/Volume", "writable")
dataref("ui_channel_mute", "FlyWithLua_UIChannelGroup/Mute", "writable")
dataref("ui_channel_volume", "FlyWithLua_UIChannelGroup/Volume", "writable")
dataref("master_channel_mute", "FlyWithLua_MasterChannelGroup/Mute", "writable")
dataref("master_channel_volume", "FlyWithLua_MasterChannelGroup/Volume", "writable")


com1_channel_volume = 1.0
interior_channel_volume = 1.0
ui_channel_volume = 1.0
master_channel_volume = 1.0

fmt_wnd = float_wnd_create(560, 500, 1, true)
float_wnd_set_position(fmt_wnd, 150, 150)
float_wnd_set_title(fmt_wnd, "Imgui Fmod Demo")
float_wnd_set_imgui_builder(fmt_wnd, "fmt_on_build")
float_wnd_set_onclose(fmt_wnd, "closed_fmod_demo")


function fmt_on_build(ihd_wnd, x, y)
    imgui.TextUnformatted(string.format("Fmod Demo:\n  wnd=%s\n  x=%d\n  y=%d", ihd_wnd, x, y))
end


function closed_fmod_demo(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end


function fmt_on_build(fmt_wnd, x, y)
--	imgui.PushItemWidth(50)
	if imgui.Button("10ft.wav com1", 100, 50) then
		play_sound_on_com1_bus(tenft_wav)
	end
	imgui.SameLine()
	imgui.SetCursorPosX(150)

	if imgui.Button("20ft.wav int", 100, 50) then
		play_sound_on_interior_bus(twentyft_wav)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 2)

	if imgui.Button("30ft.wav ui", 100, 50) then
	        logMsg("30ft.wav ui")
		play_sound_on_ui_bus(thirty_wav)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 3)

	if imgui.Button("40ft.wav mastr", 100, 50) then
	        logMsg("40ft.wav mastr")
		play_sound_on_master_bus(forthy_wav)
	end

	imgui.SetCursorPosY(75)


	if imgui.Button("flap.wav com1", 100, 50) then
		play_sound_on_com1_bus(flap_wav)
	end
	imgui.SameLine()
	imgui.SetCursorPosX(150)

	if imgui.Button("flap.wav intr", 100, 50) then
		play_sound_on_interior_bus(flap_wav)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 2)

	if imgui.Button("flap.wav ui", 100, 50) then
		play_sound_on_ui_bus(flap_wav)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 3)
	
	if imgui.Button("flap.wav mstr", 100, 50) then
		play_sound_on_master_bus(flap_wav)
	end

	imgui.SetCursorPosY(150)
		
	if imgui.Button("gear.wav com1", 100, 50) then
		play_sound_on_com1_bus(gear_wav)
	end
	
	
	imgui.SameLine()
	imgui.SetCursorPosX(150)
	
	if imgui.Button("gear.wav intr", 100, 50) then
		play_sound_on_interior_bus(gear_wav)
	end
	
	
	imgui.SameLine()
	imgui.SetCursorPosX(150 * 2)
	
	if imgui.Button("gear.wav ui", 100, 50) then
		play_sound_on_ui_bus(gear_wav)
	end
	
	
	imgui.SameLine()
	imgui.SetCursorPosX(150 * 3)
	
	if imgui.Button("gear.wav mstr", 100, 50) then
		play_sound_on_master_bus(gear_wav)
	end
	
		
	imgui.SetCursorPosY(225)
		
	if imgui.Button("StopCom1Sound", 100, 50) then
		stop_sound_on_com1_bus()
	end
	
	imgui.SameLine()
	imgui.SetCursorPosX(150)

	if imgui.Button("StopIntSound", 100, 50) then
		stop_sound_on_interior_bus()
	end

	
	imgui.SameLine()
	imgui.SetCursorPosX(300)
	
	if imgui.Button("StopUiSound", 100, 50) then
		stop_sound_on_ui_bus()
	end
	
	
	imgui.SameLine()
	imgui.SetCursorPosX(450)
	
	if imgui.Button("StopMstrSound", 100, 50) then
		stop_sound_on_master_bus()
	end
	
	
	imgui.SetCursorPosY(300)
	
	local changed, newVal = imgui.SliderFloat("", com1_channel_volume, 0, 5, "Com1 Channel Volume: %.2f")

	if changed then
		com1_channel_volume = newVal
	end
	
	
--	imgui.SameLine()
	
	if imgui.RadioButton("Mute Off    ", com1_channel_mute == 0) then
            com1_channel_mute = 0
        end
        
        imgui.SameLine()
        
        if imgui.RadioButton("Mute On    ", com1_channel_mute == 1) then
            com1_channel_mute = 1
        end
        
        
        imgui.SetCursorPosY(350)
        
        
	local changed, newVal = imgui.SliderFloat("  ", interior_channel_volume, 0, 5, "Interior Channel Volume: %.2f")

	if changed then
		interior_channel_volume = newVal
	end

	
	if imgui.RadioButton("Mute Off   ", interior_channel_mute == 0) then
            interior_channel_mute = 0
        end
        
        imgui.SameLine()
        
        if imgui.RadioButton("Mute On   ", interior_channel_mute == 1) then
            interior_channel_mute = 1
        end

        
        imgui.SetCursorPosY(400)                
        
	
	local changed, newVal = imgui.SliderFloat("   ", ui_channel_volume, 0, 5, "Ui Channel Volume: %.2f")

	if changed then
		ui_channel_volume = newVal
	end
	
	
	if imgui.RadioButton("Mute Off  ", ui_channel_mute == 0) then
            ui_channel_mute = 0
        end
        
        imgui.SameLine()
        
        if imgui.RadioButton("Mute On  ", ui_channel_mute == 1) then
            ui_channel_mute = 1
        end
        
        
        imgui.SetCursorPosY(450)                
        
	
	local changed, newVal = imgui.SliderFloat("    ", master_channel_volume, 0, 5, "Master Channel Volume: %.2f")

	if changed then
		master_channel_volume = newVal
	end
	
	
	if imgui.RadioButton("Mute Off", master_channel_mute == 0) then
            master_channel_mute = 0
        end
        
        imgui.SameLine()
        
        if imgui.RadioButton("Mute On", master_channel_mute == 1) then
            master_channel_mute = 1
        end
end
