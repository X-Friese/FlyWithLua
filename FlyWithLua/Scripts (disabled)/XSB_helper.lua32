require "XSquawkBox"

function file_TP_VFR_flightplan()
    -- is the user connected to VATSIM?
    if XSB_CON_STATUS < 2 then
        XSBUserLogin()
        return
    end
    XSB_FP_FLIGHT_TYPE = 86   -- ASCII "V"
    XSB_FP_TCAS_TYPE = 84     -- ASCII "T"
    XSB_FP_NAV_TYPE = 71      -- ASCII "G"
    XSB_FP_ROUTE = "Airfield traffic pattern"
    XSB_FP_SPEED = get("sim/aircraft/view/acf_Vno") * 0.8  -- flying 20% lower than Vno
    XSB_FP_CRUISE_ALTITUDE = math.ceil(ELEVATION * 0.032808399 + 10) * 100
    XSB_FP_DEPARTURE_TIME = tonumber(os.date("!%H%M"))
    XSB_FP_DEPARTURE_TIME_ACTUAL = tonumber(os.date("!%H%M"))
    XSB_FP_ENROUTE_HRS = 1
    XSB_FP_ENROUTE_MINS = 0
    local fuel_remaining = get("sim/flightmodel/weight/m_fuel_total") / get("sim/aircraft/limits/green_hi_FF") / 3.25
    XSB_FP_FUEL_HRS = math.floor( fuel_remaining )
    XSB_FP_FUEL_MINS = math.ceil( (fuel_remaining - math.floor( fuel_remaining )) * 60 )
    _, _, _, _, _, _, XSB_FP_DEPARTURE_AIRPORT, _ = XPLMGetNavAidInfo( XPLMFindNavAid( nil, nil, LATITUDE, LONGITUDE, nil, xplm_Nav_Airport) )
    _, _, _, _, _, _, XSB_FP_ARRIVAL_AIRPORT, _ = XPLMGetNavAidInfo( XPLMFindNavAid( nil, nil, LATITUDE, LONGITUDE, nil, xplm_Nav_Airport) )
    _, _, _, _, _, _, XSB_FP_ALTERNATE_AIRPORT, _ = XPLMGetNavAidInfo( XPLMFindNavAid( nil, nil, LATITUDE, LONGITUDE, nil, xplm_Nav_Airport) )
    XSB_FP_REMARKS = "/v/ XSquawkBox / daytime / forced VMC"
    XSBShowFlightplan()
end

add_ATC_macro("create Traffic Pattern flightplan", "file_TP_VFR_flightplan()")