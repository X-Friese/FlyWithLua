-- Imgui Fonts Demo
-- William R. Good 08-02-26

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end


ifd_wnd = float_wnd_create(550, 330, 1, true)
float_wnd_set_position(ifd_wnd, 100, 100)
float_wnd_set_title(ifd_wnd, "Imgui Font Demo")
float_wnd_set_imgui_builder(ifd_wnd, "ifd_on_build")
float_wnd_set_onclose(ifd_wnd, "closed_font_demo")

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


function closed_font_demo(wnd)
    local _ = wnd -- Reference to window, which triggered the call.
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end
