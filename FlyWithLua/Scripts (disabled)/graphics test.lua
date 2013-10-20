require("graphics")

dataref("first_axis", "sim/joystick/joystick_axis_values", "readonly", 0)
dataref("second_axis", "sim/joystick/joystick_axis_values", "readonly", 1)

function draw_some_stuff()
    for i=0, 360, 30 do
        graphics.set_color(0, 0, 1)
        graphics.draw_angle_line(500, 500, i, 200)
        graphics.set_color(1, 0, 0)
        graphics.draw_angle_arrow(500, 500, i + 15, 100, 20, 2.5)
        graphics.set_color(0, 1, 0)
        graphics.draw_tick_mark(500, 500, i + 10, 200, 20, 2.5)
        graphics.draw_outer_tracer(500, 500, i + 20, 200)
        graphics.set_color(1, 1, 0)
        graphics.draw_inner_tracer(500, 500, i + 15, 200)
    end
    graphics.set_color(1, 1, 1)
    graphics.draw_circle(500, 500, 200)
    graphics.draw_circle(500, 500, 100)
    graphics.draw_filled_circle(500, 500, 10)
    
    graphics.set_color(0, 1, 0)
    graphics.draw_filled_arc(1000, 500, 20, 80, 200)
    graphics.set_color(1, 1, 0)
    graphics.draw_filled_arc(1000, 500, 60, 80, 200)
    graphics.set_color(1, 0, 0)
    graphics.draw_filled_arc(1000, 500, 75, 80, 200)
    graphics.set_color(0, 0, 0)
    graphics.draw_filled_arc(1000, 500, 20, 80, 190)
    graphics.set_color(1, 1, 1)
    graphics.draw_arc(1000, 500, 20, 80, 200)
    graphics.draw_angle_arrow(1000, 500, first_axis * 60 + 20, 200, 20, 2.5)
    graphics.draw_outer_tracer(1000, 500, second_axis * 60 + 20, 200, 15)
    graphics.draw_filled_circle(1000, 500, 5)
end

do_every_draw("draw_some_stuff()")