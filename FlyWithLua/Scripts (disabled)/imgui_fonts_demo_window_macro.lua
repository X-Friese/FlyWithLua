-- Imgui Fonts Demo Window Macro
-- William R. Good 08-06-26

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end


-- ifd_wnd = float_wnd_create(500, 300, 1, true)
-- float_wnd_set_position(ifd_wnd, 100, 100)
-- float_wnd_set_title(ifd_wnd, "Imgui Font Demo Macro")
-- float_wnd_set_imgui_builder(ifd_wnd, "ifd_on_build")
-- float_wnd_set_onclose(ifd_wnd, "closed_font_demo")

function ifd_on_build(ifd_wnd, x, y)
    imgui.TextUnformatted(string.format("Imgui Font Demo Window:\n  wnd=%s  x=%d  y=%d", ifd_wnd, x, y))
    imgui.TextUnformatted("imgui.PushFont(customFont, 13) does not work but this selector does")
    imgui.ShowFontSelector("")
    imgui.ShowStyleSelector(" ")
    
    imgui.PushStyleColor(imgui.constant.Col.Text, 0xFF00FF00) -- Green text for more contrast
    imgui.TextUnformatted("We can now change font and the color and the size")
    imgui.SetWindowFontScale(0.5)
    imgui.TextUnformatted("We can also change the scale to 0.5")
    imgui.SetWindowFontScale(0.625)
    imgui.TextUnformatted("We can also change the scale to 0.625")        
    imgui.SetWindowFontScale(0.75)
    imgui.TextUnformatted("We can also change the scale to 0.75")
    imgui.SetWindowFontScale(0.875)
    imgui.TextUnformatted("We can also change the scale to 0.875")        
    imgui.SetWindowFontScale(1.0)
    imgui.TextUnformatted("We can also change the scale to 1.0")
    imgui.SetWindowFontScale(1.125)
    imgui.TextUnformatted("We can also change the scale to 1.125")        
    imgui.SetWindowFontScale(1.25)
    imgui.TextUnformatted("We can also change the scale to 1.25")
    imgui.SetWindowFontScale(1.375)
    imgui.TextUnformatted("We can also change the scale to 1.375")        
    imgui.SetWindowFontScale(1.5)
    imgui.TextUnformatted("We can also change the scale to 1.5")
    imgui.SetWindowFontScale(1.0)
    imgui.TextUnformatted("Because this is the font scale for the window we need to make sure")
    imgui.TextUnformatted("we have changed it back to the default of 1.0.")
    imgui.PopStyleColor()

    if imgui.TreeNode("Show ingui demo window") then
        imgui.ShowDemoWindow()
        imgui.TreePop()
    end
        
end


-------------------Show Hide Window Section with Toggle functionaility---------------------------

ifd_wnd = nil  -- flag for the show_wnd set to nil so that creation below can happen - float_wnd_create

function ifd_show_wnd() -- This is called when user toggles window on/off, if the next toggle is for ON
    ifd_wnd = float_wnd_create(550, 330, 1, true)
    float_wnd_set_title(ifd_wnd, "Imgui Font Demo Macro")
    float_wnd_set_imgui_builder(ifd_wnd, "ifd_on_build")
end


function ifd_hide_wnd()  -- This is called when user toggles window on/off, if the next toggle is for OFF
    if ifd_wnd then
        float_wnd_destroy(ifd_wnd)
    end
end

ifd_show_only_once = 0
ifd_hide_only_once = 0

function toggle_ifd_window()  -- This is the toggle window on/off function
	ifd_show_window = not ifd_show_window
	if ifd_show_window then
		if ifd_show_only_once == 0 then
			ifd_show_wnd()
			ifd_show_only_once = 1
			ifd_hide_only_once = 0
		end
	else
		if ifd_hide_only_once == 0 then
			ifd_hide_wnd()
			ifd_hide_only_once = 1
			ifd_show_only_once = 0
		end
	end
end
------------------------------------------------------------------------------------------------



----"add_macro" - adds the option to the FWL macro menu in X-Plane

add_macro("Imgui Fonts Demo: open/close", "ifd_show_wnd()", "ifd_hide_wnd()", "deactivate")
