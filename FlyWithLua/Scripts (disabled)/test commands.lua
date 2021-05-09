--[[ This example does exactly the same as:
http://www.xsquawkbox.net/xpsdk/mediawiki/Custom_Command

Decide by yourself if you like C/C++ code or Lua.
There is no practical use for this command, except to show how easy it is to define a custom command.

-- -- -- 8< -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
We should use the "old fashion way" to result in a faster binary code and avoid conflicts with other scripts: ]]

gHeadPositionXDataRef = XPLMFindDataRef("sim/aircraft/view/acf_peX")

create_command( "FlyWithLua/ViewPoint/MoveRight", "Move Right",
                "",
                "XPLMSetDataf(gHeadPositionXDataRef, XPLMGetDataf(gHeadPositionXDataRef) + .1)",
                "XPLMSetDataf(gHeadPositionXDataRef, XPLMGetDataf(gHeadPositionXDataRef) + .1)")

create_command( "FlyWithLua/ViewPoint/MoveLeft", "Move Left",
                "",
                "XPLMSetDataf(gHeadPositionXDataRef, XPLMGetDataf(gHeadPositionXDataRef) - .1)",
                "XPLMSetDataf(gHeadPositionXDataRef, XPLMGetDataf(gHeadPositionXDataRef) - .1)")

                
--[[ That's all! Nothing more to do. FlyWithLua will care about register and unregister your command.

The command "create_command()" wants these arguments:

create_command( "command/reference/string", "command description string",
                "Lua commands at command begin",
                "Lua commands during command phase",
                "Lua commands at command end" )

As in the original example there is nothing defined for a command begin, the third argument is "" (an empty string).
We can not leave it away, even if there is nothing to do, or we will cause an error.

Creating a command is an automatic combination of XPLMCreateCommand() and XPLMRegisterCommandHandler()
handled by FlyWithLua. Before FlyWithLua reloads all scripts, it will automatically unregister it's custom commands. ]]
