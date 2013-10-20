-- A little COM1 radio handler driven by the mouse handler
-- This is just a demo how to build interactive HUDs
-- (c) Carsten Lynker
-- build 2013-08-24

require "graphics"

dataref("COM1", "sim/cockpit/radios/com1_freq_hz", "writable")
dataref("COM1_POWER", "sim/cockpit2/radios/actuators/com1_power", "writable")

local COM1_STATION_NAME = "unknown"

function draw_little_radio()
	-- does we have to draw anything?
	if MOUSE_Y > 50 or MOUSE_X < SCREEN_WIDTH - 100 then
		return
	end
	
	-- init the graphics system
	XPLMSetGraphicsState(0,0,0,1,1,0,0)
	
	-- draw transparent backgroud
	graphics.set_color(0, 0, 0, 0.5)
	graphics.draw_rectangle(SCREEN_WIDTH - 100, 0, SCREEN_WIDTH, 50)
	
	-- draw colored switches info
	if COM1_POWER == 0 then
		graphics.set_color(0, 0, 0, 0.5)
	else
		graphics.set_color(0, 1, 0, 0.5)
	end
	graphics.draw_rectangle(SCREEN_WIDTH - 100, 0, SCREEN_WIDTH - 92, 50)
	
	-- draw lines around the hole block
	graphics.set_color(1, 1, 1, 1)
	graphics.set_width(2)
	graphics.draw_line(SCREEN_WIDTH - 100, 1, SCREEN_WIDTH - 100, 50)
	graphics.draw_line(SCREEN_WIDTH - 100, 50, SCREEN_WIDTH - 1, 50)
	graphics.draw_line(SCREEN_WIDTH - 1, 50, SCREEN_WIDTH - 1, 1)
	graphics.draw_line(SCREEN_WIDTH, 1, SCREEN_WIDTH - 100, 1)
		
	-- draw the info text
	draw_string_Helvetica_10(SCREEN_WIDTH - 90, 38, "COM1")
	draw_string_Helvetica_18(SCREEN_WIDTH - 90, 20, string.format("%3.2f", COM1 / 100))
	draw_string_Helvetica_10(SCREEN_WIDTH - 90,  5, COM1_STATION_NAME)
end

do_every_draw("draw_little_radio()")

function little_radio_mouse_click_events()
	-- we will only react once
	if MOUSE_STATUS ~= "down" then
		return
	end
	
	if MOUSE_X > SCREEN_WIDTH - 100 and MOUSE_Y < 50 then
		if COM1_POWER == 0 then
			COM1_POWER = 1
		else
			COM1_POWER = 0
		end
		RESUME_MOUSE_CLICK = true
	end
end

do_on_mouse_click("little_radio_mouse_click_events()")

function little_radio_wheel_events()
	-- big steps
	if MOUSE_X > SCREEN_WIDTH - 100 and MOUSE_X < SCREEN_WIDTH - 60 and MOUSE_Y < 50 then
		COM1 = COM1 + MOUSE_WHEEL_CLICKS * 100
		if COM1 > 13697 then
			COM1 = 13697
		end
		if COM1 < 11800 then
			COM1 = 11800
		end
		RESUME_MOUSE_WHEEL = true
	end
	
	-- little steps
	if MOUSE_X >= SCREEN_WIDTH - 60 and MOUSE_X < SCREEN_WIDTH - 45 and MOUSE_Y < 50 then
		COM1 = COM1 + MOUSE_WHEEL_CLICKS * 10
		if COM1 > 13697 then
			COM1 = 13697
		end
		if COM1 < 11800 then
			COM1 = 11800
		end
		RESUME_MOUSE_WHEEL = true
	end
	
	-- tiny steps
	if MOUSE_X >= SCREEN_WIDTH - 45 and MOUSE_Y < 50 then
		COM1 = COM1 + MOUSE_WHEEL_CLICKS
		if COM1 > 13697 then
			COM1 = 13697
		end
		if COM1 < 11800 then
			COM1 = 11800
		end
		RESUME_MOUSE_WHEEL = true
	end
	
end

do_on_mouse_wheel("little_radio_wheel_events()")