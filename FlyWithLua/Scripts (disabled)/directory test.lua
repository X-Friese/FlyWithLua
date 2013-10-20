-- testing the directory_to_table() function

-- the path can be "unix style" independent of the OS Lua is running
-- this is different to the read/write functions provided by pure Lua
-- we will just use the pre-defined variable to print out the Scripts folder

local dir = directory_to_table(SCRIPT_DIRECTORY)

for filenumber, filename in pairs(dir) do
    print(string.format('File no. %i = "%s"', filenumber, SCRIPT_DIRECTORY .. filename))
end