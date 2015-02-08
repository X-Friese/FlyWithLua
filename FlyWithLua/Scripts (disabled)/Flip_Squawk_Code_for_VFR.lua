-- This script flips the squawk code between 1200 and 7000
-- to set the transponder for a VFR flight. As the VFR code
-- is 7000 in Europe, we can't simply do this:

create_positive_edge_trigger(13, "sim/cockpit/radios/transponder_code", 0, 1200);

-- Instead we use the flip command to set it to 1200 or 7000:

create_positive_edge_flip(14, "sim/cockpit/radios/transponder_code", 0, 1200, 7000);

-- The flip command compares the the value of the DataRef with it's first value given.
-- If this is equal, than the DataRef will be set to the second value given.
-- If not, it will be set to the first value.
--
-- So if the transponder is set to 2015, than it will be flipped to 1200. Another press onto
-- the button will than flip the transponder code to 7000.
--
-- Please keep in mind that the value after the DataRef's name is always the index.
-- The index must be 0 (zero), even if it is a non-array value.
--
-- Play around with this little script and take a look into the manual. There are more
-- button functions you can create. All functions starting with "create_" have to be called
-- only once. After that, they will work until the scripts are reloaded or Lua stops.
-- They are the old "Button2DataRef" plugin included and work faster than pure Lua code.
-- So if you can exchange a >>do_every_frame("if button(XYZ) ...<< construction with a
-- simple >>create_<< function call, choose the >>create_<< function to get maximum 
-- execution speed (highest frame rate).