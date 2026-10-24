-- ImGui Contrib Extensions & Modern Widgets Demo for FlyWithLua
-- Demonstrates:
--   1. ImGui::Markdown:: (Rich text, headers, emphasis, pills, badges, links, fenced code blocks)
--   2. ImGui::TimedTooltip:: (Stationary, non-jittering, reading-adaptive tooltips)
--   3. Modern Contrib Widgets (ToggleButton, ToggleButtons, Sliders with Reset, Mouse Wheel Chasers, Spinners, Dials, Links)
--   4. Universal PascalCase & C++ Alias Hierarchy (MD, Markdown, TimedTooltip, Spinners)

if not SUPPORTS_FLOATING_WINDOWS then
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

local contrib_demo_wnd = float_wnd_create(860, 740, 1, true)
float_wnd_set_position(contrib_demo_wnd, 80, 80)
float_wnd_set_title(contrib_demo_wnd, "FlyWithLua ImGui Contrib & UX Demo")
float_wnd_set_imgui_builder(contrib_demo_wnd, "contrib_demo_on_build")
float_wnd_set_onclose(contrib_demo_wnd, "contrib_demo_on_close")

-- Configure responsive tooltips (~0.4s delay at 60fps instead of 1.1s)
imgui.TimedTooltip.SetHoverDelay(25)

-- State variables for interactive widgets
local avionics_master_state = true
local deice_mode            = 1  -- 0 = OFF, 1 = AUTO, 2 = MAX
local throttle_trim         = 0.15
local flaps_pct             = 0.40
local hud_brightness        = 0.75
local com1_freq             = 121.50
local altimeter_setting     = 29.92
local camera_view_idx       = 1
local camera_views          = { "Cockpit 3D", "Chase Plane", "Tower View", "Fly-By Camera", "Free Cam" }
local md_button_clicks      = 0
local demo_spinner_speed    = 1.0
local demo_timer_val        = 0.65
local demo_countdown_cur    = 42.0
local demo_countdown_max    = 60.0
local demo_countdown_running = true
local last_timer_tick       = os.clock()

function contrib_demo_on_build(wnd, x, y)
    -- Update live real-time countdown timer tick
    local now = os.clock()
    local dt = now - (last_timer_tick or now)
    last_timer_tick = now
    if dt > 0 and dt < 0.5 and demo_countdown_running then
        demo_countdown_cur = demo_countdown_cur - dt
        if demo_countdown_cur <= 0 then
            demo_countdown_cur = demo_countdown_max
        end
    end

    -- Ambient proportional typography for clean baseline alignment across widgets & text
    imgui.PushFont("Roboto-Regular-16")

    -- Header Title using Markdown H1
    imgui.MD.Text("# FlyWithLua ImGui Contrib Extensions")
    imgui.MD.Text("Modern rich formatting, smart stationary tooltips, and interactive UX widgets.")
    imgui.Separator()

    if imgui.BeginTabBar("ContribDemoTabs") then

        -- ====================================================================
        -- TAB 1: Markdown Extension
        -- ====================================================================
        if imgui.BeginTabItem("Rich Markdown (MD)") then
            imgui.Spacing()
            imgui.MD.Text("The Markdown extension transforms standard Markdown text into styled Dear ImGui elements:")

            if imgui.CollapsingHeader("Headings & Typography", 32) then -- 32 = ImGuiTreeNodeFlags_DefaultOpen
                imgui.MD.Text("<code dark># Heading 1 (20px Roboto-Medium with full line beneath)</code>")
                imgui.Indent(40)
                imgui.MD.Text("# Heading 1 (20px Roboto-Medium with full line beneath)")
                imgui.Unindent(40)
                imgui.Spacing()

                imgui.MD.Text("<code dark>## Heading 2 (18px Roboto-Medium with full line beneath)</code>")
                imgui.Indent(40)
                imgui.MD.Text("## Heading 2 (18px Roboto-Medium with full line beneath)")
                imgui.Unindent(40)
                imgui.Spacing()

                imgui.MD.Text("<code dark>### Heading 3 (16px Roboto-Medium)</code>")
                imgui.Indent(40)
                imgui.MD.Text("### Heading 3 (16px Roboto-Medium)")
                imgui.Unindent(40)
                imgui.Spacing()

                imgui.MD.Text("<code dark>*italic*, _italic_, **bold**, __bold__</code>")
                imgui.Indent(40)
                imgui.MD.Text("Body paragraph with *italic text*, _underscore italic_, and **bold text**.")
                imgui.Unindent(40)
                imgui.Spacing()

                imgui.MD.Text("<code dark>`sim/cockpit2/engine/indicators/engine_speed_rpm`</code>")
                imgui.Indent(40)
                imgui.MD.Text("Monospace inline code: `sim/cockpit2/engine/indicators/engine_speed_rpm`")
                imgui.Unindent(40)
            end

            imgui.Spacing()
            if imgui.CollapsingHeader("Pills, Badges & Custom Colors", 32) then
                imgui.MD.Text("Enhance telemetry and cockpit status displays using inline XML-style tags:")
                imgui.Spacing()
                imgui.MD.Text("Status Pills:")
                imgui.MD.Text("  `<pill green>SYSTEMS NORMAL</pill>`")
                imgui.SameLine(360)
                imgui.MD.Text("<pill green>SYSTEMS NORMAL</pill>")

                imgui.MD.Text("  `<pill warning>LOW FUEL</pill>`")
                imgui.SameLine(360)
                imgui.MD.Text("<pill warning>LOW FUEL</pill>")

                imgui.MD.Text("  `<pill error>ENGINE 2 FIRE</pill>`")
                imgui.SameLine(360)
                imgui.MD.Text("<pill error>ENGINE 2 FIRE</pill>")

                imgui.MD.Text("  `<pill gray>OFFLINE</pill>`")
                imgui.SameLine(360)
                imgui.MD.Text("<pill gray>OFFLINE</pill>")

                imgui.Spacing()
                imgui.MD.Text("Badges:")
                imgui.MD.Text("  `<badge blue>V2.2.1</badge>`")
                imgui.SameLine(360)
                imgui.MD.Text("<badge blue>V2.2.1</badge>")

                imgui.MD.Text("  `<badge purple>PRO</badge>`")
                imgui.SameLine(360)
                imgui.MD.Text("<badge purple>PRO</badge>")

                imgui.MD.Text("  `<badge green>XP12.4.4+</badge>`")
                imgui.SameLine(360)
                imgui.MD.Text("<badge green>XP12.4.4+</badge>")

                imgui.Spacing()
                imgui.MD.Text("Keyboard shortcuts:")
                imgui.MD.Text("  `<kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>R</kbd>`")
                imgui.SameLine(395)
                imgui.MD.Text("<kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>R</kbd>")

                imgui.Spacing()
                imgui.MD.Text("Custom Hex Colors:")
                imgui.MD.Text("  `<color #00bc8c>Emerald</color>`")
                imgui.SameLine(360)
                imgui.MD.Text("<color #00bc8c>Emerald</color>")

                imgui.MD.Text("  `<color #3498db>Aviation Sky</color>`")
                imgui.SameLine(360)
                imgui.MD.Text("<color #3498db>Aviation Sky</color>")

                imgui.MD.Text("  `<color #e74c3c>Crimson Warning</color>`")
                imgui.SameLine(360)
                imgui.MD.Text("<color #e74c3c>Crimson Warning</color>")
            end

            imgui.Spacing()
            if imgui.CollapsingHeader("Lists, Tables & Variadic Formatting", 32) then
                imgui.MD.Text("Pre-flight Checklist:")
                imgui.MD.Text("* Pitot tube covers removed\n* Flight control surfaces free and clear\n* Fuel quantity verified\n* Transponder set to <pill blue>1200 / VFR</pill>")

                imgui.Spacing()
                imgui.MD.Text("Variadic string formatting (pass %s, %d, %f like string.format):")
                imgui.MD.Text("<code dark>imgui.MD.Text(\"**Alt**: %d ft | **IAS**: %.1f kts\", 18500, 264.8)</code>")
                imgui.Indent(40)
                imgui.MD.Text("Telemetry readout: **Alt**: %d ft | **IAS**: %.1f kts | **QNH**: %.2f inHg", 18500, 264.8, altimeter_setting)
                imgui.Unindent(40)
            end

            imgui.Spacing()
            if imgui.CollapsingHeader("Multi-Line Fenced Code Blocks (CommonMark)", 32) then
                imgui.MD.Text("Multi-line code snippets render in shaded charcoal containers with automatic language chips:")
                imgui.Spacing()
                imgui.MD.Text([[
```lua
-- Autopilot vertical speed calculation
if is_autopilot_engaged then
    local delta_alt = target_alt - current_alt
    local target_vvi = math.min(2000, delta_alt * 0.1)
    set("sim/cockpit2/autopilot/vvi_dial_fpm", target_vvi)
end
```
]])
                imgui.Spacing()
                imgui.MD.Text([[
```cpp
    // 1. Procedural Activity Spinner
    if (is_fms_computing) {
        ImGui::Spinner("FMS Processing", 14.0f, 3.0f, ImColor(30, 144, 255));
        ImGui::SameLine();
        ImGui::TextDisabled("Calculating RNAV approach path...");
    }

    // 2. Continuous Radial Fuel Sweep
    for (int tank = 0; tank < num_tanks; ++tank) {
        float fuel_frac = fuel_remaining[tank] / tank_capacity[tank];
        ImGui::RadialTimer("Fuel Level", fuel_frac, 16.0f, 3.5f, ImColor(50, 205, 50));
    }
```
]])
            end

            imgui.Spacing()
            if imgui.CollapsingHeader("Clickable Links & Markdown Buttons", 32) then
                imgui.MD.Text("<code dark>{FlyWithLua GitHub Project}(https://github.com/sparker256/FlyWithLua)</code>")
                imgui.Indent(40)
                imgui.MD.Text("Clickable URL Link: {FlyWithLua GitHub Project}(https://github.com/sparker256/FlyWithLua)")
                imgui.MD.Text("*(Clicking the link above automatically opens your system default web browser!)*")
                imgui.Unindent(40)

                imgui.Spacing()
                imgui.MD.Text("<code dark>imgui.Markdown.ButtonMD(\"Execute Diagnostic Check##mddemo\", rich_tooltip_markdown)</code>")
                imgui.Indent(40)
                local btn_tip = string.format("### Diagnostic Check\nTotal clicks registered: **%d**\nStatus: <pill green>READY</pill>", md_button_clicks)
                if imgui.Markdown.ButtonMD(string.format("Execute Check (Clicks: %d)##mddemo", md_button_clicks), btn_tip) then
                    md_button_clicks = md_button_clicks + 1
                end
                imgui.TimedTooltip.Text("Click to advance diagnostic execution cycle.")
                imgui.MD.Text("*(Hover over the button above to view its rich Markdown tooltip!)*")
                imgui.Unindent(40)
            end

            imgui.EndTabItem()
        end

        -- ====================================================================
        -- TAB 2: TimedTooltips
        -- ====================================================================
        if imgui.BeginTabItem("Timed Tooltips") then
            imgui.Spacing()
            imgui.MD.Text("### Why TimedTooltip is Superior to Basic ImGui Tooltips:")
            imgui.MD.Text("1. **Stationary Anchor:** Does not jitter, jump, or bounce as the mouse moves over the widget.\n" ..
                          "2. **Reading-Adaptive Timeout:** Dwell duration scales automatically with content word count.\n" ..
                          "3. **Style Isolation:** Immune to ambient layout padding, spacing, or color push pollution.\n" ..
                          "4. **Full Markdown Support:** Use headings, pills, badges, and colors inside tooltips!")

            imgui.Separator()
            imgui.Spacing()

            -- Demo 1: Plain stationary tooltip
            imgui.Button("Hover over me (Plain TimedTooltip)##btn1", 320, 36)
            imgui.TimedTooltip.Text("This tooltip is locked in place and will not follow the cursor around!")
            imgui.SameLine()
            imgui.MD.Text("<code dark>imgui.TimedTooltip.Text(\"...\")</code>")

            imgui.Spacing()

            -- Demo 2: Rich Markdown tooltip (Moved 2nd per feedback: "Oh my god! It's colored text!")
            imgui.Button("Hover over me (Rich Markdown Tooltip)##btn2", 320, 42)
            imgui.TimedTooltip.MD.Text("## Autopilot Flight Director\n" ..
                                       "Current status: <pill green>ARMED</pill>\n" ..
                                       "* Lateral Mode: **LNAV (GPS)**\n" ..
                                       "* Vertical Mode: **VNAV PATH**\n\n" ..
                                       "<color #b71c1c>**CAUTION: Barometric transition at 18,000 ft.**</color>")
            imgui.SameLine()
            imgui.BeginGroup()
            imgui.MD.Text("<code dark>imgui.TimedTooltip.MD.Text(\"## Autopilot Flight Director\\n\" ..</code>")
            imgui.MD.Text("<code dark>  \"Current status: <pill green>ARMED</pill>\\n* Lateral: **LNAV**...\")</code>")
            imgui.EndGroup()

            imgui.Spacing()

            -- Demo 3: Formatted stationary tooltip (Moved 3rd)
            imgui.Button("Hover over me (Variadic Formatted)##btn3", 320, 36)
            imgui.TimedTooltip.Text("Active Transponder: %04d | Mode: %s | Reply: %s", 7700, "ALT", "ENGAGED")
            imgui.SameLine()
            imgui.MD.Text("<code dark>imgui.TimedTooltip.Text(\"Active Transponder...\", 7700, ...)</code>")

            imgui.Spacing()
            imgui.Separator()
            imgui.MD.Text("Dwell timing configuration methods:\n" ..
                          "* `imgui.TimedTooltip.SetHoverDelay(cycles)` -- Delay before appearing (e.g. 25 cycles = ~0.4s)\n" ..
                          "* `imgui.TimedTooltip.SetMaxDisplay(cycles)` -- Maximum display timeout\n" ..
                          "* `imgui.TimedTooltip.SetMaxWidth(pixels)`   -- Maximum text wrap width")

            imgui.EndTabItem()
        end

        -- ====================================================================
        -- TAB 3: Modern Contrib Widgets
        -- ====================================================================
        if imgui.BeginTabItem("Modern Widgets") then
            imgui.Spacing()
            imgui.MD.Text("### Smooth Animated & Specialized Widgets")
            imgui.MD.Text("*(Tip: Mouse-wheel directly over switches flips states; clicking toggles via either the buttons or labels)*")
            imgui.Spacing()

            -- Row 1: Avionics Master & De-Ice System (Side-by-side)
            local toggle_changed, new_toggle = imgui.ToggleButton("Avionics Master Switch", avionics_master_state)
            if toggle_changed then
                avionics_master_state = new_toggle
            end
            imgui.TimedTooltip.Text("Smooth animated slide toggle switch with tactile color transition.")

            imgui.SameLine()
            if avionics_master_state then
                imgui.MD.Text("<pill green>ONLINE</pill>")
            else
                imgui.MD.Text("<pill gray>OFFLINE</pill>")
            end

            imgui.SameLine(420)
            imgui.TextUnformatted("De-Ice System:")
            imgui.SameLine()
            local seg_changed, new_seg = imgui.ToggleButtons("##DeIceSeg", deice_mode, "OFF", "AUTO")
            if seg_changed then
                deice_mode = new_seg
            end
            imgui.TimedTooltip.Text("Segmented multi-state toggle selector.")

            imgui.Spacing()
            imgui.Separator()

            -- Row 2: Sliders with Reset and Normalized Percentage
            local trim_changed, new_trim = imgui.SliderWithReset("Elevator Trim", throttle_trim, -1.0, 1.0, 0.0)
            if trim_changed then
                throttle_trim = new_trim
            end
            imgui.TimedTooltip.Text("Click the reset button (↺) to snap trim back to neutral (0.00).")

            local flaps_changed, new_flaps = imgui.PercentSlider("Flaps Position", flaps_pct)
            if flaps_changed then
                flaps_pct = new_flaps
            end
            imgui.TimedTooltip.Text("Renders values normalized between 0.0 and 1.0 formatted as a clean percentage (0%..100%).")

            local hud_changed, new_hud = imgui.PercentSliderWithReset("HUD Brightness", hud_brightness, 0.50)
            if hud_changed then
                hud_brightness = new_hud
            end
            imgui.TimedTooltip.Text("Percent slider with built-in reset button defaulting to 50%.")

            imgui.Spacing()
            imgui.Separator()

            -- Row 3: Mouse Wheel Chasers (Side-by-side to eliminate vertical overflow)
            imgui.MD.Text("### Mouse Wheel Chasers")
            imgui.TextUnformatted("Hover your mouse cursor over the boxes below and scroll the mouse wheel:")

            imgui.Button(string.format("COM1 Active: %.3f MHz (Scroll Wheel)##combtn", com1_freq), 360, 32)
            local freq_changed, new_freq = imgui.AdjustItemOnMouseWheel(com1_freq, 118.00, 136.975, 0.025)
            if freq_changed then
                com1_freq = new_freq
            end
            imgui.TimedTooltip.Text("Scroll up/down with mouse wheel over this button to tune COM1 by 25 kHz!")

            imgui.SameLine(400)
            local cur_view_name = camera_views[camera_view_idx] or "Unknown"
            imgui.Button(string.format("Camera: [%s] (Scroll to Cycle)##cambtn", cur_view_name), 360, 32)
            local cam_changed, new_cam = imgui.CycleItemOnMouseWheel(camera_view_idx, #camera_views, true)
            if cam_changed then
                camera_view_idx = new_cam
            end
            imgui.TimedTooltip.Text("Scroll mouse wheel over this button to cycle through camera views!")

            imgui.Spacing()
            imgui.Separator()

            -- Row 4: Procedural Activity Spinners & Countdown Dials
            imgui.MD.Text("### Procedural Activity Spinners & Countdown Dials")
            imgui.MD.Text("Vector antialiased paths via ImDrawList -- no textures, no font dependencies, smooth organic easing:")
            imgui.Spacing()

            -- Spinner 1: Custom size, thickness, color table, and speed
            imgui.Spinner("FMS Navdata Ingestion (1.2x Speed)", 14, 3, {0.2, 0.7, 1.0, 1.0}, demo_spinner_speed)
            imgui.TimedTooltip.Text("Organic sinusoidal expanding/contracting arc animated automatically by ImGui::GetTime().")

            imgui.SameLine(400)
            -- Spinner 2: Compact default theme
            imgui.Spinner("Background Telemetry Sync", 10, 2.0)
            imgui.TimedTooltip.Text("Compact spinner using default theme color and 1.0x animation speed.")

            imgui.Spacing()

            -- Radial Timer Dial: Sweeps progress clockwise from 12 o'clock
            imgui.RadialTimer(string.format("Fuel Burn Cycle (%.0f%%)", demo_timer_val * 100), demo_timer_val, 16, 3.5, {0.2, 0.85, 0.3, 1.0})
            imgui.TimedTooltip.Text("Radial progress dial: sweeps from 12 o'clock clockwise based on fractional progress (0.0 .. 1.0).")

            imgui.SameLine(400)
            imgui.SetNextItemWidth(-1)
            local timer_changed, new_timer = imgui.PercentSlider("##tslider", demo_timer_val)
            if timer_changed then demo_timer_val = new_timer end

            imgui.Spacing()

            -- Countdown Timer Helper: Seconds elapsed/remaining
            imgui.CountdownTimer(string.format("Approach Timer (%.1fs / %.0fs)", demo_countdown_cur, demo_countdown_max), demo_countdown_cur, demo_countdown_max, 16, 3.5, {1.0, 0.65, 0.0, 1.0})
            imgui.TimedTooltip.Text("Countdown timer: accepts remaining seconds and max seconds, sweeping down smoothly.")

            imgui.SameLine(400)
            imgui.SetNextItemWidth(-1)
            local cd_changed, new_cd = imgui.SliderWithReset("##cdslider", demo_countdown_cur, 0.0, demo_countdown_max, 42.0)
            if cd_changed then demo_countdown_cur = new_cd end

            imgui.Spacing()
            imgui.Separator()

            -- Row 5: ImGui Mouse Cursors & Hyperlinks (Combined clean footer)
            imgui.MD.Text("### ImGui Mouse Cursors & Hyperlinks")
            local cur_cursors = float_wnd_get_imgui_cursor_handling(wnd)
            local cursor_changed, new_cursor_state = imgui.ToggleButton("Dynamic ImGui Cursors", cur_cursors)
            if cursor_changed then
                float_wnd_set_imgui_cursor_handling(wnd, new_cursor_state)
            end
            imgui.TimedTooltip.Text("Toggles dynamic ImGui cursors (Hand over links/buttons, I-beam over text) vs native hardware mouse.")

            imgui.SameLine(360)
            imgui.TextLink("X-Plane Official Org Portal", "https://forums.x-plane.org")
            imgui.SameLine()
            imgui.TextUnformatted(" | ")
            imgui.SameLine()
            imgui.ButtonLink("FlyWithLua Issues & Discussions", "https://github.com/sparker256/FlyWithLua/issues")

            imgui.EndTabItem()
        end

        -- ====================================================================
        -- TAB 4: Universal C++ & Lua Alias Matrix
        -- ====================================================================
        if imgui.BeginTabItem("C++ / Lua Aliases") then
            imgui.Spacing()
            imgui.MD.Text("### Universal Syntax & PascalCase Parity")
            imgui.MD.Text("FlyWithLua mirrors the C++ Dear ImGui naming conventions and provides complete\nalias parity so script authors can write whatever feels most natural:")

            imgui.Spacing()
            imgui.TextUnformatted("All of these equivalent forms work identically:")

            -- Showcase Markdown aliases
            imgui.Bullet()
            imgui.Markdown.Text("`imgui.Markdown.Text(...)` -- Canonical PascalCase")

            imgui.Bullet()
            imgui.MD.Text("`imgui.MD.Text(...)` -- Short namespace alias")

            imgui.Bullet()
            Markdown.Text("`Markdown.Text(...)` -- Global scope PascalCase")

            imgui.Bullet()
            MD.Text("`MD.Text(...)` -- Global short alias")

            imgui.Bullet()
            imgui.TextMD("`imgui.TextMD(...)` -- Suffix alias")

            imgui.Spacing()
            imgui.Separator()
            imgui.TextUnformatted("TimedTooltip aliases (hover each bullet to preview):")

            imgui.Bullet()
            imgui.MD.Text("`<code dark>imgui.TimedTooltip.Text(...)</code>` -- Canonical namespace")
            imgui.TimedTooltip.Text("Active via imgui.TimedTooltip.Text()")

            imgui.Bullet()
            imgui.MD.Text("`<code dark>TimedTooltip.Text(...)</code>` -- Global scope alias")
            TimedTooltip.Text("Active via TimedTooltip.Text()")

            imgui.Bullet()
            imgui.MD.Text("`<code dark>imgui.TimedTooltip.MD.Text(...)</code>` -- Compound TimedTooltip Markdown")
            imgui.TimedTooltip.MD.Text("<pill green>Active</pill> via **imgui.TimedTooltip.MD.Text()**")

            imgui.Bullet()
            imgui.MD.Text("`<code dark>TimedTooltip.MD.Text(...)</code>` -- Global TimedTooltip Markdown")
            TimedTooltip.MD.Text("<pill blue>Active</pill> via **TimedTooltip.MD.Text()**")

            imgui.Spacing()
            imgui.Separator()
            imgui.TextUnformatted("Procedural Spinner & Timer aliases:")

            imgui.Bullet()
            imgui.MD.Text("`imgui.Spinner(label, [r, thick, col, spd])` / `imgui.spinner`")

            imgui.Bullet()
            imgui.MD.Text("`imgui.RadialTimer(label, frac, [r, thick, col, bg_col, cw])` / `imgui.radial_timer`")

            imgui.Bullet()
            imgui.MD.Text("`imgui.CountdownTimer(label, cur_sec, max_sec, ...)` / `imgui.countdown_timer`")

            imgui.EndTabItem()
        end

        imgui.EndTabBar()
    end

    imgui.PopFont()
end

function contrib_demo_on_close(wnd)
end
