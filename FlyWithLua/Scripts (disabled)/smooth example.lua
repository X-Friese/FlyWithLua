require("SMA_smoother")

SMA_smoother.create_SMA(10, 5)

create_axis_median(10, "median_axis_10")

-- show the axis
do_every_draw('draw_string(1500 * real_axis_10, 20, "hardware value", "red")')

-- show the smooth value
do_every_draw('draw_string(1500 * axis_10, 40, "smoothed value", "green")')

-- show the median value
do_every_draw('draw_string(1500 * median_axis_10, 60, "median value", "yellow")')
