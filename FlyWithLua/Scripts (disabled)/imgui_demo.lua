-- imgui demo script
-- Folke Will 2018-07-15
-- Willian R. Good 2020-07-08 Updated to reflect now supporting Imgui 1.77

-- It is suggested to look at the window created by this script while reading this.

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

-- imgui only works inside a floating window, so we need to create one first:
demo_wnd = float_wnd_create(800, 450, 1, true)
-- The first two parameters specify the size of the window in boxels. Boxels are
-- scalable pixes. If the UI scale is set to 100% in the settings, a boxel equals
-- a pixel. However, if the user sets the UI scale to more than 100%, a boxel will
-- be scaled to span multiple pixels.
-- The third parameter specifies the window decoration. The following decorations are possible:
--  0: "X-Plane will draw no decoration for your window, and apply no automatic click handlers.
--      The window will not stop click from passing through its bounds. This is suitable for "windows"
--      which request, say, the full screen bounds, then only draw in a small portion of the available area."
--  1: "The default decoration for "native" windows, like the map. Provides a solid background,
--      as well as click handlers for resizing and dragging the window."
--  2: "X-Plane will draw no decoration for your window, nor will it provide resize handlers for your
--      window edges, but it will stop clicks from passing through your windows bounds."
--  3: "Like 2, but with resizing; X-Plane will draw no decoration for your window, but it will stop
--      clicks from passing through your windows bounds, and provide automatic mouse handlers for resizing."
-- The last parameter configures whether you want to use imgui for the floating window.
-- The return value is a handle to the window that can be used to configure additional things.

-- This function sets the title of the window if it has a decoration:
float_wnd_set_title(demo_wnd, "imgui Demo")
-- The first parameter must be a handle to a window previously created with float_wnd_create and the
-- second parameter is the title of the window.

-- imgui GUIs are built each frame. You will need to supply a function that is called every frame:
float_wnd_set_imgui_builder(demo_wnd, "build_demo")
-- The first parameter must be a handle to a window previously created with float_wnd_create and the
-- second parameter must be the name of a function. Note that unlike other function in FlyWithLua, you
-- can only pass the name of a function here, not an arbitrary lua string. This function will be called
-- for every frame while the window is visible, so you don't need an additional do_every_frame unless you
-- also need to do things while the window is closed.

-- If the window has a decoration with a close button (like decoration 1), you might need a way to know
-- when the users closes the window. This function can be used to setup a function to be called when the
-- user closes the window:
float_wnd_set_onclose(demo_wnd, "closed_demo")
-- The first parameter must be a handle to a window previously created with float_wnd_create and the
-- second parameter must be the name of a function. Note that unlike other function in FlyWithLua, you
-- can only pass the name of a function here, not an arbitrary lua string. This function will be called
-- when the user closes the window. After the window is closed, its builder function will not be called
-- again and it is illegal to use the window handle variable returned by float_wnd_create, e.g. it is illegal
-- to set the window title after it was closed.

-- imgui supports drawing images, so let's load one. The images must always be loaded globally to make sure they
-- are only loaded once per script!
image_id = float_wnd_load_image(SCRIPT_DIRECTORY .. "/imgui_demo.jpg")

-- imgui widgets have no state, that means we need to store
-- states of checkboxes, radio buttons etc. globally
makeRed = false
sliderVal = 0
choice = 1
angle = 0
text = ""
text2 = "/*\n The Pentium F00F bug, shorthand for F0 0F C7 C8,\nthe hexadecimal encoding of one offending instruction,\n more formally, the invalid operand with locked CMPXCHG8B\n instruction bug, is a design flaw in the majority of\n Intel Pentium, Pentium MMX, and Pentium OverDrive\n processors (all in the P5 microarchitecture).\n */\n"
text3 = ""

function build_demo(wnd, x, y)
    -- This function is called for every frame. Use this function to create your GUI.
    -- The first parameter (wnd) is the handle of the window. It is the same handle as the one returned
    -- by float_wnd_create. It is required if you have multiple windows that use the same builder function
    -- so you know which of these windows is currently being built.
    -- The x and y parameters are the current position of the window in OpenGL coordinates, i.e. the position
    -- of the lower left corner in global screen coordinates. These coordinates are only needed for using the
    -- graphics module on top of the imgui GUI. If you only use imgui, these coordinates are not required because
    -- imgui has its own coordinate system inside the window.

    -- If you additionally need the current width and height of the window, use these functions:
    local win_width = imgui.GetWindowWidth()
    local win_height = imgui.GetWindowHeight()
    -- These function return the window's width and height in boxels.

    -- The easiest thing we can do in imgui is drawing text. The function is called TextUnformatted because
    -- it doesn't support C-style format strings. imgui has a function called Text() that does support these
    -- format strings, but it is not supported in lua.
    imgui.TextUnformatted("Hello, World!")

    -- Since TextUnformatted does not support format strings, you can use lua to create a string on the fly:
    imgui.TextUnformatted("Window size: " .. win_width .. ", " .. win_height)

    -- imgui automatically arranges all widgets from top to bottom, so the following text is drawn below:
    imgui.TextUnformatted("Two Widgets")
    -- It is possible to put a widget on the same line, i.e. avoiding the automatic line feed:
    imgui.SameLine()
    imgui.TextUnformatted("One Line.")

    -- To maintain a tidy layout, let's show the next windows grouped in tree nodes.
    -- The user can collapse or expand these nodes to show or hide the content.
    -- It is not required to put widgets into tree nodes, this is just an example.
    if imgui.TreeNode("Styling Widgets") then
        -- The TreeNode function creates such a tree node. The parameter is the label of the node.
        -- The function returns true if the node is currently opened, so the if statement takes care
        -- that the contained widgets are only drawn if the node is currently expanded.

        -- It is possible to place widgets at arbitrary coordinates by manipulating the draw cursor's position.
        -- Let's center a text horizontally by first measuring its size and then putting it at the center of the line:
        local text = "Centered Text"
        local text_width, text_height = imgui.CalcTextSize(text)
        imgui.SetCursorPos(win_width / 2 - text_width / 2, imgui.GetCursorPosY())
        imgui.TextUnformatted(text)
        -- CalcTextSize returns the width and height of a text in boxels without drawing it.
        -- GetCursorPosX and GetCursorPosY return the position where the next widget will be placed.
        -- SetCursorPos can be used to manipulate this position.
        -- imgui's coordinates have the origin in the upper left and the axes grow to the right and to the bottom.

        -- The following function changes the color of all widgets until PopStyleColor is called.
        -- The format is AABBGGRR where A is alpha and B, G and R are blue, green and red.
        -- So if you have an HTML color like #2B65EC (ocean blue), swap the first two and last two hex digits
        -- and prefix it with FF to use it:
        imgui.PushStyleColor(imgui.constant.Col.Text, 0xFFEC652B)
        imgui.TextUnformatted("Ocean Blue Text")
        imgui.TextUnformatted("Also Ocean Blue")
        -- In this example we changed the value of imgui.constant.Col.Text which is the foreground text
        -- color. For other supported style colors that can be changed, see footnote [1] below.
        
        -- Restore the previous style, this _must_ be called after each pushed style:
        imgui.PopStyleColor()

        imgui.TextUnformatted("No longer Ocean Blue")

        -- This function must be called at the end of every tree node to tell imgui that the code after
        -- it is not part of the TreeNode anymore.
        imgui.TreePop()
    end
    
    if imgui.TreeNode("Buttons") then
        -- The following function creates a button:
        if imgui.Button("Push Me") then
            -- The Button function creates a button with the given label. It returns true in the
            -- moment where the button is released, i.e. the if statement is used to check whether
            -- the action behind the button should now be executed. Let's make the button toggle a variable:
            makeRed = not makeRed
        end
        
        -- Use the variable to change the color of the text
        if makeRed then
            imgui.PushStyleColor(imgui.constant.Col.Text, 0xFF0000FF)
        else
            imgui.PushStyleColor(imgui.constant.Col.Text, 0xFF00FF00)
        end

        imgui.TextUnformatted("Some Text")
        imgui.PopStyleColor()
        
        -- If you want to trigger the action while the button is clicked and not only when it is released:
        imgui.Button("Click and hold")
        if imgui.IsItemActive() then
            makeRed = not makeRed
        end
        
        -- Radio buttons: Specify whether the choice is currently active as parameter.
        -- Returns true when the choice is selected.
        if imgui.RadioButton("Choice 1", choice == 1) then
            choice = 1
        end
        if imgui.RadioButton("Choice 2", choice == 2) then
            choice = 2
        end
        if imgui.RadioButton("Choice 3", choice == 3) then
            choice = 3
        end
        
        imgui.TreePop()
    end
    
    if imgui.TreeNode("Checkboxes") then
        -- The following function creates a checkbox:
        local changed, newVal = imgui.Checkbox("Make text red", makeRed)
        -- The first parameter is the caption of the checkbox, it will be displayed right of the checkbox.
        -- The second parameter is the current boolean value.
        
        -- The function returns two values: a bool to indicate whether the value was changed and the new value. 
        if changed then
            makeRed = newVal
        end

        if makeRed then
            imgui.PushStyleColor(imgui.constant.Col.Text, 0xFF0000FF)
        else
            imgui.PushStyleColor(imgui.constant.Col.Text, 0xFF00FF00)
        end

        imgui.TextUnformatted("Some Text")
        imgui.PopStyleColor()
        imgui.TreePop()
    end
    
    if imgui.TreeNode("Sliders") then
        -- The following function creates a slider:
        local changed, newVal = imgui.SliderFloat("Slider Caption", sliderVal, 0, 100000, "Value: %.2f")
        -- The first parameter is the caption of the slider, it will be displayed right of the slider.
        -- The second parameter is the current position. Pass the variable that should be changed by the slider.
        -- The third and fourth parameter specify the range of the slider, i.e. the min and max value.
        -- The fourth parameter is a format string to show on the slider. Must contain a variant of "%f" in the text.
        -- The example format string rounds the value to two decimal places and prefixes the value with "Value: " on the slider.
        
        -- The function returns two values: a bool to indicate whether the value was changed and the new value. You must copy
        -- the new value into the slider variable if the value changed:
        if changed then
            sliderVal = newVal
        end

        -- Additionally, it is possible to create a non-linear slider by specifying the power of the slider:
        local changed, newVal = imgui.SliderFloat("Power Slider", sliderVal, 0, 100000, "Value: %.2f", 3.0)
        if changed then
            sliderVal = newVal
        end
        
        -- For non-decimals:
        local changed, newVal = imgui.SliderInt("Int Slider", sliderVal, 0, 100000, "Value: %.0f")
        if changed then
            sliderVal = newVal
        end
        
        -- For angles: 
        local changed, newVal = imgui.SliderAngle("Angle Slider", angle, -180, 180, "Value: %.0f")
        if changed then
            angle = newVal
        end

        imgui.TreePop()
    end
    
    if imgui.TreeNode("ComboBox") then
    
        local choices = {"Choice 1", "Choice 2", "Choice 3"}

        -- BeginCombo starts a combo box. The first parameter is the label, the
        -- second parameter the text of the currently selected choice.
        if imgui.BeginCombo("Combo Box", choices[choice]) then
            -- Loop over all choices
            for i = 1, #choices do
                -- The Selectable function adds a choice to a combobox.
                -- The first parameter is the label, the second one
                -- should be true if this choice is currently selected.
                if imgui.Selectable(choices[i], choice == i) then
                    -- Selectable returns true when a new choice was selected,
                    -- we should then copy the choice into our choice variable
                    choice = i
                end
            end
            -- _Must_ be called if and only if BeginCombo returns true, i.e.
            -- when the combobox is currently open
            imgui.EndCombo()
        end
        
        -- Here is an easier example to understand the loop above
        if imgui.BeginCombo("Combo Box 2", "", imgui.constant.ComboFlags.NoPreview) then
            if imgui.Selectable("Choice A", choice == 1) then
                choice = 1
            end
            if imgui.Selectable("Choice B", choice == 2) then
                choice = 2
            end
            if imgui.Selectable("Choice C", choice == 3) then
                choice = 3
            end
            imgui.EndCombo()
        end
        
    
        imgui.TreePop()
    end
    
    if imgui.TreeNode("Drag Controls") then
        -- These are somewhat similar to sliders but you can drag them multiple 
        -- times at the same position to continously increase or decrease the value
        local changed, newVal = imgui.DragFloat("Drag Float", sliderVal, 1.0, 0, 1000, "%.2f", 1.0)
        -- Parameters: Label, value, speed, min, max, caption, power
        
        if changed then
            sliderVal = newVal
        end

        local changed, newVal = imgui.DragInt("Drag Int", sliderVal, 1.0, 0, 1000, "%.2f", 1.0)
        if changed then
            sliderVal = newVal
        end
        
        imgui.TreePop()
    end
    
    if imgui.TreeNode("Plots") then
        -- demo data
        local values = {1, 3, 2, 5, 0, 6, -3}
        
        -- the following function creates a graphical plot of the values:
        imgui.PlotLines("Value Plot", values, #values)
        -- The first parameter is the caption
        -- The second parameters is the value array
        -- The third parameter is the number of values to plot (here: all values).
        -- The next parameters are optional:
        --  4: offset into the array to indicate where to start
        --  5: overlay text to show on the plot
        --  6: minimum y value for scaling. Pass FLT_MAX for auto-scale
        --  7: maximum y value for scaling. Pass FLT_MAX for auto-scale
        --  8: graph size width
        --  9: graph size height
        
        imgui.PlotLines("", values, #values - 1, 1, "Plot", -10, FLT_MAX, win_width, 100)
        -- Now the plot goes from -10 to the max value in the array and uses the full window width.
        -- The first value is ignored since we use 1 as offset and only go to num - 1
        
        -- Instead of using a line plot, it's also possible to use histogram bins. The parameters are the same.
        imgui.PlotHistogram("Value Histogram", values, #values)
        
        -- Note that you will have to calculate the actual histogram yourself, this histogram function simply
        -- uses bars instead of lines but doesn't do the histogram calculation.

        -- See the FPS example for an actual histogram and plotting of live data.

        imgui.TreePop()
    end
    
    if imgui.TreeNode("Input") then
        -- Allow the user to input text
        local changed, newText = imgui.InputText("One Line Text", text, 255)
        -- Parameters: Label, current text, maximum number of allowed characters

        if changed then
            text = newText
        end

        local changed, newText2 = imgui.InputTextMultiline("Multiline Text", text2, 500)
        -- Parameters: Label, current text, maximum number of allowed characters

        if changed then
            text2 = newText2
        end

        local changed, newText3 = imgui.InputTextWithHint("Text With a Hint", "hint", text3, 255)
        -- Parameters: Label, current text, maximum number of allowed characters

        if changed then
            text3 = newText3
        end
        
        -- Allow the user to input an integer, either using the keyboard or by clicking + / -
        local changed, newInt = imgui.InputInt("Input int", sliderVal)
        if changed then
            sliderVal = newInt
        end
        
        -- The +/- steps can optionally be changed:
        local changed, newInt = imgui.InputInt("Input int2", sliderVal, 10)
        if changed then
            sliderVal = newInt
        end

        imgui.TreePop()
    end
    
    if imgui.TreeNode("Drawing") then
        local cx, cy = imgui.GetCursorScreenPos()
        
        -- Create a kind of canvas of size 100x100.
        -- The previous variable remembers the top left
        -- edge of the canvas so we can use it as relative
        -- coordinates to draw inside the widget using
        -- absolute coordinates.
        imgui.Dummy(100, 100)
        
        -- Parameters: x1, y1, x2, y2, color, thickness
        imgui.DrawList_AddLine(cx, cy, cx + 20, cy + 20, 0xFF0000FF, 2)
        imgui.DrawList_AddLine(cx, cy + 20, cx + 20, cy, 0xFF0000FF, 2)
        
        -- Prameters: x1, y1, x2, y2, color, roundness
        imgui.DrawList_AddRect(cx, cy, cx + 20, cy + 20, 0xFFFF0000, 0.5)

        -- Prameters: x1, y1, x2, y2, color, roundness
        imgui.DrawList_AddRectFilled(cx + 20, cy, cx + 40, cy + 20, 0xFFFF0000, 0.5)
    
        -- Parameters: x1, y1, x2, y2, x3, y3, color, thickness 
        imgui.DrawList_AddTriangle(cx, cy + 50, cx + 40, cy + 50, cx + 20, cy + 20, 0xFF00FF00, 2.0)

        -- Parameters: x1, y1, x2, y2, x3, y3, color
        imgui.DrawList_AddTriangleFilled(cx + 40, cy + 50, cx + 80, cy + 50, cx + 40, cy + 20, 0xFF00FF00)

        -- Parameters: x1, y1, r, color, segments, thickness
        imgui.DrawList_AddCircle(cx + 80, cy + 20, 20, 0xFF00FF00, 20, 1.0)

        -- Parameters: x1, y1, r, color
        imgui.DrawList_AddCircleFilled(cx + 80, cy + 20, 10, 0xFF00FF00)
        
        imgui.TreePop()
    end
    
    if imgui.TreeNode("Images") then
        -- Draw a previously loaded image
        imgui.Image(image_id, win_width, 450 / 800 * win_width)
        -- Prameters: image id returned by float_wnd_load_image, diplay width, display height
        
        imgui.TreePop()
    end
    
    if imgui.TreeNode("Misc") then
        -- Create a bullet style enumeration
        imgui.Bullet(); imgui.TextUnformatted("Bullet")
        imgui.Bullet(); imgui.TextUnformatted("Style")
        imgui.Bullet(); imgui.TextUnformatted("Enumeration")
        
        -- Draw a separation line
        imgui.Separator()
        
        -- Show a progress bar, 1 = 100%
        imgui.ProgressBar(0.5)

        _, makeRed = imgui.Checkbox("Complicated Item", makeRed)
        if imgui.IsItemActive() then
            -- We can create a tooltip that is shown while the item is being clicked (click & hold):
            imgui.BeginTooltip()
            -- This function configures the wrapping inside the toolbox and thereby its width
            imgui.PushTextWrapPos(imgui.GetFontSize() * 10)
            imgui.TextUnformatted("Here's a long explanation text that describes the purpose of this widget")
            -- Reset the wrapping, this must always be done if you used PushTextWrapPos
            imgui.PopTextWrapPos()
            imgui.EndTooltip()
        end
        
        -- Note that caption strings must be unique inside a window! If you need to have
        -- multiple widgets with the same caption, append ## followed by a unique id for an
        -- invisible caption extension:

        imgui.Button("Button##1")
        imgui.Button("Button##2")
        
        -- Another variant: Create a custom ID namespace using PushID and PopID in case you want to create
        -- widgets from inside a loop
        for i = 1, 5 do
            imgui.PushID(i)
            imgui.Button("Button")
            imgui.PopID()
        end

        imgui.TreePop()
    end
end

function closed_demo(wnd)
    -- This function is called when the user closes the window. Drawing or calling imgui
    -- functions is not allowed in this function as the window is already destroyed.
end

-- Footnotes:
-- [1]: Colors that can be changed with PushStyleColor:
    -- Text, TextDisabled, TextSelectedBg,
    -- WindowBg, ChildBg, PopupBg,
    -- Border, BorderShadow,
    -- FrameBg, FrameBgHovered, FrameBgActive,
    -- TitleBg, TitleBgActive, TitleBgCollapsed,
    -- MenuBarBg,
    -- ScrollbarBg, ScrollbarGrab, ScrollbarGrabHovered, ScrollbarGrabActive,
    -- CheckMark, SliderGrab, SliderGrabActive,
    -- Button, ButtonHovered, ButtonActive,
    -- Header, HeaderHovered, HeaderActive,
    -- Separator, SeparatorHovered, SeparatorActive,
    -- ResizeGrip, ResizeGripHovered, ResizeGripActive,
    -- PlotLines, PlotLinesHovered, PlotHistogram, PlotHistogramHovered,
    -- ModalWindowDarkening
