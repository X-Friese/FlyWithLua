-- ============================================================================
-- FlyWithLua: LuiGee Modern UI & Flight-Deck UX Demo
-- ============================================================================
-- Demonstrates:
--   1. ImGui::Markdown:: (Rich text, headers, emphasis, pills, badges, links, GFM tables)
--   2. ImGui::TimedTooltip:: (Stationary, non-jittering, reading-adaptive tooltips)
--   3. Tactile Avionics Widgets (ToggleButton, Rotary Knobs, Sliders with Reset, Pulldowns)
--   4. Smart Typography (Inline sizing tags <tiny>, <small>, <regular>, <big>, <huge>)
--   5. Semantic C++ & Lua Helpers (SmallText, TinyText, TitleText, MonoText)
--   6. Dynamic Proportional Window Zoom & Aspect Ratio Scaling (50% to 200%)
--   7. Zero-Disk In-Memory GPU Texture Service (imgui.RegisterMemoryImage)
-- ============================================================================

if not SUPPORTS_FLOATING_WINDOWS then
    logMsg("imgui not supported by your FlyWithLua version")
    return
end

local screen_w, screen_h = 1920, 1080
if XPLMGetScreenSize then
    screen_w, screen_h = XPLMGetScreenSize()
end

local baseline_w   = 860
local baseline_h   = 620
local min_header_w = 686  -- Minimum width to cleanly fit title, about, theme, and full zoom suite edge-to-edge without overlap
local min_header_h = 420
local demo_x = math.max(40, math.floor((screen_w - baseline_w) * 0.15))
local demo_y = math.max(30, math.floor((screen_h - baseline_h) * 0.15))

local luigee_demo_wnd = nil
local saved_wnd_x     = nil
local saved_wnd_y     = nil
local saved_wnd_w     = baseline_w
local saved_wnd_h     = baseline_h

-- Zoom & Proportional Scaling State (range: 50% to 200%, 100% baseline)
local content_zoom           = 1.0
local zoom_pct               = 100
local last_zoom              = 1.0
local last_frame_time        = nil
local prior_zoom_rect        = nil  -- { l, t, w, h }

-- Release-to-resize & debounce timers
local slider_start_zoom      = 1.0
local wheel_quiet_timer      = 0.0
local wheel_start_zoom       = 1.0
local wheel_resize_pending   = false
local stepper_quiet_timer    = 0.0
local stepper_start_zoom     = 1.0
local stepper_resize_pending = false
local dec_repeat_count       = 0
local inc_repeat_count       = 0

-- "About LuiGee" Dialog State (opens automatically on first display)
local show_about_popup       = false
local first_run_about        = true

-- Theme Preset Selection State (Charcoal, Rusty, Classic Dark)
local active_theme_idx          = 0
local active_theme_name         = "Theme: Charcoal"
local current_applied_theme_idx = nil
local theme_names = {
    [0] = "Theme: Charcoal",
    [1] = "Theme: Rusty",
    [2] = "Theme: Dark"
}

local function push_button_repeat(enable)
    if imgui.PushButtonRepeat then
        imgui.PushButtonRepeat(enable)
    end
end

local function pop_button_repeat()
    if imgui.PopButtonRepeat then
        imgui.PopButtonRepeat()
    end
end

-- Ensure TextStyledWrapped and styled_text_wrapped perform true word wrapping across inline styled tags
imgui.TextStyledWrapped = function(fmt, ...)
    local str = fmt
    if select("#", ...) > 0 then
        str = string.format(fmt, ...)
    end
    -- Convert HTML-like tags (<i>, <b>, </i>, </b>) to markdown (*, **) so ImGui::Markdown::TextWrapped
    -- parses emphasis and performs true word wrapping across styled tokens!
    local md_str = str:gsub("<b>", "**"):gsub("</b>", "**")
                      :gsub("<i>", "*"):gsub("</i>", "*")
    if imgui.MD and imgui.MD.TextWrapped then
        imgui.MD.TextWrapped(md_str)
    elseif imgui.TextWrapped then
        imgui.TextWrapped(str)
    end
end
imgui.styled_text_wrapped = imgui.TextStyledWrapped

local function scale_window_geometry(from_scale, to_scale)
    if not luigee_demo_wnd or not float_wnd_get_geometry or not float_wnd_set_geometry then return end
    if from_scale <= 0.001 or to_scale <= 0.001 then return end
    local ratio = to_scale / from_scale
    if math.abs(ratio - 1.0) < 0.0001 then return end

    local l, t, r, b = float_wnd_get_geometry(luigee_demo_wnd)
    if l and t and r and b and r > l and t > b then
        local cur_w = r - l
        local cur_h = t - b
        local new_w = math.floor(cur_w * ratio + 0.5)
        local new_h = math.floor(cur_h * ratio + 0.5)
        new_w = math.max(min_header_w, new_w)
        new_h = math.max(min_header_h, new_h)
        float_wnd_set_geometry(luigee_demo_wnd, l, t, l + new_w, t - new_h)
    end
end

local function resize_window_to_zoom(target_zoom)
    if not luigee_demo_wnd or not float_wnd_get_geometry or not float_wnd_set_geometry then return end
    local l, t, r, b = float_wnd_get_geometry(luigee_demo_wnd)
    if l and t and r and b and r > l and t > b then
        local new_w = math.floor(baseline_w * target_zoom + 0.5)
        local new_h = math.floor(baseline_h * target_zoom + 0.5)
        new_w = math.max(min_header_w, new_w)
        new_h = math.max(min_header_h, new_h)
        float_wnd_set_geometry(luigee_demo_wnd, l, t, l + new_w, t - new_h)
    end
end

local about_luigee_markdown = [[# About FlyWithLua LuiGee
<pill color=#f39c12>v1.0.0</pill> <badge green>Powered by ImGee</badge> <pill blue>FlyWithLua Phase 3</pill>

**LuiGee** — *The Lua Companion to ImGee* — is the turnkey Dear ImGui avionics & flight-deck UI toolkit bundled with **FlyWithLua**.

It brings physical avionics controls, intelligent stationary tooltips, CommonMark 4.1 Markdown, sized typography, and zero-disk texture services directly to Lua script authors without low-level C++ boilerplate.

---

### Key Capabilities:
* **Tactile Avionics Widgets:** Animated toggle switches, rotary dials, mouse-wheel stepped sliders with 1-click Reset/Undo memory, and upward-blossoming pulldown selectors.
* **Smart Typography:** Inline sized text tags (`<tiny>`, `<small>`, `<regular>`, `<big>`, `<huge>`), direct semantic text helpers (`imgui.SmallText`, `imgui.TinyText`, `imgui.TitleText`, `imgui.MonoText`), and dynamic multi-size font scaling.
* **CommonMark 4.1 & GFM Tables:** Turnkey Markdown rendering with header anchors, drag-resizable data tables, inline status pills, release badges, keyboard shortcuts (`<kbd>`), and fenced code blocks with copy buttons.
* **Stationary Adaptive Tooltips:** `imgui.TimedTooltip` provides jitter-free hover documentation with reading-delay adaptation and embedded Markdown.
* **In-Memory Image Service:** Render raw compressed image bytes (PNG/JPEG) directly into GPU VRAM with zero temporary files and zero disk I/O.
* **Proportional Canvas Zoom:** Seamlessly scale font sizes and window geometry from 50% to 200% with exact baseline preservation.

---

### Explore & Build Your Own UI!
Everything in this demo is pure Lua! Open and inspect the source code of:
`FlyWithLua/Scripts/ImGui_LuiGee_Demo.lua`

For complete tutorials, API references, and architectural guides, see:
* **User Guide:** [FlyWithLua_LuiGee_UX_Guide.pdf](file://Resources/plugins/FlyWithLua/Documentation/FlyWithLua_LuiGee_UX_Guide.pdf)
* **ImGee C++ Architecture:** [slgoldberg/ImgWindow/contrib/ImGee/README.md](https://github.com/slgoldberg/ImgWindow/blob/dev/contrib/ImGee/README.md)

---
Created by **Steven L. Goldberg** (@slgoldberg) and the Rogers, with appreciation to **William R. Good** (Bill) for FlyWithLua and **Omar Cornut** for Dear ImGui.
]]

function show_luigee_demo()
    if luigee_demo_wnd then
        float_wnd_set_visible(luigee_demo_wnd, true)
        float_wnd_bring_to_front(luigee_demo_wnd)
        return
    end

    local wnd_w = math.max(min_header_w, saved_wnd_w or math.floor(baseline_w * content_zoom + 0.5))
    local wnd_h = math.max(min_header_h, saved_wnd_h or math.floor(baseline_h * content_zoom + 0.5))
    local wnd_x = saved_wnd_x or demo_x
    local wnd_y = saved_wnd_y or demo_y

    luigee_demo_wnd = float_wnd_create(wnd_w, wnd_h, 1, true)
    float_wnd_set_position(luigee_demo_wnd, wnd_x, wnd_y)
    float_wnd_set_title(luigee_demo_wnd, "FlyWithLua LuiGee Modern UI Demo")
    float_wnd_set_imgui_builder(luigee_demo_wnd, "luigee_demo_on_build")
    float_wnd_set_onclose(luigee_demo_wnd, "luigee_demo_on_close")

    if float_wnd_set_resizing_limits then
        float_wnd_set_resizing_limits(luigee_demo_wnd, min_header_w, min_header_h, 10000, 10000)
    end
    if float_wnd_set_scrolling then
        float_wnd_set_scrolling(luigee_demo_wnd, false)
    end
end

function hide_luigee_demo()
    if luigee_demo_wnd then
        if float_wnd_get_geometry then
            local l, t, r, b = float_wnd_get_geometry(luigee_demo_wnd)
            if l and t and r and b and r > l and t > b then
                saved_wnd_x = l
                saved_wnd_y = b
                saved_wnd_w = r - l
                saved_wnd_h = t - b
            end
        end
        float_wnd_destroy(luigee_demo_wnd)
        luigee_demo_wnd = nil
        current_applied_theme_idx = nil
    end
end

function toggle_luigee_demo()
    if luigee_demo_wnd then
        hide_luigee_demo()
    else
        show_luigee_demo()
    end
end

function luigee_demo_on_close(wnd)
    hide_luigee_demo()
end

-- Command & Macro Menu Registration (Never pops up at startup uninvited)
create_command(
    "FlyWithLua/LuiGee/toggle",
    "Toggle FlyWithLua LuiGee Modern UI Demo window",
    "toggle_luigee_demo()",
    "",
    ""
)

add_macro("FlyWithLua: LuiGee Modern UI Demo", "command_once('FlyWithLua/LuiGee/toggle')")

-- Configure responsive tooltips (~0.4s delay at 60fps instead of 1.1s)
if imgui.TimedTooltip and imgui.TimedTooltip.SetHoverDelay then
    imgui.TimedTooltip.SetHoverDelay(25)
end

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

-- Phase 3 state variables
local nav_source_idx        = 0  -- 0-based for SelectPulldown
local nav_sources           = { "GPS 1 (Primary / WAAS)", "VOR 1 (114.70 OAK)", "LOC / ILS (Rwy 28R)", "FMS 1 (Internal RNAV)", "TACAN (Ch 47X)" }
local nav_tooltips          = { "Garmin GPS 1 with SBAS/WAAS precision guidance", "Oakland VOR/DME 114.70 MHz", "Instrument Landing System Localizer", "Flight Management System lateral route", "Tactical Air Navigation tactical channel" }
local selected_view_action  = "Cockpit 3D"

function luigee_demo_on_build(wnd, x, y)
    -- Synchronize active theme preset across FlyWithLua windows
    if _G.FWL_ACTIVE_THEME_IDX == nil then
        _G.FWL_ACTIVE_THEME_IDX = active_theme_idx
    end
    if _G.FWL_ACTIVE_THEME_IDX ~= active_theme_idx then
        active_theme_idx = _G.FWL_ACTIVE_THEME_IDX
        active_theme_name = theme_names[active_theme_idx]
    end

    -- Ensure active theme preset is applied to the window's ImGui context
    if current_applied_theme_idx ~= active_theme_idx then
        if active_theme_idx == 0 then
            if imgui.StyleColorsCharcoal then imgui.StyleColorsCharcoal() end
        elseif active_theme_idx == 1 then
            if imgui.StyleColorsRusty then imgui.StyleColorsRusty() end
        elseif active_theme_idx == 2 then
            if imgui.StyleColorsDark then imgui.StyleColorsDark() end
        end
        current_applied_theme_idx = active_theme_idx
    end

    -- First-run: auto-display the About modal popup
    if first_run_about then
        show_about_popup = true
        first_run_about = false
    end

    -- Frame delta time calculation for smooth debouncing
    local now = os.clock()
    local dt = (last_frame_time and (now - last_frame_time)) or 0.016
    last_frame_time = now
    if dt < 0.0 or dt > 0.5 then dt = 0.016 end

    -- Debounce timer for mouse wheel zoom: window geometry stays stationary while spinning!
    if wheel_quiet_timer > 0.0 then
        wheel_quiet_timer = wheel_quiet_timer - dt
        if wheel_quiet_timer <= 0.0 and wheel_resize_pending then
            wheel_resize_pending = false
            if math.abs(content_zoom - wheel_start_zoom) > 0.001 then
                scale_window_geometry(wheel_start_zoom, content_zoom)
                wheel_start_zoom = content_zoom
            end
        end
    end

    -- Debounce timer for stepper buttons: hold to step smoothly, resize on release
    if stepper_quiet_timer > 0.0 then
        stepper_quiet_timer = stepper_quiet_timer - dt
        if stepper_quiet_timer <= 0.0 and stepper_resize_pending then
            stepper_resize_pending = false
            if math.abs(content_zoom - stepper_start_zoom) > 0.001 then
                scale_window_geometry(stepper_start_zoom, content_zoom)
                stepper_start_zoom = content_zoom
            end
        end
    end

    -- 1. Window Chrome: Fixed baseline 1.0x scale (rock-solid, non-twitching controls)
    if imgui.SetWindowFontScale then
        imgui.SetWindowFontScale(1.0)
    end
    imgui.PushFont("Roboto-Regular-16")

    -- Top Header Row: Title on Left, About button, Zoom Suite on Right
    local win_w = (imgui.GetWindowWidth and imgui.GetWindowWidth()) or baseline_w

    -- Title using Roboto-Medium-20 if available
    if imgui.PushFont then imgui.PushFont("Roboto-Medium-20") end
    imgui.TextUnformatted("FlyWithLua LuiGee Modern UI Demo")
    if imgui.PopFont then imgui.PopFont() end

    imgui.SameLine(0, 10)
    local about_clicked = false
    if imgui.SmallButton then
        about_clicked = imgui.SmallButton("About LuiGee...##btn_about")
    else
        about_clicked = imgui.Button("About LuiGee...##btn_about")
    end
    if about_clicked then
        show_about_popup = true
    end
    if imgui.TimedTooltip and imgui.TimedTooltip.TextMD then
        imgui.TimedTooltip.TextMD("**About LuiGee**\n\nView toolkit capabilities, version info, and architecture credits.")
    end

    imgui.SameLine(0, 6)
    local theme_clicked = false
    if imgui.SmallButton then
        theme_clicked = imgui.SmallButton(active_theme_name .. "##btn_theme")
    else
        theme_clicked = imgui.Button(active_theme_name .. "##btn_theme")
    end
    if theme_clicked then
        active_theme_idx = (active_theme_idx + 1) % 3
        active_theme_name = theme_names[active_theme_idx]
        _G.FWL_ACTIVE_THEME_IDX = active_theme_idx
        if active_theme_idx == 0 then
            if imgui.StyleColorsCharcoal then imgui.StyleColorsCharcoal() end
        elseif active_theme_idx == 1 then
            if imgui.StyleColorsRusty then imgui.StyleColorsRusty() end
        elseif active_theme_idx == 2 then
            if imgui.StyleColorsDark then imgui.StyleColorsDark() end
        end
        current_applied_theme_idx = active_theme_idx
    end
    if imgui.TimedTooltip and imgui.TimedTooltip.TextMD then
        local note = (imgui.StyleColorsDark == nil) and "\n\n*(Note: Classic Dark requires restarting X-Plane to load the updated FlyWithLua binary)*" or ""
        imgui.TimedTooltip.TextMD("**Click to Switch Theme Preset**\n\nCycles through:\n* **Charcoal** (Muted graphite & slate - ImGee standard)\n* **Rusty** (Warm aviation bronze & terracotta from ABC)\n* **Classic Dark** (Default Dear ImGui electric blue)" .. note)
    end

    -- Align Zoom Suite cleanly to the right margin of the content area
    -- Suite width: [-](22) + 3 + Slider(78) + 3 + [+](22) + 3 + [↺](24) + 3 + [Fit](30) = 188px
    local zoom_suite_w = 188
    imgui.SameLine(0, 8)
    local min_x = (imgui.GetCursorPosX and imgui.GetCursorPosX()) or 510
    local avail_w = (imgui.GetContentRegionAvail and imgui.GetContentRegionAvail()) or (win_w - min_x)
    local content_right = min_x + avail_w
    local desired_x = content_right - zoom_suite_w - 6
    local target_x = math.max(min_x, desired_x)
    if target_x > min_x then
        if imgui.SetCursorPosX then
            imgui.SetCursorPosX(target_x)
        else
            imgui.SameLine(target_x)
        end
    end

    -- [-] Zoom Out Stepper (10% step per click or hold to repeat)
    push_button_repeat(true)
    local dec_clicked = imgui.Button(" - ##zoom_dec", 22, 0)
    pop_button_repeat()
    if dec_clicked then
        local old_zoom = content_zoom
        local cur_pct = math.floor(content_zoom * 100.0 + 0.5)
        local new_pct = math.max(50, cur_pct - 10)
        if new_pct ~= cur_pct then
            content_zoom = new_pct / 100.0
            zoom_pct = new_pct
            scale_window_geometry(old_zoom, content_zoom)
        end
    end
    if imgui.TimedTooltip and imgui.TimedTooltip.TextMD then
        imgui.TimedTooltip.TextMD("**Zoom Out** (-10%)\n\nDecrease canvas font size and window geometry.\n*Click or hold down to step smoothly*")
    end

    imgui.SameLine(0, 3)

    -- Zoom Track Slider (Release-to-resize & debounced mouse-wheel)
    if imgui.SetNextItemWidth then imgui.SetNextItemWidth(78) end
    local pre_slider_zoom = content_zoom
    local slider_changed, new_val = imgui.SliderInt("##zoom_slider", zoom_pct, 50, 200, "%d%%")
    if slider_changed and new_val then
        content_zoom = new_val / 100.0
        zoom_pct = new_val
    end
    if imgui.IsItemActivated and imgui.IsItemActivated() then
        slider_start_zoom = pre_slider_zoom
    end
    if imgui.IsItemDeactivatedAfterEdit and imgui.IsItemDeactivatedAfterEdit() then
        if math.abs(content_zoom - slider_start_zoom) > 0.001 then
            scale_window_geometry(slider_start_zoom, content_zoom)
            slider_start_zoom = content_zoom
        end
    end
    -- Debounced mouse wheel: font updates in real time, window resizes after 350ms quiet pause
    if imgui.IsItemHovered and imgui.IsItemHovered() then
        local wheel = (imgui.GetMouseWheel and imgui.GetMouseWheel(true)) or 0
        if wheel ~= 0 then
            if not wheel_resize_pending then
                wheel_start_zoom = content_zoom
                wheel_resize_pending = true
            end
            local cur_pct = math.floor(content_zoom * 100.0 + 0.5)
            local step = (wheel > 0) and 2 or -2
            local new_pct = math.max(50, math.min(200, cur_pct + step))
            content_zoom = new_pct / 100.0
            zoom_pct = new_pct
            wheel_quiet_timer = 0.35
        end
    end
    if imgui.TimedTooltip and imgui.TimedTooltip.TextMD then
        imgui.TimedTooltip.TextMD(string.format("**Canvas & Window Zoom:** `%d%%`\n\n• **Drag Thumb:** Live font scaling, resizes on release\n• **Mouse Wheel:** Debounced smooth zoom (2%%/step)\n• **Reset (↺):** 1-click A/B toggle memory\n• **Fit:** Snap window to baseline proportions", zoom_pct))
    end

    imgui.SameLine(0, 3)

    -- [+] Zoom In Stepper (10% step per click or hold to repeat)
    push_button_repeat(true)
    local inc_clicked = imgui.Button(" + ##zoom_inc", 22, 0)
    pop_button_repeat()
    if inc_clicked then
        local old_zoom = content_zoom
        local cur_pct = math.floor(content_zoom * 100.0 + 0.5)
        local new_pct = math.min(200, cur_pct + 10)
        if new_pct ~= cur_pct then
            content_zoom = new_pct / 100.0
            zoom_pct = new_pct
            scale_window_geometry(old_zoom, content_zoom)
        end
    end
    if imgui.TimedTooltip and imgui.TimedTooltip.TextMD then
        imgui.TimedTooltip.TextMD("**Zoom In** (+10%)\n\nIncrease canvas font size and window geometry.\n*Click or hold down to step smoothly*")
    end

    imgui.SameLine(0, 3)

    -- ↺ Reset Button with Vector's 1-Click A/B Toggle Memory
    local is_modified = (zoom_pct ~= 100)
    local can_restore = (not is_modified) and (last_zoom and math.abs(last_zoom - 1.0) > 0.001)
    local rst_clicked = false
    if imgui.ResetButton then
        rst_clicked = imgui.ResetButton("##zoom_rst", is_modified, 0, false, nil, can_restore)
    else
        rst_clicked = imgui.Button("↺##zoom_rst", 24, 0)
    end
    if rst_clicked then
        local old_z = content_zoom
        if is_modified then
            -- State A -> Reset to 100% baseline: snapshot current geometry
            if luigee_demo_wnd and float_wnd_get_geometry then
                local l, t, r, b = float_wnd_get_geometry(luigee_demo_wnd)
                if l and t and r and b then
                    prior_zoom_rect = { l = l, t = t, w = r - l, h = t - b }
                end
            end
            last_zoom = content_zoom
            content_zoom = 1.0
            zoom_pct = 100
            scale_window_geometry(old_z, 1.0)
        elseif can_restore then
            -- State B -> 1-click Toggle back to prior zoom
            content_zoom = last_zoom
            zoom_pct = math.floor(content_zoom * 100.0 + 0.5)
            if prior_zoom_rect and prior_zoom_rect.w and prior_zoom_rect.h and luigee_demo_wnd and float_wnd_get_geometry and float_wnd_set_geometry then
                local l, t, r, b = float_wnd_get_geometry(luigee_demo_wnd)
                if l and t then
                    float_wnd_set_geometry(luigee_demo_wnd, l, t, l + prior_zoom_rect.w, t - prior_zoom_rect.h)
                end
            else
                scale_window_geometry(1.0, content_zoom)
            end
        end
    end
    if imgui.TimedTooltip and imgui.TimedTooltip.TextMD then
        if is_modified then
            imgui.TimedTooltip.TextMD(string.format("Reset zoom from **%d%%** back to **100%% baseline**.", zoom_pct))
        elseif can_restore then
            imgui.TimedTooltip.TextMD(string.format("1-click toggle back to prior **%d%% zoom**.", math.floor(last_zoom * 100.0 + 0.5)))
        else
            imgui.TimedTooltip.TextMD("Zoom is at **100%% baseline**.")
        end
    end

    imgui.SameLine(0, 3)

    -- Fit Window Button: snaps window to baseline proportions for current zoom
    if imgui.Button("Fit##zoom_fit", 30, 0) then
        resize_window_to_zoom(content_zoom)
    end
    if imgui.TimedTooltip and imgui.TimedTooltip.TextMD then
        imgui.TimedTooltip.TextMD(string.format("**Fit Window to Baseline**\n\nResets window dimensions to %dx%d scaled to %d%%.", baseline_w, baseline_h, zoom_pct))
    end

    imgui.Separator()

    -- 2. Content Tabs: Scale font dynamically with content_zoom!
    if imgui.SetWindowFontScale then
        imgui.SetWindowFontScale(content_zoom)
    end

    if imgui.BeginTabBar("LuiGeeDemoTabs") then

        -- ====================================================================
        -- TAB 1: Markdown Extension (Pinned TabBar with Scrolling Child Region)
        -- ====================================================================
        if imgui.BeginTabItem("Rich Markdown (MD)") then
            if imgui.BeginChild and imgui.BeginChild("##tab1_content_scroll", 0, 0, false) then
                if imgui.SetWindowFontScale then
                    imgui.SetWindowFontScale(content_zoom)
                end
                imgui.Spacing()
            imgui.MD.TextWrapped("The Markdown extension transforms standard Markdown text into styled Dear ImGui elements:")

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
                imgui.MD.TextWrapped("Body paragraph with *italic text*, _underscore italic_, and **bold text**.")
                imgui.Unindent(40)
                imgui.Spacing()

                imgui.MD.Text("<code dark>`sim/cockpit2/engine/indicators/engine_speed_rpm`</code>")
                imgui.Indent(40)
                imgui.MD.TextWrapped("Monospace inline code: `sim/cockpit2/engine/indicators/engine_speed_rpm`")
                imgui.Unindent(40)
            end

            imgui.Spacing()
            if imgui.CollapsingHeader("Pills, Badges & Custom Colors", 32) then
                imgui.MD.TextWrapped("Enhance telemetry and cockpit status displays using inline XML-style tags:")
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

                imgui.Spacing()
                imgui.MD.Text("Embedded Icons & Unary Annunciators:")
                imgui.MD.Text("  `<pill icon=avionics_beacon color=#3498db>NAV / ALT</pill>`")
                imgui.SameLine(360)
                imgui.MD.Text("<pill icon=avionics_beacon color=#3498db>NAV / ALT</pill>")

                imgui.MD.Text("  `<badge icon=avionics_beacon>v2.3.0</badge>`")
                imgui.SameLine(360)
                imgui.MD.Text("<badge icon=avionics_beacon>v2.3.0</badge>")

                imgui.MD.Text("  `<pill icon=avionics_beacon orange>ON</pill>`")
                imgui.SameLine(360)
                imgui.MD.Text("<pill icon=avionics_beacon orange>ON</pill>")

                imgui.MD.Text("  `<pill icon=avionics_beacon color=#e74c3c />` (Unary Capsule)")
                imgui.SameLine(360)
                imgui.MD.Text("<pill icon=avionics_beacon color=#e74c3c />")
            end

            imgui.Spacing()
            if imgui.CollapsingHeader("Strikethrough, Underline & Escaping", 32) then
                imgui.MD.Text("Strikethrough & Deletion Syntax:")
                imgui.MD.Text("  `~~GFM strikethrough~~`")
                imgui.SameLine(360)
                imgui.MD.Text("~~GFM strikethrough~~")

                imgui.MD.Text("  `$$ABC legacy strike$$`")
                imgui.SameLine(360)
                imgui.MD.Text("$$ABC legacy strike$$")

                imgui.MD.Text("  `<s>HTML strike</s>` / `<del>deletion</del>`")
                imgui.SameLine(360)
                imgui.MD.Text("<s>HTML strike</s> / <del>deletion</del>")

                imgui.MD.Text("  `<s red>Colored strikethrough</s>`")
                imgui.SameLine(360)
                imgui.MD.Text("<s red>Colored strikethrough</s>")

                imgui.Spacing()
                imgui.MD.Text("Underline & Insertion Syntax:")
                imgui.MD.Text("  `++CriticMarkup underline++`")
                imgui.SameLine(360)
                imgui.MD.Text("++CriticMarkup underline++")

                imgui.MD.Text("  `<u>HTML underline</u>` / `<ins>insertion</ins>`")
                imgui.SameLine(360)
                imgui.MD.Text("<u>HTML underline</u> / <ins>insertion</ins>")

                imgui.MD.Text("  `<u #f39c12>Warning yellow underline</u>`")
                imgui.SameLine(360)
                imgui.MD.Text("<u #f39c12>Warning yellow underline</u>")

                imgui.Spacing()
                imgui.MD.Text("Punctuation Escaping (render formatting tokens literally):")
                imgui.TextUnformatted("  \\*not italic\\*")
                imgui.SameLine(360)
                imgui.MD.Text("\\*not italic\\*")

                imgui.TextUnformatted("  \\[brackets\\]")
                imgui.SameLine(360)
                imgui.MD.Text("\\[brackets\\]")

                imgui.TextUnformatted("  \\$100")
                imgui.SameLine(360)
                imgui.MD.Text("\\$100")

                imgui.TextUnformatted("  \\# not heading")
                imgui.SameLine(360)
                imgui.MD.Text("\\# not heading")

                imgui.Spacing()
                imgui.MD.Text("Code Spans & Backdropped Code Badges:")
                imgui.TextUnformatted("  `inline monospace`")
                imgui.SameLine(360)
                imgui.MD.Text("`inline monospace`")

                imgui.TextUnformatted("  <code>code badge</code>")
                imgui.SameLine(360)
                imgui.MD.Text("<code>code badge</code>")

                imgui.TextUnformatted("  <mono>mono badge</mono>")
                imgui.SameLine(360)
                imgui.MD.Text("<mono>mono badge</mono>")
            end

            imgui.Spacing()
            if imgui.CollapsingHeader("Lists, Tables & Variadic Formatting", 32) then
                imgui.MD.TextWrapped("Pre-flight Checklist (Unordered):")
                imgui.MD.TextWrapped("* Pitot tube covers removed\n* Flight control surfaces free and clear\n* Fuel quantity verified\n* Transponder set to <pill blue>1200 / VFR</pill>")

                imgui.Spacing()
                imgui.MD.TextWrapped("Engine Start Procedure (Ordered with Hanging Indents):")
                imgui.MD.TextWrapped("1. Parking brake: SET\n2. Beacon light: ON\n3. Starter switch: ENGAGE\n4. Engine ITT & oil pressure: MONITOR")

                imgui.Spacing()
                imgui.MD.TextWrapped("GFM Drag-Resizable Data Table with Alignment & Status Badges:")
                imgui.MD.Text([[
| Flight Parameter | Current Value | Target / Limit | Status |
| :--- | :--- | :--- | :---: |
| **Pressure Altitude** | 18,500 ft | FL240 | <pill green>CLIMB</pill> |
| **Indicated Airspeed** | 264.8 kts | 280 kts Max | <pill green>NORMAL</pill> |
| **Baro Setting (QNH)** | 29.92 inHg | Standard (29.92) | <badge blue>STD</badge> |
| **Engine 1 N1** | 92.4% | 95.0% Max | <pill green>NOMINAL</pill> |
| **Fuel Remaining** | 1,420 lbs | 500 lbs Reserve | <pill warning>BALANCED</pill> |
]])

                imgui.Spacing()
                imgui.MD.TextWrapped("Variadic string formatting (pass %s, %d, %f like string.format):")
                imgui.MD.Text("<code dark>imgui.MD.Text(\"**Alt**: %d ft | **IAS**: %.1f kts\", 18500, 264.8)</code>")
                imgui.Indent(40)
                imgui.MD.TextWrapped("Telemetry readout: **Alt**: %d ft | **IAS**: %.1f kts | **QNH**: %.2f inHg", 18500, 264.8, altimeter_setting)
                imgui.Unindent(40)
            end

            imgui.Spacing()
            if imgui.CollapsingHeader("Multi-Line Fenced Code Blocks (CommonMark)", 32) then
                imgui.MD.TextWrapped("Multi-line code snippets render in shaded charcoal containers with automatic language chips:")
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
                imgui.Spacing()
                imgui.MD.Text([[
````markdown
### Embedded Checklist & Flight Telemetry
* System Status: <pill green>SYSTEMS READY</pill>
* Baro Altimeter: **29.92 inHg** | Transition: <color #f39c12>18,000 ft</color>

```lua
-- Inner nested Lua snippet inside Markdown container
local qnh = get("sim/cockpit2/gauges/actuators/barometer_setting_in_hg")
if qnh < 29.92 then set_altimeter_std() end
```
````
]])
                -- Optional untyped / unsupported code blocks (commented out per user request):
                -- imgui.Spacing()
                -- imgui.MD.Text([=[
                -- ```
                -- -- Untyped code block (no language tag after backticks)
                -- raw_telemetry_packet = {
                --     latitude = 37.6188,
                --     longitude = -122.3750,
                --     heading_deg = 280.5
                -- }
                -- ```
                -- ]=])
                -- imgui.Spacing()
                -- imgui.MD.Text([=[
                -- ```javascript
                -- // Unsupported language tag ("javascript")
                -- function calculateApproachDescent(distanceNm, currentAltitudeFt, targetAltitudeFt) {
                --     const altitudeLoss = currentAltitudeFt - targetAltitudeFt;
                --     const descentRateFpm = (altitudeLoss / distanceNm) * 3.5;
                --     return Math.min(2500, Math.round(descentRateFpm));
                -- }
                -- ```
                -- ]=])
            end

            imgui.Spacing()
            if imgui.CollapsingHeader("Clickable Links & Markdown Buttons", 32) then
                imgui.MD.Text("<code dark>{FlyWithLua GitHub Project}(https://github.com/sparker256/FlyWithLua)</code>")
                imgui.Indent(40)
                imgui.MD.TextWrapped("Clickable URL Link: {FlyWithLua GitHub Project}(https://github.com/sparker256/FlyWithLua)")
                imgui.MD.TextWrapped("*(Clicking the link above automatically opens your system default web browser!)*")
                imgui.Unindent(40)

                imgui.Spacing()
                imgui.MD.Text("<code dark>imgui.Markdown.ButtonMD(\"Execute Diagnostic Check##mddemo\", rich_tooltip_markdown)</code>")
                imgui.Indent(40)
                local btn_tip = string.format("### Diagnostic Check\nTotal clicks registered: **%d**\nStatus: <pill green>READY</pill>", md_button_clicks)
                if imgui.Markdown.ButtonMD(string.format("Execute Check (Clicks: %d)##mddemo", md_button_clicks), btn_tip) then
                    md_button_clicks = md_button_clicks + 1
                end
                imgui.TimedTooltip.Text("Click to advance diagnostic execution cycle.")
                imgui.MD.TextWrapped("*(Hover over the button above to view its rich Markdown tooltip!)*")
                imgui.Unindent(40)
            end

            if imgui.EndChild then imgui.EndChild() end
            end
            imgui.EndTabItem()
        end

        -- ====================================================================
        -- TAB 2: TimedTooltips
        -- ====================================================================
        if imgui.BeginTabItem("Timed Tooltips") then
            imgui.Spacing()
            imgui.MD.TextWrapped("### Why TimedTooltip is Superior to Basic ImGui Tooltips:")
            imgui.MD.TextWrapped("1. **Stationary Anchor:** Does not jitter, jump, or bounce as the mouse moves over the widget.\n" ..
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
            imgui.MD.TextWrapped("Dwell timing configuration methods:\n" ..
                          "* `imgui.TimedTooltip.SetHoverDelay(cycles)` -- Delay before appearing (e.g. 25 cycles = ~0.4s)\n" ..
                          "* `imgui.TimedTooltip.SetMaxDisplay(cycles)` -- Maximum display timeout\n" ..
                          "* `imgui.TimedTooltip.SetMaxWidth(pixels)`   -- Maximum text wrap width")

            imgui.EndTabItem()
        end

        -- ====================================================================
        -- TAB 3: Tactile Controls & Sliders (Guaranteed Zero-Scroll)
        -- ====================================================================
        if imgui.BeginTabItem("Tactile Controls") then
            imgui.Spacing()
            imgui.MD.TextWrapped("### Smooth Animated & Mouse-Wheel Adjusted Controls")
            imgui.MD.TextWrapped("*(Tip: Mouse-wheel directly over switches flips states; clicking toggles via either the buttons or labels)*")
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
            imgui.TimedTooltip.TextMD("Click the reset button (<icon undo />) to snap trim back to neutral (0.00).")

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

            -- Row 3: Mouse Wheel Chasers (Side-by-side)
            imgui.MD.TextWrapped("### Mouse Wheel Chasers")
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

            -- Row 4: Procedural Sweeping Dials (Adjustable via Sliders)
            imgui.MD.TextWrapped("### Sweeping Progress & Countdown Dials")
            imgui.TextUnformatted("Procedural vector antialiased arcs -- drag sliders or scroll mouse wheel:")
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

            imgui.EndTabItem()
        end

        -- ====================================================================
        -- TAB 4: Badges, Spinners & Links
        -- ====================================================================
        if imgui.BeginTabItem("Badges, Spinners & Links") then
            imgui.Spacing()
            imgui.MD.TextWrapped("### Standalone First-Class Widgets (No Markdown Parsing)")
            imgui.MD.TextWrapped("Call directly in pure ImGui panels without Markdown string wrapping:")
            imgui.Spacing()

            imgui.TextUnformatted("Status Badges & Tags:")
            imgui.SameLine()
            imgui.Badge("ONLINE", "success")
            imgui.SameLine()
            imgui.Badge("EXPIRING", "warning")
            imgui.SameLine()
            imgui.Tag("FAULT", "danger")

            imgui.Spacing()
            imgui.TextUnformatted("Pills & Dual Colors:")
            imgui.SameLine()
            imgui.Pill("v2.2.2", "primary")
            imgui.SameLine()
            imgui.Pill("CUSTOM DUAL", {0.9, 0.7, 0.1, 1.0}, {0.0, 0.0, 0.0, 1.0})

            imgui.Spacing()
            imgui.TextUnformatted("3D Tactile Keycaps:")
            imgui.SameLine()
            imgui.Kbd("Ctrl+Shift+P")
            imgui.SameLine()
            imgui.Key("Enter")
            imgui.SameLine()
            imgui.KeyCap("Esc")

            imgui.Spacing()
            imgui.Separator()

            -- Procedural Activity Spinners
            imgui.MD.TextWrapped("### Procedural Activity Spinners")
            imgui.MD.TextWrapped("Vector antialiased paths via ImDrawList -- zero textures, zero fonts, sinusoidal organic easing:")
            imgui.Spacing()

            imgui.Spinner("FMS Navdata Ingestion (1.2x Speed)", 14, 3, {0.2, 0.7, 1.0, 1.0}, demo_spinner_speed)
            imgui.TimedTooltip.Text("Organic sinusoidal expanding/contracting arc animated automatically by ImGui::GetTime().")

            imgui.SameLine(400)
            imgui.Spinner("Background Telemetry Sync", 10, 2.0)
            imgui.TimedTooltip.Text("Compact spinner using default theme color and 1.0x animation speed.")

            imgui.Spacing()
            imgui.TextUnformatted("Spinner Aliases:")
            imgui.SameLine()
            imgui.ActivitySpinner("ActivitySpinner##alias", 10, 2.0)
            imgui.SameLine()
            imgui.ProgressDial("ProgressDial##alias", 0.75, 12, 2.5)
            imgui.SameLine()
            imgui.CountdownDial("CountdownDial##alias", 15.0, 60.0, 12, 2.5)

            imgui.Spacing()
            imgui.Separator()

            -- Dynamic ImGui Mouse Cursors & Hyperlinks
            imgui.MD.TextWrapped("### Dynamic ImGui Mouse Cursors & Hyperlinks")
            local cur_cursors = float_wnd_get_imgui_cursor_handling(wnd)
            local cursor_changed, new_cursor_state = imgui.ToggleButton("Dynamic ImGui Cursors", cur_cursors)
            if cursor_changed then
                float_wnd_set_imgui_cursor_handling(wnd, new_cursor_state)
            end
            imgui.TimedTooltip.Text("Toggles dynamic ImGui cursors (Hand over links/buttons, I-beam over text) vs native hardware mouse.")

            imgui.Spacing()
            imgui.TextUnformatted("Hyperlinks & Affordances:")
            imgui.SameLine()
            imgui.TextLink("X-Plane Official Org Portal", "https://forums.x-plane.org")
            imgui.SameLine()
            imgui.TextUnformatted(" | ")
            imgui.SameLine()
            imgui.ButtonLink("FlyWithLua Issues", "https://github.com/sparker256/FlyWithLua/issues")
            imgui.SameLine()
            imgui.SmallButtonLink("SmallButtonLink Demo", "https://github.com/sparker256/FlyWithLua")

            imgui.EndTabItem()
        end

        -- ====================================================================
        -- TAB 5: Universal C++ & Lua Alias Matrix
        -- ====================================================================
        if imgui.BeginTabItem("C++ / Lua Aliases") then
            imgui.Spacing()
            imgui.MD.TextWrapped("### Universal Syntax & PascalCase Parity")
            imgui.MD.TextWrapped("FlyWithLua mirrors the C++ Dear ImGui naming conventions and provides complete alias parity so script authors can write whatever feels most natural:")

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
            imgui.MD.Text("`imgui.Spinner(...)` <-> `imgui.ActivitySpinner(...)` / `imgui.spinner`")

            imgui.Bullet()
            imgui.MD.Text("`imgui.RadialTimer(...)` <-> `imgui.ProgressDial(...)` / `imgui.radial_timer`")

            imgui.Bullet()
            imgui.MD.Text("`imgui.CountdownTimer(...)` <-> `imgui.CountdownDial(...)` / `imgui.countdown_timer`")

            imgui.Spacing()
            imgui.Separator()
            imgui.TextUnformatted("First-Class Badge, Pill & Keycap aliases:")

            imgui.Bullet()
            imgui.MD.Text("`imgui.Badge(text, variant)` <-> `imgui.Tag(...)` / `imgui.badge`")

            imgui.Bullet()
            imgui.MD.Text("`imgui.Pill(text, variant)` / `imgui.pill`")

            imgui.Bullet()
            imgui.MD.Text("`imgui.Kbd(key)` <-> `imgui.Key(key)` <-> `imgui.KeyCap(key)` / `imgui.kbd`")

            imgui.EndTabItem()
        end

        -- ====================================================================
        -- TAB 6: Phase 3 Widgets (SelectPulldown, Anchored Popups & StyledText)
        -- ====================================================================
        if imgui.BeginTabItem("Phase 3 Widgets") then
            if imgui.BeginChild and imgui.BeginChild("##tab6_content_scroll", 0, 0, false) then
                if imgui.SetWindowFontScale then
                    imgui.SetWindowFontScale(content_zoom)
                end
                imgui.Spacing()
            imgui.MD.TextWrapped("### Phase 3: Directional Popups, SelectPulldown & StyledText Engine")
            imgui.MD.TextWrapped("Introduced in ImgWindow v2.3 Phase 3 and exposed to FlyWithLua via native LuiGee C++ bindings.")
            imgui.Separator()
            imgui.Spacing()

            -- 1. SelectPulldown Widget
            if imgui.CollapsingHeader("1. SelectPulldown Widget (Zero-Scroll Leak)", 32) then
                imgui.MD.TextWrapped("A compact 1-line dataref and option selector with built-in down-caret indicator:")
                imgui.Bullet()
                imgui.MD.TextWrapped("**When Closed:** Mouse-wheel events **pass through** to parent container so scrolling lists scroll smoothly without accidental setting changes.")
                imgui.Bullet()
                imgui.MD.TextWrapped("**When Open:** Mouse wheel **cycles dynamically** through the items list without closing the popup!")
                imgui.Bullet()
                imgui.MD.TextWrapped("**Tooltips Support:** Optional per-item hover tooltips.")
                imgui.Spacing()

                imgui.TextUnformatted("Active Navigation Source:")
                imgui.SameLine()
                local nav_changed, new_nav = imgui.SelectPulldown("##NavSelect", nav_source_idx, nav_sources, nav_tooltips, 320)
                if nav_changed then
                    nav_source_idx = new_nav
                end
                imgui.TimedTooltip.Text("Click to open pulldown, or spin mouse wheel while open to cycle options.")

                imgui.SameLine()
                imgui.MD.TextWrapped(string.format("Selected: <pill blue>%s</pill> (Index: %d)", nav_sources[nav_source_idx + 1] or "Unknown", nav_source_idx))

                imgui.Spacing()
                imgui.TextUnformatted("Lua Code:")
                imgui.MD.Text([[
```lua
local changed, new_idx = imgui.SelectPulldown("##NavSelect", nav_source_idx, nav_sources, nav_tooltips, 320)
if changed then
    nav_source_idx = new_idx
end
```
]])
            end

            imgui.Spacing()
            -- 2. Directional Popup Anchoring
            if imgui.CollapsingHeader("2. Directional Popup Anchoring (Blossom Up / Down)", 32) then
                imgui.MD.TextWrapped("Anchors popup windows relative to the preceding item's bounding box with sub-pixel precision:")
                imgui.Bullet()
                imgui.MD.TextWrapped("`imgui.PopupAnchor.AboveItem` -- Blossoms **UP** (essential for status bars docked at screen bottom).")
                imgui.Bullet()
                imgui.MD.TextWrapped("`imgui.PopupAnchor.BelowItem` -- Blossoms **DOWN** (standard dropdown menu).")
                imgui.Bullet()
                imgui.MD.TextWrapped("`imgui.PopupAnchor.Auto` -- Automatically chooses direction based on remaining vertical screen space.")
                imgui.Bullet()
                imgui.MD.TextWrapped("**Toggle Safety:** Clicking the opener button while open cleanly toggles it closed without re-opening flicker.")
                imgui.Bullet()
                imgui.MD.TextWrapped("**Headroom Queries:** `imgui.GetLastPopupHeight()` & `imgui.GetLastPopupSize()` report exact measured dimensions.")
                imgui.Spacing()

                -- Interactive Demo: Simulated Bottom Cockpit Toolbar
                imgui.MD.TextWrapped("#### Interactive Demonstration (Bottom Status Strip Simulator):")
                imgui.TextUnformatted("Active View Action: ")
                imgui.SameLine()
                imgui.Badge(selected_view_action, "primary")

                imgui.Spacing()
                -- Button 1: Blossoms UP
                if imgui.Button("View Options (Blossoms UP ^)##btnUp", 260, 32) then
                    imgui.OpenPopupAtItem("##ViewOptsPopup", imgui.PopupAnchor.AboveItem, imgui.PopupAlign.Left)
                end
                imgui.TimedTooltip.Text("Anchored with PopupAnchor.AboveItem -- opens upwards above this button!")

                imgui.SameLine()
                -- Button 2: Blossoms DOWN
                if imgui.Button("Action Menu (Blossoms DOWN v)##btnDown", 260, 32) then
                    imgui.OpenPopupAtItem("##ActionMenuPopup", imgui.PopupAnchor.BelowItem, imgui.PopupAlign.Right)
                end
                imgui.TimedTooltip.Text("Anchored with PopupAnchor.BelowItem -- opens downwards below this button!")

                -- Popups
                if imgui.BeginAnchoredPopup("##ViewOptsPopup") then
                    imgui.TextDisabled("CAMERAS & VIEWS")
                    imgui.Separator()
                    if imgui.MenuItem("Cockpit 3D View") then selected_view_action = "Cockpit 3D" end
                    if imgui.MenuItem("Chase Plane Cam") then selected_view_action = "Chase Plane" end
                    if imgui.MenuItem("Tower Observer View") then selected_view_action = "Tower View" end
                    if imgui.MenuItem("Fly-By Flyover") then selected_view_action = "Fly-By Camera" end
                    imgui.Separator()
                    if imgui.MenuItem("Close Menu") then imgui.CloseCurrentPopup() end
                    imgui.EndPopup()
                end

                if imgui.BeginAnchoredPopup("##ActionMenuPopup") then
                    imgui.TextDisabled("COCKPIT ACTIONS")
                    imgui.Separator()
                    if imgui.MenuItem("Reset Flight Trim") then throttle_trim = 0.0 end
                    if imgui.MenuItem("Center Altimeter (29.92)") then altimeter_setting = 29.92 end
                    if imgui.MenuItem("Cycle Transponder (1200)") then end
                    imgui.Separator()
                    if imgui.MenuItem("Dismiss") then imgui.CloseCurrentPopup() end
                    imgui.EndPopup()
                end

                imgui.Spacing()
                local pw, ph = imgui.GetLastPopupSize("##ViewOptsPopup")
                imgui.TextUnformatted(string.format("Last Measured Size of '##ViewOptsPopup': %.0f x %.0f px (Height: %.0f px)", pw, ph, imgui.GetLastPopupHeight("##ViewOptsPopup")))
            end

            imgui.Spacing()
            -- 3. Standalone StyledText Engine
            if imgui.CollapsingHeader("3. Standalone Inline StyledText (imgui.TextStyled)", 32) then
                imgui.MD.TextWrapped("Render rich styled text inline inside any Dear ImGui window **without** the document wrapping overhead of full Markdown:")
                imgui.Bullet()
                imgui.MD.TextWrapped("Supports `<b>bold</b>`, `<i>italic</i>`, `<mono>monospace</mono>`, headings (`<h1>`..`<h3>`), and `<color name/#hex>`.")
                imgui.Bullet()
                imgui.MD.TextWrapped("Supports inline `<pill>` and `<badge>` tags with embedded icons!")
                imgui.Bullet()
                imgui.MD.TextWrapped("Full variadic `printf`-style formatting support.")
                imgui.Spacing()

                imgui.TextUnformatted("Live Output:")
                imgui.Indent(20)
                imgui.TextStyled("Flight Mode: <b><color #2ecc71>CRUISE</color></b> | Mach: <mono>%.2f</mono> | Target: <color gold>FL%03d</color>", 0.82, 380)
                imgui.TextStyled("Annunciators: <pill green>AP1 ENGAGED</pill> <pill blue>LNAV / VNAV</pill> <pill warning>ICE DETECTED</pill>")
                imgui.TextStyled("Inline Sizing: <tiny color=#888888>tiny (10px)</tiny> | <small>small (12px)</small> | <regular>regular (14px)</regular> | <big>big (16px)</big> | <huge color=orange>huge (16px+ bold)</huge>")
                imgui.TextStyledWrapped("Wrapped Styled Text: <i>This is an inline styled paragraph</i> rendered with <b>imgui.TextStyledWrapped</b>. It flows smoothly across window boundaries and supports <color #3498db>custom hex colored highlights</color> on demand.")
                imgui.Unindent(20)

                imgui.Spacing()
                imgui.TextUnformatted("Lua Code:")
                imgui.MD.Text([[
```lua
imgui.TextStyled("Flight Mode: <b><color #2ecc71>CRUISE</color></b> | Mach: <mono>%.2f</mono>", 0.82)
imgui.TextStyled("Inline Sizing: <tiny>tiny</tiny> | <small>small</small> | <regular>body</regular> | <big>big</big> | <huge>huge</huge>")
imgui.TextStyledWrapped("Wrapped text with <b>bold</b> and <color gold>gold text</color>.")
```
]])
            end

            imgui.Spacing()
            -- 4. Direct Semantic Text Helpers
            if imgui.CollapsingHeader("4. Direct Semantic Text Helpers (SmallText, TinyText, TitleText, MonoText)", 32) then
                imgui.MD.TextWrapped("Direct immediate-mode text rendering helpers mapped to the typography hierarchy:")
                imgui.Spacing()

                imgui.Indent(20)
                imgui.TitleText("TitleText: Avionics Nav Log & Route Plan")
                imgui.TextUnformatted("Regular Text: Cruising at FL340 over DLL VOR.")
                imgui.SmallText("SmallText (12px): Standard caption, secondary annotations, and table subtext.")
                imgui.TinyText("TinyText (10px): Micro annotations, build hashes, and footnote disclaimers.")
                imgui.MonoText("MonoText: sim/cockpit2/radios/actuators/com1_frequency_hz = 121500")
                imgui.Unindent(20)

                imgui.Spacing()
                imgui.TextUnformatted("Lua Code Example:")
                imgui.MD.Text([[
```lua
imgui.TitleText("Avionics Status: %s", "NORMAL")
imgui.SmallText("Last GPS update: %d seconds ago", 2)
imgui.TinyText("Firmware build: %s", "v2.3.0-4268f3d")
imgui.MonoText("Active Dataref: %s", "sim/flightmodel/position/latitude")
```
]])
            end

            imgui.Spacing()
            -- 5. Turnkey In-Memory Image Service
            if imgui.CollapsingHeader("5. In-Memory Image Service (imgui.RegisterMemoryImage)", 32) then
                imgui.MD.TextWrapped("Register raw compressed image bytes (PNG/JPEG) directly into GPU VRAM with **zero disk writes and zero temp files**:")
                imgui.Bullet()
                imgui.MD.TextWrapped("Ideal for live radar tiles from web requests (`luasocket`), or self-contained single-file `.lua` distributions with embedded Base64 icons.")
                imgui.Bullet()
                imgui.MD.TextWrapped("Once registered, the texture can be rendered via `imgui.ImageNamed()`, `![Alt](name)`, `<img name=... />`, or `<pill icon=...>`!")
                imgui.Spacing()

                imgui.TextUnformatted("Lua Code Example:")
                imgui.MD.Text([[
```lua
-- Decode raw bytes directly into GPU VRAM (no temp files!)
local ok, w, h = imgui.RegisterMemoryImage("live_radar", raw_png_bytes)
if ok then
    imgui.ImageNamed("live_radar", w, h)
end

-- Also works inside Markdown and Pill icons:
imgui.MD.Text("Radar Tile: ![Radar](live_radar)")
imgui.MD.Text("Status: <pill icon=live_radar color=#3498db>ONLINE</pill>")
```
]])
            end

            if imgui.EndChild then imgui.EndChild() end
            end
            imgui.EndTabItem()
        end

        imgui.EndTabBar()
    end

    -- 3. About LuiGee Modal Popup Dialog
    if show_about_popup then
        if imgui.OpenPopup then
            imgui.OpenPopup("About FlyWithLua LuiGee")
        end
        show_about_popup = false
    end

    if imgui.SetNextWindowSize then
        imgui.SetNextWindowSize(640, 540, 2)
    end
    if imgui.BeginPopupModal and imgui.BeginPopupModal("About FlyWithLua LuiGee") then
        if imgui.SetWindowFontScale then
            imgui.SetWindowFontScale(1.0)
        end
        if imgui.BeginChild("AboutLuiGeeScroll", 0, -42, 1) then
            imgui.MD.TextWrapped(about_luigee_markdown)
            imgui.EndChild()
        end
        imgui.Separator()
        if imgui.Button("  Close  ", 100, 0) then
            imgui.CloseCurrentPopup()
        end
        imgui.EndPopup()
    end

    -- 4. Font scale reset guard: ensure window font scale is ALWAYS 1.0 at frame exit
    if imgui.SetWindowFontScale then
        imgui.SetWindowFontScale(1.0)
    end

    imgui.PopFont()
end
