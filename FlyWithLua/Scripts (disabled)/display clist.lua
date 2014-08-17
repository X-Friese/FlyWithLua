-- Yet Another Checklister Lister
-- V1.2.1 by Carsten Lynker
-- build 2013-11-15

-- Warning:
-- This is a very advanced script, using Lua's string functions
-- like pattern search and matching.
-- If you don't understand it, just use it.

require("graphics")


-------8<------------------------------------------------------------------------------------
-- You can edit these 5 parameters to customize the output of this script.
local rowhight = 21                -- the line spacing in screen pixel
local framewidth = 5               -- the space between text and frame in screen pixel
local y_offset = 25                -- the distance of the window from the upper screen limit
local show_on_right_side = false   -- set this to true to display the pages on the right side
local transparent_percent = 0.65   -- the darkness of the windows background
---------------------------------------------------------------------->8---------------------


local max_title_width = 0
local checklist = {}
local lastpage = 0
local lastrow = 0
local page_to_show = 0

-- measuring the pages
function measure_checklist()
	if lastpage > 0 then
		for i = 1, lastpage do
			checklist[i].width = framewidth * 2 + measure_string(checklist[i].title, "Helvetica_18")
			if checklist[i].width > max_title_width then
				max_title_width = checklist[i].width
			end
			checklist[i].hight = framewidth * 2 + checklist[i].rows * rowhight + rowhight
			for j = 1, checklist[i].rows do
				local newwidth = 0
				if string.find(checklist[i][j], '|') ~= nil then
					checklist[i].tab = math.max(checklist[i].tab, measure_string(string.match(checklist[i][j]:gsub("%%%%(.-)%%%%",  function (s)
											return loadstring(s)()
											end), "(.*)|"), "Helvetica_12")+20)
					newwidth = measure_string(string.match(checklist[i][j]:gsub("%%%%(.-)%%%%",  function (s)
											return loadstring(s)()
											end), '.*|(.*)'), "Helvetica_12") + checklist[i].tab + 2*framewidth
				else
					newwidth = measure_string(checklist[i][j]:gsub("%%%%(.-)%%%%",  function (s)
											return loadstring(s)()
											end), "Helvetica_12") + 2*framewidth
				end
				if newwidth > checklist[i].width then
					checklist[i].width = newwidth
				end
			end
		end
	end
end

-- parsing a clist.txt file (checklister format)
function load_clist( filename )
	-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	-- a modification for Mac OSX by fatroom http://forums.x-plane.org/index.php?showuser=281414
	-- see http://forums.x-plane.org/index.php?app=downloads&showfile=17468&st=30#commentsStart
	-- and http://forums.x-plane.org/index.php?showtopic=79097&p=842262
	if SYSTEM == "APL" then
		filename = "/Volumes/" .. string.gsub(filename, ":", "/")
	end
	-- end of modification
	-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	local file_to_parse = io.open( filename )
	if file_to_parse ~= nil then
		for lines in file_to_parse:lines() do
			if string.find(lines, "sw_checklist:", 1, true) ~= nil then
				lastpage = lastpage + 1
				lastrow = 0
				checklist[lastpage] = {}
				checklist[lastpage].rows = 0
				checklist[lastpage].tab = 0
				checklist[lastpage].title = string.match(lines, '([^:]*)', 14)
			end
			if string.find(lines, "sw_item:", 1, true) ~= nil then
				lastrow = lastrow + 1
				checklist[lastpage].rows = lastrow
				checklist[lastpage][lastrow] = string.match(lines, '([^:]*)', 9)
			end
			if string.find(lines, "sw_iteminfo:", 1, true) ~= nil then
				lastrow = lastrow + 1
				checklist[lastpage].rows = lastrow
				checklist[lastpage][lastrow] = string.match(lines, '([^:]*)', 13)
			end
			if string.find(lines, "sw_itemvoid:", 1, true) ~= nil then
				lastrow = lastrow + 1
				checklist[lastpage].rows = lastrow
				checklist[lastpage][lastrow] = string.sub(lines, 13)
			end
		end
		file_to_parse:close()
		print(string.format('FlyWithLua Info: Checklister file "%s" loaded.', filename))
		logMsg(string.format('FlyWithLua Info: Checklister file "%s" loaded.', filename))
	else
		logMsg(string.format([[FlyWithLua Info: Sorry, no checklister file found. Can't find "%s".]], filename))
	end
	measure_checklist()
end

-- provide a function to load additional checklists (easy format)
function load_checklist( filename )
	-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	-- a modification for Mac OSX by fatroom http://forums.x-plane.org/index.php?showuser=281414
	-- see http://forums.x-plane.org/index.php?app=downloads&showfile=17468&st=30#commentsStart
	-- and http://forums.x-plane.org/index.php?showtopic=79097&p=842262
	if SYSTEM == "APL" then
		filename = "/Volumes/" .. string.gsub(filename, ":", "/")
	end
	-- end of modification
	-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
	local file_to_parse = io.open( filename )
	if file_to_parse ~= nil then
		-- the first title comes in the first line
		lastpage = lastpage + 1
		lastrow = 0
		checklist[lastpage] = {}
		checklist[lastpage].rows = 0
		checklist[lastpage].tab = 0
		for lines in file_to_parse:lines() do
			-- do we have a line of Lua code to execute?
			if (lines:sub(1,1) == '>') then
				local string_to_execute = string.match(lines, ">(.*)")
				assert(loadstring(string_to_execute))()
				lines = "--"
			end
			-- replace tabstops by doublespaces
			lines = lines:gsub("\t", "  ")
			-- replace more than one space with '|'
			lines = lines:gsub("  [ ]*", "|", 1)
			-- insert "return " if needed
			if lines:find("return") == nil then
				lines = lines:gsub("%$(.-)%$","%$return %1%$")
				lines = lines:gsub("%%%%(.-)%%%%","%%%%return %1%%%%")
			end
			-- parse all lines not containing comments
			if (string.find(lines, "--", 1, true) == nil) and (string.find(lines, "===", 1, true) == nil) then
				-- execute Lua code inside a line
				lines = lines:gsub("%$(.-)%$",  function (s)
												return loadstring(s)()
												end)
				if lines == "" and lastrow > 0 then
					lastpage = lastpage + 1
					lastrow = 0
					checklist[lastpage] = {}
					checklist[lastpage].rows = 0
					checklist[lastpage].tab = 0
				elseif lastrow == 0 and lines ~= "" then
					checklist[lastpage].title = lines
					lastrow = lastrow + 1
				elseif lines ~= "" then
					checklist[lastpage][lastrow] = lines
					checklist[lastpage].rows = lastrow
					lastrow = lastrow + 1
				end
			end
		end
		file_to_parse:close()
		-- does the file end with empty lines?
		if checklist[lastpage].rows == 0 then
			lastpage = lastpage - 1
		end
		print(string.format('FlyWithLua Info: Checklist "%s" loaded.', filename))
		logMsg(string.format('FlyWithLua Info: Checklist "%s" loaded.', filename))
	else
		logMsg(string.format([[FlyWithLua Info: Can't find checklist "%s".]], filename))
	end
	measure_checklist()
end

-- try to load a Checklister file or a FWL style file inside the aircraft directory
load_clist( AIRCRAFT_PATH .. "clist.txt" )
load_checklist( AIRCRAFT_PATH .. "FWL_checklist.txt")

-- look inside the Checklists directory and try to catch a tailnumber
load_clist( SCRIPT_DIRECTORY .. "Checklists" .. DIRECTORY_SEPARATOR .. "TAILNUMBER_" .. PLANE_TAILNUMBER .. "_clist.txt")
load_checklist( SCRIPT_DIRECTORY .. "Checklists" .. DIRECTORY_SEPARATOR .. "TAILNUMBER_" .. PLANE_TAILNUMBER .. "_checklist.txt")

-- look inside the Checklists directory and try to catch an ICAO code
if lastpage == 0 then
	load_clist( SCRIPT_DIRECTORY .. "Checklists" .. DIRECTORY_SEPARATOR .. "ICAO_" .. PLANE_ICAO .. "_clist.txt")
	load_checklist( SCRIPT_DIRECTORY .. "Checklists" .. DIRECTORY_SEPARATOR .. "ICAO_" .. PLANE_ICAO .. "_checklist.txt")
end

-- always load the global checklist
load_checklist( SCRIPT_DIRECTORY .. "Checklists" .. DIRECTORY_SEPARATOR .. "global_checklist.txt")


-- move the page to the right (local helper function)
local function x_offset()
	if show_on_right_side then
		return SCREEN_WIDTH - checklist[page_to_show].width
	else
		return 0
	end
end

local function x_title_offset()
	if show_on_right_side then
		return SCREEN_WIDTH - framewidth*2 - max_title_width
	else
		return 0
	end
end

-- showing the menu or the pages
function display_checklist()
	if (MOUSE_X < 2 and not show_on_right_side) or (MOUSE_X > SCREEN_WIDTH - 2 and show_on_right_side) then
		-- we have to show the menu
		XPLMSetGraphicsState(0,0,0,1,1,0,0)
		glColor4f(0,0,0,transparent_percent)
		glRectf(x_title_offset() + 0, SCREEN_HIGHT - y_offset, x_title_offset() + framewidth*2 + max_title_width, SCREEN_HIGHT - framewidth*2 - lastpage*rowhight - y_offset)
		local choose_page
		choose_page = math.floor((SCREEN_HIGHT - MOUSE_Y - y_offset) / rowhight + 0.5)
		if choose_page <= lastpage then
			page_to_show = choose_page
		else
			page_to_show = 0
		end
		for i = 1, lastpage do
			if page_to_show == i then
				glColor4f(1,1,1,1)
			else
				glColor4f(0.75,0.75,0.75,1)
			end
			draw_string_Helvetica_18(x_title_offset() + framewidth, SCREEN_HIGHT - framewidth - y_offset - i*rowhight, checklist[i].title)
		end
	elseif page_to_show > 0 then
		-- we have to show a page
		XPLMSetGraphicsState(0,0,0,1,1,0,0)
		glColor4f(0,0,0,transparent_percent)
		glRectf(x_offset() + 0, SCREEN_HIGHT - y_offset, x_offset() + checklist[page_to_show].width, SCREEN_HIGHT - y_offset - checklist[page_to_show].hight)
		glColor4f(1,1,1,1)
		draw_string_Helvetica_18(x_offset() + framewidth, SCREEN_HIGHT - framewidth - y_offset - rowhight, checklist[page_to_show].title)
		graphics.set_width( 1.0 )
		graphics.draw_line( x_offset() + framewidth, SCREEN_HIGHT - framewidth - y_offset - rowhight - 4, x_offset() + checklist[page_to_show].width - framewidth, SCREEN_HIGHT - framewidth - y_offset - rowhight - 4)
		for i = 1, checklist[page_to_show].rows do
			local string_to_show = checklist[page_to_show][i]
			-- execute Lua code inside a line
			string_to_show = string_to_show:gsub("%%%%(.-)%%%%",  function (s)
											return loadstring(s)()
											end)
			if string.find(string_to_show, '|') == nil then
				draw_string_Helvetica_12(x_offset() + framewidth, SCREEN_HIGHT - framewidth - y_offset - (i+1)*rowhight, string_to_show)
			else
				draw_string_Helvetica_12(x_offset() + framewidth, SCREEN_HIGHT - framewidth - y_offset - (i+1)*rowhight, string.match(string_to_show, '(.*)|'))
				draw_string_Helvetica_12(x_offset() + framewidth + checklist[page_to_show].tab, SCREEN_HIGHT - framewidth - y_offset - (i+1)*rowhight, string.match(string_to_show, '|(.*)'))
			end
		end
	end
end

-- create the drawing calls
if lastpage > 0 then
	do_every_draw("display_checklist()")
	do_often("measure_checklist()")
end
