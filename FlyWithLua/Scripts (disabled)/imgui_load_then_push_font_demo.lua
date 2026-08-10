-- Imgui Load Then Push Font Demo
-- William R. Good 08-09-26

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end


iltpfd_wnd = float_wnd_create(500, 450, 1, true)
float_wnd_set_position(iltpfd_wnd, 100, 100)
float_wnd_set_title(iltpfd_wnd, "Imgui Load Then Push Font Demo")
float_wnd_set_imgui_builder(iltpfd_wnd, "iltpfd_on_build")
float_wnd_set_onclose(iltpfd_wnd, "closed_imgui_load_then_push_font")
loop = 0

function iltpfd_on_build(iltpfd_wnd, x, y)
    if (loop < 5) then
        loop = loop + 1
    end
    
    if loop == 5 then
            
        imgui.TextUnformatted("Default Font")
        -- font number 0 thru 3
        -- 0 is a NULL so will use the same font but change its size
        -- font size  10 thru 20 in steps of 1 so 10, 11, 12, 13...20
        -- and font size  20 thru 65 in steps of 5 so 20, 25, 30, 35...65
    	imgui_push_font(1, 10)
    	imgui.TextUnformatted("ProFontWindows.ttf is 1 size 10")
    	imgui.PopFont()
    	-- 0 means use the same font but change the size.	
    	imgui_push_font(0, 15)
    	imgui.TextUnformatted("ProFontWindows.ttf is 1 size 15")
    	imgui.PopFont()	
    	imgui_push_font(0, 20)
    	imgui.TextUnformatted("ProFontWindows.ttf is 1 size 20")
    	imgui.PopFont()	
    	imgui_push_font(0, 25)
    	imgui.TextUnformatted("ProFontWindows.ttf is 1 size 25")
    	imgui.PopFont()	
    	imgui_push_font(2, 10)
    	imgui.TextUnformatted("Roboto-Light.ttf is 2 size 10")
    	imgui.PopFont()	
    	imgui_push_font(0, 15)
    	imgui.TextUnformatted("Roboto-Light.ttf is 2 size 15")	
    	imgui.PopFont()
    	imgui_push_font(0, 20)
    	imgui.TextUnformatted("Roboto-Light.ttf is 2 size 20")	
    	imgui.PopFont()
    	imgui_push_font(0, 25)
    	imgui.TextUnformatted("Roboto-Light.ttf is 2 size 25")	
    	imgui.PopFont()    	    	    	
    	imgui_push_font(3, 10)
    	imgui.TextUnformatted("Roboto-Regular.ttf is 3 size 10")	
    	imgui.PopFont()
    	imgui_push_font(0, 15)
    	imgui.TextUnformatted("Roboto-Regular.ttf is 3 size 15")	
    	imgui.PopFont()    	    	
    	imgui_push_font(0, 20)
    	imgui.TextUnformatted("Roboto-Regular.ttf is 3 size 20")	
    	imgui.PopFont()
    	imgui_push_font(0, 25)
    	imgui.TextUnformatted("Roboto-Regular.ttf is 3 size 25")	
    	imgui.PopFont()	
    	imgui.TextUnformatted("Default Font")
    	-- 0 means use the same font but change the size.
    	-- In this case it will be the default font because of the PopFont()
    	imgui_push_font(0, 10)
    	imgui.TextUnformatted("Default Font size 10")
    	imgui.PopFont()
    	imgui_push_font(0, 12)
    	imgui.TextUnformatted("Default Font size 12")
    	imgui.PopFont()
    	imgui_push_font(0, 14)
    	imgui.TextUnformatted("Default Font size 14")
    	imgui.PopFont()
    	imgui_push_font(0, 16)
    	imgui.TextUnformatted("Default Font size 16")
    	imgui.PopFont()    	    	    	
    	imgui.TextUnformatted("Default Font")    	
    	
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
