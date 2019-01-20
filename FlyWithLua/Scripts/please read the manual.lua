-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- This is the infamous welcome message of FlyWithLua.   --
--                                                       --
-- If you don't know what to do, just delete this little --
-- script and peace will come back into your simulator.  --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --


DataRef( "sim_time", "sim/network/misc/network_time_sec")

function MyHelloWorld()
	local msgtime
	msgtime = sim_time % 3
	if msgtime < 1.0 then
		draw_string( 50, 300, "Please", "red" )
	end
	if msgtime >= 1.0 and msgtime < 2.0 then
		draw_string( 50, 300, "read", "green" )
	end
	if msgtime >= 2.0 then
		draw_string( 50, 300, "the manual!", "blue" )
	end
	draw_string( 50 + sim_time * 30 % 400, 250,
				 "If you want this message to disappear you will have to follow the bubble's instruction!",
				 sim_time - math.floor(sim_time),
				 sim_time - math.floor(sim_time + 0.3) + 0.3,
				 sim_time - math.floor(sim_time + 0.6) + 0.6 )
	big_bubble(MOUSE_X - 50, MOUSE_Y + 20,
	           'First time using FlyWithLua?',
               '',
	           'This plugin will execute all files matching "*.lua" or "*.fwl" inside',
	           'the folder "Scripts". So please decide which are usefull to you and',
	           'delete all other Lua files inside the Scripts folder.',
	           '',
	           'Or to say it more precise, delete the following file to get away this bubble:',
	           '"/Resources/plugins/FlyWithLua/Scripts/please read the manual.lua"',
	           '',
	           'If you just want to install a super duper enhancer based on FWL, you',
	           'can throw away all the content of the Scripts folder and fill in the',
	           'new scripts you want to use.',
	           '',
	           'But if you are clever, you will carefully read the manual, study all the',
	           'example scripts delivered with FlyWithLua and get access to',
	           "Pandora's box.")
end

do_every_draw('MyHelloWorld()')