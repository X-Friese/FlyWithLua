-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
--
--	New XPlane 11 XPLMDisplay Methods
--
--	Modified: Nov 18 2018 - IanQ
--
-- Lua/C++ Testing
--
-- NEW:
--
-- XPLMGetMouseLocationGlobal - void XPLMGetMouseLocationGlobal(int * outX, int * outY)
-- XPLMGetScreenSize - void XPLMGetScreenSize(int * outWidth, int * outHeight)
-- XPLMGetScreenBoundsGlobal - void XPLMGetScreenBoundsGlobal(int * inLeft, int * inTop, int * inRight, int * inBottom)
-- XPLMGetAllMonitorBoundsOS - void XPLMGetAllMonitorBoundsOS(XPLMReceiveMonitorBoundsOS_f inMonitorBoundsCallback, void * inRefcon)
-- XPLMGetAllMonitorBoundsGlobal - void XPLMGetAllMonitorBoundsGlobal(XPLMReceiveMonitorBoundsGlobal_f inMonitorBoundsCallback, void * inRefcon)
--
-- float_wnd_is_popped - int XPLMWindowIsPoppedOut(XPLMWindowID inWindowID)
-- float_wnd_get_visible - int XPLMGetWindowIsVisible(XPLMWindowID inWindowID)
-- float_wnd_set_visible - void XPLMSetWindowIsVisible(XPLMWindowID inWindowID, int inIsVisible)
-- float_wnd_is_front - int XPLMIsWindowInFront(XPLMWindowID inWindowID)
-- float_wnd_bring_to_front - void XPLMBringWindowToFront(XPLMWindowID inWindowID)
-- float_wnd_set_resizing_limits - void XPLMSetWindowResizingLimits(XPLMWindowID inWindowID, int inMinWidthBoxels, int inMinHeightBoxels, int inMaxWidthBoxels, int inMaxHeightBoxels)
-- float_wnd_set_positioning_mode - void XPLMSetWindowPositioningMode(XPLMWindowID inWindowID, XPLMWindowPositioningMode inPositioningMode, int inMonitorIndex)
-- float_wnd_set_gravity - void XPLMSetWindowGravity(XPLMWindowID inWindowID, float inLeftGravity, float inTopGravity, float inRightGravity, float inBottomGravity)
-- float_wnd_set_geometry
--		- for floating window, equivalent to - void XPLMSetWindowGeometry(XPLMWindowID inWindowID, int inLeft, int inTop, int inRight, int inBottom))
--		- for OS window, equivalent to       - void XPLMSetWindowGeometryOS(XPLMWindowID inWindowID, int inLeft, int inTop, int inRight, int inBottom)
-- float_wnd_get_geometry
--		- for floating window, equivalent to - void XPLMGetWindowGeometry(XPLMWindowID inWindowID, int * outLeft, int * outTop, int * outRight, int * outBottom)
--		- for OS window, equivalent to       - void XPLMGetWindowGeometryOS(XPLMWindowID inWindowID, int * outLeft, int * outTop, int * outRight, int * outBottom)
--
-- NOT TESTED AS HAVE NO VR HARDWARE----------------------------------------------------------------
-- float_wnd_is_vr - int XPLMWindowIsInVR(XPLMWindowID inWindowID)
-- float_wnd_set_geometry, equivalent to - void XPLMSetWindowGeometryVR(XPLMWindowID inWindowID, int widthBoxels, int heightBoxels)
-- float_wnd_get_geometry, equivalent to - void XPLMGetWindowGeometryVR(XPLMWindowID inWindowID, int * outWidthBoxels, int * outHeightBoxels)
--
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- NOTES:
--
-- float_wnd_set_title: Only available for floating window
-- float_wnd_set_resizing_limits: Needs to be set in floating window, then will work for OS window, but setGeometryOS will change it
-- float_wnd_set_gravity: This one works, but get weird results depending on parms used
-- float_wnd_bring_to_front: Works with floating, not OS, the workaround is to change from OS to floating to OS
-- float_wnd_get_visible: Current FWL uses isVisible to close/destroy window, have a fix available to resolve this
--		- For OS window, can hide, can't get back unless first change to FW window
-- float_wnd_set_positioning_mode: If use to change window from floating to OS, be warned that the window position may change
--		- If OS window calls positioning_mode, window gets set to floating window
-- XPLMGetAllMonitorBoundsOS - returns lua multi-dimensional table with data from XPLMReceiveMonitorBoundsOS_f
-- XPLMGetAllMonitorBoundsGlobal - returns lua multi-dimensional table with data from XPLMReceiveMonitorBoundsGlobal_f
--
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

require("graphics")

local is_popped = false		-- flag for window popped
local is_visible = true		-- flag for window visible
local is_front = false		-- flag for window in front
local is_vr = false			-- flag for window in VR

local winx, winy = 0,0												-- window x,y coordinates
local mousex, mousey = 0,0											-- mouse click x,y coordinates

local minWidth, minHeight, maxWidth, maxHeight = 800,600,800,600	-- float_wnd_set_resizing_limits
local gLeft, gTop, gRight, gBottom = 0.0,1.0,1.0,1.0				-- float_wnd_set_gravity
local winLeft, winTop, winRight, winBottom = 0,0,0,0				-- float_wnd_get_geometry (fw & os)
local vrWidth, vrHeight = 0,0										-- float_wnd_get_geometry (vr)
local dWidth, dHeight = 0,0											-- float_wnd_get_dimensions
local msgx, msgy = 0,0												-- XPLMGetMouseLocationGlobal
local ssWidth, ssHeight = 0,0										-- XPLMGetScreenSize
local bLeft, bTop, bRight, bBottom = 0,0,0,0						-- XPLMGetScreenBoundsGlobal
local tLeft, tTop, tRight, tBottom = 0,0,0,0						-- temp coordinates for pop/FW type toggle

local numPopped = 0					-- numeric representation of boolean is_popped
local chg_title = false				-- toggle for title change
local toggle_second_window = "off"	-- toggle for creating 2nd window
local pTitle = ""					-- title for 2nd window

local toggle_timer = 0				-- for timer function
local call_timer = 0
local generic_timer = 0
local generic_counter = 0

local text_vr = {"not ", ""}		-- for draw.string
local text_os = {"", "OS"}
local text_wt = {"Floating", "OS"}

local tOS = {}						-- table for XPLMGetAllMonitorBoundsOS
local tGB = {}						-- table for XPLMGetAllMonitorBoundsGlobal

-- convert bool to number
local function bool_to_number(value)
 return value == true and 1 or value == false and 0
end

-- used for 2nd window of inFront/isVisible methods
function des_ondraw(des_wnd, x, y)
	if (call_timer ~= 0 and toggle_timer ~= 0) then 
		local dWidth, dHeight = float_wnd_get_dimensions(des_wnd)
		draw_string(x + dWidth - 100, y + 5, "Time Count = " ..  os.time() - generic_counter, "white")
	end
end

-- x and y are the origin of the window, i.e. the lower left
-- x increases to the right, y increases to the top
function on_draw(fwl_wnd, x, y)
		
	-- used in logMsg to display on_draw window coords
	winx, winy = x, y
	
	is_popped = float_wnd_is_popped(fwl_wnd)
	is_visible = float_wnd_get_visible(fwl_wnd)
	is_front = float_wnd_is_front(fwl_wnd)
	is_vr = float_wnd_is_vr(fwl_wnd)
	--is_vr = true	-- if this is set can pseudo-emulate VR, Get/SetGeometry methods don't work though!

	-- numerical rep of boolean, used for math calc
	numPopped = bool_to_number(is_popped)
	
	-- get geometry
	if (is_vr) then
		-- uses boxels of a window in VR
		vrWidth, vrHeight = float_wnd_get_geometry(fwl_wnd)
		winTop, winBottom = minHeight, 0
		draw_string(x + 2, y + 250, "VR Window Size = " .. vrWidth .. " x " .. vrHeight, "white")
	else
		-- if floating window, uses global desktop boxel coordinates
		-- if popped window, uses operating system pixel coordinates
		winLeft, winTop, winRight, winBottom = float_wnd_get_geometry(fwl_wnd)
	end

	-- pre-existing float_wnd command, calculates window size
	dWidth, dHeight = float_wnd_get_dimensions(fwl_wnd)

	-- returns the current mouse location in global desktop boxels
	msgx, msgy = XPLMGetMouseLocationGlobal()

	-- returns the size of the main X-Plane OpenGL window in pixels
	ssWidth, ssHeight = XPLMGetScreenSize()
	
	-- returns the bounds of the "global" X-Plane desktop, in boxels
	bLeft, bTop, bRight, bBottom = XPLMGetScreenBoundsGlobal()
	
	-- OS: calls you back with the bounds (in pixels) of each monitor within the operating system's global desktop space
	-- Global: calls you back with the bounds (in boxels) of each full-screen X-Plane window within the X-Plane global desktop space
	tOS = XPLMGetAllMonitorBoundsOS()
	tGB = XPLMGetAllMonitorBoundsGlobal()

	draw_string(x + 2, y + 200, "All Bounds Monitor OS Element Count:   " .. #tOS)
	draw_string(x + 2, y + 185, "All Bounds Monitor Global Element Count:   " .. #tGB)

	for i = 1,#tOS,1 do
		draw_string(x + 2, y + 170 - (i * 15), "All Bounds Monitor " .. tOS[i].MonIndex .. " :      OS: " .. tOS[i].inLeft .. " , " ..
			 tOS[i].inTop  .. " , " .. tOS[i].inRight .. " , " .. tOS[i].inBottom)
	end

	for i = 1,#tGB,1 do
		draw_string(x + 350, y + 170 - (i * 15), "Global: " .. tGB[i].inLeft .. " , " ..
			 tGB[i].inTop  .. " , " .. tGB[i].inRight .. " , " .. tGB[i].inBottom)
	end

	-- timer loops for inFront/isVisible functions
	if (toggle_timer == 1 and os.time() > generic_timer) then
		if (call_timer == 1) then
			logMsg("isFront timer done")
			toggle_second_window = "front"
		elseif (call_timer == 2) then
			logMsg("isVisible timer hide start")
			toggle_second_window = "hide"
		end
		toggle_timer = 2
		generic_timer = os.time() + 10.0
	end

	if (toggle_timer == 2 and os.time() > generic_timer) then
		if (call_timer == 1) then
			logMsg("isFront close test window")
			toggle_timer = 0
			call_timer = 0
			toggle_second_window = "close"
		elseif (call_timer == 2) then
			logMsg("isVisible timer show start")
			toggle_second_window = "show"
			toggle_timer = 3
			generic_timer = os.time() + 10.0
		end
	end

	if (toggle_timer == 3 and os.time() > generic_timer) then
		if (call_timer == 2) then
			logMsg("isVisible close test window")
			toggle_timer = 0
			call_timer = 0
			toggle_second_window = "close"
		end
	end

	if (toggle_timer ~= 0) then 
		draw_string(x + dWidth - 130, y + 5, "Time Remaining = " .. generic_timer - os.time(), "white")
	end

	-- floating windows use Boxels, OS windows use pixels
	-- for OS window, size is 20 units bigger width & height
	draw_string(x + 20, y + (winTop - winBottom - (20 * numPopped)) - 20, "Window is " .. text_wt[numPopped + 1] .. " Window", "white")
	if (not is_popped) then draw_string(x + 20, y + (winTop - winBottom - (20 * numPopped)) - 60, "Click HERE to change window title", "cyan") end
	draw_string(x + 260, y + (winTop - winBottom - (20 * numPopped)) - 20, "Window is " .. text_vr[bool_to_number(is_vr) + 1] .. "VR", "white")
	draw_string(x + 20, y + (winTop - winBottom - (20 * numPopped)) - 85, "Click HERE to toggle window type: Floating/Popped", "yellow")
	draw_string(x + 20, y + (winTop - winBottom - (20 * numPopped)) - 110, "Click HERE to toggle VR mode", "magenta")
	draw_string(x + 20, y + (winTop - winBottom - (20 * numPopped)) - 135, "Click HERE to change window positioning mode", "gray")
	draw_string(x + 20, y + (winTop - winBottom - (20 * numPopped)) - 160, "Click HERE to set window gravity to " .. gLeft .. "," .. gTop .. "," .. gRight .. "," .. gBottom, "green")
	draw_string(x + 20, y + (winTop - winBottom - (20 * numPopped)) - 185, "Click HERE to set window resizing limits", "red")
	draw_string(x + 20, y + (winTop - winBottom - (20 * numPopped)) - 210, "Click HERE to increase window geometry; click HERE to decrease; click HERE for normal", "cyan")
	draw_string(x + 20, y + (winTop - winBottom - (20 * numPopped)) - 235, "Click HERE to bring window to front", "yellow")
	draw_string(x + 20, y + (winTop - winBottom - (20 * numPopped)) - 260, "Click HERE to change window visibility - 2nd window will hide/unhide/close", "magenta")
	
	draw_string(x + 2, y + 110, "Mouse Click x,y = " .. mousex .. " , " .. mousey, "white")
	draw_string(x + 2, y + 95, "Global Mouse Location x,y = " .. msgx .. " , " .. msgy, "white")
	draw_string(x + 2, y + 80, "float_wnd_get_dimensions = " .. dWidth .. " x " .. dHeight, "white")
	draw_string(x + 2, y + 65, "Window Geometry" .. text_os[numPopped + 1] .. " LTRB = " .. winLeft .. " , " .. winTop .. " , " .. winRight .. " , " .. winBottom, "white")
	draw_string(x + 2, y + 50, "Window Size = " .. winRight - winLeft .. " x " .. winTop - winBottom, "white")
	draw_string(x + 2, y + 35, "Screen Size = " .. ssWidth .. " x " .. ssHeight, "white")
	draw_string(x + 2, y + 20, "Screen Bounds = " .. bLeft .. " , " .. bTop .. " , " .. bRight .. " , " .. bBottom, "white")
	draw_string(x + 2, y + 5, "float_wnd_set_ondraw x,y = " .. winx .. " , " .. winy, "white")

end	-- function on_draw

-- x and y are relative from the origin of the window, i.e. the lower left
-- state: 1 = mouse down, 2 = mouse drag, 3 = mouse up
function on_click(fwl_wnd, x, y, state)

	mousex, mousey = x,y
	
	-- we only care about button down mouse events
	if (state == 1) then 
		
		logMsg("winx , winy = " .. winx .. " , " .. winy)
		logMsg("LTRB = " .. winLeft .. " , " .. winTop .. " , " .. winRight .. " , " .. winBottom)
		logMsg("mouse x , y = " .. x .. " , " .. y)
		
		-- can only set title of floating window, not OS window
		if (not is_popped and x >= 45 and x <= 240 and y >= winTop - winBottom - 60 - (20 * numPopped) and y <= winTop - winBottom - 40 - (20 * numPopped)) then
			logMsg("in setTitle statement")
			if (not chg_title) then
				float_wnd_set_title(fwl_wnd, "Title Changed")
				chg_title = true
			else
				float_wnd_set_title(fwl_wnd, "Title Changed Again")
				chg_title = false
			end
		end

		-- set window type
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 85 - (20 * numPopped) and y <= winTop - winBottom - 65 - (20 * numPopped)) then
			logMsg("in window type statement")
			if (not is_popped) then
				tLeft, tTop, tRight, tBottom = float_wnd_get_geometry(fwl_wnd)
				float_wnd_set_positioning_mode(fwl_wnd, 4, -1)
			else
				float_wnd_set_positioning_mode(fwl_wnd, 0, -1)
				float_wnd_set_geometry(fwl_wnd, tLeft, tTop, tLeft + minWidth, tTop - minHeight) 
			end
		end

		-- set VR mode
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 110 - (20 * numPopped) and y <= winTop - winBottom - 90 - (20 * numPopped)) then
			logMsg("in vr mode statement")
			if (not is_vr) then
				tLeft, tTop, tRight, tBottom = float_wnd_get_geometry(fwl_wnd)
				float_wnd_set_positioning_mode(fwl_wnd, 5, -1)
			else
				float_wnd_set_positioning_mode(fwl_wnd, 0, -1)
				float_wnd_set_geometry(fwl_wnd, tLeft, tTop, tLeft + minWidth, tTop - minHeight) 
			end
		end
	
		-- sets the policy for how X-Plane will position your window
	    -- xplm_WindowPositionFree				0
	    -- xplm_WindowCenterOnMonitor			1
	    -- xplm_WindowFullScreenOnMonitor		2
	    -- xplm_WindowFullScreenOnAllMonitors	3
		-- xplm_WindowPopOut					4
	    -- xplm_WindowVR						5
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 135 - (20 * numPopped) and y <= winTop - winBottom - 115 - (20 * numPopped)) then
			logMsg("in positioning statement")
			float_wnd_set_positioning_mode(fwl_wnd, 1, -1)
		end

		-- window gravity
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 160 - (20 * numPopped) and y <= winTop - winBottom - 140 - (20 * numPopped)) then
			logMsg("in gravity statement")
			float_wnd_set_gravity(fwl_wnd, gLeft, gTop, gRight, gBottom)
		end
			
		-- set window resizing limits
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 185 - (20 * numPopped) and y <= winTop - winBottom - 165 - (20 * numPopped)) then
			logMsg("in resizing statement")
			float_wnd_set_resizing_limits(fwl_wnd, minWidth, minHeight, maxWidth, maxHeight)
		end

		-- set window geometry
		if (x >= 45 and x <= 530 and y >= winTop - winBottom - 210 - (20 * numPopped) and y <= winTop - winBottom - 190 - (20 * numPopped)) then
			logMsg("in Geometry statement")
			
			-- set size depending on where user clicks
			if (x >= 45 and x <= 150) then
				hsize, vsize, vrsize = 1024, 768, 200
			elseif (x >= 285 and x <= 385) then
				hsize, vsize, vrsize = 640, 480, -200
			elseif (x >= 420 and x <= 520) then
				hsize, vsize, vrsize = 800, 600, 0
			end
			
			if (is_vr) then
				-- uses boxels of a window in VR
				logMsg("VR coords: " .. vrWidth + vrsize .. " , " .. vrHeight + vrsize)
				-- float_wnd_set_geometry(fwl_wnd, vrWidth + vrsize, vrHeight + vrsize)
				float_wnd_set_geometry(fwl_wnd, vsize, hsize)
			elseif (is_popped) then
				-- uses operating system pixel coordinates
				logMsg("OS coords: " .. winLeft .. " , " .. winTop .. " , " .. winLeft + hsize + (20 * numPopped) .. " , " .. winTop - vsize - (20 * numPopped))
				float_wnd_set_geometry(fwl_wnd, winLeft, winTop, winLeft + hsize + (20 * numPopped), winTop - vsize - (20 * numPopped))
			else
				-- uses global desktop boxel coordinates
				logMsg("NORM coords: " .. winLeft .. " , " .. winTop .. " , " .. winLeft + hsize .. " , " .. winTop - vsize)
				float_wnd_set_geometry(fwl_wnd, winLeft, winTop, winLeft + hsize, winTop - vsize) 
			end
		end
			
		-- window is front
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 235 - (20 * numPopped) and y <= winTop - winBottom - 215 - (20 * numPopped)) then
			logMsg("in isFront statement")
			toggle_second_window = "new"
			pTitle = "FlyWithLua isFront Window 2"
			call_timer = 1
			toggle_timer = 1
			generic_timer = os.time() + 10.0
			generic_counter = os.time()		
			logMsg("isFront timer start")
		end

		-- isVisible - setting to false ends the script
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 260 - (20 * numPopped) and y <= winTop - winBottom - 240 - (20 * numPopped)) then
			logMsg("in isVisible statement")
			toggle_second_window = "new"
			pTitle = "FlyWithLua isVisible Window 2"
			call_timer = 2
			toggle_timer = 1
			generic_timer = os.time() + 10.0
			generic_counter = os.time()		
			logMsg("isVisible timer show start")
		end		
	end	-- IF state
end	-- function on_click

-- when on_close is called, it is illegal to do anything with the fwl_wnd variable outside of this function
-- it is also not allowed to create new windows in on_close!
function on_close(fwl_wnd)
	logMsg("fwl_wnd on_close")
end

function des_onclick(des_wnd)
end

function des_onclose(des_wnd)
	logMsg("des_wnd on_close")
end

function second_window()
	if (toggle_second_window == "new") then
		logMsg("in second_window new")
		float_wnd_set_geometry(fwl_wnd, winLeft + 100, winTop - 100, winLeft + minWidth + 100, winTop - minHeight - 100) 
		des_wnd = float_wnd_create(minWidth, minHeight, 1, true)
		float_wnd_set_title(des_wnd, pTitle)
		float_wnd_set_ondraw(des_wnd, "des_ondraw")
		float_wnd_set_onclick(des_wnd, "des_onclick")
		float_wnd_set_onclose(des_wnd, "des_onclose")
		if (is_popped) then 
			float_wnd_set_positioning_mode(des_wnd, 4, -1) 
		end
	end

	if (toggle_second_window == "hide") then
		logMsg("in second_window hide")
		float_wnd_set_positioning_mode(des_wnd, 0, -1) 
		float_wnd_set_visible(des_wnd, 0)
	end
	
	if (toggle_second_window == "show") then
		logMsg("in second_window show")
		float_wnd_set_visible(des_wnd, 1)
		if (is_popped) then 
			float_wnd_set_positioning_mode(des_wnd, 4, -1) 
			float_wnd_set_geometry(des_wnd, winLeft - 100, winTop + 100, winLeft + 820 - 100, winTop - 620 + 100) 
		end
	end

	if (toggle_second_window == "front") then
		logMsg("in second_window front")
		if (is_popped) then
			-- isFront doesn't appear to work for a popped window, thus this workaround
			-- I tried to just do a set_positioning_mode(fwl_wnd, 4, -1) but that created an orphan window
			float_wnd_set_positioning_mode(fwl_wnd, 0, -1) 
			float_wnd_set_positioning_mode(fwl_wnd, 4, -1) 
			float_wnd_set_geometry(fwl_wnd, winLeft, winTop, winLeft + 820, winTop - 620) 
		else
			float_wnd_bring_to_front(fwl_wnd)
		end
	end
	
	if (toggle_second_window == "close") then
		logMsg("in second_window close")
		des_onclose(des_wnd) 
		float_wnd_destroy(des_wnd)
	end
		
	toggle_second_window = "off"
end -- function second_window

-- width, height, decoration style as per XPLMCreateWindowEx. 1 for solid background, 3 for transparent
fwl_wnd = float_wnd_create(minWidth, minHeight, 1, true)
float_wnd_set_title(fwl_wnd, "FlyWithLua Test")
float_wnd_set_ondraw(fwl_wnd, "on_draw")
float_wnd_set_onclick(fwl_wnd, "on_click")
float_wnd_set_onclose(fwl_wnd, "on_close")

do_every_frame("second_window()")

-- To destroy a window:
-- float_wnd_destroy(fwl_wnd) 
-- after that, do not use the wnd variable anymore
