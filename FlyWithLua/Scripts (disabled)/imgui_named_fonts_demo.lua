-- Named Fonts Demo for FlyWithLua
-- Demonstrates the modern named font registry:
--   imgui.PushFont("Roboto-Regular-16")
--   imgui.PopFont()

if not SUPPORTS_FLOATING_WINDOWS then
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

named_font_demo_wnd = float_wnd_create(600, 480, 1, true)
float_wnd_set_position(named_font_demo_wnd, 150, 150)
float_wnd_set_title(named_font_demo_wnd, "Named Fonts Demo")
float_wnd_set_imgui_builder(named_font_demo_wnd, "named_font_demo_on_build")
float_wnd_set_onclose(named_font_demo_wnd, "named_font_demo_on_close")

function named_font_demo_on_build(wnd, x, y)
    imgui.TextUnformatted("FlyWithLua Modern Named Font Registry Demo")
    imgui.Separator()

    imgui.TextUnformatted("1. Default fonts:")
    imgui.PushFont("ProggyClean")
    imgui.TextUnformatted("  ProggyClean: The classic bitmap font (13px)")
    imgui.PopFont()

    imgui.PushFont("ProggyForever")
    imgui.TextUnformatted("  ProggyForever: The scalable vector default font")
    imgui.PopFont()

    imgui.Separator()
    imgui.TextUnformatted("2. Bundled TTF fonts at standard sizes:")

    imgui.PushFont("Roboto-Light-13")
    imgui.TextUnformatted("  Roboto-Light (13px)")
    imgui.PopFont()

    imgui.PushFont("Roboto-Light-16")
    imgui.TextUnformatted("  Roboto-Light (16px)")
    imgui.PopFont()

    imgui.PushFont("Roboto-Light-20")
    imgui.TextUnformatted("  Roboto-Light (20px)")
    imgui.PopFont()

    imgui.PushFont("Roboto-Regular-13")
    imgui.TextUnformatted("  Roboto-Regular (13px)")
    imgui.PopFont()

    imgui.PushFont("Roboto-Regular-16")
    imgui.TextUnformatted("  Roboto-Regular (16px)")
    imgui.PopFont()

    imgui.PushFont("Roboto-Regular-20")
    imgui.TextUnformatted("  Roboto-Regular (20px)")
    imgui.PopFont()

    imgui.PushFont("Roboto-Medium-13")
    imgui.TextUnformatted("  Roboto-Medium (13px)")
    imgui.PopFont()

    imgui.PushFont("Roboto-Medium-16")
    imgui.TextUnformatted("  Roboto-Medium (16px)")
    imgui.PopFont()

    imgui.PushFont("Roboto-Medium-20")
    imgui.TextUnformatted("  Roboto-Medium (20px)")
    imgui.PopFont()

    imgui.Separator()
    imgui.TextUnformatted("3. Dynamic sizing via 2nd parameter:")
    imgui.PushFont("Roboto-Regular", 24)
    imgui.TextUnformatted("  Roboto-Regular dynamically scaled to 24px")
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
    if imgui.TreeNode("Show all registered fonts") then
        local fonts = imgui.get_font_list()
        for i, name in ipairs(fonts) do
            imgui.BulletText(name)
        end
        imgui.TreePop()
    end
end

function named_font_demo_on_close(wnd)
end
