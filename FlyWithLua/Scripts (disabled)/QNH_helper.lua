DataRef( "QNH_Pilot", "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot" )
last_QNH_Pilot = QNH_Pilot
QNH_display_time = os.clock()

function QNH_helper()
	-- if the QNH setting has changed, we want to display the info for 3 sec
	if QNH_Pilot ~= last_QNH_Pilot then
		QNH_display_time = os.clock() + 3.0
	end
	-- we place the info above the mouse pointer
	if os.clock() < QNH_display_time then
		big_bubble(MOUSE_X, MOUSE_Y, "a QNH of " .. math.floor( (QNH_Pilot * 100) + 0.5) / 100 .. "in/hg is " .. math.floor(QNH_Pilot * 33.8637526 + 0.5) .. "hPa in Europe")
		last_QNH_Pilot = QNH_Pilot
	end
end

do_every_draw("QNH_helper()")