--[[
-- PlaceAircraftAtLocation test script
-- 
-- Demonstrates a number of coordinates around the world to start at, at different speeds, headings, elevations.

-- XPLM Function Used --
-- XPLMPlaceUserAtLocation(double latitudeDegrees, double longitudeDegrees, float elevationMetersMSL, float headingDegreesTrue, float speedMetersPerSecond)
-- Places the user at a specific location after performing any necessary scenery loads.
-- As with in-air starts initiated from the X-Plane user interface, the aircraft will always start with its engines running, 
-- regardless of the user’s preferences (i.e., regardless of what the dataref sim/operation/prefs/startup_running says)

-- Note that the elevation is meters MSL, heading is true heading and speed is m/s
-- WARNING: teleport reloads scenery and FlyWithLua!!
--]]

function teleport(lat, long, ele, hdg, spd)
  place_aircraft_at_location(lat, long, ele, hdg, spd)
end

add_macro("Teleport Sydney", "teleport(-33.8568, 151.2153, 500, 285, 47)")
add_macro("Teleport NY", "teleport(40.6892, -74.0445, 1000, 265, 63)")
add_macro("Teleport London", "teleport(51.5079, -0.0877, 2000, 050, 70)")
add_macro("Teleport Las Vegas", "teleport(36.1699, -115.1398, 900, 012, 54)")
add_macro("Teleport Paris", "teleport(48.8584, 2.2945, 999, 352, 49)")
add_macro("Teleport Rome", "teleport(41.9029, 12.4534, 0, 0, 0)")