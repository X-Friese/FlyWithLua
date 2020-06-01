local lfs = require("lfs_ffi") -- https://github.com/sonoro1234/luafilesystem

-- Reference LuaFileSystem http://keplerproject.github.io/luafilesystem/manual.html#reference

--[[ 
Testcase: 

Check if Folder exist --> if not create folder "LFS_TEST" in our Script directory done...
Reload FlyWithLua --> this prints Directory Attributes to the Dev Console and removes the Directory
]]

local folder = {}
folder = lfs.attributes(SCRIPT_DIRECTORY .. "LFS_TEST")
if folder == nil then -- folder does not exist
    lfs.mkdir(SCRIPT_DIRECTORY .. "LFS_TEST")
end

if (folder) then
    -- print Attributes to Dev Console
    print("LFS ---> Print Folder Attribute START <--- LFS")

    print("#########################")
    print("dev")
    print(folder.dev)
    print("#########################")

    print("ino")
    print(folder.ino)
    print("#########################")

    print("mode")
    print(folder.mode)
    print("#########################")

    print("mode")
    print(folder.mode)
    print("#########################")

    print("nlink")
    print(folder.nlink)
    print("#########################")

    print("uid")
    print(folder.uid)
    print("#########################")

    print("gid")
    print(folder.gid)
    print("#########################")

    print("rdev")
    print(folder.rdev)
    print("#########################")

    print("access")
    print(folder.access)
    print("#########################")

    print("modification")
    print(folder.modification)
    print("#########################")

    print("change")
    print(folder.change)
    print("#########################")

    print("size")
    print(folder.size)
    print("#########################")

    print("permissions")
    print(folder.permissions)
    print("#########################")

    print("blocks")
    print(folder.blocks)
    print("#########################")

    print("blksize")
    print(folder.blksize)
    print("#########################")

    print("LFS ---> Print Folder Attribute END <--- LFS")

    local currentdir = lfs.currentdir()
    print("LFS ---> current working directory START <--- LFS")
    print(currentdir) -- X-Plane Folder
    print("LFS ---> current working directory END <--- LFS")

    lfs.rmdir(SCRIPT_DIRECTORY .. "LFS_TEST")

end

