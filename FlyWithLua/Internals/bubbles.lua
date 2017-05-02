-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- --                                                                                                 -- --
-- --      D               ''  T                    D    II    T      !!                              -- --
-- --      D               ''  T           EE       D          T      !!                              -- --
-- --    DDD   OO   N NN      TTT         E  E    DDD    II   TTT     !!                              -- --
-- --   D  D  O  O  NN  N      T          EEE    D  D     I    T      !!                              -- --
-- --   D  D  O  O  N   N      T          E      D  D     I    T                                      -- --
-- --    DDD   OO   N   N       TT         EEE    DDD      I    TT    !!                              -- --
-- --                                                                                                 -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

-- bubbles.lua version 1.0 build 2013-09-22

function bubble(x, y, title, ...)
	x = x or 20
	y = y or 0
	x = x - 10
	y = y + 10
	title = title or "FlyWithLua"

	-- negative values are not allowed
	if x < 0 then x = 0 end
	if y < 0 then y = 0 end
	
	-- measure the bubble
	local bubble_hight = 35 + select("#", ...) * 12
	local bubble_width = measure_string(title, "Helvetica_12")
	for i, text_line in ipairs({...}) do
		if measure_string(text_line, "Helvetica_10") > bubble_width then
			bubble_width = measure_string(text_line, "Helvetica_10")
		end
	end
	
	bubble_width = bubble_width + 20
	
	-- check if the bubble will fit onto the screen
	if x+bubble_width > SCREEN_WIDTH then x = SCREEN_WIDTH - bubble_width end
	if y+bubble_hight > SCREEN_HIGHT then y = SCREEN_HIGHT - bubble_hight end

	max_y = y+bubble_hight
	
	-- draw the bubble background
	XPLMSetGraphicsState(0,0,0,1,1,0,0)
	glColor4f(0, 0, 0, 0.75)
	glBegin_POLYGON()
	glVertex2f(x, y)
	glVertex2f(x, max_y)
	glVertex2f(x+bubble_width, max_y)
	glVertex2f(x+bubble_width, y)
	glVertex2f(x+20, y)
	glVertex2f(x+10, y-10)
	glEnd()
	glColor4f(1, 1, 1, 1)
	glBegin_LINE_LOOP()
	glVertex2f(x, y)
	glVertex2f(x, y+bubble_hight)
	glVertex2f(x+bubble_width, y+bubble_hight)
	glVertex2f(x+bubble_width, y)
	glVertex2f(x+20, y)
	glVertex2f(x+10, y-10)
	glEnd()
	
	-- draw the text
	x = x + 10
	y = max_y - 20
	draw_string_Helvetica_12(x, y, title)
	if select("#", ...) > 0 then
		for i,text_line in ipairs({...}) do
			y = y - 12
			draw_string_Helvetica_10(x, y, text_line)
		end
	end
	return max_y, x+bubble_width
end

function big_bubble(x, y, title, ...)
	x = x or 20
	y = y or 0
	x = x - 10
	y = y + 10
	title = title or "FlyWithLua"
	
	-- negative values are not allowed
	if x < 0 then x = 0 end
	if y < 0 then y = 0 end
	
	-- measure the bubble
	local bubble_hight = 40 + select("#", ...) * 15
	local bubble_width = measure_string(title, "Helvetica_18")
	for i, text_line in ipairs({...}) do
		if measure_string(text_line, "Helvetica_12") > bubble_width then
			bubble_width = measure_string(text_line, "Helvetica_12")
		end
	end
	
	bubble_width = bubble_width + 20
	
	-- check if the bubble will fit onto the screen
	if x+bubble_width > SCREEN_WIDTH then x = SCREEN_WIDTH - bubble_width end
	if y+bubble_hight > SCREEN_HIGHT then y = SCREEN_HIGHT - bubble_hight end

	max_y = y+bubble_hight
	
	-- draw the bubble background
	XPLMSetGraphicsState(0,0,0,1,1,0,0)
	glColor4f(0, 0, 0, 0.75)
	glBegin_POLYGON()
	glVertex2f(x, y)
	glVertex2f(x, max_y)
	glVertex2f(x+bubble_width, max_y)
	glVertex2f(x+bubble_width, y)
	glVertex2f(x+20, y)
	glVertex2f(x+10, y-10)
	glEnd()
	glColor4f(1, 1, 1, 1)
	glBegin_LINE_LOOP()
	glVertex2f(x, y)
	glVertex2f(x, y+bubble_hight)
	glVertex2f(x+bubble_width, y+bubble_hight)
	glVertex2f(x+bubble_width, y)
	glVertex2f(x+20, y)
	glVertex2f(x+10, y-10)
	glEnd()
	
	-- draw the text
	x = x + 10
	y = max_y - 25
	draw_string_Helvetica_18(x, y, title)
	y = y - 2
	if select("#", ...) > 0 then
		for i,text_line in ipairs({...}) do
			y = y - 15
			draw_string_Helvetica_12(x, y, text_line)
		end
	end
	return max_y, x+bubble_width
end

function huge_bubble(x, y, title, ...)
	x = x or 20
	y = y or 0
	x = x - 10
	y = y + 10
	title = title or "FlyWithLua"
	
	-- negative values are not allowed
	if x < 0 then x = 0 end
	if y < 0 then y = 0 end
	
	-- measure the bubble
	local bubble_hight = 50 + select("#", ...) * 20
	local bubble_width = measure_string(title, "Times_Roman_24")
	for i, text_line in ipairs({...}) do
		if measure_string(text_line, "Helvetica_18") > bubble_width then
			bubble_width = measure_string(text_line, "Helvetica_18")
		end
	end
	
	bubble_width = bubble_width + 20
	
	-- check if the bubble will fit onto the screen
	if x+bubble_width > SCREEN_WIDTH then x = SCREEN_WIDTH - bubble_width end
	if y+bubble_hight > SCREEN_HIGHT then y = SCREEN_HIGHT - bubble_hight end

	max_y = y+bubble_hight
	
	-- draw the bubble background
	XPLMSetGraphicsState(0,0,0,1,1,0,0)
	glColor4f(0, 0, 0, 0.75)
	glBegin_POLYGON()
	glVertex2f(x, y)
	glVertex2f(x, max_y)
	glVertex2f(x+bubble_width, max_y)
	glVertex2f(x+bubble_width, y)
	glVertex2f(x+20, y)
	glVertex2f(x+10, y-10)
	glEnd()
	glColor4f(1, 1, 1, 1)
	glBegin_LINE_LOOP()
	glVertex2f(x, y)
	glVertex2f(x, y+bubble_hight)
	glVertex2f(x+bubble_width, y+bubble_hight)
	glVertex2f(x+bubble_width, y)
	glVertex2f(x+20, y)
	glVertex2f(x+10, y-10)
	glEnd()
	
	-- draw the text
	x = x + 10
	y = max_y - 30
	draw_string_Times_Roman_24(x, y, title)
	y = y - 5
	if select("#", ...) > 0 then
		for i,text_line in ipairs({...}) do
			y = y - 20
			draw_string_Helvetica_18(x, y, text_line)
		end
	end
	return max_y, x+bubble_width
end
