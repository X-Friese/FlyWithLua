-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
-- A little home made radio instrument build with an Arcaze USB   --
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
--
-- version 1.1 for X-Plane 10.30+
-- build 2015-03-28
-- author Carsten Lynker
--
-- changes since version 1.0:
-- Using the new DataRefs from "sim/cockpit2/.." to make it easier to access the COM1 and COM1_STDBY frequency,
-- as we now can handle 1 kHz steps (before X-Plane 10.30 we can only handle a 10 kHz stepping).
-- Thanks to Ben Supnik for the advice.
--
-- -- -- -- -- 8< -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
--
-- All we need are these items:
-- 1x Arcaze USB board (you can get it here: http://wiki.simple-solutions.de/de/products/Arcaze/Arcaze-USB )
-- 1x Arcaze Display Driver 32 (this one: http://wiki.simple-solutions.de/de/products/Arcaze/Arcaze-USB/Arcaze_Display_Driver_32 )
-- 2x Arcaze Display Board 9 mm with 6 Digits (this one: http://simple-solutions.de/shop/product_info.php?products_id=76 )
-- 12x 7-Segment-Display 9 mm in your favourite colour (maybe this: http://simple-solutions.de/shop/product_info.php?products_id=78 )
-- 1x ELMA Dual Encoder with push-button (this one: http://www.elma.com/en/products/rotary-switches/rotary-switching-products/product-pages/type-e37-detail/ )
-- 1x A Jumper Cable Set with 10 pieces (this one: http://simple-solutions.de/shop/product_info.php?products_id=49 )
--
-- And of course you will need a box to fit it all in. That's all. If you buy these items at Simple Solution store, all other cable
-- needed will be included into the delivery. The jumper cable is nice and easy, but it's cheaper to kill an old PC and use the
-- 40 pin parallel ATA cable ( http://en.wikipedia.org/wiki/Parallel_ATA ).
--
-- Our new instrument should have two displays and one double rotary encoder with a push button. It will look like this:
--
--   left display           right display           double rotary with push button
--    - - - - - -            - - - - - -              -
--   | | | | | | |          | | | | | | |           ( O )
--    - - - - - -            - - - - - -              -
--
-- The left display should show the COM1 active frequency in MHz, the right on the standby frequency.
-- When you turn the big wheel of the rotary encoder, you will increase or decrease the standby frequency 
-- in steps of 1 MHz. When turning the smaller top wheel, the standby frequency should change in steps
-- of 25 kHz. When you push the button, active and standby values should switch.
--
-- To connect the rotary encoder to the Arcaze USB, we connect channel A and B of the big wheel to the
-- input pin pairs A1 and A2. The pairs A3 and A4 must be connected to the smaller wheel and A5 should
-- be connected to the push button switch.
--
-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- >8 -- -- -- -- -- -- -- 

-- We need the Arcaze module
require "arcaze"

-- The DataRefs we want to use are first implemented into X-Plane 10.30, so we have to check it
if XPLANE_VERSION < 10300 then
    logMsg('FlyWithLua error: The script "little arcaze radio.lua" needs X-Plane 10.30 or newer!')
else
    -- These are the DataRefs we need: COM1, COM1_STDBY, INSTRUMENT_BRIGHTNESS
    dataref("NEW_COM1", "sim/cockpit2/radios/actuators/com1_frequency_hz_833", "writable")
    dataref("NEW_COM1_STDBY", "sim/cockpit2/radios/actuators/com1_standby_frequency_hz_833", "writable")
    dataref("INSTRUMENT_BRIGHTNESS", "sim/cockpit/electrical/instrument_brightness", "readonly")
end

-- First we have to find the Arcaze USB device
local my_arcaze = arcaze.open_first_device()

-- we will check, if an Arcaze was found, to avoid crashing the sim if the Arcaze isn't plugged in
-- when no Arcaze was found, the function arcaze.open_first_device() will return a nil value 
if not ((my_arcaze == nil) or (XPLANE_VERSION < 10300)) then

    -- As both rotary encoders will result in a relative value, we will have to remember the last value
    -- to decide if the change is made up or down and how many steps it was turned. We need two local variables.
    local last_big_radio_wheel = 0
    local last_small_radio_wheel = 0

    -- As we connected the rotary to the first encoder pins, we can easily get the relative values:
    last_big_radio_wheel, last_small_radio_wheel = arcaze.read_encoders( my_arcaze )

    -- The same with the push button, but we simply guess it is not pushed at the moment
    local last_radio_push_button = 0

    -- Now we initialize the Display Driver 32 connectors (the brightness DataRef has a range 0.0 to 1.0, but we need 0 to 15).
    -- Don't worry about float*integer=???, FlyWithLua knows that we want an integer and will convert the result
    -- before sending it to the Arcaze Display Driver 32 module.
    arcaze.init_display( my_arcaze, "1b", INSTRUMENT_BRIGHTNESS*15 )
    arcaze.init_display( my_arcaze, "3a", INSTRUMENT_BRIGHTNESS*15 )

    -- We will store the brightness to see if it changes. This will avoid an new initialization with every frame.
    local last_instrument_brightness = INSTRUMENT_BRIGHTNESS

    -- We will create a function to be called every frame
    function little_arcaze_radio()
        -- define some locals to reduce name conflicts
        local big_radio_wheel, small_radio_wheel, radio_push_button, wheel_diff
        
        -- and fill them
        big_radio_wheel, small_radio_wheel = arcaze.read_encoders( my_arcaze )
        _, _, _, _, radio_push_button = arcaze.read_pins( my_arcaze )            -- A1, A2, A3 and A4 are unused
        
        -- do the button action if necessary (on positive edge)
        if (last_radio_push_button == 0) and (radio_push_button == 1) then
            local temp_value = NEW_COM1
            NEW_COM1 = NEW_COM1_STDBY
            NEW_COM1_STDBY = temp_value
        end
        
        -- do the change if necessary
        if not (last_big_radio_wheel == big_radio_wheel) then
            -- put the difference into a local variable
            wheel_diff = last_big_radio_wheel - big_radio_wheel
            
            -- fix the turnovers (... 65534 <-> 65535 <-> 0 <-> 1 <-> 2 ...)
            if wheel_diff >  60000 then wheel_diff = wheel_diff - 65536 end
            if wheel_diff < -60000 then wheel_diff = wheel_diff + 65536 end
            
            -- our rotary encoder will produce 4 edges every single turn
            wheel_diff = wheel_diff / 4

            -- change the value of COM1_STDBY in 1 MHz steps
            NEW_COM1_STDBY = NEW_COM1_STDBY + wheel_diff * 1000
            
            -- fix the value of COM1_STDBY
            if NEW_COM1_STDBY < 118000 then NEW_COM1_STDBY = 136975 end
            if NEW_COM1_STDBY > 136975 then NEW_COM1_STDBY = 118000 end
            NEW_COM1_STDBY = NEW_COM1_STDBY - NEW_COM1_STDBY % 25
        end
        
        -- and the same with the little wheel
        if not (last_small_radio_wheel == small_radio_wheel) then
            -- put the difference into a local variable
            wheel_diff = last_small_radio_wheel - small_radio_wheel
            
            -- fix the turnovers (... 65534 <-> 65535 <-> 0 <-> 1 <-> 2 ...)
            if wheel_diff >  60000 then wheel_diff = wheel_diff - 65536 end
            if wheel_diff < -60000 then wheel_diff = wheel_diff + 65536 end
            
            -- our rotary encoder will produce 4 edges every single turn
            wheel_diff = wheel_diff / 4

            -- change the value of COM1_STDBY in 10 kHz steps
            NEW_COM1_STDBY = NEW_COM1_STDBY + wheel_diff * 25
            
            -- fix the value of COM1_STDBY
            if NEW_COM1_STDBY < 118000 then NEW_COM1_STDBY = 136975 end
            if NEW_COM1_STDBY > 136975 then NEW_COM1_STDBY = 118000 end
            NEW_COM1_STDBY = NEW_COM1_STDBY - NEW_COM1_STDBY % 25
        end
        
        -- remember the state
        last_radio_push_button = radio_push_button
        last_big_radio_wheel = big_radio_wheel
        last_small_radio_wheel = small_radio_wheel
        
        -- show the COM1 and COM1_STDBY
        -- Since X-Plane 10.30 we have a new DataRef for radio frequencies
        -- Please keep in mind that we are using 6-digits displays. On the 1b, 2b, 3b and 4b
        -- pin connectors we will have to fill the last six digits of a eight digit string.
        arcaze.show( my_arcaze, "1b", 0xff, string.format( "  %.3f", NEW_COM1/1000 ) )
        -- The 1a, 2a, 3a and 4a displays will show the given string from it's beginning.
        -- So no two spaces for alignment are needed.
        arcaze.show( my_arcaze, "3a", 0xff, string.format( "%.3f", NEW_COM1_STDBY/1000 ) ) 
        
        -- adjust the brightness if changes have occurred
        if not (last_instrument_brightness == INSTRUMENT_BRIGHTNESS) then
            arcaze.init_display( my_arcaze, "1b", INSTRUMENT_BRIGHTNESS*15 )
            arcaze.init_display( my_arcaze, "3a", INSTRUMENT_BRIGHTNESS*15 )
            last_instrument_brightness = INSTRUMENT_BRIGHTNESS
        end
    end

    do_every_frame("little_arcaze_radio()")

end -- of checking if the Arcaze is really connected