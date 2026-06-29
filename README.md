# Beater-Bar BattleBot

A 3-pound (beetleweight) combat robot designed and built across the full hardware stack: mechanical CAD, PCB layout, and embedded firmware. The goal was a **reproducible end-to-end build process** — from CAD model to fabricated board to driving firmware — rather than peak performance. V2 will optimize for weight and combat performance.

> A full reflection on the build is on Substack: [Earning Three Minutes in the Ring](https://open.substack.com/pub/carterstonesifer/p/earning-three-minutes-in-the-ring?r=2y2f28&utm_campaign=post&utm_medium=web&showWelcomeOnShare=true)

**Skills demonstrated:** Fusion 360 / FDM prototyping · KiCad schematic + layout · custom footprint creation · Gerber fabrication output · ESP32 embedded C · RC PWM decode · LEDC motor PWM · skid-steer mixing

---

## Repo Structure

```
cad/          — 3D-printable chassis, wheels, and weapon (STL + 3MF)
pcb/          — KiCad schematic, layout, custom footprints, and gerbers
firmware/     — Arduino sketch for ESP32 (RC decode + motor mixing)
```

---

## Design Decisions

- **Beater bar weapon** — a relatively simple and proven design with high impact potential in the arena. The main weakness is being flipped; a lower profile in V2 addresses this directly.
- **Unified motor driver (DRV8833)** — the same driver module is used for all three motors (left drive, right drive, weapon). Simplifies the BOM and firmware; the weapon motor runs off its own dedicated DRV8833 instance.
- **Skid-steer drivetrain** — keeps mechanical complexity low while maintaining full mobility. No steering linkage, no Ackermann geometry, just two independently driven wheels.
- **Belt-driven weapon** — the GT2 20T/60T pulley ratio amplifies the motor's output speed to spin the beater bar faster than a direct-drive setup would allow.
- **Unitary printed body** — a single-piece chassis provides structural strength without mechanical interlocks between parts. Drive and weapon geometry were validated as separate prints first, then merged once the layout was confirmed.

---

## Bill of Materials

| # | Component | Part | Qty | Specs | Link |
|---|-----------|------|-----|-------|------|
| 1 | MCU | ESP32 dev board | 1 | Dual-core 32-bit, 240 MHz, 3.3V, WiFi + BT | [Amazon](https://www.amazon.com/dp/B0C9TGJRPH) |
| 2 | Motor driver | DRV8833 | 3 | 1.5 A continuous, 2 A peak | [Amazon](https://www.amazon.com/dp/B0DXF6LZB1) |
| 3 | Buck regulator | LM2596 | 1 | 3–40V in, 1.5–35V adj out | [Amazon](https://www.amazon.com/dp/B0DBVYP91F) |
| 4 | Battery | 2S LiPo | 1 | 7.4V, 2000mAh, T-plug | [Amazon](https://www.amazon.com/dp/B0FCFVWS1K) |
| 5 | RC transmitter + receiver | — | 1 set | PWM out, 2-channel minimum | [Amazon](https://www.amazon.com/dp/B0FJDFJ38M) |
| 6 | Drive motors | 390 brushed | 2 | 6–12V, 6,300 RPM @ 12V, 1A no-load, 2.3mm shaft | [Amazon](https://www.amazon.com/dp/B01M58POHF) |
| 7 | Weapon motor | 550 brushed, 55T | 1 | 7.4V (2S), 36mm dia, 3.17mm shaft | [Amazon](https://www.amazon.com/dp/B0FXG6J3Z9) |
| 8 | Weapon belt drive | GT2 pulley + belt | 1 set | 20T / 60T, 8mm bore, 200mm belt | [Amazon](https://www.amazon.com/dp/B08QZ4365D) |
| 9 | Connectors | JST-XH 3-pin | 10 pairs | 2.54mm pitch, 26AWG, 200mm | [Amazon](https://www.amazon.com/dp/B0D3LVLMP9) |
| 10 | Print filament | PLA-CF 1.75mm | 1 spool | 1kg, carbon-fiber reinforced | [Amazon](https://www.amazon.com/dp/B0D7VV1YC7) |

---

## CAD

**Tool:** Fusion 360 → FDM (PLA-CF)  
**All parts:** `cad/` — available as both `.stl` and `.3mf`

| File | Description |
|------|-------------|
| `chassis.stl` / `.3mf` | Unitary chassis: electronics bay, drive motor mounts, weapon mount |
| `drive_wheel.stl` / `.3mf` | Drive wheels (print ×2) |
| `weapon_impactor.stl` / `.3mf` | Beater-bar weapon impactor |

Drive and weapon subsystems were first validated as separate prints, then consolidated into a single `chassis` once geometry was confirmed. Total bot target: ≤ 3 lb; drive motors are the dominant mass — body is designed to be as light as possible.

| | |
|---|---|
| ![Fusion 360 chassis render](resources/CAD-3d-render.png) | ![Assembled Mark 1](resources/Mark_1.png) |
| *Fusion 360 render — `chassis`* | *Mark 1 assembled — PCB and drive motor installed* |

---

## ECAD

**Tool:** KiCad 7  
**Fabrication output:** `pcb/gerbers/battlebot_gerbers_v1.zip` — ready to send to any fab house

### Files

| File | Purpose |
|------|---------|
| `pcb/BattleBot.kicad_sch` | Full schematic |
| `pcb/BattleBot.kicad_pcb` | Board layout |
| `pcb/BattleBot-parts.kicad_sym` | Custom KiCad symbol library |
| `pcb/BattleBot.pretty/` | Custom footprint library (6 footprints) |
| `pcb/gerbers/` | Gerber + drill files |

### Power Architecture

```
LiPo ──→ LM2596 (buck) ──→ 5V ──→ ESP32-WROVER-E
LiPo ──→ DRV8833 (×3) ──→ Drive motors + weapon motor (direct VBAT)
```

### GPIO Pin Assignments

| Signal | ESP32 GPIO |
|--------|-----------|
| RC Steering (PWM in) | 35 |
| RC Throttle (PWM in) | 34 |
| RC Weapon toggle (PWM in) | 18 |
| Left motor IN1 | 25 |
| Left motor IN2 | 26 |
| Right motor IN1 | 27 |
| Right motor IN2 | 14 |
| Weapon motor IN1 | 2 |
| Weapon motor IN2 | 15 |
| UART0 debug | 115200 baud |

### Custom Footprints

All footprints in `pcb/BattleBot.pretty/` were drawn from datasheets:

| Footprint | File |
|-----------|------|
| ESP32-WROVER-E | `ESP32-WROVER-E.kicad_mod` |
| DRV8833 | `DRV8833.kicad_mod` |
| LM2596 | `LM2596.kicad_mod` |
| RC Receiver | `Receiver.kicad_mod` |
| 3-pin connector | `3Pin-Connector.kicad_mod` |
| Test point pad | `Testpoint-Pad.kicad_mod` |

![KiCad schematic](resources/kicad_schematic.png)
*Schematic — `BattleBot.kicad_sch`*

| | |
|---|---|
| ![KiCad PCB layout](resources/kicad_pcb.png) | ![PCB 3D render](resources/PCB%203D%20Render.png) |
| *Layout — `BattleBot.kicad_pcb`* | *KiCad 3D render* |

![Populated PCB](resources/final_pcb.jpg)
*Fabricated and populated board*

---

## Firmware

**Toolchain:** Arduino IDE + ESP32 board support package  
**Board target:** ESP32 Wrover Module  
**Sketch:** `firmware/signal_driver/signal_driver.ino`

### Control Loop

```
loop()
  ├── pulseIn GPIO 35/34/18  — read steering, throttle, weapon (25ms timeout each)
  ├── failsafe check         — any invalid/missing pulse → stopAll() and skip frame
  ├── steeringFromPulse()    — map to [-100, 100] → deadband
  ├── throttleFromPulse()    — map to [-100, 100] → deadband
  ├── mixAndDrive()          — skid-steer mix → setMotor() → LEDC PWM duty
  └── setWeapon()            — full speed on WEAPON_IN1 when pulse > 1500 µs
```

> **Failsafe:** `pulseIn` returns 0 on timeout, and a valid RC frame sits ~1000–2000 µs. Any pulse outside the 900–2100 µs band is treated as link loss — all drive and weapon output is cut for that loop. Without this guard a lost signal maps to full reverse, so the bot drives off on its own when the transmitter drops.

### Motor Mixing

- **Straight:** `left = right = throttle`
- **Driving turn:** inner wheel = `throttle × (TURN_MIX_RATIO / 100)`
- **Spin-in-place:** `left = +steering`, `right = −steering`

### Calibration Constants

| Constant | V1 Value | Purpose |
|----------|----------|---------|
| `STEERING_MIN` | 965 µs | Min receiver pulse |
| `STEERING_MAX` | 1882 µs | Max receiver pulse |
| `THROTTLE_MIN` | 1055 µs | Min throttle pulse |
| `THROTTLE_IDLE` | 1565 µs | Neutral position |
| `THROTTLE_MAX` | 2000 µs | Max throttle pulse |
| `DEADBAND` | 5 | Noise rejection around neutral |
| `TURN_MIX_RATIO` | 50 | Inner-wheel scaling (0–100) |
| `PWM_FREQ` | 20,000 Hz | LEDC motor PWM frequency |
| `PWM_RES` | 8-bit | Duty cycle range (0–255) |

> These constants are radio-specific. Re-measure pulse widths for your transmitter before first drive.

### Flash Instructions

1. Install ESP32 board support in Arduino IDE
2. Open `firmware/signal_driver/signal_driver.ino`
3. Select **ESP32 Wrover Module**, set the correct COM port
4. Update calibration constants for your RC radio
5. Upload

---

## Testing & Validation

The bot was driven under full battery power with all systems operating independently:

- **Drive** — successfully accelerated, turned, and reversed at varying speeds under RC control.
- **Weapon** — spun up to full speed and was toggled on and off from the transmitter.
- **Carpet performance** — struggled on carpet. The wheels lacked grip, and the design relies on the body sliding along the ground to stay low against opponents — which doesn't work well on high-friction surfaces.
- **Battery connector** — the 3-pin plugs do not physically prevent reverse polarity insertion. Connecting the battery backwards is easy to do and will damage the board.

<img src="resources/fried.jpg" width="300" alt="Fried ESP32 from reverse polarity" />
<br><em>ESP32 after a flipping power contacts</em>

---

## Known Issues & Further Improvements (V2)

- **Chassis Side Overhangs** — the overhangs on the side were quickly added to thicken the width of the walls to strengthen the bot, and for style. They provide a grip for another bot to easily flip the vehicle. The next design should minimize the gap between the side edges and the ground.
- **Hard Power Cut-Off** — as per NHRL rules, the bot requires a quick physical power disconnect to disable movement and weapon systems, not just a code deactivation.
- **Reverse-polarity protection** — V1 fried a board because the 3-pin power plug can be inserted backwards. V2 should add a series Schottky / P-channel MOSFET ideal-diode at the battery input *and* a keyed/slotted connector so the battery physically can't seat the wrong way. The board itself is cheap and easily reproducible from the gerbers, so losing one to this wasn't costly — but it's a trivial thing to design out.
- **Undersized motor drivers** — the DRV8833 is rated 1.5 A continuous / 2 A peak, but the 390 drive motors stall around 5–7 A and the 550 weapon motor well above that. Under real load the part is pushed past its peak, where it current-limits or fails. V2 should size the H-bridge to *stall* current, not no-load current, and add a firmware soft-start ramp so the weapon spins up gradually instead of slamming to full duty and drawing a large inrush spike. The broader lesson: parts that work together aren't necessarily matched on performance — each one needs headroom for the worst case the system will actually see.
- **Hardened weapon impactor** — machine or print in PETG/nylon/polycarbonate; evaluate a metal impactor for the beater bar.
- **Weight optimization** — the current design is 2.76 lbs, which leaves room to upgrade/add additional parts (i.e. the weapon material should be metal).
- **Optimize print settings** — a final print with more granular layer heights and higher infill will strengthen the hull.
- **A Roof** — the top of the bot needs a solid roof that could also act as a battery retainer. Batteries are currently loose within the chassis.
- **Sweet Paint Job** — the bot needs some decals and a layer of spray paint to make it look more threatening.
- **Wheel Grips and Covers** — the wheels are exposed and use electrical tape for grip. They must be better protected, as immobility is an immediate loss in the arena. A set of proper tires would help with mobility, and the front shovel can catch on uneven surfaces.
- **Beater Bar orientation is reversed** — the edge of the beater bar faces inward, counter to the direction the bar spins. Opposing robots are hit with the blunt edge of the weapon rather than the sharp, scraping side that should brush close to the ground.
- **Blocking RC read (`pulseIn`)** — the firmware reads each RC channel sequentially using `pulseIn()`, which blocks for up to ~25ms per channel, so a full loop can take ~75ms. Signal loss is now handled (an invalid/missing pulse disarms all motors), but the read is still blocking. V2 should use hardware interrupts or RTOS tasks to read channels concurrently.

---

## Tools Required

- **3D printer** — Bambu A1 (or equivalent FDM printer capable of PLA-CF)
- **Soldering iron** — flux is mandatory; makes the joints significantly cleaner and easier
- **Wire stripper**
- **Extra header pins**
- **Assorted screws and a screwdriver**

---

## Status

| Version | Status | Focus |
|---------|--------|-------|
| V1 | Complete | Cost-optimized, full-stack process established |
| V2 | Planned | Weight/performance, hardened materials, active fault handling |
