-- -- -- -- -- -- -- -- -- -- -- -- --
-- -- Lua module "HUD.lua" v1.0  -- --
-- -- -- -- -- -- -- -- -- -- -- -- --
module(..., package.seeall);
require("graphics")

local HUD_X = 0
local HUD_Y = 0
local HUD_W = 200
local HUD_H = 50

local HUD_X_MIN = SCREEN_WIDTH
local HUD_X_MAX = 0
local HUD_Y_MIN = SCREEN_HIGHT
local HUD_Y_MAX = 0

local DRAW_BACKGROUND_PHASE = ""
local DRAW_LINES_PHASE = ""
local MOUSE_WHEEL_ACTION = ""
local MOUSE_CLICK_ACTION = ""
local DRAW_STRINGS_PHASE = ""

local HUD_ELEMENT_NAME = "unknown"
local HUD_NAME = "MY_LITTLE_HUD"
local HUD_COUNTER = 0
local HUD_MODE_ONLY = true

function create_element(name, x, y, w, h, r, g, b, a)
	-- set module's global element position
	HUD_X = HUD_X_MIN + x or HUD_X_MIN
	HUD_Y = HUD_Y_MIN + y or HUD_Y_MIN
	HUD_W = w or HUD_W
	HUD_H = h or HUD_H
	HUD_ELEMENT_NAME = name or "unknown"
	r = r or 1
	g = g or 1
	b = b or 1
	a = a or 1
	
	-- allow relative positions from right or upper screen border
	local x1 = HUD_X
	local x2 = HUD_X + HUD_W
	local y1 = HUD_Y
	local y2 = HUD_Y + HUD_H
	
	if HUD_X_MIN < 0 then
		x1 = string.format("SCREEN_WIDTH+(%i)", HUD_X_MIN + x)
		x2 = string.format("SCREEN_WIDTH+(%i)", HUD_X_MIN + x + HUD_W)
	end
	if HUD_Y_MIN < 0 then
		y1 = string.format("SCREEN_HIGHT+(%i)", HUD_Y_MIN + y)
		y2 = string.format("SCREEN_HIGHT+(%i)", HUD_Y_MIN + y + HUD_H)
	end
	
	-- draw lines around the element
	DRAW_LINES_PHASE = DRAW_LINES_PHASE .. "  -- draw lines around element '" .. HUD_ELEMENT_NAME .. "'\n"
	DRAW_LINES_PHASE = DRAW_LINES_PHASE .. "  graphics.set_color(" .. r .. "," .. g .. "," .. b .. "," .. a .. ")\n"
	DRAW_LINES_PHASE = DRAW_LINES_PHASE .. "  graphics.draw_line(" .. x1 .. ", " .. y1 .. ", " .. x2 .. ", " .. y1 .. ")\n"
	DRAW_LINES_PHASE = DRAW_LINES_PHASE .. "  graphics.draw_line(" .. x2 .. ", " .. y1 .. ", " .. x2 .. ", " .. y2 .. ")\n"
	DRAW_LINES_PHASE = DRAW_LINES_PHASE .. "  graphics.draw_line(" .. x2 .. ", " .. y2 .. ", " .. x1 .. ", " .. y2 .. ")\n"
	DRAW_LINES_PHASE = DRAW_LINES_PHASE .. "  graphics.draw_line(" .. x1 .. ", " .. y2 .. ", " .. x1 .. ", " .. y1 .. ")\n\n"
end

function create_backlight_indicator(x1, y1, x2, y2, condition, r, g, b, a)
	-- error checking
	if not condition then return end
	
	-- set colors
	r = r or 0
	g = g or 1
	b = b or 0
	a = a or 0.5
	
	-- position of the indicator
	x1 = x1 + HUD_X or HUD_X
	y1 = y1 + HUD_Y or HUD_Y
	x2 = x2 + x1 or HUD_X + HUD_W
	y2 = y2 + y1 or HUD_Y + HUD_H
	
	-- allow relative positions from right or upper screen border
	if HUD_X_MIN < 0 then
		x1 = string.format("SCREEN_WIDTH+(%i)", x1)
		x2 = string.format("SCREEN_WIDTH+(%i)", x2)
	end
	if HUD_Y_MIN < 0 then
		y1 = string.format("SCREEN_HIGHT+(%i)", y1)
		y2 = string.format("SCREEN_HIGHT+(%i)", y2)
	end

	-- prepare drawing the background
	DRAW_BACKGROUND_PHASE = DRAW_BACKGROUND_PHASE .. "  -- draw backlight indicator for element '" .. HUD_ELEMENT_NAME .. "'\n"
	DRAW_BACKGROUND_PHASE = DRAW_BACKGROUND_PHASE .. "  if " .. condition .. " then\n    graphics.set_color(" .. r .. "," .. g .. "," .. b .. "," .. a .. ")\n    graphics.draw_rectangle(" .. x1 .. "," .. y1 .. "," .. x2 .. "," .. y2 .. ")\n  end\n\n"
end

function create_click_action(x1, y1, x2, y2, action)
	-- error checking
	if not action then return end
	
	-- position of the indicator
	x1 = x1 + HUD_X or HUD_X
	y1 = y1 + HUD_Y or HUD_Y
	x2 = x2 + x1 or HUD_X + HUD_W
	y2 = y2 + y1 or HUD_Y + HUD_H
	
	-- allow relative positions from right or upper screen border
	if HUD_X_MIN < 0 then
		x1 = string.format("SCREEN_WIDTH+(%i)", x1)
		x2 = string.format("SCREEN_WIDTH+(%i)", x2)
	end
	if HUD_Y_MIN < 0 then
		y1 = string.format("SCREEN_HIGHT+(%i)", y1)
		y2 = string.format("SCREEN_HIGHT+(%i)", y2)
	end

	-- prepare the action
	MOUSE_CLICK_ACTION = MOUSE_CLICK_ACTION .. "  -- mouse click action for element '" .. HUD_ELEMENT_NAME .. "'\n"
	MOUSE_CLICK_ACTION = MOUSE_CLICK_ACTION .. "  if MOUSE_X >= " .. x1 .. " and MOUSE_X <= " .. x2 .. " and MOUSE_Y >= " .. y1 .. " and MOUSE_Y <= " .. y2 .. " then\n     " .. action .. "\n   RESUME_MOUSE_CLICK = true\n  end\n\n"
end

function create_wheel_action(x1, y1, x2, y2, action)
	-- error checking
	if not action then return end
	
	-- position of the indicator
	x1 = x1 + HUD_X or HUD_X
	y1 = y1 + HUD_Y or HUD_Y
	x2 = x2 + x1 or HUD_X + HUD_W
	y2 = y2 + y1 or HUD_Y + HUD_H
	
	-- allow relative positions from right or upper screen border
	if HUD_X_MIN < 0 then
		x1 = string.format("SCREEN_WIDTH+(%i)", x1)
		x2 = string.format("SCREEN_WIDTH+(%i)", x2)
	end
	if HUD_Y_MIN < 0 then
		y1 = string.format("SCREEN_HIGHT+(%i)", y1)
		y2 = string.format("SCREEN_HIGHT+(%i)", y2)
	end

	-- prepare the action
	MOUSE_WHEEL_ACTION = MOUSE_WHEEL_ACTION .. "  -- mouse wheel action for element '" .. HUD_ELEMENT_NAME .. "'\n"
	MOUSE_WHEEL_ACTION = MOUSE_WHEEL_ACTION .. "  if MOUSE_X >= " .. x1 .. " and MOUSE_X <= " .. x2 .. " and MOUSE_Y >= " .. y1 .. " and MOUSE_Y <= " .. y2 .. " then\n     " .. action .. "\n   RESUME_MOUSE_WHEEL = true\n  end\n\n"
end

function create_click_switch(x1, y1, x2, y2, variable, value, alternative_value)
	-- error checking
	if not variable then return end
	
	-- position of the indicator
	x1 = x1 + HUD_X or HUD_X
	y1 = y1 + HUD_Y or HUD_Y
	x2 = x2 + x1 or HUD_X + HUD_W
	y2 = y2 + y1 or HUD_Y + HUD_H
	
	-- allow relative positions from right or upper screen border
	if HUD_X_MIN < 0 then
		x1 = string.format("SCREEN_WIDTH+(%i)", x1)
		x2 = string.format("SCREEN_WIDTH+(%i)", x2)
	end
	if HUD_Y_MIN < 0 then
		y1 = string.format("SCREEN_HIGHT+(%i)", y1)
		y2 = string.format("SCREEN_HIGHT+(%i)", y2)
	end

	-- default values?
	value = value or 0
	alternative_value = alternative_value or 1
	
	-- prepare the action
	MOUSE_CLICK_ACTION = MOUSE_CLICK_ACTION .. "  -- mouse click switch for element '" .. HUD_ELEMENT_NAME .. "'\n"
	MOUSE_CLICK_ACTION = MOUSE_CLICK_ACTION .. "  if MOUSE_X >= " .. x1 .. " and MOUSE_X <= " .. x2 .. " and MOUSE_Y >= " .. y1 .. " and MOUSE_Y <= " .. y2 .. " then\n    if " .. variable .. " == " .. value .. " then\n      "
	MOUSE_CLICK_ACTION = MOUSE_CLICK_ACTION .. variable .. " = " .. alternative_value .. "\n    else\n      " .. variable .. " = " .. value .. "\n    end\n   RESUME_MOUSE_CLICK = true\n  end\n\n"
end

function draw_string(x, y, fontsize, text, r, g, b, a)
	-- must use default colors?
	r = r or 1
	g = g or 1
	b = b or 1
	a = a or 1

	-- check coordinates
	x = x + HUD_X or HUD_X + 2
	y = y + HUD_Y or HUD_Y + 2
	
	-- allow relative positions from right or upper screen border
	if HUD_X_MIN < 0 then
		x = string.format("SCREEN_WIDTH+(%i)", x)
	end
	if HUD_Y_MIN < 0 then
		y = string.format("SCREEN_HIGHT+(%i)", y)
	end

	-- set the color
	DRAW_STRINGS_PHASE = DRAW_STRINGS_PHASE .. "  graphics.set_color(" .. r .. "," .. g .. "," .. b .. "," .. a .. ")\n"
	
	-- draw the string depending on fontsize
	if fontsize == 10 then
		DRAW_STRINGS_PHASE = DRAW_STRINGS_PHASE .. "  draw_string_Helvetica_10(" .. x .. ", " .. y .. ', "' .. text .. '")\n'
	elseif fontsize == 12 then
		DRAW_STRINGS_PHASE = DRAW_STRINGS_PHASE .. "  draw_string_Helvetica_12(" .. x .. ", " .. y .. ', "' .. text .. '")\n'
	elseif fontsize == 18 then
		DRAW_STRINGS_PHASE = DRAW_STRINGS_PHASE .. "  draw_string_Helvetica_18(" .. x .. ", " .. y .. ', "' .. text .. '")\n'
	else
		DRAW_STRINGS_PHASE = DRAW_STRINGS_PHASE .. "  draw_string(" .. x .. ", " .. y .. ', "' .. text .. '")\n'
	end
end

function draw_fstring(x, y, fontsize, text, variables, r, g, b, a)
	-- check coordinates
	x = x + HUD_X or HUD_X + 2
	y = y + HUD_Y or HUD_Y + 2
	
	-- allow relative positions from right or upper screen border
	if HUD_X_MIN < 0 then
		x = string.format("SCREEN_WIDTH+(%i)", x)
	end
	if HUD_Y_MIN < 0 then
		y = string.format("SCREEN_HIGHT+(%i)", y)
	end

	-- change color if given
	if r and g and b and a then
		DRAW_STRINGS_PHASE = DRAW_STRINGS_PHASE .. "  graphics.set_color(" .. r .. "," .. g .. "," .. b .. "," .. a .. ")\n"
	end
	
	-- draw the string depending on fontsize
	if fontsize == 10 then
		DRAW_STRINGS_PHASE = DRAW_STRINGS_PHASE .. "  draw_string_Helvetica_10(" .. x .. ", " .. y .. ', string.format("' .. text .. '", ' .. variables .. '))\n'
	elseif fontsize == 12 then
		DRAW_STRINGS_PHASE = DRAW_STRINGS_PHASE .. "  draw_string_Helvetica_12(" .. x .. ", " .. y .. ', string.format("' .. text .. '", ' .. variables .. '))\n'
	elseif fontsize == 18 then
		DRAW_STRINGS_PHASE = DRAW_STRINGS_PHASE .. "  draw_string_Helvetica_18(" .. x .. ", " .. y .. ', string.format("' .. text .. '", ' .. variables .. '))\n'
	else
		DRAW_STRINGS_PHASE = DRAW_STRINGS_PHASE .. "  draw_string(" .. x .. ", " .. y .. ', string.format("' .. text .. '", ' .. variables .. '))\n'
	end
end

function begin_HUD(x, y, w, h, name, always)
	-- default values?
	HUD_X = x or 0
	HUD_Y = y or 0
	HUD_W = w or SCREEN_WIDTH
	HUD_H = h or SCREEN_HIGHT
	
	HUD_X_MIN = HUD_X
	HUD_X_MAX = HUD_X + HUD_W
	HUD_Y_MIN = HUD_Y
	HUD_Y_MAX = HUD_Y + HUD_H
	
	DRAW_BACKGROUND_PHASE = ""
	DRAW_LINES_PHASE = ""
	MOUSE_WHEEL_ACTION = ""
	MOUSE_CLICK_ACTION = ""
	DRAW_STRINGS_PHASE = ""

	HUD_ELEMENT_NAME = "unknown"
	
	HUD_COUNTER = HUD_COUNTER + 1
	HUD_NAME = name or "HUD" .. HUD_COUNTER
	
	always = always or "no"
	if always == "always" then
		HUD_MODE_ONLY = false
	else
		HUD_MODE_ONLY = true
	end
end

function end_HUD()
    local textfile
    textfile = io.open(SCRIPT_DIRECTORY .. "HUD_module_" .. HUD_NAME .. "_autogen.txt", "w")
	textfile:write("-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --\n")
	textfile:write("-- --   FlyWithLua HUD module automatic generated HUD file, do not edit!   -- --\n")
	textfile:write("-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --\n\n")
	
	textfile:write('require "graphics"\n\n')

	-- datarefs we need
	if HUD_MODE_ONLY then
		textfile:write('dataref( "view_type", "sim/graphics/view/view_type" )\n\n')
	end
	
	-- some relative positioning voodoo
	local REL_X = ""
	local REL_Y = ""
	if HUD_X_MIN < 0 then REL_X = "SCREEN_WIDTH+" end
	if HUD_Y_MIN < 0 then REL_Y = "SCREEN_HIGHT+" end
	
	-- the drawings
	textfile:write(string.format('function draw_%s()\n', HUD_NAME))
	if HUD_MODE_ONLY then
		textfile:write('  -- are we in HUD view?\n  if view_type ~= 1023 then return end\n\n')
	else
		textfile:write(string.format('  -- are we inside the HUD?\n  if MOUSE_X < %s(%i) or MOUSE_X > %s(%i) or MOUSE_Y < %s(%i) or MOUSE_Y > %s(%i) then return end\n\n', REL_X, HUD_X_MIN, REL_X, HUD_X_MAX, REL_Y, HUD_Y_MIN, REL_Y, HUD_Y_MAX))
		textfile:write(string.format('  -- we draw some background to the HUD\n  graphics.set_color(0, 0, 0, 0.5)\n  graphics.draw_rectangle(%s(%i), %s(%i), %s(%i), %s(%i))\n\n', REL_X, HUD_X_MIN, REL_Y, HUD_Y_MIN, REL_X, HUD_X_MAX, REL_Y, HUD_Y_MAX))
	end
	textfile:write('  -- init the graphics system\n  XPLMSetGraphicsState(0,0,0,1,1,0,0)\n\n')
	textfile:write(DRAW_BACKGROUND_PHASE)
	textfile:write('\n')
	textfile:write('  -- set color and width\n  graphics.set_color(1, 1, 1, 1)\n  graphics.set_width(2)\n\n')
	textfile:write(DRAW_LINES_PHASE)
	if DRAW_STRINGS_PHASE ~= "" then textfile:write('  -- draw some text\n') end
	textfile:write(DRAW_STRINGS_PHASE)
	textfile:write('end\n\n')
	textfile:write(string.format('do_every_draw("draw_%s()")\n\n', HUD_NAME))
	
	-- the click actions
	textfile:write(string.format('function %s_mouse_click_events()\n', HUD_NAME))
	textfile:write('  -- we will only react once\n  if MOUSE_STATUS ~= "down" then\n    return\n  end\n\n')
	textfile:write(MOUSE_CLICK_ACTION)
	textfile:write('end\n\n')
	textfile:write(string.format('do_on_mouse_click("%s_mouse_click_events()")\n\n', HUD_NAME))
	
	-- the wheel action
	textfile:write(string.format('function %s_mouse_wheel_events()\n', HUD_NAME))
	textfile:write(MOUSE_WHEEL_ACTION)
	textfile:write('end\n\n')
	textfile:write(string.format('do_on_mouse_wheel("%s_mouse_wheel_events()")\n\n', HUD_NAME))
	
	textfile:close()
	dofile(SCRIPT_DIRECTORY .. "HUD_module_" .. HUD_NAME .. "_autogen.txt")
end