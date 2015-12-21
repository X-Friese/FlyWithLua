-- This script made by "INTJ Mastermind" (see here: http://forums.x-plane.org/index.php?showtopic=91838&p=977784 )
-- allows to set the autopilot in steps of 100 ft.
-- When the assigned button (or key) is pressed shortly, you get a single 100 ft step.
-- When holding down the button, the altitude starts to increase or decrease quickly.

-- This script is licenced free with the same MIT Licence FlyWithLua uses. So you are allowed to
-- use and modify it for your own needs.
-- It is part of FlyWithLua since version 2.4.3, but should be run with older versions 2.x withot any issue.
-- Thanks to Mastermind for sharing his work.

dataref("ALT", "sim/cockpit2/autopilot/altitude_dial_ft", "writable")

local alt_time = 1
local alt_value = 1

create_command("FlyWithLua/autopilot/inc_alt_100",
               "Increase the altitude to the next 100 value",
               "alt_time = os.clock()+1\nalt_value = ALT\nALT = math.floor(alt_value/100 + 1)*100",
               "if os.clock() > alt_time then\nALT = math.floor(alt_value/100 + 50*(os.clock() - alt_time)+1)*100\nend",
               "")

create_command("FlyWithLua/autopilot/dec_alt_100",
               "Decrease the altitude to the next 100 value",
               "alt_time = os.clock()+1\nalt_value = ALT\nALT = math.floor(alt_value/100 - 1)*100",
               "if os.clock() > alt_time then\nALT = math.floor(alt_value/100 - 50*(os.clock() - alt_time))*100\nend",
               "")