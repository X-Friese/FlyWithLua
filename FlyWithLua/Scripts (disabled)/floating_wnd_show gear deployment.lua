-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Script:  show gear deployment
-- Version: 1.0
-- Build:   2012-10-18
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Description:
-- Simply shows the gear deployment of all six gears a plane can have.
-- The deployment will be shown as dots colored yellow during deployment
-- and green when fully deployed. If you can't see a dot, it is fully
-- rejected.
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

-- Modified to support floating windows
-- William Good 2018-08-26

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end


local gfx = require "graphics"

local gear_deployment = dataref_table("sim/aircraft/parts/acf_gear_deploy")
local gear_leglen = dataref_table("sim/aircraft/parts/acf_gear_leglen")

function dgdi_on_draw(dgdi_wnd, x, y)
    -- choose the state we need
    XPLMSetGraphicsState(0, 0, 0, 1, 1, 0, 0)

    -- draw the info
    for i = 0, 5 do
        -- choose the color
        if gear_deployment[i] == 1 then glColor4f(0,1,0,0.5)
        elseif gear_deployment[i] == 0 then glColor4f(0,0,0,0.5)
        else glColor4f(1,1,0,0.5) end
        
        -- draw the circle (gears with a leglen of 0 don't exist)
        if gear_leglen[i] > 0 then
--            gfx.draw_filled_circle(SCREEN_WIDTH/2-105+i*30, SCREEN_HIGHT-15, 10)
			gfx.draw_filled_circle(x + 10 + i*30, y + 10, 10)
        end
    end
end

-- When on_close it called, it is illegal to do anything with the wnd variable outside of this function
-- It is also not allowed to create new windows in on_close!
function dgdi_on_close(dgdi_wnd)
end

-- width, height, decoration style as per XPLMCreateWindowEx. 1 for solid background, 3 for transparent
local dgdi_wnd = float_wnd_create(640, 480, 1, false)
float_wnd_set_title(dgdi_wnd, "Draw Gear Deployment for Floating Windows")
float_wnd_set_ondraw(dgdi_wnd, "dgdi_on_draw")
float_wnd_set_onclick(dgdi_wnd, "dgdi_on_click")
float_wnd_set_onclose(dgdi_wnd, "dgdi_on_close")

-- To destroy a window:
-- float_wnd_destroy(wnd) 
-- after that, do not use the wnd variable anymore


-- do_every_draw("draw_gear_deployment_info()")
