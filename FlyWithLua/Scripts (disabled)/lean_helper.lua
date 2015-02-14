require("graphics")

dataref("xp_egt1", "sim/flightmodel/engine/ENGN_EGT_c", "readonly", 0)
dataref("xp_egt2", "sim/flightmodel/engine/ENGN_EGT_c", "readonly", 1)
dataref("xp_mixt1", "sim/flightmodel/engine/ENGN_mixt", "readonly", 0)
dataref("xp_mixt2", "sim/flightmodel/engine/ENGN_mixt", "readonly", 1)
last_mixt1 = xp_mixt1
last_mixt2 = xp_mixt2

-- screen coordinates
LEAN_X = 100
LEAN_Y = 100

-- where to draw the red line?
EGT_left_max = 0
EGT_right_max = 0

-- should we draw the helper?
show_EGT_helper = 0

function draw_egt()
	if show_EGT_helper < os.clock() then return end
    
    -- reset OpenGL state
    XPLMSetGraphicsState(0, 0, 0, 1, 1, 0, 0)
	
	local left_egt = xp_egt1 - 800
	local right_egt = xp_egt2 - 800
	local i
	
	if left_egt < 10 then
		left_egt = 10
	end
	if right_egt < 10 then
		right_egt = 10
	end
	
	-- check for max values
	if EGT_left_max < left_egt then
		EGT_left_max = left_egt
	end
	if EGT_right_max < right_egt then
		EGT_right_max = right_egt
	end
	
	-- draw background lines
	graphics.set_color(1,1,1,0.5)
	for i = 100, 750, 50 do
		graphics.draw_line(LEAN_X, LEAN_Y + i, LEAN_X + 200, LEAN_Y + i)
	end

	-- draw max EGT lines
	if EGT_left_max > 200 then
		glLineWidth(1.0)
		graphics.set_color(1,0,0,0.75)
		graphics.draw_line(LEAN_X + 5, LEAN_Y + EGT_left_max, LEAN_X + 95, LEAN_Y + EGT_left_max)
		graphics.set_color(1,1,0,0.75)
		graphics.draw_line(LEAN_X + 15, LEAN_Y + EGT_left_max - 25, LEAN_X + 85, LEAN_Y + EGT_left_max -25)
		graphics.draw_line(LEAN_X + 15, LEAN_Y + EGT_left_max - 75, LEAN_X + 85, LEAN_Y + EGT_left_max -75)
		glLineWidth(2.0)
		graphics.set_color(0,1,0,0.75)
		graphics.draw_line(LEAN_X + 5, LEAN_Y + EGT_left_max - 50, LEAN_X + 95, LEAN_Y + EGT_left_max - 50)
	end
	if EGT_right_max > 200 then
		glLineWidth(1.0)
		graphics.set_color(1,0,0,0.75)
		graphics.draw_line(LEAN_X + 105, LEAN_Y + EGT_right_max, LEAN_X + 195, LEAN_Y + EGT_right_max)
		graphics.set_color(1,1,0,0.75)
		graphics.draw_line(LEAN_X + 115, LEAN_Y + EGT_right_max - 25, LEAN_X + 185, LEAN_Y + EGT_right_max -25)
		graphics.draw_line(LEAN_X + 115, LEAN_Y + EGT_right_max - 75, LEAN_X + 185, LEAN_Y + EGT_right_max -75)
		glLineWidth(2.0)
		graphics.set_color(0,1,0,0.75)
		graphics.draw_line(LEAN_X + 105, LEAN_Y + EGT_right_max - 50, LEAN_X + 195, LEAN_Y + EGT_right_max - 50)
	end
	
	-- draw EGT rectangles
	glLineWidth(1.0)
	graphics.set_color(0,1,1,0.5)
	graphics.draw_rectangle(LEAN_X + 25, LEAN_Y, LEAN_X + 75, LEAN_Y + left_egt)
	graphics.draw_rectangle(LEAN_X + 125, LEAN_Y, LEAN_X + 175, LEAN_Y + right_egt)
	
	-- draw strings
	for i = 100, 700, 100 do
		draw_string(LEAN_X + 220, LEAN_Y + i, 800 + i, "grey")
	end
end

function show_EGT_or_not()
	if math.abs(xp_mixt1 - last_mixt1) > 0.02 then
		if show_EGT_helper < os.clock() then
			show_EGT_helper = os.clock() + 8
			last_mixt1 = xp_mixt1
			last_mixt2 = xp_mixt2
			EGT_left_max = 0
			EGT_right_max = 0
			return
		else
			show_EGT_helper = os.clock() + 8
			last_mixt1 = xp_mixt1
			last_mixt2 = xp_mixt2
			return
		end
	end
	if math.abs(xp_mixt1 - last_mixt1) > 0.02 then
		if show_EGT_helper < os.clock() then
			show_EGT_helper = os.clock() + 8
			last_mixt1 = xp_mixt1
			last_mixt2 = xp_mixt2
			EGT_left_max = 0
			EGT_right_max = 0
			return
		else
			show_EGT_helper = os.clock() + 8
			last_mixt1 = xp_mixt1
			last_mixt2 = xp_mixt2
			return
		end
	end
end

if PLANE_ICAO == "C172" or PLANE_ICAO == "BE58" or PLANE_TAILNUMBER == "N399CK" then
    do_every_draw("draw_egt()")
    do_often("show_EGT_or_not()")
end