-- ============================================================================
-- FlyWithLua Cockpit Markdown Editor & Live Previewer
-- ----------------------------------------------------------------------------
-- Author: Antigravity pair programming with Steve Goldberg (@slgoldberg)
-- Architecture: Roger2 (ImgWindow Contrib) & Sir Lance (FlyWithLua)
-- Description:
--   An in-sim floating scratchpad, ATIS note-taker, and markdown editor with
--   real-time preview, 1-click aviation templates, and markdown persistence.
-- ============================================================================

if not SUPPORTS_FLOATING_WINDOWS then
    logMsg("FlyWithLua: Floating windows not supported by this installation.")
    return
end

-- ----------------------------------------------------------------------------
-- ImGui Color Constants & Helpers
-- ----------------------------------------------------------------------------
local ImGuiCol_Button = 21
if imgui.Col and imgui.Col.Button then
    ImGuiCol_Button = imgui.Col.Button
elseif imgui.constant and imgui.constant.Col and imgui.constant.Col.Button then
    ImGuiCol_Button = imgui.constant.Col.Button
end

-- ImGuiInputTextFlags (WordWrap = 1 << 24, AllowTabInput = 1 << 5)
local ImGuiInputTextFlags_WordWrap = 16777216
local ImGuiInputTextFlags_AllowTabInput = 32
if imgui.constant and imgui.constant.InputTextFlags then
    if imgui.constant.InputTextFlags.AllowTabInput then
        ImGuiInputTextFlags_AllowTabInput = imgui.constant.InputTextFlags.AllowTabInput
    end
    if imgui.constant.InputTextFlags.WordWrap then
        ImGuiInputTextFlags_WordWrap = imgui.constant.InputTextFlags.WordWrap
    end
end
local editor_input_flags = ImGuiInputTextFlags_WordWrap + ImGuiInputTextFlags_AllowTabInput

-- ----------------------------------------------------------------------------
-- State & Buffers
-- ----------------------------------------------------------------------------
local editor_wnd = nil
local view_mode  = "Split"  -- "Write", "Split", "Preview"
local save_status_msg = ""
local save_status_path = ""
local save_status_time = 0

-- Default scratchpad template loaded on initial start
local initial_doc = [[# Cockpit Notes & Scratchpad
Welcome to your in-cockpit **FlyWithLua Markdown Editor**!

* **Live Formatting:** Headers, **bold**, *italic*, <u>underline</u>, and ~~strikethrough~~.
* **Avionics Badges:** <badge green>ENG 1 NORMAL</badge> <badge yellow>FUEL PUMP AUTO</badge> <badge red>FIRE TEST</badge>
* **Keycaps & Shortcuts:** Press <kbd>Alt+F4</kbd> or <kbd>Ctrl+Shift+P</kbd>
* **Checklists:**
  - [x] Pre-flight briefing complete
  - [x] Altimeter set to standard: `29.92 inHg`
  - [ ] Clearance copied and read back

```lua
-- Quick Cockpit Dataref Inspection
local tas = get("sim/flightmodel/position/true_airspeed")
local alt = get("sim/flightmodel/position/elevation")
```

<pill blue>TIP</pill> Use the **Mode** buttons above to switch between pure Editor, Split View, and full-screen Preview.
]]

local editor_text = initial_doc
local MAX_TEXT_BYTES = 65536

-- ----------------------------------------------------------------------------
-- Aviation Templates
-- ----------------------------------------------------------------------------
local templates = {
    ["ATIS Copy Pad"] = [[# ATIS Information [ALPHA]
* **Airport:** `KORD (Chicago O'Hare)` | **Time:** `1851Z`
* **Wind:** `280° @ 12kt G18kt` | **Visibility:** `10 SM`
* **Sky Condition:** `FEW 045, SCT 250`
* **Temp / Dew Point:** `18°C / 08°C`
* **Altimeter:** `30.12 inHg`
* **Landing / Departing:** `Runway 28C, 28R`
* **NOTAMs:** Taxiway Bravo closed between B4 and B6
]],

    ["IFR Clearance (CRAFT)"] = [[# IFR Clearance Briefing
* **C - Clearance Limit:** `Cleared to KMSP via JOT2 departure`
* **R - Route:** `JOT VORD V100 BAE DLL ALTRN`
* **A - Initial Altitude:** `Maintain 5,000 ft, expect FL330 in 10 minutes`
* **F - Departure Frequency:** `120.40 MHz`
* **T - Transponder (Squawk):** <kbd>4217</kbd>

---
- [ ] Readback verified and accepted by ATC
- [ ] Squawk set in Transponder
- [ ] Initial altitude armed on MCP
]],

    ["Pre-Flight Checklist"] = [[# Cockpit Pre-Flight Checklist
- [ ] Weather & NOTAM briefing reviewed
- [ ] Fuel onboard verified: `14,200 lbs`
- [ ] Weight & Balance computed (CG within envelope)
- [ ] FMS flight plan & waypoints cross-checked
- [ ] Altimeters cross-checked & set: `29.92`
- [ ] V-Speeds calculated: `V1: 138`, `VR: 144`, `V2: 151`
- [ ] Transponder: <pill green>ALT / 1200</pill>
- [ ] Thrust reversers checked: <badge green>ARMED</badge>
]],

    ["Diversion Briefing"] = [[# Diversion Briefing
* **Alternate Destination:** `KSBN (South Bend Regional)`
* **Bearing / Distance:** `085° / 64 NM`
* **Estimated Fuel at Alternate:** `4,100 lbs` (Min Reserve: 2,400 lbs)
* **Longest Runway:** `09R/27L (8,414 x 150 ft)`
* **Current Weather:** `310° @ 14kt G22, 6SM -RA, OVC 018`
* **Primary Nav Aid:** `ILS 27L (109.90 MHz / CRS 274°)`
* **Decision Altitude (DA):** `973 ft MSL (200 ft AGL)`
]],

    ["FlyWithLua Scratchpad"] = [[# FlyWithLua Scratchpad
```lua
-- Lua Scratchpad: Test Datarefs & Logic
local groundspeed = get("sim/flightmodel/position/groundspeed")
local heading_mag = get("sim/flightmodel/position/mag_psi")
local pitch_deg   = get("sim/flightmodel/position/theta")

-- Quick status tags
```
* **Aircraft State:** <badge green>Airborne</badge>
* **Autopilot:** <pill blue>LNAV / VNAV PATH</pill>
* **Flaps:** `UP / 0°`
]]
}

-- ----------------------------------------------------------------------------
-- Smart Selection Wrapping & Formatting Helpers
-- ----------------------------------------------------------------------------
local cached_sel_start = -1
local cached_sel_end   = -1
local cached_cur_pos   = -1
local just_formatted   = false

local function wrap_or_insert(prefix, suffix, default_text, is_line_prefix)
    suffix = suffix or ""
    default_text = default_text or ""
    
    local sel_start, sel_end, cur_pos = -1, -1, -1
    if imgui.GetInputTextSelection then
        sel_start, sel_end, cur_pos = imgui.GetInputTextSelection()
    end

    -- Fall back to cached selection if focus was just blurred by clicking the button
    if (sel_start < 0 or sel_end <= sel_start) and cached_sel_start >= 0 and cached_sel_end > cached_sel_start then
        sel_start = cached_sel_start
        sel_end   = cached_sel_end
        cur_pos   = cached_cur_pos
    end
    
    if sel_start >= 0 and sel_end > sel_start and sel_end <= #editor_text then
        if is_line_prefix then
            -- Prepend prefix to the start of the line containing sel_start
            local line_start = 1
            local prev_nl = editor_text:sub(1, sel_start):reverse():find("\n")
            if prev_nl then
                line_start = sel_start - prev_nl + 2
            end
            local before = editor_text:sub(1, line_start - 1)
            local after  = editor_text:sub(line_start)
            editor_text = before .. prefix .. after
            just_formatted = true
            if imgui.SetInputTextSelection then
                local new_start = sel_start + #prefix
                local new_end   = sel_end + #prefix
                imgui.SetInputTextSelection(new_start, new_end)
                cached_sel_start = new_start
                cached_sel_end   = new_end
            end
        else
            -- Wrap the exact selection in-place:
            local before = editor_text:sub(1, sel_start)
            local selected = editor_text:sub(sel_start + 1, sel_end)
            local after = editor_text:sub(sel_end + 1)
            editor_text = before .. prefix .. selected .. suffix .. after
            just_formatted = true
            if imgui.SetInputTextSelection then
                local new_start = sel_start + #prefix
                local new_end   = new_start + #selected
                imgui.SetInputTextSelection(new_start, new_end)
                cached_sel_start = new_start
                cached_sel_end   = new_end
            end
        end
    elseif cur_pos >= 0 and cur_pos <= #editor_text then
        if is_line_prefix then
            -- Find beginning of current line and insert prefix
            local line_start = 1
            local prev_nl = editor_text:sub(1, cur_pos):reverse():find("\n")
            if prev_nl then
                line_start = cur_pos - prev_nl + 2
            end
            local before = editor_text:sub(1, line_start - 1)
            local after  = editor_text:sub(line_start)
            editor_text = before .. prefix .. after
            just_formatted = true
            if imgui.SetInputTextSelection then
                imgui.SetInputTextSelection(cur_pos + #prefix, cur_pos + #prefix)
            end
        else
            -- Insert snippet at cursor position
            local before = editor_text:sub(1, cur_pos)
            local after  = editor_text:sub(cur_pos + 1)
            local snippet = prefix .. default_text .. suffix
            editor_text = before .. snippet .. after
            just_formatted = true
            if imgui.SetInputTextSelection then
                local new_cursor = cur_pos + #prefix + #default_text
                imgui.SetInputTextSelection(new_cursor, new_cursor)
            end
        end
    else
        -- Fallback if no cursor/selection: append cleanly to end
        if is_line_prefix then
            if #editor_text > 0 and not editor_text:match("\n$") then
                editor_text = editor_text .. "\n"
            end
            editor_text = editor_text .. prefix .. default_text .. "\n"
        else
            editor_text = editor_text .. prefix .. default_text .. suffix
        end
    end
end

-- ----------------------------------------------------------------------------
-- File Persistence (Save & Load)
-- ----------------------------------------------------------------------------
local function get_save_path()
    local path = "FlyWithLua/cockpit_notes.md"
    if CURRENT_SCRIPT_DIRECTORY and #CURRENT_SCRIPT_DIRECTORY > 0 then
        path = CURRENT_SCRIPT_DIRECTORY .. "/cockpit_notes.md"
    end
    return path
end

local function save_document()
    local path = get_save_path()
    local f, err = io.open(path, "w")
    if f then
        f:write(editor_text)
        f:close()
        save_status_msg = "Saved: cockpit_notes.md"
        save_status_path = path
        save_status_time = os.time()
    else
        save_status_msg = "Error Saving Notes"
        save_status_path = tostring(err)
        save_status_time = os.time()
    end
end

local function load_document()
    local path = get_save_path()
    local f, err = io.open(path, "r")
    if f then
        editor_text = f:read("*a")
        f:close()
        save_status_msg = "Loaded: cockpit_notes.md"
        save_status_path = path
        save_status_time = os.time()
    else
        save_status_msg = "No Saved Notes Found"
        save_status_path = path
        save_status_time = os.time()
    end
end

-- Helper for active mode buttons with blue accent styling
local function render_mode_button(label, is_active, tooltip)
    if is_active then
        imgui.PushStyleColor(ImGuiCol_Button, 0.20, 0.45, 0.85, 1.0)
    end
    local clicked = imgui.Button(label)
    if is_active then
        imgui.PopStyleColor()
    end
    if imgui.IsItemHovered() and tooltip then
        imgui.SetTooltip(tooltip)
    end
    return clicked
end

-- 1-Click Toolbar Button: triggers on first click even when multiline text input is active
local last_toolbar_click_time = 0
local function toolbar_button(label, tooltip)
    local now = os.clock()
    local clicked = imgui.Button(label)
    -- If multiline input is active, Button() eats click on first press; detect hover+click directly
    if not clicked and imgui.IsItemHovered and imgui.IsMouseClicked then
        if imgui.IsItemHovered(128) and imgui.IsMouseClicked(0) then
            clicked = true
        end
    end
    if clicked then
        if now - last_toolbar_click_time < 0.25 then
            return false
        end
        last_toolbar_click_time = now
        return true
    end
    if imgui.IsItemHovered() and tooltip then
        imgui.SetTooltip(tooltip)
    end
    return false
end

-- ----------------------------------------------------------------------------
-- Internal Render Routine (Protected by pcall)
-- ----------------------------------------------------------------------------
local function md_editor_render_content()
    local avail_w, avail_h = imgui.GetContentRegionAvail()
    if not avail_w or avail_w <= 0 then
        avail_w = 900
    end

    -- Continuously track active selection while editing
    if imgui.GetInputTextSelection then
        local s, e, c = imgui.GetInputTextSelection()
        if s >= 0 and e > s then
            cached_sel_start = s
            cached_sel_end   = e
            cached_cur_pos   = c
        end
    end

    -- ------------------------------------------------------------------------
    -- TOP BAR: Title & Transient Status Notification (above buttons)
    -- ------------------------------------------------------------------------
    if imgui.PushFont then imgui.PushFont("Roboto-Medium-20") end
    imgui.TextUnformatted("Cockpit Markdown Scratchpad")
    if imgui.PopFont then imgui.PopFont() end

    -- Status notification aligned above the persistence buttons on the right
    if os.time() - save_status_time < 3 and #save_status_msg > 0 then
        local msg_w = 200
        if imgui.CalcTextSize then
            local tw = imgui.CalcTextSize(save_status_msg)
            msg_w = tw + 24
        end
        local badge_x = math.max(300, avail_w - msg_w)
        imgui.SameLine(badge_x)
        if imgui.Badge then
            local variant = save_status_msg:match("^Error") and "danger" or "success"
            imgui.Badge(save_status_msg, variant)
        else
            imgui.TextColored(0.2, 0.8, 0.2, 1.0, save_status_msg)
        end
        if imgui.IsItemHovered() and #save_status_path > 0 then
            imgui.SetTooltip(save_status_path)
        end
    end

    imgui.Separator()

    -- Mode Selectors (Left)
    if render_mode_button("  Write  ", view_mode == "Write", "Editor-only view: maximum horizontal and vertical space for typing.") then
        view_mode = "Write"
    end

    imgui.SameLine()
    if render_mode_button("  Split View  ", view_mode == "Split", "Side-by-side view: editor on left, real-time live preview on right.") then
        view_mode = "Split"
    end

    imgui.SameLine()
    if render_mode_button("  Preview  ", view_mode == "Preview", "Clean reading view: rendered Markdown document only.") then
        view_mode = "Preview"
    end

    -- Persistence Buttons (Right)
    local right_start = avail_w - 260
    local cur_x = (imgui.GetCursorPosX and imgui.GetCursorPosX()) or 0
    if right_start > cur_x + 10 then
        imgui.SameLine(right_start)
    else
        imgui.SameLine()
    end
    if toolbar_button("Save Notes", "Save current text to cockpit_notes.md") then
        save_document()
    end

    imgui.SameLine()
    if toolbar_button("Load Notes", "Reload cockpit_notes.md from disk") then
        load_document()
    end

    imgui.SameLine()
    if toolbar_button("Clear", "Clear entire editor buffer") then
        editor_text = ""
    end

    imgui.Separator()

    -- ------------------------------------------------------------------------
    -- TOOLBAR: Quick Format Buttons & Template Drawer (Visible in Write & Split)
    -- ------------------------------------------------------------------------
    if view_mode ~= "Preview" then
        if toolbar_button("H1", "Prepend Heading 1 (# ...)") then wrap_or_insert("# ", "", "Heading 1", true) end
        imgui.SameLine()
        if toolbar_button("H2", "Prepend Heading 2 (## ...)") then wrap_or_insert("## ", "", "Heading 2", true) end
        imgui.SameLine()
        if toolbar_button(" B ", "Wrap selection in **bold**") then wrap_or_insert("**", "**", "bold text") end
        imgui.SameLine()
        if toolbar_button(" I ", "Wrap selection in *italic*") then wrap_or_insert("*", "*", "italic text") end
        imgui.SameLine()
        if toolbar_button(" U ", "Wrap selection in <u>underline</u>") then wrap_or_insert("<u>", "</u>", "underlined text") end
        imgui.SameLine()
        if toolbar_button(" S ", "Wrap selection in ~~strikethrough~~") then wrap_or_insert("~~", "~~", "strikethrough") end
        imgui.SameLine()
        if toolbar_button("* List", "Convert line or insert bullet list (* ...)") then wrap_or_insert("* ", "", "Item description", true) end
        imgui.SameLine()
        if toolbar_button("[ ] Task", "Convert line or insert checklist task (- [ ] ...)") then wrap_or_insert("- [ ] ", "", "Checklist task to verify", true) end
        imgui.SameLine()
        if toolbar_button("<badge>", "Wrap selection in <badge green|yellow|red>...</badge>") then wrap_or_insert("<badge green>", "</badge>", "NORMAL") end
        imgui.SameLine()
        if toolbar_button("<pill>", "Wrap selection in <pill blue|green|default>...</pill>") then wrap_or_insert("<pill blue>", "</pill>", "ACTIVE") end
        imgui.SameLine()
        if toolbar_button("<kbd>", "Wrap selection in keycap <kbd>Key</kbd>") then wrap_or_insert("<kbd>", "</kbd>", "Key") end
        imgui.SameLine()
        if toolbar_button("Code", "Wrap selection in fenced code block") then wrap_or_insert("```lua\n", "\n```", "-- code block\n") end

        -- Templates Dropdown
        imgui.SameLine()
        imgui.TextUnformatted("|")
        imgui.SameLine()
        if imgui.SetNextItemWidth then
            imgui.SetNextItemWidth(175)
        end
        if imgui.BeginCombo("##templates", "Aviation Templates...", 0) then
            for name, content in pairs(templates) do
                if imgui.Selectable(name, false) then
                    editor_text = content
                end
            end
            imgui.EndCombo()
        end
        if imgui.IsItemHovered() then imgui.SetTooltip("Load pre-formatted cockpit templates (ATIS, CRAFT clearance, checklists)") end

        imgui.Separator()
    end

    -- ------------------------------------------------------------------------
    -- CONTENT PANES: Write, Split, or Preview
    -- ------------------------------------------------------------------------
    if view_mode == "Write" then
        -- 1. Fullscreen Editor
        if imgui.BeginChild("FullEditorChild", 0, 0, 1) then
            local changed, new_text = imgui.InputTextMultiline("##editor_full", editor_text, MAX_TEXT_BYTES, -1, -1, editor_input_flags)
            if just_formatted then
                just_formatted = false
            elseif changed and new_text then
                editor_text = new_text
            end
            imgui.EndChild()
        end

    elseif view_mode == "Preview" then
        -- 2. Fullscreen Live Rendered Preview
        if imgui.BeginChild("FullPreviewChild", 0, 0, 1) then
            imgui.MD.TextWrapped(editor_text)
            imgui.EndChild()
        end

    else -- "Split"
        -- 3. Side-by-Side Dual Pane
        local col_w = math.floor((avail_w - 16) * 0.5)

        -- Left Pane: Editor (with Word-Wrap enabled)
        if imgui.BeginChild("SplitEditorChild", col_w, 0, 1) then
            imgui.TextDisabled("RAW MARKDOWN SOURCE:")
            local changed, new_text = imgui.InputTextMultiline("##editor_split", editor_text, MAX_TEXT_BYTES, -1, -1, editor_input_flags)
            if just_formatted then
                just_formatted = false
            elseif changed and new_text then
                editor_text = new_text
            end
            imgui.EndChild()
        end

        imgui.SameLine(0, 16)

        -- Right Pane: Live Rendered Preview (with Soft-Wrapping enabled)
        if imgui.BeginChild("SplitPreviewChild", col_w, 0, 1) then
            imgui.TextDisabled("LIVE RENDERED PREVIEW:")
            imgui.MD.TextWrapped(editor_text)
            imgui.EndChild()
        end
    end
end

-- ----------------------------------------------------------------------------
-- UI Builder Callback (Wrapped in pcall to guarantee Push/Pop font pairing)
-- ----------------------------------------------------------------------------
function md_editor_on_build(wnd, x, y)
    imgui.PushFont("Roboto-Regular-16")
    local ok, err = pcall(md_editor_render_content)
    imgui.PopFont()

    if not ok then
        imgui.TextUnformatted("Editor UI Error: " .. tostring(err))
    end
end

function md_editor_on_close(wnd)
    -- Autosave on close for user convenience
    save_document()
end

-- ----------------------------------------------------------------------------
-- Window Creation & Initialization
-- ----------------------------------------------------------------------------
local screen_w, screen_h = 1920, 1080
if XPLMGetScreenSize then
    screen_w, screen_h = XPLMGetScreenSize()
end

local wnd_w = 980
local wnd_h = 720
local wnd_x = math.floor((screen_w - wnd_w) * 0.5)
local wnd_y = math.floor((screen_h - wnd_h) * 0.5)

-- Create ImGui floating window (4th arg MUST be true for withImgui)
editor_wnd = float_wnd_create(wnd_w, wnd_h, 1, true)
float_wnd_set_position(editor_wnd, wnd_x, wnd_y)
float_wnd_set_title(editor_wnd, "Cockpit Markdown Notes & Scratchpad")
float_wnd_set_imgui_builder(editor_wnd, "md_editor_on_build")
float_wnd_set_onclose(editor_wnd, "md_editor_on_close")

-- Explicitly disable outer window scrollbars and mouse-wheel scrolling so the
-- window never overflows horizontally. Inner child panes handle vertical scrolling.
if float_wnd_set_scrolling then
    float_wnd_set_scrolling(editor_wnd, false)
end
