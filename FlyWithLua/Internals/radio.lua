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

dataref("COM1", "sim/cockpit/radios/com1_freq_hz", "writable")
dataref("COM2", "sim/cockpit/radios/com2_freq_hz", "writable")
dataref("COM1_STDBY", "sim/cockpit/radios/com1_stdby_freq_hz", "writable")
dataref("COM2_STDBY", "sim/cockpit/radios/com2_stdby_freq_hz", "writable")

dataref("NAV1", "sim/cockpit/radios/nav1_freq_hz", "writable")
dataref("NAV2", "sim/cockpit/radios/nav2_freq_hz", "writable")
dataref("NAV1_STDBY", "sim/cockpit/radios/nav1_stdby_freq_hz", "writable")
dataref("NAV2_STDBY", "sim/cockpit/radios/nav2_stdby_freq_hz", "writable")

dataref("ADF1", "sim/cockpit/radios/adf1_freq_hz", "writable")
dataref("ADF2", "sim/cockpit/radios/adf2_freq_hz", "writable")
dataref("ADF1_STDBY", "sim/cockpit/radios/adf1_stdby_freq_hz", "writable")
dataref("ADF2_STDBY", "sim/cockpit/radios/adf2_stdby_freq_hz", "writable")

dataref("DME", "sim/cockpit/radios/dme_freq_hz", "writable")
dataref("DME_STDBY", "sim/cockpit/radios/dme_stdby_freq_hz", "writable")

dataref("OBS1", "sim/cockpit/radios/nav1_obs_degt", "writable")
dataref("OBS2", "sim/cockpit/radios/nav2_obs_degt", "writable")

dataref("SQUAWK", "sim/cockpit/radios/transponder_code", "writable")
dataref("TRANSPONDER_MODE", "sim/cockpit/radios/transponder_mode", "writable")

dataref("HDG", "sim/cockpit/autopilot/heading", "writable")
