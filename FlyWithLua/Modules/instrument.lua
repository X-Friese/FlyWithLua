-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- Lua module "instrument.lua" v1.1    -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
module(..., package.seeall);
require("graphics")

dataref("xp_MPR_in_hg0", "sim/cockpit2/engine/indicators/MPR_in_hg", "readonly", 0)
dataref("xp_prop_speed_rpm0", "sim/cockpit2/engine/indicators/prop_speed_rpm", "readonly", 0)

function manifold_pressure_and_propeller_speed(x, y, inHg_range, rpm_range)
    -- calculate the angles for the pointer
    local rpm
    local inHg
    if xp_MPR_in_hg0 < inHg_range then
        inHg = xp_MPR_in_hg0 / inHg_range * 300 - 150
    else
        inHg = 150
    end
    if xp_prop_speed_rpm0 < rpm_range then
        rpm = xp_prop_speed_rpm0 / rpm_range * 300 - 150
    else
        rpm = 150
    end
    
    -- first define OpenGL state
    XPLMSetGraphicsState(0, 0, 0, 0, 0, 0, 0)

    -- draw the instruments base
    glColor3f(0, 0, 0)
    graphics.draw_filled_circle(x + 100, y + 100, 100)
    glColor3f(1, 1, 1)
    graphics.draw_arc(x + 100, y + 100, -150, 150, 100)
    draw_string(x + 90, y + 50, "inHg")
    draw_string(x + 75, y + 30, "x100 rpm", "blue")
    
    -- redefine OpenGL state after drawing strings
    XPLMSetGraphicsState(0, 0, 0, 0, 0, 0, 0)
    
    -- draw tick marks
    local tick_angle
    -- small rpm ticks
    glColor3f(0, 0, 1)
    for tick = 100, rpm_range, 100 do
        tick_angle = tick / rpm_range * 300 - 150
        graphics.draw_tick_mark(x + 100, y + 100, tick_angle, 100, 10, 1)
    end
    -- big rpm ticks
    for tick = 500, rpm_range, 500 do
        tick_angle = tick / rpm_range * 300 - 150
        graphics.draw_tick_mark(x + 100, y + 100, tick_angle, 100, 20, 3)
    end
    -- small inHg ticks
    glColor3f(1, 1, 1)
    for tick = 1, inHg_range, 1 do
        tick_angle = tick / inHg_range * 300 - 150
        graphics.draw_tick_mark(x + 100, y + 100, tick_angle, 100, 10, 1)
    end
    -- big inHg ticks
    for tick = 5, inHg_range, 5 do
        tick_angle = tick / inHg_range * 300 - 150
        graphics.draw_tick_mark(x + 100, y + 100, tick_angle, 100, 20, 3)
    end
    
    local x1
    local y1

    -- draw numbers to the big rpm tick marks
    for tick = 5, rpm_range / 100, 5 do
        tick_angle = tick / rpm_range * 30000 - 150
        x1, y1 = graphics.move_angle(x + 100, y + 100, tick_angle, 65)
        x1 = x1 - 5
        y1 = y1 - 5
        draw_string(x1, y1, tick, "blue")
    end
    
    -- draw numbers to the big inHg tick marks
    for tick = 5, inHg_range, 5 do
        tick_angle = tick / inHg_range * 300 - 150
        x1, y1 = graphics.move_angle(x + 100, y + 100, tick_angle, 65)
        x1 = x1 - 5
        y1 = y1 - 5
        draw_string(x1, y1, tick, "white")
    end

    -- redefine OpenGL state after drawing strings
    XPLMSetGraphicsState(0, 0, 0, 0, 0, 0, 0)

    -- draw the pointer
    glColor3f(0, 0, 1)
    graphics.draw_angle_arrow(x + 100, y + 100, rpm, 100, 20, 3)
    glColor3f(1, 1, 1)
    if inHg > rpm + 15 then
        glColor3f(1, 0, 0)
    end
    if inHg > rpm + 30 then
        glColor3f(math.abs(math.sin(os.clock()*5)), 0, 0)
    end
    graphics.draw_angle_arrow(x + 100, y + 100, inHg, 100, 20, 3)
    
    -- make the middle of the instrument more eye-candy
    glColor3f(0, 0, 0)
    graphics.draw_filled_circle(x + 100, y + 100, 7.5)
    glColor3f(1, 1, 1)
    graphics.draw_circle(x + 100, y + 100, 7.5)
end