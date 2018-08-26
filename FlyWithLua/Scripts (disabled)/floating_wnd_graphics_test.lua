-- July 22 2018

-- This is the original graphics test.lua in the Scripts (disabled) folder
-- modified to work with floating wondows


if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

require("graphics")

dataref("first_axis", "sim/joystick/joystick_axis_values", "readonly", 0)

flLastClickX = 640 / 2
flLastClickY = 480 / 2

-- x and y are the origin of the window, i.e. the lower left
-- x increases to the right, y increases to the top
function fl_on_draw(wnd, x, y)
    centerX = x + flLastClickX
    centerY = y + flLastClickY

    for i = 0, 360, 30 do
        graphics.set_color(0, 0, 1)
        graphics.draw_angle_line(centerX, centerY, i, 200)
        graphics.set_color(1, 0, 0)
        graphics.draw_angle_arrow(centerX, centerY, i + 15, 100, 20, 2.5)
        graphics.set_color(0, 1, 0)
        graphics.draw_tick_mark(centerX, centerY, i + 10, 200, 20, 2.5)
        graphics.draw_outer_tracer(centerX, centerY, i + 20, 200)
        graphics.set_color(1, 1, 0)
        graphics.draw_inner_tracer(centerX, centerY, i + 15, 200)
    end
    graphics.set_color(1, 1, 1)
    graphics.draw_circle(centerX, centerY, 200)
    graphics.draw_circle(centerX, centerY, 100)
    graphics.draw_filled_circle(centerX, centerY, 10)
    graphics.draw_angle_line(centerX, centerY, first_axis * 360, 200)
end

-- x and y are relative from the origin of the window, i.e. the lower left
-- state: 1 = mouse down, 2 = mouse drag, 3 = mouse up
function fl_on_click(wnd, x, y, state)
    flLastClickX = x
    flLastClickY = y
end

-- When on_close it called, it is illegal to do anything with the wnd variable outside of this function
-- It is also not allowed to create new windows in on_close!
function fl_on_close(wnd)
end

-- width, height, decoration style as per XPLMCreateWindowEx. 1 for solid background, 3 for transparent
local wnd = float_wnd_create(640, 480, 1, false)
float_wnd_set_title(wnd, "Graphics Test for Floating Windows")
float_wnd_set_ondraw(wnd, "fl_on_draw")
float_wnd_set_onclick(wnd, "fl_on_click")
float_wnd_set_onclose(wnd, "fl_on_close")

-- To destroy a window:
-- float_wnd_destroy(wnd) 
-- after that, do not use the wnd variable anymore
