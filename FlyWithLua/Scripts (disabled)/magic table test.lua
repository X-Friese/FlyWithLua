-- This is only an example file, useless for pilots, nice for developers.
-- The script turns on the first battary and the landing lights. The same can be done with:
-- set_array("sim/cockpit/electrical/battery_array_on", 0, 1)
-- set("sim/cockpit/electrical/landing_lights_on", 1)
-- But this script shows the use of a magic table accessing X-Plane's DataRefs

-- first we define a local magic table
local batty = dataref_table("sim/cockpit/electrical/battery_array_on")

-- show the content of the magic table
for ref, val in pairs(batty) do
    print(string.format("batty.%s = %s", ref, tostring(val)))
end

-- where are the values batty[0] to batty[8]?
-- they are produced on demand by the metatable DATAREF_META_TABLE
print("That's magic:")
for i = 0, 7, 1 do
    print(string.format("batty[%i] = %i", i, batty[i]))
end

-- turn on the first battery
batty[0] = 1

-- the single value DataRefs can be accessed by magic tables with a dummy index 0 (zero)
local landinglights = dataref_table("sim/cockpit/electrical/landing_lights_on")
print("before: landinglights[0] = " .. landinglights[0])
landinglights[0] = 1
print("after:  landinglights[0] = " .. landinglights[0])

-- even string DataRefs can be accessed with a dummy index 0
local plane_description = dataref_table("sim/aircraft/view/acf_descrip")
print("description of your plane: " .. plane_description[0])