-- Imugi Test Write Xlua Datarefs
-- William R. Good 08-11-18

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end


if (PLANE_ICAO == "C172") then
	dataref("knob_TAS", "laminar/c172/knob_TAS", "writable")
	dataref("knob_EGT", "laminar/c172/knob_EGT", "writable")
	dataref("knob_OAT", "laminar/c172/knob_OAT", "writable")
end


if (PLANE_ICAO == "BE9L") then
	dataref("c90_dac_knob", "laminar/c90/airCon/knob/defrost_air_control", "writable")
	dataref("c90_acp_knob", "laminar/c90/airCon/knob/air_control_pilot", "writable")
	dataref("c90_acc_knob", "laminar/c90/airCon/knob/air_control_copilot", "writable")
	dataref("c90_autopilot_knob_turn", "laminar/c90/autopilot/knob/turn", "writable")
end

if (PLANE_ICAO == "BE58") then
	dataref("B58_flight_inst_rheo", "laminar/b58/lighting/flight_inst_rheo", "writable")
	dataref("B58_inst_flood_rheo", "laminar/b58/lighting/inst_flood_rheo", "writable")
	dataref("B58_eng_inst_rheo", "laminar/b58/lighting/eng_inst_rheo", "writable")
	dataref("B58_sub_panel_rheo", "laminar/b58/lighting/sub_panel_rheo", "writable")
end


function itwxd_on_build(itwxd_wnd, x, y)

	if (PLANE_ICAO == "C172") then
		local changed, newVal = imgui.SliderFloat("##Slider Caption1", knob_TAS, 0.000, 1.000, "knob_TAS﻿: %.2f")
		if changed then
			knob_TAS = newVal
			imgui.TextUnformatted("knob_TAS Changed  knob_TAS newVal = " .. newVal .. "   knob_TAS = " .. knob_TAS)
	    end
		imgui.TextUnformatted("")

		local changed, newVal = imgui.SliderFloat("##Slider Caption1a", knob_EGT, 0.000, 1.000, "knob_EGT: %.2f")
		if changed then
			knob_EGT = newVal
			imgui.TextUnformatted("knob_EGT Changed  knob_EGT newVal = " .. newVal .. "   knob_EGT = " .. knob_EGT)
	    end
		imgui.TextUnformatted("")

		local changed, newVal = imgui.SliderFloat("##Slider Caption1b", knob_OAT, 1.000, 0.000, "knob_OAT: %.2f")
		if changed then
			knob_OAT = newVal
			imgui.TextUnformatted("knob_OAT Changed  knob_OAT newVal = " .. newVal .. "   knob_OAT = " .. knob_OAT)
	    end

	end
	imgui.TextUnformatted("")

	if (PLANE_ICAO == "BE9L") then
		local changed, newVal = imgui.SliderFloat("##Slider Caption2", c90_dac_knob, 0.00, 1.00, "Defrost Air Control Pilot Knob: %.2f")
		if changed then
			c90_dac_knob = newVal
			imgui.TextUnformatted("c90_dac_knob﻿ Changed  c90_dac_knob newVal = " .. newVal .. "   c90_dac_knob = " .. c90_dac_knob)
	    end
		imgui.TextUnformatted("")

		local changed, newVal = imgui.SliderFloat("##Slider Caption3", c90_acp_knob, 0.00, 100.00, "Air Control Pilot Knob: %.2f")
		if changed then
			c90_acp_knob = newVal
			imgui.TextUnformatted("c90_acp_knob﻿ Changed  c90_acp_knob newVal = " .. newVal .. "   c90_acp_knob = " .. c90_acp_knob)
	    end
		imgui.TextUnformatted("")

		local changed, newVal = imgui.SliderFloat("##Slider Caption4", c90_acc_knob, 0.00, 100.00, "Air Control Copilot Knob: %.2f")
		if changed then
			c90_acc_knob = newVal
			imgui.TextUnformatted("c90_acc_knob﻿ Changed  c90_acc_knob newVal = " .. newVal .. "   c90_acc_knob = " .. c90_acc_knob)
	    end
		imgui.TextUnformatted("")

		local changed, newVal = imgui.SliderFloat("##Slider Caption5", c90_autopilot_knob_turn, 0.00, 100.00, "Autopilot Knob Turn: %.2f")
		if changed then
			c90_autopilot_knob_turn = newVal
			imgui.TextUnformatted("c90_autopilot_knob_turn Changed  c90_autopilot_knob_turn newVal = " .. newVal .. "   c90_autopilot_knob_turn = " .. c90_autopilot_knob_turn)
	    end
		imgui.TextUnformatted("")
	end

	if (PLANE_ICAO == "BE58") then

		local changed, newVal = imgui.SliderFloat("##Slider Caption6", B58_flight_inst_rheo, 0.00, 1.00, "Defrost Air Control Pilot Knob: %.2f")
		if changed then
			B58_flight_inst_rheo = newVal
			imgui.TextUnformatted("B58_flight_inst_rheo Changed  B58_flight_inst_rheo newVal = " .. newVal .. "   B58_flight_inst_rheo = " .. B58_flight_inst_rheo)
	    end
		imgui.TextUnformatted("")

		local changed, newVal = imgui.SliderFloat("##Slider Caption7", B58_inst_flood_rheo, 0.00, 100.00, "Air Control Pilot Knob: %.2f")
		if changed then
			B58_inst_flood_rheo = newVal
			imgui.TextUnformatted("B58_inst_flood_rheo﻿ Changed  B58_inst_flood_rheo newVal = " .. newVal .. "   B58_inst_flood_rheo = " .. B58_inst_flood_rheo)
	    end
		imgui.TextUnformatted("")

		local changed, newVal = imgui.SliderFloat("##Slider Caption8", B58_eng_inst_rheo, 0.00, 100.00, "Air Control Copilot Knob: %.2f")
		if changed then
			B58_eng_inst_rheo = newVal
			imgui.TextUnformatted("B58_eng_inst_rheo Changed  B58_eng_inst_rheo newVal = " .. newVal .. "   B58_eng_inst_rheo = " .. B58_eng_inst_rheo)
	    end
		imgui.TextUnformatted("")

		local changed, newVal = imgui.SliderFloat("##Slider Caption9", B58_sub_panel_rheo, 0.00, 100.00, "Autopilot Knob Turn: %.2f")
		if changed then
			B58_sub_panel_rheo = newVal
			imgui.TextUnformatted("B58_sub_panel_rheo Changed  B58_sub_panel_rheo newVal = " .. newVal .. "   B58_sub_panel_rheo = " .. B58_sub_panel_rheo)
	    end
		imgui.TextUnformatted("")
	
	end
	
end

itwxd_wnd = nil

function itwxd_show_wnd()
    itwxd_wnd = float_wnd_create(640, 480, 1, true)
    float_wnd_set_title(itwxd_wnd, "Test Write Xlua Data Refs")
    float_wnd_set_imgui_builder(itwxd_wnd, "itwxd_on_build")
end

function itwxd_hide_wnd()
    if itwxd_wnd then
        float_wnd_destroy(itwxd_wnd)
    end
end

itwxd_show_only_once = 0
itwxd_hide_only_once = 0

function toggle_test_write_xlua_data_refs_window()
	itwxd_show_window = not itwxd_show_window
	if itwxd_show_window then
		if itwxd_show_only_once == 0 then
			itwxd_show_wnd()
			itwxd_show_only_once = 1
			itwxd_hide_only_once = 0
		end
	else
		if itwxd_hide_only_once == 0 then
			itwxd_hide_wnd()
			itwxd_hide_only_once = 1
			itwxd_show_only_once = 0
		end
	end
end


add_macro("Test Write Xlua Data Refs: open/close", "itwxd_show_wnd()", "itwxd_hide_wnd()", "deactivate")
create_command("FlyWithLua/test-write-xlua-data-refs/show_toggle", "open/close test custom data refs window", "toggle_test_write_xlua_data_refs_window()", "", "")

