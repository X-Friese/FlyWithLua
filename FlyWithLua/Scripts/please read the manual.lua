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
				 "If you want this message to disappear you will have to read the manual!",
				 sim_time - math.floor(sim_time),
				 sim_time - math.floor(sim_time + 0.3) + 0.3,
				 sim_time - math.floor(sim_time + 0.6) + 0.6 )
end

do_every_draw('MyHelloWorld()')