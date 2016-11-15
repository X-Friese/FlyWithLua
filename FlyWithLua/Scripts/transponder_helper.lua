-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Script:  transponder_helper
-- Version: 1.1
-- Build:   2012-10-15
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Description:
-- Sets the transponder to active or standby depending on altitude and gear force.
-- The pilot can set the transponder by his own, the script will only act during takeoff/touchdown.
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

-- using pre-defined DataRefs from module
require "radio"

-- discover the radio altitude to check if we are in the air
-- as a heli can hoover, we toggle at 25m agl
-- and to be heli friendly, we check the force on the gear before turning the transponder to standby
-- if we want a DataRef readonly, we can leave the third parameter away
dataref("ro_fnrml_gear", "sim/flightmodel/forces/fnrml_gear")
dataref("ro_y_agl", "sim/flightmodel/position/y_agl")

-- check only while takeup or landing
local last_y_agl = 0
local last_fnrml_gear = 0

-- we start in Europe most of the time
SQUAWK = 7000

-- turn on your transponder when flying higher than 25m agl
function check_transponder()
	if (transponder_help == true) then
		if ((ro_y_agl > 25) and (last_y_agl <= 25) and (TRANSPONDER_MODE ~= 2)) then
			TRANSPONDER_MODE = 2
			XPLMSpeakString("Transponder set to active")
		end
		if ((ro_fnrml_gear > 1000) and (last_fnrml_gear <= 1000) and (TRANSPONDER_MODE > 1)) then
			TRANSPONDER_MODE = 1
			XPLMSpeakString("Transponder set to standby")
		end
        last_y_agl = ro_y_agl
        last_fnrml_gear = ro_fnrml_gear
	end
end

-- check it every 10 sec
do_sometimes("check_transponder()")

-- make a switchable menu entry, default is on
add_macro("Automatically set Transponder", "transponder_help = true", "transponder_help = false", "activate")
