my_XSB_info = {}

function get_my_XSB_info()
  local my_freq = XSB_TEXT_FREQ / 1000
  local new_text = "(" .. os.date() .. " " .. my_freq .. ") " .. XSB_TEXT_FROM .. ": " .. XSB_TEXT_MESSAGE
  my_XSB_info[2], my_XSB_info[3], my_XSB_info[4], my_XSB_info[5] = my_XSB_info[1], my_XSB_info[2], my_XSB_info[3], my_XSB_info[4]
  my_XSB_info[1] = new_text
end

do_on_new_XSB_text("get_my_XSB_info()")

do_every_draw("huge_bubble(SCREEN_WIDTH - 600, 10,'Messages received from XSquawkBox',  my_XSB_info[5], my_XSB_info[4], my_XSB_info[3], my_XSB_info[2], my_XSB_info[1])")