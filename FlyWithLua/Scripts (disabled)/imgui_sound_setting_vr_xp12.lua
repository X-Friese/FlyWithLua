--[[
IMGUI Blank Template
Author: Joe Kipfer 2019-06-06
Use in conjuction with Folko's IMGUI Demo script for some great examples and explaination.
When Using IMGUI Demo script mentioned above, don't forget to put the imgui demo.jpg in with it or
you'll get an error.
sparker added link to SimVRlabs Lua Scripting BEGINNER tutorial - Get Scripty episode 6
https://www.youtube.com/watch?time_continue=3&v=1KIi8HsP1jo
Starting with Joe's initial script you can now adjust Sound Settings on the fly.
]]

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop with old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
  return
end
-----------------------------------Variables go here--------------------------------------------
--Set you variables here, datarefs, etc...
-- 
dataref("master_volume", "sim/operation/sound/master_volume_ratio", "writable")
dataref("aircraft_exterior_volume", "sim/operation/sound/exterior_volume_ratio", "writable")
dataref("aircraft_interior_volume", "sim/operation/sound/interior_volume_ratio", "writable")
dataref("pilot_volume", "sim/operation/sound/pilot_volume_ratio", "writable")
dataref("copilot_volume", "sim/operation/sound/copilot_volume_ratio", "writable")
dataref("radio_volume", "sim/operation/sound/radio_volume_ratio", "writable")
dataref("enviro_volume", "sim/operation/sound/enviro_volume_ratio", "writable")
dataref("ui_volume", "sim/operation/sound/ui_volume_ratio", "writable")


-- dataref("copilot_audio_panel_volume", "sim/operation/sound/radio_copilot_audio_panel_volume_ratio", "writable")
-- dataref("engine_volume", "sim/operation/sound/engine_volume_ratio", "writable")
-- dataref("prop_volume", "sim/operation/sound/prop_volume_ratio", "writable")
-- dataref("ground_volume", "sim/operation/sound/ground_volume_ratio", "writable")
-- dataref("weather_volume", "sim/operation/sound/weather_volume_ratio", "writable")
-- dataref("warning_volume", "sim/operation/sound/warning_volume_ratio", "writable")
-- dataref("fan_volume", "sim/operation/sound/fan_volume_ratio", "writable")


-------------------------------------Build Your GUI Here----------------------------------------

function issv_on_build(issv_wnd, x, y)  --<-- your GUI code goes in this section.
  
	-- The following function creates a slider:
	local changed, newVal = imgui.SliderFloat("", master_volume, 0, 1, "Master Volume Value: %.2f")
	-- The first parameter is the caption of the slider, it will be displayed right of the slider.
	-- The second parameter is the current position. Pass the variable that should be changed by the slider.
	-- The third and fourth parameter specify the range of the slider, i.e. the min and max value.
	-- The fourth parameter is a format string to show on the slider. Must contain a variant of "%f" in the text.
	-- The example format string rounds the value to two decimal places and prefixes the value with "Value: " on the slider.
        
	-- The function returns two values: a bool to indicate whether the value was changed and the new value. You must copy
	-- the new value into the slider variable if the value changed:
	if changed then
		master_volume = newVal
	end


	local changed, newVal = imgui.SliderFloat(" ", aircraft_exterior_volume, 0, 1, "Aircraft Exterior Volume Value: %.2f")

	if changed then
		aircraft_exterior_volume = newVal
	end


	local changed, newVal = imgui.SliderFloat("  ", aircraft_interior_volume, 0, 1, "Aircraft Interior Volume Value: %.2f")

	if changed then
		aircraft_interior_volume = newVal
	end


	local changed, newVal = imgui.SliderFloat("   ", pilot_volume, 0, 1, "Pilot Volume Value: %.2f")

	if changed then
		pilot_volume = newVal
	end

	local changed, newVal = imgui.SliderFloat("    ", copilot_volume, 0, 1, "Copilot Volume Value: %.2f")

	if changed then
		copilot_volume = newVal
	end

	local changed, newVal = imgui.SliderFloat("     ", radio_volume, 0, 1, "Radio Volume Value: %.2f")

	if changed then
		radio_volume = newVal
	end

	local changed, newVal = imgui.SliderFloat("       ", enviro_volume, 0, 1, "Enviro Volume Value: %.2f")

	if changed then
		enviro_volume = newVal
	end

	local changed, newVal = imgui.SliderFloat("        ", ui_volume, 0, 1, "Ui Volume Value: %.2f")

	if changed then
		ui_volume = newVal
	end
	
	
--	local changed, newVal = imgui.SliderFloat("         ", engine_volume, 0, 1, "Engine Volume Value: %.2f")

--	if changed then
--		engine_volume = newVal
--	end

--	local changed, newVal = imgui.SliderFloat("          ", prop_volume, 0, 1, "Prop Volume Value: %.2f")

--	if changed then
--		prop_volume = newVal
--	end

--	local changed, newVal = imgui.SliderFloat("           ", ground_volume, 0, 1, "Ground Volume Value: %.2f")

--	if changed then
--		ground_volume = newVal
--	end	
	
--	local changed, newVal = imgui.SliderFloat("            ", weather_volume, 0, 1, "Weather Volume Value: %.2f")

--	if changed then
--		weather_volume = newVal
--	end
	
--	local changed, newVal = imgui.SliderFloat("             ", warning_volume, 0, 1, "Warning Volume Value: %.2f")

--	if changed then
--		warning_volume = newVal
--	end	

--	local changed, newVal = imgui.SliderFloat("              ", fan_volume, 0, 1, "Fan Volume Value: %.2f")

--	if changed then
--		fan_volume = newVal
--	end

	if XPLMFindDataRef("simcoders/rep/settings/wind_volume") ~= nil then
		dataref("wind_volume", "simcoders/rep/settings/wind_volume", "writable")
		local changed, newVal = imgui.SliderInt("       ", wind_volume, 0, 100, "REP Wind sound level: %.0f")

		if changed then
			wind_volume = newVal
		end
	end


end -- function issv_on_build


-------------------------------------------------------------------------------------------------




-------------------Show Hide Window Section with Toggle functionaility---------------------------

issv_wnd = nil  -- flag for the show_wnd set to nil so that creation below can happen - float_wnd_create

function issv_show_wnd() -- This is called when user toggles window on/off, if the next toggle is for ON
    issv_wnd = float_wnd_create(400, 250, 1, true)
    float_wnd_set_title(issv_wnd, "Imgui Sound Settings v1.1")
    float_wnd_set_imgui_builder(issv_wnd, "issv_on_build")
end


function issv_hide_wnd()  -- This is called when user toggles window on/off, if the next toggle is for OFF
    if issv_wnd then
        float_wnd_destroy(issv_wnd)
    end
end

issv_show_only_once = 0
issv_hide_only_once = 0

function toggle_issv_window()  -- This is the toggle window on/off function
	issv_show_window = not issv_show_window
	if issv_show_window then
		if issv_show_only_once == 0 then
			issv_show_wnd()
			issv_show_only_once = 1
			issv_hide_only_once = 0
		end
	else
		if issv_hide_only_once == 0 then
			issv_hide_wnd()
			issv_hide_only_once = 1
			issv_show_only_once = 0
		end
	end
end
------------------------------------------------------------------------------------------------


----"add_macro" - adds the option to the FWL macro menu in X-Plane
----"create command" - creates a show/hide toggle command that calls the toggle_issv_window()

add_macro("Imgui Sound Settings: open/close", "issv_show_wnd()", "issv_hide_wnd()", "deactivate")
create_command("issv_menus/show_toggle", "open/close Imgui Sound Settings Window", "toggle_issv_window()", "", "")

--[[
footnotes:  If changing color using PushStyleColor, here are common color codes:  
    BLACK       = 0xFF000000;
    DKGRAY      = 0xFF444444;
    GRAY        = 0xFF888888;
    LTGRAY      = 0xFFCCCCCC;
    WHITE       = 0xFFFFFFFF;
    RED         = 0xFFFF0000;
    GREEN       = 0xFF00FF00;
    BLUE        = 0xFF0000FF;
    YELLOW      = 0xFFFFFF00;
    CYAN        = 0xFF00FFFF;
    MAGENTA     = 0xFFFF00FF;
    ]]
