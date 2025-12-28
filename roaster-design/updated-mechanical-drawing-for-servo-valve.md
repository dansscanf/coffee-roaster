# Mechanical drawing notes — Servo (and Stepper) actuated Valve & Burner integration (Reference)

This file documents mechanical integration options for a valve actuator (both servo and stepper). The final design in this repo uses a stepper-actuated valve; the servo notes are kept for reference.

Design goals
- Place burner under insulated jacket with a diffuser plate to avoid hotspots.
- Solenoid upstream and accessible for inspection.
- Ball valve at a convenient location for mounting actuator; use a short lever ball valve for easy actuation (quarter-turn).
- Actuator mounted on bracket with mechanical coupling (horn/clamp) to valve lever; include closed-position microswitch.

Recommended mechanical choices
- For stepper actuation: prefer planetary gearbox on NEMA17 stepper (5:1 or 10:1) and clamp coupling to valve lever or stem.
- For servo actuation: use metal gear servo with bracket and clamp (if using servo note torque requirements—prefer stepper for reliability).

Mounting & material notes
- All parts exposed to heat should be stainless or heat tolerant.
- Keep electronics (actuator driver, Arduino) out of direct heat path. Make a small non-flammable mounting shelf for actuator.
- Use flexible high-temp hoses and stainless fittings for gas lines near the burner area.
