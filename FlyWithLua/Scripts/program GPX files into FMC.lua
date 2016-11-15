-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Script:  program GPX files into FMC
-- Version: 1.1
-- Build:   2012-10-23
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Description:
-- Create a subfolder "GPX" inside the Scripts folder and put all GPX files made with other software
-- like www.fl95.de (use the export function) into these subfolder.
-- This script will automatically generate ATC macros to fill the FMC with the first path
-- found inside the GPX file.
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

require("LuaXml")

-- define the path to the GPX files and load the directory
local gpx_path = SCRIPT_DIRECTORY .. "GPX" .. DIRECTORY_SEPARATOR
local gpx_files = directory_to_table(gpx_path)

-- generate the ATC menu entries
if #gpx_files == 0 then
    print('No subfolder "GPX" found or no GPX files in it. Create a folder "GPX" inside the Scripts folder and put all your GPX files in it!')
else
    for _, filename in ipairs(gpx_files) do
        if (string.find(filename, ".gpx") == #filename - 3) or (string.find(filename, ".GPX") == #filename - 3) then
            add_ATC_macro(string.format('program "%s" into FMC', filename), string.format('program_gpx_into_FMC(%q)', gpx_path .. filename))
        end
    end
end

-- program the FMC (all elevation values are set to 0)
function program_gpx_into_FMC( filename )
    -- load the GPX file with the help of LuaXML library
    local gpxfile = xml.load(filename)
    -- find the first route
    local route = gpxfile:find("rte")
    -- if we found a route, take them into the FMC
    if route ~= nil then
        -- clear FMS
        logMsg("FlyWithLua Info: Clearing all FMS entries.")
        for i = 0, 99 do
            XPLMClearFMSEntry(i)
        end
        -- set startpoint to actual position
        XPLMSetFMSEntryLatLon(0, LATITUDE, LONGITUDE, 0)
        -- add new waypoints by Lat/Lon
        -- not every route subelement contains Lat/Lon information, so we use j as a second counter
        local j = 1
        for i in ipairs(route) do
            local point = route[i]:find("rtept")
            if point ~= nil then
                XPLMSetFMSEntryLatLon(j, point.lat, point.lon, 0)
                logMsg(string.format("FlyWithLua Info: Programming FMS waypoint no. %i to LAT=%f, LON=%f", j, point.lat, point.lon))
                j = j+1
            end
        end
        -- go to the first waypoint
        XPLMSetDisplayedFMSEntry(1)
        XPLMSetDestinationFMSEntry(1)
    end
end