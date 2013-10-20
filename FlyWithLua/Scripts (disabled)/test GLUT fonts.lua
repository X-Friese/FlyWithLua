-- this is a demo how to use the GLUT fonts

function print_some_text()
	-- first you have to set the color (defined by it's RGB values)
	glColor3f(1,1,1)
	
	-- then you can print
	draw_string_Helvetica_18(300, 400, "Dieser Text erscheint in Helvetica 18")
	draw_string_Helvetica_12(300, 350, "Dieser Text erscheint in Helvetica 12")
	draw_string_Helvetica_10(300, 300, "Dieser Text erscheint in Helvetica 10")
	draw_string_Times_Roman_10(300, 250, "Dieser Text erscheint in Times Roman 10")
	draw_string_Times_Roman_24(300, 200, "Dieser Text erscheint in Times Roman 24")
end

do_every_draw("print_some_text()")