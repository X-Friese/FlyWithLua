

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end


require("graphics")

dataref("first_axis", "sim/joystick/joystick_axis_values", "readonly", 0)

lastClickX3 = 640 / 2
lastClickY3 = 480 / 2

local line_y_inc = 50

-- x and y are the origin of the window, i.e. the lower left
-- x increases to the right, y increases to the top
function on_draw3(wnd3, x3, y3)
    centerX3 = x3 + lastClickX3
    centerY3 = y3 + lastClickY3
    glColor3f(1,1,0)
    draw_string_Times_Roman_24(x3, y3 + (line_y_inc * 9) , "Line One")
    glColor3f(1,0,1)
    draw_string_Times_Roman_24(x3, y3 + (line_y_inc * 8) , "Line Two")
    glColor3f(0,0,1)
    draw_string_Times_Roman_24(x3, y3 + (line_y_inc * 7) , "Line Three")
    glColor3f(0,1,0)
    draw_string_Times_Roman_24(x3, y3 + (line_y_inc * 6) , "Line Four")
    glColor3f(0,1,1)
    draw_string_Times_Roman_24(x3, y3 + (line_y_inc * 5) , "Line Five")
    glColor3f(1,0,0)
    draw_string_Times_Roman_24(x3, y3 + (line_y_inc * 4) , "Line Six")
    draw_string_Helvetica_10(x3, y3 + (line_y_inc * 3) , "Line Seven")
    glColor3f(1,0,1)
    draw_string_Helvetica_12(x3, y3 + (line_y_inc * 2) , "Line Eight")
    glColor3f(0,1,1)
    draw_string_Helvetica_18(x3, y3 + (line_y_inc * 1) , "Line Nine")


end

-- x and y are relative from the origin of the window, i.e. the lower left
function on_click3(wnd3, x3, y3)
    lastClickX3 = x3
    lastClickY3 = y3
end

-- When on_close it called, it is illegal to do anything with the wnd variable
-- It is also not allowed to create new windows in on_close!
function on_close(wnd3)
end

-- width, height, decoration style as per XPLMCreateWindowEx. 1 for solid background, 3 for transparent
wnd3 = float_wnd_create(640, 480, 1, false)
float_wnd_set_title(wnd3, "FlyWithLua Floating Window Text Font Color Test")
float_wnd_set_ondraw(wnd3, "on_draw3")
float_wnd_set_onclick(wnd3, "on_click3")
float_wnd_set_onclose(wnd3, "on_close3")
