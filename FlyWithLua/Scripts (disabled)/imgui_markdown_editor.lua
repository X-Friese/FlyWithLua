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

-- ----------------------------------------------------------------------------
-- State & Buffers
-- ----------------------------------------------------------------------------
local editor_wnd = nil
local view_mode  = "Split"  -- "Write", "Split", "Preview"
local save_status_msg = ""
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

> [!NOTE]
> Use the **Mode** buttons above to switch between pure Editor, Split View, and full-screen Preview.
]]

local editor_text = initial_doc
local MAX_TEXT_BYTES = 65536

-- ----------------------------------------------------------------------------
-- Aviation Templates
-- ----------------------------------------------------------------------------
local templates = {
    ["ATIS Copy"] = [[# ATIS Departure / Arrival Copy
* **Airport:** `KLAX (Los Angeles Intl)`
* **Information:** <badge blue>Bravo</badge>
* **Time (UTC):** `1950Z`
* **Wind:** `250° @ 11kt`
* **Visibility:** `10 SM`
* **Sky:** `SCT 025, BKN 200`
* **Temp / Dew:** `21°C / 14°C`
* **Altimeter:** `29.98 inHg` (1015 hPa)
* **Active Runways:** `24L / 24R / 25L / 25R`
* **Approach in Use:** `ILS Rwy 25L, RNAV (RNP) Rwy 24R`
* **NOTAMs / Remarks:** Caution bird activity vicinity runway 25L.
]],

    ["IFR Clearance (CRAFT)"] = [[# IFR Clearance Copy (C-R-A-F-T)
* **C - Clearance Limit:** `KORD (Chicago O'Hare)`
* **R - Route:** `MERIT4 BLZ J584 OBK DIRECT`
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
-- Helper Functions: Formatting Quick Inserts
-- ----------------------------------------------------------------------------
local function insert_text(snippet)
    editor_text = editor_text .. snippet
end

local function insert_block(snippet)
    if #editor_text > 0 and not editor_text:match("\n$") then
        editor_text = editor_text .. "\n"
    end
    editor_text = editor_text .. snippet .. "\n"
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
        save_status_msg = "Saved to " .. path
        save_status_time = os.time()
    else
        save_status_msg = "Error saving: " .. tostring(err)
        save_status_time = os.time()
    end
end

local function load_document()
    local path = get_save_path()
    local f, err = io.open(path, "r")
    if f then
        editor_text = f:read("*a")
        f:close()
        save_status_msg = "Loaded from " .. path
        save_status_time = os.time()
    else
        save_status_msg = "No file found at " .. path
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

-- ----------------------------------------------------------------------------
-- Internal Render Routine (Protected by pcall)
-- ----------------------------------------------------------------------------
local function md_editor_render_content()
    local avail_w, avail_h = imgui.GetContentRegionAvail()
    if not avail_w or avail_w <= 0 then
        avail_w = 900
    end

    -- ------------------------------------------------------------------------
    -- TOP BAR: Mode Selector & Action Buttons
    -- ------------------------------------------------------------------------
    imgui.MD.Text("# Cockpit Markdown Scratchpad")

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

    -- Persistence Buttons on the Right
    imgui.SameLine(math.max(10, avail_w - 280))
    if imgui.Button("Save Notes") then
        save_document()
    end
    if imgui.IsItemHovered() then imgui.SetTooltip("Save current text to cockpit_notes.md") end

    imgui.SameLine()
    if imgui.Button("Load Notes") then
        load_document()
    end
    if imgui.IsItemHovered() then imgui.SetTooltip("Reload cockpit_notes.md from disk") end

    imgui.SameLine()
    if imgui.Button("Clear") then
        editor_text = ""
    end
    if imgui.IsItemHovered() then imgui.SetTooltip("Clear entire editor buffer") end

    -- Display transient save/load notification
    if os.time() - save_status_time < 3 and #save_status_msg > 0 then
        imgui.SameLine()
        imgui.Badge(save_status_msg, "success")
    end

    imgui.Separator()

    -- ------------------------------------------------------------------------
    -- TOOLBAR: Quick Format Buttons & Template Drawer (Visible in Write & Split)
    -- ------------------------------------------------------------------------
    if view_mode ~= "Preview" then
        if imgui.Button("H1") then insert_block("# New Heading 1") end
        if imgui.IsItemHovered() then imgui.SetTooltip("Insert Heading 1 (# ...)") end

        imgui.SameLine()
        if imgui.Button("H2") then insert_block("## New Heading 2") end
        if imgui.IsItemHovered() then imgui.SetTooltip("Insert Heading 2 (## ...)") end

        imgui.SameLine()
        if imgui.Button(" B ") then insert_text("**bold text** ") end
        if imgui.IsItemHovered() then imgui.SetTooltip("Insert **bold** markup") end

        imgui.SameLine()
        if imgui.Button(" I ") then insert_text("*italic text* ") end
        if imgui.IsItemHovered() then imgui.SetTooltip("Insert *italic* markup") end

        imgui.SameLine()
        if imgui.Button(" U ") then insert_text("<u>underlined text</u> ") end
        if imgui.IsItemHovered() then imgui.SetTooltip("Insert <u>underline</u> markup") end

        imgui.SameLine()
        if imgui.Button(" S ") then insert_text("~~strikethrough~~ ") end
        if imgui.IsItemHovered() then imgui.SetTooltip("Insert ~~strikethrough~~ markup") end

        imgui.SameLine()
        if imgui.Button("• List") then insert_block("* Item description") end
        if imgui.IsItemHovered() then imgui.SetTooltip("Insert unordered bullet item (* ...)") end

        imgui.SameLine()
        if imgui.Button("✓ Task") then insert_block("- [ ] Checklist item to verify") end
        if imgui.IsItemHovered() then imgui.SetTooltip("Insert interactive checklist task (- [ ] ...)") end

        imgui.SameLine()
        if imgui.Button("<badge>") then insert_text("<badge green>NORMAL</badge> ") end
        if imgui.IsItemHovered() then imgui.SetTooltip("Insert status badge (<badge green|yellow|red>...</badge>)") end

        imgui.SameLine()
        if imgui.Button("<pill>") then insert_text("<pill blue>ACTIVE</pill> ") end
        if imgui.IsItemHovered() then imgui.SetTooltip("Insert pill chip (<pill blue|green|default>...</pill>)") end

        imgui.SameLine()
        if imgui.Button("<kbd>") then insert_text("<kbd>Alt+F4</kbd> ") end
        if imgui.IsItemHovered() then imgui.SetTooltip("Insert keycap tag (<kbd>Key</kbd>)") end

        imgui.SameLine()
        if imgui.Button("```Code") then insert_block("```lua\n-- code block\n```") end
        if imgui.IsItemHovered() then imgui.SetTooltip("Insert syntax-highlighted code block") end

        -- Templates Dropdown
        imgui.SameLine()
        imgui.TextUnformatted("|")
        imgui.SameLine()
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
        if imgui.BeginChild("FullEditorChild", avail_w, 0, 1) then
            local changed, new_text = imgui.InputTextMultiline("##editor_full", editor_text, MAX_TEXT_BYTES, -1, -1)
            if changed and new_text then
                editor_text = new_text
            end
            imgui.EndChild()
        end

    elseif view_mode == "Preview" then
        -- 2. Fullscreen Live Rendered Preview
        if imgui.BeginChild("FullPreviewChild", avail_w, 0, 1) then
            imgui.MD.Text(editor_text)
            imgui.EndChild()
        end

    else -- "Split"
        -- 3. Side-by-Side Dual Pane
        local col_w = math.floor((avail_w - 16) * 0.5)

        -- Left Pane: Editor
        if imgui.BeginChild("SplitEditorChild", col_w, 0, 1) then
            imgui.TextDisabled("RAW MARKDOWN SOURCE:")
            local changed, new_text = imgui.InputTextMultiline("##editor_split", editor_text, MAX_TEXT_BYTES, -1, -1)
            if changed and new_text then
                editor_text = new_text
            end
            imgui.EndChild()
        end

        imgui.SameLine(0, 16)

        -- Right Pane: Live Rendered Preview
        if imgui.BeginChild("SplitPreviewChild", col_w, 0, 1) then
            imgui.TextDisabled("LIVE RENDERED PREVIEW:")
            imgui.MD.Text(editor_text)
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

editor_wnd = float_wnd_create(wnd_w, wnd_h, 1, true)
float_wnd_set_position(editor_wnd, wnd_x, wnd_y)
float_wnd_set_title(editor_wnd, "Cockpit Markdown Notes & Scratchpad")
float_wnd_set_imgui_builder(editor_wnd, "md_editor_on_build")
float_wnd_set_onclose(editor_wnd, "md_editor_on_close")
