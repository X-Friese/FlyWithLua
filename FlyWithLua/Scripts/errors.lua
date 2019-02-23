failure_mode = 8

if failure_mode == 1 then
    local fail
    fail()
elseif failure_mode == 2 then
    error("manually triggered error")
elseif failure_mode == 3 then
    crash_lua_legacy()
elseif failure_mode == 4 then
    crash_lua_legacy_exception()
elseif failure_mode == 5 then
    crash_lua_exception()
elseif failure_mode == 6 then
    local function f(arg)
        f(arg)
    end
    f("Crash and burn!")
elseif failure_mode == 7 then
    do_often("print('Will error now!')")
    do_often("error('manually triggered error')")
elseif failure_mode == 8 then
    function crash_and_burn()
        print('Will error now!')
        error('manually triggered error')
    end
    do_often("crash_and_burn()")
end