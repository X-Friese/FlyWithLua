# Cockpit Notes & Scratchpad

<badge blue>FlyWithLua Phase 3 Suite</badge> <badge>Simulator: X-Plane 12</badge> <badge green>Status: Active</badge>

Welcome to **Cockpit Notes** for FlyWithLua!

## Quick Start
* **Modes:** Use the toolbar above to switch between **Write** (distraction-free editor), **Split** (side-by-side editing and live rendered preview), and **Preview** (clean briefing document).
* **Zoom:** Use `[-]` and `[+]` or the mouse wheel in the header to dynamically zoom text from **50% to 200%**. Click `↺` to toggle between 100% baseline and your previous zoom.
* **Aviation Templates:** Click the dropdown above to load pre-formatted checklists, ATIS copy pads, navigation logs, and fuel calculations.
* **In-Cockpit File Management:** Switch notes with the native `Note:` dropdown, or click **Open...** and **Save As...** to manage flight notes anywhere in-sim.
* **Interactive GFM Tables:** Standard markdown tables (`| Col 1 | Col 2 |`) support resizable columns and column sorting!
* **Async Remote Badges & Images:** Badges and online charts load smoothly in the background without frame drops.
* **Procedural Vector Icons:** Zero-font vector geometry rendered directly to screen:
  * HTML Tags: `<icon blue plane />` Flight Enroute, `<icon green check />` Checked, `<icon orange warning />` Caution
  * Emoji Shortcodes: `:plane:` Route, `:check:` Verified, `:warning:` Advisory
  * Raw-Fallback Macro: `#[^](ICON_CARET_UP)` Pitch Up, `#[↺](ICON_UNDO)` Reset
  * Rescued Raw Unicode: `↺` Reset, `↻` Restore, `▲` Up, `▼` Down, `✓` Check, `⚠` Warning
* **Inline Sized Typography:**
  * Sizing Tags: <tiny color=#888888>tiny (10px)</tiny>, <small color=gray>small (12px)</small>, <regular>regular (14px)</regular>, <big>big (16px)</big>, <huge color=orange>huge (16px+ bold)</huge>
* **Avionics Annunciators:**
  * Autopilot: <pill icon=plane color=#3498db>NAV / ALT</pill>
  * Transponder: <pill icon=check color=#2ecc71>ALT (Mode C)</pill>
  * Pitot Heat: <pill icon=warning orange>ON</pill>
  * Master Warning: <pill icon=warning color=#e74c3c />
  * Firmware: <badge icon=plane>v2.3.0</badge> <tiny color=#888888>(build 4268f3d)</tiny>

---

### Flight Progress Overview
| Fix / Point | Altitude | Distance | ETE | Fuel Rem | Status |
| :--- | :---: | :---: | :---: | :---: | :---: |
| **KORD** (Dep) | Field | 0 NM | 00:00 | 18,800 lbs | <badge icon=plane green>DEPARTED</badge> |
| **JOT** (VOR) | 5,000 | 28 NM | 00:07 | 17,900 lbs | <pill icon=check color=#2ecc71>PASSED</pill> |
| **BAE** (VOR) | FL240 | 64 NM | 00:15 | 16,200 lbs | <pill icon=check color=#2ecc71>PASSED</pill> |
| **DLL** (VOR) | FL340 | 78 NM | 00:18 | 14,800 lbs | <pill icon=plane color=#3498db>ACTIVE</pill> |
| **KMSP** (Dest) | Field | 142 NM | 00:32 | 9,400 lbs | <badge icon=plane yellow>PLANNED</badge> |

---

### Flight Notes & Checklist
- `[x]` <icon green check /> Departure clearance copied & verified
- `[ ]` :warning: Altimeter set & cross-checked
- `[ ]` #[^](ICON_CARET_UP) Departure runway confirmed
