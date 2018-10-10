-- Imgui Load Situation Demo
-- William R. Good 10-07-18

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

ilsd_wnd = float_wnd_create(640, 480, 1, true)

float_wnd_set_title(ilsd_wnd, "Imgui Load Situation Demo")

float_wnd_set_imgui_builder(ilsd_wnd, "ilsd_on_build")

float_wnd_set_onclose(ilsd_wnd, "closed_load_situation_demo")

function ilsd_on_build(ilsd_wnd, x, y)
	if imgui.Button("Load KERI Output situations Situation_Tests") then
        load_situation(SYSTEM_DIRECTORY .. "/Output/situations/Situation_Tests/KERI_Cessna Skyhawk Situation.sit" )
    end
	imgui.SameLine()
	imgui.TextUnformatted("    ")
	imgui.SameLine()
	if imgui.Button("Load KERI Scripts Situation_Tests") then
		load_situation(SCRIPT_DIRECTORY .. "Situation_Tests/KERI_Cessna Skyhawk Situation.sit" )
    end

	imgui.TextUnformatted("")
	if imgui.Button("Load KCLE Output situations Situation_Tests") then
        load_situation(SYSTEM_DIRECTORY .. "/Output/situations/Situation_Tests/KCLE_Cessna Skyhawk Situation.sit" )
    end
	imgui.SameLine()
	imgui.TextUnformatted("    ")
	imgui.SameLine()
	if imgui.Button("Load KCLE Scripts Situation_Tests") then
		load_situation(SCRIPT_DIRECTORY .. "Situation_Tests/KCLE_Cessna Skyhawk Situation.sit" )
    end

	imgui.TextUnformatted("")
	if imgui.Button("Load KROC Output situations Situation_Tests") then
        load_situation(SYSTEM_DIRECTORY .. "/Output/situations/Situation_Tests/KROC_Cessna Skyhawk Situation.sit" )
    end
	imgui.SameLine()
	imgui.TextUnformatted("    ")
	imgui.SameLine()
	if imgui.Button("Load KROC Scripts Situation_Tests") then
		load_situation(SCRIPT_DIRECTORY .. "Situation_Tests/KROC_Cessna Skyhawk Situation.sit" )
    end

	imgui.TextUnformatted("")
	if imgui.Button("Load KDEN Output situations Situation_Tests") then
        load_situation(SYSTEM_DIRECTORY .. "/Output/situations/Situation_Tests/KDEN_Cessna Skyhawk Situation.sit" )
    end
	imgui.SameLine()
	imgui.TextUnformatted("    ")
	imgui.SameLine()
	if imgui.Button("Load KDEN Scripts Situation_Tests") then
		load_situation(SCRIPT_DIRECTORY .. "Situation_Tests/KDEN_Cessna Skyhawk Situation.sit" )
    end


	imgui.TextUnformatted("")

	if imgui.Button("Load KYNG Output situations Situation_Tests") then
        load_situation(SYSTEM_DIRECTORY .. "/Output/situations/Situation_Tests/KYNG_Cessna Skyhawk Situation.sit" )
    end
	imgui.SameLine()
	imgui.TextUnformatted("    ")
	imgui.SameLine()
	if imgui.Button("Load KYNG Scripts Situation_Tests") then
		load_situation(SCRIPT_DIRECTORY .. "Situation_Tests/KYNG_Cessna Skyhawk Situation.sit" )
    end
end

function closed_closed_load_situation_demo(wnd)
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end
