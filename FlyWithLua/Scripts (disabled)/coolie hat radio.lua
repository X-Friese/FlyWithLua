-- coolie hat radio script
-- turn in frequency using a coolie hat

-- -- -- 8< -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
local chrs_flip_button = 15
local chrs_change_button = 14
local chrs_coolie_head_up = 16
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- >8 -- -- --

-- define the complete coolie hat
local chrs_coolie_head_right = chrs_coolie_head_up + 2
local chrs_coolie_head_down = chrs_coolie_head_up + 4
local chrs_coolie_head_left = chrs_coolie_head_up + 6

-- the global variable showing the state
chrs_state = "COM1"

-- init the coolie hat
set_button_assignment(chrs_coolie_head_up, "sim/radios/stby_com1_fine_up")
set_button_assignment(chrs_coolie_head_right, "sim/radios/stby_com1_coarse_down")
set_button_assignment(chrs_coolie_head_down, "sim/radios/stby_com1_fine_down")
set_button_assignment(chrs_coolie_head_left, "sim/radios/stby_com1_coarse_up")

-- init the flip button (flipping standby and active)
set_button_assignment(chrs_flip_button, "sim/radios/com1_standy_flip")

function chrs_change_target()
	if chrs_state == "COM1" then
		chrs_state = "COM2"
		set_button_assignment(chrs_coolie_head_up, "sim/radios/stby_com2_fine_up")
		set_button_assignment(chrs_coolie_head_right, "sim/radios/stby_com2_coarse_down")
		set_button_assignment(chrs_coolie_head_down, "sim/radios/stby_com2_fine_down")
		set_button_assignment(chrs_coolie_head_left, "sim/radios/stby_com2_coarse_up")
		set_button_assignment(chrs_flip_button, "sim/radios/com2_standy_flip")
		return
	end
	if chrs_state == "COM2" then
		chrs_state = "NAV1"
		set_button_assignment(chrs_coolie_head_up, "sim/radios/stby_nav1_fine_up")
		set_button_assignment(chrs_coolie_head_right, "sim/radios/stby_nav1_coarse_down")
		set_button_assignment(chrs_coolie_head_down, "sim/radios/stby_nav1_fine_down")
		set_button_assignment(chrs_coolie_head_left, "sim/radios/stby_nav1_coarse_up")
		set_button_assignment(chrs_flip_button, "sim/radios/nav1_standy_flip")
		return
	end
	if chrs_state == "NAV1" then
		chrs_state = "NAV2"
		set_button_assignment(chrs_coolie_head_up, "sim/radios/stby_nav2_fine_up")
		set_button_assignment(chrs_coolie_head_right, "sim/radios/stby_nav2_coarse_down")
		set_button_assignment(chrs_coolie_head_down, "sim/radios/stby_nav2_fine_down")
		set_button_assignment(chrs_coolie_head_left, "sim/radios/stby_nav2_coarse_up")
		set_button_assignment(chrs_flip_button, "sim/radios/nav2_standy_flip")
		return
	end
	if chrs_state == "NAV2" then
		chrs_state = "COM1"
		set_button_assignment(chrs_coolie_head_up, "sim/radios/stby_com1_fine_up")
		set_button_assignment(chrs_coolie_head_right, "sim/radios/stby_com1_coarse_down")
		set_button_assignment(chrs_coolie_head_down, "sim/radios/stby_com1_fine_down")
		set_button_assignment(chrs_coolie_head_left, "sim/radios/stby_com1_coarse_up")
		set_button_assignment(chrs_flip_button, "sim/radios/com1_standy_flip")
		return
	end
end

create_command("FlyWithLua/coolie_hat/change_state", "change the coolie hat target", "chrs_change_target()", "", "")
set_button_assignment(chrs_change_button, "FlyWithLua/coolie_hat/change_state")

-- display a short info
do_every_draw([[draw_string(30, 10, chrs_state, "magenta")]])
