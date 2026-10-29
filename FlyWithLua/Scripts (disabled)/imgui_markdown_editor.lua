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

-- ImGuiInputTextFlags (WordWrap = 1 << 24, AllowTabInput = 1 << 5, EnterReturnsTrue = 1 << 6)
local ImGuiInputTextFlags_WordWrap = 16777216
local ImGuiInputTextFlags_AllowTabInput = 32
local ImGuiInputTextFlags_EnterReturnsTrue = 64
if imgui.constant and imgui.constant.InputTextFlags then
    if imgui.constant.InputTextFlags.AllowTabInput then
        ImGuiInputTextFlags_AllowTabInput = imgui.constant.InputTextFlags.AllowTabInput
    end
    if imgui.constant.InputTextFlags.WordWrap then
        ImGuiInputTextFlags_WordWrap = imgui.constant.InputTextFlags.WordWrap
    end
    if imgui.constant.InputTextFlags.EnterReturnsTrue then
        ImGuiInputTextFlags_EnterReturnsTrue = imgui.constant.InputTextFlags.EnterReturnsTrue
    end
end
local editor_input_flags = ImGuiInputTextFlags_WordWrap + ImGuiInputTextFlags_AllowTabInput

-- Button Repeat Flag (ImGuiItemFlags_ButtonRepeat = 1 << 3 = 8)
local ImGuiItemFlags_ButtonRepeat = 8
if imgui.constant and imgui.constant.ItemFlags and imgui.constant.ItemFlags.ButtonRepeat then
    ImGuiItemFlags_ButtonRepeat = imgui.constant.ItemFlags.ButtonRepeat
end

local function push_button_repeat(repeat_enabled)
    if imgui.PushButtonRepeat then
        imgui.PushButtonRepeat(repeat_enabled)
    elseif imgui.PushItemFlag then
        imgui.PushItemFlag(ImGuiItemFlags_ButtonRepeat, repeat_enabled)
    end
end

local function pop_button_repeat()
    if imgui.PopButtonRepeat then
        imgui.PopButtonRepeat()
    elseif imgui.PopItemFlag then
        imgui.PopItemFlag()
    end
end

-- ----------------------------------------------------------------------------
-- State & Buffers
-- ----------------------------------------------------------------------------
local editor_wnd = nil
local view_mode  = "Split"  -- "Write", "Split", "Preview"
local save_status_msg = ""
local save_status_path = ""
local save_status_time = 0

-- Content Canvas Zoom State (1 pt quantum increments, range 50% to 200%)
local zoom_pct       = 100   -- Integer percentage: 50 to 200
local content_zoom   = 1.0   -- Multiplier: 0.50 to 2.00
local last_zoom_pct  = nil   -- Memory for 1-click A/B toggle on reset button
local last_zoom_change_time = 0 -- Timestamp of last zoom adjustment for tooltip suppression
local dec_repeat_count = 0   -- Counter to throttle repeat speed on [-] (1/4 rate)
local inc_repeat_count = 0   -- Counter to throttle repeat speed on [+] (1/4 rate)

-- ----------------------------------------------------------------------------
-- File Persistence (Save & Load in Scripts/MarkdownEditor/)
-- ----------------------------------------------------------------------------
local is_windows = (SYSTEM == "IBM") or (package.config:sub(1,1) == "\\")

-- Windows-specific pure Win32 kernel32 FFI file enumeration (Zero Shell / No cmd.exe / No Deadlock)
local ffi_ok, ffi = pcall(require, "ffi")
if is_windows and ffi_ok and ffi then
    pcall(ffi.cdef, [[
        typedef struct _WIN32_FIND_DATAA {
            uint32_t dwFileAttributes;
            uint32_t ftCreationTime_dwLowDateTime;
            uint32_t ftCreationTime_dwHighDateTime;
            uint32_t ftLastAccessTime_dwLowDateTime;
            uint32_t ftLastAccessTime_dwHighDateTime;
            uint32_t ftLastWriteTime_dwLowDateTime;
            uint32_t ftLastWriteTime_dwHighDateTime;
            uint32_t nFileSizeHigh;
            uint32_t nFileSizeLow;
            uint32_t dwReserved0;
            uint32_t dwReserved1;
            char     cFileName[260];
            char     cAlternateFileName[14];
        } WIN32_FIND_DATAA;

        void* FindFirstFileA(const char* lpFileName, WIN32_FIND_DATAA* lpFindFileData);
        int   FindNextFileA(void* hFindFile, WIN32_FIND_DATAA* lpFindFileData);
        int   FindClose(void* hFindFile);
        int   CreateDirectoryA(const char* lpPathName, void* lpSecurityAttributes);
    ]])
end

local function get_kernel32()
    if not (ffi_ok and ffi) then return nil end
    local ok, k32 = pcall(function() return ffi.load("kernel32") end)
    if ok and k32 then return k32 end
    return ffi.C
end

local function scan_notes_windows(dir)
    local files = {}
    if not (ffi_ok and ffi) then return files end
    pcall(function()
        local k32 = get_kernel32()
        if not k32 then return end
        
        -- Ensure directory exists via kernel32 CreateDirectoryA (zero shell)
        k32.CreateDirectoryA(dir, nil)

        local find_data = ffi.new("WIN32_FIND_DATAA")
        local pattern = dir .. "\\*.md"
        local handle = k32.FindFirstFileA(pattern, find_data)
        if handle ~= nil then
            local int_h = tonumber(ffi.cast("intptr_t", handle))
            if int_h and int_h ~= -1 and int_h ~= 0 then
                while true do
                    local fname = ffi.string(find_data.cFileName)
                    if fname and #fname > 0 and fname:match("%.md$") and not fname:match("^%.") then
                        table.insert(files, fname)
                    end
                    if k32.FindNextFileA(handle, find_data) == 0 then
                        break
                    end
                end
                k32.FindClose(handle)
            end
        end
    end)
    return files
end

local function scan_notes_posix(dir)
    local files = {}
    local files_map = {}
    local ok, lfs = pcall(require, "lfs_ffi")
    if ok and lfs then
        if lfs.mkdir then
            pcall(lfs.mkdir, dir)
        end
        if lfs.dir then
            pcall(function()
                for entry in lfs.dir(dir) do
                    if entry and entry:match("%.md$") and not entry:match("^%.") then
                        if not files_map[entry] then
                            files_map[entry] = true
                            table.insert(files, entry)
                        end
                    end
                end
            end)
        end
    end

    -- Fallback to ls on POSIX (macOS & Linux) where popen is standard and safe
    if #files == 0 and io.popen then
        pcall(function()
            local p = io.popen('ls -1 "' .. dir .. '" 2>/dev/null')
            if p then
                for line in p:lines() do
                    line = line:gsub("%s+$", "")
                    if line:match("%.md$") and not line:match("^%.") then
                        if not files_map[line] then
                            files_map[line] = true
                            table.insert(files, line)
                        end
                    end
                end
                p:close()
            end
        end)
    end
    return files
end

local function ensure_directory(dir)
    if is_windows then
        if ffi_ok and ffi then
            pcall(function()
                local k32 = get_kernel32()
                if k32 then k32.CreateDirectoryA(dir, nil) end
            end)
        end
    else
        local ok, lfs = pcall(require, "lfs_ffi")
        if ok and lfs and lfs.mkdir then
            pcall(lfs.mkdir, dir)
        end
    end
end

local function get_notes_dir()
    local sep = is_windows and "\\" or "/"
    local base_dir = "FlyWithLua"
    if CURRENT_SCRIPT_DIRECTORY and #CURRENT_SCRIPT_DIRECTORY > 0 then
        base_dir = CURRENT_SCRIPT_DIRECTORY:gsub("[/\\]+$", "")
    end
    if is_windows then
        base_dir = base_dir:gsub("/", "\\")
    else
        base_dir = base_dir:gsub("\\", "/")
    end
    if base_dir:match("[/\\]MarkdownEditor$") then
        return base_dir
    end
    return base_dir .. sep .. "MarkdownEditor"
end

local function get_legacy_save_path()
    local sep = is_windows and "\\" or "/"
    local base_dir = "FlyWithLua"
    if CURRENT_SCRIPT_DIRECTORY and #CURRENT_SCRIPT_DIRECTORY > 0 then
        base_dir = CURRENT_SCRIPT_DIRECTORY:gsub("[/\\]+$", "")
    end
    if is_windows then
        base_dir = base_dir:gsub("/", "\\")
    else
        base_dir = base_dir:gsub("\\", "/")
    end
    return base_dir .. sep .. "cockpit_notes.md"
end

local current_note_file = "cockpit_notes.md"
local show_new_file_modal = false
local new_file_name_input = ""
local focus_new_note_input = false

local function get_note_path(filename)
    filename = filename or current_note_file
    filename = filename:gsub("%.[mM][dD]$", "") .. ".md"
    local sep = is_windows and "\\" or "/"
    return get_notes_dir() .. sep .. filename
end

local cached_note_files = {}
local last_files_scan_time = 0

local function list_note_files(force)
    local now = os.clock()
    if not force and #cached_note_files > 0 and (now - last_files_scan_time < 1.5) then
        return cached_note_files
    end
    last_files_scan_time = now

    local dir = get_notes_dir()
    ensure_directory(dir)
    local raw_files = is_windows and scan_notes_windows(dir) or scan_notes_posix(dir)

    local files_map = {}
    local files = {}
    if raw_files then
        for _, f in ipairs(raw_files) do
            if not files_map[f] then
                files_map[f] = true
                table.insert(files, f)
            end
        end
    end

    -- Always ensure cockpit_notes.md and current_note_file are included
    if not files_map["cockpit_notes.md"] then
        table.insert(files, "cockpit_notes.md")
        files_map["cockpit_notes.md"] = true
    end
    if current_note_file and not files_map[current_note_file] then
        table.insert(files, current_note_file)
        files_map[current_note_file] = true
    end

    table.sort(files)
    cached_note_files = files
    return cached_note_files
end

-- Default scratchpad template loaded if no saved notes exist on disk
local initial_doc = [[# Cockpit Notes & Scratchpad
Welcome to your in-cockpit **FlyWithLua Markdown Editor**!

* **Live Formatting:** Headers, **bold**, *italic*, <u>underline</u>, and ~~strikethrough~~.
* **Avionics Badges:** <badge green>ENG 1 NORMAL</badge> <badge yellow>FUEL PUMP AUTO</badge> <badge red>FIRE TEST</badge>
* **Keycaps & Shortcuts:** Press <kbd>Alt+F4</kbd> or <kbd>Ctrl+Shift+P</kbd>
* **Checklists:**
  - `[x]` Pre-flight briefing complete
  - `[x]` Altimeter set to standard: `29.92 inHg`
  - `[ ]` Clearance copied and read back

```lua
-- Quick Cockpit Dataref Inspection
local tas = get("sim/flightmodel/position/true_airspeed")
local alt = get("sim/flightmodel/position/elevation")
```

<pill blue>TIP</pill> Use the **Mode** buttons above to switch between pure Editor, Split View, and full-screen Preview.
]]

-- Rich Markdown content for the "About" dialog
local about_markdown_text = [[# Cockpit Markdown Notes & Scratchpad
<badge green>v1.0</badge> <pill blue>FlyWithLua Next-Gen</pill>

An interactive in-cockpit Markdown scratchpad and flight notes system for X-Plane 12 flight operations.

---

### Key Capabilities
* **Live Markdown Typography:** Headers (`#`, `##`), **bold**, *italic*, <u>underline</u>, ~~strikethrough~~, bulleted lists, and formatted code blocks with syntax styling.
* **Avionics Badges & Annunciators:** Instrument callouts and status indicators using `<badge green|yellow|red>` and `<pill blue|green|default>`.
* **Cockpit Checklists & Tasks:** Prepend `- [ ]` or click `[ ] Task` to format verify-and-execute flight crew checklists with inline monospace boxes.
* **Tactile Canvas Zoom:** Dynamic font scaling from **50% to 200%** with 1-point quantum steppers, mouse-wheel adjust, and 1-click A/B toggle back to 100% baseline.
* **Dedicated Multi-Note Storage:** Automatic file persistence in `FlyWithLua/Scripts/MarkdownEditor/` with active note file selection, manual Finder file discovery, and instant new note creation.
* **Three Workflow Modes:**
  - **Write:** High-efficiency source editor.
  - **Split:** Live side-by-side editing with instant soft-wrapped Markdown preview.
  - **Preview:** Distraction-free, reading-only clipboard for flight ops.

---

### Credits & Technologies
* **FlyWithLua:** Enhanced Lua scripting engine for X-Plane.
* **Dear ImGui:** Bloat-free immediate-mode GUI by Omar Cornut.
* **imgui_markdown:** Markdown rendering extension by Juliette Foucaut, enhanced with Roger2 and ImgWindow extensions.
* **ImgWindow:** Modern Floating Window integration by Steven L. Goldberg.
]]

-- Load saved notes on startup if available, otherwise fall back to template
local function load_initial_document()
    local path = get_note_path("cockpit_notes.md")
    local f = io.open(path, "r")
    if f then
        local content = f:read("*a")
        f:close()
        if content and #content > 0 then
            current_note_file = "cockpit_notes.md"
            return content
        end
    end
    -- Check legacy location in Scripts/
    local leg_path = get_legacy_save_path()
    f = io.open(leg_path, "r")
    if f then
        local content = f:read("*a")
        f:close()
        if content and #content > 0 then
            -- Auto-migrate to MarkdownEditor directory
            ensure_directory(get_notes_dir())
            local fw = io.open(path, "w")
            if fw then
                fw:write(content)
                fw:close()
            end
            current_note_file = "cockpit_notes.md"
            return content
        end
    end
    return initial_doc
end

local editor_text = load_initial_document()
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
- `[ ]` Readback verified and accepted by ATC
- `[ ]` Squawk set in Transponder
- `[ ]` Initial altitude armed on MCP
]],

    ["Pre-Flight Checklist"] = [[# Cockpit Pre-Flight Checklist
- `[ ]` Weather & NOTAM briefing reviewed
- `[ ]` Fuel onboard verified: `14,200 lbs`
- `[ ]` Weight & Balance computed (CG within envelope)
- `[ ]` FMS flight plan & waypoints cross-checked
- `[ ]` Altimeters cross-checked & set: `29.92`
- `[ ]` V-Speeds calculated: `V1: 138`, `VR: 144`, `V2: 151`
- `[ ]` Transponder: <pill green>ALT / 1200</pill>
- `[ ]` Thrust reversers checked: <badge green>ARMED</badge>
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
local has_active_selection = false
local cached_sel_start = -1
local cached_sel_end   = -1
local cached_cur_pos   = -1
local just_formatted   = false
local editor_epoch     = 0

local function wrap_or_insert(prefix, suffix, default_text, is_line_prefix)
    suffix = suffix or ""
    default_text = default_text or ""

    local sel_start, sel_end, cur_pos = -1, -1, -1
    if imgui.GetInputTextSelection then
        sel_start, sel_end, cur_pos = imgui.GetInputTextSelection()
    end

    -- If the text input is active with a real selection, use it directly
    local is_selected = (sel_start >= 0 and sel_end > sel_start)
    if not is_selected and has_active_selection and cached_sel_start >= 0 and cached_sel_end > cached_sel_start then
        -- Button click just blurred the text box during the current interaction
        sel_start = cached_sel_start
        sel_end   = cached_sel_end
        cur_pos   = cached_cur_pos
        is_selected = true
    end

    if is_selected and sel_end <= #editor_text then
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
            end
        else
            -- Wrap the exact selection in-place:
            local before = editor_text:sub(1, sel_start)
            local selected = editor_text:sub(sel_start + 1, sel_end)
            local after = editor_text:sub(sel_end + 1)

            -- Trim leading and trailing whitespace/newlines so tags only wrap the word/text itself
            local lead_ws = selected:match("^%s*") or ""
            local trail_ws = selected:match("%s*$") or ""
            local core_text = selected
            if #lead_ws + #trail_ws < #selected then
                core_text = selected:sub(#lead_ws + 1, #selected - #trail_ws)
            else
                lead_ws = ""
                trail_ws = ""
            end

            editor_text = before .. lead_ws .. prefix .. core_text .. suffix .. trail_ws .. after
            just_formatted = true
            if imgui.SetInputTextSelection then
                local new_start = sel_start + #lead_ws + #prefix
                local new_end   = new_start + #core_text
                imgui.SetInputTextSelection(new_start, new_end)
            end
        end
    else
        -- NO active selection: This is a pure INSERT with placeholder text selected!
        local ins_pos = cur_pos
        if ins_pos < 0 or ins_pos > #editor_text then
            if cached_cur_pos >= 0 and cached_cur_pos <= #editor_text then
                ins_pos = cached_cur_pos
            else
                ins_pos = #editor_text
            end
        end

        if is_line_prefix then
            -- Find start of line at ins_pos
            local line_start = 1
            local prev_nl = editor_text:sub(1, ins_pos):reverse():find("\n")
            if prev_nl then
                line_start = ins_pos - prev_nl + 2
            end
            -- Check if this line is essentially empty
            local next_nl = editor_text:find("\n", line_start) or (#editor_text + 1)
            local line_content = editor_text:sub(line_start, next_nl - 1)
            if #line_content:gsub("%s", "") == 0 then
                -- Empty line: insert prefix + default_text, with default_text selected
                local before = editor_text:sub(1, line_start - 1)
                local after  = editor_text:sub(next_nl)
                editor_text = before .. prefix .. default_text .. after
                just_formatted = true
                if imgui.SetInputTextSelection then
                    local sel_s = #before + #prefix
                    local sel_e = sel_s + #default_text
                    imgui.SetInputTextSelection(sel_s, sel_e)
                end
            else
                -- Line has existing text: prepend prefix to the line
                local before = editor_text:sub(1, line_start - 1)
                local after  = editor_text:sub(line_start)
                editor_text = before .. prefix .. after
                just_formatted = true
                if imgui.SetInputTextSelection then
                    imgui.SetInputTextSelection(ins_pos + #prefix, ins_pos + #prefix)
                end
            end
        else
            -- Inline insert: insert prefix .. default_text .. suffix, with default_text selected!
            local before = editor_text:sub(1, ins_pos)
            local after  = editor_text:sub(ins_pos + 1)
            editor_text = before .. prefix .. default_text .. suffix .. after
            just_formatted = true
            if imgui.SetInputTextSelection then
                local sel_s = ins_pos + #prefix
                local sel_e = sel_s + #default_text
                imgui.SetInputTextSelection(sel_s, sel_e)
            end
        end
    end

    -- Always clear cached selection state after an action so subsequent clicks never re-wrap
    has_active_selection = false
    cached_sel_start = -1
    cached_sel_end   = -1
end

local function save_document(filename)
    filename = filename or current_note_file
    filename = filename:gsub("%.[mM][dD]$", "") .. ".md"
    current_note_file = filename
    local dir = get_notes_dir()
    ensure_directory(dir)
    local path = get_note_path(filename)
    local f, err = io.open(path, "w")
    if f then
        f:write(editor_text)
        f:close()
        save_status_msg = "Saved: " .. filename
        save_status_path = path
        save_status_time = os.time()
        list_note_files(true)
    else
        save_status_msg = "Error Saving Notes"
        save_status_path = tostring(err)
        save_status_time = os.time()
    end
end

local function load_document(filename)
    filename = filename or current_note_file
    filename = filename:gsub("%.[mM][dD]$", "") .. ".md"
    local path = get_note_path(filename)
    local f, err = io.open(path, "r")
    if not f and filename == "cockpit_notes.md" then
        -- Check legacy path
        local leg_path = get_legacy_save_path()
        f = io.open(leg_path, "r")
    end
    if f then
        editor_text = f:read("*a")
        f:close()
        current_note_file = filename
        editor_epoch = editor_epoch + 1
        just_formatted = true
        save_status_msg = "Loaded: " .. filename
        save_status_path = path
        save_status_time = os.time()
        list_note_files(true)
    else
        save_status_msg = "File not found: " .. filename
        save_status_path = path
        save_status_time = os.time()
    end
end

-- Helper to dismiss and reset the stationary TimedTooltip low-watermark timer
local function clear_active_tooltip()
    if imgui.TimedTooltip and imgui.TimedTooltip.ClearActiveTooltip then
        imgui.TimedTooltip.ClearActiveTooltip()
    elseif imgui.TimedTooltip and imgui.TimedTooltip.clear then
        imgui.TimedTooltip.clear()
    elseif TimedTooltip and TimedTooltip.clear then
        TimedTooltip.clear()
    elseif imgui.ClearActiveTooltip then
        imgui.ClearActiveTooltip()
    end
end

-- Helper function to render modern Markdown TimedTooltips (or fallback to SetTooltip)
local function show_tooltip(md_text)
    if not md_text or #md_text == 0 then return end
    if imgui.TimedTooltip and imgui.TimedTooltip.TextMD then
        imgui.TimedTooltip.TextMD(md_text)
    elseif imgui.IsItemHovered and imgui.IsItemHovered() and imgui.SetTooltip then
        imgui.SetTooltip(md_text)
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
    if tooltip then
        show_tooltip(tooltip)
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
    if tooltip then
        show_tooltip(tooltip)
    end
    return false
end

-- 1-Click Copy Buffer Button with 1.5s "Copied!" feedback and TimedTooltip
local last_copied_time = 0
local function render_copy_button(btn_id, text_to_copy)
    if imgui.CopyButton then
        local clicked, is_copied = imgui.CopyButton("##" .. btn_id, text_to_copy)
        return clicked
    end

    local now = os.clock()
    local is_copied = (now - last_copied_time < 1.5)
    local label = is_copied and "Copied!" or "Copy"

    if is_copied then
        imgui.PushStyleColor(ImGuiCol_Button, 0.15, 0.55, 0.25, 1.0)
    end
    local clicked = imgui.Button(label .. "##" .. btn_id)
    if is_copied then
        imgui.PopStyleColor()
    end

    if clicked then
        if imgui.SetClipboardText then
            imgui.SetClipboardText(text_to_copy)
        end
        last_copied_time = now
    end

    if is_copied then
        show_tooltip("**Copied to Clipboard!**\n\nEntire Markdown source buffer copied.")
    else
        show_tooltip("**Copy Buffer**\n\nCopy the entire raw Markdown source text to the OS clipboard.")
    end

    return clicked
end

-- Helper to format checklist tasks with monospace code spans so [ ] and [x] match in width
local function format_tasks(text)
    if not text or not text:find("%[") then return text end
    return (text:gsub("(\n%s*[%-*]%s+)%[%s*%]", "%1`[ ]`")
                :gsub("(\n%s*[%-*]%s+)%[[xX]%]", "%1`[x]`")
                :gsub("^(%s*[%-*]%s+)%[%s*%]", "%1`[ ]`")
                :gsub("^(%s*[%-*]%s+)%[[xX]%]", "%1`[x]`"))
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
        if s >= 0 then
            if e > s then
                has_active_selection = true
                cached_sel_start = s
                cached_sel_end   = e
            else
                has_active_selection = false
                cached_sel_start = -1
                cached_sel_end   = -1
            end
            if c >= 0 then
                cached_cur_pos = c
            end
        end
    end


    -- ------------------------------------------------------------------------
    -- TOP BAR: Title & Transient Status Notification (above buttons)
    -- ------------------------------------------------------------------------
    if imgui.PushFont then imgui.PushFont("Roboto-Medium-20") end
    imgui.TextUnformatted("Cockpit Markdown Scratchpad")
    if imgui.PopFont then imgui.PopFont() end

    imgui.SameLine(0, 10)
    local about_clicked = false
    if imgui.SmallButton then
        about_clicked = imgui.SmallButton("About")
    else
        about_clicked = imgui.Button("About")
    end
    if about_clicked then
        if imgui.OpenPopup then
            imgui.OpenPopup("About Cockpit Markdown Scratchpad")
        end
    end
    show_tooltip("**About Cockpit Markdown Scratchpad**\n\nView features, version, and library credits.")

    -- Status notification aligned above the persistence buttons on the right
    if os.time() - save_status_time < 3 and #save_status_msg > 0 then
        local msg_w = 200
        if imgui.CalcTextSize then
            local tw = imgui.CalcTextSize(save_status_msg)
            msg_w = tw + 24
        end
        local badge_x = math.max(380, avail_w - msg_w)
        imgui.SameLine(badge_x)
        if imgui.Badge then
            local variant = save_status_msg:match("^Error") and "danger" or "success"
            imgui.Badge(save_status_msg, variant)
        else
            imgui.TextColored(0.2, 0.8, 0.2, 1.0, save_status_msg)
        end
        if #save_status_path > 0 then
            show_tooltip("**File Path:**\n`" .. save_status_path .. "`")
        end
    end

    imgui.Separator()

    -- Mode Selectors (Left)
    if render_mode_button("  Write  ", view_mode == "Write", "**Write Mode** (Editor Only)\n\nMaximum horizontal and vertical space for drafting notes.") then
        view_mode = "Write"
    end

    imgui.SameLine()
    if render_mode_button("  Split View  ", view_mode == "Split", "**Split View** (Side-by-Side)\n\nLive side-by-side editing on the left with instant Markdown preview on the right.") then
        view_mode = "Split"
    end

    imgui.SameLine()
    if render_mode_button("  Preview  ", view_mode == "Preview", "**Preview Mode** (Reader Only)\n\nClean reading view displaying formatted Markdown typography, badges, and checklists.") then
        view_mode = "Preview"
    end

    -- Content Zoom Controls in Header Chrome (Unscaled baseline 1.0x)
    imgui.SameLine(0, 14)
    imgui.TextUnformatted("|")
    imgui.SameLine(0, 14)
    imgui.TextUnformatted("Zoom:")
    imgui.SameLine(0, 8)

    -- Zoom Out Stepper [-] (Steps by 1 exact font pixel: 16px -> 15px -> 14px...)
    push_button_repeat(true)
    local dec_clicked = imgui.Button(" - ##zoom_dec")
    pop_button_repeat()
    if dec_clicked then
        local is_initial = (imgui.IsItemActivated and imgui.IsItemActivated()) or (imgui.IsMouseClicked and imgui.IsMouseClicked(0))
        local allow_step = false
        if is_initial then
            dec_repeat_count = 0
            allow_step = true
        else
            dec_repeat_count = dec_repeat_count + 1
            if (dec_repeat_count % 4) == 0 then
                allow_step = true
            end
        end
        if allow_step then
            local cur_pt = math.floor(content_zoom * 16.0 + 0.5)
            local new_pt = math.max(8, cur_pt - 1)
            content_zoom = new_pt / 16.0
            zoom_pct = math.floor(content_zoom * 100.0 + 0.5)
            last_zoom_change_time = os.clock()
            clear_active_tooltip()
        end
    end
    local now = os.clock()
    local zoom_adjusting = (now - last_zoom_change_time < 0.6) or (imgui.IsItemActive and imgui.IsItemActive())
    if not zoom_adjusting then
        show_tooltip(string.format("**Zoom Out** (-1 pt)\n\nDecrease canvas font size to %d px.\n\n*Hold down to step smoothly*", math.max(8, math.floor(content_zoom * 16.0 + 0.5) - 1)))
    end

    imgui.SameLine(0, 4)

    -- Zoom Slider (with mouse-wheel adjust snapping to exact font pixels)
    if imgui.SetNextItemWidth then imgui.SetNextItemWidth(90) end
    local slider_changed, new_zoom = imgui.SliderInt("##zoom_slider", zoom_pct, 50, 200, "%d%%")
    local is_scrolling_zoom = false
    if slider_changed and new_zoom then
        local target_pt = math.floor((new_zoom / 100.0) * 16.0 + 0.5)
        target_pt = math.max(8, math.min(32, target_pt))
        content_zoom = target_pt / 16.0
        zoom_pct = math.floor(content_zoom * 100.0 + 0.5)
        last_zoom_change_time = os.clock()
        clear_active_tooltip()
    end
    if imgui.IsItemHovered() then
        -- Query and consume mouse wheel delta immediately so no residual ticks leak
        local wheel = (imgui.GetMouseWheel and imgui.GetMouseWheel(true)) or 0
        if wheel == 0 and imgui.GetMouseWheelEither then
            wheel = imgui.GetMouseWheelEither(true)
        end
        if wheel ~= 0 then
            is_scrolling_zoom = true
            local cur_pt = math.floor(content_zoom * 16.0 + 0.5)
            local step = (wheel > 0) and 1 or -1
            local new_pt = math.max(8, math.min(32, cur_pt + step))
            content_zoom = new_pt / 16.0
            zoom_pct = math.floor(content_zoom * 100.0 + 0.5)
            last_zoom_change_time = os.clock()
            clear_active_tooltip()
        end
    end
    now = os.clock()
    zoom_adjusting = is_scrolling_zoom or (now - last_zoom_change_time < 0.6) or (imgui.IsItemActive and imgui.IsItemActive())
    if zoom_adjusting then
        clear_active_tooltip()
    else
        show_tooltip(string.format("**Content Canvas Zoom:** `%d%%` (`%d px`)\n\n* **Steppers & Wheel:** Step by 1 exact font pixel\n* **100%% Baseline:** 16 px\n* **Reset (↺):** Toggle 100%% and prior zoom", zoom_pct, math.floor(content_zoom * 16.0 + 0.5)))
    end

    imgui.SameLine(0, 4)

    -- Zoom In Stepper [+] (Steps by 1 exact font pixel: 16px -> 17px -> 18px...)
    push_button_repeat(true)
    local inc_clicked = imgui.Button(" + ##zoom_inc")
    pop_button_repeat()
    if inc_clicked then
        local is_initial = (imgui.IsItemActivated and imgui.IsItemActivated()) or (imgui.IsMouseClicked and imgui.IsMouseClicked(0))
        local allow_step = false
        if is_initial then
            inc_repeat_count = 0
            allow_step = true
        else
            inc_repeat_count = inc_repeat_count + 1
            if (inc_repeat_count % 4) == 0 then
                allow_step = true
            end
        end
        if allow_step then
            local cur_pt = math.floor(content_zoom * 16.0 + 0.5)
            local new_pt = math.min(32, cur_pt + 1)
            content_zoom = new_pt / 16.0
            zoom_pct = math.floor(content_zoom * 100.0 + 0.5)
            last_zoom_change_time = os.clock()
            clear_active_tooltip()
        end
    end
    now = os.clock()
    zoom_adjusting = (now - last_zoom_change_time < 0.6) or (imgui.IsItemActive and imgui.IsItemActive())
    if not zoom_adjusting then
        show_tooltip(string.format("**Zoom In** (+1 pt)\n\nIncrease canvas font size to %d px.\n\n*Hold down to step smoothly*", math.min(32, math.floor(content_zoom * 16.0 + 0.5) + 1)))
    end

    imgui.SameLine(0, 4)

    -- Resettable Button with A/B Toggle Memory (↺)
    local is_modified = (zoom_pct ~= 100)
    local reset_clicked = false
    if imgui.ResetButton then
        reset_clicked = imgui.ResetButton("##reset_zoom", is_modified, false)
    else
        reset_clicked = imgui.Button("100%##reset_zoom")
    end
    if reset_clicked then
        if zoom_pct ~= 100 then
            last_zoom_pct = zoom_pct
            zoom_pct = 100
            content_zoom = 1.0
        elseif last_zoom_pct then
            local tmp = zoom_pct
            zoom_pct = last_zoom_pct
            last_zoom_pct = tmp
            content_zoom = zoom_pct / 100.0
        end
        last_zoom_change_time = os.clock()
        clear_active_tooltip()
    end
    if zoom_pct ~= 100 then
        show_tooltip("**Reset Zoom (100%)**\n\nReset canvas font scale back to 100% baseline.")
    elseif last_zoom_pct then
        show_tooltip(string.format("**Restore Zoom (%d%%)**\n\n1-click A/B toggle back to prior %d%% zoom.", last_zoom_pct, last_zoom_pct))
    else
        show_tooltip("**Zoom Baseline (100%)**\n\nCanvas font scale is at 100% baseline.")
    end

    -- Persistence & Note File Selector (Right)
    local right_start = avail_w - 280
    local cur_x = (imgui.GetCursorPosX and imgui.GetCursorPosX()) or 0
    if right_start > cur_x + 10 then
        imgui.SameLine(right_start)
    else
        imgui.SameLine()
    end

    imgui.TextUnformatted("Note:")
    imgui.SameLine(0, 4)
    if imgui.SetNextItemWidth then
        imgui.SetNextItemWidth(140)
    end
    if imgui.BeginCombo("##active_note_file", current_note_file, 0) then
        local ok, err = pcall(function()
            local note_files = list_note_files()
            for _, nfile in ipairs(note_files) do
                local is_selected = (nfile == current_note_file)
                if imgui.Selectable(nfile, is_selected) then
                    if nfile ~= current_note_file then
                        load_document(nfile)
                    end
                end
                if is_selected and imgui.SetItemDefaultFocus then
                    imgui.SetItemDefaultFocus()
                end
            end
            imgui.Separator()
            if imgui.Selectable("+ New Note...", false) then
                show_new_file_modal = true
                new_file_name_input = ""
                focus_new_note_input = true
            end
        end)
        imgui.EndCombo()
        if not ok then
            print("[FlyWithLua MarkdownEditor] Error in Note Combo: " .. tostring(err))
        end
    end
    show_tooltip("**Active Note File**\n\n* Select any `.md` file in `Scripts/MarkdownEditor/`\n* Files dropped into folder in Finder/Explorer appear automatically\n* Click `+ New Note...` to create a new file")

    imgui.SameLine(0, 4)
    if toolbar_button("Save", string.format("**Save Note**\n\nPersist buffer to `Scripts/MarkdownEditor/%s` on disk.", current_note_file)) then
        save_document(current_note_file)
    end

    imgui.SameLine(0, 4)
    if toolbar_button("+ New", "**Create New Note**\n\nCreate a new named Markdown file in `Scripts/MarkdownEditor/`.") then
        show_new_file_modal = true
        new_file_name_input = ""
        focus_new_note_input = true
    end

    -- Optional inline bar to name and create a new note (aligned to right under Note selector)
    if show_new_file_modal then
        imgui.Separator()
        local modal_w = 350
        local modal_start = avail_w - modal_w
        if modal_start > 0 and imgui.SetCursorPosX then
            imgui.SetCursorPosX(modal_start)
        end
        imgui.TextUnformatted("New Note:")
        imgui.SameLine(0, 4)
        if imgui.SetNextItemWidth then imgui.SetNextItemWidth(130) end
        if focus_new_note_input then
            if imgui.SetKeyboardFocusHere then
                imgui.SetKeyboardFocusHere(0)
            end
            focus_new_note_input = false
        end
        local enter_pressed, new_val = imgui.InputText("##new_note_input", new_file_name_input, 64, ImGuiInputTextFlags_EnterReturnsTrue)
        if new_val then
            new_file_name_input = new_val
        end
        imgui.SameLine(0, 4)
        imgui.TextDisabled(".md")
        imgui.SameLine(0, 6)
        local create_clicked = imgui.Button("Create##new_note_ok")
        if (create_clicked or enter_pressed) then
            local raw_name = new_file_name_input:gsub("^%s+", ""):gsub("%s+$", "")
            raw_name = raw_name:gsub("%.[mM][dD]$", "")
            local clean_name = raw_name:gsub("[^%w_%-]", "_")
            if #clean_name > 0 then
                local file_name = clean_name .. ".md"
                current_note_file = file_name
                editor_text = "# " .. raw_name .. "\n\n"
                editor_epoch = editor_epoch + 1
                just_formatted = true
                save_document(file_name)
                save_status_msg = "Created note: " .. file_name
                show_new_file_modal = false
            end
        end
        imgui.SameLine(0, 4)
        if imgui.Button("Cancel##new_note_cancel") then
            show_new_file_modal = false
        end
    end

    imgui.Separator()

    -- ------------------------------------------------------------------------
    -- TOOLBAR: Quick Format Buttons & Template Drawer (Visible in Write & Split)
    -- ------------------------------------------------------------------------
    if view_mode ~= "Preview" then
        if toolbar_button("H1", "**Heading 1**\n\nPrepend `# ` to the current line.") then wrap_or_insert("# ", "", "Heading 1", true) end
        imgui.SameLine()
        if toolbar_button("H2", "**Heading 2**\n\nPrepend `## ` to the current line.") then wrap_or_insert("## ", "", "Heading 2", true) end
        imgui.SameLine()
        if toolbar_button(" B ", "**Bold Text**\n\nWrap selected text in `**bold**` asterisks.") then wrap_or_insert("**", "**", "bold text") end
        imgui.SameLine()
        if toolbar_button(" I ", "**Italic Text**\n\nWrap selected text in `*italic*` asterisks.") then wrap_or_insert("*", "*", "italic text") end
        imgui.SameLine()
        if toolbar_button(" U ", "**Underline**\n\nWrap selected text in `<u>underline</u>` tags.") then wrap_or_insert("<u>", "</u>", "underlined text") end
        imgui.SameLine()
        if toolbar_button(" S ", "**Strikethrough**\n\nWrap selected text in `~~strikethrough~~` tildes.") then wrap_or_insert("~~", "~~", "strikethrough") end
        imgui.SameLine()
        if toolbar_button("* List", "**Bullet List Item**\n\nPrepend `* ` bullet to current line or selection.") then wrap_or_insert("* ", "", "Item description", true) end
        imgui.SameLine()
        if toolbar_button("[ ] Task", "**Checklist Task Item**\n\nPrepend `- `[ ]` ` interactive checklist item.") then wrap_or_insert("- `[ ]` ", "", "Checklist task to verify", true) end
        imgui.SameLine()
        if toolbar_button("<badge>", "**Avionics Badge**\n\nWrap selection in `<badge green|yellow|red>...</badge>` for instrument annunciators.") then wrap_or_insert("<badge green>", "</badge>", "NORMAL") end
        imgui.SameLine()
        if toolbar_button("<pill>", "**Avionics Pill**\n\nWrap selection in `<pill blue|green|default>...</pill>` for system mode callouts.") then wrap_or_insert("<pill blue>", "</pill>", "ACTIVE") end
        imgui.SameLine()
        if toolbar_button("<kbd>", "**Keyboard Keycap**\n\nWrap selection in physical keycap `<kbd>Key</kbd>`.") then wrap_or_insert("<kbd>", "</kbd>", "Key") end
        imgui.SameLine()
        if toolbar_button("Code", "**Lua Code Block**\n\nWrap selection in fenced ```lua block with live preview.") then wrap_or_insert("```lua\n", "\n```", "-- code block\n") end

        -- Templates Dropdown
        imgui.SameLine()
        imgui.TextUnformatted("|")
        imgui.SameLine()
        if imgui.SetNextItemWidth then
            imgui.SetNextItemWidth(175)
        end
        if imgui.BeginCombo("##templates", "Aviation Templates...", 0) then
            local ok, err = pcall(function()
                for name, content in pairs(templates) do
                    if imgui.Selectable(name, false) then
                        editor_text = content
                        editor_epoch = editor_epoch + 1
                        just_formatted = true
                    end
                end
            end)
            imgui.EndCombo()
            if not ok then
                print("[FlyWithLua MarkdownEditor] Error in Templates Combo: " .. tostring(err))
            end
        end
        show_tooltip("**Aviation Templates**\n\nLoad pre-formatted cockpit checklists and scratchpads:\n* **ATIS Copy Pad**\n* **IFR Clearance (CRAFT)**\n* **Pre-Flight Checklist**\n* **Diversion Briefing**\n* **FlyWithLua Scratchpad**")

        imgui.Separator()
    end

    -- ------------------------------------------------------------------------
    -- CONTENT PANES: Write, Split, or Preview
    -- ------------------------------------------------------------------------
    if view_mode == "Write" then
        -- 1. Fullscreen Editor
        if imgui.BeginChild("FullEditorChild", 0, 0, 1) then
            imgui.TextDisabled("RAW MARKDOWN SOURCE:")

            local win_w = (imgui.GetWindowWidth and imgui.GetWindowWidth()) or avail_w
            local copy_fn = imgui.CopyBufferButton or imgui.CopyButton
            local right_target = win_w - (copy_fn and 135 or 65)
            local cur_x = (imgui.GetCursorPosX and imgui.GetCursorPosX()) or 160
            if right_target > cur_x + 10 then
                imgui.SameLine(right_target)
            else
                imgui.SameLine()
            end
            if toolbar_button("Clear##full", "**Clear Buffer**\n\nEmpty the entire editor scratchpad.") then
                editor_text = ""
                editor_epoch = editor_epoch + 1
                just_formatted = true
            end
            if copy_fn then
                imgui.SameLine(0, 4)
                copy_fn("##copy_buf_full", editor_text)
            end

            local editor_font = "RobotoMono-Regular"
            local editor_font_size = 16.0 * content_zoom
            if imgui.PushFont then
                imgui.PushFont(editor_font, editor_font_size)
            end
            local changed, new_text = imgui.InputTextMultiline("##editor_full_" .. editor_epoch, editor_text, MAX_TEXT_BYTES, -1, -1, editor_input_flags)
            if imgui.PopFont then
                imgui.PopFont()
            end

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
            imgui.TextDisabled("RENDERED MARKDOWN PREVIEW:")

            local win_w = (imgui.GetWindowWidth and imgui.GetWindowWidth()) or avail_w
            local copy_fn = imgui.CopyBufferButton or imgui.CopyButton
            if copy_fn then
                -- Reserve extra clearance (108 px) for the vertical scrollbar track
                local right_target = win_w - 108
                local cur_x = (imgui.GetCursorPosX and imgui.GetCursorPosX()) or 180
                if right_target > cur_x + 10 then
                    imgui.SameLine(right_target)
                else
                    imgui.SameLine()
                end
                copy_fn("##copy_buf_preview", editor_text)
            end

            if imgui.SetWindowFontScale then
                imgui.SetWindowFontScale(content_zoom)
            end
            imgui.MD.TextWrapped(format_tasks(editor_text))
            imgui.EndChild()
        end

    else -- "Split"
        -- 3. Side-by-Side Dual Pane
        local col_w = math.floor((avail_w - 16) * 0.5)

        -- Left Pane: Editor (with Word-Wrap enabled)
        if imgui.BeginChild("SplitEditorChild", col_w, 0, 1) then
            imgui.TextDisabled("RAW MARKDOWN SOURCE:")

            local win_w = (imgui.GetWindowWidth and imgui.GetWindowWidth()) or col_w
            local copy_fn = imgui.CopyBufferButton or imgui.CopyButton
            local right_target = win_w - (copy_fn and 135 or 65)
            local cur_x = (imgui.GetCursorPosX and imgui.GetCursorPosX()) or 160
            if right_target > cur_x + 10 then
                imgui.SameLine(right_target)
            else
                imgui.SameLine()
            end
            if toolbar_button("Clear##split", "**Clear Buffer**\n\nEmpty the entire editor scratchpad.") then
                editor_text = ""
                editor_epoch = editor_epoch + 1
                just_formatted = true
            end
            if copy_fn then
                imgui.SameLine(0, 4)
                copy_fn("##copy_buf_split", editor_text)
            end

            local editor_font = "RobotoMono-Regular"
            local editor_font_size = 16.0 * content_zoom
            if imgui.PushFont then
                imgui.PushFont(editor_font, editor_font_size)
            end
            local changed, new_text = imgui.InputTextMultiline("##editor_split_" .. editor_epoch, editor_text, MAX_TEXT_BYTES, -1, -1, editor_input_flags)
            if imgui.PopFont then
                imgui.PopFont()
            end

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

            if imgui.SetWindowFontScale then
                imgui.SetWindowFontScale(content_zoom)
            end
            imgui.MD.TextWrapped(format_tasks(editor_text))

            imgui.EndChild()
        end
    end

    -- ------------------------------------------------------------------------
    -- ABOUT MODAL POPUP
    -- ------------------------------------------------------------------------
    if imgui.SetNextWindowSize then
        imgui.SetNextWindowSize(560, 480, 2) -- ImGuiCond_FirstUseEver
    end
    if imgui.BeginPopupModal and imgui.BeginPopupModal("About Cockpit Markdown Scratchpad") then
        if imgui.BeginChild("AboutScrollArea", 0, -38, 1) then
            imgui.MD.TextWrapped(about_markdown_text)
            imgui.EndChild()
        end
        imgui.Separator()
        if imgui.Button("  Close  ", 100, 0) then
            imgui.CloseCurrentPopup()
        end
        imgui.EndPopup()
    end
end

-- ----------------------------------------------------------------------------
-- UI Builder Callback (Wrapped in pcall to guarantee Push/Pop font pairing)
-- ----------------------------------------------------------------------------
function md_editor_on_build(wnd, x, y)
    if imgui.SetWindowFontScale then
        imgui.SetWindowFontScale(1.0)
    end
    imgui.PushFont("Roboto-Regular-16")
    local ok, err = pcall(md_editor_render_content)
    imgui.PopFont()

    if not ok then
        imgui.TextUnformatted("Editor UI Error: " .. tostring(err))
    end
end

function md_editor_on_close(wnd)
    -- Do not overwrite notes on window close; saves must be explicit via 'Save Notes'
end

-- ----------------------------------------------------------------------------
-- Window Creation & Initialization
-- ----------------------------------------------------------------------------
local screen_w, screen_h = 1920, 1080
if XPLMGetScreenSize then
    screen_w, screen_h = XPLMGetScreenSize()
end

local wnd_w = 980
local wnd_h = 470
local wnd_x = math.max(30, math.floor((screen_w - wnd_w) * 0.10) + 200)
local wnd_y = math.max(40, math.floor((screen_h - wnd_h) * 0.15))

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

-- Prevent resizing the window smaller than the minimum toolbar and pane bounds (540x260)
if float_wnd_set_resizing_limits then
    float_wnd_set_resizing_limits(editor_wnd, 540, 260, 10000, 10000)
end
