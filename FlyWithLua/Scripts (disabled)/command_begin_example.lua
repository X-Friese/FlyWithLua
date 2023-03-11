--[[
This script will provide an example of how to use command_begin and command_end using the default Sikorsky S76.
I have chosen to do this example in imgui window because it’s loop will not freeze the game.

How to use: From the Aircrafts select Sikorsky S76, then start the aircraft, and  from the Plugins menu select FlyWithLua > FlyWithLua Macros > command_begin: Start.

By HurricanetwistR 2023
--]]

if PLANE_ICAO == "S76" then
    timestamp  = create_dataref_table("flywithlua/test/custom/stamp", "IntArray")

    --GMT time stamp saved in Dataref.
    function stamp_dataRef()
        timestamp[0] = tonumber(os.time(os.date("!*t")))

    end
    --GMT time stamp saved in Dataref.
    function stamp_dataRef2()
        timestamp[1] = tonumber(os.time(os.date("!*t")))

    end

    --current GMT time.
    function returnTime()
        return tonumber(os.time(os.date("!*t")))
    end

    function close_wnd()
        if ex_wnd then
            float_wnd_destroy(ex_wnd)
        end
    end

    function test_command_build()
      uWidth = 200
      uHieght = 120
      ex_wnd = float_wnd_create(uWidth, uHieght, 1, true)
            -- print("With="..uWidth.." Height="..uHieght)
            float_wnd_set_title(ex_wnd, "command_begin Example")
            float_wnd_set_imgui_builder(ex_wnd, "test_command")
            float_wnd_set_onclose(ex_wnd, "close_wnd")
             fireTest = 0
    fireTestEnd = 0
    timestamp[1] = 0

    end

    function test_command()
        if timestamp[0] + 1 < returnTime() and fireTest == 0 then
            command_begin("sim/annunciator/test_fire_1_annun")
            fireTest = 1
            stamp_dataRef2()
        end
        if timestamp[1] ~= 0 and (timestamp[2] + 6) < returnTime() and fireTest == 1 then
            command_end("sim/annunciator/test_fire_1_annun")
            fireTestEnd = 1
        end

        if fireTest == 1 then
            imgui.PushStyleColor(imgui.constant.Col.Text, 0xFF4e6a00)
        end
        imgui.TextUnformatted("Example Started.")
        if fireTest == 1 then
            imgui.PopStyleColor()
        end
        imgui.Separator()
        if fireTestEnd == 1 then
            imgui.PushStyleColor(imgui.constant.Col.Text, 0xFF4e6a00)
        end
        imgui.TextUnformatted("Example Ended.")
        if fireTestEnd == 1 then
            imgui.PopStyleColor()
        end
    end

    function startexamP()
        stamp_dataRef()
        test_command_build()
    end
    add_macro("command_begin: Start", "startexamP()")
end
