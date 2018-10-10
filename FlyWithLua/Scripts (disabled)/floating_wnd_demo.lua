-- floating window demo script
-- William Good 09-07-2018

-- It is suggested to look at the window created by this script while reading this.

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("floating windows not supported by your FlyWithLua version")
    return
end

require("graphics")

lastClickX3 = 640 / 2
lastClickY3 = 480 / 2

local line_y_inc = 50

-- imgui only works inside a floating window, so we need to create one first:
demo_floating_wnd = float_wnd_create(640, 480, 1, false)
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
float_wnd_set_title(demo_floating_wnd, "floating window Demo")
-- The first parameter must be a handle to a window previously created with float_wnd_create and the
-- second parameter is the title of the window.


-- This function sets the initial position of the window.
-- We are trying to put the window in the center of the screen.
-- We find the SCREEN_WIDTH and divide it by 2 then subtract the window width by 2.
-- We next find the SCREEN_HIGHT and divide it by 2 then subtract the window higth divided by 2.
float_wnd_set_position(demo_floating_wnd, SCREEN_WIDTH / 2 - 640 / 2, SCREEN_HIGHT / 2 - 480 / 2)
-- The first parameter must be a handle to a window previously created with float_wnd_create and the
-- second and third parameters set the initial position of the window.


-- Sets the name of the ondraw function.
float_wnd_set_ondraw(demo_floating_wnd, "on_draw_floating_window")
-- The first parameter must be a handle to a window previously created with float_wnd_create and the
-- second parameter set the name of the ondraw function.


-- Sets the name of the onclick function.
float_wnd_set_onclick(demo_floating_wnd, "on_click_floating_window")
-- The first parameter must be a handle to a window previously created with float_wnd_create and the
-- second parameter set the name of the onclick function.


-- Sets the name of the onclose function.
float_wnd_set_onclose(demo_floating_wnd, "on_close_floating_window")
-- The first parameter must be a handle to a window previously created with float_wnd_create and the
-- second parameter set the name of the onclose function.


-- x and y are the origin of the window, i.e. the lower left
-- x increases to the right, y increases to the top
function on_draw_floating_window(demo_floating_wnd, x3, y3)
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
	glColor3f(1,1,0)
    draw_string_Helvetica_10(x3, y3 + (line_y_inc * 3) , "Line Seven")
    glColor3f(1,0,1)
    draw_string_Helvetica_12(x3, y3 + (line_y_inc * 2) , "Line Eight")
    glColor3f(0,1,1)
    draw_string_Helvetica_18(x3, y3 + (line_y_inc * 1) , "Line Nine")
end


-- x and y are relative from the origin of the window, i.e. the lower left
function on_click_floating_window(demo_floating_wnd, x3, y3)
    lastClickX3 = x3
    lastClickY3 = y3
end


-- When on_close it called, it is illegal to do anything with the wnd variable
-- It is also not allowed to create new windows in on_close!
function on_close_floating_window(demo_floating_wnd)
end
