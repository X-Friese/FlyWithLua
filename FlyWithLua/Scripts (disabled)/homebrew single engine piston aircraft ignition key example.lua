-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- This is a demonstrator how to get two switches and one push button to work as a starter key switch for single piston aircrafts.  --
--                                                                                                                                  --
-- It is the homebrew cockpit example from the 15. German Flight Simulator Conference at Paderborn Haxterberg Airfield (EDLR).      --
--                                                                                                                                  --
-- If you want to test it for your own, please check the button numbers. There is a sniffer function implemented into FlyWithLua    --
-- that helps you getting the right button numbers. You find it inside the menu                                                     --
-- "Plugins" --> FlyWithLua" --> "FlyWithLua Macros" --> "Show joystick button numbers"                                             --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --


-- first we need to make access to the DataRef for the ignition key
local ignition_keys = dataref_table("sim/cockpit2/engine/actuators/ignition_key")

-- and we need the numbers of the switches and the push button (please check them with the joystick button sniffer)
local right_magneto_switch = 7
local left_magneto_switch  = 8
local ignition_push_button = 9

-- now we create a function to force the dataref to a calculated value
function set_ignition_key_for_single_piston_aircraft()
    if button(ignition_push_button) then
        ignition_keys[0] = 4
    elseif     button(right_magneto_switch) and     button(left_magneto_switch) then
        ignition_keys[0] = 3
    elseif not button(right_magneto_switch) and     button(left_magneto_switch) then
        ignition_keys[0] = 2
    elseif     button(right_magneto_switch) and not button(left_magneto_switch) then
        ignition_keys[0] = 1
    else
        ignition_keys[0] = 0
    end
end

-- this function has to be executed every frame
do_every_frame("set_ignition_key_for_single_piston_aircraft()")

create_switch(13, "sim/cockpit2/electrical/battery_on")