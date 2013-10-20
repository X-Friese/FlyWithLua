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

local gfx = require "graphics"

local gear_deployment = dataref_table("sim/aircraft/parts/acf_gear_deploy")
local gear_leglen = dataref_table("sim/aircraft/parts/acf_gear_leglen")

function draw_gear_deployment_info()
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
            gfx.draw_filled_circle(SCREEN_WIDTH/2-105+i*30, SCREEN_HIGHT-15, 10)
        end
    end
end

do_every_draw("draw_gear_deployment_info()")