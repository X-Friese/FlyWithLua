-- This Script needs the LuaXML library.
-- Read the manual how to include it!

require('LuaXml')

-- load XML data from file "test.xml" into local table xfile
local xfile = xml.load(SCRIPT_DIRECTORY .. "test.xml")
-- search for substatement having the tag "scene"
local xscene = xfile:find("scene")
-- if this substatement is found...
if xscene ~= nil then
  --  ...print it to screen
  print(tostring(xscene))
  --  print  tag, attribute id and first substatement
  print( xscene:tag(), xscene.id, xscene[1] )
end

xfile:save(SCRIPT_DIRECTORY .. "t.xml")
print("---")
print("READY.")