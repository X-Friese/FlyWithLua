module(..., package.seeall);
-- -------------------------------
-- -- Access Arcaze USB devices --
-- -------------------------------

local A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20 = 0
local B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13, B14, B15, B16, B17, B18, B19, B20 = 0

function open_first_device()
    local device = hid_open(5776, 65043)
    return device
end

function read_pins( device )
    local nobw = 0
    nobw = hid_send_filled_feature_report(device, 0, 65, 8)
    _, _, _, _, _, _, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13, B14, B15, B16, B17, B18, B19, B20 = hid_get_feature_report( device, 65 )
    return A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13, B14, B15, B16, B17, B18, B19, B20
end

function read_encoders( device )
    local nobw = 0
    nobw = hid_send_filled_feature_report(device, 0, 65, 44)
    _, _, _, _, _, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15, A16, A17, A18, A19, A20, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13, B14, B15, B16, B17, B18, B19, B20 = hid_get_feature_report( device, 65 )
    return A1+A2*256, A3+A4*256, A5+A6*256, A7+A8*256, A9+A10*256, A11+A12*256, A13+A14*256, A15+A16*256, A17+A18*256, A19+A20*256, B1+B2*256, B3+B4*256, B5+B6*256, B7+B8*256, B9+B10*256, B11+B12*256, B13+B14*256, B15+B16*256, B17+B18*256, B19+B20*256
end

function init_display( device, adress, intensity, scan_limit )
    nobw = hid_send_filled_feature_report(device, 0, 65, 33, adress, 0xff, intensity, scan_limit)
end

function show( device, adress, mask, text )
    if string.find(adress, "A") then
        text = string.reverse(text)
    end
    local point = string.find(text, '%.')
    text = string.gsub(text, '%.', '')
    local digits = {}
    local adressbyte = 0
    adressbyte = tonumber(string.sub(adress, 1, 1))-1
    digits[1] = string.byte(text, 1) or 0
    digits[2] = string.byte(text, 2) or 0
    digits[3] = string.byte(text, 3) or 0
    digits[4] = string.byte(text, 4) or 0
    digits[5] = string.byte(text, 5) or 0
    digits[6] = string.byte(text, 6) or 0
    digits[7] = string.byte(text, 7) or 0
    digits[8] = string.byte(text, 8) or 0
    if  point then
        if string.find(adress, "A") then 
            digits[point] = digits[point] + 128
        else
            digits[point-1] = digits[point-1] + 128
        end
    end
    
    local nobw = 0
    nobw = hid_send_filled_feature_report(device, 0, 65, 0x24, adressbyte, digits[1], digits[2], digits[3], digits[4], digits[5], digits[6], digits[7], digits[8], mask)
end
