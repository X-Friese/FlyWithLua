-- A little home made radio instrument build with an Arcaze USB
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
-- of 10 kHz. When you push the button, active and standby values should switch.
--
-- To connect the rotary encoder to the Arcaze USB, we connect channel A and B of the big wheel to the
-- input pin pairs A1 and A2. The pairs A3 and A4 must be connected to the smaller wheel and A5 should
-- be connected to the push button switch.

-- These are the DataRefs we need: COM1, COM1_STDBY, INSTRUMENT_BRIGHTNESS
dataref("COM1", "sim/cockpit/radios/com1_freq_hz", "writable")
dataref("COM1_STDBY", "sim/cockpit/radios/com1_stdby_freq_hz", "writable")
dataref("INSTRUMENT_BRIGHTNESS", "sim/cockpit/electrical/instrument_brightness", "readonly")

-- and we need the Arcaze module
require "arcaze"

-- Now we have to find the Arcaze USB device
local my_arcaze = arcaze.open_first_device()

-- we will check, if an Arcaze was found, to avoid crashing the sim if the Arcaze isn't plugged in
-- when no Arcaze was found, the function arcaze.open_first_device() will return a nil value 
if not (my_arcaze == nil) then

    -- As both rotary encoders will result in a relative value, we will have to remember the last value
    -- to decide if the change is made up or down and how many steps it was turned. We need two local variables.
    local last_big_radio_wheel = 0
    local last_small_radio_wheel = 0

    -- As we connected the rotary to the first encoder pins, we can easily get the relative values:
    last_big_radio_wheel, last_small_radio_wheel = arcaze.read_encoders( my_arcaze )

    -- The same with the push button, but we simply guess it is not pushed at the moment
    local last_radio_push_button = 0

    -- Now we init the Display Driver 32 connectors (the brightness DataRef has a range 0.0 to 1.0, but we need 0 to 15)
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
            local temp_value = COM1
            COM1 = COM1_STDBY
            COM1_STDBY = temp_value
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
            COM1_STDBY = COM1_STDBY + wheel_diff * 100
            
            -- fix the value of COM1_STDBY
            if COM1_STDBY < 11800 then COM1_STDBY = 13697 end
            if COM1_STDBY > 13697 then COM1_STDBY = 11800 end
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
            COM1_STDBY = COM1_STDBY + wheel_diff * 2.5
            
            -- fix the value of COM1_STDBY
            if COM1_STDBY < 11800 then COM1_STDBY = 13697 end
            if COM1_STDBY > 13697 then COM1_STDBY = 11800 end
        end
        
        -- remember the state
        last_radio_push_button = radio_push_button
        last_big_radio_wheel = big_radio_wheel
        last_small_radio_wheel = small_radio_wheel
        
        -- show the COM1 frequencies
        -- As the internal storage is an integer DataRef in 10 kHz values, we will
        -- have to program the 25 kHz steps of real aviation into the display.
        local number_to_display = string.format( "  %.2f", COM1/100 )
        if string.sub(number_to_display, -1) == "2" or string.sub(number_to_display, -1) == "7" then
            arcaze.show( my_arcaze, "1b", 0xff, number_to_display.."5" ) 
        else
            arcaze.show( my_arcaze, "1b", 0xff, number_to_display.."0" ) 
        end
        number_to_display = string.format( "  %.2f", COM1_STDBY/100 )
        if string.sub(number_to_display, -1) == "2" or string.sub(number_to_display, -1) == "7" then
            arcaze.show( my_arcaze, "3a", 0xff, number_to_display.."5" ) 
        else
            arcaze.show( my_arcaze, "3a", 0xff, number_to_display.."0" ) 
        end
        
        -- adjust the brightness if changes have occurred
        if not (last_instrument_brightness == INSTRUMENT_BRIGHTNESS) then
            arcaze.init_display( my_arcaze, "1b", INSTRUMENT_BRIGHTNESS*15 )
            arcaze.init_display( my_arcaze, "3a", INSTRUMENT_BRIGHTNESS*15 )
            last_instrument_brightness = INSTRUMENT_BRIGHTNESS
        end
    end

    do_every_frame("little_arcaze_radio()")

end -- of checking if the Arcaze is really connected