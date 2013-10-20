-- A quick settings menu driven by the mouse handler
-- (c) Carsten Lynker, Markus
-- build 2013-08-10

require "graphics"

local qs_landing_light = dataref_table("sim/cockpit/electrical/landing_lights_on")
local qs_heading = dataref_table("sim/cockpit/autopilot/heading")
local qs_real_heading = dataref_table("sim/flightmodel/position/magpsi")
local qs_altitude = dataref_table("sim/cockpit/autopilot/altitude")
local qs_real_altitude = dataref_table("sim/flightmodel/position/elevation")
local qs_baro = dataref_table("sim/cockpit/misc/barometer_setting")
local qs_real_baro = dataref_table("sim/weather/barometer_sealevel_inhg")
local qs_beacon_light = dataref_table("sim/cockpit/electrical/beacon_lights_on")

function draw_quick_settings()
	-- does we have to draw anything?
	if MOUSE_Y > 100 or MOUSE_X > 600 then
		return
	end
	
	-- init the graphics system
	XPLMSetGraphicsState(0,0,0,1,1,0,0)
	
	-- draw transparent backgroud
	graphics.set_color(0, 0, 0, 0.5)
	graphics.draw_rectangle(0, 0, 600, 100)
	
	-- draw colored switches info
	if qs_beacon_light[0] == 0 then
		graphics.set_color(0, 0, 0, 0.5)
	else
		graphics.set_color(0, 1, 0, 0.5)
	end
	graphics.draw_rectangle(0, 0, 40, 30)
	
	-- draw lines around the hole block
	graphics.set_color(1, 1, 1, 1)
	graphics.set_width(2)
	graphics.draw_line(1, 1, 1, 100)
	graphics.draw_line(1, 100, 600, 100)
	graphics.draw_line(600, 100, 600, 1)
	graphics.draw_line(600, 1, 1, 1)
	
	-- draw some separation lines
	graphics.draw_line(1, 70, 599, 70)
	graphics.draw_line(40, 70, 40, 99)
	graphics.draw_line(110, 70, 110, 99)
	graphics.draw_line(160, 70, 160, 99)

	graphics.draw_line(1, 30, 599, 30)
	graphics.draw_line(40, 1, 40, 29)
	
	-- draw the info text
	draw_string_Helvetica_10(5, 89, "HDG")
	draw_string_Helvetica_18(5, 74, math.floor(qs_heading[0]))
	draw_string_Helvetica_10(45, 89, "ALT")
	draw_string_Helvetica_18(45, 74, math.floor(qs_altitude[0]))
	draw_string_Helvetica_10(115, 89, "BARO")
	draw_string_Helvetica_18(115, 74, math.floor(qs_baro[0] * 33.8637526 + 0.5))
	
	draw_string_Helvetica_10(5, 17, "Beacon")
	draw_string_Helvetica_10(8, 5, "Light")
end

do_every_draw("draw_quick_settings()")

function qs_mouse_click_events()
	-- we will only react once
	if MOUSE_STATUS ~= "down" then
		return
	end
	
	-- HDG
	if MOUSE_X < 40 and MOUSE_Y > 70 and MOUSE_Y < 100 then
		qs_heading[0] = qs_real_heading[0]
		RESUME_MOUSE_CLICK = true
	end
	
	-- ALT
	if MOUSE_X > 40 and MOUSE_X < 110 and MOUSE_Y > 70 and MOUSE_Y < 100 then
		qs_altitude[0] = qs_real_altitude[0] * 3.2808399
		RESUME_MOUSE_CLICK = true
	end

	-- BARO
	if MOUSE_X > 110 and MOUSE_X < 160 and MOUSE_Y > 70 and MOUSE_Y < 100 then
		qs_baro[0] = qs_real_baro[0]
		RESUME_MOUSE_CLICK = true
	end

	-- Beacon Light
	if MOUSE_X < 40 and MOUSE_Y < 30 then
		if qs_beacon_light[0] == 0 then
			qs_beacon_light[0] = 1
		else
			qs_beacon_light[0] = 0
		end
		RESUME_MOUSE_CLICK = true
	end
end

do_on_mouse_click("qs_mouse_click_events()")

function qs_mouse_wheel_events()
	-- HDG
	if MOUSE_X < 40 and MOUSE_Y > 70 and MOUSE_Y < 100 then
		qs_heading[0] = 10 * (math.floor(qs_heading[0]/10) + MOUSE_WHEEL_CLICKS)
		if qs_heading[0] < 0 then
			qs_heading[0] = 350
		end
		if qs_heading[0] > 360 then
			qs_heading[0] = 10
		end
		RESUME_MOUSE_WHEEL = true
	end
	
	-- ALT
	if MOUSE_X > 40 and MOUSE_X < 110 and MOUSE_Y > 70 and MOUSE_Y < 100 then
		qs_altitude[0] = 100 * (math.floor(qs_altitude[0]/100) + MOUSE_WHEEL_CLICKS)
		if qs_altitude[0] < 0 then
			qs_altitude[0] = 0
		end
		RESUME_MOUSE_WHEEL = true
	end

	-- BARO
	if MOUSE_X > 110 and MOUSE_X < 160 and MOUSE_Y > 70 and MOUSE_Y < 100 then
		qs_baro[0] = qs_baro[0] + MOUSE_WHEEL_CLICKS * 0.01
		RESUME_MOUSE_WHEEL = true
	end
end

do_on_mouse_wheel("qs_mouse_wheel_events()")