module(..., package.seeall);

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- >8 -- -- -- -- --
-- FlyWithLua XSquawkBox library V1.0 build 2012-10-03
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- This library sets all basic XSB DataRefs, use it to name them equal in different scripts
--
-- For more info how to use them, please read the official XSquawkBox developer manual.
--
-- Usage: Instead of defining the DataRefs, just insert a line >>require("XSquawkBox")<<
-- -- -- -- 8< -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

-- info about XSB
dataref("XSB_VERS_NUMBER",		"xsquawkbox/vers/number", "readonly")		-- int,  100 = 1.0
dataref("XSB_VERS_STRING",		"xsquawkbox/vers/string", "readonly", 0)	-- string, e.g. 1.0b4

-- DataRefs used for connecting to the VATSIM server
dataref("XSB_CON_CALLSIGN",		"xsquawkbox/login/callsign", "writable", 0)	-- string
dataref("XSB_CON_SERVER",		"xsquawkbox/login/server", "writable", 0)	-- string (DNS address or IP as string
dataref("XSB_CON_PORT",			"xsquawkbox/login/port", "writable")		-- int
dataref("XSB_CON_PILOT_ID",		"xsquawkbox/login/pilot_id", "writable", 0)	-- string
dataref("XSB_CON_PASSWORD",		"xsquawkbox/login/password", "writable", 0)	-- string
dataref("XSB_CON_REALNAME",		"xsquawkbox/login/realname", "writable", 0)	-- string
dataref("XSB_CON_MODEL",		"xsquawkbox/login/model", "writable", 0)	-- string
dataref("XSB_CON_STATUS",		"xsquawkbox/login/status", "readonly")		-- int, 0 = disconnected, 1 = connecting, 2 = connected, NOT WRITABLE

-- your flightplan, remember that an integer is not a char!
dataref("XSB_FP_FLIGHT_TYPE",			"xsquawkbox/fp/flight_type", "writable")			-- int, either I, V, D, or S
dataref("XSB_FP_TCAS_TYPE",				"xsquawkbox/fp/tcas_type", "writable")				-- int, T, H, B, L, or 0
dataref("XSB_FP_NAV_TYPE",				"xsquawkbox/fp/nav_type", "writable")				-- int, F, G, R, A, etc.
dataref("XSB_FP_SPEED",					"xsquawkbox/fp/speed", "writable")					-- int, knots
dataref("XSB_FP_DEPARTURE_AIRPORT",		"xsquawkbox/fp/departure_airport", "writable", 0)	-- string, ICAO code
dataref("XSB_FP_DEPARTURE_TIME",		"xsquawkbox/fp/departure_time", "writable")			-- int, zulu time, e.g. 1340 = 13:40
dataref("XSB_FP_DEPARTURE_TIME_ACTUAL",	"xsquawkbox/fp/departure_time_actual", "writable")	-- int, zulu time
dataref("XSB_FP_CRUISE_ALTITUDE",		"xsquawkbox/fp/cruise_altitude", "writable", 0)		-- string, either FLxxx or yyyyy
dataref("XSB_FP_ARRIVAL_AIRPORT",		"xsquawkbox/fp/arrival_airport", "writable", 0)		-- string, ICAO code
dataref("XSB_FP_ENROUTE_HRS",			"xsquawkbox/fp/enroute_hrs", "writable")			-- int, hrs
dataref("XSB_FP_ENROUTE_MINS",			"xsquawkbox/fp/enroute_mins", "writable")			-- int, mins
dataref("XSB_FP_FUEL_HRS",				"xsquawkbox/fp/fuel_hrs", "writable")				-- int, hrs
dataref("XSB_FP_FUEL_MINS",				"xsquawkbox/fp/fuel_mins", "writable")				-- int, mins
dataref("XSB_FP_ALTERNATE_AIRPORT",		"xsquawkbox/fp/alternate_airport", "writable", 0)	-- string, ICAO code
dataref("XSB_FP_REMARKS",				"xsquawkbox/fp/remarks", "writable", 0)				-- string
dataref("XSB_FP_ROUTE",					"xsquawkbox/fp/route", "writable", 0)				-- string
dataref("XSB_MIC_OPEN",			        "xsquawkbox/mic/open", "readonly")					-- int, 1 if the mic is open right now, NOT WRITABLE
dataref("XSB_MIC_ENABLED",			    "xsquawkbox/mic/enabled", "writable")				-- int, 0 to mute mic, 1 to let XSB decide, 2 to force open
