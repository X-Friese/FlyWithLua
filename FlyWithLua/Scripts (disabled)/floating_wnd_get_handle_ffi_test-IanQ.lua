-- Original Script zfw_XPLM_testing.lua by IanQ 10-27-18
--
-- For resizing, if you set the ResizingLimit in a floating window, then the popped window stays set, even a SetGeometry won't change it.
-- However, for a popped window, a SetGeometryOS will still change the size, but if you use the drag handles, it pops back to the limited size.
-- Tested this in C too, same results.

-- The SetGravity is still a weird one, it appears to work, I think the challenge is how to interpret the instructions and what a 0,0.5,
-- or 1 does in each field, almost need to write up a matrix to understand it.

-- Have tested all the methods I wanted, the only ones I never got to with ffi is GetAllMonitorBounds* and the Get/SetRefCon.
-- GetAllMonitorBounds*, from what I understand, can't be accessed via ffi as there is no way to code the callback functions as I need a
-- C pointer to the callback function, and for the RefCon still haven't worked out in ffi how to reference a void* pointer and 
-- void* passed parameter, I tried ffi.cast, ffi.new, etc, etc.

-- From all my hours of testing, never had an issue with the XPLM window handle method, it appears to work perfectly.
--
-- 
-- Modified by William R Good 10-31-18


if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

require("graphics")

-- load the XPLM library
local ffi = require("ffi")

-- find the right lib to load
local XPLMlib = ""
if SYSTEM == "IBM" then
	-- Windows OS (no path and file extension needed)
  	if SYSTEM_ARCHITECTURE == 64 then
    		XPLMlib = "XPLM_64"  -- 64bit
  	else
    		XPLMlib = "XPLM"     -- 32bit
  	end
elseif SYSTEM == "LIN" then
  	-- Linux OS (we need the path "Resources/plugins/" here for some reason)
  	if SYSTEM_ARCHITECTURE == 64 then
    		XPLMlib = "Resources/plugins/XPLM_64.so"  -- 64bit
  	else
    		XPLMlib = "Resources/plugins/XPLM.so"     -- 32bit
  	end
elseif SYSTEM == "APL" then
  	-- Mac OS (we need the path "Resources/plugins/" here for some reason)
  	XPLMlib = "Resources/plugins/XPLM.framework/XPLM" -- 64bit and 32 bit
else
  	return -- this should not happen
end

-- load the lib and store in local variable
local XPLM = ffi.load(XPLMlib)

-- define all XPLM functions
ffi.cdef("typedef void * XPLMWindowID")
ffi.cdef("typedef int XPLMWindowPositioningMode")
ffi.cdef[[typedef void (* XPLMReceiveMonitorBoundsGlobal_f)(
                                   int                  inMonitorIndex,    
                                   int                  inLeftBx,    
                                   int                  inTopBx,    
                                   int                  inRightBx,    
                                   int                  inBottomBx,    
                                   void *               inRefcon);]]
ffi.cdef[[typedef void (* XPLMReceiveMonitorBoundsOS_f)(
                                   int                  inMonitorIndex,    
                                   int                  inLeftPx,    
                                   int                  inTopPx,    
                                   int                  inRightPx,    
                                   int                  inBottomPx,    
                                   void *               inRefcon);]]

ffi.cdef("int XPLMWindowIsPoppedOut(XPLMWindowID inWindowID)")	--DONE
ffi.cdef("void XPLMSetWindowTitle(XPLMWindowID inWindowID, const char * inWindowTitle)")	--DONE
ffi.cdef("void XPLMGetMouseLocationGlobal(int * outX, int * outY)")	--DONE

ffi.cdef("void XPLMGetWindowGeometry(XPLMWindowID inWindowID, int * outLeft, int * outTop, int * outRight, int * outBottom)")	--DONE
ffi.cdef("void XPLMSetWindowGeometry(XPLMWindowID inWindowID, int inLeft, int inTop, int inRight, int inBottom)")	--DONE

ffi.cdef("void XPLMGetWindowGeometryOS(XPLMWindowID inWindowID, int * outLeft, int * outTop, int * outRight, int * outBottom)")	--DONE
ffi.cdef("void XPLMSetWindowGeometryOS(XPLMWindowID inWindowID, int inLeft, int inTop, int inRight, int inBottom)")	--DONE

								   
ffi.cdef("void XPLMGetScreenSize(int * outWidth, int * outHeight)")	--DONE
ffi.cdef("void XPLMGetScreenBoundsGlobal(int * inLeft, int * inTop, int * inRight, int * inBottom)")	--DONE
ffi.cdef("void XPLMGetAllMonitorBoundsGlobal(XPLMReceiveMonitorBoundsGlobal_f inMonitorBoundsCallback, void * inRefcon)")	-- Not sure if can do callback via ffi
ffi.cdef("void XPLMGetAllMonitorBoundsOS(XPLMReceiveMonitorBoundsOS_f inMonitorBoundsCallback, void * inRefcon)")	-- Not sure if can do callback via ffi

ffi.cdef("void XPLMSetWindowGravity(XPLMWindowID inWindowID, float inLeftGravity, float inTopGravity, float inRightGravity, float inBottomGravity)")	--DONE
ffi.cdef("void XPLMSetWindowResizingLimits(XPLMWindowID inWindowID, int inMinWidthBoxels, int inMinHeightBoxels, int inMaxWidthBoxels, int inMaxHeightBoxels)")	--DONE
ffi.cdef("void XPLMSetWindowPositioningMode(XPLMWindowID inWindowID, XPLMWindowPositioningMode inPositioningMode, int inMonitorIndex)")	--DONE

ffi.cdef("int XPLMIsWindowInFront(XPLMWindowID inWindowID)")	--DONE
ffi.cdef("void XPLMBringWindowToFront(XPLMWindowID inWindowID)")	--DONE

ffi.cdef("int XPLMGetWindowIsVisible(XPLMWindowID inWindowID)")	--DONE
ffi.cdef("void XPLMSetWindowIsVisible(XPLMWindowID inWindowID, int inIsVisible)")	--DONE

ffi.cdef("void * XPLMGetWindowRefCon(XPLMWindowID inWindowID)")					--can't seem to work out how to define a pointer to a void function in ffi
ffi.cdef("void XPLMSetWindowRefCon(XPLMWindowID inWindowID, void * inRefcon)")	--can't seem to work out how to define a void* in ffi

ffi.cdef("int XPLMWindowIsInVR(XPLMWindowID inWindowID)")	--DONE


-- set up variables to handle c pointers
local wLeft = ffi.new("int[1]")
local wRight = ffi.new("int[1]")
local wTop = ffi.new("int[1]")
local wBottom = ffi.new("int[1]")
local sWidth = ffi.new("int[1]")
local sHeight = ffi.new("int[1]")
local mgx = ffi.new("int[1]")
local mgy = ffi.new("int[1]")

local xplm_wnd			-- variable for XPLM window handle
local xplm_des_wnd		-- variable for XPLM 2nd window handle
local is_popped = 0		-- flag for window popped
local is_visible = 1	-- flag for window visible
local is_front = 0		-- flag for window in front
local is_vr = 0			-- flag for window in VR

local winx, winy = 0,0												-- window x,y coordinates
local mousex, mousey = 0,0											-- mouse click x,y coordinates
local winLeft, winTop, winRight, winBottom = 0,0,0,0				-- float_wnd_get_geometry/os
local ssWidth, ssHeight = 0,0										-- float_wnd_get_screen_size
local gLeft, gTop, gRight, gBottom = 0.0,1.0,1.0,1.0				-- float_wnd_set_gravity
local dWidth, dHeight = 0,0											-- float_wnd_get_dimensions
local bLeft, bTop, bRight, bBottom = 0,0,0,0						-- float_wnd_get_screen_bounds_global
local msgx, msgy = 0,0												-- float_wnd_get_mouse_location_global(fwl_wnd)
local minWidth, minHeight, maxWidth, maxHeight = 800,600,800,600	-- float_wnd_set_resizing_limits

local chg_title = 0				-- toggle for title change
local toggle_create_window = 0	-- toggle for creating 2nd window

local toggle_timer = 0			-- for timer function
local generic_timer = os.time()

local text_vr = {"not ", ""}	-- for draw.string
local text_os = {"", "OS"}


-- used for 2nd window of inFront method
function des_ondraw(des_wnd, x, y)
	if (toggle_timer > 0 and toggle_timer < 3) then 
		local dWidth, dHeight = float_wnd_get_dimensions(des_wnd)
		draw_string(x + dWidth - 130, y + 5, "Time Remaining = " .. generic_timer - os.time(), "white")
	end
end

-- x and y are the origin of the window, i.e. the lower left
-- x increases to the right, y increases to the top
function on_draw(fwl_wnd, x, y)
	
	-- used in logMsg to display window coords
	winx, winy = x, y
	
	is_popped = XPLM.XPLMWindowIsPoppedOut(xplm_wnd)
	is_visible = XPLM.XPLMGetWindowIsVisible(xplm_wnd)
	is_front = XPLM.XPLMIsWindowInFront(xplm_wnd)
	is_vr = XPLM.XPLMWindowIsInVR(xplm_wnd)
	
	if (is_popped == 0) then 
		-- uses global desktop boxel coordinates
		XPLM.XPLMGetWindowGeometry(xplm_wnd, wLeft, wTop, wRight, wBottom)
	else
		-- uses operating system pixel coordinates
		XPLM.XPLMGetWindowGeometryOS(xplm_wnd, wLeft, wTop, wRight, wBottom)
	end

	winLeft, winTop, winRight, winBottom = wLeft[0], wTop[0], wRight[0], wBottom[0]

	-- Pre-existing float_wnd command, calculates window size
	dWidth, dHeight = float_wnd_get_dimensions(fwl_wnd)
	
	-- returns the current mouse location in global desktop boxels
	XPLM.XPLMGetMouseLocationGlobal(mgx, mgy)
	msgx, msgy = mgx[0], mgy[0]

	-- returns the size of the main X-Plane OpenGL window in pixels
	XPLM.XPLMGetScreenSize(sWidth, sHeight)
	ssWidth, ssHeight = sWidth[0], sHeight[0]
	
	-- returns the bounds of the "global" X-Plane desktop, in boxels
	XPLM.XPLMGetScreenBoundsGlobal(wLeft, wTop, wRight, wBottom)
	bLeft, bTop, bRight, bBottom = wLeft[0], wTop[0], wRight[0], wBottom[0]

	-- brings the window to the front of the Z-order for its layer
	if (toggle_timer == 1 and os.time() > generic_timer) then
		logMsg("windowFront timer done")
		XPLM.XPLMBringWindowToFront(xplm_wnd)
		toggle_timer = 2
		generic_timer = os.time() + 10.0
	end

	if (toggle_timer == 2 and os.time() > generic_timer) then
		logMsg("windowFront close test window")
		XPLM.XPLMBringWindowToFront(xplm_des_wnd)
		toggle_timer = 0
		toggle_create_window = 3
	end

	if (toggle_timer > 0 and toggle_timer < 3) then 
		logMsg("window in front = " .. is_front .. " , " .. toggle_timer .. " , " .. os.time() .. " , " .. generic_timer)
		draw_string(x + dWidth - 130, y + 5, "Time Remaining = " .. generic_timer - os.time(), "white")
	end

	if (is_popped == 0) then 
		draw_string(winLeft + 260, winTop - 20, "Window is " .. text_vr[is_vr + 1] .. "VR", "white")
		draw_string(winLeft + 20, winTop - 20, "Window is Floating Window", "white")
		draw_string(winLeft + 20, winTop - 60, "Click HERE to change window title", "cyan")
		draw_string(winLeft + 20, winTop - 85, "Click HERE to change window positioning mode", "cyan")
		draw_string(winLeft + 20, winTop - 110, "Click HERE to set window gravity to " .. gLeft .. "," .. gTop .. "," .. gRight .. "," .. gBottom, "green")
		draw_string(winLeft + 20, winTop - 135, "Click HERE to set window resizing limits", "yellow")
		draw_string(winLeft + 20, winTop - 160, "Click HERE to change window geometry", "magenta")
		draw_string(winLeft + 20, winTop - 185, "Click HERE to bring window to front", "gray")
		draw_string(winLeft + 20, winTop - 210, "Click HERE to change window visibility - reload lua scripts after", "red")
	else
		draw_string(x + 260, y + (winTop - winBottom - 20) - 20, "Window is " .. text_vr[is_vr + 1] .. "VR", "white")
		draw_string(x + 20, y + (winTop - winBottom - 20) - 20, "Window is OS Window", "white")
		draw_string(x + 20, y + (winTop - winBottom - 20) - 85, "Click HERE to change window positioning mode", "cyan")
		draw_string(x + 20, y + (winTop - winBottom - 20) - 110, "Click HERE to set window gravity to " .. gLeft .. "," .. gTop .. "," .. gRight .. "," .. gBottom, "green")
		draw_string(x + 20, y + (winTop - winBottom - 20) - 135, "Click HERE to set window resizing limits", "yellow")
		draw_string(x + 20, y + (winTop - winBottom - 20) - 160, "Click HERE to change window geometry", "magenta")
		draw_string(x + 20, y + (winTop - winBottom - 20) - 185, "Click HERE to bring window to front", "gray")
		draw_string(x + 20, y + (winTop - winBottom - 20) - 210, "Click HERE to change window visibility - reload lua scripts after", "red")
	end

	draw_string(x + 2, y + 110, "Mouse Click x,y = " .. mousex .. " , " .. mousey, "white")
	draw_string(x + 2, y + 95, "Global Mouse Location x,y = " .. msgx .. " , " .. msgy, "white")
	draw_string(x + 2, y + 80, "float_wnd_get_dimensions = " .. dWidth .. " x " .. dHeight, "white")
	draw_string(x + 2, y + 65, "Window Geometry" .. text_os[is_popped + 1] .. " LTRB = " .. winLeft .. " , " .. winTop .. " , " .. winRight .. " , " .. winBottom, "white")
	draw_string(x + 2, y + 50, "Window Size = " .. winLeft - winRight .. " x " .. winTop - winBottom, "white")
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
				XPLM.XPLMSetWindowTitle(xplm_wnd, "Title Changed") 
				chg_title = 1
			else
				XPLM.XPLMSetWindowTitle(xplm_wnd, "Title Changed Again") 
				chg_title = 0
			end
		end
	
		-- Sets the policy for how X-Plane will position your window
	    -- xplm_WindowPositionFree				0
	    -- xplm_WindowCenterOnMonitor			1
	    -- xplm_WindowFullScreenOnMonitor		2
	    -- xplm_WindowFullScreenOnAllMonitors	3
		-- xplm_WindowPopOut					4
	    -- xplm_WindowVR						5
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 85 - (20 * is_popped) and y <= winTop - winBottom - 65 - (20 * is_popped)) then
			logMsg("in positioning statement")
			XPLM.XPLMSetWindowPositioningMode(xplm_wnd, 1, -1)
		end

		-- window gravity
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 110 - (20 * is_popped) and y <= winTop - winBottom - 90 - (20 * is_popped)) then
			logMsg("in gravity statement")
			XPLM.XPLMSetWindowGravity(xplm_wnd, gLeft, gTop, gRight, gBottom)
		end
			
		-- set window resizing limits
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 135 - (20 * is_popped) and y <= winTop - winBottom - 115 - (20 * is_popped)) then
			logMsg("in resizing statement")
			XPLM.XPLMSetWindowResizingLimits(xplm_wnd, minWidth, minHeight, maxWidth, maxHeight)
		end

		-- set window geometry
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 160 - (20 * is_popped) and y <= winTop - winBottom - 140 - (20 * is_popped)) then
			logMsg("in Geometry statement")
			if (is_popped == 0) then
				-- uses global desktop boxel coordinates
				XPLM.XPLMSetWindowGeometry(xplm_wnd, winLeft + 300, winTop - 300, winLeft + 640 + 300, winTop - 480 - 300) 
			else
				-- uses operating system pixel coordinates
				XPLM.XPLMSetWindowGeometryOS(xplm_wnd, winLeft + 500, winTop - 500, winLeft + 660 + 500, winTop - 500 - 500) 
			end
		end
			
		-- window in front
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 185 - (20 * is_popped) and y <= winTop - winBottom - 165 - (20 * is_popped)) then
			logMsg("in windowFront statement")
			generic_timer = os.time() + 10.0
			toggle_timer = 1
			toggle_create_window = 1
			create_window()
			if (is_popped == 1) then 
				XPLM.XPLMSetWindowPositioningMode(xplm_des_wnd, 4, -1) 
				XPLM.XPLMSetWindowGeometryOS(xplm_des_wnd, winLeft + 100, winTop - 100, winLeft + 660 + 100, winTop - 500 - 100) 
			end
		end

		-- set Visible
		if (x >= 45 and x <= 250 and y >= winTop - winBottom - 210 - (20 * is_popped) and y <= winTop - winBottom - 190 - (20 * is_popped)) then
			logMsg("in isVisible statement")
			-- setting to 0 effectively ends the window display
			XPLM.XPLMSetWindowIsVisible(xplm_wnd, 1 - XPLM.XPLMGetWindowIsVisible(xplm_wnd))
		end
	end	-- IF state
end	-- function on_click

-- When on_close it called, it is illegal to do anything with the fwl_wnd variable outside of this function
-- It is also not allowed to create new windows in on_close!
function on_close(fwl_wnd)
	logMsg("fwl_wnd on_close")
end

function des_onclick(des_wnd)
end

function des_onclose(des_wnd)
end

function create_window()
	if (toggle_create_window == 1) then
		XPLM.XPLMSetWindowGeometry(xplm_wnd, winLeft + 100, winTop - 100, winLeft + 800 + 100, winTop - 600 - 100) 
		des_wnd = float_wnd_create(640, 480, 1, true)
		xplm_des_wnd = float_wnd_get_xplm_handle(des_wnd)	-- get the XPLM window handle for 2nd window
		float_wnd_set_title(des_wnd, "FlyWithLua inFront Window 2")
		float_wnd_set_ondraw(des_wnd, "des_ondraw")
		float_wnd_set_onclick(des_wnd, "des_onclick")
		float_wnd_set_onclose(des_wnd, "des_onclose")
		toggle_create_window = 2
	end

	if (toggle_create_window == 3) then
		des_onclose(des_wnd) 
		float_wnd_destroy(des_wnd)
		toggle_create_window = 0
	end

end

-- width, height, decoration style as per XPLMCreateWindowEx. 1 for solid background, 3 for transparent
fwl_wnd = float_wnd_create(800, 600, 1, true)
xplm_wnd = float_wnd_get_xplm_handle(fwl_wnd)	-- get the XPLM window handle
float_wnd_set_title(fwl_wnd, "FlyWithLua Test")
float_wnd_set_ondraw(fwl_wnd, "on_draw")
float_wnd_set_onclick(fwl_wnd, "on_click")
float_wnd_set_onclose(fwl_wnd, "on_close")

do_every_frame("create_window()")

-- To destroy a window:
-- float_wnd_destroy(fwl_wnd) 
-- after that, do not use the wnd variable anymore
