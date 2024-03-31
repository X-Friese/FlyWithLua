-- Imugi Get Latitude and Longitude
-- William R. Good 03-27-24

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end


local dec_deg_latitude = 0.0
local dec_deg_longitude = 0.0


function iglal_on_build(iglal_wnd, x, y)
	if imgui.Button("Get Location", 100, 50) then
		get_location()
	end
	
	imgui.TextUnformatted(string.format("Latitude  %f", dec_deg_latitude))
	imgui.TextUnformatted(string.format("Longitude  %f", dec_deg_longitude))

end

function get_location()
	dec_deg_latitude = LATITUDE
	dec_deg_longitude = LONGITUDE
end

iglal_wnd = nil

function iglal_show_wnd()
    iglal_wnd = float_wnd_create(640, 480, 1, true)
    float_wnd_set_title(iglal_wnd, "Imgui Imugi Get Latitude and Longitude")
    float_wnd_set_imgui_builder(iglal_wnd, "iglal_on_build")
end

function iglal_hide_wnd()
    if iglal_wnd then
        float_wnd_destroy(iglal_wnd)
    end
end

iglal_show_only_once = 0
iglal_hide_only_once = 0

function toggle_imgui_show_hide_window_with_command()
	iglal_show_window = not iglal_show_window
	if iglal_show_window then
		if iglal_show_only_once == 0 then
			iglal_show_wnd()
			iglal_show_only_once = 1
			iglal_hide_only_once = 0
		end
	else
		if iglal_hide_only_once == 0 then
			iglal_hide_wnd()
			iglal_hide_only_once = 1
			iglal_show_only_once = 0
		end
	end
end

add_macro("Imgui Show Hide Imgui Get Latitude and Longitude: open/close", "iglal_show_wnd()", "iglal_hide_wnd()", "deactivate")
create_command("FlyWithLua/imgui-get-latitude-and-longitude-window/show_toggle", "open/close umgui get latitude and longitude window", "toggle_imgui_get_latitude_and_longitude()", "", "")