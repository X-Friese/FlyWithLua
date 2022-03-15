-- Imgui 185 Demo
-- William R. Good 03-11-22

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end


i185d_wnd = float_wnd_create(1000, 450, 1, true)
float_wnd_set_position(i185d_wnd, 50, 120)
float_wnd_set_title(i185d_wnd, "Imgui 185 Demo")
float_wnd_set_imgui_builder(i185d_wnd, "i185d_on_build")
float_wnd_set_onclose(i185d_wnd, "closed_imgui_185_demo")

function i185d_on_build(i185d_wnd, x, y)
--    imgui.ShowAboutWindow()
    imgui.ShowDemoWindow()
end


function closed_imgui_185_demo(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end
