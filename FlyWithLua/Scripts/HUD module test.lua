require "graphics"
require "HUD"
require "radio"
require "bit"

dataref("ALT", "sim/cockpit2/autopilot/altitude_dial_ft", "writable")
dataref("VVI", "sim/cockpit2/autopilot/vvi_dial_fpm", "writable")
dataref("ALT_ARMED", "sim/cockpit2/autopilot/altitude_hold_armed")
dataref("AP_STATE", "sim/cockpit/autopilot/autopilot_state")
dataref("xp_autopilot_mode", "sim/cockpit/autopilot/autopilot_mode", "writable")
dataref("xp_autopilot_airspeed", "sim/cockpit/autopilot/airspeed", "writable")
dataref("xp_autothrottle_enabled", "sim/cockpit2/autopilot/autothrottle_enabled", "writable")
dataref("QNH_Pilot", "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot", "writable")

-- define a global variable to be used in other scripts (shared with another script)
autopilot_helper_vvi = 600

-- init the HUD
HUD.begin_HUD( -81, 1, 80, 315, "my_radios", "always")

-- Autopilot
HUD.create_element("AP", 0, 280, 40, 35)
HUD.draw_string(5, 10, 18, "AP")
HUD.create_backlight_indicator( 30, 0, 10, 35, "xp_autopilot_mode == 1", 1, 1, 0, 1)
HUD.create_backlight_indicator( 30, 0, 10, 35, "xp_autopilot_mode == 2", 0, 1, 0, 1)
HUD.create_click_switch(0, 0, 40, 35, "xp_autopilot_mode", 1, 2)

-- Speed
HUD.create_element("Speed", 40, 280, 40, 35)
HUD.draw_string(5, 23, 10, "SPD")
HUD.draw_fstring(5, 5, 18, "%03i", "xp_autopilot_airspeed")
HUD.create_wheel_action(0, 0, 25, 35, "xp_autopilot_airspeed = math.floor(xp_autopilot_airspeed / 5 + MOUSE_WHEEL_CLICKS) * 5")
HUD.create_wheel_action(25, 0, 15, 35, "xp_autopilot_airspeed = xp_autopilot_airspeed + MOUSE_WHEEL_CLICKS")
HUD.create_backlight_indicator( 30, 0, 10, 35, "xp_autothrottle_enabled == 1", 0, 1, 0, 1)
HUD.create_click_switch(0, 0, 40, 35, "xp_autothrottle_enabled", 0, 1)

-- ALT
HUD.create_element("ALT", 0, 245, 80, 35)
HUD.draw_string(5, 23, 10, "ALT")
HUD.draw_fstring(15, 5, 18, "%05i", "ALT")
HUD.create_wheel_action( 0, 0, 35, 35, "ALT = math.floor(ALT / 100 + MOUSE_WHEEL_CLICKS * 10) * 100")
HUD.create_wheel_action(36, 0, 40, 35, "ALT = math.floor(ALT / 100 + MOUSE_WHEEL_CLICKS) * 100")
HUD.create_wheel_action( 0, 0, 80, 35, "autopilot_helper_set_VVI()")
HUD.create_backlight_indicator( 70, 0, 10, 35, "bit.band(AP_STATE, 32) > 0", 1, 0, 0, 1)
HUD.create_backlight_indicator( 70, 0, 10, 35, "bit.band(AP_STATE, 16384) > 0", 0, 1, 0, 1)
HUD.create_click_action(0, 0, 80, 35, 'command_once("sim/autopilot/altitude_hold")')

-- VVI
HUD.create_element("VVI", 0, 210, 80, 35)
HUD.draw_string(5, 23, 10, "V V I")
HUD.draw_fstring(25, 5, 18, "%04i", "VVI")
HUD.create_wheel_action(0, 0, 80, 35, "VVI = math.floor(VVI / 100 + MOUSE_WHEEL_CLICKS) * 100")
HUD.create_backlight_indicator( 70, 0, 10, 35, "bit.band(AP_STATE, 16) > 0", 0, 1, 0, 1)
HUD.create_click_action(0, 0, 80, 35, 'command_once("sim/autopilot/vertical_speed")')

-- COM1 radio
HUD.create_element("COM", 0, 140, 80, 35)
HUD.draw_string(5, 23, 10, "COM")
HUD.draw_fstring(12, 5, 18, "%3.2f", "COM1/100")
HUD.create_wheel_action( 0, 0, 45, 35, "COM1 = COM1 + MOUSE_WHEEL_CLICKS * 100")
HUD.create_wheel_action(45, 0, 10, 35, "COM1 = COM1 + MOUSE_WHEEL_CLICKS * 10")
HUD.create_wheel_action(55, 0, 25, 35, "COM1 = COM1 + MOUSE_WHEEL_CLICKS")

-- UNICOM
HUD.create_click_action(0, 0, 80, 35, "COM1 = 12280")

-- NAV1 radio
HUD.create_element("NAV", 0, 105, 80, 35)
HUD.draw_string(5, 23, 10, "NAV")
HUD.draw_fstring(12, 5, 18, "%3.2f", "NAV1/100")
HUD.create_wheel_action( 0, 0, 45, 35, "NAV1 = NAV1 + MOUSE_WHEEL_CLICKS * 100")
HUD.create_wheel_action(45, 0, 10, 35, "NAV1 = NAV1 + MOUSE_WHEEL_CLICKS * 10")
HUD.create_wheel_action(55, 0, 25, 35, "NAV1 = NAV1 + MOUSE_WHEEL_CLICKS")
HUD.create_backlight_indicator( 70, 0, 10, 35, "bit.band(AP_STATE, 256) > 0", 1, 0, 0, 1)
HUD.create_backlight_indicator( 70, 0, 10, 35, "bit.band(AP_STATE, 512) > 0", 0, 1, 0, 1)
HUD.create_click_action(0, 0, 80, 35, 'command_once("sim/autopilot/NAV")')

-- ADF1 radio
HUD.create_element("ADF", 0, 70, 80, 35)
HUD.draw_string(5, 23, 10, "ADF")
HUD.draw_fstring(27, 5, 18, "%04i", "ADF1")
HUD.create_wheel_action( 0, 0, 45, 35, "ADF1 = ADF1 + MOUSE_WHEEL_CLICKS * 100")
HUD.create_wheel_action(45, 0, 10, 35, "ADF1 = ADF1 + MOUSE_WHEEL_CLICKS * 10")
HUD.create_wheel_action(55, 0, 25, 35, "ADF1 = ADF1 + MOUSE_WHEEL_CLICKS")

-- OBS1
HUD.create_element("OBS", 40, 175, 40, 35)
HUD.draw_string(5, 23, 10, "OBS")
HUD.draw_fstring(5, 5, 18, "%03i", "OBS1")
HUD.create_wheel_action(0, 0, 25, 35, "OBS1 = math.floor(OBS1 / 10 + MOUSE_WHEEL_CLICKS) * 10")
HUD.create_wheel_action(25, 0, 15, 35, "OBS1 = OBS1 + MOUSE_WHEEL_CLICKS")
HUD.create_click_action(0, 0, 40, 35, 'command_once("sim/autopilot/NAV")')

-- HDG
HUD.create_element("HDG", 0, 175, 40, 35)
HUD.draw_string(5, 23, 10, "HDG")
HUD.draw_fstring(5, 5, 18, "%03i", "HDG")
HUD.create_wheel_action(0, 0, 25, 35, "HDG = math.floor(HDG / 10 + MOUSE_WHEEL_CLICKS) * 10")
HUD.create_wheel_action(25, 0, 15, 35, "HDG = HDG + MOUSE_WHEEL_CLICKS")
HUD.create_backlight_indicator( 30, 0, 10, 35, "bit.band(AP_STATE, 2) > 0", 0, 1, 0, 1)
HUD.create_click_action(0, 0, 40, 35, 'command_once("sim/autopilot/heading")')

-- SQUAWK
HUD.create_element("SQUAWK", 0, 35, 80, 35)
HUD.draw_string(5, 23, 10, "SQUAWK")
HUD.draw_fstring(27, 5, 18, "%04i", "SQUAWK")
HUD.create_wheel_action( 27, 0, 9, 35, "SQUAWK = SQUAWK + 1000 * MOUSE_WHEEL_CLICKS")
HUD.create_wheel_action( 37, 0, 9, 35, "SQUAWK = SQUAWK +  100 * MOUSE_WHEEL_CLICKS")
HUD.create_wheel_action( 47, 0, 9, 35, "SQUAWK = SQUAWK +   10 * MOUSE_WHEEL_CLICKS")
HUD.create_wheel_action( 57, 0, 9, 35, "SQUAWK = SQUAWK +        MOUSE_WHEEL_CLICKS")
HUD.create_click_switch(  0, 0, 80, 35, "TRANSPONDER_MODE", 1, 2)
HUD.create_backlight_indicator( 70, 0, 10, 35, "TRANSPONDER_MODE == 0", 0, 0, 1, 1)
HUD.create_backlight_indicator( 70, 0, 10, 35, "TRANSPONDER_MODE == 1", 1, 0, 0, 1)
HUD.create_backlight_indicator( 70, 0, 10, 35, "TRANSPONDER_MODE == 2", 0, 1, 0, 1)
HUD.create_backlight_indicator( 70, 0, 10, 35, "TRANSPONDER_MODE == 3", 1, 1, 0, 1)

-- BARO
HUD.create_element("BARO", 0, 0, 80, 35)
HUD.draw_string(5, 23, 10, "BARO")
if XPLANE_LANGUAGE == "English" then
	HUD.draw_fstring(25, 5, 18, "%2.2f", "QNH_Pilot")
else
	HUD.draw_fstring(25, 5, 18, "%4.0f", "QNH_Pilot * 33.86531")
end
HUD.create_wheel_action(0, 0, 80, 35, "QNH_Pilot = QNH_Pilot + MOUSE_WHEEL_CLICKS / 100")
HUD.create_click_action(0, 0, 80, 35, 'QNH_Pilot = 29.92')

-- finish the HUD
HUD.end_HUD()

-- define a function uesed by the HUD
function autopilot_helper_set_VVI()
    if ALT > ELEVATION*3.2808399 then
        if bit.band(AP_STATE, 16) == 0 then command_once( "sim/autopilot/vertical_speed" ) end
        VVI = autopilot_helper_vvi
        if bit.band(AP_STATE, 32) == 0 then command_once("sim/autopilot/altitude_arm") end
    end
    if ALT < ELEVATION*3.2808399 then
        if bit.band(AP_STATE, 16) == 0 then command_once( "sim/autopilot/vertical_speed" ) end
        VVI = -autopilot_helper_vvi
        if bit.band(AP_STATE, 32) == 0 then command_once("sim/autopilot/altitude_arm") end
    end
end

-- define some functions taken from another script
plane_PSI =  dataref_table("sim/flightmodel/position/psi")
autopilot_mode = dataref_table("sim/cockpit/autopilot/autopilot_mode")

function activate_autopilot_tweak()
	HDG = plane_PSI[0]
	command_once( "sim/autopilot/reentry" )
	command_once( "sim/autopilot/heading" )
	command_once( "sim/autopilot/altitude_hold" )
	ALT = ELEVATION*3.2808399
	autopilot_mode[0] = 2
end

function stop_autopilot_tweak()
	autopilot_mode[0] = 0
	xp_autothrottle_enabled = 0
end

-- provide a custom command to activate it
create_command( "FlyWithLua/autopilot/activate_autopilot", "activate autopilot and set actual heading and altitude",
                "activate_autopilot_tweak()", "", "" )

create_command( "FlyWithLua/autopilot/set_autopilot_off", "deactivate autopilot tweak and get back normal steering",
                "stop_autopilot_tweak()", "", "" )


-- do some corrections
function radio_limiter()
	if COM1 < 11800 then COM1 = 11800 end
	if COM1 > 13697 then COM1 = 13697 end
	if NAV1 < 10800 then NAV1 = 10800 end
	if NAV1 > 11797 then NAV1 = 11797 end
	if ADF1 <  530 then ADF1 =  530 end
	if ADF1 > 1700 then ADF1 = 1700 end
	if OBS1 <   0 then OBS1 = 350 end
	if OBS1 > 360 then OBS1 =  10 end
	if HDG <   0 then HDG = 350 end
	if HDG > 360 then HDG =  10 end
	local SQUAWKSTRING = string.format("%4i", SQUAWK)
	SQUAWKSTRING = string.gsub(SQUAWKSTRING, '8', '0')
	SQUAWKSTRING = string.gsub(SQUAWKSTRING, '9', '0')
	SQUAWK = tonumber(SQUAWKSTRING)
	if SQUAWK == 7500 then SQUAWK = 7550 end
	if SQUAWK < 0 then SQUAWK = 7000 end
end

do_on_mouse_wheel("radio_limiter()")
