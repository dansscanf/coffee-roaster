# Coffee Roaster — Current Status (snapshot)

Date: 2025-12-28

This status file summarizes the current design state, files present in roaster-design/, decisions made, safety items, outstanding tasks, and next steps. Use this to resume work later or to share with collaborators.

---  
## Repo & folder
Path: `roaster-design/` in repository `dansscanf/coffee-roaster`.

Files present (current snapshot)
- README.md
- wiring-kicad-ready.md
- mechanical-drawing-updated.md  (updated for 140 mm × 180 mm drum)
- updated-wiring-and-control.md
- updated-mechanical-drawing-for-servo-valve.md
- gas_valve_stepper_roaster.ino
- mechanical-drawing.svg  (updated for 140 mm × 180 mm drum)
- evaluator.py
- design_spec.yaml (drum diameter updated to 140 mm)
- create_roaster_files.sh (script used to populate repo)
- update_drum_140.sh (script used to update drum files)

---  
## Key design choices (final / current)
- Roast capacity target: 500 g batch.
- Drum: 140 mm OD × 180 mm length (perforated stainless).
- Drum shaft: 10 mm stainless rod, baffles: 3 × ~18 mm paddles.
- Drive motors: LIN 4118L steppers (drum + valve actuator).
- Stepper drivers: DM542 (24 V VMOT).
- Gas system (optional): upstream normally‑closed (NC) solenoid; stepper-actuated ball valve for modulation; electronic spark igniter; IR/UV flame detector; exhaust blower.
- Fans: 12 V centrifugal blower for exhaust/jacket; 12 V axial/blower for cooling tray.
- Control: Arduino Uno/Nano realtime PID (bean probe via MAX31856), optional Raspberry Pi for logging/UI.
- Safety: E‑Stop, hardware high-limit thermostat recommended, CO detector recommended, flame lockout logic implemented in firmware.

---  
## Safety items — implemented & required
Implemented in docs / firmware:
- Flame detection and ignition timeout logic included in sketch.
- Software lockout on ignition failure.
- Valve homing and valve closed as safe position in code.

Must implement in hardware before testing with gas:
- NC solenoid physically wired upstream of modulating valve (fail‑closed).
- Mechanical high-limit thermostat in series with solenoid power (hardware cutoff).
- Mains fuse / breaker and mains E‑Stop that physically open Live conductor.
- CO detector and ventilation for indoor use.
- Proper gas regulator, pressure gauge, leak testing (soap test).
- Flame detector placement and commissioning tests.

---  
## Tests performed / recommended commissioning order
(Do these in order; stop on any anomaly)

1. Low-voltage smoke tests (done or must do)
   - Power 5 V and 24 V rails; verify no short circuits.
2. Steppers & drivers (no gas)
   - Set driver current, test microsteps, verify homing (valve) and drum rotation.
3. Igniter & solenoid (no gas)
   - Verify solenoid energizes and closes when de‑energized. Verify igniter pulses.
4. Fan & MOSFET (no gas)
   - Test soft-start fan ramp on 12 V supply; verify MOSFET wiring and fuse.
5. Gas plumbing (expert if needed)
   - Pressure test for leaks (soap test), verify regulator, manual valve, and accessible shutoff.
6. Ignition with no bean load (outdoors or ventilated)
   - Verify pilot ignition, flame stability under low blower speed.
7. Stepwise blower ramp & flame stability checks
   - Determine safe blower duty and static pressure operating window; record values.
8. Short monitored roasts
   - With CO monitor & extinguisher, run short roasts, tune PID and fan mapping.
9. Full roast + cooldown tests

---  
## Outstanding / next tasks
- [ ] Add MOSFET wiring diagram to KiCad schematic (I can generate).
- [ ] Add fan PWM routine to firmware in repo (code snippet provided earlier; integrate & test).
- [ ] Acquire and mount blower and MOSFET parts; installer must size fuses and PSUs.
- [ ] Generate DXF of adapter flange (for blower) and finalize drilling pattern for your chosen blower model.
- [ ] Commission gas system outdoors with manometer and CO monitor; record safe draft/pressure setpoints.
- [ ] Create GitHub Actions to run `evaluator.py` on each push (optional).

---  
## Useful commands (create this file locally)
Paste the following into WSL to write this exact file into your local clone:

cat > roaster-design/STATUS.md <<'EOF'
# Coffee Roaster — Current Status (snapshot)

Date: 2025-12-28

This status file summarizes the current design state, files present in roaster-design/, decisions made, safety items, outstanding tasks, and next steps. Use this to resume work later or to share with collaborators.

---  
## Repo & folder
Path: `roaster-design/` in repository `dansscanf/coffee-roaster`.

Files present (current snapshot)
- README.md
- wiring-kicad-ready.md
- mechanical-drawing-updated.md  (updated for 140 mm × 180 mm drum)
- updated-wiring-and-control.md
- updated-mechanical-drawing-for-servo-valve.md
- gas_valve_stepper_roaster.ino
- mechanical-drawing.svg  (updated for 140 mm × 180 mm drum)
- evaluator.py
- design_spec.yaml (drum diameter updated to 140 mm)
- create_roaster_files.sh (script used to populate repo)
- update_drum_140.sh (script used to update drum files)

---  
## Key design choices (final / current)
- Roast capacity target: 500 g batch.
- Drum: 140 mm OD × 180 mm length (perforated stainless).
- Drum shaft: 10 mm stainless rod, baffles: 3 × ~18 mm paddles.
- Drive motors: LIN 4118L steppers (drum + valve actuator).
- Stepper drivers: DM542 (24 V VMOT).
- Gas system (optional): upstream normally‑closed (NC) solenoid; stepper-actuated ball valve for modulation; electronic spark igniter; IR/UV flame detector; exhaust blower.
- Fans: 12 V centrifugal blower for exhaust/jacket; 12 V axial/blower for cooling tray.
- Control: Arduino Uno/Nano realtime PID (bean probe via MAX31856), optional Raspberry Pi for logging/UI.
- Safety: E‑Stop, hardware high-limit thermostat recommended, CO detector recommended, flame lockout logic implemented in firmware.

---  
## Safety items — implemented & required
Implemented in docs / firmware:
- Flame detection and ignition timeout logic included in sketch.
- Software lockout on ignition failure.
- Valve homing and valve closed as safe position in code.

Must implement in hardware before testing with gas:
- NC solenoid physically wired upstream of modulating valve (fail‑closed).
- Mechanical high-limit thermostat in series with solenoid power (hardware cutoff).
- Mains fuse / breaker and mains E‑Stop that physically open Live conductor.
- CO detector and ventilation for indoor use.
- Proper gas regulator, pressure gauge, leak testing (soap test).
- Flame detector placement and commissioning tests.

---  
## Tests performed / recommended commissioning order
(Do these in order; stop on any anomaly)

1. Low-voltage smoke tests (done or must do)
   - Power 5 V and 24 V rails; verify no short circuits.
2. Steppers & drivers (no gas)
   - Set driver current, test microsteps, verify homing (valve) and drum rotation.
3. Igniter & solenoid (no gas)
   - Verify solenoid energizes and closes when de‑energized. Verify igniter pulses.
4. Fan & MOSFET (no gas)
   - Test soft-start fan ramp on 12 V supply; verify MOSFET wiring and fuse.
5. Gas plumbing (expert if needed)
   - Pressure test for leaks (soap test), verify regulator, manual valve, and accessible shutoff.
6. Ignition with no bean load (outdoors or ventilated)
   - Verify pilot ignition, flame stability under low blower speed.
7. Stepwise blower ramp & flame stability checks
   - Determine safe blower duty and static pressure operating window; record values.
8. Short monitored roasts
   - With CO monitor & extinguisher, run short roasts, tune PID and fan mapping.
9. Full roast + cooldown tests

---  
## Outstanding / next tasks
- [ ] Add MOSFET wiring diagram to KiCad schematic (I can generate).
- [ ] Add fan PWM routine to firmware in repo (code snippet provided earlier; integrate & test).
- [ ] Acquire and mount blower and MOSFET parts; installer must size fuses and PSUs.
- [ ] Generate DXF of adapter flange (for blower) and finalize drilling pattern for your chosen blower model.
- [ ] Commission gas system outdoors with manometer and CO monitor; record safe draft/pressure setpoints.
- [ ] Create GitHub Actions to run `evaluator.py` on each push (optional).

---  
## Useful commands (commit & push)
git add roaster-design/STATUS.md
git commit -m "Add STATUS snapshot — 2025-12-28"
git push origin main

EOF

---  
## Download / copy options
1. Copy‑paste the `cat > ... <<'EOF'` block above into WSL to write the file locally (fast & reliable).
2. Or copy the file content from this code block directly and save it in your editor as `roaster-design/STATUS.md`.

---  
If you want, I can:
- Create a small script to commit & push this STATUS.md automatically (you must run it locally), or
- Convert this status snapshot to a PDF and provide a ZIP for download.

Reply with which you prefer (script to commit, PDF+ZIP, or nothing) and I’ll prepare it.