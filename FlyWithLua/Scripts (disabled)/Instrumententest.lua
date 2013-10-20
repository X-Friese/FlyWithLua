require("instrument")

show_MPPS_instrument = false

dataref("xp_view_is_external", "sim/graphics/view/view_is_external")

function show_a_little_instrument()
    if xp_view_is_external > 0 and show_MPPS_instrument then
        instrument.manifold_pressure_and_propeller_speed(SCREEN_WIDTH - 210, SCREEN_HIGHT - 560, 25, 2500)
    end
end

do_every_draw("show_a_little_instrument()")