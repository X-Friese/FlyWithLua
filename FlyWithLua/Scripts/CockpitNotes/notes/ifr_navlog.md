# IFR Navigation Log & Route Plan

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
