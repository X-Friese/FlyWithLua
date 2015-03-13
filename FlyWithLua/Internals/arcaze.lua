module(..., package.seeall);
-- -------------------------------
-- -- Access Arcaze USB devices --
-- -------------------------------

local A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20
local B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13, B14, B15, B16, B17, B18, B19, B20

function open_first_device()
    local device = hid_open(5776, 65043)
    return device
end

function set_all_pins_for_input( device )
    local nobw = 0
    nobw = hid_send_filled_feature_report(device, 0, 65, 31, 0)
    nobw = hid_send_filled_feature_report(device, 0, 65, 31, 1)
end

function set_pin_direction( device, pin, direction )
    if direction == "input" then direction = 0 end
    if not direction == 0 then direction = 1 end
    if pin < 0 or pin > 39 then return end
    local nobw = 0
    nobw = hid_send_filled_feature_report(device, 0, 65, 29, pin, direction)
end

function set_pin( device, pin, value )
    if value == "off" then value = 0 end
    if not value == 0 then value = 1 end
    if pin < 0 or pin > 39 then return end
    local nobw = 0
    nobw = hid_send_filled_feature_report(device, 0, 65, 30, pin, value)
end

function read_pins( device )
    local nobw = 0
    nobw = hid_send_filled_feature_report(device, 0, 65, 8)
    _, _, _, _, _, _, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13, B14, B15, B16, B17, B18, B19, B20 = hid_get_feature_report( device, 65 )
    return A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13, B14, B15, B16, B17, B18, B19, B20
end

function read_ADCs( device )
    local nobw = 0
    nobw = hid_send_filled_feature_report(device, 0, 65, 27)
    _, _, _, _, _, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12 = hid_get_feature_report( device, 65 )
    return A1+A2*256, A3+A4*256, A5+A6*256, A7+A8*256, A9+A10*256, A11+A12*256
end

function read_encoders( device )
    local nobw = 0
    nobw = hid_send_filled_feature_report(device, 0, 65, 44)
    _, _, _, _, _, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13, B14, B15, B16, B17, B18, B19, B20 = hid_get_feature_report( device, 65 )
    return A1+A2*256, A3+A4*256, A5+A6*256, A7+A8*256, A9+A10*256, A11+A12*256, A13+A14*256, A15+A16*256, A17+A18*256, A19+A20*256, B1+B2*256, B3+B4*256, B5+B6*256, B7+B8*256, B9+B10*256, B11+B12*256, B13+B14*256, B15+B16*256, B17+B18*256, B19+B20*256
end

function init_display( device, address, intensity, scan_limit )
    if address == "1a" or address == "1b" then address = 0 end
    if address == "2a" or address == "2b" then address = 1 end
    if address == "3a" or address == "3b" then address = 2 end
    if address == "4a" or address == "4b" then address = 3 end
    intensity = intensity or 15
    scan_limit = scan_limit or 8
    if intensity < 0 then intensity = 0 end
    if intensity > 15 then intensity = 15 end
    if scan_limit < 4 then scan_limit = 4 end
    if scan_limit > 8 then scan_limit = 8 end
    nobw = hid_send_filled_feature_report(device, 0, 65, 33, address, 0xff, intensity, scan_limit)
end

function show( device, address, mask, text )
    if string.find(address, "a") then
        text = string.reverse(text)
    end
    local point = string.find(text, '%.')
    text = string.gsub(text, '%.', '')
    text = string.gsub(text, '%-', ':')
    text = string.gsub(text, ' ', '?')
    local digits = {}
    local addressbyte = 0
    addressbyte = tonumber(string.sub(address, 1, 1))-1
    digits[1] = string.byte(text, 1) or 15
    digits[2] = string.byte(text, 2) or 15
    digits[3] = string.byte(text, 3) or 15
    digits[4] = string.byte(text, 4) or 15
    digits[5] = string.byte(text, 5) or 15
    digits[6] = string.byte(text, 6) or 15
    digits[7] = string.byte(text, 7) or 15
    digits[8] = string.byte(text, 8) or 15
    if  point then
        if string.find(address, "a") then 
            digits[point] = digits[point] + 128
        else
            digits[point-1] = digits[point-1] + 128
        end
    end
    
    local nobw = 0
    nobw = hid_send_filled_feature_report(device, 0, 65, 0x24, addressbyte, digits[1], digits[2], digits[3], digits[4], digits[5], digits[6], digits[7], digits[8], mask)
end
