-- FILE:        alt_set_1000.lua
-- VERSION:     1.0
-- AUTHOR:      Carsten Lynker
-- DESCRIPTION: Will increase or decrese the value of the autopilot's altitude setting
--              to the next 1000th, to tune in the values more effective.
--              You can easily modify this script to change the step size.


-- set the dataref ALT to read and write the altitude tuned into the autopilot
dataref("ALT", "sim/cockpit2/autopilot/altitude_dial_ft", "writable")

-- create a command to increase the altitude value of the autopilot
create_command("FlyWithLua/autopilot/inc_alt_1000",
               "Increase the altitude to the next 1000 value",
               "ALT = math.floor(ALT / 1000 + 1) * 1000",
               "",
               "")

-- create a command to increase the altitude value of the autopilot
create_command("FlyWithLua/autopilot/dec_alt_1000",
               "Decrease the altitude to the next 1000 value",
               "ALT = math.floor( (ALT - 1) / 1000) * 1000",
               "",
               "")
