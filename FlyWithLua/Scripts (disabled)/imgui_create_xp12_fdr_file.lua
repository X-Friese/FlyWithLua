-- Imgui Create XP12 FDR File using imgui
-- William R. Good 06-24-22

if not SUPPORTS_FLOATING_WINDOWS then
    -- to make sure the script doesn't stop old FlyWithLua versions
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

dataref("Altitude", "sim/cockpit2/gauges/indicators/altitude_ft_pilot")  -- Altitude Baro

dataref("HDG", "sim/cockpit/misc/compass_indicated")  -- Indicated cockpit heading in magnetic degrees

dataref("Pitch", "sim/cockpit/gyros/the_vac_ind_deg")  -- The indicated pitch on the panel for the first vacuum instrument

dataref("Roll", "sim/cockpit/gyros/phi_vac_ind_deg")  -- The indicated roll on the panel for the first vacuum instrument

dataref("BaroA", "sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot") -- Baro Setting (in/HG)

dataref("AltMSL", "sim/flightmodel/misc/h_ind") -- Indicated height, MSL, in feet, primary system, based on pilots barometric pressure input.

dataref("VSpd", "sim/cockpit2/gauges/indicators/vvi_fpm_pilot") -- Vertical Speed (Feet/min)

dataref("TAS", "sim/cockpit2/gauges/indicators/true_airspeed_kts_pilot") -- True Air Speed (ktas)

dataref("IAS", "sim/cockpit2/gauges/indicators/airspeed_kts_pilot") -- Indicated Airspeed (knots)

dataref("GndSpd", "sim/cockpit2/gauges/indicators/ground_speed_kt") -- Ground Speed (gps-Knots)

dataref("Stall_Warning", "sim/cockpit2/annunciators/stall_warning_ratio") -- Stall Warning

dataref("flap_handle_ratio", "sim/cockpit2/controls/flap_system_deploy_ratio") -- 0.5 Flaps (System)

dataref("flap_ratio", "sim/cockpit2/controls/flap_handle_request_ratio") -- 0.5 Flaps (surface) ratio in X-Plane, steps in FDR file, so convert here

dataref("OAT", "sim/cockpit2/temperature/outside_air_temp_degc") -- Outside Air Temp (deg C)

dataref("wind", "sim/cockpit2/gauges/indicators/wind_heading_deg_mag") -- Wind Angle

dataref("wind_speed", "sim/cockpit2/gauges/indicators/wind_speed_kts") -- Wind Speed

FQty = dataref_table("sim/flightmodel/weight/m_fuel") -- 2.662587 Fuel Qty Left (gal) kg in X-Plane. so this converts gallons to kilograms

bus_volts = dataref_table("sim/cockpit2/electrical/bus_volts") -- volt

battery_amps = dataref_table("sim/cockpit2/electrical/battery_amps") -- amp

oil_pressure_psi = dataref_table("sim/cockpit2/engine/indicators/oil_pressure_psi") -- Oil Pressure (PSI)

oil_temperature_deg_C = dataref_table("sim/cockpit2/engine/indicators/oil_temperature_deg_C") -- Oil Temp (deg F)

ENGN_power = dataref_table("sim/flightmodel/engine/ENGN_power") -- Eng1 Percent Power	watts in X-Plane. so this turns and entry of 100 into 149140 watts, or 200 hp.

prop_speed_rsc = dataref_table("sim/cockpit2/engine/indicators/prop_speed_rsc") -- RPM

MPR_in_hg = dataref_table("sim/cockpit2/engine/indicators/MPR_in_hg") -- Manifold Pressure (In/Hg)

fuel_flow_kg_sec = dataref_table("sim/cockpit2/engine/indicators/fuel_flow_kg_sec") -- 0.0007396 Fuel Flow (gal/hr) kg/sec in X-Plane. so this converts gallons per hour to kilograms per second.

CHT_CYL_deg_C = dataref_table("sim/cockpit2/engine/indicators/CHT_CYL_deg_C") -- Cylinder Head Temp per cylinder, deg C (for multi-engine planes, start second engine at index 12, third engine at index 24 etc)

EGT_CYL_deg_C = dataref_table("sim/cockpit2/engine/indicators/EGT_CYL_deg_C") -- Exhaust Gas Temp cylinder 1, deg C

button_pressed = 0

write_file = 0
file = nil
time = -1


function icxff_on_build(icxff_wnd, x, y)
	if imgui.Button("Start Writing FDR File", 200, 50) then
		write_file = 1

	end
	imgui.SameLine()
	imgui.SetCursorPosX(640)

	if imgui.Button("Stop Writing FDR File", 200, 50) then
		write_file = 0

	end
	
	imgui.TextUnformatted(string.format("\n write_file = %d  time = %d\n" , write_file, time))
	imgui.TextUnformatted("\n")
	imgui.TextUnformatted(string.format("time = %d  LONGITUDE = %f  LATITUDE = %f   Altitude = %.1f   HDG = %.1f   Pitch = %.1f   Roll = %.1f\n\n" , time, LONGITUDE, LATITUDE, Altitude, HDG, Pitch, Roll))
	imgui.TextUnformatted(string.format("BaroA = %.2f  AltMSL = %.1f  VSpd = %.1f   TAS = %.1f   IAS = %.1f   GndSpd = %.1f   Stall_Warning = %.1f\n\n" , BaroA, AltMSL, VSpd, TAS, IAS, GndSpd, Stall_Warning))
	imgui.TextUnformatted(string.format("flap_handle_ratio = %.2f  flap_ratio = %.2f OAT = %.1f  wind = %.1f  wind_speed = %.1f  FQtyL = %.1f  FQtyR = %.1f\n\n" , flap_handle_ratio, flap_ratio, OAT, wind, wind_speed, FQty[0]*0.375574582,FQty[1]*0.375574582))
	imgui.TextUnformatted(string.format("volt1 = %.2f  amp1 = %.1f OilP = %.1f  OilT = %.1f  Eng1_Percent_Power = %.1f  RPM = %.1f  MAPr = %.1f  FFlow = %.1f\n\n" , bus_volts[0], battery_amps[0], oil_pressure_psi[0], oil_temperature_deg_C[0], ENGN_power[0]*0.000670511, prop_speed_rsc[0]*9.549274255, MPR_in_hg[0], fuel_flow_kg_sec[0]*1352))
	imgui.TextUnformatted(string.format("CHT-1 = %.2f CHT-2 = %.1f CHT-3 = %.1f  CHT-4 = %.1f  CHT-5 = %.1f  CHT-6 = %.1f\n\n" , CHT_CYL_deg_C[0], CHT_CYL_deg_C[1], CHT_CYL_deg_C[2], CHT_CYL_deg_C[3], CHT_CYL_deg_C[4], CHT_CYL_deg_C[5]))
	imgui.TextUnformatted(string.format("EGT-1 = %.2f EGT-2 = %.1f EGT-3 = %.1f  EGT-4 = %.1f  EGT-5 = %.1f  EGT-6 = %.1f\n\n" , EGT_CYL_deg_C[0], EGT_CYL_deg_C[1], EGT_CYL_deg_C[2], EGT_CYL_deg_C[3], EGT_CYL_deg_C[4], EGT_CYL_deg_C[5]))	

end

icxff_wnd = nil

function icxff_show_wnd()
    icxff_wnd = float_wnd_create(850, 300, 1, true)
    float_wnd_set_title(icxff_wnd, "Imgui Create XP12 FDR File")
    float_wnd_set_imgui_builder(icxff_wnd, "icxff_on_build")
end

function icxff_hide_wnd()
    if icxff_wnd then
        float_wnd_destroy(icxff_wnd)
    end
end

icxff_show_only_once = 0
icxff_hide_only_once = 0

function toggle_imgui_button_demo()
	icxff_show_window = not icxff_show_window
	if icxff_show_window then
		if icxff_show_only_once == 0 then
			icxff_show_wnd()
			icxff_show_only_once = 1
			icxff_hide_only_once = 0
		end
	else
		if icxff_hide_only_once == 0 then
			icxff_hide_wnd()
			icxff_hide_only_once = 1
			icxff_show_only_once = 0
		end
	end
end


function start_write_data_to_fdr_file()
	if write_file == 1 then
	
		if time == -1  then
			file = io.open("./Output/FDR files/test_flight_" .. PLANE_ICAO .. "_" .. os.date("%Y%m%d%H%M%S") ..".fdr", "w")
			file:write("A\n")
			file:write("3\n")
			file:write("COMM, We need to edit the following path to start with  Aircraft/ not the full path shown\n")
			file:write("ACFT, " .. AIRCRAFT_PATH .. AIRCRAFT_FILENAME .. "\n")
			file:write("TAIL, " .. PLANE_TAILNUMBER .. "\n")
			file:write("TIME, " .. os.date("%H:%m:%S") .. "\n")
			file:write("DATE, " .. os.date("%m/%d/%Y") .. "\n")
			file:write("PRES, " .. math.floor(BaroA*100)/100 .. "\n")
			file:write("DISA, 0\n")
			file:write("WIND, " .. math.floor(wind*1)/1 .. "," .. math.floor(wind_speed*1)/1 .. "\n")
			file:write("COMM, time,Longitude,Latitude,Altitude,HDG,Pitch,Roll,BaroA,AltMSL,VSpd,TAS,IAS,GndSpd,Stall Warning,flap,flap,OAT,wind,wind speed,FQtyL,FQtyR,volt1,amp1,OilP,OilT,Eng1 Percent Power,RPM,MAP,FFlow,CHT-1,CHT-2,CHT-3,CHT-4,CHT-5,CHT-6,EGT-1,EGT-2,EGT-3,EGT-4,EGT-5,EGT-6\n")
		end
		time = time + 1
		
		LONGITUDE = math.floor(LONGITUDE*1000000)/1000000
		LATITUDE = math.floor(LATITUDE*1000000)/1000000
		Altitude = math.floor(Altitude*10)/10
		HDG = math.floor(HDG*10)/10
		Pitch = math.floor(Pitch*10)/10
		Roll = math.floor(Roll*10)/10
		BaroA = math.floor(BaroA*100)/100
		AltMSL = math.floor(AltMSL*10)/10
		VSpd = math.floor(VSpd*10)/10
		TAS = math.floor(TAS*1)/1
		IAS = math.floor(IAS*10)/10
		GndSpd = math.floor(GndSpd*10)/10
		Stall_Warning = math.floor(Stall_Warning*10)/10
		flap_handle_ratio = math.floor(flap_handle_ratio*100)/100
		flap_ratio = math.floor(flap_ratio*100)/100
		OAT = math.floor(OAT*10)/10
		wind = math.floor(wind*1)/1
		wind_speed = math.floor(wind_speed*1)/1
		FQtyL = math.floor(FQty[0]*10)/10
		FQtyL = FQtyL * 0.375574582
		FQtyL = math.floor(FQtyL*10)/10
		FQtyR = math.floor(FQty[1]*10)/10
		FQtyR = FQtyR * 0.375574582
		FQtyR = math.floor(FQtyR*10)/10
		volt1 = math.floor(bus_volts[0]*10)/10
		amp1 = math.floor(battery_amps[0]*10)/10	
		OilP = math.floor(oil_pressure_psi[0]*10)/10
		OilT = math.floor(oil_temperature_deg_C[0]*10)/10
		Eng1_Percent_Power = math.floor(ENGN_power[0]*10)/10
		Eng1_Percent_Power = Eng1_Percent_Power * 0.000670511
		Eng1_Percent_Power = math.floor(Eng1_Percent_Power*10)/10
		RPM = math.floor(prop_speed_rsc[0]*10)/10
		RPM = RPM * 9.549274255
		RPM = math.floor(RPM*10)/10
		MAP = math.floor(MPR_in_hg[0]*10)/10
		FFlow = math.floor(fuel_flow_kg_sec[0]*1000000)/1000000
		FFlow = FFlow * 1352 -- Change to gal/hr from kg/sec
		FFlow = math.floor(FFlow*10)/10
		CHT_1 = math.floor(CHT_CYL_deg_C[0]*10)/10
		CHT_2 = math.floor(CHT_CYL_deg_C[1]*10)/10
		CHT_3 = math.floor(CHT_CYL_deg_C[2]*10)/10
		CHT_4 = math.floor(CHT_CYL_deg_C[3]*10)/10
		CHT_5 = math.floor(CHT_CYL_deg_C[4]*10)/10
		CHT_6 = math.floor(CHT_CYL_deg_C[5]*10)/10
		EGT_1 = math.floor(EGT_CYL_deg_C[0]*10)/10
		EGT_2 = math.floor(EGT_CYL_deg_C[1]*10)/10
		EGT_3 = math.floor(EGT_CYL_deg_C[2]*10)/10
		EGT_4 = math.floor(EGT_CYL_deg_C[3]*10)/10
		EGT_5 = math.floor(EGT_CYL_deg_C[4]*10)/10
		EGT_6 = math.floor(EGT_CYL_deg_C[5]*10)/10
		
		
		if file then -- write only if opened
			file:write("DATA," .. time .. "," .. LONGITUDE .. "," .. LATITUDE .. "," .. Altitude .. "," .. HDG .. "," .. Pitch .. "," .. Roll .. ",")
		end
		
		if file then -- write only if opened
			file:write(BaroA .. "," .. AltMSL .. "," .. VSpd .. "," .. TAS .. "," .. IAS .. "," .. GndSpd .. "," .. Stall_Warning .. ",")
		end
		
		if file then -- write only if opened
			file:write(flap_handle_ratio .. "," .. flap_ratio .. "," .. OAT .. "," .. wind .. "," .. wind_speed .. "," .. FQtyL .. "," .. FQtyR .. ",")
		end
		
		if file then -- write only if opened
			file:write(volt1 .. "," .. amp1 .. "," .. OilP .. "," .. OilT .. "," .. Eng1_Percent_Power .. "," .. RPM .. "," .. MAP .. "," .. FFlow .. ",")
		end
		
		if file then -- write only if opened
			file:write(CHT_1 .. "," .. CHT_2  .. "," .. CHT_3 .. "," .. CHT_4 .. "," .. CHT_5  .. "," .. CHT_6 .. ",")
		end
		
		if file then -- write only if opened
			file:write(EGT_1 .. "," .. EGT_2 .. "," .. EGT_3 .. "," .. EGT_4 .. "," .. EGT_5 .. "," .. EGT_6 .. "\n")
		end
	end
	
end

function stop_write_data_to_fdr_file()
	if write_file == 0 then
		if file then --close only if opened
			file:close()
			file = nil
			time = -1
		end
	
	end

end


do_often("start_write_data_to_fdr_file()")
do_often("stop_write_data_to_fdr_file()")

add_macro("Imgui Create XP12 FDR File: open/close", "icxff_show_wnd()", "icxff_hide_wnd()", "deactivate")
create_command("FlyWithLua/imgui-create-xp12-fdr-file/show_toggle", "open/close imgui button demo", "toggle_imgui_button_demo()", "", "")