DataRefName1 = "sim/version/xplm_build_string"

if XPLMFindDataRef(DataRefName1) then
    testing_result1 = '"sim/version/xplm_build_string" is an existing DataRef!'
else
    testing_result1 = '"sim/version/xplm_build_string" is not an existing DataRef!'
end

DataRefName2 = "sim/sala/bim"

if XPLMFindDataRef(DataRefName2) then
    testing_result2 = '"sim/sala/bim" is an existing DataRef!'
else
    testing_result2 = '"sim/sala/bim" is not an existing DataRef!'
end

function check_dataref()
    x, y = MOUSE_X, MOUSE_Y
    y = bubble(x, y, DataRefName1, tostring(XPLMFindDataRef( DataRefName1 )))
    y = bubble(x, y, 'Testing "' .. DataRefName1 .. '"', testing_result1)
    y = bubble(x, y, DataRefName2, tostring(XPLMFindDataRef( DataRefName2 )))
    y = bubble(x, y, 'Testing "' .. DataRefName2 .. '"', testing_result2)
end

do_every_draw("check_dataref()")