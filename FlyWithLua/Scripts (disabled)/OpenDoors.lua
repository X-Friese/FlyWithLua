door = dataref_table("sim/cockpit2/switches/custom_slider_on")

function open_all_doors()
    for i = 0, 23 do
        door[i] = 1
    end
end

function close_all_doors()
    for i = 0, 23 do
        door[i] = 0
    end
end

add_macro("Open all doors",  "open_all_doors()", "close_all_doors()", "deactivate")

-- if we start "cold&dark", we want the pilot's door to be open
if get("sim/cockpit/electrical/battery_on") == 0 then
    door[0] = 1
end
