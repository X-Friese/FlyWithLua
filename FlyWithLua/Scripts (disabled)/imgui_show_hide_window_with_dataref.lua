-- Imugi Show Hide Window With Dataref
-- William R. Good 03-22-19

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

define_shared_DataRef("FlyWithLua/test/integer1", "Int")
dataref("Test_Integer1", "FlyWithLua/test/integer1", "writable")
Test_Integer1 = 0;

local done = false

function ishwwd_on_build(ishwwd_wnd, x, y)
	imgui.TextUnformatted("This is a test window")
end

ishwwd_wnd = nil

function ishwwd_show_wnd()
	if Test_Integer1 == 1 and done == false then
	    ishwwd_wnd = float_wnd_create(640, 480, 1, true)
    	float_wnd_set_title(ishwwd_wnd, "Imgui Show Hide Window With Dataref")
    	float_wnd_set_imgui_builder(ishwwd_wnd, "ishwwd_on_build")
		done = true
	end
end

function ishwwd_hide_wnd()
	if Test_Integer1 == 0 then
	    if ishwwd_wnd then
    	    float_wnd_destroy(ishwwd_wnd)
			done = false
    	end
	end
end

do_often("ishwwd_show_wnd()")
do_often("ishwwd_hide_wnd()")
