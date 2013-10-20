-- script: call ATC on airport
-- build:  2012-10-04

require "radio"
require "XSquawkBox"

function call_aproach_ATC()
    -- find the ICAO name of the airport
    local airport_ICAO
    _, _, _, _, _, _, airport_ICAO, _ = XPLMGetNavAidInfo( XPLMFindNavAid( nil, nil, LATITUDE, LONGITUDE, nil, xplm_Nav_Airport) )

    -- try to find a normal aproach controller
    local frequency = XSBLookupATC(airport_ICAO .. "_APP")
    if frequency > 0 then
        COM1 = frequency
        XSBSpeakString("COM1 was set to " .. airport_ICAO .. "_APP.")
        return
    end
    -- or is a director online?
    frequency = XSBLookupATC(airport_ICAO .. "_F_APP")
    if frequency > 0 then
        COM1 = frequency
        XSBSpeakString("COM1 was set to " .. airport_ICAO .. "_F_APP.")
    else
        XSBSpeakString("Sorry, no aproach controller or director online, COM1 is set to UNICOM.")
        COM1 = 12280
    end
end

function call_tower_ATC()
    -- find the ICAO name of the airport
    local airport_ICAO
    _, _, _, _, _, _, airport_ICAO, _ = XPLMGetNavAidInfo( XPLMFindNavAid( nil, nil, LATITUDE, LONGITUDE, nil, xplm_Nav_Airport) )
    
    -- check for the tower controller
    local frequency = XSBLookupATC(airport_ICAO .. "_TWR")
    if frequency > 0 then
        COM1 = frequency
        XSBSpeakString("COM1 was set to " .. airport_ICAO .. "_TWR.")
        return
    end
    -- more than one tower online? Trying compass directions
    frequency = XSBLookupATC(airport_ICAO .. "_N_TWR")
    if frequency > 0 then
        COM1 = frequency
        XSBSpeakString("COM1 was set to " .. airport_ICAO .. "_N_TWR.")
        return
    end
    frequency = XSBLookupATC(airport_ICAO .. "_S_TWR")
    if frequency > 0 then
        COM1 = frequency
        XSBSpeakString("COM1 was set to " .. airport_ICAO .. "_S_TWR.")
        return
    end
    frequency = XSBLookupATC(airport_ICAO .. "_W_TWR")
    if frequency > 0 then
        COM1 = frequency
        XSBSpeakString("COM1 was set to " .. airport_ICAO .. "_W_TWR.")
        return
    end
    frequency = XSBLookupATC(airport_ICAO .. "_E_TWR")
    if frequency > 0 then
        COM1 = frequency
        XSBSpeakString("COM1 was set to " .. airport_ICAO .. "_E_TWR.")
        return
    end
    -- or is it an info tower?
    frequency = XSBLookupATC(airport_ICAO .. "_I_TWR")
    if frequency > 0 then
        COM1 = frequency
        XSBSpeakString("COM1 was set to " .. airport_ICAO .. "_I_TWR.")
    else
        XSBSpeakString("Sorry, no tower controller online, try to find an aproach controller.")
        call_aproach_ATC()
    end
end

function call_ground_ATC()
    -- find the ICAO name of the airport
    local airport_ICAO
    _, _, _, _, _, _, airport_ICAO, _ = XPLMGetNavAidInfo( XPLMFindNavAid( nil, nil, LATITUDE, LONGITUDE, nil, xplm_Nav_Airport) )
    
    -- check for the ground controller
    local frequency = XSBLookupATC(airport_ICAO .. "_GND")
    if frequency > 0 then
        COM1 = frequency
        XSBSpeakString("COM1 was set to " .. airport_ICAO .. "_GND.")
    else
        XSBSpeakString("Sorry, no ground controller online, try to find a tower controller.")
        call_tower_ATC()
    end
end

function call_delivery_ATC()
    -- find the ICAO name of the airport
    local airport_ICAO
    _, _, _, _, _, _, airport_ICAO, _ = XPLMGetNavAidInfo( XPLMFindNavAid( nil, nil, LATITUDE, LONGITUDE, nil, xplm_Nav_Airport) )
    
    -- check for the ground controller
    local frequency = XSBLookupATC(airport_ICAO .. "_DEL")
    if frequency > 0 then
        COM1 = frequency
        XSBSpeakString("COM1 was set to " .. airport_ICAO .. "_DEL.")
    else
        XSBSpeakString("Sorry, no delivery controller online, try to find a ground controller.")
        call_ground_ATC()
    end
end

function call_destination_aproach_ATC()
    -- get the destination airport ICAO from flightplan
    local airport_ICAO = XSB_FP_ARRIVAL_AIRPORT

    -- try to find a normal aproach controller
    local frequency = XSBLookupATC(airport_ICAO .. "_APP")
    if frequency > 0 then
        COM1 = frequency
        XSBSpeakString("COM1 was set to " .. airport_ICAO .. "_APP.")
        return
    end
    -- or is a director online?
    frequency = XSBLookupATC(airport_ICAO .. "_F_APP")
    if frequency > 0 then
        COM1 = frequency
        XSBSpeakString("COM1 was set to " .. airport_ICAO .. "_F_APP.")
    else
        XSBSpeakString("Sorry, no aproach controller or director online, COM1 is set to UNICOM.")
        COM1 = 12280
    end
end

function call_destination_tower_ATC()
    -- get the destination airport ICAO from flightplan
    local airport_ICAO = XSB_FP_ARRIVAL_AIRPORT
    
    -- check for the tower controller
    local frequency = XSBLookupATC(airport_ICAO .. "_TWR")
    if frequency > 0 then
        COM1 = frequency
        XSBSpeakString("COM1 was set to " .. airport_ICAO .. "_TWR.")
        return
    end
    -- more than one tower online? Trying compass directions
    frequency = XSBLookupATC(airport_ICAO .. "_N_TWR")
    if frequency > 0 then
        COM1 = frequency
        XSBSpeakString("COM1 was set to " .. airport_ICAO .. "_N_TWR.")
        return
    end
    frequency = XSBLookupATC(airport_ICAO .. "_S_TWR")
    if frequency > 0 then
        COM1 = frequency
        XSBSpeakString("COM1 was set to " .. airport_ICAO .. "_S_TWR.")
        return
    end
    frequency = XSBLookupATC(airport_ICAO .. "_W_TWR")
    if frequency > 0 then
        COM1 = frequency
        XSBSpeakString("COM1 was set to " .. airport_ICAO .. "_W_TWR.")
        return
    end
    frequency = XSBLookupATC(airport_ICAO .. "_E_TWR")
    if frequency > 0 then
        COM1 = frequency
        XSBSpeakString("COM1 was set to " .. airport_ICAO .. "_E_TWR.")
        return
    end
    -- or is it an info tower?
    frequency = XSBLookupATC(airport_ICAO .. "_I_TWR")
    if frequency > 0 then
        COM1 = frequency
        XSBSpeakString("COM1 was set to " .. airport_ICAO .. "_I_TWR.")
    else
        XSBSpeakString("Sorry, no tower controller online, try to find an aproach controller.")
        call_destination_aproach_ATC()
    end
end

function call_ATIS()
    -- find the ICAO name of the airport
    local airport_ICAO
    _, _, _, _, _, _, airport_ICAO, _ = XPLMGetNavAidInfo( XPLMFindNavAid( nil, nil, LATITUDE, LONGITUDE, nil, xplm_Nav_Airport) )
    
    -- check for ATIS
    local frequency = XSBLookupATC(airport_ICAO .. "_ATIS")
    if frequency > 0 then
        COM2 = frequency
        XSBSpeakString("COM2 was set to " .. airport_ICAO .. "_ATIS at frequency " .. frequency .. "0 kHz.")
    else
        XSBSpeakString("Sorry, no ATIS found.")
    end
end

function call_destination_ATIS()
    -- get the destination airport ICAO from flightplan
    local airport_ICAO = XSB_FP_ARRIVAL_AIRPORT
    
    -- check for ATIS
    local frequency = XSBLookupATC(airport_ICAO .. "_ATIS")
    if frequency > 0 then
        COM2 = frequency
        XSBSpeakString("COM2 was set to " .. airport_ICAO .. "_ATIS at frequency " .. frequency .. "0 kHz.")
    else
        XSBSpeakString("Sorry, no ATIS found.")
    end
end

-- the macros are added in the order we want inside the menu, so we start with the ground controller
add_ATC_macro("listen to ATIS on COM2", "call_ATIS()")
add_ATC_macro("call delivery controller on VATSIM", "call_delivery_ATC()")
add_ATC_macro("call ground controller on VATSIM", "call_ground_ATC()")
add_ATC_macro("call tower controller on VATSIM", "call_tower_ATC()")
add_ATC_macro("call aproach controller on VATSIM", "call_aproach_ATC()")
add_ATC_macro("listen to arrival ATIS on COM2", "call_destination_ATIS()")
add_ATC_macro("call arrival tower controller on VATSIM", "call_destination_tower_ATC()")
add_ATC_macro("call arrival aproach controller on VATSIM", "call_destination_aproach_ATC()")

-- if the user wants to define buttons or keys, we provide the custom commands
create_command("FlyWithLua/ATC/listen_ATIS", "listen to ATIS on COM2", "call_ATIS()", "", "")
create_command("FlyWithLua/ATC/contact_DEL", "contact delivery controller", "call_delivery_ATC()", "", "")
create_command("FlyWithLua/ATC/contact_GND", "contact ground controller", "call_ground_ATC()", "", "")
create_command("FlyWithLua/ATC/contact_TWR", "contact tower controller", "call_tower_ATC()", "", "")
create_command("FlyWithLua/ATC/contact_APP", "contact aproach controller or director", "call_aproach_ATC()", "", "")
create_command("FlyWithLua/ATC/listen_arrival_ATIS", "listen to arrival ATIS on COM2", "call_destination_ATIS()", "", "")
create_command("FlyWithLua/ATC/contact_arrival_TWR", "contact arrival tower controller", "call_destination_tower_ATC()", "", "")
create_command("FlyWithLua/ATC/contact_arrival_APP", "contact arrival aproach controller or director", "call_destination_aproach_ATC()", "", "")