-- automatic set QNH (MissingB Plus)
-- (c) 2012 Ingo Alm, Carsten Lynker
-- build 2012-09-30

-- we want a classic plugin for performance and to avoid conficts to unknown user scripts
-- so we need references instead of dataref() comfort, all references start with "r_"
r_MBPlus_QNHPilot   = XPLMFindDataRef("sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot")
r_MBPlus_QNHCoPilot = XPLMFindDataRef("sim/cockpit2/gauges/actuators/barometer_setting_in_hg_copilot")
r_MBPlus_QNH        = XPLMFindDataRef("sim/weather/barometer_sealevel_inhg")

-- the user can set QNH to automatic
MBPlus_automatic_on = false
add_macro("automatic set QNH", "MBPlus_automatic_on = true", "MBPlus_automatic_on = false", "deactivate")

-- now we provide a custom command to set the QNH (single shot)
create_command("FlyWithLua/Ingo/SetQNH", "set QNH for pilot and copilot",
[[-- set QNH (single shot)
XPLMSetDataf(r_MBPlus_QNHPilot,   XPLMGetDataf(r_MBPlus_QNH))
XPLMSetDataf(r_MBPlus_QNHCoPilot, XPLMGetDataf(r_MBPlus_QNH))
XPLMSpeakString("QNH adjusted!")]],
"", -- do nothing if the button is hold down
"") -- do nothing if the button is released

-- we set QNH every secound when in automatic mode
do_often([[-- QNH auto set?
if MBPlus_automatic_on and (math.abs(XPLMGetDataf(r_MBPlus_QNHPilot)-XPLMGetDataf(r_MBPlus_QNH)) > 0.01) then
    command_once('FlyWithLua/Ingo/SetQNH')
end]])

-- create an info function
function MBPlus_info_to_screen()
    if math.abs(XPLMGetDataf(r_MBPlus_QNHPilot)-XPLMGetDataf(r_MBPlus_QNH)) > 0.01 then
        draw_string(40, 60, "Your Altitude may show wrong values!")
    end
    if math.abs(XPLMGetDataf(r_MBPlus_QNHPilot)-XPLMGetDataf(r_MBPlus_QNHCoPilot)) > 0.01 then
        draw_string(40,40, "Pilot and Copilot have different altimeter settings!")
    end
    if math.abs(XPLMGetDataf(r_MBPlus_QNHPilot)-29.92) < 0.01 then
        draw_string(40,20, "Pilot's altimeter shows flightlevel.")
    end
end

-- show QNH Info on bottom left screen like FlyVFR
do_every_draw("MBPlus_info_to_screen()")