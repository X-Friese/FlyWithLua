-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Script:  visual_trim_system
-- Author:  Carsten Lynker
-- Version: 1.0
-- Build:   2015-02-14
-- Licence: Same as FlyWithLua (MIT licence)
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Description:
-- Display trim and steering information and provide mouse wheel trim function.
-- Please read the manual, you find it inside the Documentation subfolder.
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
require "graphics"

-- local variable result in a better performance than global variables
local VTSXT, VTSYT

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Position of the dignostic display elements (center point in screen coordinates).
-- Edit the values to replace the disgnostic display and it's alpha blending.
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

-- When the simulator starts or the user changes the window size (if X-Plane isn't fullscreen),
-- it uses wrong screen coordinates. As a solution, we use a repaeted function to set the values.
function set_visual_trim_display_position()
    -- Trim and stick info area
    VTSXT = SCREEN_WIDTH - 105
    VTSYT = SCREEN_HIGHT -125
    -- Alpha value for the diagnostic display (OpenGL alpha blending, range 0.0 = invisible to 1.0 = full solid)
    VTSALPHA = 0.5
end

-- The step size a mouse wheel click will change the trim value
VisualTrim_sensitivity = 0.005

-- -- -- 8< -- -- -- -- end of area a normal user should edit -- -- -- -- -- -- -- -- -- -- -- -- -- -- --


-- init the values
set_visual_trim_display_position()
-- repeat init every 10 seconds
do_sometimes("set_visual_trim_display_position()")

-- all DataRefs we need (this is a mordern style script)
dataref("VTS_aileron_trim", "sim/cockpit2/controls/aileron_trim", "writable")
dataref("VTS_elevator_trim", "sim/cockpit2/controls/elevator_trim", "writable")
dataref("VTS_rudder_trim", "sim/cockpit2/controls/rudder_trim", "writable")
dataref("yoke_pitch_ratio", "sim/joystick/yoke_pitch_ratio", "writable")
dataref("yoke_roll_ratio", "sim/joystick/yoke_roll_ratio", "writable")
dataref("yoke_heading_ratio", "sim/joystick/yoke_heading_ratio", "writable")

-- the function to show the diagnostic display
function show_visual_trim()
    -- do nothing until the mouse is over the right position
    if (MOUSE_X < VTSXT-100) or (MOUSE_Y < VTSYT-100) then return end 
    
    -- init OpenGL
    XPLMSetGraphicsState(0, 0, 0, 1, 1, 0, 0)
        
    -- draw the trim info frame
    graphics.set_color(0,0,0,VTSALPHA)
    graphics.draw_rectangle(VTSXT-100, VTSYT-100, VTSXT+100, VTSYT+100)
    graphics.set_color(1,1,1,VTSALPHA/4)
    graphics.draw_line(VTSXT-100, VTSYT, VTSXT+100, VTSYT)
    graphics.draw_line(VTSXT, VTSYT-100, VTSXT, VTSYT+100)

    -- draw stick info
    graphics.set_color(0,1,0,VTSALPHA)
    graphics.set_width(2)
    graphics.draw_line(VTSXT+100*yoke_roll_ratio-10, VTSYT-100*yoke_pitch_ratio, VTSXT+100*yoke_roll_ratio+10, VTSYT-100*yoke_pitch_ratio)
    graphics.draw_line(VTSXT+100*yoke_roll_ratio, VTSYT-100*yoke_pitch_ratio-10, VTSXT+100*yoke_roll_ratio, VTSYT-100*yoke_pitch_ratio+10)
    graphics.draw_angle_arrow(VTSXT, VTSYT, yoke_heading_ratio*90, 100, 20, 2)
    
    -- draw trim info cross and arrow
    graphics.set_color(1,0,1,VTSALPHA)
    graphics.set_width(2)
    graphics.draw_line(VTSXT+100*VTS_aileron_trim-10, VTSYT-100*VTS_elevator_trim, VTSXT+100*VTS_aileron_trim+10, VTSYT-100*VTS_elevator_trim)
    graphics.draw_line(VTSXT+100*VTS_aileron_trim, VTSYT-100*VTS_elevator_trim-10, VTSXT+100*VTS_aileron_trim, VTSYT-100*VTS_elevator_trim+10)
    graphics.draw_angle_arrow(VTSXT, VTSYT, VTS_rudder_trim*90, 100, 20, 2)
    
    -- draw function info
    if (MOUSE_X > VTSXT) and (MOUSE_Y < VTSYT) then
        draw_string(VTSXT - 95, VTSYT - 95, "Wheel sets elevator trim", "grey")
    end
    if (MOUSE_X > VTSXT) and (MOUSE_Y > VTSYT) then
        draw_string(VTSXT - 95, VTSYT - 95, "Wheel sets aileron trim", "grey")
    end
    if (MOUSE_X < VTSXT) and (MOUSE_Y < VTSYT) then
        draw_string(VTSXT - 95, VTSYT - 95, "Wheel sets rudder trim", "grey")
    end
    if (MOUSE_X < VTSXT) and (MOUSE_Y > VTSYT) then
        draw_string(VTSXT - 95, VTSYT - 95, "Click to reset trim", "grey")
    end
end

do_every_draw("show_visual_trim()")

-- reset the trimming
function vts_reset_trim()
    if (MOUSE_X < VTSXT) and (MOUSE_Y > VTSYT) and (MOUSE_X > VTSXT - 100) and (MOUSE_Y < VTSYT + 100) then
        VTS_elevator_trim = 0
        VTS_rudder_trim = 0
        VTS_aileron_trim = 0
        RESUME_MOUSE_CLICK = true
    end
end

do_on_mouse_click("vts_reset_trim()")

-- provide mouse wheel function
function visual_trim_mouse_wheel_action()
    -- do nothing until the mouse is over the right position
    if (MOUSE_X < VTSXT-100) or (MOUSE_Y < VTSYT-100) then return end 

    if (MOUSE_X > VTSXT) and (MOUSE_Y < VTSYT) then
        VTS_elevator_trim = VTS_elevator_trim - VisualTrim_sensitivity * MOUSE_WHEEL_CLICKS
        RESUME_MOUSE_WHEEL = true
    end
    if (MOUSE_X > VTSXT) and (MOUSE_Y > VTSYT) then
        VTS_aileron_trim = VTS_aileron_trim + VisualTrim_sensitivity * MOUSE_WHEEL_CLICKS
        RESUME_MOUSE_WHEEL = true
    end
    if (MOUSE_X < VTSXT) and (MOUSE_Y < VTSYT) then
        VTS_rudder_trim = VTS_rudder_trim + VisualTrim_sensitivity * MOUSE_WHEEL_CLICKS
        RESUME_MOUSE_WHEEL = true
    end
    
    -- limit the wheel movement
    if VTS_elevator_trim >  1 then VTS_elevator_trim =  1 end
    if VTS_elevator_trim < -1 then VTS_elevator_trim = -1 end
    if VTS_aileron_trim >  1 then VTS_aileron_trim =  1 end
    if VTS_aileron_trim < -1 then VTS_aileron_trim = -1 end
    if VTS_rudder_trim >  1 then VTS_rudder_trim =  1 end
    if VTS_rudder_trim < -1 then VTS_rudder_trim = -1 end
end

do_on_mouse_wheel("visual_trim_mouse_wheel_action()")
