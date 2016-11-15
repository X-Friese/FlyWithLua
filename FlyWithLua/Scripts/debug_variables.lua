function debug_vars()
	logMsg("FlyWithLua Debug: dumping all variables in current Lua engine:")
	for n,v in pairs(_G) do
		logMsg(tostring(n) .. " = " .. tostring(v))
	end
end

add_macro("Show all variables (debug)", "debug_vars()")