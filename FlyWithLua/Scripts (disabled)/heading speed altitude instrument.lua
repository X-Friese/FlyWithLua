-- Heading Altitude and Speed Instrument (HASI)
-- V1.2 by Carsten Lynker
-- build 2015-02-01

require("graphics")

-- these datarefs are variable during flight
dataref("xp_groundspeed", "sim/flightmodel/position/groundspeed")
dataref("xp_psi", "sim/flightmodel/position/psi")
dataref("xp_y_agl", "sim/flightmodel/position/y_agl")
dataref("xp_h_ind2", "sim/flightmodel/misc/h_ind2")
dataref("xp_indicated_airspeed", "sim/flightmodel/position/indicated_airspeed")
dataref("xp_hpath", "sim/flightmodel/position/hpath")
dataref("xp_wind_direction_degt", "sim/weather/wind_direction_degt")
dataref("xp_wind_speed_kt", "sim/weather/wind_speed_kt")
dataref("xp_vh_ind_fpm2", "sim/flightmodel/position/vh_ind_fpm2")
dataref("xp_turn_rate_roll_deg_pilot", "sim/cockpit2/gauges/indicators/turn_rate_roll_deg_pilot")
dataref("xp_flap1_deploy_ratio", "sim/flightmodel2/controls/flap1_deploy_ratio")
dataref("xp_autopilot_mode", "sim/cockpit/autopilot/autopilot_mode", "writable")
dataref("ro_altitude", "sim/cockpit/autopilot/altitude")
dataref("ro_heading", "sim/cockpit/autopilot/heading")
dataref("xp_view_is_external", "sim/graphics/view/view_is_external")

-- these datarefs won't change until you change the aircraft (that forces a script reload)
-- for a better performance, we will use them as constants
xp_acf_Vne = get("sim/aircraft/view/acf_Vne")
xp_acf_Vno = get("sim/aircraft/view/acf_Vno")
xp_acf_Vfe = get("sim/aircraft/view/acf_Vfe")
xp_acf_Vs = get("sim/aircraft/view/acf_Vs")
xp_acf_Vso = get("sim/aircraft/view/acf_Vso")

if xp_acf_Vfe > xp_acf_Vne then
    xp_acf_Vfe = xp_acf_Vne
end

always_show_HASI = false

function show_hasi(x, y, r, ms, colored)
    -- x, y = lower left corner
    -- r = radius
    -- ms = maximum speed to display

    local i           -- increment to for
    local x1          -- a screen coordinate
    local y1          -- a screen coordinate
    local ra          -- radio altitude
    local alt         -- altitude
    local gl          -- ground level
    
    -- set default values
    x = x or 30
    y = y or (SCREEN_HIGHT - 280)
    r = r or 125
    ms = ms or 150
    colored = colored or "yes"
    
    -- force max speed to Vne + 20 if lower
    if ms < xp_acf_Vne + 20 then
        ms = xp_acf_Vne + 20
    end
    
    -- choose the state we need
    XPLMSetGraphicsState(0, 0, 0, 1, 1, 0, 0)
    
    -- draw the black base circle
    glColor3f(0,0,0)
    graphics.draw_filled_circle(x+r, y+r, r+20)
    
    -- draw groundlevel indicator
    gl = (ELEVATION - xp_y_agl) * 0.32808399
    if gl < 360 then
        -- Saddle Brown	= (139, 69, 19) = #8b4513 = (0.55, 0.27, 0.07)
        glColor3f(0.55, 0.27, 0.07)
        graphics.draw_filled_arc(x+r, y+r, 0, gl, r+10)
        glColor3f(0, 0, 0)
        graphics.draw_filled_arc(x+r, y+r, 0, gl, r)
    end
    
    -- draw vvi line
    glColor3f(1, 1, 0)
    if xp_vh_ind_fpm2 > 0 then
        graphics.draw_filled_arc(x+r, y+r, xp_h_ind2*0.1, (xp_h_ind2 + xp_vh_ind_fpm2)*0.1, r+5)
        glColor3f(0, 0, 0)
        graphics.draw_filled_arc(x+r, y+r, xp_h_ind2*0.1, (xp_h_ind2 + xp_vh_ind_fpm2)*0.1, r)
    else
        graphics.draw_filled_arc(x+r, y+r, (xp_h_ind2 + xp_vh_ind_fpm2)*0.1, xp_h_ind2*0.1, r+5)
        glColor3f(0, 0, 0)
        graphics.draw_filled_arc(x+r, y+r, (xp_h_ind2 + xp_vh_ind_fpm2)*0.1, xp_h_ind2*0.1, r)
    end
    
    -- draw turn rate line
    glColor3f(1, 1, 1)
    if xp_turn_rate_roll_deg_pilot > 0 then
        graphics.draw_filled_arc(x+r, y+r, xp_psi, (xp_psi + xp_turn_rate_roll_deg_pilot), r)
        glColor3f(0, 0, 0)
        graphics.draw_filled_arc(x+r, y+r, xp_psi, (xp_psi + xp_turn_rate_roll_deg_pilot), r-4)
    else
        graphics.draw_filled_arc(x+r, y+r, (xp_psi + xp_turn_rate_roll_deg_pilot), xp_psi, r)
        glColor3f(0, 0, 0)
        graphics.draw_filled_arc(x+r, y+r, (xp_psi + xp_turn_rate_roll_deg_pilot), xp_psi, r-4)
    end
    
    -- color the speed limit areas
    if colored == "yes" then
        glColor4f(0.9, 0.75, 0, 1)
        graphics.draw_filled_circle(x+r, y+r, xp_acf_Vne/ms*r)
        glColor4f(0, 0.5, 0, 1)
        graphics.draw_filled_circle(x+r, y+r, xp_acf_Vno/ms*r)
        glColor4f(1, 1, 1, 0.5)
        graphics.draw_filled_circle(x+r, y+r, xp_acf_Vfe/ms*r)
        glColor4f(1, 1, 1, 1)
        graphics.draw_filled_circle(x+r, y+r, xp_acf_Vs/ms*r)
        glColor4f(0, 0, 0, 1)
        graphics.draw_filled_circle(x+r, y+r, xp_acf_Vso/ms*r)
    end
        
    -- draw heading lines
    glColor3f(0.25, 0.25, 0.25)
    for i=0, 330, 30 do
        graphics.draw_angle_line(x+r, y+r, i, r)
    end
    
    -- draw speed circles
    for i=10, ms, 10 do
        glColor3f(0.25, 0.25, 0.25)
        graphics.draw_circle(x+r, y+r, i/ms*r)
    end
    for i=50, ms, 50 do
        glColor3f(1, 1, 1)
        graphics.draw_circle(x+r, y+r, i/ms*r, 1.5)
    end
    
    -- draw the red Vne circle
    if xp_flap1_deploy_ratio == 0.0 then
        glColor4f(1, 0, 0, 1)
        graphics.draw_circle(x+r, y+r, xp_acf_Vne/ms*r, 2)
    else
        glColor4f(math.abs(math.sin(os.clock()*5)), 0, 0, 1)
        graphics.draw_circle(x+r, y+r, xp_acf_Vfe/ms*r, 3)
        glColor4f(1, 1, 1, 0.25)
        graphics.draw_filled_arc(x+1.5*r+20, y+2*r+15, 90, 135, r*0.5)
        glColor4f(1, 1, 1, 0.75)
        graphics.draw_filled_arc(x+1.5*r+20, y+2*r+15, 90, 90 + 45*xp_flap1_deploy_ratio, r*0.5)        
    end
    
    if colored ~= "yes" then
        -- draw the yello Vno circle
        glColor4f(1, 1, 0, 1)
        graphics.draw_circle(x+r, y+r, xp_acf_Vno/ms*r, 2)
        -- draw the green Vs circle
        glColor4f(0, 1, 0, 1)
        graphics.draw_circle(x+r, y+r, xp_acf_Vs/ms*r, 2)
    end
    
    -- draw the tick marks
    for i = 0, 350, 10 do
        glColor3f(1, 1, 1)
        graphics.draw_tick_mark(x+r, y+r, i, r)
    end
    
    -- draw wind direction and speed at plane's position
    glColor3f(0, 0, 1)
    x1, y1 = graphics.move_angle(x+r, y+r, xp_psi, xp_indicated_airspeed/ms*r)
    graphics.draw_angle_arrow(x1, y1, xp_wind_direction_degt-180, xp_wind_speed_kt/ms*r*1.94384449, 10, 2)
    graphics.draw_inner_tracer(x+r, y+r, xp_wind_direction_degt-180, r, 12.5)
    
    -- draw ground heading and groundspeed
    glColor3f(1, 0, 0)
    graphics.draw_angle_arrow(x+r, y+r, xp_hpath, xp_groundspeed/ms*r*1.94384449, 10, 2)
    graphics.draw_inner_tracer(x+r, y+r, xp_hpath, r, 12.5)
    
    -- draw true heading and indicated speed
    glColor3f(1, 1, 1)
    graphics.draw_angle_arrow(x+r, y+r, xp_psi, xp_indicated_airspeed/ms*r, 12.5, 2.5)
    graphics.draw_inner_tracer(x+r, y+r, xp_psi, r, 12.5)

    -- draw indicated altitude
    alt = xp_h_ind2 * 0.1
    if alt < 360 then
        glColor3f(1, 1, 0)
        graphics.draw_outer_tracer(x+r, y+r, alt, r, 20)
    end    
    
    -- draw radio altitude
    ra = xp_y_agl * 3.2808399
    if ra < 360 then
        glColor3f(1, 0, 0)
        graphics.draw_outer_tracer(x+r, y+r, ra, r, 20)
    end
        
    -- draw autopilot info
    if xp_autopilot_mode == 2 then
        glColor3f(math.abs(math.sin(os.clock()*2)), 0, math.abs(math.sin(os.clock()*2)))
    else
        glColor3f(1, 0, 1)
    end
    graphics.draw_tick_mark(x+r, y+r, ro_heading, r, 15, 3)
    if ro_altitude < 3600 then
        graphics.draw_tick_mark(x+r, y+r, ro_altitude*0.1, r+15, 15, 3)
    end
    
    -- draw some text
    for i=0, 33, 3 do
        x1, y1 = graphics.move_angle(x+r, y+r, i*10, r+10)
        draw_string(x1-5, y1-3, i)
    end

    -- choose the state we need
    XPLMSetGraphicsState(0, 0, 0, 1, 1, 0, 0)
    
    -- draw the final white circle
    glColor4f(1, 1, 1, 1)
    graphics.draw_circle(x+r, y+r, r, 1.5)
end

create_command("FlyWithLua/cockpit/toggle_HASI", "toggle visibility of HASI in cockpit view", "alway_show_HASI = not alway_show_HASI", "", "")

do_every_draw('if (xp_view_is_external > 0) or alway_show_HASI then show_hasi(SCREEN_WIDTH-330, SCREEN_HIGHT-340, 150) end')