-- Imgui Hello World
-- William R. Good 09-03-18

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end


ihd_wnd = float_wnd_create(200, 100, 1, true)
float_wnd_set_position(ihd_wnd, 100, 100)
float_wnd_set_title(ihd_wnd, "Imgui Hello World")
float_wnd_set_imgui_builder(ihd_wnd, "ihd_on_build")
float_wnd_set_onclose(ihd_wnd, "closed_hello_world")

function ihd_on_build(ihd_wnd, x, y)
    imgui.TextUnformatted(string.format("Hello World:\n  wnd=%s\n  x=%d\n  y=%d", ihd_wnd, x, y))
end


function closed_hello_world(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end

-- Example below is identical to the above, but passes references to callback functions instead of their names.
-- This allows using non-global and even anonymous functions to handle window events.

local function ihd2_on_build(ihd_wnd, x, y)
    imgui.TextUnformatted(string.format("Hello World from callback:\n  wnd=%s\n  x=%d\n  y=%d", ihd_wnd, x, y))
end

ihd_wnd2 = float_wnd_create(200, 100, 1, true)
float_wnd_set_position(ihd_wnd2, 400, 100)
float_wnd_set_title(ihd_wnd2, "Imgui Hello World 2")
float_wnd_set_imgui_builder(ihd_wnd2, ihd2_on_build)
-- This won't work (notice quotes around function name) because ihd2_on_build is defined as local:
--float_wnd_set_imgui_builder(ihd_wnd2, "ihd2_on_build")