dataref("first_xp_ENGN_oil_temp", "sim/flightmodel/engine/ENGN_oil_temp", "readonly", 0)
dataref("second_xp_ENGN_oil_temp", "sim/flightmodel/engine/ENGN_oil_temp", "readonly", 1)
dataref("first_xp_ENGN_cowl", "sim/flightmodel/engine/ENGN_cowl", "writable", 0)
dataref("second_xp_ENGN_cowl", "sim/flightmodel/engine/ENGN_cowl", "writable", 1)


function automatic_cowl_flaps()
	-- check and set the first engine
	if first_xp_ENGN_oil_temp < 0.35 then
		first_xp_ENGN_cowl = 0
	elseif first_xp_ENGN_oil_temp >= 0.35 and first_xp_ENGN_oil_temp < 0.65 then
		first_xp_ENGN_cowl = first_xp_ENGN_oil_temp * 3.3333333 - 1.1166666
	else
		first_xp_ENGN_cowl = 1
	end
	-- check and set the second engine
	if second_xp_ENGN_oil_temp < 0.35 then
		second_xp_ENGN_cowl = 0
	elseif second_xp_ENGN_oil_temp >= 0.35 and second_xp_ENGN_oil_temp < 0.65 then
		second_xp_ENGN_cowl = second_xp_ENGN_oil_temp * 3.3333333 - 1.1166666
	else
		second_xp_ENGN_cowl = 1
	end
end

do_often("automatic_cowl_flaps()")