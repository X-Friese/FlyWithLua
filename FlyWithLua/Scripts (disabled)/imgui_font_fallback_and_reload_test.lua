-- Fallback Point-Sizing and Mid-Flight Reload Test
-- Positions on the right side of the screen at (760, 100)

if not SUPPORTS_FLOATING_WINDOWS then
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

font_test2_wnd = float_wnd_create(660, 560, 1, true)
float_wnd_set_position(font_test2_wnd, 760, 100)
float_wnd_set_title(font_test2_wnd, "Font Fallback & Mid-Flight Reload Test")
float_wnd_set_imgui_builder(font_test2_wnd, "font_test2_on_build")
float_wnd_set_onclose(font_test2_wnd, "font_test2_on_close")

function font_test2_on_build(wnd, x, y)
    imgui.TextUnformatted("Test Companion: Fallback Rescaling & Mid-Flight Reload")
    imgui.Separator()

    imgui.TextUnformatted("TEST 1: Fallback Point-Size Rescaling")
    imgui.TextUnformatted("When a font is missing, it rescales the default font rather than shrinking to 13px:")

    -- Missing font with -28 suffix: should rescale default font to 28px!
    imgui.PushFont("MissingFont-28")
    imgui.TextUnformatted("  [28px Fallback] MissingFont-28 @ 28px")
    imgui.PopFont()

    -- Missing font with 2nd arg 20: should rescale default font to 20px!
    imgui.PushFont("GhostHeading", 20)
    imgui.TextUnformatted("  [20px Fallback] GhostHeading (arg 20) @ 20px")
    imgui.PopFont()

    -- Standard default font (13px) for comparison
    imgui.PushFont("default")
    imgui.TextUnformatted("  [13px Default] Standard baseline default font for size comparison")
    imgui.PopFont()

    imgui.Separator()
    imgui.TextUnformatted("TEST 2: Mid-Flight Config Injection Test")
    imgui.TextUnformatted("Looking up: 'Font.CustomHero' / 'CustomHero'")

    -- Check if CustomHero is in the registered font list
    local fonts = imgui.get_font_list()
    local has_hero = false
    for _, name in ipairs(fonts) do
        if string.lower(name) == "customhero" or string.lower(name) == "font.customhero" then
            has_hero = true
            break
        end
    end

    if has_hero then
        imgui.PushStyleColor(imgui.constant.Col.Text, 0xFF00FF00) -- Green
        imgui.TextUnformatted(">> STATUS: [PART 2 SUCCESS!] 'Font.CustomHero' is loaded in the font atlas!")
        imgui.PopStyleColor()
        imgui.PushFont("CustomHero")
        imgui.TextUnformatted("  SUCCESS: Rendered in CustomHero (22px Roboto-Light)!")
        imgui.PopFont()
    else
        imgui.PushStyleColor(imgui.constant.Col.Text, 0xFF00FFFF) -- Yellow
        imgui.TextUnformatted(">> STATUS: [PART 1 ACTIVE] 'Font.CustomHero' not in config yet.")
        imgui.PopStyleColor()
        imgui.TextUnformatted("  Notice fallback safely preserves size without crash:")
        imgui.PushFont("CustomHero", 22)
        imgui.TextUnformatted("  Fallback preview: CustomHero requested at 22px")
        imgui.PopFont()
        imgui.Spacing()
        imgui.TextUnformatted("Next step: When you're ready, tell Antigravity in chat.")
        imgui.TextUnformatted("Antigravity will inject the INI font line, and you only need")
        imgui.TextUnformatted("to click 'Plugins -> FlyWithLua -> Reload all Lua script files'!")
    end
end

function font_test2_on_close(wnd)
end
