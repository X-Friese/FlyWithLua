-- Imgui Hello World
-- William R. Good 09-03-18

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end


ihd_wnd = float_wnd_create(640, 480, 1, true)

float_wnd_set_title(ihd_wnd, "Imgui Hello World")

float_wnd_set_imgui_builder(ihd_wnd, "ihd_on_build")

float_wnd_set_onclose(ihd_wnd, "closed_hello_world")

function ihd_on_build(ihd_wnd, x, y)
	imgui.TextUnformatted("Hello World")
end


function closed_hello_world(wnd)
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end
