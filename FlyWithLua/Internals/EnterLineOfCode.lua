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

-- EnterLineOfCode.lua version 1.2 build 2013-10-03

-- this is the code line
local line_of_code = ""
code_line_history = {}
code_line_history[1] = "-- Welcome to FlyWithLua running " .. _VERSION .. " (for more info see SYSTEM, SYSTEM_ARCHITECTURE, XPLANE_VERSION and XPLANE_LANGUAGE)."
code_line_history[2] = "-- Get mouse, keyboard and screen info from MOUSE_X, MOUSE_Y, SCREEN_WIDTH, SCREEN_HIGHT, VKEY and CKEY."
number_of_code_lines_in_history = 2
local position_in_history = 3
enter_the_code_line = false

function DrawFLyWithLuaInputLine()
	local bubble_y = 40
	
	if not enter_the_code_line then return end
	-- draw the background
	XPLMSetGraphicsState(0,0,0,1,1,0,0)
	glColor4f(1,0,0,0.75)
	glRectf(5, 10, SCREEN_WIDTH-5, 40)
	
	-- draw the line of code
	glColor4f(0,0,1,1)
	draw_string_Helvetica_18(10, 15, line_of_code)
	
	-- draw cursor
	glColor4f(1,1,1,1)
	glBegin_LINES()
	local x_pos = 10 + measure_string(line_of_code, "Helvetica_18")
	glVertex2f(x_pos, 37)
	glVertex2f(x_pos, 12)
	glEnd()
	
	-- can we draw some debug info?
	for debug_string in string.gmatch(line_of_code, "[%w_]+") do
		-- is it a variable?
		for n,v in pairs(_G) do
			if n == debug_string then
				if type(v) == "function" then
					bubble_y = bubble(20, bubble_y, "function", tostring(n) .. " = " .. tostring(v))
				elseif type(v) == "table" then
					bubble_y = bubble(20, bubble_y, "table", tostring(n) .. " = " .. tostring(v))
				else
					if type(v) == "number" then
                        bubble_y = bubble(20, bubble_y, "number", n .. " = " .. tostring(v))
                    elseif type(v) == "string" then
                        bubble_y = bubble(20, bubble_y, "string", n .. " = " .. string.format("%q",v))
                    elseif type(v) == "boolean" then
                        bubble_y = bubble(20, bubble_y, "boolean", n .. " = " .. tostring(v))
                    else
                        bubble_y = bubble(20, bubble_y, "variable", n .. " = " .. tostring(v))
                    end
					-- does it binds to a DataRef?
					local DREF_name
					local DREF_index
					local DREF_readonly
					local DREF_type
					DREF_name, DREF_index, DREF_readonly, DREF_type = get_DataRef_binding(n)
					if DREF_type == 1 then
						bubble_y = bubble(20, bubble_y, 	"DataRef binding",
															"Variable = " .. n,
															"DataRef = " .. DREF_name,
															"Readonly = " .. tostring(DREF_readonly),
															"Type = integer")
					elseif DREF_type == 2 then
						bubble_y = bubble(20, bubble_y, 	"DataRef binding",
															"Variable = " .. n,
															"DataRef = " .. DREF_name,
															"Readonly = " .. tostring(DREF_readonly),
															"Type = float")
					elseif DREF_type == 4 then
						bubble_y = bubble(20, bubble_y, 	"DataRef binding",
															"Variable = " .. n,
															"DataRef = " .. DREF_name,
															"Readonly = " .. tostring(DREF_readonly),
															"Type = double")
					elseif DREF_type == 8 then
						bubble_y = bubble(20, bubble_y, 	"DataRef binding",
															"Variable = " .. n,
															"DataRef = " .. DREF_name,
															"Index = " .. tostring(DREF_index),
															"Readonly = " .. tostring(DREF_readonly),
															"Type = float array")
					elseif DREF_type == 16 then
						bubble_y = bubble(20, bubble_y, 	"DataRef binding",
															"Variable = " .. n,
															"DataRef = " .. DREF_name,
															"Index = " .. tostring(DREF_index),
															"Readonly = " .. tostring(DREF_readonly),
															"Type = integer array")
					elseif DREF_type == 32 then
						bubble_y = bubble(20, bubble_y, 	"DataRef binding",
															"Variable = " .. n,
															"DataRef = " .. DREF_name,
															"Index = " .. tostring(DREF_index),
															"Readonly = " .. tostring(DREF_readonly),
															"Type = data (string)")
					end
				end
			end
		end
	end
end

do_every_draw("DrawFLyWithLuaInputLine()")

function GetLuaLineFromKeyboard()
	if enter_the_code_line and KEY_ACTION == "pressed" then
		RESUME_KEY = true
		-- escape
		if VKEY == 27 then
			line_of_code = ""
			enter_the_code_line = false
			position_in_history = number_of_code_lines_in_history + 1
			return
		end
		-- delete
		if VKEY == 8 and line_of_code ~= "" then
			line_of_code = string.sub(line_of_code, 1, string.len(line_of_code)-1)
			return
		end
		-- up
		if VKEY == 38 then
			position_in_history = position_in_history - 1
			if position_in_history < 1 then
				position_in_history = number_of_code_lines_in_history
			end
			line_of_code = code_line_history[position_in_history]
			return
		end
		-- down
		if VKEY == 40 then
			position_in_history = position_in_history + 1
			if position_in_history > number_of_code_lines_in_history then
				position_in_history = 1
			end
			line_of_code = code_line_history[position_in_history]
			return
		end
		-- enter
		if VKEY == 13 then
			if line_of_code ~= "" then
				assert(loadstring(line_of_code))()
				if number_of_code_lines_in_history < 100 then
					number_of_code_lines_in_history = number_of_code_lines_in_history + 1
					code_line_history[number_of_code_lines_in_history] = line_of_code
					position_in_history = number_of_code_lines_in_history + 1
				else
					for i = 1, number_of_code_lines_in_history - 1 do
						code_line_history[i] = code_line_history[i+1]
					end
					code_line_history[number_of_code_lines_in_history] = line_of_code
				end
			end
			line_of_code = ""
			enter_the_code_line = false
			return
		end
		-- anything else
		line_of_code = line_of_code .. CKEY
	end
end

do_on_keystroke("GetLuaLineFromKeyboard()")

add_macro("Enter a line of code", "enter_the_code_line = true")
create_command("FlyWithLua/debugging/enter_code", "enter some Lua code", "enter_the_code_line = true", "", "")
