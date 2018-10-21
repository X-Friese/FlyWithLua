require('LuaXml')

-- load XML data from file "test.xml" into local table xfile
local xfile = xml.load("test.xml")
-- search for substatement having the tag "scene"
local xscene = xfile:find("scene")
-- if this substatement is found…
if xscene ~= nil then
  --  …print it to screen
  print(xscene)
  --  print  tag, attribute id and first substatement
  print( xscene:tag(), xscene.id, xscene[1] )
end

xfile:save"t.xml"
print("---\nREADY.")