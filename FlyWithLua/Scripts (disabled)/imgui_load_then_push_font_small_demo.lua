-- Imgui Load Then Push Font Small Demo
-- William R. Good 08-09-26

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

test_loop = 0

iltpfd_wnd = float_wnd_create(550, 400, 1, true)
float_wnd_set_position(iltpfd_wnd, 100, 50)
float_wnd_set_title(iltpfd_wnd, "Imgui Load Then Push Font Small Demo")
float_wnd_set_imgui_builder(iltpfd_wnd, "iltpfd_on_build")
float_wnd_set_onclose(iltpfd_wnd, "closed_imgui_load_then_push_font")


function iltpfd_on_build(iltpfd_wnd, x, y)
    if (test_loop < 5) then
        test_loop = test_loop + 1
    end
    
    if test_loop == 5 then
            
        imgui.TextUnformatted("Default Font Size 13")
        -- font number 0 thru 3
        -- 0 is a NULL so will use the same font but change its size
        -- That only seems to work with the default font
        -- font size  10 thru 20 in steps of 1 so 10, 11, 12, 13...20
        -- and font size  20 thru 65 in steps of 5 so 20, 25, 30, 35...65
    	imgui_push_font(1)
    	imgui.TextUnformatted("ProFontWindows.ttf Size 13")
    	imgui.PopFont()
    	imgui_push_font(2)
    	imgui.TextUnformatted("ProFontWindows.ttf Size 16")
    	imgui.PopFont()	
    	imgui_push_font(3)
    	imgui.TextUnformatted("ProFontWindows.ttf Size 20")
    	imgui.PopFont()
    	imgui_push_font(4)
    	imgui.TextUnformatted("Roboto-Light.ttf Size 13")
    	imgui.PopFont()
    	imgui_push_font(5)
    	imgui.TextUnformatted("Roboto-Light.ttf  Size 16")
    	imgui.PopFont()
    	imgui_push_font(6)
    	imgui.TextUnformatted("Roboto-Light.ttf Size 20")
    	imgui.PopFont()
    	imgui_push_font(7)
    	imgui.TextUnformatted("Roboto-Regular.ttf Size 13")	
    	imgui.PopFont()
    	imgui_push_font(8)
    	imgui.TextUnformatted("Roboto-Regular.ttf Size 16")	
    	imgui.PopFont()
    	imgui_push_font(9)
    	imgui.TextUnformatted("Roboto-Regular.ttf Size 20")	
    	imgui.PopFont()
    	imgui.TextUnformatted("Default Font Size 13")    	
    end
    if imgui.TreeNode("Show ingui demo window") then
        imgui.ShowDemoWindow()
        imgui.TreePop()
    end     
end


function closed_imgui_load_then_push_font(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end

-- Example below is identical to the above, but passes references to callback functions instead of their names.
-- This allows using non-global and even anonymous functions to handle window events.
