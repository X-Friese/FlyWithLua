require "HUD"

dataref("QNH_Pilot", "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot", "writable")
dataref("GPU", "sim/cockpit/electrical/gpu_on", "writable")

HUD.begin_HUD(-81, -200, 80, 45, "MyExample")

HUD.create_element("caption", 0, 30, 80, 15)
HUD.draw_string(12, 3, 10, "set to STD")
HUD.create_click_action(0, 0, 80, 15, "QNH_Pilot = 29.92")

HUD.create_element("baro", 0, 0, 80, 30, 0, 0, 0, 0)
HUD.draw_string(12, 20, 10, "BARO")
HUD.draw_fstring(12, 3, 18, "%2.2f", "QNH_Pilot")
HUD.create_wheel_action(0, 0, 80, 30, "QNH_Pilot = QNH_Pilot + MOUSE_WHEEL_CLICKS / 100")

HUD.end_HUD()

HUD.begin_HUD(100, 1, 30, 12, "GPU", "always")
HUD.create_element("GPU", 0, 0, 30, 12)
HUD.draw_string(4, 2, 10, "GPU")
HUD.create_backlight_indicator( 0, 0, 30, 12, "GPU == 1", 0, 1, 0, 0.5)
HUD.create_click_switch(0, 0, 30, 12, "GPU", 0, 1)
HUD.end_HUD()