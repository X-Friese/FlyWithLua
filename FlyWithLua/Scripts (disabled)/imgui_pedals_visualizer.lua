-- Imgui Pedals Visualizer
-- William R. Good 02-08-23

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

dataref("rudder_pedal_position", "sim/flightmodel/controls/ldruddef")
dataref("left_brake_pedal_position", "sim/cockpit2/controls/left_brake_ratio")
dataref("right_brake_pedal_position", "sim/cockpit2/controls/right_brake_ratio")


ipv_wnd = float_wnd_create(300, 250, 1, true)
float_wnd_set_position(ipv_wnd, 100, 350)
float_wnd_set_title(ipv_wnd, "Imgui Pedals Visualizer")
float_wnd_set_imgui_builder(ipv_wnd, "ipv_on_build")
float_wnd_set_onclose(ipv_wnd, "closed_pedals_visualizer")

function ipv_on_build(ipv_wnd, x, y)
    imgui.TextUnformatted("")
    imgui.SetCursorPosX(30)
    imgui.VSliderFloat(" ", 40, 200, rudder_pedal_position, -19.0, 19.0, "")
    imgui.SameLine()
    imgui.SetCursorPosX(100)
    imgui.VSliderFloat(" ", 40, 200, left_brake_pedal_position, 0.0, 1.0, "")
    imgui.SameLine()
    imgui.SetCursorPosX(170)
    imgui.VSliderFloat(" ", 40, 200, right_brake_pedal_position, 0.0, 1.0, "")
    imgui.SameLine()      
    imgui.SetCursorPosX(240)
    imgui.VSliderFloat(" ", 40, 200, rudder_pedal_position, 19.0, -19.0, "")    
    imgui.TextUnformatted("")
end


function closed_pedals_visualizer(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end
