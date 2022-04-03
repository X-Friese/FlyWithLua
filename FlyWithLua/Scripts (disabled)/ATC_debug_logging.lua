-- Turn on ATC Flow Logging in Log.txt file
-- William R. Good (SparkerInVR) 04-03-22
-- from https://developer.x-plane.com/article/atc-flow-authoring-in-wed/

dataref("rwy_flow_logging", "sim/private/controls/atc/debug/rwy_flow", "writable")
dataref("rwy_selection_logging", "sim/private/controls/atc/debug/rwy_selection", "writable")

run_once = 0

function turn_on_logging()
	if run_once == 0 then
		rwy_flow_logging = 1
		rwy_selection_logging = 1
		run_once = 1
	else

	end

end



do_sometimes("turn_on_logging()")
