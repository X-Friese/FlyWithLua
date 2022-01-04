dataref("FRAMERATE_PERIOD", "sim/time/framerate_period", "readonly")

TARGET_FPS = 25.0
FPS = 0
SLEEP_MILLIS = 0
ENABLED = 0

add_macro("FPS Limit", "ENABLED = 1", "ENABLED = 0", "deactivate")

do_every_frame("every_frame()")

function every_frame()
    if ENABLED == 1 then
        calc_limit()
        limit()
    end
end

function limit()
    if SLEEP_MILLIS > 0 then
        sleep(SLEEP_MILLIS)
    end
end

function calc_limit()
    FPS = 1.0 / FRAMERATE_PERIOD

    if TARGET_FPS < FPS then
        SLEEP_MILLIS = SLEEP_MILLIS + 1000
    elseif TARGET_FPS > FPS then
        SLEEP_MILLIS = SLEEP_MILLIS - 1000
    end

    if SLEEP_MILLIS < 0 then
        SLEEP_MILLIS = 0
    end
end
