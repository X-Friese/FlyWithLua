-- Imgui Push Font Demo
-- William R. Good 08-07-26

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end


ihd_wnd = float_wnd_create(500, 200, 1, true)
float_wnd_set_position(ihd_wnd, 100, 100)
float_wnd_set_title(ihd_wnd, "Imgui Push Font Demo")
float_wnd_set_imgui_builder(ihd_wnd, "ihd_on_build")
float_wnd_set_onclose(ihd_wnd, "closed_hello_world")
loop = 0

function ihd_on_build(ihd_wnd, x, y)
    imgui.TextUnformatted("This should be the first line")
    imgui.TextUnformatted(string.format("Loop =  %d", loop))	
    if (loop < 5) then
        loop = loop + 1
    end
    
    if loop == 5 then
            
        imgui.TextUnformatted("Default Font")
    	imgui_push_font(1, 80)
    	imgui.TextUnformatted("Custom font 1")	
    	imgui.PopFont()
    	imgui.TextUnformatted("Default Font")
    	imgui_push_font(2, 20)
    	imgui.TextUnformatted("Custom font 2")	
    	imgui.PopFont()
    	imgui.TextUnformatted("Default Font")
    	imgui_push_font(3, 20)
    	imgui.TextUnformatted("Custom font 3")	
    	imgui.PopFont()
    	imgui.TextUnformatted("Default Font")
    	imgui_push_font(4, 20)
    	imgui.TextUnformatted("Custom font 4")	
    	imgui.PopFont()
    	imgui.TextUnformatted("Default Font")
    end
    if imgui.TreeNode("Show ingui demo window") then
        imgui.ShowDemoWindow()
        imgui.TreePop()
    end     
end


function closed_hello_world(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end

-- Example below is identical to the above, but passes references to callback functions instead of their names.
-- This allows using non-global and even anonymous functions to handle window events.
