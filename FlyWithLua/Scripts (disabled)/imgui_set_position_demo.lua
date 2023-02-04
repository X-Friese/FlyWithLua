-- Imgui Set Position Demo
-- William R. Good 02-04-23

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

x_position = create_dataref_table("FlyWithLua/imgui/x_position", "Int")
y_position = create_dataref_table("FlyWithLua/imgui/y_position", "Int")

x_position[0] = 100
y_position[0] = 100

ispd_wnd = float_wnd_create(250, 175, 1, true)
float_wnd_set_position(ispd_wnd, x_position[0], y_position[0])
float_wnd_set_title(ispd_wnd, "Imgui Set Position Demo")
float_wnd_set_imgui_builder(ispd_wnd, "ispd_on_build")
float_wnd_set_onclose(ispd_wnd, "closed_set_position_demo")

function ispd_on_build(ispd_wnd, x, y)
    local win_width = imgui.GetWindowWidth()
    local win_height = imgui.GetWindowHeight()
    imgui.TextUnformatted(string.format("Imgui Set Position Demo:\n  wnd = %s\n  win_width = %s\n  win_height = %s\n  x = %d\n  y = %d", ispd_wnd, win_width, win_height, x, y))
    imgui.TextUnformatted(string.format("FlyWithLua/imgui/x_position = %s\n", x_position[0]))
    imgui.TextUnformatted(string.format("FlyWithLua/imgui/y_position = %s\n", y_position[0]))
    if imgui.Button("Save Position") then
            -- The Button function creates a button with the given label. It returns true in the
            -- moment where the button is released, i.e. the if statement is used to check whether
            -- the action behind the button should now be executed. Let's make the button toggle a variable:
            x_position[0] = x
            y_position[0] = y
    end
    
    imgui.SameLine()
    
    if imgui.Button("Set Position") then
            -- The Button function creates a button with the given label. It returns true in the
            -- moment where the button is released, i.e. the if statement is used to check whether
            -- the action behind the button should now be executed. Let's make the button toggle a variable:
            float_wnd_set_position(ispd_wnd, x_position[0], y_position[0])
    end
    
    if imgui.Button("Set Default Position") then
            -- The Button function creates a button with the given label. It returns true in the
            -- moment where the button is released, i.e. the if statement is used to check whether
            -- the action behind the button should now be executed. Let's make the button toggle a variable:
            float_wnd_set_position(ispd_wnd, 100, 100)
    end       
end


function closed_set_position_demo(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end

-- Example below is identical to the above, but passes references to callback functions instead of their names.
-- This allows using non-global and even anonymous functions to handle window events.
