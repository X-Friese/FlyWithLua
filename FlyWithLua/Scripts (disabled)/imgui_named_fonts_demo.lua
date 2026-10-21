-- Named Fonts Demo for FlyWithLua
-- Demonstrates the modern named font registry:
--   imgui.PushFont("Roboto-Regular-16")
--   imgui.PopFont()

if not SUPPORTS_FLOATING_WINDOWS then
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

named_font_demo_wnd = float_wnd_create(780, 580, 1, true)
float_wnd_set_position(named_font_demo_wnd, 120, 120)
float_wnd_set_title(named_font_demo_wnd, "Named Fonts Demo")
float_wnd_set_imgui_builder(named_font_demo_wnd, "named_font_demo_on_build")
float_wnd_set_onclose(named_font_demo_wnd, "named_font_demo_on_close")

function named_font_demo_on_build(wnd, x, y)
    imgui.TextUnformatted("FlyWithLua Modern Named Font Registry Demo")
    imgui.Separator()

    imgui.TextUnformatted("1. Default ImGui fonts:")
    imgui.PushFont("ProggyClean")
    imgui.TextUnformatted("  [ProggyClean 13px] The classic pixel-crisp bitmap font")
    imgui.PopFont()

    imgui.PushFont("ProggyForever")
    imgui.TextUnformatted("  [ProggyForever] Scalable vector default font")
    imgui.PopFont()

    imgui.Separator()
    imgui.TextUnformatted("2. Bundled TTF fonts rendered in their own typeface:")

    imgui.PushFont("Roboto-Light-13")
    imgui.TextUnformatted("  Roboto-Light-13: Sphinx of black quartz, judge my vow. (12345)")
    imgui.PopFont()

    imgui.PushFont("Roboto-Light-16")
    imgui.TextUnformatted("  Roboto-Light-16: Sphinx of black quartz, judge my vow. (12345)")
    imgui.PopFont()

    imgui.PushFont("Roboto-Light-20")
    imgui.TextUnformatted("  Roboto-Light-20: Sphinx of black quartz, judge my vow. (12345)")
    imgui.PopFont()

    imgui.PushFont("Roboto-Regular-13")
    imgui.TextUnformatted("  Roboto-Regular-13: Sphinx of black quartz, judge my vow. (12345)")
    imgui.PopFont()

    imgui.PushFont("Roboto-Regular-16")
    imgui.TextUnformatted("  Roboto-Regular-16: Sphinx of black quartz, judge my vow. (12345)")
    imgui.PopFont()

    imgui.PushFont("Roboto-Regular-20")
    imgui.TextUnformatted("  Roboto-Regular-20: Sphinx of black quartz, judge my vow. (12345)")
    imgui.PopFont()

    imgui.PushFont("Roboto-Medium-13")
    imgui.TextUnformatted("  Roboto-Medium-13: Sphinx of black quartz, judge my vow. (12345)")
    imgui.PopFont()

    imgui.PushFont("Roboto-Medium-16")
    imgui.TextUnformatted("  Roboto-Medium-16: Sphinx of black quartz, judge my vow. (12345)")
    imgui.PopFont()

    imgui.PushFont("Roboto-Medium-20")
    imgui.TextUnformatted("  Roboto-Medium-20: Sphinx of black quartz, judge my vow. (12345)")
    imgui.PopFont()

    imgui.Separator()
    imgui.TextUnformatted("3. Dynamic sizing via 2nd parameter:")
    imgui.PushFont("Roboto-Regular", 26)
    imgui.TextUnformatted("  Roboto-Regular scaled to 26px on the fly!")
    imgui.PopFont()

    imgui.Separator()
    imgui.TextUnformatted("4. Backward compatibility with legacy slot numbers:")
    imgui.PushFont(1)
    imgui.TextUnformatted("  Legacy slot 1 via imgui.PushFont(1)")
    imgui.PopFont()
    imgui_push_font(5)
    imgui.TextUnformatted("  Legacy slot 5 via imgui_push_font(5)")
    imgui.PopFont()

    imgui.Separator()
    if imgui.TreeNode("Show all registered fonts (rendered in each font)") then
        local fonts = imgui.get_font_list()
        for i, name in ipairs(fonts) do
            imgui.PushFont(name)
            imgui.BulletText(string.format("%s: Quick brown fox 12345", name))
            imgui.PopFont()
        end
        imgui.TreePop()
    end
end

function named_font_demo_on_close(wnd)
end
