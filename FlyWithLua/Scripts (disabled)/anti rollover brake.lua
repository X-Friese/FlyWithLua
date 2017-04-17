-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Script:  anti rollover brake
-- Version: 1.2
-- Build:   2017-04-16
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Description:
-- Providing a custom command to use the brakes: FlyWithLua/flight_controls/ARB
-- You can use this command to bind it to a joystick button. If you press the button, the script
-- will retract flaps (flaps are bad for short way braking). If you hold down the button, the
-- script will mangage the brakes depending on the pitch moment due to gear forces. This will provide
-- your plane from rollover desaster.
-- If you use the rudder pedals (twisting your stick), the power of the brake will be
-- different on left and right brake, to help steering the plane on the ground.
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- use this variable to turn on or off visual braking help
   local draw_brake_into_rectangles = true
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- you have to use these global variables to control the anti rollover brake
   plane_has_speedbrakes = false
   plane_has_reverser = false
   plane_has_flaps = true
   plane_is_taildragger = false
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

-- DataRefs we need
local pitch_moment = dataref_table("sim/flightmodel/forces/M_total")
local acf_weight = dataref_table("sim/flightmodel/weight/m_total")
local flaprqst = dataref_table("sim/flightmodel/controls/flaprqst")
local sbrkrqst = dataref_table("sim/flightmodel/controls/sbrkrqst")
local l_brake_add = dataref_table("sim/flightmodel/controls/l_brake_add")
local r_brake_add = dataref_table("sim/flightmodel/controls/r_brake_add")
local parkbrake = dataref_table("sim/flightmodel/controls/parkbrake")
local groundspeed = dataref_table("sim/flightmodel/position/groundspeed")
local prop_mode = dataref_table("sim/cockpit2/engine/actuators/prop_mode")
local acf_num_engines = dataref_table("sim/aircraft/engine/acf_num_engines")
local yoke_heading_ratio = dataref_table("sim/joystick/yoke_heading_ratio")  -- range: -1.0 .. +1.0

-- ground steering helper variables
ARB_is_braking = false
ground_steering_helper_is_on = true

-- number of engines
local noe = acf_num_engines[0] - 1

-- starting to brake
function start_ARB()
    -- let other scripts know you are braking
    ARB_is_braking = true
    -- retract flaps
    if plane_has_flaps then flaprqst[0] = 0 end
    -- don't act when slower than 15m/s
    if groundspeed[0] < 15 then return end
    -- deploy speedbrakes full
    if plane_has_speedbrakes then sbrkrqst[0] = 1 end
end

-- continue braking
function do_ARB()
    -- reverse thrust above 20m/s
    if plane_has_reverser then
        if groundspeed[0] > 20 then
            for i=0, noe do
                prop_mode[i] = 3
            end
        else
            for i=0, noe do
                prop_mode[i] = 1
            end
        end
    end
    
    local full_brake_power = 1 + pitch_moment[0]/acf_weight[0]
    if full_brake_power > 1 then full_brake_power = 1 end
    if full_brake_power < 0 then full_brake_power = 0 end
    if plane_is_taildragger then
        if full_brake_power > 0.5 then
            full_brake_power = 0.5
        end
    end
    
    -- we brake using the parkbrake DataRef
    parkbrake[0] = full_brake_power * 0.5
end

function stop_ARB()
    -- retract speedbrakes
    if plane_has_speedbrakes then sbrkrqst[0] = 0 end
    -- reset prop mode to normal
    if plane_has_reverser then
        for i=0, noe do
            prop_mode[i] = 1
        end
    end
    -- release brakes
    parkbrake[0] = 0
    -- say goodby to brakes
    ARB_is_braking = false
end

-- create the command
create_command("FlyWithLua/flight_controls/ARB", "anti rollover brake assistance", "start_ARB()", "do_ARB()", "stop_ARB()")

function draw_brakes()
    -- should we draw the info?
    if not draw_brake_into_rectangles then return end
    
    -- is there something we can draw?
    if (parkbrake[0] == 0) and (l_brake_add[0] == 0) and (r_brake_add[0] == 0) then return end
    
    -- choose the state we need
    XPLMSetGraphicsState(0, 0, 0, 1, 1, 0, 0)

    -- draw info
    glColor4f(1,0,0,0.5)
    glRectf(SCREEN_WIDTH/2+parkbrake[0]*200, 0, SCREEN_WIDTH/2-parkbrake[0]*200, 45)
    
    glColor4f(1,1,0,0.5)
    glRectf(50, 0, 0, l_brake_add[0]*200)
    glRectf(SCREEN_WIDTH, 0, SCREEN_WIDTH - 50, r_brake_add[0]*200)
end

-- register drawing
do_every_draw("draw_brakes()")

-- help steering on ground
function ground_steering_helper()
    -- does the user wants us to work?
    if not ground_steering_helper_is_on then return end
    
    -- don't disturb the ARB
    if ARB_is_braking then return end
    
    -- stop working at 20m/s or above
    if groundspeed[0] > 20 then
        l_brake_add[0] = 0
        r_brake_add[0] = 0
        return
    end
    
    -- else do the work!
    if yoke_heading_ratio[0] < 0 then
        l_brake_add[0] = -yoke_heading_ratio[0] / 2
        r_brake_add[0] = 0
    else
        l_brake_add[0] = 0
        r_brake_add[0] = yoke_heading_ratio[0] / 2
    end
end

-- register the function
do_every_frame("ground_steering_helper()")

-- add a macro menu entry
add_macro(  "rudder pedals should brake beneath 20m/s",
            "ground_steering_helper_is_on = true",
            "ground_steering_helper_is_on = false",
            "activate")
