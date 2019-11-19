-------------------------------
--[[
Example of use for XPLMFindCommand.
  Some addons are creating custom commands/datarefs while they load.
  This could cause issues if you use a FWL script that requires these inputs.
  Since the FWL script also runs during load you may find that the command/dataref you want does not exist yet.
  This is an example of how you could create a custom delay based on their creation.
  In this example we get a message on the display as long as the command does not exist.

If you wanted to use this to map joysticks dynamically:

  You must load xplane once and go to the joystick setting once for these to be associated to Laminar's map.
  For mapping you are interested in these arrays (found in the prf here: "<XP11Folder>\Output\preferences\X-Plane Joystick Settings.prf")

  Axis: _joy_AXIS_use<i> <f>
    where <i> = 0..499
            Each joystick identified by xplane will have 25 axis reserved meaning that each joystick will be starting at a 25 axis offset.
            In turn this means that XP11 has a limit of 20 usb input devices.
          <f> = function to map to axis
            It's a number so you'll have to go back and forth with XP11 to figure out what value = which axis.
            For example, roll pitch yaw are 1,2 and 3 (not sure about the order however)
            0 = not used.

  Buttons: _joy_BUTN_use<i> <command>
    where <i> = 0..3199
            Each joystick identified by xplane will have 160 buttons reserved meaning that each joystick will be starting at a 160 button offset.
            More on that is that the order of the button blocks will match the order of the joystick blocks.
            This is usefull because if you've identified that the joystick you are trying to map with FWL is using axis 75-76-77 then you know it's buttons will start at 480.
            Buttons are a little weirder than joysticks and sometimes it will skip. If your joystick has 10 buttons they may not be in the first 10 of the button block.
            They only ceertainty is that they will be within the 160 block situated at 0 + 160x<joystick number>.
          <command> = command name
            This is a string so this makes it quite easy to find.
            Set your button to something unique in XP11 then reload the prf file in your favorite text editor.
            Found the line and copy the string into a set_button_assignment. (An example is provided below).
--]]
------------------------------

loop_number = 0
test_value = 0
check = 0
check2 = 0

function delay_creating_comands()
	create_command("FlyWithLua/testing/increase",
               "This command increases test_value.",
               "test_value = test_value + 1",
               "",
               "")

	create_command("FlyWithLua/testing/decrease",
               "This command decreases test_value.",
               "test_value = test_value - 1",
               "",
               "")
	check2 = 1

end



function check_if_command_exists()

	if loop_number < 1005 then
		if loop_number == 1000 then
			check = 1
			delay_creating_comands()
			loop_number = 1010
		end
		loop_number = loop_number + 1
	end

	if XPLMFindCommand("FlyWithLua/testing/increase") == nil then
		draw_string_Helvetica_18(50, 500, "Waiting on increase command to be created!  " .. loop_number)
	else
		draw_string_Helvetica_18(50, 500, "Increase command has been created!  " .. loop_number)
	end

	if XPLMFindCommand("FlyWithLua/testing/decrease") == nil then
		draw_string_Helvetica_18(50, 520, "Waiting on decrease command to be created!  " .. loop_number)
	else
		draw_string_Helvetica_18(50, 520, "Decrease command has been created!  " .. loop_number)
	end
	draw_string_Helvetica_18(50, 540, "Have we tried to create the commands?  " .. check .. "   " .. check2)

end


do_every_draw("check_if_command_exists()")


