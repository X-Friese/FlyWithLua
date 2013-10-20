-- this is only a test script to see mouse action

local mouse_wheel = 0
local mouse_no = 0
local mouse_status = "unknown"

function mouse_wheel_handler()
	mouse_wheel = MOUSE_WHEEL_CLICKS
	mouse_no = MOUSE_WHEEL_NUMBER
	print("You move your wheel " .. MOUSE_WHEEL_CLICKS .. " clicks.")
end

function mouse_click_handler()
	mouse_status = MOUSE_STATUS
	mouse_wheel = 0
	mouse_no = 0
	if MOUSE_STATUS == "down" then
		print("The mouse is down!")
	end
end

function draw_wheel_info()
	draw_string(150, 150, "mouse wheel no. " .. mouse_no .. " reports " .. mouse_wheel .. " while mouse is " .. mouse_status)
end

do_on_mouse_wheel("mouse_wheel_handler()")
do_on_mouse_click("mouse_click_handler()")
do_every_draw("draw_wheel_info()")