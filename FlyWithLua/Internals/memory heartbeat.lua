-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
-- Script:   memory heartbeat.lua                                              
-- Version:  1.0
-- Build:    2015-03-22 16:50:42z
-- Author:   Carsten Lynker
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
-- Usage
--
-- Just copy this file into the "Scripts" directory of FlyWithLua.
-- After that you will find a macro "Show memory usage heartbeat" inside the plugin menu.
-- If you check this macro, a heartbeat monitor will be printed on your screen,
-- showing the memory usage of the last 500 frames.
--
-- This is only the memory Lua needs for the stack and all Lua stored stuff like variables, functions,
-- tables and so on. Some objects like wave files or HID handler are not stored into Lua, but will be
-- allocated and managed by the core C code of FlyWithLua.
--
-- Look at the cardioid curve of Lua's memory consumption. If it looks like a saw tooth line,
-- everything is okay. If it goes more and more upwards, you have a memory issue!
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 

require "graphics"

local memory_usage_table = {}
local max_table_elements = 500
show_memory_heartbeat = false

function show_memory_usage()
    -- do nothing when the function isn't wanted
    if show_memory_heartbeat == false then return end
    
    -- fill the table
    table.insert(memory_usage_table, LUA_MEMORY_USAGE_KB)
    
    -- limit the table size to the given maximum
    if table.maxn(memory_usage_table) > max_table_elements then
        table.remove(memory_usage_table, 1)
    end
    
    -- draw the memory usage
    -- background first
    local x = (SCREEN_WIDTH / 2) - max_table_elements
    local y = (SCREEN_HIGHT / 2) - 100
    XPLMSetGraphicsState(0,0,0,1,1,0,0)
    graphics.set_color(0, 0, 0, 0.75)
    graphics.draw_rectangle(x, y, x + (max_table_elements * 2), y + 200)
    
    -- calculate the maximum memory usage
    local max_memory_used = 0
    for k, v in pairs(memory_usage_table) do
        if v > max_memory_used then
            max_memory_used = v
        end
    end
    
    -- and print it on the screen
    graphics.set_color(1, 1, 1, 1)
    graphics.draw_line(x + (max_table_elements * 2), y + 200, x + (max_table_elements * 2) + 10, y + 200)
    graphics.draw_line(x + (max_table_elements * 2), y, x + (max_table_elements * 2) + 10, y)
    draw_string_Helvetica_12(x + (max_table_elements * 2) + 15, y, "  0 kB")
    draw_string_Helvetica_12(x + (max_table_elements * 2) + 15, y + 200, tostring(max_memory_used).." kB")
    
    -- now draw the heartbeat line
    graphics.set_color(1, 0, 0, 1)
    graphics.set_width(3)
    local last_memory_usage = memory_usage_table[1]
    for k, v in pairs(memory_usage_table) do
        graphics.draw_line(x, y + (last_memory_usage / max_memory_used * 200), x + 2, y + (v / max_memory_used * 200))
        x = x + 2
        last_memory_usage = v
    end
end

do_every_draw("show_memory_usage()")

add_macro("Show memory usage heartbeat", "show_memory_heartbeat = true", "show_memory_heartbeat = false", "deactivate")