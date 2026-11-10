# Fuel & Weight Planning

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
