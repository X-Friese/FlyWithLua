-- Imgui VR Pedals Visualizer
-- William R. Good 02-11-23

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

dataref("rudder_pedal_position", "sim/flightmodel/controls/ldruddef")
dataref("left_brake_pedal_position", "sim/cockpit2/controls/left_brake_ratio")
dataref("right_brake_pedal_position", "sim/cockpit2/controls/right_brake_ratio")
dataref("vr_enabled","sim/graphics/VR/enabled","readonly")

ivpv_wnd = nil
vr_enabled_delayed = 0
vr_disabled_delayed = 0
loop_count = 0

function ivpv_on_build(ivpv_wnd, x, y)
    imgui.TextUnformatted("")
    imgui.SetCursorPosX(30)
    imgui.VSliderFloat(" ", 40, 120, rudder_pedal_position, -19.0, 19.0, "")
    imgui.SameLine()
    imgui.SetCursorPosX(100)
    imgui.VSliderFloat(" ", 40, 120, left_brake_pedal_position, 0.0, 1.0, "")
    imgui.SameLine()
    imgui.SetCursorPosX(170)
    imgui.VSliderFloat(" ", 40, 120, right_brake_pedal_position, 0.0, 1.0, "")
    imgui.SameLine()      
    imgui.SetCursorPosX(240)
    imgui.VSliderFloat(" ", 40, 120, rudder_pedal_position, 19.0, -19.0, "")
    loop_count = loop_count + 1   
    imgui.TextUnformatted("")
end


function closed_vr_pedals_visualizer(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end


function check_for_vr()
	if vr_enabled == 1 and vr_enabled_delayed == 1 then
		if ivpv_wnd then
			float_wnd_destroy(ivpv_wnd)
		end
		ivpv_wnd = float_wnd_create(300, 175, 0, true)
		float_wnd_set_title(ivpv_wnd, "Imgui VR Pedals Visualizer In VR")
		--                                                0x Alphia Red Green Blue
		imgui.PushStyleColor(imgui.constant.Col.WindowBg, 0xCC101112) -- Black like Background
		float_wnd_set_imgui_builder(ivpv_wnd, "ivpv_on_build")
		vr_disabled_delayed = 0
	end
	if vr_enabled == 1 and vr_enabled_delayed < 2 then
		vr_enabled_delayed = vr_enabled_delayed + 1	
	end
	
	if vr_enabled == 0 and vr_disabled_delayed == 1 then
		if ivpv_wnd then
			float_wnd_destroy(ivpv_wnd)
		end
		ivpv_wnd = float_wnd_create(300, 175, 1, true)
		float_wnd_set_title(ivpv_wnd, "Imgui VR Pedals Visualizer In 2d")
		float_wnd_set_position(ivpv_wnd, 100, 350)
		--                                                0x Alphia Red Green Blue
		imgui.PushStyleColor(imgui.constant.Col.WindowBg, 0xCC101112) -- Black like Background
		float_wnd_set_imgui_builder(ivpv_wnd, "ivpv_on_build")
		vr_enabled_delayed = 0
	end
	if vr_enabled == 0 and vr_disabled_delayed < 2 then
		vr_disabled_delayed = vr_disabled_delayed + 1	
	end
end


do_often("check_for_vr()")
