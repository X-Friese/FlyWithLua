require("SMA_smoother")

SMA_smoother.create_SMA(12, 10)

-- show the axis
do_every_draw('draw_string(1500 * real_axis_12, 20, "hardware value", "red")')

-- show the smooth value
do_every_draw('draw_string(1500 * axis_12, 40, "smoothed value", "green")')
