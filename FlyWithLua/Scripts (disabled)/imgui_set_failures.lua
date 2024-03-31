-- Imugi Set Failures
-- William R. Good 03-28-24

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

dataref("rel_engfai0", "sim/operation/failures/rel_engfai0", "writable")
dataref("rel_vacuum", "sim/operation/failures/rel_vacuum", "writable")
dataref("rel_genera0", "sim/operation/failures/rel_genera0", "writable")


function isf_on_build(isf_wnd, x, y)
	if imgui.Button("Fail Engine", 200, 25) then
		rel_engfai0 = 6

	end
	imgui.SameLine()
	imgui.SetCursorPosX(250)

	if imgui.Button("Fix Engine", 200, 25) then
		rel_engfai0 = 0
	end
	imgui.SameLine()
	imgui.SetCursorPosX(500)
	
	if rel_engfai0 == 0 then
		-- The following function changes the color of all widgets until PopStyleColor is called.
        	-- The format is AABBGGRR where A is alpha and B, G and R are blue, green and red.
        	-- So if you have an HTML color like #2B65EC (ocean blue), swap the first two and last two hex digits
        	-- and prefix it with FF to use it:
        	imgui.PushStyleColor(imgui.constant.Col.Text, 0xFF00FF00)
		imgui.TextUnformatted("Working")
		-- Restore the previous style, this _must_ be called after each pushed style:
        	imgui.PopStyleColor()
	else
		imgui.PushStyleColor(imgui.constant.Col.Text, 0xFF0000FF)
		imgui.TextUnformatted("Failed")
		imgui.PopStyleColor()
	end	
	
	imgui.SetCursorPosY(50)
	
	if imgui.Button("Fail Vacuum Pump", 200, 25) then
		rel_vacuum = 6

	end
	imgui.SameLine()
	imgui.SetCursorPosX(250)

	if imgui.Button("Fix Vacuum Pump", 200, 25) then
		rel_vacuum = 0
	end
	imgui.SameLine()
	imgui.SetCursorPosX(500)
	
	if rel_vacuum == 0 then
        	imgui.PushStyleColor(imgui.constant.Col.Text, 0xFF00FF00)
		imgui.TextUnformatted("Working")
        	imgui.PopStyleColor()
	else
		imgui.PushStyleColor(imgui.constant.Col.Text, 0xFF0000FF)
		imgui.TextUnformatted("Failed")
		imgui.PopStyleColor()
	end		
	
	
	imgui.SetCursorPosY(95)
	
	if imgui.Button("Fail Generator", 200, 25) then
		rel_genera0 = 6

	end
	imgui.SameLine()
	imgui.SetCursorPosX(250)

	if imgui.Button("Fix Generator", 200, 25) then
		rel_genera0 = 0
	end
	imgui.SameLine()
	imgui.SetCursorPosX(500)	
	
	if rel_genera0 == 0 then
        	imgui.PushStyleColor(imgui.constant.Col.Text, 0xFF00FF00)
		imgui.TextUnformatted("Working")
        	imgui.PopStyleColor()	
	else
		imgui.PushStyleColor(imgui.constant.Col.Text, 0xFF0000FF)
		imgui.TextUnformatted("Failed")
		imgui.PopStyleColor()	
	end	

end

isf_wnd = nil

function isf_show_wnd()
    isf_wnd = float_wnd_create(640, 480, 1, true)
    float_wnd_set_title(isf_wnd, "Imgui Set Failures")
    float_wnd_set_imgui_builder(isf_wnd, "isf_on_build")
end

function isf_hide_wnd()
    if isf_wnd then
        float_wnd_destroy(isf_wnd)
    end
end

isf_show_only_once = 0
isf_hide_only_once = 0

function toggle_imgui_show_hide_window_with_command()
	isf_show_window = not isf_show_window
	if isf_show_window then
		if isf_show_only_once == 0 then
			isf_show_wnd()
			isf_show_only_once = 1
			isf_hide_only_once = 0
		end
	else
		if isf_hide_only_once == 0 then
			isf_hide_wnd()
			isf_hide_only_once = 1
			isf_show_only_once = 0
		end
	end
end

add_macro("Imgui Set Failures: open/close", "isf_show_wnd()", "isf_hide_wnd()", "deactivate")
create_command("FlyWithLua/imgui-set-failures-window/show_toggle", "open/close imgui set failures window", "toggle_imgui_set_failures_window()", "", "")
