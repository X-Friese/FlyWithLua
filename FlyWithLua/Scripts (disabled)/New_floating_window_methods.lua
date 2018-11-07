-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
--
--	New Floating Window Display Methods
--
--
-- NEW:
--
-- float_wnd_is_popped - int XPLMWindowIsPoppedOut(XPLMWindowID inWindowID)
-- float_wnd_get_visible - int XPLMGetWindowIsVisible(XPLMWindowID inWindowID)
-- float_wnd_set_visible - void XPLMSetWindowIsVisible(XPLMWindowID inWindowID, int inIsVisible)
-- float_wnd_is_front - int XPLMIsWindowInFront(XPLMWindowID inWindowID)
-- float_wnd_bring_to_front - void XPLMBringWindowToFront(XPLMWindowID inWindowID)
-- float_wnd_get_mouse_location_global - void XPLMGetMouseLocationGlobal(int * outX, int * outY)
-- float_wnd_set_resizing_limits - void XPLMSetWindowResizingLimits(XPLMWindowID inWindowID, int inMinWidthBoxels, int inMinHeightBoxels, int inMaxWidthBoxels, int inMaxHeightBoxels)
-- float_wnd_set_positioning_mode - void XPLMSetWindowPositioningMode(XPLMWindowID inWindowID, XPLMWindowPositioningMode inPositioningMode, int inMonitorIndex)
-- float_wnd_set_gravity - void XPLMSetWindowGravity(XPLMWindowID inWindowID, float inLeftGravity, float inTopGravity, float inRightGravity, float inBottomGravity)
-- float_wnd_get_screen_size - void XPLMGetScreenSize(int * outWidth, int * outHeight)
-- float_wnd_get_screen_bounds_global - void XPLMGetScreenBoundsGlobal(int * inLeft, int * inTop, int * inRight, int * inBottom)
-- float_wnd_set_geometry - void XPLMGetWindowGeometry(XPLMWindowID inWindowID, int * outLeft, int * outTop, int * outRight, int * outBottom)
-- float_wnd_get_geometry - void XPLMSetWindowGeometry(XPLMWindowID inWindowID, int inLeft, int inTop, int inRight, int inBottom)
-- float_wnd_set_geometry_os - void XPLMGetWindowGeometryOS(XPLMWindowID inWindowID, int * outLeft, int * outTop, int * outRight, int * outBottom)
-- float_wnd_get_geometry_os - void XPLMSetWindowGeometryOS(XPLMWindowID inWindowID, int inLeft, int inTop, int inRight, int inBottom)
-- float_wnd_set_monitor_index - New method created to set monitor index to specific monitor
-- float_wnd_get_monitor_bounds_os - void XPLMGetAllMonitorBoundsOS(XPLMReceiveMonitorBoundsOS_f inMonitorBoundsCallback, void * inRefcon)
-- float_wnd_get_monitor_bounds_global - void XPLMGetAllMonitorBoundsGlobal(XPLMReceiveMonitorBoundsGlobal_f inMonitorBoundsCallback, void * inRefcon)
-- float_wnd_is_vr - int XPLMWindowIsInVR(XPLMWindowID inWindowID)
-- NOT FULLY TESTED AS CAN'T DO VR ----------------------------------------------------------------
-- float_wnd_set_geometry_vr - void XPLMSetWindowGeometryVR(XPLMWindowID inWindowID, int widthBoxels, int heightBoxels)
-- float_wnd_get_geometry_vr - void XPLMGetWindowGeometryVR(XPLMWindowID inWindowID, int * outWidthBoxels, int * outHeightBoxels)
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
--
--
--

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

require("graphics")

local is_popped = 0		-- flag for window popped
local is_visible = 1	-- flag for window visible
local is_front = 0		-- flag for window in front
local is_vr = 0			-- flag for window in VR

local winx, winy = 0,0		-- window x,y coordinates
local mousex, mousey = 0,0	-- mouse click x,y coordinates

local winLeft, winTop, winRight, winBottom = 0,0,0,0				-- float_wnd_get_geometry/_os
local ssWidth, ssHeight = 0,0										-- float_wnd_get_screen_size
local gLeft, gTop, gRight, gBottom = 0.0,1.0,1.0,1.0				-- float_wnd_set_gravity
local dWidth, dHeight = 0,0											-- float_wnd_get_dimensions
local bLeft, bTop, bRight, bBottom = 0,0,0,0						-- float_wnd_get_screen_bounds_global
local msgx, msgy = 0,0												-- float_wnd_get_mouse_location_global(fwl_wnd)
local minWidth, minHeight, maxWidth, maxHeight = 800,600,800,600	-- float_wnd_set_resizing_limits
local omIndex, oLeft, oTop, oRight, oBottom, oRefcon = 0,0,0,0,0,0	-- float_wnd_get_monitor_bounds_os
local lmIndex, lLeft, lTop, lRight, lBottom, lRefcon = 0,0,0,0,0,0	-- float_wnd_get_monitor_bounds_os/global
local vrWidth, vrHeight = 0,0										-- float_wnd_get_geometry_vr
local tLeft, tTop, tRight, tBottom = 0,0,0,0						-- temp coordinates for pop/FW toggle

local monIndex = 0					-- monitor index
local chg_title = 0					-- toggle for title change
local toggle_second_window = "off"	-- toggle for creating 2nd window
local pTitle = ""					-- title for 2nd window

local toggle_timer = 0				-- for timer function
local call_timer = 0
local generic_timer = 0
local generic_counter = 0

local text_vr = {"not ", ""}		-- for draw.string
local text_os = {"", "OS"}
local text_wt = {"Floating", "OS"}


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
	--is_vr = 1	-- if this is set can pseudo-emulate VR, Get/SetGeometry methods work - a little!
	
	if (is_popped == 0) then 
		-- uses global desktop boxel coordinates
		winLeft, winTop, winRight, winBottom = float_wnd_get_geometry(fwl_wnd)
	else
		-- uses operating system pixel coordinates
		winLeft, winTop, winRight, winBottom = float_wnd_get_geometry_os(fwl_wnd)
	end

	if (is_vr == 1) then
		-- uses boxels of a window in VR
		vrWidth, vrHeight = float_wnd_get_geometry_vr(fwl_wnd)
		draw_string(x + 2, y + 140, "VR Window Size = " .. vrWidth .. " x " .. vrHeight, "white")
	end

	-- Pre-existing float_wnd command, calculates window size
	dWidth, dHeight = float_wnd_get_dimensions(fwl_wnd)

	-- returns the current mouse location in global desktop boxels
	msgx, msgy = float_wnd_get_mouse_location_global(fwl_wnd)

	-- returns the size of the main X-Plane OpenGL window in pixels
	ssWidth, ssHeight = float_wnd_get_screen_size(fwl_wnd)
	
	-- returns the bounds of the "global" X-Plane desktop, in boxels
	bLeft, bTop, bRight, bBottom = float_wnd_get_screen_bounds_global(fwl_wnd)
	
	-- OS: calls you back with the bounds (in pixels) of each monitor within the operating system's global desktop space
	-- Global: calls you back with the bounds (in boxels) of each full-screen X-Plane window within the X-Plane global desktop space
	for i = 0,5,1 do
		float_wnd_set_monitor_index(fwl_wnd, i)
		omIndex, oLeft, oTop, oRight, oBottom, oRefcon = float_wnd_get_monitor_bounds_os(fwl_wnd)
		lmIndex, lLeft, lTop, lRight, lBottom, lRefcon = float_wnd_get_monitor_bounds_global(fwl_wnd)
		if (oLeft + oTop + oRight + oBottom ~= 0) then
			draw_string(x + 2, y + 200 - (i * 15), "Monitor Bounds: Monitor " .. omIndex .. " : " .. "OS : " .. oLeft .. " , " .. oTop  .. " , " ..
			 oRight .. " , " .. oBottom .. " , " .. oRefcon .. " ; " .. "Global : " .. lLeft .. " , " .. lTop  .. " , " .. lRight .. " , " .. lBottom ..
			 " , " .. lRefcon,"white")
		end
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
	draw_string(x + 20, y + (winTop - winBottom - (20 * is_popped)) - 20, "Window is " .. text_wt[is_popped + 1] .. " Window", "white")
	if (is_popped == 0) then draw_string(x + 20, y + (winTop - winBottom - (20 * is_popped)) - 60, "Click HERE to change window title", "cyan") end
	draw_string(x + 260, y + (winTop - winBottom - (20 * is_popped)) - 20, "Window is " .. text_vr[is_vr + 1] .. "VR", "white")
	draw_string(x + 20, y + (winTop - winBottom - (20 * is_popped)) - 85, "Click HERE to toggle window type: Floating/Popped", "yellow")
	draw_string(x + 20, y + (winTop - winBottom - (20 * is_popped)) - 110, "Click HERE to toggle VR mode", "magenta")
	draw_string(x + 20, y + (winTop - winBottom - (20 * is_popped)) - 135, "Click HERE to change window positioning mode", "gray")
	draw_string(x + 20, y + (winTop - winBottom - (20 * is_popped)) - 160, "Click HERE to set window gravity to " .. gLeft .. "," .. gTop .. "," .. gRight .. "," .. gBottom, "green")
	draw_string(x + 20, y + (winTop - winBottom - (20 * is_popped)) - 185, "Click HERE to set window resizing limits", "red")
	draw_string(x + 20, y + (winTop - winBottom - (20 * is_popped)) - 210, "Click HERE to change window geometry", "cyan")
	draw_string(x + 20, y + (winTop - winBottom - (20 * is_popped)) - 235, "Click HERE to bring window to front", "yellow")
	draw_string(x + 20, y + (winTop - winBottom - (20 * is_popped)) - 260, "Click HERE to change window visibility - 2nd window will hide/unhide/close", "magenta")
	
	draw_string(x + 2, y + 110, "Mouse Click x,y = " .. mousex .. " , " .. mousey, "white")
	draw_string(x + 2, y + 95, "Global Mouse Location x,y = " .. msgx .. " , " .. msgy, "white")
	draw_string(x + 2, y + 80, "float_wnd_get_dimensions = " .. dWidth .. " x " .. dHeight, "white")
	draw_string(x + 2, y + 65, "Window Geometry" .. text_os[is_popped + 1] .. " LTRB = " .. winLeft .. " , " .. winTop .. " , " .. winRight .. " , " .. winBottom, "white")
	draw_string(x + 2, y + 50, "Window Size = " .. winRight - winLeft .. " x " .. winTop - winBottom, "white")
	draw_string(x + 2, y + 35, "Screen Size = " .. ssWidth .. " x " .. ssHeight, "white")
	draw_string(x + 2, y + 20, "Screen Bounds = " .. bLeft .. " , " .. bTop .. " , " .. bRight .. " , " .. bBottom, "white")
	draw_string(x + 2, y + 5, "float_wnd_set_ondraw x,y = " .. winx .. " , " .. winy, "white")

end	-- function on_draw

-- x and y are relative from the origin of the window, i.e. the lower left
-- state: 1 = mouse down, 2 = mouse drag, 3 = mouse up
function on_click(fwl_wnd, x, y, state)

	mousex, mousey = x,y
	
	-- I only care about button down mouse events
	if (state == 1) then 
		
		logMsg("winx , winy = " .. winx .. " , " .. winy)
		logMsg("LTRB = " .. winLeft .. " , " .. winTop .. " , " .. winRight .. " , " .. winBottom)
		logMsg("mouse x , y = " .. x .. " , " .. y)

		-- For OS window, size is 20 units bigger width & height
		
		-- can only set title of floating window, not OS window
		if (is_popped == 0 and x >= 45 and x <= 240 and y >= winTop - winBottom - 60 - (20 * is_popped) and y <= winTop - winBottom - 40 - (20 * is_popped)) then
			logMsg("in setTitle statement")
			if (chg_title == 0) then
				float_wnd_set_title(fwl_wnd, "Title Changed")
				chg_title = 1
			else
				float_wnd_set_title(fwl_wnd, "Title Changed Again")
				chg_title = 0
			end
		end

		-- set window type
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 85 - (20 * is_popped) and y <= winTop - winBottom - 65 - (20 * is_popped)) then
			logMsg("in window type statement")
			if (is_popped == 0) then
				tLeft, tTop, tRight, tBottom = float_wnd_get_geometry(fwl_wnd)
				float_wnd_set_positioning_mode(fwl_wnd, 4, -1)
			else
				float_wnd_set_positioning_mode(fwl_wnd, 0, -1)
				float_wnd_set_geometry(fwl_wnd, tLeft, tTop, tLeft + 800, tTop - 600) 
			end
		end

		-- set VR mode
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 110 - (20 * is_popped) and y <= winTop - winBottom - 90 - (20 * is_popped)) then
			logMsg("in vr mode statement")
			if (is_vr == 0) then
				tLeft, tTop, tRight, tBottom = float_wnd_get_geometry(fwl_wnd)
				float_wnd_set_positioning_mode(fwl_wnd, 5, -1)
			else
				float_wnd_set_positioning_mode(fwl_wnd, 0, -1)
				float_wnd_set_geometry(fwl_wnd, tLeft, tTop, tLeft + 800, tTop - 600) 
			end
		end
	
		-- Sets the policy for how X-Plane will position your window
	    -- xplm_WindowPositionFree				0
	    -- xplm_WindowCenterOnMonitor			1
	    -- xplm_WindowFullScreenOnMonitor		2
	    -- xplm_WindowFullScreenOnAllMonitors	3
		-- xplm_WindowPopOut					4
	    -- xplm_WindowVR						5
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 135 - (20 * is_popped) and y <= winTop - winBottom - 115 - (20 * is_popped)) then
			logMsg("in positioning statement")
			float_wnd_set_positioning_mode(fwl_wnd, 1, -1)
		end

		-- window gravity
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 160 - (20 * is_popped) and y <= winTop - winBottom - 140 - (20 * is_popped)) then
			logMsg("in gravity statement")
			float_wnd_set_gravity(fwl_wnd, gLeft, gTop, gRight, gBottom)
		end
			
		-- set window resizing limits
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 185 - (20 * is_popped) and y <= winTop - winBottom - 165 - (20 * is_popped)) then
			logMsg("in resizing statement")
			float_wnd_set_resizing_limits(fwl_wnd, minWidth, minHeight, maxWidth, maxHeight)
		end

		-- set window geometry
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 210 - (20 * is_popped) and y <= winTop - winBottom - 190 - (20 * is_popped)) then
			logMsg("in Geometry statement")
			if (is_popped == 1) then
				-- uses operating system pixel coordinates
				float_wnd_set_geometry_os(fwl_wnd, winLeft + 500, winTop - 500, winLeft + 660 + 500, winTop - 500 - 500)
			elseif (is_vr == 1) then
				-- uses boxels of a window in VR
				float_wnd_set_geometry_vr(fwl_wnd, vrWidth + 200, vrHeight + 200) 
			else
				-- uses global desktop boxel coordinates
				float_wnd_set_geometry(fwl_wnd, winLeft + 300, winTop - 300, winLeft + 640 + 300, winTop - 480 - 300) 
			end
		end
			
		-- window is front
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 235 - (20 * is_popped) and y <= winTop - winBottom - 215 - (20 * is_popped)) then
			logMsg("in isFront statement")
			toggle_second_window = "new"
			pTitle = "FlyWithLua isFront Window 2"
			call_timer = 1
			toggle_timer = 1
			generic_timer = os.time() + 10.0
			generic_counter = os.time()		
			logMsg("isFront timer start")
		end

		-- isVisible - setting to 0 ends the script
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 260 - (20 * is_popped) and y <= winTop - winBottom - 240 - (20 * is_popped)) then
			logMsg("in isVisible statement")
			toggle_second_window = "new"
			pTitle = "FlyWithLua isVisible Window 2"
			call_timer = 2
			toggle_timer = 1
			generic_timer = os.time() + 10.0
			generic_counter = os.time()		
			logMsg("isVisible timer show start")
		end		
	end	--  IF state
end	-- function on_click

-- When on_close is called, it is illegal to do anything with the fwl_wnd variable outside of this function
-- It is also not allowed to create new windows in on_close!
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
		float_wnd_set_geometry(fwl_wnd, winLeft + 100, winTop - 100, winLeft + 800 + 100, winTop - 600 - 100) 
		des_wnd = float_wnd_create(800, 600, 1, true)
		float_wnd_set_title(des_wnd, pTitle)
		float_wnd_set_ondraw(des_wnd, "des_ondraw")
		float_wnd_set_onclick(des_wnd, "des_onclick")
		float_wnd_set_onclose(des_wnd, "des_onclose")
		if (is_popped == 1) then 
			float_wnd_set_positioning_mode(des_wnd, 4, -1) 
			float_wnd_set_geometry_os(des_wnd, winLeft - 200, winTop - 100, winLeft + 820 - 200, winTop - 620 - 100) 
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
		if (is_popped == 1) then 
			float_wnd_set_positioning_mode(des_wnd, 4, -1) 
			float_wnd_set_geometry_os(des_wnd, winLeft - 200, winTop - 100, winLeft + 820 - 200, winTop - 620 - 100) 
		end
	end

	if (toggle_second_window == "front") then
		logMsg("in second_window front")
		if (is_popped == 1) then
			-- isFront doesn't appear to work for a popped window, thus this workaround
			-- I tried to just do a set_positioning_mode(fwl_wnd, 4, -1) but that created an orphan window
			float_wnd_set_positioning_mode(fwl_wnd, 0, -1) 
			float_wnd_set_positioning_mode(fwl_wnd, 4, -1) 
			float_wnd_set_geometry_os(fwl_wnd, winLeft, winTop, winLeft + 820, winTop - 620) 
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
end

-- width, height, decoration style as per XPLMCreateWindowEx. 1 for solid background, 3 for transparent
fwl_wnd = float_wnd_create(800, 600, 1, true)
float_wnd_set_title(fwl_wnd, "FlyWithLua Test")
float_wnd_set_ondraw(fwl_wnd, "on_draw")
float_wnd_set_onclick(fwl_wnd, "on_click")
float_wnd_set_onclose(fwl_wnd, "on_close")

do_every_frame("second_window()")

-- To destroy a window:
-- float_wnd_destroy(fwl_wnd) 
-- after that, do not use the wnd variable anymore
