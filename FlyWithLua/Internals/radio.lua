module(..., package.seeall);

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- >8 -- -- -- -- --
-- FlyWithLua radio library V1.0 build 2012-09-29
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- This library sets all basic radio DataRefs, use it to name them equal in different scripts
--
-- You will get as writable DataRefs:
--
-- COM1, COM2, COM1_STDBY, COM2_STDBY,
-- NAV1, NAV2, VAV1_STDBY, NAV2_STDBY,
-- ADF1, ADF2, ADF1_STDBY, ADF2_STDBY,
-- DME, DME_STDBY,
-- OBS1, OBS2
-- SQUAWK (0000 to 7777), TRANSPONDER_MODE (0=off, 1=standby, 2=on, 3=test)
-- HDG
--
-- Usage: Instead of defining the DataRefs, just insert a line >>require("radio")<<
-- -- -- -- 8< -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

dataref("COM1", "sim/cockpit2/radios/actuators/com1_frequency_hz", "writable")
dataref("COM2", "sim/cockpit2/radios/actuators/com2_frequency_hz", "writable")
dataref("COM1_STDBY", "sim/cockpit2/radios/actuators/com1_standby_frequency_hz", "writable")
dataref("COM2_STDBY", "sim/cockpit2/radios/actuators/com2_standby_frequency_hz", "writable")

dataref("NAV1", "sim/cockpit2/radios/actuators/nav1_frequency_hz", "writable")
dataref("NAV2", "sim/cockpit2/radios/actuators/nav2_frequency_hz", "writable")
dataref("NAV1_STDBY", "sim/cockpit2/radios/actuators/nav1_standby_frequency_hz", "writable")
dataref("NAV2_STDBY", "sim/cockpit2/radios/actuators/nav2_standby_frequency_hz", "writable")

dataref("ADF1", "sim/cockpit2/radios/actuators/adf1_frequency_hz", "writable")
dataref("ADF2", "sim/cockpit2/radios/actuators/adf2_frequency_hz", "writable")
dataref("ADF1_STDBY", "sim/cockpit2/radios/actuators/adf1_standby_frequency_hz", "writable")
dataref("ADF2_STDBY", "sim/cockpit2/radios/actuators/adf2_standby_frequency_hz", "writable")

dataref("DME", "sim/cockpit2/radios/actuators/dme_frequency_hz", "writable")
dataref("DME_STDBY", "sim/cockpit2/radios/actuators/dme_standby_frequency_hz", "writable")

dataref("OBS1", "sim/cockpit2/radios/actuators/nav1_obs_deg_mag_pilot", "writable")
dataref("OBS2", "sim/cockpit2/radios/actuators/nav2_obs_deg_mag_pilot", "writable")

dataref("SQUAWK", "sim/cockpit/radios/transponder_code", "writable")
dataref("TRANSPONDER_MODE", "sim/cockpit/radios/transponder_mode", "writable")

dataref("HDG", "sim/cockpit/autopilot/heading", "writable")
