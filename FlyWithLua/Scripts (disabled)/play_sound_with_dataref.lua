-- play_sound_with_dataref.lua
-- William R. Good 06-23-20


local avionics_sound_on = load_WAV_file(SYSTEM_DIRECTORY .. "Resources/sounds/systems/avionics.wav")
  
dataref("avionics_on", "sim/cockpit2/switches/avionics_power_on")
local previous_avionics_on = 0
  
function avionics_sound()

	if avionics_on == 1 and previous_avionics_on == 0 then
		play_sound(avionics_sound_on)
		previous_avionics_on = 1
	end
    if avionics_on == 0 then
		previous_avionics_on = 0
    end
end

do_every_frame("avionics_sound()")
