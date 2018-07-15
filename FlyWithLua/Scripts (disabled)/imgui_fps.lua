-- Show FPS in imgui
-- Folke Will 2018-07-13
dataref("frame_period", "sim/operation/misc/frame_rate_period")

buffer = {}
histogram = {}

function update_histogram()
    local cur_period = frame_period
    if cur_period > 0 then
        fps = math.floor(1.0 / frame_period)
        for i = #histogram, fps do
            histogram[#histogram + 1] = 0
        end
        histogram[fps] = histogram[fps] + 1
    end
end

function update_buffer()
    local cur_period = frame_period
    if cur_period > 0 then
        if #buffer > 50 then
            table.remove(buffer, 1)
        end
        buffer[#buffer + 1] = 1 / cur_period
    end
end

function build_fps(wnd, x, y)
    w, h = float_wnd_get_dimensions(wnd)
    
    if imgui.Button("Clear Histogram") then
        histogram = {}
    end

    imgui.PlotLines("", buffer, #buffer, 0, "FPS", 0, 150, w, h / 2 - 10)
    imgui.PlotHistogram("FPS", histogram, #histogram, 0, "Histogram", FLT_MAX, FLT_MAX, w, h / 2 - 40)
end

fps_wnd = float_wnd_create(500, 375, 1, true)
float_wnd_set_title(fps_wnd, "FlyWithFPS")
float_wnd_set_imgui_builder(fps_wnd, "build_fps")

do_every_frame("update_histogram()")
do_often("update_buffer()")
