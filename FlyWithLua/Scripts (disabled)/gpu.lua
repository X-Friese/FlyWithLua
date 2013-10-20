-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Script:  ground power unit on/off
-- Version: 1.0
-- Build:   2012-10-20
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
-- Description:
-- Provide a macro to turn on/off the GPU. And show if the GPU is connected.
-- Usable in all planes, no matter if it has a GPU connector or not.
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --

-- we need one DataRef
dataref("GPU", "sim/cockpit/electrical/gpu_on", "writable")

-- provide a macro to turn the GPU on/off, do not touch the actual state of the GPU
if GPU == 0 then
    add_macro("Ground Power Unit", "GPU = 1", "GPU = 0", "deactivate")
else
    add_macro("Ground Power Unit", "GPU = 1", "GPU = 0", "activate")
end

-- show if the GPU is running
function show_if_gpu_is_running()
    if GPU == 1 then
        draw_string(SCREEN_WIDTH/2-50, SCREEN_HIGHT-40, "GPU connected!", (math.sin(os.clock()*5)+1)/2, 0, 0)
    end
end

-- register info function
do_every_draw("show_if_gpu_is_running()")
