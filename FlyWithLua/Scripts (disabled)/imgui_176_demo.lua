-- Imgui 176 Demo
-- William R. Good 06-03-20

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end


i176d_wnd = float_wnd_create(1000, 450, 1, true)
float_wnd_set_position(i176d_wnd, 50, 120)
float_wnd_set_title(i176d_wnd, "Imgui 176 Demo")
float_wnd_set_imgui_builder(i176d_wnd, "i176d_on_build")
float_wnd_set_onclose(i176d_wnd, "closed_imgui_176_demo")

function i176d_on_build(i176d_wnd, x, y)
--    imgui.ShowAboutWindow()
    imgui.ShowDemoWindow()
end


function closed_imgui_176_demo(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end
