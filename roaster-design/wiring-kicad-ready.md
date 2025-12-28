# Wiring notes — KiCad-ready (500 g drum roaster, LIN 4118L motor, 5 mm shaft)

Overview / choices
- Drum size: perforated stainless drum 150 mm OD × 180 mm length.
- Motor: LIN Engineering 4118L (2.10 A/phase, 1.8° step, 5 mm shaft).
- Stepper driver: DM542 recommended (TB6600 alternative).
- VMOT: 24 V DC supply for driver(s).
- MCU: Arduino Uno / Nano (real-time PID + step generation).
- Optional UI: Raspberry Pi over USB/serial (logging, GUI). Arduino remains the realtime controller.
- Heater: 120 V AC, 1500 W tubular element switched by 40 A zero-cross SSR.
- Fans: 12 V DC blower(s) controlled by logic-level N‑channel MOSFET (low-side PWM).
- Thermocouples: 2 × K-type -> Adafruit MAX31856 breakouts (SPI).

Safety summary (must read)
- Mains Live (hot) must be fused before E‑Stop and before the high-limit thermostat. E‑Stop must physically open Live conductor to isolate the heater.
- Protective Earth (PE) connected to frame and heater chassis.
- Use a dedicated 15 A (or 20 A if you select 2000 W heater) slow-blow fuse/circuit breaker on Live.
- SSR must be mounted on an adequate heatsink & kept away from low‑voltage wiring.
- Keep mains wiring physically separated from low-voltage control wiring and cable-glanded when passing into the control enclosure.

Arduino pin mapping (Uno example)
- D13 SCK    -> SPI SCLK (MAX31856)
- D12 MISO   -> SPI MISO (MAX31856)
- D11 MOSI   -> SPI MOSI (unused by MAX318xx but keep for expansion)
- D10 CS_BEAN -> MAX31856 CS Bean thermocouple
- D9  CS_EXH  -> MAX31856 CS Exhaust thermocouple
- D6  SSR_HEATER -> SSR_IN+ (SSR_IN- -> Arduino GND) (Use 220 Ω series if desired)
- D5  FAN_PWM -> MOSFET gate (gate resistor 100–220 Ω, gate pulldown 100 kΩ)
- D3  STEP    -> DM542 PUL- (PUL+ -> +5 V)
- D4  DIR     -> DM542 DIR- (DIR+ -> +5 V)
- D7  ENABLE  -> DM542 ENA- (ENA+ -> +5 V) (optional; active-low to enable)
- D8  ESTOP_IN -> E-Stop supervisory input (INPUT_PULLUP; low = E‑Stop pressed)
- A4/A5 (SDA/SCL) -> OLED display (I2C) if used
- 5 V -> driver logic +5 V (power DM542 PUL+/DIR+/ENA+)
- GND -> common ground for Arduino, driver logic, MOSFET source, PSU negative

Stepper driver wiring (DM542 / TB6600 style)
- VMOT (+) -> +24 V DC supply
- VMOT (−) -> 0 V (PSU negative)
- PUL+ -> +5 V (logic +5V)
- PUL- -> Arduino STEP (D3) (active low pulse)
- DIR+ -> +5 V
- DIR- -> Arduino DIR (D4)
- ENA+ -> +5 V
- ENA- -> Arduino ENABLE (D7)  (tie ENA- to ENA+ for always-enabled or control from Arduino)
- MOT A+/A− and B+/B− -> motor coil wires (identify coil pairs with multimeter)
- Set microstep DIP switches (1/8) and set current limit to 2.10 A (per driver instructions)

Stepper physical wiring / coil identification
- Disconnect motor from driver for measurement.
- On a 4-wire bipolar: there are two coil pairs. Use multimeter to find pairs (two wires with low resistance between them).
- Connect one pair to A+/A− and the other to B+/B−. If motor turns the wrong direction, swap one coil pair (A+/A− <-> B+/B−) or invert DIR wiring.

SSR & heater mains wiring (textual)
- Main IN (L, N, PE) -> Fuse (15 A slow) on Live -> E-Stop (mains-rated, breaks Live)
- After E-Stop -> High-limit thermostat (NC) -> SSR LOAD1 -> SSR LOAD2 -> Heater Live
- Heater neutral -> Mains neutral
- PE -> Frame / heater chassis
- SSR control: SSR_IN+ -> Arduino D6 (or through 220 Ω), SSR_IN- -> Arduino GND
- Use zero-cross SSR and time-proportional (window) control (suggest 1000 ms window)

Fan MOSFET wiring (low-side)
- Fan + -> 12 V supply +
- Fan - -> MOSFET drain
- MOSFET source -> supply GND
- MOSFET gate -> Arduino D5 via 100–220 Ω, gate pulldown 100 kΩ
- Use a logic-level MOSFET that can handle stall/inrush (IRLZ44N or preferably a modern low-Rds-on MOSFET e.g., IRLZ44N replacement / IRLZ34)

Control & monitoring sensors
- MAX31856 (Bean)
  - VCC -> 3.3 V or 5 V per breakout spec (Adafruit typical uses 3.3 V) — match MCU logic or use level shifter
  - GND -> GND
  - SCK -> D13, MISO -> D12, CS -> D10
- MAX31856 (Exhaust)
  - same SPI bus, CS -> D9

Power supplies
- 120 V mains -> 24 V DC PSU (for DM542 VMOT) — 24 V, 6–8 A recommended (conservative)
- 12 V DC PSU (for fans and cooling) — 12 V, 5–10 A depending fan selection
- 5 V for Arduino from regulated 5 V SMPS or USB (2 A minimum for Arduino + display + driver logic)

Netlist (core items)
- Arduino Uno / Nano
- 2 × Adafruit MAX31856 Thermocouple amplifiers
- 1 × DM542 stepper driver (or TB6600)
- 1 × LIN 4118L stepper (you already have)
- 1 × 24 V 6–8 A SMPS (for drivers)
- 1 × 12 V 5–10 A SMPS (fans/cooling)
- 1 × SSR 40 A zero-cross 3–32 V DC input
- 1 × MOSFET (logic level N-channel)
- 1 × 120 V 1500 W tubular heating element
- 1 × 15 A slow-blow fuse or breaker
- 1 × E-Stop (mains-rated)
- 1 × High-limit thermostat (manual or auto reset)
- terminal blocks, cable glands, crimp ferrules, heat-shrink tubing

Driver configuration summary (DM542)
- Microstep switches: set for 1/8 step (see driver doc)
- Current pot / Vref: set to match 2.10 A/phase (follow DM542 Vref->Current chart)
- Cooling: fan or heatsinking for driver under sustained use

Testing checklist (short)
1. Power up low-voltage (24 V, 12 V, 5 V). Verify no shorts.
2. With 24 V supply only, verify driver logic: power driver, check indicator LEDs.
3. Without motor attached, verify STEP/DIR pulses light the PUL LED on driver.
4. Connect motor, set driver current to minimum, test small movements, then raise incrementally to 2.10 A while watching temperature.
5. Verify thermal cutoff and E-Stop remove mains Live to heater immediately.
6. Use incandescent lamp as temporary test load before connecting heater.

Notes & extension items (if you want the full KiCad .sch)
- I can export a KiCad schematic (symbol + netlist + PDF) with labeled terminal blocks, SSR terminals, driver symbol, Arduino pin labels and recommended connectors. Request “KiCad schematic” and I’ll produce downloadable files.
- I can also produce a wiring harness pinout PDF for your builder.
