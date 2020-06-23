--[[
-- Place Aircraft At ICAO Airport Location test script
-- 
-- WARNING: place_aircraft_at reloads scenery and FlyWithLua!!
--]]
myICAO = "KERI"

function place_aircraft(myICAO)
  place_aircraft_at(myICAO)
end

add_macro("Place At KCLE", "place_aircraft('KCLE')")
add_macro("Place At KYNG", "place_aircraft('KYNG')")
add_macro("Place At KERI", "place_aircraft('KERI')")
add_macro("Place At KDKK", "place_aircraft('KDKK')")
add_macro("Place At KGKJ", "place_aircraft('KGKJ')")
add_macro("Place At KJHW", "place_aircraft('KJHW')")
