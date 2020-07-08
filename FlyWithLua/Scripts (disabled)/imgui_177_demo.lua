-- Imgui 177 Demo
-- William R. Good 07-07-20

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end


i177d_wnd = float_wnd_create(1000, 450, 1, true)
float_wnd_set_position(i177d_wnd, 50, 120)
float_wnd_set_title(i177d_wnd, "Imgui 177 Demo")
float_wnd_set_imgui_builder(i177d_wnd, "i177d_on_build")
float_wnd_set_onclose(i177d_wnd, "closed_imgui_177_demo")

function i177d_on_build(i177d_wnd, x, y)
--    imgui.ShowAboutWindow()
    imgui.ShowDemoWindow()
end


function closed_imgui_177_demo(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end
