-- Imgui 170 Demo
-- William R. Good 05-14-20

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end


i170d_wnd = float_wnd_create(1000, 450, 1, true)
float_wnd_set_position(i170d_wnd, 50, 120)
float_wnd_set_title(i170d_wnd, "Imgui 170 Demo")
float_wnd_set_imgui_builder(i170d_wnd, "i170d_on_build")
float_wnd_set_onclose(i170d_wnd, "closed_imgui_170_demo")

function i170d_on_build(i170d_wnd, x, y)
--    imgui.ShowAboutWindow()
    imgui.ShowDemoWindow()
end


function closed_hello_world(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end
