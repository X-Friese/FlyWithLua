-- define some help variables
local last_a = 0
local last_b = 0
local last_c = 0
local last_d = 0
local last_e = 0
local last_f = 0
local last_g = 0
local last_h = 0
local last_i = 0

-- connect to the first HID device found (and hope it is a stick)
first_HID_device = hid_open_path(ALL_HID_DEVICES[1].path)
if first_HID_device == nil then
    print("Oh, no!")
else
    hid_set_nonblocking(first_HID_device, 1)
end

-- show input of nine values
function draw_HID_message()
    nov, a, b, c, d, e, f, g, h, i = hid_read(first_HID_device, 9)
    nov = nov or 0
    a = a or last_a
    b = b or last_b
    c = c or last_c
    d = d or last_d
    e = e or last_e
    f = f or last_f
    g = g or last_g
    h = h or last_h
    i = i or last_i
    last_a = a
    last_b = b
    last_c = c
    last_d = d
    last_e = e
    last_f = f
    last_g = g
    last_h = h
    last_i = i
    draw_string(100, SCREEN_HIGHT - 200, "nov, a, b, c, d, e, f, g, h, i = hid_read(first_HID_device, 9)")
    draw_string(100, SCREEN_HIGHT - 215, string.format("nov = %i, a = %03i, b = %03i, c = %03i, d = %03i, e = %03i, f = %03i, g = %03i, h = %03i, i = %03i", nov, a, b, c, d, e, f, g, h, i))
end

do_every_draw("draw_HID_message()")
