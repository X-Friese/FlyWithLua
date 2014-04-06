local now = os.clock()

for i = 1, 1000000 do
	set("sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot", get("sim/weather/barometer_sealevel_inhg"))
	set("sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot", get("sim/weather/barometer_sealevel_inhg"))
end

local duration_ms = (os.clock() - now) * 1000
logMsg("FlyWithLua Info: One million accesses with two get/set has a duration of " .. duration_ms .. " ms.")

DataRef("QNH_Pilot", "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot", "writable")
DataRef("QNH_CoPilot", "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot", "writable")
DataRef("QNH", "sim/weather/barometer_sealevel_inhg")

now = os.clock()

for i = 1, 1000000 do
	QNH_Pilot = QNH
	QNH_CoPilot = QNH
end

duration_ms = (os.clock() - now) * 1000
logMsg("FlyWithLua Info: One million accesses with two variables has a duration of " .. duration_ms .. " ms.")

QNH_Pilot_T = dataref_table("sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot")
QNH_CoPilot_T = dataref_table("sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot")
QNH_T = dataref_table("sim/weather/barometer_sealevel_inhg")

now = os.clock()

for i = 1, 1000000 do
	QNH_Pilot_T[0] = QNH_T[0]
	QNH_CoPilot_T[0] = QNH_T[0]
end

duration_ms = (os.clock() - now) * 1000
logMsg("FlyWithLua Info: One million accesses with two magic tables has a duration of " .. duration_ms .. " ms.")
