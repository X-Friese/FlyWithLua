module(..., package.seeall);

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- >8 -- -- -- -- --
-- FlyWithLua graphics library V1.3
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- we can use these functions, to access to OpenGL:
--
-- glBegin_POINTS()
--
-- glBegin_LINES()
--
-- glBegin_LINE_STRIP()
--
-- glBegin_LINE_LOOP()
--
-- glBegin_POLYGON()
--
-- glBegin_TRIANGLES()
--
-- glBegin_TRIANGLE_STRIP()
--
-- glBegin_TRIANGLE_FAN()
--
-- glBegin_QUADS()
--
-- glBegin_QUAD_STRIP()
--
-- glEnd()
--
-- glVertex2f(x, y)
--
-- glVertex3f(x, y, z)
--
-- glLineWidth(width)
--
-- glColor3f(red, green, blue)
--
-- glColor4f(red, green, blue, alpha)
--
-- glRectf(x1, y1, x2, y2)
--
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- If you are not familiar with OpenGL, we create some nice functions to be used in 2D window mode:
-- -- -- 8< -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

function draw_line(x1, y1, x2, y2)
	x1 = x1 or 0
	y1 = y1 or 0
	x2 = x2 or 0
	y2 = y2 or 0
	glBegin_LINES()
	glVertex2f(x1, y1)
	glVertex2f(x2, y2)
	glEnd()
end

function draw_rectangle(x1, y1, x2, y2)
	x1 = x1 or 0
	y1 = y1 or 0
	x2 = x2 or 0
	y2 = y2 or 0
    if x1 > x2 then
        x1, x2 = x2, x1
    end
    if y1 > y2 then
        y1, y2 = y2, y1
    end
	glBegin_POLYGON()
	glVertex2f(x1, y1)
	glVertex2f(x1, y2)
	glVertex2f(x2, y2)
	glVertex2f(x2, y1)
	glEnd()
end

function draw_triangle(x1, y1, x2, y2, x3, y3)
	x1 = x1 or 0
	y1 = y1 or 0
	x2 = x2 or 0
	y2 = y2 or 0
	x3 = x3 or 0
	y3 = y3 or 0
	glBegin_TRIANGLES()
	glVertex2f(x1, y1)
	glVertex2f(x2, y2)
	glVertex2f(x3, y3)
	glEnd()
end

function set_color(red, green, blue, alpha)
	red = red or 1.0
	green = green or 1.0
	blue = blue or 1.0
	alpha = alpha or 1.0
	glColor4f(red, green, blue, alpha)
end

function set_width(width)
    width = width or 1
    glLineWidth(width)
end

-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- >8 -- -- -- -- --
-- The following commands will use these type of arguments:
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- x = horizontal screen coordinate (starts left)
-- y = vertical screen coordinate (starts at bottom)
-- a = an angle in degrees, 0 points towards top, clockwise (alpha)
-- b = an angle in degrees, 0 points towards top, clockwise (beta)
-- r = radius in screen pixel
-- l = a lenght in screen pixel
-- w = line width in screen pixel
-- -- -- 8< -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

-- calculate a point, when a starting point plus angle and lenght are given
-- usage: new_x, new_y = graphics.move( start_x, start_y, angle, lenght )
function move_angle(x,y,a,l)
    local tx
    local ty
    local arad
    arad = (-a+90)*math.pi/180
    tx = x + l * math.cos(arad)
    ty = y + l * math.sin(arad)
    return tx, ty
end

function draw_angle_line(x, y, a, l, w)
    w = w or 1
    local x2
    local y2
    glLineWidth(w)
    x2, y2 = move_angle(x, y, a, l)
    draw_line(x, y, x2, y2)
end

function draw_angle_arrow(x, y, a, r, l, w)
    l = l or 7.5
    w = w or 1
    local x1
    local y1
    local x2
    local y2
    local x3
    local y3
    glLineWidth(w)
    x1, y1 = move_angle(x, y, a, r - w)
    draw_line(x , y, x1, y1)
    x1, y1 = move_angle(x, y, a, r)
    x2, y2 = move_angle(x1, y1, a - 200, l)
    x3, y3 = move_angle(x1, y1, a - 160, l)
    draw_triangle(x1, y1, x2, y2, x3, y3)
end

function draw_circle(x, y, r, w)
    w = w or 1
    local x1
    local y1
    local step
    glLineWidth(w)
    if r > 50 then
        step = 750 / r
    else
        step = 15
    end
    glBegin_LINE_LOOP()
    for i = 0, 360, step do
        x1, y1 = move_angle(x, y, i, r)
        glVertex2f(x1, y1)
    end
    glEnd()
end

function draw_filled_circle(x, y, r)
    local x1
    local y1
    local step
    if r > 50 then
        step = 750 / r
    else
        step = 15
    end
    glBegin_POLYGON()
    for i = 0, 360, step do
        x1, y1 = move_angle(x, y, i, r)
        glVertex2f(x1, y1)
    end
    glEnd()
end

function draw_arc_line(x, y, a1, a2, r, w)
    w = w or 1
    local x1
    local y1
    local x2
    local y2
    local step
    glLineWidth(w)
    if r > 50 then
        step = 750 / r
    else
        step = 15
    end
    glBegin_LINES()
    for i = a1, a2 - step, step do
        x1, y1 = move_angle(x, y, i, r)
        glVertex2f(x1, y1)
        x2, y2 = move_angle(x, y, i + step, r)
        glVertex2f(x2, y2)
    end
    glEnd()
end

function draw_arc(x, y, a1, a2, r, w)
    w = w or 1
    local x1
    local y1
    local step
    glLineWidth(w)
    if r > 50 then
        step = 750 / r
    else
        step = 15
    end
    glBegin_LINE_LOOP()
    glVertex2f(x, y)
    for i = a1, a2, step do
        x1, y1 = move_angle(x, y, i, r)
        glVertex2f(x1, y1)
    end
    x1, y1 = move_angle(x, y, a2, r)
    glVertex2f(x1, y1)
    glEnd()
end

function draw_filled_arc(x, y, a1, a2, r)
    local x1
    local y1
    local step
    if r > 50 then
        step = 750 / r
    else
        step = 15
    end
    glBegin_POLYGON()
    glVertex2f(x, y)
    for i = a1, a2, step do
        x1, y1 = move_angle(x, y, i, r)
        glVertex2f(x1, y1)
    end
    x1, y1 = move_angle(x, y, a2, r)
    glVertex2f(x1, y1)
    glEnd()
end

function draw_tick_mark(x, y, a, r, l, w)
    l = l or 10
    w = w or 1
    local x1
    local y1
    local x2
    local y2
    x1, y1 = move_angle(x, y, a, r - l)
    x2, y2 = move_angle(x, y, a, r)
    glLineWidth(w)
    draw_line(x1, y1, x2, y2)
end

function draw_outer_tracer(x, y, a, r, l)
    l = l or 7.5
    local x1
    local y1
    local x2
    local y2
    local x3
    local y3
    x1, y1 = move_angle(x, y, a, r)
    x2, y2 = move_angle(x1, y1, a - 30, l)
    x3, y3 = move_angle(x1, y1, a + 30, l)
    draw_triangle(x1, y1, x2, y2, x3, y3)
end

function draw_inner_tracer(x, y, a, r, l)
    l = l or 7.5
    local x1
    local y1
    local x2
    local y2
    local x3
    local y3
    x1, y1 = move_angle(x, y, a, r)
    x2, y2 = move_angle(x1, y1, a - 210, l)
    x3, y3 = move_angle(x1, y1, a - 150, l)
    draw_triangle(x1, y1, x2, y2, x3, y3)
end
