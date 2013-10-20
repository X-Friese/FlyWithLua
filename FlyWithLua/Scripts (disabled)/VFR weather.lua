function set_VFR()
    -- sharing a single variable to create the references needed for XPLM's functions
    local reference

    -- we need at least 8.000 m visibility
    reference = XPLMFindDataRef("sim/weather/visibility_reported_m")
    if XPLMGetDataf(reference) < 8000 then
        XPLMSetDataf(reference, 8000)
    end

    -- vertical clouds to plane distance must be 1000 ft or more
    -- we want 2625 ft (800m) agl for cloudbase
    local radioalt = XPLMGetDataf(XPLMFindDataRef("sim/flightmodel/position/y_agl"))
    reference = XPLMFindDataRef("sim/weather/cloud_base_msl_m[0]")
    if XPLMGetDataf(reference) < (800 + ELEVATION - radioalt) then
        XPLMSetDataf(reference, (800 + ELEVATION - radioalt))
    end

    -- limit rain and thunder as wanted (not an official VFR condition)
    reference = XPLMFindDataRef("sim/weather/rain_percent")
    if XPLMGetDataf(reference) > 0.0 then
        XPLMSetDataf(reference, 0.0)
    end
    reference = XPLMFindDataRef("sim/weather/thunderstorm_percent")
    if XPLMGetDataf(reference) > 0.0 then
        XPLMSetDataf(reference, 0.0)
    end
    
    -- limit the wind to Echo-class plane's scope
    reference = XPLMFindDataRef("sim/weather/wind_speed_kt[0]")
    if XPLMGetDataf(reference) > 25.0 then
        XPLMSetDataf(reference, 25.0)
    end
    reference = XPLMFindDataRef("sim/weather/wind_speed_kt[1]")
    if XPLMGetDataf(reference) > 30.0 then
        XPLMSetDataf(reference, 30.0)
    end
    reference = XPLMFindDataRef("sim/weather/wind_speed_kt[2]")
    if XPLMGetDataf(reference) > 40.0 then
        XPLMSetDataf(reference, 40.0)
    end
    
    -- limit shear wind
    reference = XPLMFindDataRef("sim/weather/shear_speed_kt[0]")
    if XPLMGetDataf(reference) > 5.0 then
        XPLMSetDataf(reference, 5.0)
    end
    reference = XPLMFindDataRef("sim/weather/shear_speed_kt[1]")
    if XPLMGetDataf(reference) > 5.0 then
        XPLMSetDataf(reference, 5.0)
    end
    reference = XPLMFindDataRef("sim/weather/shear_speed_kt[2]")
    if XPLMGetDataf(reference) > 10.0 then
        XPLMSetDataf(reference, 10.0)
    end
    
    -- limit turbulence
    reference = XPLMFindDataRef("sim/weather/turbulence[0]")
    if XPLMGetDataf(reference) > 1.0 then
        XPLMSetDataf(reference, 1.0)
    end
    reference = XPLMFindDataRef("sim/weather/turbulence[1]")
    if XPLMGetDataf(reference) > 2.0 then
        XPLMSetDataf(reference, 2.0)
    end
    reference = XPLMFindDataRef("sim/weather/turbulence[2]")
    if XPLMGetDataf(reference) > 4.0 then
        XPLMSetDataf(reference, 4.0)
    end
end

-- manual single shot
add_macro("set Visual Meteorological Conditions", "set_VFR()")
-- register function to new metar callback
do_on_new_metar("if force_online_VFR then set_VFR() end")

-- airplanes D-Axxx, D-Bxx and D-Cxxx will fly IFR
if XPLMGetDataf(XPLMFindDataRef("sim/aircraft/weight/acf_m_max")) < 5701 then
    add_macro("force online VFR", "force_online_VFR = true", "force_online_VFR = false", "activate")
else
    add_macro("force online VFR", "force_online_VFR = true", "force_online_VFR = false", "deactivate")
end