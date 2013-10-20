-- we will show the last metar at the top of the screen
-- and we will give the next airport too
-- plus we show the FPS value

-- we need a local DataRef for FPS calc
local frame_rate_ref = XPLMFindDataRef("sim/operation/misc/frame_rate_period")
local fps = 0

-- If the function calc_metar_and_airport() fails, we provide a dummy text.
-- This is important when Lua starts the script, and the drawing callback starts before the calculation.
local last_metar_and_airport_info = "Something goes wrong!"

-- We will also have to provide the length of the string in pixel
local info_lenght = measure_string( last_metar_and_airport_info )

-- make a function to be every second, finding out the info we need
function calc_metar_and_airport()
    -- the last metar is a string inside the predefined variable XSB_METAR and only filled when online
    -- we only have to find out the next airport name and position
    -- we will only get an index to the nav database
    -- LATITUDE and LONGITUDE are predefined datarefs representing our plane's position
    -- the nil arguments are used, when we do not care about the search value (name, ID, frequency)
    next_airport_index = XPLMFindNavAid( nil, nil, LATITUDE, LONGITUDE, nil, xplm_Nav_Airport)
    
    -- let's examine the name of the airport we found, all variables should be local
    -- we do not waste Lua with variables to avoid conflicts with other scripts
    local outID
    local outName
    
    -- all output we are not intereted in can be send to variable _ (a dummy variable)
    _, _, _, _, _, _, outID, outName = XPLMGetNavAidInfo( next_airport_index )
    
    -- the last step is to create a global variable the printing function can read out
    fps = 1/XPLMGetDataf(frame_rate_ref)
	last_metar_and_airport_info = string.format("Last METAR: %s, your next airport: %s (%s) [%s at %2.1f fps]", XSB_METAR, outName, outID, PLANE_ICAO, fps)
    info_lenght = measure_string( last_metar_and_airport_info, "Helvetica_12" )
end

-- This function only prints the info text. If it has to calc it every frame, we will get a poor performance!
function show_metar_and_airport()
    -- we can use the predefined variables SCREEN_WIDTH and SCREEN_HIGHT to position the text at the top of the screen
    -- but we need the lenght of the info in pixel
	glColor4f(1,1,1,1)
    draw_string_Helvetica_12(SCREEN_WIDTH - info_lenght - 10, SCREEN_HIGHT - 15, last_metar_and_airport_info)
end

-- register the functions to the callbacks
do_often("calc_metar_and_airport()")
do_every_draw("show_metar_and_airport()")