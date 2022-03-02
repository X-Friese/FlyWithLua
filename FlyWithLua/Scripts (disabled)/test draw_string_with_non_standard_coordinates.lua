-- this is a test pf draw_string with non standard coordinates

function draw_string_test()
	-- first you have to set the color (defined by it's RGB values)
	glColor3f(0,1,0)
	
	-- then you can print
	draw_string_Helvetica_18(100.0, 100.0, "TEST1")
	draw_string_Helvetica_18(200.5, 100.0, "TEST2")
	draw_string_Helvetica_18(100.0, 150.5, "TEST3")
	
	glColor3f(1,1,0)
	
	draw_string_Helvetica_18(300.0, 100.0, "TEST4")
	draw_string_Helvetica_18(400.5, 100.0, "TEST5")
	draw_string_Helvetica_18(200.0, 150.5, "TEST6")
	
	
	
end

do_every_draw("draw_string_test()")
