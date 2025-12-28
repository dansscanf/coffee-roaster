# Updated Wiring & Control Notes — Gas‑fired Roaster with Stepper‑modulated Valve

This document updates the electrical design for a propane (or natural gas) 500 g drum roaster using:
- a stepper-actuated ball valve (modulation using LIN 4118L stepper)
- an upstream normally‑closed (NC) gas solenoid (fail‑safe)
- an electronic spark igniter
- a flame detector
- Arduino for realtime control (PID → valve position)
- optional Raspberry Pi for logging/UI
- stepper motor for drum drive (LIN 4118L, 2.10 A) using DM542/TB6600 driver

Important safety summary (read first)
- Solenoid must be normally‑closed (NC). When de‑energized it must block gas.
- Use a gas regulator appropriate for your burner and cylinder; NEVER feed cylinder pressure directly to valves.
- Add a mechanical high‑limit thermostat that cuts solenoid power on overtemp.
- Flame detector + ignition timeout + auto lockout on ignition failure are mandatory.
- Install CO detector and ensure proper ventilation.
- If unsure about plumbing, hire a licensed gas fitter to do the gas connections.

Control hardware overview
- Valve actuator: LIN 4118L stepper (2.10 A) driving valve via a geared reduction (recommended) or direct coupling if torque allows.
- Driver: DM542 per stepper (drum + valve). Share 24 V VMOT supply (size appropriately).
- Microstepping: 1/8–1/16 microstepping for fine control.
- Homing: closed‑position limit switch for valve homing; Arduino homes valve at startup.
- Ignition: ignition routine opens the valve a small calibrated number of steps (pilot opening) then pulses igniter until flame detected.

Key signals and wiring
- VALVE_STEP -> DM542 PUL- (PUL+ -> +5 V)
- VALVE_DIR  -> DM542 DIR- (DIR+ -> +5 V)
- VALVE_HOME -> limit switch to Arduino input (active LOW)
- SOLENOID_CTRL -> MOSFET/relay driving solenoid coil
- IGNITER_CTRL -> MOSFET/relay driving ignition module
- FLAME_IN -> flame detector digital input
- Other signals: FAN_PWM, DRUM stepper PUL/DIR, thermocouples on SPI (MAX31856), SSR removed for gas heater

Software behavior
- Homing: on startup, homing routine steps valve toward closed until home switch asserts; sets position zero.
- PID: bean temperature PID maps to valve open steps (0..VALVE_OPEN_STEPS)
- Ignition: energize solenoid, move valve to pilot steps, pulse igniter until flame detected or timeout -> lockout
- Safety: E‑Stop and high-limit kill solenoid power and set lockout; flame loss triggers relight attempts, then lockout if unsuccessful.

Valve motion mapping
- Motor 1.8° step (200 full steps/rev); microstepping e.g., 1/16; gearbox ratio e.g., 5:1
- Steps for 90° = 200 * microstep * gear * 0.25
- PID output 0..100 → map linearly to 0..VALVE_OPEN_STEPS

Power / driver sizing
- VMOT: 24 V, 6–8 A recommended for two steppers (drum + valve) and driver headroom
- Solenoid & igniter: 12 V supply sized for their coil and igniter current

Safety wiring best practices
- Wire high-limit thermostat in series with solenoid coil supply (hardware cutoff on overtemp)
- E-Stop should immediately de-energize solenoid and optionally the driver enable pins
- Keep mains wiring and gas plumbing away from control electronics and high-temp surfaces

Testing and commissioning
- Test steppers/homing with no gas first. Verify valve motion, homing switch, and step counts.
- Bench-test ignition without gas: verify solenoid & igniter pulses and flame sensor logic (simulate flame)
- Pressure test gas plumbing for leaks before attempting ignition
- Conduct initial roasts outdoors or in well-ventilated area with extinguisher and CO detector

Parts sourcing suggestions
- DM542 or TB6600 drivers; NEMA17 planetary gearbox 5:1 or 10:1; 5 mm couplings; 1/4" gas ball valve; propane NC solenoid valve 1/4" NPT; IR flame sensor; spark igniter module; 24 V 6–8 A PSU; 12 V 5–10 A PSU.

Notes on safety and legal compliance
- For indoor or permanent installations consult a licensed gas fitter. Follow local codes and standards for gas appliances.
