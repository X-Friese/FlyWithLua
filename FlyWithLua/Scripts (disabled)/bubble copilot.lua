DataRef("QNH_Pilot", "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot", "writable")
DataRef("QNH_CoPilot", "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot", "writable")
DataRef("QNH", "sim/weather/barometer_sealevel_inhg")

local starting_time = os.clock()
local QNH_y1
local QNH_y2
local QNH_x
local QNH_problem = false

function Steffi_says()
	local pos = 0
	if math.abs(QNH-QNH_Pilot) > 0.01 then
		QNH_y1 = pos
		pos, QNH_x = big_bubble(20, pos, "Please check the barometer setting!", "Your altimeter is showing wrong values.", string.format("The right QNH is %2.2f in/Hg or %.1f hPa.", QNH, QNH * 33.8637526), "Or just click on this bubble and I'll fix it for you.")
		QNH_y2 = pos
		QNH_problem = true
	else
		QNH_problem = false
	end
	if os.clock() < starting_time + 10 then
		pos = big_bubble(20, pos, "Steffi says:", "Hi!", "My name is Steffi, and I'm your virtual copilot.", "Let's enjoy our flight.")
	end
end

do_every_draw("Steffi_says()")

function Steffi_does_on_clicks()
	if QNH_problem and MOUSE_X > 10 and MOUSE_X < QNH_x and MOUSE_Y > QNH_y1 and MOUSE_Y < QNH_y2 then
		QNH_Pilot = QNH
		QNH_CoPilot = QNH
		RESUME_MOUSE_CLICK = true
	end
end

do_on_mouse_click("Steffi_does_on_clicks()")