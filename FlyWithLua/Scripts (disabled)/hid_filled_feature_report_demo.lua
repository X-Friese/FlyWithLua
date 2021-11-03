-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- hid_filled_feature_report_demo
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Included as a test case to verify that https://github.com/X-Friese/FlyWithLua/pull/78 works,
-- but also serves as a demo of sending feature reports to an HID device, in this case toggling all LEDs
-- of a Honeycomb Bravo throttle quadrant, but other devices should only differ in the structure
-- and length of the data being sent to them in the feature report.
--
-- Please note that on some systems, the default permissions don't allow FlyWithLua to work with certain
-- HID devices. If this script fails to find a device with hid_open, this might be the problem. On Linux,
-- this can be fixed by creating an appropriate udev rule, if you run into this problem on macOS
-- or Windows and find a solution, feel free to add it to this description.
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

local bitwise = require "bit"

-- Let's connect to our Honeycomb Bravo throttle quadrant
local bravo = hid_open(10571, 6401)

if bravo == nil then
	-- If we can't find one, bail out
	logMsg("Unable to connect to the Honeycomb Bravo throttle quadrant")
else
	-- We found our device, let's prepare the data structure and the functions to manipulate it
	local buffer = {}

	-- Send our local data structure to the device in the form of a HID feature report
	function send_hid_data()
		local data = {}

		-- We take the status info for our LEDs and convert it into a set of ints, each LED corresponds to a single bit
		for bank = 1, 4 do
			data[bank] = 0

			for bit = 1, 8 do
				if buffer[bank][bit] == true then
					data[bank] = bitwise.bor(data[bank], bitwise.lshift(1, bit - 1))
				end
			end
		end

		-- We send our feature report filled up to 65 bytes in length (1 byte with the report ID and 64 bytes of data), which is what the official Windows test application does
		local bytes_written = hid_send_filled_feature_report(bravo, 0, 65, data[1], data[2], data[3], data[4])

		-- Check if the sending failed, if so, let's complain
		if bytes_written == -1 then
			logMsg("Feature report send failed, an error occured")
		elseif bytes_written < 65 then
			logMsg("Feature report send failed, only "..bytes_written.." bytes written")
		end
	end

	-- Set all LEDs to on or off based on the state parameter
	function set_all_leds(state)
		for bank = 1, 4 do
			buffer[bank] = {}
			for bit = 1, 8 do
				-- We only have 28 LEDs in total, the rest is always set to off
				if bank < 4 or bit < 5 then
					buffer[bank][bit] = state
				else
					buffer[bank][bit] = false
				end
			end
		end

		send_hid_data()
	end

	-- We're ready, let's setup our macro and turn all LEDs off on startup and exit
	add_macro("Turn all Honeycomb Bravo LEDs on", "set_all_leds(true)", "set_all_leds(false)", "deactivate")

	set_all_leds(false)
	do_on_exit("set_all_leds(false)")
end
