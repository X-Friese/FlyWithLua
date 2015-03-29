----------------------------------------------------------------------------
-- Script:   3D head positioning                                          --
-- Author:   Carsten Lynker                                               --
-- Version:  1.0                                                          --
-- Build:    2015-02-21                                                   --
-- Description:                                                           --
-- This script sets the pilot's head inside the 3D view.                  --
-- It will fit much better on a personal screen than the default          --
-- position X-Plane or the aircraft manufacturer sets.                    --
----------------------------------------------------------------------------

-- define the DataRef to check the view type
DataRef( "view_type", "sim/graphics/view/view_type" )
local last_view_type = 0

-- a switch to turn it on
local use_3D_head_positioning = false

-- define the initial settings (for the standard C172)
local store_heading   = {}
local default_head    = {x=-0.1, y=0.4, z=0.06, heading=0.3, pitch=5}
local left_head       = {x=-0.1, y=0.4, z=0, heading=270, pitch=2.5}
local right_head      = {x=-0.1, y=0.4, z=0, heading=90, pitch=2.5}
local instrument_view = {x=-0.076, y=0.23, z=-0.2, heading=0, pitch=0.2}

function set_default_3d_position()
	-- type 1026 is the inside 3d view
	if (( view_type == 1026 ) and ( last_view_type ~= 1026 )) then
        -- does it have a position stored?
        if use_3D_head_positioning then
            set_pilots_head(default_head.x, default_head.y, default_head.z, default_head.heading, default_head.pitch)
        else
            -- if a plane has no individual configuration, use the default one
            default_head.x, default_head.y, default_head.z, default_head.heading, default_head.pitch = get_pilots_head()
            set_left_looking_head()
            set_right_looking_head()
            set_instrument_looking_head()
        end
	end
	-- remember some last values for next frame
	last_view_type = view_type
end

-- register the function to set the default view
do_every_frame("set_default_3d_position()")

-- provide the functions for the commands
function look_left()
    store_heading.x, store_heading.y, store_heading.z, store_heading.heading, store_heading.pitch = get_pilots_head()
    set_pilots_head(left_head.x, left_head.y, left_head.z, left_head.heading, left_head.pitch)
end

function look_right()
    store_heading.x, store_heading.y, store_heading.z, store_heading.heading, store_heading.pitch = get_pilots_head()
    set_pilots_head(right_head.x, right_head.y, right_head.z, right_head.heading, right_head.pitch)
end

function look_instruments()
    store_heading.x, store_heading.y, store_heading.z, store_heading.heading, store_heading.pitch = get_pilots_head()
    set_pilots_head(instrument_view.x, instrument_view.y, instrument_view.z, instrument_view.heading, instrument_view.pitch)
end

function resume_head()
    set_pilots_head(store_heading.x, store_heading.y, store_heading.z, store_heading.heading, store_heading.pitch)
end

-- register the positioning commands
create_command("FlyWithLua/pilots_head/look_left", "Take a look on the left side.", "look_left()", "", "resume_head()")
create_command("FlyWithLua/pilots_head/look_right", "Take a look on the left side.", "look_right()", "", "resume_head()")
create_command("FlyWithLua/pilots_head/look_on_instruments", "Take a look on the instrument panel.", "look_instruments()", "", "resume_head()")

-- provide some functions to other scripts to define new settings
function set_default_looking_head(x, y, z, heading, pitch)
    default_head.x = x or 0
    default_head.y = y or 0
    default_head.z = z or 0
    default_head.heading = heading or 0
    default_head.pitch = pitch or 0
    use_3D_head_positioning = true
end

function set_left_looking_head(x, y, z, heading, pitch)
    left_head.x = x or default_head.x
    left_head.y = y or default_head.y
    left_head.z = z or default_head.z
    left_head.heading = heading or 270
    left_head.pitch = pitch or default_head.pitch
end

function set_right_looking_head(x, y, z, heading, pitch)
    right_head.x = x or default_head.x
    right_head.y = y or default_head.y
    right_head.z = z or default_head.z
    right_head.heading = heading or 90
    right_head.pitch = pitch or default_head.pitch
end

function set_instrument_looking_head(x, y, z, heading, pitch)
    instrument_view.x = x or default_head.x
    instrument_view.y = y or default_head.y
    instrument_view.z = z or default_head.z
    instrument_view.heading = heading or 0
    instrument_view.pitch = pitch or default_head.pitch
end

