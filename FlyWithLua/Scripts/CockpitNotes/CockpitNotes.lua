-- ============================================================================
-- FlyWithLua Cockpit Markdown Editor & Live Previewer
-- ----------------------------------------------------------------------------
-- Author: Antigravity pair programming with Steve Goldberg (@slgoldberg)
-- Architecture: Roger2 (ImGee / ImgWindow) & Sir Lance (FlyWithLua)
-- Description:
--   An in-sim floating scratchpad, ATIS note-taker, and markdown editor with
--   real-time preview, 1-click aviation templates, and markdown persistence.
-- ============================================================================

if not SUPPORTS_FLOATING_WINDOWS then
    logMsg("FlyWithLua: Floating windows not supported by this installation.")
    return
end

-- Anchor script directory at boot so get_notes_dir() never decays to root Scripts/
local SCRIPT_BASE_DIR = (CURRENT_SCRIPT_DIRECTORY and #CURRENT_SCRIPT_DIRECTORY > 0) and CURRENT_SCRIPT_DIRECTORY or "FlyWithLua/Scripts/CockpitNotes"

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

-- Split View Layout State (table-bundled to respect LuaJIT 60 upvalue limit)
local split = {
    ratio         = 0.50,  -- Draggable divider ratio (clamped 0.15 to 0.85)
    should_scroll = false, -- Auto-scroll flag triggered on Quick Action insertion
    word_wrap     = true,  -- Auto-wrap toggle on editor (defaults to ON)
}

-- Shared Theme Preset State (5 ImGee Themes)
local theme_names = {
    "Charcoal (Default)",
    "Rusty (Aviation)",
    "ImGui Dark",
    "ImGui Classic",
    "ImGui Light"
}
local theme_tips = {
    "**Charcoal (Default)**\nSleek modern slate/graphite flight deck palette.",
    "**Rusty (Aviation)**\nWarm aviation bronze & terracotta palette (ABC classic).",
    "**ImGui Dark**\nStock Dear ImGui dark gray color scheme.",
    "**ImGui Classic**\nLegacy Dear ImGui purplish-blue color scheme.",
    "**ImGui Light**\nHigh-contrast light daytime color scheme."
}
local current_applied_theme_idx = nil

local function apply_theme(idx)
    if idx == 0 then
        if imgui.ApplyDefaultTheme then imgui.ApplyDefaultTheme()
        elseif imgui.StyleColorsCharcoal then imgui.StyleColorsCharcoal() end
    elseif idx == 1 then
        if imgui.StyleColorsRusty then imgui.StyleColorsRusty() end
    elseif idx == 2 then
        if imgui.StyleColorsDark then imgui.StyleColorsDark() end
    elseif idx == 3 then
        if imgui.StyleColorsClassic then imgui.StyleColorsClassic() end
    elseif idx == 4 then
        if imgui.StyleColorsLight then imgui.StyleColorsLight() end
    end
end

-- ----------------------------------------------------------------------------
-- File Persistence & In-Cockpit File Dialogs (Turnkey ImgWindow File Suite)
-- ----------------------------------------------------------------------------
local is_windows = (SYSTEM == "IBM") or (package.config:sub(1,1) == "\\")

local function ensure_directory(dir)
    local ok, lfs = pcall(require, "lfs_ffi")
    if not (ok and lfs) then
        ok, lfs = pcall(require, "lfs")
    end
    if ok and lfs and lfs.mkdir then
        pcall(lfs.mkdir, dir)
    end
end

local function get_notes_dir()
    local sep = is_windows and "\\" or "/"
    local base_dir = SCRIPT_BASE_DIR:gsub("[/\\]+$", "")
    if is_windows then
        base_dir = base_dir:gsub("/", "\\")
    else
        base_dir = base_dir:gsub("\\", "/")
    end
    if base_dir:match("[/\\]notes$") then
        return base_dir
    end
    return base_dir .. sep .. "notes"
end

local function get_legacy_save_path()
    local sep = is_windows and "\\" or "/"
    local base_dir = SCRIPT_BASE_DIR:gsub("[/\\]+$", "")
    if is_windows then
        base_dir = base_dir:gsub("/", "\\")
    else
        base_dir = base_dir:gsub("\\", "/")
    end
    return base_dir .. sep .. "cockpit_notes.md"
end

local current_filename = "cockpit_notes.md"
local current_note_file = current_filename -- Synced alias for backwards compatibility
local show_new_file_modal = false
local new_file_name_input = ""
local focus_new_note_input = false

-- Modal file management dialog states (safe for VR, non-blocking, within viewport)
local show_open_dialog = false
local show_save_dialog = false

local function get_note_path(filename)
    filename = filename or current_filename
    if not filename:lower():match("%.[a-z0-9]+$") then
        filename = filename .. ".md"
    end
    local sep = is_windows and "\\" or "/"
    return get_notes_dir() .. sep .. filename
end

-- Default scratchpad template loaded if no saved notes exist on disk
local initial_doc = [[# Cockpit Notes & Scratchpad

<badge blue>FlyWithLua Phase 3 Suite</badge> <badge>Simulator: X-Plane 12</badge> <badge green>Status: Active</badge>

Welcome to your in-cockpit **FlyWithLua Markdown Editor**!

* **Live Formatting:** Headers, **bold**, *italic*, <u>underline</u>, and ~~strikethrough~~.
* **Interactive GFM Tables:** Markdown tables support drag-resizable column dividers and column sorting.
* **Async Remote Badges:** Syntax like `![Badge](https://img.shields.io/...)` loads smoothly in the background.
* **Procedural Vector Icons:** Zero-font, DPI-scaled procedural vector icons:
  * HTML Tags: `<icon blue plane />` Flight Enroute, `<icon green check />` Verified, `<icon orange warning />` Caution
  * Emoji Shortcodes: `:plane:` Route, `:check:` Verified, `:warning:` Advisory
  * Raw-Fallback Macro: `#[^](ICON_CARET_UP)` Pitch Up, `#[↺](ICON_UNDO)` Reset
  * Rescued Raw Unicode: `↺` Reset, `↻` Restore, `▲` Up, `▼` Down, `✓` Check, `⚠` Warning
* **Inline Sized Typography:** <tiny color=#888888>tiny (10px)</tiny> | <small color=gray>small (12px)</small> | <regular>regular (14px)</regular> | <big>big (16px)</big> | <huge color=orange>huge (16px+ bold)</huge>
* **Avionics Annunciators:** <pill icon=plane color=#3498db>NAV / ALT</pill> <pill icon=check color=#2ecc71>ALT (Mode C)</pill> <pill icon=warning orange>PITOT ON</pill> <pill icon=warning color=#e74c3c />
* **Keycaps & Shortcuts:** Press <kbd>Alt+F4</kbd> or <kbd>Ctrl+Shift+P</kbd>
* **Hyperlinks:** [FlyWithLua Documentation](https://github.com/sparker256/FlyWithLua) or [Cockpit Notes](https://github.com/slgoldberg/FlyWithLua-CockpitNotes)

### Flight Plan Waypoint Sample
| Fix / Point | Altitude | Distance | Fuel Rem | Status |
| :--- | :---: | :---: | :---: | :---: |
| **KORD** (Dep) | Field | 0 NM | 18,800 lbs | <badge icon=plane green>DEPARTED</badge> |
| **JOT** (VOR) | 5,000 | 28 NM | 17,900 lbs | <pill icon=check color=#2ecc71>CLIMB</pill> |
| **DLL** (VOR) | FL340 | 142 NM | 14,800 lbs | <pill icon=plane color=#3498db>CRUISE</pill> |
| **KMSP** (Dest) | Field | 334 NM | 9,400 lbs | <badge icon=plane yellow>PLANNED</badge> |

### Cockpit Checklist
- `[x]` <icon green check /> Pre-flight briefing complete
- `[x]` <icon green check /> Altimeter set to standard: `29.92 inHg`
- `[ ]` :warning: Clearance copied and read back
- `[ ]` #[^](ICON_CARET_UP) Departure runway confirmed

<pill blue>TIP</pill> Use the **Mode** buttons above to switch between pure Editor, Split View, and full-screen Preview.
]]

-- Rich Markdown content for the "About" dialog
local about_markdown_text = [[# FlyWithLua Cockpit Notes
<badge green>v1.0.2 Beta</badge> <pill blue>Phase 3 Suite</pill>

An interactive in-cockpit Markdown scratchpad and flight notes system for X-Plane 12 flight operations.

---

### Key Capabilities
* **Interactive GFM Tables:** Standard GitHub Flavored Markdown tables (`| Col 1 | Col 2 |`) with drag-resizable column dividers, right-border table width adjustment, and column sorting.
* **Async Remote Images & Badges:** Non-blocking background worker threads for `![Badge](https://img.shields.io/...)` badges and online diagrams without sim frame drops.
* **Procedural Vector Icons & Annunciators:** Native zero-font vector icons (`<icon blue plane />`, `:check:`, `#[^](ICON_CARET_UP)`, `↺`, `▲`) and live annunciator capsules (`<pill icon=plane>...</pill>`).
* **Zero-Scroll-Leak Pulldowns:** Non-capturing `imgui.SelectPulldown` lets you browse templates without stealing mouse-wheel scroll from your notes.
* **Directional Blossoming Popups:** Quick action menus blossom upward (`PopupAnchor.AboveItem`) from the bottom status bar without spilling off the screen.
* **Live Markdown Typography:** Headers (`#`, `##`), **bold**, *italic*, <u>underline</u>, ~~strikethrough~~, bulleted lists, and formatted code blocks with syntax styling.
* **Dual-Bracket Hyperlinks:** Standard CommonMark `[Label](url)` and `![Alt](img)` links supported side-by-side with legacy `{...}` brackets.
* **In-Cockpit File Management:** Turnkey C++11 native directory combo (`imgui.FilePickerCombo`) and in-viewport modal `[Open...]` and `[Save As...]` dialogs.
* **Cockpit Checklists & Tasks:** Prepend `- [ ]` or click `[ ] Task` to format verify-and-execute flight crew checklists with inline monospace boxes.
* **Tactile Canvas Zoom:** Dynamic font scaling from **50% to 200%** with 1-point quantum steppers, mouse-wheel adjust, and 1-click A/B toggle back to 100% baseline.
* **Dedicated Multi-Note Storage:** Automatic file persistence in `FlyWithLua/Scripts/CockpitNotes/notes/` with active note file selection and instant new note creation.
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
* **Repository:** [FlyWithLua-CockpitNotes on GitHub](https://github.com/slgoldberg/FlyWithLua-CockpitNotes)
]]

-- Load saved notes on startup if available, otherwise fall back to template
local function load_initial_document()
    local path = get_note_path("cockpit_notes.md")
    local f = io.open(path, "r")
    if f then
        local content = f:read("*a")
        f:close()
        if content and #content > 0 then
            current_filename = "cockpit_notes.md"
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
            -- Auto-migrate to notes directory
            ensure_directory(get_notes_dir())
            local fw = io.open(path, "w")
            if fw then
                fw:write(content)
                fw:close()
            end
            current_filename = "cockpit_notes.md"
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
    ["ATIS Copy Pad"] = [[# ATIS Information [ALPHA] <icon blue plane />
* **Airport:** `KORD (Chicago O'Hare)` | **Time:** `1851Z`
* **Wind:** `280° @ 12kt G18kt` | **Visibility:** `10 SM`
* **Sky Condition:** `FEW 045, SCT 250`
* **Temp / Dew Point:** `18°C / 08°C`
* **Altimeter:** `30.12 inHg`
* **Landing / Departing:** `Runway 28C, 28R`
* **NOTAMs:** <icon orange warning /> Taxiway Bravo closed between B4 and B6
]],

    ["IFR Clearance (CRAFT)"] = [[# IFR Clearance Briefing
* **C - Clearance Limit:** `Cleared to KMSP via JOT2 departure`
* **R - Route:** `JOT VORD V100 BAE DLL ALTRN`
* **A - Initial Altitude:** `Maintain 5,000 ft, expect FL330 in 10 minutes` #[▲](ICON_CARET_UP)
* **F - Departure Frequency:** `120.40 MHz`
* **T - Transponder (Squawk):** <kbd>4217</kbd>

---
- `[ ]` :check: Readback verified and accepted by ATC
- `[ ]` :check: Squawk set in Transponder
- `[ ]` :check: Initial altitude armed on MCP
]],

    ["Pre-Flight Checklist"] = [[# Cockpit Pre-Flight Checklist
- `[ ]` :check: Weather & NOTAM briefing reviewed
- `[ ]` :check: Fuel onboard verified: `14,200 lbs`
- `[ ]` :check: Weight & Balance computed (CG within envelope)
- `[ ]` :check: FMS flight plan & waypoints cross-checked
- `[ ]` :check: Altimeters cross-checked & set: `29.92`
- `[ ]` :check: V-Speeds calculated: `V1: 138`, `VR: 144`, `V2: 151`
- `[ ]` Transponder: <pill icon=check green>ALT / 1200</pill>
- `[ ]` Thrust reversers checked: <badge icon=check green>ARMED</badge>
- `[ ]` Pitot Heat: <pill icon=warning orange>CHECKED</pill>
]],

    ["IFR Navigation Log (NavLog)"] = [[# IFR Navigation Log & Route Plan

![Flight](https://img.shields.io/badge/Flight-KORD_%E2%9C%88_KMSP-blue.png) ![Cruise](https://img.shields.io/badge/Cruise-FL340_M0.78-informational.png) ![Rules](https://img.shields.io/badge/Rules-IFR-green.png) ![Dispatch](https://img.shields.io/badge/Dispatch-SimBrief-orange.png)

* **Route:** `KORD DCT JOT V100 BAE DLL ALTRN KMSP`
* **Cruise Profile:** `FL340 / Mach 0.78` | **Groundspeed:** `~440 kt`
* **Distance:** `334 NM` | **Est Flight Time:** `00:54`

---

## Flight Deck Status & Automation
* **Autopilot Mode:** <pill icon=plane color=#3498db>NAV / ALT</pill>
* **Transponder Mode:** <pill icon=check color=#2ecc71>ALT (Mode C)</pill>
* **Pitot & Anti-Ice:** <pill icon=warning orange>ON</pill>
* **Master Warning:** <pill icon=warning color=#e74c3c />
* **TCAS Surveillance:** <badge icon=plane>TA/RA</badge>

---

## Communication & Nav Frequencies
| Station / Service | Facility | Frequency | Status |
| :--- | :--- | :---: | :---: |
| **ORD ATIS** | Arrival / Departure | 135.400 | <badge icon=check green>ACTIVE</badge> |
| **ORD Clearance** | Pre-Taxi Clearance | 121.150 | Monitored |
| **ORD Ground** | Metering / North Ground | 124.125 | Standby |
| **ORD Tower** | Local Control (28R) | 120.750 | Standby |
| **Chicago Center** | Enroute Radar Control | 133.500 | Standby |
| **MSP Approach** | Minneapolis Terminal | 119.300 | Planned |
| **MSP ILS 30R** | Localizer & Glidepath | 110.300 | Planned |

---

## Waypoint Navigation Log
| Fix / Waypoint | Airway | Course | Leg (NM) | Rem (NM) | Altitude | Est Fuel (lbs) | Status |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **KORD** | — | — | 0 | 334 | Field | 18,800 | <badge icon=plane green>DEP</badge> |
| **JOT** (Joliet VOR) | DCT | 215° | 28 | 306 | 5,000 | 17,900 | <pill icon=check color=#2ecc71>CLIMB</pill> |
| **BAE** (Badger VOR) | V100 | 322° | 64 | 242 | FL240 | 16,100 | <pill icon=check color=#2ecc71>CLIMB</pill> |
| **DLL** (Dells VOR) | V100 | 305° | 78 | 164 | FL340 | 14,800 | <pill icon=plane color=#3498db>CRUISE</pill> |
| **ALTRN** (Waypoint) | Direct | 296° | 92 | 72 | FL340 | 12,600 | <pill icon=plane color=#3498db>CRUISE</pill> |
| **KMSP** (Destination)| Direct | 298° | 72 | 0 | Field | 9,400 | <badge icon=plane yellow>ARR</badge> |

---

## In-Flight Verification Checklist
- `[ ]` :check: Altimeter reset to standard (29.92 / 1013 hPa) passing 18,000 ft (FL180)
- `[ ]` :plane: Top of Climb fuel check verified against flight plan burn
- `[ ]` :warning: Minneapolis ATIS copied (Wind, Runway in use, Altimeter)
- `[ ]` Top of Descent calculated: 3x altitude loss rule + 10 NM margin
- `[ ]` Approach briefed: ILS 30R DA `1,041 ft MSL`, missed approach altitude `4,000 ft`
]],

    ["Fuel & Weight Planning"] = [[# Fuel & Weight Planning

![SimBrief](https://img.shields.io/badge/Dispatch-SimBrief_Validated-green.png) ![Fuel State](https://img.shields.io/badge/Ramp_Fuel-18.8k_lbs-blue.png) ![Type](https://img.shields.io/badge/Type-B737--800-informational.png)

* **Aircraft:** `B737-800 / A320` | **Route:** `KORD -> KMSP`
* **Cruise:** `FL340 / M0.78` | **Cost Index:** `28`
* **Dispatch Planning:** [SimBrief Flight Plan](https://www.simbrief.com)

---

## Weight & Balance Limits
| Parameter | Planned (lbs) | Max Limit (lbs) | Margin | Status |
| :--- | :---: | :---: | :---: | :---: |
| **OEW** (Operating Empty) | 91,300 | — | — | <badge icon=check green>OK</badge> |
| **Payload** (Pax + Cargo) | 35,400 | 43,000 | +7,600 | <badge icon=check green>OK</badge> |
| **ZFW** (Zero Fuel Weight) | 126,700 | 138,300 | +11,600 | <badge icon=check green>+11,600 lbs</badge> |
| **TOW** (Takeoff Weight) | 145,500 | 174,200 | +28,700 | <badge icon=check green>+28,700 lbs</badge> |
| **ELW** (Landing Weight) | 136,100 | 146,300 | +10,200 | <badge icon=check green>+10,200 lbs</badge> |

* **Center of Gravity (CG):** `21.4% MAC` | **Takeoff Trim:** <pill icon=check color=#2ecc71>5.2 ANU</pill>

---

## Fuel Planning & Reserves (FAR / ICAO)
| Fuel Stage | Amount (lbs) | Flow Time | Profile Notes |
| :--- | :---: | :---: | :--- |
| **Taxi Out** | 800 | 00:15 | APU + single-engine taxi |
| **Trip Fuel (KORD-KMSP)** | 9,400 | 01:08 | Cruise FL340 enroute burn |
| **Contingency (5%)** | 470 | 00:05 | Enroute wind / track variance |
| **Alternate (KSBN)** | 3,100 | 00:25 | Climb to FL220, descent & missed |
| **Final Reserve** | 2,450 | 00:30 | ICAO legal 30 min @ 1,500 ft |
| **Extra / Discretionary** | 2,580 | 00:20 | Terminal vectoring & winter ops |

---

### Fuel Thresholds & Bingo State
| Threshold | Quantity | Indicator | Required Action |
| :--- | :---: | :---: | :--- |
| **Ramp / Block Fuel** | 18,800 lbs | <pill icon=check color=#2ecc71>NORMAL</pill> | Totalizer verified against uplift receipt |
| **Min Takeoff Fuel** | 18,000 lbs | <pill icon=check color=#2ecc71>NORMAL</pill> | Abort takeoff if below threshold |
| **Diversion Bingo** | 5,550 lbs | <badge icon=warning yellow>BINGO</badge> | Divert immediately to KSBN alternate |
| **Emergency Fuel** | 2,450 lbs | <badge icon=warning red>MAYDAY</badge> | Declare "Mayday Fuel" with ATC |

---

### Pre-Departure Verification Checklist
- `[ ]` :check: Totalizer gauge verified against fuel uplift receipt (`18,800 lbs`)
- `[ ]` :warning: Cross-feed valve tested and set closed
- `[ ]` FMC PERF INIT entered: ZFW `126.7`, Reserves `5.5`, Cost Index `28`
- `[ ]` Takeoff stabilizer trim `5.2 ANU` dialed & confirmed in green band
- `[ ]` Takeoff V-Speeds calculated for runway length & condition
]],

    ["Diversion Briefing"] = [[# In-Flight Diversion Briefing

![Diversion Plan](https://img.shields.io/badge/Alternate-KSBN_South_Bend-yellow.png) ![Approach](https://img.shields.io/badge/Approach-ILS_27L-blue.png) ![Fuel](https://img.shields.io/badge/Bingo-5.5k_lbs-orange.png)

* **Primary Destination:** `KMSP (Minneapolis-St. Paul)`
* **Designated Alternate:** `KSBN (South Bend Regional)`
* **Diversion Reason:** `Below Minima / Weather / System Advisory`
* **Airport Information:** [AirNav KSBN Overview](https://www.airnav.com/airport/KSBN)

---

## Alternate Airport & Runway Assessment
| Runway | Dimensions | Surface | Approach NavAid | Wind Component |
| :--- | :---: | :---: | :--- | :---: |
| **09R / 27L** | 8,414 x 150 ft | Concrete | ILS 27L (109.90 MHz / 274°) | 11 kt HW, 9 kt XW |
| **09L / 27R** | 4,300 x 75 ft | Asphalt | Visual / GPS | Not Authorized |
| **18 / 36** | 7,100 x 150 ft | Asphalt | VOR / GPS 36 | 13 kt XW |

* **Airport Elevation:** `799 ft MSL` | **Bearing & Distance:** `085° Mag / 64 NM`
* **Estimated Time Enroute (ETE):** `14 minutes` (High-speed descent profile)
* **Approach Minima / DA:** <pill icon=plane color=#3498db>999 ft MSL (200 ft AGL) / RVR 2400</pill>

---

## Weather Minima & Runway Conditions
* **Current METAR:** `KSBN 281854Z 31014G22KT 6SM -RA SCT018 OVC025 08/04 A2994`
* **Surface Wind:** `310° @ 14 kt, gusts 22 kt`
* **Braking Action:** <icon green check /> GOOD (Runway wet, no standing water reported)

---

## Fuel Reserves & Bingo Status
| State / Threshold | Fuel (lbs) | Endurance | Status |
| :--- | :---: | :---: | :---: |
| **FOB at Decision** | 7,600 | 01:35 | <pill icon=check color=#2ecc71>ONBOARD</pill> |
| **Burn to Alternate** | 2,100 | 00:25 | Planned Burn |
| **Estimated Touchdown FOB** | 5,500 | 01:10 | <badge icon=check green>SAFE</badge> |
| **Final Legal Reserve** | 2,450 | 00:30 | Required |
| **Usable Holding Margin** | +3,050 | +00:38 | <badge icon=check green>+38 min hold</badge> |
| **Minimum Diversion Bingo** | 5,550 | — | <badge icon=warning yellow>BINGO</badge> |
| **Emergency Fuel Threshold** | 2,450 | — | <badge icon=warning red>MAYDAY</badge> |

---

## ATC & Cockpit Diversion Checklist
- `[ ]` :plane: **Aviate:** Maintain positive aircraft control; turn toward alternate heading (`085°`)
- `[ ]` :warning: **Declare Intentions:** Advise ATC ("Request diversion to alternate KSBN")
- `[ ]` **Squawk & Fuel State:** State souls on board and fuel endurance to ATC if requested
- `[ ]` **FMS Setup:** Select KSBN as active destination, insert ILS 27L approach
- `[ ]` **Radios & Nav Aids:** Set NAV1 `109.90`, course `274°`, monitor ATIS `120.675`
- `[ ]` **Landing Performance:** Recalculate landing distance, verify VREF and VAPP
- `[ ]` **Cabin Crew Notification:** Brief flight attendants (NITS briefing: Nature, Intention, Time, Special)
- `[ ]` **Passenger PA:** Deliver calm, informative diversion announcement
- `[ ]` :check: **Approach & Landing Checklist:** Completed before initial approach fix
]],

    ["METAR / TAF Scratchpad"] = [[# METAR & TAF Weather Scratchpad

![NOAA Weather](https://img.shields.io/badge/Source-NOAA_AWC-blue.png) ![ORD Category](https://img.shields.io/badge/ORD-VFR-green.png) ![MSP Category](https://img.shields.io/badge/MSP-MVFR-yellow.png)

* **Flight:** `KORD -> KMSP` | **Briefing Time:** `1850Z`
* **Weather Source:** [NOAA Aviation Weather Center](https://aviationweather.gov)

---

## Route Weather Stations Comparison
| Station | Category | Wind | Visibility | Ceiling (AGL) | Altimeter |
| :--- | :---: | :---: | :---: | :---: | :---: |
| **KORD** (Departure) | <badge green>VFR</badge> | 280° @ 14G21kt | 10 SM | BKN 4,500 ft | 30.12 inHg |
| **KMSP** (Destination) | <badge yellow>MVFR</badge> | 330° @ 18G26kt | 4 SM (-SN) | BKN 1,500 ft | 29.88 inHg |
| **KSBN** (Alternate) | <badge green>VFR</badge> | 310° @ 14G22kt | 6 SM (-RA) | OVC 2,500 ft | 29.94 inHg |

---

## Departure Weather (KORD)
```text
KORD 281851Z 28014G21KT 10SM BKN045 OVC090 12/03 A3012 RMK AO2 SLP201
```
* **Station & Time:** `KORD (Chicago O'Hare)` @ `18:51 UTC`
* **Wind:** `280° at 14 kt, gusts to 21 kt`
* **Visibility:** `10 Statute Miles (Unrestricted)`
* **Clouds / Ceiling:** `Broken 4,500 ft AGL, Overcast 9,000 ft AGL`
* **Temperature / Dew Point:** `12°C / 03°C` (Spread: `9°C`)
* **Altimeter (QNH):** <pill green>30.12 inHg (1020 hPa)</pill>
* **Active Runways:** `28C, 28R` (Departures)

---

## Destination Weather (KMSP)
```text
KMSP 281853Z 33018G26KT 4SM -SN BLSN BKN015 OVC030 M02/M06 A2988 RMK AO2
```
* **Station & Time:** `KMSP (Minneapolis-St. Paul)` @ `18:53 UTC`
* **Wind:** `330° at 18 kt, gusts to 26 kt`
* **Visibility:** `4 Statute Miles in Light Snow & Blowing Snow (-SN BLSN)`
* **Ceiling & Sky:** `Broken 1,500 ft AGL (Ceiling), Overcast 3,000 ft AGL`
* **Temperature / Dew Point:** `-02°C / -06°C`
* **Altimeter (QNH):** <pill blue>29.88 inHg (1012 hPa)</pill>
* **Surface Condition:** <icon orange warning /> Contaminated runway; braking action Medium

---

## Destination Forecast (TAF Copy Pad)
```text
TAF KMSP 281720Z 2818/2924 34016G24KT 4SM -SN BKN018 OVC035
  TEMPO 2819/2823 2SM -SN BKN010
  FM290200 32012KT 6SM NSW BKN025
  FM291200 29010KT P6SM SKC
```
* **Arrival Trend (19:00Z - 21:00Z):** `TEMPO 2SM in light snow, ceiling 1,000 ft AGL`
* **Runway In Use:** Expect `Runway 30L / 30R` (ILS Cat I / Cat II available)
* **Crosswind Calculation:** `340° vs 300° runway heading = 40° angle @ 16G24kt (~15kt X-Wind)`

---

## Weather Verification Checklist
- `[ ]` :check: Destination altimeter copied: `29.88 inHg`
- `[ ]` :warning: Engine & wing anti-ice requirements briefed for descent into freezing clouds
- `[ ]` :check: Approach category and minima briefed for ILS 30L/30R
- `[ ]` Cold temperature altitude corrections applied if airport temp < 0°C
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

local template_names = {
    "Aviation Templates...",
    "ATIS Copy Pad",
    "IFR Clearance (CRAFT)",
    "Pre-Flight Checklist",
    "IFR Navigation Log (NavLog)",
    "Fuel & Weight Planning",
    "Diversion Briefing",
    "METAR / TAF Scratchpad",
    "FlyWithLua Scratchpad",
}

local template_tips = {
    "Choose a pre-formatted flight template to insert into your note",
    "Standard ATIS copy pad with airport, weather, runways and NOTAMs",
    "Full IFR clearance CRAFT briefing and verification checklist",
    "Pre-flight cockpit preparation and avionics setup checklist",
    "Full IFR navigation log with frequencies, waypoints, and leg fuel",
    "Fuel calculations, reserves, weight & balance, and bingo thresholds",
    "In-flight diversion briefing, alternate runway assessment, and fuel",
    "METAR & TAF weather briefing and station comparison table",
    "Quick Lua scratchpad for inspecting Datarefs and avionics state",
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

local function load_note_from_full_path(chosen_path)
    if not chosen_path or #chosen_path == 0 then return end
    local f, err = io.open(chosen_path, "r")
    if f then
        editor_text = f:read("*a")
        f:close()
        local fname = chosen_path:match("([^/\\]+)$") or chosen_path
        current_filename = fname
        current_note_file = fname
        editor_epoch = editor_epoch + 1
        just_formatted = true
        save_status_msg = "Loaded: " .. fname
        save_status_path = chosen_path
        save_status_time = os.time()
    else
        save_status_msg = "File not found"
        save_status_path = tostring(err)
        save_status_time = os.time()
    end
end

local function save_note_to_full_path(save_path)
    if not save_path or #save_path == 0 then return end
    if not save_path:lower():match("%.[a-z0-9]+$") then
        save_path = save_path .. ".md"
    end
    local f, err = io.open(save_path, "w")
    if f then
        f:write(editor_text)
        f:close()
        local fname = save_path:match("([^/\\]+)$") or save_path
        current_filename = fname
        current_note_file = fname
        save_status_msg = "Saved: " .. fname
        save_status_path = save_path
        save_status_time = os.time()
    else
        save_status_msg = "Error Saving Notes"
        save_status_path = tostring(err)
        save_status_time = os.time()
    end
end

local function load_note(filename)
    filename = filename or current_filename
    if filename:find("[/\\]") then
        return load_note_from_full_path(filename)
    end
    local path = get_note_path(filename)
    local f, err = io.open(path, "r")
    if not f and (filename == "cockpit_notes.md" or filename == "cockpit_notes") then
        local leg_path = get_legacy_save_path()
        f = io.open(leg_path, "r")
    end
    if f then
        editor_text = f:read("*a")
        f:close()
        current_filename = filename
        current_note_file = filename
        editor_epoch = editor_epoch + 1
        just_formatted = true
        save_status_msg = "Loaded: " .. filename
        save_status_path = path
        save_status_time = os.time()
    else
        save_status_msg = "File not found: " .. filename
        save_status_path = path
        save_status_time = os.time()
    end
end

local function save_note(filename)
    filename = filename or current_filename
    if filename:find("[/\\]") then
        return save_note_to_full_path(filename)
    end
    if not filename:lower():match("%.[a-z0-9]+$") then
        filename = filename .. ".md"
    end
    current_filename = filename
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
    else
        save_status_msg = "Error Saving Notes"
        save_status_path = tostring(err)
        save_status_time = os.time()
    end
end

-- Backward compatibility aliases
local load_document = load_note
local save_document = save_note

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
-- Bottom Status Bar with Directional Blossoming Quick Actions Popup
-- ----------------------------------------------------------------------------
local function render_bottom_status_bar(avail_w)
    imgui.Separator()

    if imgui.AlignTextToFramePadding then
        imgui.AlignTextToFramePadding()
    end

    local text = editor_text or ""
    local num_chars = #text
    local num_lines = 1
    for _ in string.gmatch(text, "\n") do
        num_lines = num_lines + 1
    end

    local zoom_pct = math.floor(content_zoom * 100.0 + 0.5)
    local status_str = string.format("Note: %s  |  %d lines  |  %d chars  |  Zoom: %d%%",
        current_filename, num_lines, num_chars, zoom_pct)

    imgui.TextDisabled(status_str)

    -- Right-aligned Quick Actions button
    local btn_label = "Quick Actions <icon caret-up />##btnQuickActions"
    local btn_w = 145
    local win_w = (imgui.GetWindowWidth and imgui.GetWindowWidth()) or avail_w
    local right_x = win_w - btn_w - 20
    local cur_x = (imgui.GetCursorPosX and imgui.GetCursorPosX()) or 0
    if right_x > cur_x then
        imgui.SameLine(right_x)
    else
        imgui.SameLine()
    end

    local btn_clicked = false
    if imgui.MD and imgui.MD.ButtonMD then
        btn_clicked = imgui.MD.ButtonMD(btn_label)
    elseif imgui.ButtonMD then
        btn_clicked = imgui.ButtonMD(btn_label)
    elseif imgui.Button then
        btn_clicked = imgui.Button("Quick Actions ^##btnQuickActions", btn_w, 20)
    end

    if btn_clicked then
        if imgui.OpenPopupAtItem then
            imgui.OpenPopupAtItem("##QuickActionsPopup",
                (imgui.PopupAnchor and imgui.PopupAnchor.AboveItem) or 1,
                (imgui.PopupAlign and imgui.PopupAlign.Right) or 1)
        else
            imgui.OpenPopup("##QuickActionsPopup")
        end
    end
    show_tooltip("**Quick Actions & Flight Deck Snippets**\n\nOpens upward from the status bar to quickly insert:\n* Aviation scratchpads & checklists\n* Annunciator status capsules\n* Procedural vector icons (`<icon ...>`, `:plane:`, `#[^](ICON_CARET_UP)`)\n* Markdown tables and checklists")

    -- Blossoming anchored popup (AboveItem, Right aligned)
    local is_open = false
    if imgui.BeginAnchoredPopup then
        is_open = imgui.BeginAnchoredPopup("##QuickActionsPopup")
    elseif imgui.BeginPopup then
        is_open = imgui.BeginPopup("##QuickActionsPopup")
    end

    if is_open then
        imgui.TextDisabled("FLIGHT DECK QUICK ACTIONS")
        imgui.Separator()
        if imgui.MenuItem("Insert ATIS Copy Pad") then
            editor_text = templates["ATIS Copy Pad"]
            editor_epoch = editor_epoch + 1
            just_formatted = true
            split.should_scroll = true
        end
        if imgui.MenuItem("Insert CRAFT Clearance") then
            editor_text = templates["IFR Clearance (CRAFT)"]
            editor_epoch = editor_epoch + 1
            just_formatted = true
            split.should_scroll = true
        end
        if imgui.MenuItem("Insert NavLog Template") then
            editor_text = templates["IFR Navigation Log (NavLog)"]
            editor_epoch = editor_epoch + 1
            just_formatted = true
            split.should_scroll = true
        end
        imgui.Separator()
        if imgui.MenuItem("Insert NavLog Row") then
            wrap_or_insert("| **FIX** | Direct | 000° | 0 | 0 | FL000 | 0,000 | <pill icon=plane color=#3498db>CRUISE</pill> |\n", "", "")
            split.should_scroll = true
        end
        if imgui.MenuItem("Insert Annunciator Pill") then
            wrap_or_insert("<pill icon=plane color=#3498db>NAV / ALT</pill>", "", "")
            split.should_scroll = true
        end
        if imgui.MenuItem("Insert Master Warning Light") then
            wrap_or_insert("<pill icon=warning color=#e74c3c />", "", "")
            split.should_scroll = true
        end
        if imgui.MenuItem("Insert Vector Icon (:plane:)") then
            wrap_or_insert("<icon blue plane /> ", "", "")
            split.should_scroll = true
        end
        if imgui.MenuItem("Insert Status Check (:check:)") then
            wrap_or_insert("<icon green check /> ", "", "")
            split.should_scroll = true
        end
        if imgui.MenuItem("Insert Caution Warning (:warning:)") then
            wrap_or_insert("<icon orange warning /> ", "", "")
            split.should_scroll = true
        end
        if imgui.MenuItem("Insert Checklist Task") then
            wrap_or_insert("- `[ ]` ", "", "Task description", true)
            split.should_scroll = true
        end
        if imgui.MenuItem("Insert Interactive Table") then
            wrap_or_insert("\n| Parameter | Value | Status |\n| :--- | :---: | :---: |\n| Engine 1 | Normal | <icon green check /> <badge green>OK</badge> |\n| Pitot Heat | Off | <icon orange warning /> <badge red>OFF</badge> |\n", "", "")
            split.should_scroll = true
        end
        imgui.Separator()
        if imgui.MenuItem("Clear Note Scratchpad") then
            editor_text = ""
            editor_epoch = editor_epoch + 1
            just_formatted = true
        end
        if imgui.MenuItem("Close Menu") then
            imgui.CloseCurrentPopup()
        end
        imgui.EndPopup()
    end
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

    imgui.SameLine(0, 6)
    local active_theme_idx = _G.FWL_ACTIVE_THEME_IDX or 0
    imgui.TextUnformatted("Theme:")
    imgui.SameLine(0, 4)
    if imgui.SelectPulldown then
        local theme_changed, new_theme = imgui.SelectPulldown("##ThemePicker", active_theme_idx, theme_names, theme_tips, 145.0, nil, nil, true, true)
        if not theme_changed and imgui.IsItemHovered and imgui.IsItemHovered() then
            local wheel = (imgui.GetMouseWheel and imgui.GetMouseWheel(true)) or 0
            if wheel ~= 0 then
                local dir = (wheel < 0) and 1 or -1
                new_theme = (active_theme_idx + dir) % 5
                if new_theme < 0 then new_theme = new_theme + 5 end
                theme_changed = true
            end
        end
        if theme_changed and new_theme ~= active_theme_idx then
            active_theme_idx = new_theme
            _G.FWL_ACTIVE_THEME_IDX = active_theme_idx
            apply_theme(active_theme_idx)
            current_applied_theme_idx = active_theme_idx
        end
    else
        local cur_name = theme_names[active_theme_idx + 1] or "Charcoal (Default)"
        if imgui.Button(cur_name .. "##btn_notes_theme") then
            active_theme_idx = (active_theme_idx + 1) % 5
            _G.FWL_ACTIVE_THEME_IDX = active_theme_idx
            apply_theme(active_theme_idx)
            current_applied_theme_idx = active_theme_idx
        end
    end
    if show_tooltip then
        show_tooltip(
            "**ImGee Theme Presets** (Click dropdown or roll mouse wheel):\n" ..
            "• **Charcoal**: Sleek modern slate/graphite flight deck palette (default).\n" ..
            "• **Rusty**: Warm aviation bronze & terracotta palette (ABC classic).\n" ..
            "• **ImGui Dark / Classic / Light**: Stock Dear ImGui fallback schemes.\n\n" ..
            "*(Synchronized across active FlyWithLua windows)*"
        )
    end

    local cur_top_x = (imgui.GetCursorPosX and imgui.GetCursorPosX()) or 260
    local zoom_group_w = 210
    local zoom_start = avail_w - zoom_group_w

    -- Status notification aligned between About and Zoom
    if os.time() - save_status_time < 3 and #save_status_msg > 0 then
        local msg_w = 200
        if imgui.CalcTextSize then
            local tw = imgui.CalcTextSize(save_status_msg)
            msg_w = tw + 24
        end
        local badge_x = zoom_start - msg_w - 12
        if badge_x > cur_top_x + 10 then
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
    end

    if zoom_start > cur_top_x + 10 then
        imgui.SameLine(zoom_start)
    else
        imgui.SameLine()
    end

    imgui.TextUnformatted("Zoom:")
    imgui.SameLine(0, 6)

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
    if imgui.SetNextItemWidth then imgui.SetNextItemWidth(75) end
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
        show_tooltip(string.format("**Content Canvas Zoom:** `%d%%` (`%d px`)\n\n* **Steppers & Wheel:** Step by 1 exact font pixel\n* **100%% Baseline:** 16 px\n* **Reset (↺ 100%%):** Toggle 100%% and prior zoom", zoom_pct, math.floor(content_zoom * 16.0 + 0.5)))
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
        show_tooltip("**Reset Zoom (↺ 100%)**\n\nReset canvas font scale back to 100% baseline.")
    elseif last_zoom_pct then
        show_tooltip(string.format("**Restore Zoom (↻ %d%%)**\n\n1-click A/B toggle back to prior %d%% zoom.", last_zoom_pct, last_zoom_pct))
    else
        show_tooltip("**Zoom Baseline (100%)**\n\nCanvas font scale is at 100% baseline.")
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

    -- Persistence & Note File Selector (Right)
    local right_start = avail_w - 475
    local cur_x = (imgui.GetCursorPosX and imgui.GetCursorPosX()) or 0
    if right_start > cur_x + 10 then
        imgui.SameLine(right_start)
    else
        imgui.SameLine()
    end

    imgui.TextUnformatted("Note:")
    imgui.SameLine(0, 4)
    if imgui.SetNextItemWidth then
        imgui.SetNextItemWidth(200)
    end
    local notes_dir = get_notes_dir()
    if imgui.FilePickerCombo then
        local changed, chosen_file = imgui.FilePickerCombo("##notes_select", current_filename, notes_dir, ".md;.txt")
        if changed then
            current_filename = chosen_file
            current_note_file = chosen_file
            load_note(current_filename)
        end
    end
    show_tooltip("**Active Note File**\n\n* Select any `.md` or `.txt` file in `notes/`\n* Microsecond C++ native directory scanning")

    imgui.SameLine(0, 4)
    if toolbar_button("Save", string.format("**Save Note**\n\nPersist buffer to `%s` on disk.", current_filename)) then
        save_note(current_filename)
    end

    imgui.SameLine(0, 4)
    if toolbar_button("Open...", "**Open Note Dialog**\n\nBrowse and load any `.md` or `.txt` file using the in-cockpit modal dialog.") then
        show_open_dialog = true
    end

    imgui.SameLine(0, 4)
    if toolbar_button("Save As...", "**Save Note As Dialog**\n\nSave current note under a new filename.") then
        show_save_dialog = true
    end

    imgui.SameLine(0, 4)
    if toolbar_button("+ New", "**Create New Note**\n\nCreate a new named Markdown file in `notes/`.") then
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
                current_filename = file_name
                current_note_file = file_name
                editor_text = "# " .. raw_name .. "\n\n"
                editor_epoch = editor_epoch + 1
                just_formatted = true
                save_note(file_name)
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
        if toolbar_button("<badge>", "**Avionics Badge**\n\nWrap selection in `<badge icon=avionics_beacon green>...</badge>` for instrument annunciators.") then wrap_or_insert("<badge icon=avionics_beacon green>", "</badge>", "NORMAL") end
        imgui.SameLine()
        if toolbar_button("<pill>", "**Avionics Pill**\n\nWrap selection in `<pill icon=avionics_beacon color=#3498db>...</pill>` for system mode callouts.") then wrap_or_insert("<pill icon=avionics_beacon color=#3498db>", "</pill>", "ACTIVE") end
        imgui.SameLine()
        if toolbar_button("<kbd>", "**Keyboard Keycap**\n\nWrap selection in physical keycap `<kbd>Key</kbd>`.") then wrap_or_insert("<kbd>", "</kbd>", "Key") end
        imgui.SameLine()
        if toolbar_button("Code", "**Lua Code Block**\n\nWrap selection in fenced ```lua block with live preview.") then wrap_or_insert("```lua\n", "\n```", "-- code block\n") end
        imgui.SameLine()
        if toolbar_button("Link", "**Markdown Link**\n\nWrap selection in CommonMark `[Label](url)` hyperlink.") then wrap_or_insert("[", "](https://)", "Link text") end
        imgui.SameLine()
        if toolbar_button("Table", "**GFM Markdown Table**\n\nInsert interactive GFM table with drag-resizable columns and column sorting.") then
            wrap_or_insert("\n| Parameter | Value | Status |\n| :--- | :---: | :---: |\n| Item 1 | 100 | <badge icon=avionics_beacon green>OK</badge> |\n| Item 2 | 200 | <pill icon=avionics_beacon color=#3498db>ACTIVE</pill> |\n", "", "")
        end
        imgui.SameLine()
        if toolbar_button("Badge", "**Async Remote Badge / Image**\n\nInsert async non-blocking shields.io or web image badge.") then
            wrap_or_insert("![Badge](https://img.shields.io/badge/", "-blue.png)", "Title-Value")
        end

        -- Templates Dropdown (Zero-Scroll-Leak SelectPulldown)
        imgui.SameLine()
        imgui.TextUnformatted("|")
        imgui.SameLine()
        if imgui.SelectPulldown then
            local changed, new_idx = imgui.SelectPulldown("##templates_pulldown", 0, template_names, template_tips, 195)
            if changed and new_idx and new_idx > 0 then
                local chosen_name = template_names[new_idx + 1]
                if chosen_name and templates[chosen_name] then
                    editor_text = templates[chosen_name]
                    editor_epoch = editor_epoch + 1
                    just_formatted = true
                end
            end
        else
            if imgui.SetNextItemWidth then
                imgui.SetNextItemWidth(175)
            end
            if imgui.BeginCombo("##templates", "Aviation Templates...", 0) then
                local ok, err = pcall(function()
                    for i = 2, #template_names do
                        local name = template_names[i]
                        if imgui.Selectable(name, false) then
                            editor_text = templates[name]
                            editor_epoch = editor_epoch + 1
                            just_formatted = true
                        end
                    end
                end)
                imgui.EndCombo()
                if not ok then
                    print("[FlyWithLua CockpitNotes] Error in Templates Combo: " .. tostring(err))
                end
            end
        end
        show_tooltip("**Aviation Templates**\n\nLoad pre-formatted cockpit checklists and scratchpads:\n* **ATIS Copy Pad**\n* **IFR Clearance (CRAFT)**\n* **Pre-Flight Checklist**\n* **IFR Navigation Log (NavLog)**\n* **Fuel & Weight Planning**\n* **Diversion Briefing**\n* **METAR / TAF Scratchpad**\n* **FlyWithLua Scratchpad**")

        imgui.Separator()
    end

    -- ------------------------------------------------------------------------
    -- CONTENT PANES: Write, Split, or Preview
    -- ------------------------------------------------------------------------
    if view_mode == "Write" then
        -- 1. Fullscreen Editor
        if imgui.BeginChild("FullEditorChild", 0, -28, 1) then
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

            -- Expand input height to match line content so FullEditorChild possesses the scroll range
            local line_h = (imgui.GetTextLineHeightWithSpacing and imgui.GetTextLineHeightWithSpacing()) or (editor_font_size * 1.35)
            local avail_pw, avail_ph = imgui.GetContentRegionAvail()
            local visual_lines = 0
            for line in string.gmatch((editor_text or "") .. "\n", "(.-)\r?\n") do
                local wrap_chars = math.max(20, math.floor((avail_w - 40) / (9.5 * content_zoom)))
                visual_lines = visual_lines + math.max(1, math.ceil(#line / wrap_chars))
            end
            local editor_box_h = math.max(avail_ph or 200, (visual_lines + 4) * line_h + 60)

            local changed, new_text = imgui.InputTextMultiline("##editor_full_" .. editor_epoch, editor_text, MAX_TEXT_BYTES, -1, editor_box_h, editor_input_flags)
            if imgui.PopFont then
                imgui.PopFont()
            end

            if just_formatted then
                just_formatted = false
            elseif changed and new_text then
                editor_text = new_text
            end

            -- Auto-scroll to bottom on Quick Action insert in single-editor mode
            if split.should_scroll then
                if imgui.SetScrollHereY then
                    imgui.SetScrollHereY(1.0)
                end
                split.should_scroll = false
            end

            imgui.EndChild()
        end

    elseif view_mode == "Preview" then
        -- 2. Fullscreen Live Rendered Preview
        if imgui.BeginChild("FullPreviewChild", 0, -28, 1) then
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
        -- 3. Side-by-Side Dual Pane with Draggable Sliding Divider
        local splitter_w = 6
        local pane_h     = -28
        local left_w     = math.max(60, math.min(avail_w - splitter_w - 60, math.floor((avail_w - splitter_w) * split.ratio)))
        local right_w    = avail_w - splitter_w - left_w

        -- Configure InputTextMultiline flags with or without word wrap:
        local split_flags = ImGuiInputTextFlags_AllowTabInput
        if split.word_wrap then
            split_flags = split_flags + ImGuiInputTextFlags_WordWrap
        end

        -- Left Pane: Non-scrolling outer frame with pinned header
        if imgui.BeginChild("SplitEditorOuter", left_w, pane_h, 1, 24) then
            imgui.AlignTextToFramePadding()
            imgui.TextDisabled("RAW MARKDOWN SOURCE:")

            -- Pinned right-aligned Auto-wrap ToggleButton (permanently visible!)
            local wrap_w = 105
            local cur_x = (imgui.GetCursorPosX and imgui.GetCursorPosX()) or 160
            local right_wrap_x = left_w - wrap_w - 15
            if right_wrap_x > cur_x + 10 then
                imgui.SameLine(right_wrap_x)
            else
                imgui.SameLine()
            end
            local wrap_changed, new_wrap
            if imgui.ToggleButton then
                wrap_changed, new_wrap = imgui.ToggleButton("Auto-wrap##opt_wrap", split.word_wrap)
            else
                wrap_changed, new_wrap = imgui.Checkbox("Auto-wrap##opt_wrap", split.word_wrap)
            end
            if wrap_changed then
                split.word_wrap = new_wrap
                editor_epoch = editor_epoch + 1
            end
            show_tooltip("**Auto-wrap (Word Wrap)**\n\nWrap long lines soft to pane width.\nClick or mouse-wheel to toggle on/off.\nUnchecked allows horizontal scrolling.")

            imgui.Separator()

            local editor_font = "RobotoMono-Regular"
            local editor_font_size = 16.0 * content_zoom
            if imgui.PushFont then
                imgui.PushFont(editor_font, editor_font_size)
            end

            -- Sizing (-1, -1) fills remaining height; native ImGui editor clamps to text with zero overscroll!
            local changed, new_text = imgui.InputTextMultiline("##editor_split_" .. editor_epoch, editor_text, MAX_TEXT_BYTES, -1, -1, split_flags)
            if imgui.PopFont then
                imgui.PopFont()
            end

            if just_formatted then
                just_formatted = false
            elseif changed and new_text then
                editor_text = new_text
            end

            -- Auto-scroll to bottom on Quick Action insert
            if split.should_scroll then
                if imgui.SetScrollHereY then
                    imgui.SetScrollHereY(1.0)
                end
                split.should_scroll = false
            end

            imgui.EndChild()
        end

        -- Interactive Draggable Splitter Divider between Left & Right Panes
        imgui.SameLine(0, 0)
        imgui.InvisibleButton("##split_divider", splitter_w, pane_h)
        if (imgui.IsItemHovered and imgui.IsItemHovered()) or (imgui.IsItemActive and imgui.IsItemActive()) then
            local cursor_ew = (imgui.constant and imgui.constant.MouseCursor and imgui.constant.MouseCursor.ResizeEW) or 4
            if imgui.SetMouseCursor then
                imgui.SetMouseCursor(cursor_ew)
            end
        end
        if imgui.IsItemActive and imgui.IsItemActive() then
            if imgui.GetMouseDragDelta then
                local dx, _ = imgui.GetMouseDragDelta(0, 0.0)
                if dx ~= 0 then
                    split.ratio = math.max(0.15, math.min(0.85, split.ratio + dx / math.max(1, avail_w)))
                    if imgui.ResetMouseDragDelta then
                        imgui.ResetMouseDragDelta(0)
                    end
                end
            end
        end
        if imgui.IsItemHovered and imgui.IsItemHovered() and imgui.IsMouseDoubleClicked and imgui.IsMouseDoubleClicked(0) then
            split.ratio = 0.50 -- Reset to 50/50 on double click
        end

        -- Subtle visual separator bar using window draw list
        local add_rect = imgui.DrawList_AddRectFilled or imgui.AddRectFilled
        if add_rect and imgui.GetItemRectMin and imgui.GetItemRectMax then
            local p0_x, p0_y = imgui.GetItemRectMin()
            local p1_x, p1_y = imgui.GetItemRectMax()
            local is_act = imgui.IsItemActive and imgui.IsItemActive()
            local is_hov = imgui.IsItemHovered and imgui.IsItemHovered()
            local bar_col = 0x33FFFFFF -- subtle 20% white bar
            if is_act then
                bar_col = 0xCC66AAFF -- active drag accent
            elseif is_hov then
                bar_col = 0x8888BBFF -- hover accent
            end
            local mid_x = math.floor((p0_x + p1_x) * 0.5)
            add_rect(mid_x - 1, p0_y + 2, mid_x + 1, p1_y - 2, bar_col, 1.0)
        end

        imgui.SameLine(0, 0)

        -- Right Pane: Non-scrolling outer frame with pinned header
        if imgui.BeginChild("SplitPreviewOuter", right_w, pane_h, 1, 24) then
            imgui.AlignTextToFramePadding()
            imgui.TextDisabled("LIVE RENDERED PREVIEW:")

            imgui.Separator()

            -- Inner scrolling child pane fills remaining height:
            if imgui.BeginChild("SplitPreviewScroll", -1, -1, 0) then
                if imgui.SetWindowFontScale then
                    imgui.SetWindowFontScale(content_zoom)
                end
                imgui.MD.TextWrapped(format_tasks(editor_text))
                imgui.EndChild()
            end

            imgui.EndChild()
        end
    end

    -- ------------------------------------------------------------------------
    -- BOTTOM STATUS BAR (With Directional Blossoming Quick Actions Popup)
    -- ------------------------------------------------------------------------
    render_bottom_status_bar(avail_w)

    -- ------------------------------------------------------------------------
    -- IN-COCKPIT FILE DIALOGS (Modal, VR-safe, non-blocking)
    -- ------------------------------------------------------------------------
    local notes_dir = get_notes_dir()
    if show_open_dialog and imgui.OpenFileDialog then
        local confirmed, chosen_path, still_open = imgui.OpenFileDialog("Select Note File", show_open_dialog, notes_dir, ".md;.txt")
        if confirmed then
            load_note_from_full_path(chosen_path)
        end
        show_open_dialog = still_open
    end

    if show_save_dialog and imgui.SaveFileDialog then
        local confirmed, save_path, still_open = imgui.SaveFileDialog("Save Note As", show_save_dialog, notes_dir, current_filename, ".md")
        if confirmed then
            save_note_to_full_path(save_path)
        end
        show_save_dialog = still_open
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
    -- Synchronize active theme preset across FlyWithLua windows
    if _G.FWL_ACTIVE_THEME_IDX == nil then
        _G.FWL_ACTIVE_THEME_IDX = 0
    end
    local target_theme = _G.FWL_ACTIVE_THEME_IDX
    if current_applied_theme_idx ~= target_theme then
        apply_theme(target_theme)
        current_applied_theme_idx = target_theme
    end

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

local saved_wnd_x = nil
local saved_wnd_y = nil
local saved_wnd_w = nil
local saved_wnd_h = nil

local function capture_window_geometry()
    if editor_wnd and float_wnd_get_geometry then
        local l, t, r, b = float_wnd_get_geometry(editor_wnd)
        if r and b and r > l and t > b then
            saved_wnd_w = r - l
            saved_wnd_h = t - b
            saved_wnd_x = l
            saved_wnd_y = b
        end
    end
end

function md_editor_on_close(wnd)
    -- Capture window bounds before destruction
    capture_window_geometry()
    editor_wnd = nil
    current_applied_theme_idx = nil
end

-- ----------------------------------------------------------------------------
-- Window Lifecycle (Lazy Startup & In-Cockpit Toggle)
-- ----------------------------------------------------------------------------
function show_cockpit_notes()
    if editor_wnd then return end

    local screen_w, screen_h = 1920, 1080
    if XPLMGetScreenSize then
        screen_w, screen_h = XPLMGetScreenSize()
    end

    local wnd_w = saved_wnd_w or 980
    local wnd_h = saved_wnd_h or 470
    local wnd_x = saved_wnd_x or math.max(30, math.floor((screen_w - wnd_w) * 0.10) + 200)
    local wnd_y = saved_wnd_y or math.max(40, math.floor((screen_h - wnd_h) * 0.15))

    -- Create ImGui floating window (4th arg MUST be true for withImgui)
    editor_wnd = float_wnd_create(wnd_w, wnd_h, 1, true)
    float_wnd_set_position(editor_wnd, wnd_x, wnd_y)
    float_wnd_set_title(editor_wnd, "Cockpit Notes v1.0.2 [Beta]")
    float_wnd_set_imgui_builder(editor_wnd, "md_editor_on_build")
    float_wnd_set_onclose(editor_wnd, "md_editor_on_close")

    -- Explicitly disable outer window scrollbars and mouse-wheel scrolling so the
    -- window never overflows horizontally. Inner child panes handle vertical scrolling.
    if float_wnd_set_scrolling then
        float_wnd_set_scrolling(editor_wnd, false)
    end

    -- Prevent resizing the window smaller than the minimum toolbar and pane bounds (740x280)
    if float_wnd_set_resizing_limits then
        float_wnd_set_resizing_limits(editor_wnd, 740, 280, 10000, 10000)
    end
end

function hide_cockpit_notes()
    if editor_wnd then
        capture_window_geometry()
        float_wnd_destroy(editor_wnd)
        editor_wnd = nil
    end
end

function toggle_cockpit_notes()
    if editor_wnd then
        hide_cockpit_notes()
    else
        show_cockpit_notes()
    end
end

-- ----------------------------------------------------------------------------
-- Command & Macro Menu Registration
-- ----------------------------------------------------------------------------
create_command(
    "FlyWithLua/CockpitNotes/toggle",
    "Toggle Cockpit Notes & Scratchpad window",
    "toggle_cockpit_notes()",
    "",
    ""
)

add_macro("Cockpit Notes: Toggle", "command_once('FlyWithLua/CockpitNotes/toggle')")
