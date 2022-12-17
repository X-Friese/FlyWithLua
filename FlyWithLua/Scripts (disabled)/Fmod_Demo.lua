--- Fmod Demo
--- by William Good (SparkerInVR)
--- Ver 0.2 
--- 08-25-2022


if not SUPPORTS_FLOATING_WINDOWS then
	logMsg("Upgrade your FlyWithLua! to NG, need Floating Windows")
	return
end


local tenft_wav = load_fmod_sound(SYSTEM_DIRECTORY .. "Resources/sounds/alert/10ft.wav")
local twentyft_wav = load_fmod_sound(SYSTEM_DIRECTORY .. "Resources/sounds/alert/20ft.wav")
local stall_wav = load_fmod_sound(SYSTEM_DIRECTORY .. "Resources/sounds/alert/stall.wav")
local sink_wav = load_fmod_sound(SYSTEM_DIRECTORY .. "Resources/sounds/alert/sink.wav")
local pull_wav = load_fmod_sound(SYSTEM_DIRECTORY .. "Resources/sounds/alert/pull.wav")
local mini_wav = load_fmod_sound(SYSTEM_DIRECTORY .. "Resources/sounds/alert/mini.wav")
local tcas_wav = load_fmod_sound(SYSTEM_DIRECTORY .. "Resources/sounds/alert/tcas.wav")
local APU_wav = load_fmod_sound(SYSTEM_DIRECTORY .. "Resources/sounds/systems/APU.wav")
local flap_wav = load_fmod_sound(SYSTEM_DIRECTORY .. "Resources/sounds/systems/flap.wav")
local gear_wav = load_fmod_sound(SYSTEM_DIRECTORY .. "Resources/sounds/systems/gear.wav")
local clunck_wav = load_fmod_sound(SYSTEM_DIRECTORY .. "Resources/sounds/contact/clunck.wav")
local boom3_wav = load_fmod_sound(SYSTEM_DIRECTORY .. "Resources/sounds/contact/boom3.wav")


dataref("master_channel_mute", "FlyWithLua_MasterChannelGroup/Mute", "writable")
dataref("master_channel_volume", "FlyWithLua_MasterChannelGroup/Volume", "writable")
dataref("radios_channel_mute", "FlyWithLua_RadiosChannelGroup/Mute", "writable")
dataref("radios_channel_volume", "FlyWithLua_RadiosChannelGroup/Volume", "writable")
dataref("interior_channel_mute", "FlyWithLua_InteriorChannelGroup/Mute", "writable")
dataref("interior_channel_volume", "FlyWithLua_InteriorChannelGroup/Volume", "writable")

master_channel_volume = 1.0
radios_channel_volume = 1.0
interior_channel_volume = 1.0

fmt_wnd = float_wnd_create(600, 450, 1, true)
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
	if imgui.Button("10ft.wav mastr", 100, 50) then
		play_sound_on_master_bus(tenft_wav)
	end
	imgui.SameLine()
	imgui.SetCursorPosX(150)

	if imgui.Button("20ft.wav rad", 100, 50) then
		play_sound_on_radios_bus(twentyft_wav)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 2)

	if imgui.Button("stall.wav int", 100, 50) then
		play_sound_on_interior_bus(stall_wav)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 3)

	if imgui.Button("sink.wav rad", 100, 50) then
		play_sound_on_radios_bus(sink_wav)
	end

	imgui.SetCursorPosY(75)


	if imgui.Button("pull.wav mastr", 100, 50) then
		play_sound_on_master_bus(pull_wav)
	end
	imgui.SameLine()
	imgui.SetCursorPosX(150)

	if imgui.Button("mini.wav rad", 100, 50) then
		play_sound_on_radios_bus(mini_wav)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 2)

	if imgui.Button("tcas.wav int", 100, 50) then
		play_sound_on_interior_bus(tcas_wav)
	end

	imgui.SameLine()
	imgui.SetCursorPosX(150 * 3)
	
	if imgui.Button("APU.wav rad", 100, 50) then
		play_sound_on_radios_bus(APU_wav)
	end

	imgui.SetCursorPosY(150)
		
	if imgui.Button("flap.wav mastr", 100, 50) then
		play_sound_on_master_bus(flap_wav)
	end
	
	
	imgui.SameLine()
	imgui.SetCursorPosX(150)
	
	if imgui.Button("gear.wav rad", 100, 50) then
		play_sound_on_radios_bus(gear_wav)
	end
	
	
	imgui.SameLine()
	imgui.SetCursorPosX(150 * 2)
	
	if imgui.Button("clunck.wav int", 100, 50) then
		play_sound_on_interior_bus(clunck_wav)
	end
	
	
	imgui.SameLine()
	imgui.SetCursorPosX(150 * 3)
	
	if imgui.Button("boom3.wav rad", 100, 50) then
		play_sound_on_radios_bus(boom3_wav )
	end
	
		
	imgui.SetCursorPosY(225)
		
	if imgui.Button("StopMastSound", 100, 50) then
		stop_sound_on_master_bus()
	end
	
	imgui.SameLine()
	imgui.SetCursorPosX(150)

	if imgui.Button("StopRadSound", 100, 50) then
		stop_sound_on_radios_bus()
	end

	
	imgui.SameLine()
	imgui.SetCursorPosX(300)
	
	if imgui.Button("StopIntSound", 100, 50) then
		stop_sound_on_interior_bus()
	end	

	
	imgui.SetCursorPosY(300)
	
	local changed, newVal = imgui.SliderFloat("   ", master_channel_volume, 0, 5, "Master Channel Volume: %.2f")

	if changed then
		master_channel_volume = newVal
	end
	
	
	imgui.SameLine()
	
	if imgui.RadioButton("Mute Off", master_channel_mute == 0) then
            master_channel_mute = 0
        end
        
        imgui.SameLine()
        
        if imgui.RadioButton("Mute On", master_channel_mute == 1) then
            master_channel_mute = 1
        end
        
        
        imgui.SetCursorPosY(350)
        
        
	local changed, newVal = imgui.SliderFloat("  ", radios_channel_volume, 0, 5, "Radios Channel Volume: %.2f")

	if changed then
		radios_channel_volume = newVal
	end
	
	
	imgui.SameLine()
	
	if imgui.RadioButton("Mute Off ", radios_channel_mute == 0) then
            radios_channel_mute = 0
        end
        
        imgui.SameLine()
        
        if imgui.RadioButton("Mute On ", radios_channel_mute == 1) then
            radios_channel_mute = 1
        end
        
        
        imgui.SetCursorPosY(400)                
        
	
	local changed, newVal = imgui.SliderFloat("", interior_channel_volume, 0, 5, "Interior Channel Volume: %.2f")

	if changed then
		interior_channel_volume = newVal
	end
	
	
	imgui.SameLine()
	
	if imgui.RadioButton("Mute Off  ", interior_channel_mute == 0) then
            interior_channel_mute = 0
        end
        
        imgui.SameLine()
        
        if imgui.RadioButton("Mute On  ", interior_channel_mute == 1) then
            interior_channel_mute = 1
        end
end
